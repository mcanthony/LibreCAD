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

#include <QAction>
#include <QMouseEvent>
#include "rs_actiondrawlineangle.h"

#include "rs_dialogfactory.h"
#include "rs_graphicview.h"
#include "rs_commandevent.h"
#include "rs_settings.h"
#include "rs_line.h"
#include "rs_coordinateevent.h"
#include "rs_math.h"

RS_ActionDrawLineAngle::RS_ActionDrawLineAngle(RS_EntityContainer& container,
        RS_GraphicView& graphicView,
        double angle,
        bool fixedAngle, RS2::ActionType actionType)
        :RS_PreviewActionInterface("Draw lines with given angle",
						   container, graphicView)
		,data(new RS_LineData())
		,pos(false)
		,angle(angle)
		,length(1.)
		,fixedAngle(fixedAngle)
		,snpPoint(0)
{

    this->actionType=actionType;

    RS_DIALOGFACTORY->requestOptions(this, true,false);
    reset();
}



RS_ActionDrawLineAngle::~RS_ActionDrawLineAngle() {
    RS_SETTINGS->beginGroup("/Draw");
    if (!hasFixedAngle()) {
        RS_SETTINGS->writeEntry("/LineAngleAngle", RS_Math::rad2deg(getAngle()));
    }
    RS_SETTINGS->writeEntry("/LineAngleLength", getLength());
    RS_SETTINGS->writeEntry("/LineAngleSnapPoint", getSnapPoint());
    RS_SETTINGS->endGroup();
}


void RS_ActionDrawLineAngle::reset() {
	data.reset(new RS_LineData{});
}

void RS_ActionDrawLineAngle::init(int status) {
    RS_PreviewActionInterface::init(status);

    reset();
}

void RS_ActionDrawLineAngle::trigger() {
    RS_PreviewActionInterface::trigger();

    preparePreview();
	RS_Line* line = new RS_Line{container, *data};
    line->setLayerToActive();
    line->setPenToActive();
    container->addEntity(line);

    // upd. undo list:
    if (document) {
        document->startUndoCycle();
        document->addUndoable(line);
        document->endUndoCycle();
    }

	graphicView->moveRelativeZero(data->startpoint);
        graphicView->redraw(RS2::RedrawDrawing);
    RS_DEBUG->print("RS_ActionDrawLineAngle::trigger(): line added: %d",
                    line->getId());
}

void RS_ActionDrawLineAngle::mouseMoveEvent(QMouseEvent* e) {
    RS_DEBUG->print("RS_ActionDrawLineAngle::mouseMoveEvent begin");

    if (getStatus()==SetPos) {
        pos = snapPoint(e);
        deletePreview();
        preparePreview();
		preview->addEntity(new RS_Line(preview.get(),
									   *data));
        drawPreview();
    }

    RS_DEBUG->print("RS_ActionDrawLineAngle::mouseMoveEvent end");
}

void RS_ActionDrawLineAngle::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button()==Qt::LeftButton) {
        if (getStatus()==SetPos) {
            RS_CoordinateEvent ce(snapPoint(e));
            coordinateEvent(&ce);
        }
    } else if (e->button()==Qt::RightButton) {
        deletePreview();
        init(getStatus()-1);
    }
}


void RS_ActionDrawLineAngle::preparePreview() {
    RS_Vector p1, p2;
    // End:
    if (snpPoint == 2) {
        p2.setPolar(length * -1, angle);
    } else {
        p2.setPolar(length, angle);
    }

    // Middle:
    if (snpPoint == 1) {
        p1 = pos - (p2 / 2);
    } else {
        p1 = pos;
    }

    p2 += p1;
	data.reset(new RS_LineData{p1, p2});
}

void RS_ActionDrawLineAngle::coordinateEvent(RS_CoordinateEvent* e) {
	if (!e) return;

    switch (getStatus()) {
    case SetPos:
        pos = e->getCoordinate();
        trigger();
        break;

    default:
        break;
    }
}

void RS_ActionDrawLineAngle::commandEvent(RS_CommandEvent* e) {
    QString c = e->getCommand().toLower();

    if (checkCommand("help", c)) {
        RS_DIALOGFACTORY->commandMessage(msgAvailableCommands()
                                         + getAvailableCommands().join(", "));
        return;
    }

    switch (getStatus()) {
    case SetPos:
        if (!fixedAngle && checkCommand("angle", c)) {
            deletePreview();
            setStatus(SetAngle);
        } else if (checkCommand("length", c)) {
            deletePreview();
            setStatus(SetLength);
        }
        break;

    case SetAngle: {
            bool ok;
            double a = RS_Math::eval(c, &ok);
            if (ok) {
                e->accept();
                angle = RS_Math::deg2rad(a);
            } else {
                RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
            }
            RS_DIALOGFACTORY->requestOptions(this, true, true);
            setStatus(SetPos);
        }
        break;

    case SetLength: {
            bool ok;
            double l = RS_Math::eval(c, &ok);
            if (ok) {
                e->accept();
                length = l;
            } else {
                RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
            }
            RS_DIALOGFACTORY->requestOptions(this, true, true);
            setStatus(SetPos);
        }
        break;

    default:
        break;
    }
}

void RS_ActionDrawLineAngle::setSnapPoint(int sp) {
	snpPoint = sp;
}

int RS_ActionDrawLineAngle::getSnapPoint() const{
	return snpPoint;
}

void RS_ActionDrawLineAngle::setAngle(double a) {
	angle = a;
}

double RS_ActionDrawLineAngle::getAngle() const{
	return angle;
}

void RS_ActionDrawLineAngle::setLength(double l) {
	length = l;
}

double RS_ActionDrawLineAngle::getLength() const{
	return length;
}

bool RS_ActionDrawLineAngle::hasFixedAngle() const{
	switch(rtti()){
	case RS2::ActionDrawLineHorizontal:
	case RS2::ActionDrawLineVertical:
		return true;
	default:
		return false;
	}
}

QStringList RS_ActionDrawLineAngle::getAvailableCommands() {
    QStringList cmd;

    switch (getStatus()) {
    case SetPos:
        if (!fixedAngle) {
            cmd += command("angle");
        }
        cmd += command("length");
        break;
    default:
        break;
    }

    return cmd;
}


void RS_ActionDrawLineAngle::updateMouseButtonHints() {
    switch (getStatus()) {
    case SetPos:
        RS_DIALOGFACTORY->updateMouseWidget(tr("Specify position"),
                                            tr("Cancel"));
        break;

    case SetAngle:
        RS_DIALOGFACTORY->updateMouseWidget(tr("Enter angle:"), tr("Back"));
        break;

    case SetLength:
        RS_DIALOGFACTORY->updateMouseWidget(tr("Enter length:"), tr("Back"));
        break;

    default:
        break;
    }
}

void RS_ActionDrawLineAngle::showOptions() {
    RS_ActionInterface::showOptions();

    RS_DIALOGFACTORY->requestOptions(this, true,true);
}

void RS_ActionDrawLineAngle::hideOptions() {
    RS_ActionInterface::hideOptions();

    RS_DIALOGFACTORY->requestOptions(this, false);
}

void RS_ActionDrawLineAngle::updateMouseCursor() {
    graphicView->setMouseCursor(RS2::CadCursor);
}

// EOF
