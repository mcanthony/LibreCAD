/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/

#include <memory>
#include <QAction>
#include <QImage>
#include <QMouseEvent>
#include "rs_image.h"
#include "rs_line.h"
#include "rs_units.h"

#include "rs_dialogfactory.h"
#include "rs_graphicview.h"
#include "rs_commandevent.h"
#include "rs_creation.h"
#include "rs_actiondrawimage.h"
#include "rs_coordinateevent.h"
#include "rs_math.h"

/**
 * Constructor.
 */
RS_ActionDrawImage::RS_ActionDrawImage(RS_EntityContainer& container,
                                       RS_GraphicView& graphicView)
    :RS_PreviewActionInterface("Image",
							   container, graphicView)
	,lastStatus(ShowDialog)
{
	actionType=RS2::ActionDrawImage;
}

RS_ActionDrawImage::~RS_ActionDrawImage() {}


void RS_ActionDrawImage::init(int status) {
    RS_PreviewActionInterface::init(status);

    reset();

	data->file = RS_DIALOGFACTORY->requestImageOpenDialog();
    // RVT_PORT should we really redarw here?? graphicView->redraw();

	if (!data->file.isEmpty()) {
		//std::cout << "file: " << data->file << "\n";
		//qDebug() << "file: " << data->file;

		img.reset(new QImage(data->file));

        setStatus(SetTargetPoint);
    } else {
        setFinished();
        //RS_DIALOGFACTORY->requestToolBar(RS2::ToolBarMain);
    }
}



void RS_ActionDrawImage::reset() {
	data.reset(new RS_ImageData(
				   0,
				   RS_Vector(0.0,0.0),
				   RS_Vector(1.0,0.0),
				   RS_Vector(0.0,1.0),
				   RS_Vector(1.0,1.0),
				   "",
				   50, 50, 0)
			   );
}



void RS_ActionDrawImage::trigger() {
    deletePreview();

	if (!data->file.isEmpty()) {
        RS_Creation creation(container, graphicView);
		creation.createImage(data.get());
    }

    graphicView->redraw(RS2::RedrawDrawing);
    finish(false);
}


void RS_ActionDrawImage::mouseMoveEvent(QMouseEvent* e) {
	if (getStatus() == SetTargetPoint) {
		data->insertionPoint = snapPoint(e);

        deletePreview();
        //RS_Creation creation(preview, NULL, false);
        //creation.createInsert(data);
		double const w=img->width();
		double const h=img->height();
		RS_Line* line = new RS_Line{preview.get(), {0., 0.}, {w, 0.}};
        preview->addEntity(line);
		line = new RS_Line{preview.get(), {w, 0.}, {w, h}};
        preview->addEntity(line);
		line = new RS_Line{preview.get(), {w, h}, {0., h}};
        preview->addEntity(line);
		line = new RS_Line{preview.get(), {0., h}, {0., 0.}};
        preview->addEntity(line);
		preview->scale({0., 0.},
			{data->uVector.magnitude(), data->uVector.magnitude()});
		preview->rotate({0.,0.}, data->uVector.angle());
		preview->move(data->insertionPoint);
		drawPreview();
    }
}



void RS_ActionDrawImage::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button()==Qt::LeftButton) {
        RS_CoordinateEvent ce(snapPoint(e));
        coordinateEvent(&ce);
    } else if (e->button()==Qt::RightButton) {
        //init(getStatus()-1);
        finish(false);
    }
}



void RS_ActionDrawImage::coordinateEvent(RS_CoordinateEvent* e) {
    if (e==NULL) {
        return;
    }

	data->insertionPoint = e->getCoordinate();
    trigger();
}



void RS_ActionDrawImage::commandEvent(RS_CommandEvent* e) {
    QString c = e->getCommand().toLower();

    if (checkCommand("help", c)) {
        RS_DIALOGFACTORY->commandMessage(msgAvailableCommands()
                                         + getAvailableCommands().join(", "));
        return;
    }

    switch (getStatus()) {
    case SetTargetPoint:
        if (checkCommand("angle", c)) {
            deletePreview();
            lastStatus = (Status)getStatus();
            setStatus(SetAngle);
        } else if (checkCommand("factor", c)) {
            deletePreview();
            lastStatus = (Status)getStatus();
            setStatus(SetFactor);
        } else if (checkCommand("dpi",c)) {
            deletePreview();
            lastStatus =(Status)getStatus();
            setStatus(SetDPI);
        }
        break;

    case SetAngle: {
        bool ok;
        double a = RS_Math::eval(c, &ok);
		if (ok) {
            setAngle(RS_Math::deg2rad(a));
        } else {
            RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
        }
        RS_DIALOGFACTORY->requestOptions(this, true, true);
        setStatus(lastStatus);
    }
        break;

    case SetFactor: {
        bool ok;
        double f = RS_Math::eval(c, &ok);
		if (ok) {
            setFactor(f);
        } else {
            RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
        }
        RS_DIALOGFACTORY->requestOptions(this, true, true);
        setStatus(lastStatus);
    }
        break;

    case SetDPI : {
        bool ok;
        double dpi = RS_Math::eval(c, &ok);

		if(ok) {
            setFactor(RS_Units::dpiToScale(dpi, document->getGraphicUnit()));
        } else {
            RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
        }
        RS_DIALOGFACTORY->requestOptions(this, true, true);
        setStatus(lastStatus);
    }
        break;

    default:
        break;
    }
}


double RS_ActionDrawImage::getAngle() const {
	return data->uVector.angle();
}

void RS_ActionDrawImage::setAngle(double a) const{
	double l = data->uVector.magnitude();
	data->uVector.setPolar(l, a);
	data->vVector.setPolar(l, a+M_PI_2);
}

double RS_ActionDrawImage::getFactor() const {
	return data->uVector.magnitude();
}

void RS_ActionDrawImage::setFactor(double f) const {
	double a = data->uVector.angle();
	data->uVector.setPolar(f, a);
	data->vVector.setPolar(f, a+M_PI_2);
}

double RS_ActionDrawImage::dpiToScale(double dpi) const {
	return RS_Units::dpiToScale(dpi, document->getGraphicUnit());
}



double RS_ActionDrawImage::scaleToDpi(double scale) const {
	return RS_Units::scaleToDpi(scale, document->getGraphicUnit());
}



void RS_ActionDrawImage::updateMouseCursor() {
	graphicView->setMouseCursor(RS2::CadCursor);
}

QStringList RS_ActionDrawImage::getAvailableCommands() {
    QStringList cmd;

    switch (getStatus()) {
    case SetTargetPoint:
        cmd += command("angle");
        cmd += command("factor");
        cmd += command("dpi");
        break;
    default:
        break;
    }

    return cmd;
}



void RS_ActionDrawImage::showOptions() {
    RS_ActionInterface::showOptions();

    if(RS_DIALOGFACTORY){
        RS_DIALOGFACTORY->requestOptions(this, true);
    }
}



void RS_ActionDrawImage::hideOptions() {
    RS_ActionInterface::hideOptions();
    if(RS_DIALOGFACTORY){
        RS_DIALOGFACTORY->requestOptions(this, false);
    }
}


void RS_ActionDrawImage::updateMouseButtonHints() {
    if(RS_DIALOGFACTORY==NULL) return;
    switch (getStatus()) {
    case SetTargetPoint:
        RS_DIALOGFACTORY->updateMouseWidget(tr("Specify reference point"),
                                            tr("Cancel"));
        break;
    case SetAngle:
        RS_DIALOGFACTORY->updateMouseWidget(tr("Enter angle:"),
                                            "");
        break;
    case SetFactor:
        RS_DIALOGFACTORY->updateMouseWidget(tr("Enter factor:"),
                                            "");
        break;
    case SetDPI:
        RS_DIALOGFACTORY->updateMouseWidget(tr("Enter dpi:"),
                                            "");
        break;
    default:
		RS_DIALOGFACTORY->updateMouseWidget();
        break;
    }
}




// EOF
