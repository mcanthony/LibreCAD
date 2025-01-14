/****************************************************************************
**
 * Draw ellipse by foci and a point on ellipse

Copyright (C) 2011-2015 Dongxu Li (dongxuli2011@gmail.com)
Copyright (C) 2011 R. van Twisk (librecad@rvt.dds.nl)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**********************************************************************/

#include <QAction>
#include <QMouseEvent>
#include "rs_actiondrawellipsecenter3points.h"

#include "rs_dialogfactory.h"
#include "rs_graphicview.h"
#include "rs_commandevent.h"
#include "rs_circle.h"
#include "rs_ellipse.h"
#include "rs_coordinateevent.h"

/**
 * Constructor.
 *
 */
RS_ActionDrawEllipseCenter3Points::RS_ActionDrawEllipseCenter3Points(
    RS_EntityContainer& container,
    RS_GraphicView& graphicView)
        :RS_PreviewActionInterface("Draw ellipse by center and 3 points",
						   container, graphicView),
		  cData(new RS_CircleData({0.,0.},1.)),
		  eData(new RS_EllipseData{{0.,0.}, {1.,0.}, 1., 0., 0., false})
{
	actionType=RS2::ActionDrawEllipseCenter3Points;
}

RS_ActionDrawEllipseCenter3Points::~RS_ActionDrawEllipseCenter3Points(){}

void RS_ActionDrawEllipseCenter3Points::init(int status) {
    RS_PreviewActionInterface::init(status);

    if (status==SetCenter) {
        points.clean();
    }
    drawSnapper();
}



void RS_ActionDrawEllipseCenter3Points::trigger() {
    RS_PreviewActionInterface::trigger();


	RS_Ellipse* ellipse=new RS_Ellipse(container, *eData);

    deletePreview();
    container->addEntity(ellipse);

    // upd. undo list:
    if (document) {
        document->startUndoCycle();
        document->addUndoable(ellipse);
        document->endUndoCycle();
    }

    graphicView->moveRelativeZero(ellipse->getCenter());
    graphicView->redraw(RS2::RedrawDrawing);
    drawSnapper();

    setStatus(SetCenter);

    RS_DEBUG->print("RS_ActionDrawEllipseCenter3Points::trigger():"
                    " entity added: %d", ellipse->getId());
}



void RS_ActionDrawEllipseCenter3Points::mouseMoveEvent(QMouseEvent* e) {
    //    RS_DEBUG->print("RS_ActionDrawEllipseCenter3Points::mouseMoveEvent begin");
    RS_Vector mouse = snapPoint(e);
    if(getStatus() == SetCenter) return;
    points.resize(getStatus());
    points.push_back(mouse);
    if(preparePreview()) {
        switch(getStatus()) {

        case SetPoint1:
        {
			RS_Circle* circle=new RS_Circle(preview.get(), *cData);
            deletePreview();
            preview->addEntity(circle);
            drawPreview();
        }
            break;

        case SetPoint2:
        case SetPoint3:
        {
            deletePreview();
			RS_Ellipse* e=new RS_Ellipse(preview.get(), *eData);
            preview->addEntity(e);
            drawPreview();
        }
        default:
            break;
        }

    }
    RS_DEBUG->print("RS_ActionDrawEllipseCenter3Points::mouseMoveEvent end");
}


bool RS_ActionDrawEllipseCenter3Points::preparePreview(){
    valid=false;
    switch(getStatus()) {
    case SetPoint1:
    {
		RS_Circle c(preview.get(), *cData);
        valid= c.createFromCR(points.get(0),points.get(0).distanceTo(points.get(1)));
        if(valid){
			cData.reset(new RS_CircleData(c.getData()));
        }

    }
        break;
    case SetPoint2:
    case SetPoint3:
    {
		RS_Ellipse e(preview.get(), *eData);
        valid= e.createFromCenter3Points(points);
        if(valid){
			eData.reset(new RS_EllipseData(e.getData()));
        }
    }
        break;
    default:
        break;
    }
    return valid;
}

void RS_ActionDrawEllipseCenter3Points::mouseReleaseEvent(QMouseEvent* e) {
    // Proceed to next status
    if (e->button()==Qt::LeftButton) {
        RS_CoordinateEvent ce(snapPoint(e));
        coordinateEvent(&ce);
    }

    // Return to last status:
    else if (e->button()==Qt::RightButton) {
        deletePreview();
        init(getStatus()-1);
    }
}


void RS_ActionDrawEllipseCenter3Points::coordinateEvent(RS_CoordinateEvent* e) {
	if (!e) {
        return;
    }
    RS_Vector mouse = e->getCoordinate();
    points.alloc(getStatus()+1);
    points.set(getStatus(),mouse);

    switch (getStatus()) {
    case SetCenter:
        graphicView->moveRelativeZero(mouse);
        setStatus(SetPoint1);
        break;
    case SetPoint1:
    case SetPoint2:
        for(int i=0;i<getStatus()-1;i++) {
			if( (mouse-points.get(i)).squared() < RS_TOLERANCE15) {
                return;//refuse to accept points already chosen
            }
        }
//                setStatus(getStatus()+1);
//                break;
    case SetPoint3:
        if( preparePreview()) {
            if(getStatus() == SetPoint3) {
                trigger();
            }else{
                setStatus(getStatus()+1);
            }
        }
        break;

    default:
        break;
    }
}

//fixme, support command line

/*
void RS_ActionDrawEllipseCenter3Points::commandEvent(RS_CommandEvent* e) {
    QString c = e->getCommand().toLower();

    if (checkCommand("help", c)) {
        if (RS_DIALOGFACTORY) {
            RS_DIALOGFACTORY->commandMessage(msgAvailableCommands()
                                             + getAvailableCommands().join(", "));
        }
        return;
    }

    switch (getStatus()) {
    case SetFocus1: {
            bool ok;
            double m = RS_Math::eval(c, &ok);
            if (ok) {
                ratio = m / major.magnitude();
                if (!isArc) {
                    trigger();
                } else {
                    setStatus(SetAngle1);
                }
            } else {
                if (RS_DIALOGFACTORY) {
                    RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
                }
            }
        }
        break;

    case SetAngle1: {
            bool ok;
            double a = RS_Math::eval(c, &ok);
            if (ok) {
                angle1 = RS_Math::deg2rad(a);
                setStatus(SetAngle2);
            } else {
                if (RS_DIALOGFACTORY) {
                    RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
                }
            }
        }
        break;

    case SetAngle2: {
            bool ok;
            double a = RS_Math::eval(c, &ok);
            if (ok) {
                angle2 = RS_Math::deg2rad(a);
                trigger();
            } else {
                if (RS_DIALOGFACTORY) {
                    RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
                }
            }
        }
        break;

    default:
        break;
    }
}
*/


QStringList RS_ActionDrawEllipseCenter3Points::getAvailableCommands() {
    QStringList cmd;
    return cmd;
}



void RS_ActionDrawEllipseCenter3Points::updateMouseButtonHints() {
    if (RS_DIALOGFACTORY) {
        switch (getStatus()) {
        case SetCenter:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify the center of ellipse"),
                                                tr("Cancel"));
            break;

        case SetPoint1:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify the first point on ellipse"),
                                                tr("Cancel"));
            break;
        case SetPoint2:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify the second point on ellipse"),
                                                tr("Back"));
            break;

        case SetPoint3:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify the third point on ellipse"),
                                                tr("Back"));
            break;

        default:
            RS_DIALOGFACTORY->updateMouseWidget();
            break;
        }
    }
}

void RS_ActionDrawEllipseCenter3Points::updateMouseCursor() {
    graphicView->setMouseCursor(RS2::CadCursor);
}

// EOF
