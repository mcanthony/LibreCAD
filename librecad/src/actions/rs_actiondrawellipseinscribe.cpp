/****************************************************************************
**
 * Draw ellipse by foci and a point on ellipse

Copyright (C) 2011 Dongxu Li (dongxuli2011@gmail.com)
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
#include "rs_actiondrawellipseinscribe.h"

#include "rs_dialogfactory.h"
#include "rs_graphicview.h"
#include "rs_commandevent.h"
#include "rs_ellipse.h"
#include "rs_line.h"

/**
 * Constructor.
 *
 */
RS_ActionDrawEllipseInscribe::RS_ActionDrawEllipseInscribe(
    RS_EntityContainer& container,
    RS_GraphicView& graphicView)
        :RS_PreviewActionInterface("Draw ellipse inscribed",
						   container, graphicView)
		,eData(new RS_EllipseData{})
		,valid(false)
{
	actionType=RS2::ActionDrawEllipseInscribe;
}

RS_ActionDrawEllipseInscribe::~RS_ActionDrawEllipseInscribe(){}

void RS_ActionDrawEllipseInscribe::clearLines(bool checkStatus)
{
	while(lines.size() ){
		if(checkStatus && (int) lines.size()<=getStatus() )
			break;
		lines.back()->setHighlighted(false);
		graphicView->drawEntity(lines.back());
		lines.pop_back();
	}
}

void RS_ActionDrawEllipseInscribe::init(int status) {
    RS_PreviewActionInterface::init(status);
    if(status>=0) {
        RS_Snapper::suspend();
    }
	clearLines(true);
}

void RS_ActionDrawEllipseInscribe::finish(bool updateTB){
	clearLines(false);
    RS_PreviewActionInterface::finish(updateTB);
}


void RS_ActionDrawEllipseInscribe::trigger() {
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

	for(RS_Line*const p: lines) {
		if(!p) continue;
		p->setHighlighted(false);
		graphicView->drawEntity(p);

	}
    drawSnapper();

	clearLines(false);
	setStatus(SetLine1);

    RS_DEBUG->print("RS_ActionDrawEllipse4Line::trigger():"
                    " entity added: %d", ellipse->getId());
}



void RS_ActionDrawEllipseInscribe::mouseMoveEvent(QMouseEvent* e) {
    RS_DEBUG->print("RS_ActionDrawEllipse4Line::mouseMoveEvent begin");

    if(getStatus() == SetLine4) {
        RS_Entity*  en = catchEntity(e, RS2::EntityLine, RS2::ResolveAll);
        if(!en) return;
        if(!(en->isVisible() && en->rtti()== RS2::EntityLine)) return;
        for(auto p: lines){
            if(en == p) return; //do not pull in the same line again
        }

        if(en->getParent() && en->getParent()->ignoredOnModification()){
                return;
            }

		deletePreview();

		clearLines(true);
		lines.push_back(static_cast<RS_Line*>(en));
		if(preparePreview()) {
			lines.back()->setHighlighted(true);
			graphicView->drawEntity(lines.back());
			RS_Ellipse* e=new RS_Ellipse(preview.get(), *eData);
            preview->addEntity(e);
            drawPreview();
        }

    }
    RS_DEBUG->print("RS_ActionDrawEllipse4Line::mouseMoveEvent end");
}


bool RS_ActionDrawEllipseInscribe::preparePreview(){
    valid=false;
    if(getStatus() == SetLine4) {
		RS_Ellipse e{preview.get(), RS_EllipseData()};
        valid= e.createInscribeQuadrilateral(lines);
        if(valid){
			eData.reset(new RS_EllipseData(e.getData()));
        }else if( RS_DIALOGFACTORY){
            RS_DIALOGFACTORY->commandMessage(tr("Can not determine uniquely an ellipse"));
        }
    }
    return valid;
}

void RS_ActionDrawEllipseInscribe::mouseReleaseEvent(QMouseEvent* e) {
    // Proceed to next status
    if (e->button()==Qt::LeftButton) {
        if (!e) return;
        RS_Entity*  en = catchEntity(e, RS2::EntityLine, RS2::ResolveAll);
        if(!en) return;
        if(!(en->isVisible() && en->rtti()== RS2::EntityLine)) return;
        for(int i=0;i<getStatus();i++) {
            if(en->getId() == lines[i]->getId()) return; //do not pull in the same line again
        }
        if(en->getParent()) {
			if ( en->getParent()->ignoredOnModification()) return;
        }
		clearLines(true);
		lines.push_back(static_cast<RS_Line*>(en));

        switch (getStatus()) {
        case SetLine1:
        case SetLine2:
        case SetLine3:
            en->setHighlighted(true);
			graphicView->drawEntity(en);
			setStatus(getStatus()+1);
            break;
        case SetLine4:
            if( preparePreview()) {
                trigger();
            }

        default:
            break;
        }
    } else if (e->button()==Qt::RightButton) {
        // Return to last status:
        if(getStatus()>0){
			clearLines(true);
			lines.back()->setHighlighted(false);
			graphicView->drawEntity(lines.back());
            lines.pop_back();
            deletePreview();
        }
        init(getStatus()-1);
    }
}


//void RS_ActionDrawEllipseInscribe::coordinateEvent(RS_CoordinateEvent* e) {

//}

//fixme, support command line

/*
void RS_ActionDrawEllipse4Line::commandEvent(RS_CommandEvent* e) {
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


QStringList RS_ActionDrawEllipseInscribe::getAvailableCommands() {
    QStringList cmd;
    return cmd;
}



void RS_ActionDrawEllipseInscribe::updateMouseButtonHints() {
    if (RS_DIALOGFACTORY) {
        switch (getStatus()) {
        case SetLine1:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify the first line"),
                                                tr("Cancel"));
            break;

        case SetLine2:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify the second line"),
                                                tr("Back"));
            break;

        case SetLine3:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify the third line"),
                                                tr("Back"));
            break;

        case SetLine4:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify the fourth line"),
                                                tr("Back"));
            break;

        default:
            RS_DIALOGFACTORY->updateMouseWidget();
            break;
        }
    }
}

void RS_ActionDrawEllipseInscribe::updateMouseCursor() {
    graphicView->setMouseCursor(RS2::CadCursor);
}

// EOF
