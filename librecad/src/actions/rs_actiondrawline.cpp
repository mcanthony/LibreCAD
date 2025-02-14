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

#include<cmath>
#include <QAction>
#include <QMouseEvent>
#include "rs_actiondrawline.h"

#include "rs_actioneditundo.h"
#include "rs_dialogfactory.h"
#include "rs_graphicview.h"
#include "rs_commands.h"
#include "rs_commandevent.h"
#include "rs_line.h"
#include "rs_coordinateevent.h"

RS_ActionDrawLine::RS_ActionDrawLine(RS_EntityContainer& container,
                                     RS_GraphicView& graphicView)
    :RS_PreviewActionInterface("Draw lines",
							   container, graphicView)
	,data(new RS_LineData())
{

    RS_DEBUG->print("RS_ActionDrawLine::RS_ActionDrawLine");
	actionType=RS2::ActionDrawLine;
    reset();
    RS_DEBUG->print("RS_ActionDrawLine::RS_ActionDrawLine: OK");
}

RS_ActionDrawLine::~RS_ActionDrawLine(){}

void RS_ActionDrawLine::reset() {
	RS_DEBUG->print("RS_ActionDrawLine::reset");
	data.reset(new RS_LineData());
    start = RS_Vector(false);
    history.clear();
    historyIndex=-1;
    RS_DEBUG->print("RS_ActionDrawLine::reset: OK");
}

void RS_ActionDrawLine::init(int status) {
    RS_DEBUG->print("RS_ActionDrawLine::init");
    RS_PreviewActionInterface::init(status);

    reset();
    drawSnapper();
    RS_DEBUG->print("RS_ActionDrawLine::init: OK");
}

void RS_ActionDrawLine::trigger() {
    RS_PreviewActionInterface::trigger();

	RS_Line* line = new RS_Line(container, *data);
    line->setLayerToActive();
    line->setPenToActive();
    container->addEntity(line);

    // upd. undo list:
    if (document) {
        document->startUndoCycle();
        document->addUndoable(line);
        document->endUndoCycle();
    }

    graphicView->redraw(RS2::RedrawDrawing);
    graphicView->moveRelativeZero(history.at(historyIndex));
    //    graphicView->moveRelativeZero(line->getEndpoint());
    RS_DEBUG->print("RS_ActionDrawLine::trigger(): line added: %d",
                    line->getId());
}

RS_Vector RS_ActionDrawLine::snapToAngle(const RS_Vector &currentCoord)
{
    if(getStatus() != SetEndpoint)
    {
        RS_DEBUG->print(RS_Debug::D_WARNING, "Trying to snap to angle when not setting EndPoint!");
        return currentCoord;
    }
    if(snapMode.restriction != RS2::RestrictNothing ||
            snapMode.snapGrid) {
        return currentCoord;
    }
	double angle = data->startpoint.angleTo(currentCoord)*180.0/M_PI;
    /*Snapping to angle(15*) if shift key is pressed*/
    const double angularResolution=15.;
    angle -= remainder(angle,angularResolution);
    angle *= M_PI/180.;
	RS_Vector res = RS_Vector::polar(data->startpoint.distanceTo(currentCoord),
                 angle);
	res += data->startpoint;
    snapPoint(res, true);
    return res;
}



void RS_ActionDrawLine::mouseMoveEvent(QMouseEvent* e) {
    //    RS_DEBUG->print("RS_ActionDrawLine::mouseMoveEvent begin");

    //    RS_DEBUG->print("RS_ActionDrawLine::mouseMoveEvent: snap point");
    RS_Vector mouse = snapPoint(e);
    //    RS_DEBUG->print("RS_ActionDrawLine::mouseMoveEvent: snap point: OK");
	if (getStatus()==SetEndpoint && data->startpoint.valid) {
        RS_DEBUG->print("RS_ActionDrawLine::mouseMoveEvent: update preview");

        /*Snapping to angle(15*) if shift key is pressed*/
        if(e->modifiers() & Qt::ShiftModifier)
            mouse = snapToAngle(mouse);

        deletePreview();
		preview->addEntity(new RS_Line{preview.get(), data->startpoint, mouse});
        RS_DEBUG->print("RS_ActionDrawLine::mouseMoveEvent: draw preview");
        drawPreview();
    }

    //    RS_DEBUG->print("RS_ActionDrawLine::mouseMoveEvent end");
}

void RS_ActionDrawLine::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button()==Qt::LeftButton) {
        RS_Vector snapped = snapPoint(e);

        /*Snapping to angle(15*) if shift key is pressed*/
        if((e->modifiers() & Qt::ShiftModifier) && getStatus() == SetEndpoint )
            snapped = snapToAngle(snapped);
        RS_CoordinateEvent ce(snapped);
        coordinateEvent(&ce);
    } else if (e->button()==Qt::RightButton) {
        deletePreview();
        init(getStatus()-1);
    }
}

void RS_ActionDrawLine::coordinateEvent(RS_CoordinateEvent* e) {
    RS_DEBUG->print("RS_ActionDrawLine::coordinateEvent");
    if (e==NULL) {
        RS_DEBUG->print("RS_ActionDrawLine::coordinateEvent: event was NULL");
        return;
    }

    RS_Vector mouse = e->getCoordinate();
	if(data->startpoint.valid == false && getStatus()==SetEndpoint) setStatus(SetStartpoint);
    switch (getStatus()) {
    case SetStartpoint:
		data->startpoint = mouse;
        addHistory(mouse);

		start = data->startpoint;
        setStatus(SetEndpoint);
        graphicView->moveRelativeZero(mouse);
        updateMouseButtonHints();
        break;

    case SetEndpoint:
		if((mouse-data->startpoint).squared() > RS_TOLERANCE2) {
            //refuse zero length lines
			data->endpoint = mouse;
            addHistory(mouse);
            trigger();
			data->startpoint = data->endpoint;
            if(history.size()>=2) updateMouseButtonHints();
        }
        //graphicView->moveRelativeZero(mouse);
        break;

    default:
        break;
    }
    RS_DEBUG->print("RS_ActionDrawLine::coordinateEvent: OK");
}

void RS_ActionDrawLine::commandEvent(RS_CommandEvent* e) {
    RS_DEBUG->print("RS_ActionDrawLine::commandEvent");
    QString c = e->getCommand().toLower();

    switch (getStatus()) {
    case SetStartpoint:
        if (checkCommand("help", c)) {
            RS_DIALOGFACTORY->commandMessage(msgAvailableCommands()
                                             + getAvailableCommands().join(", "));
			e->accept();
			return;
		}
		break;

    case SetEndpoint:
        if (checkCommand("close", c)) {
            close();
			e->accept();
			updateMouseButtonHints();
			return;
        }

        if (checkCommand("undo", c)) {
            undo();
			e->accept();
			updateMouseButtonHints();
			return;
        }
		break;

    default:
		return;
    }
    if (checkCommand("redo", c)) {
        redo();
		e->accept();
		updateMouseButtonHints();
	}
	//    RS_DEBUG->print("RS_ActionDrawLine::commandEvent: OK");
}

QStringList RS_ActionDrawLine::getAvailableCommands() {
    QStringList cmd;
	if(historyIndex+1<(int) history.size()) {
        cmd += command("redo");
    }

    switch (getStatus()) {
    case SetStartpoint:
        break;
    case SetEndpoint:
        if (historyIndex>=1) {
            cmd += command("undo");
        }
        if (historyIndex>=2) {
            cmd += command("close");
        }
        break;
    default:
        break;
    }

    return cmd;
}

void RS_ActionDrawLine::updateMouseButtonHints() {
    if(RS_DIALOGFACTORY != NULL){
        switch (getStatus()) {
        case SetStartpoint:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify first point"),
                                                tr("Cancel"));
            break;
        case SetEndpoint: {
            QString msg = "";

            if (historyIndex>=2) {
                msg += RS_COMMANDS->command("close");
            }
			if(historyIndex+1<(int) history.size()) {
                if(msg.size()>0)  msg += "/";
                msg += RS_COMMANDS->command("redo");
            }
            if (historyIndex>=1) {
                if(msg.size()>0)  msg += "/";
                msg += RS_COMMANDS->command("undo");
            }

            if (historyIndex>=1) {
                RS_DIALOGFACTORY->updateMouseWidget(
                            tr("Specify next point or [%1]").arg(msg),
                            tr("Back"));
            } else {
                RS_DIALOGFACTORY->updateMouseWidget(
                            tr("Specify next point"),
                            tr("Back"));
            }
        }
            break;
        default:
            RS_DIALOGFACTORY->updateMouseWidget();
            break;
        }
    }
}

void RS_ActionDrawLine::showOptions() {
    RS_DEBUG->print("RS_ActionDrawLine::showOptions");
    if(RS_DIALOGFACTORY != NULL){
        RS_ActionInterface::showOptions();

        RS_DIALOGFACTORY->requestOptions(this, true);
    }
    RS_DEBUG->print("RS_ActionDrawLine::showOptions: OK");
}

void RS_ActionDrawLine::hideOptions() {
    if(RS_DIALOGFACTORY != NULL){
        RS_ActionInterface::hideOptions();

        RS_DIALOGFACTORY->requestOptions(this, false);
    }
}


void RS_ActionDrawLine::updateMouseCursor() {
    graphicView->setMouseCursor(RS2::CadCursor);
}

void RS_ActionDrawLine::close() {
	if (historyIndex>2 && start.valid && (data->startpoint - start).squared() > RS_TOLERANCE2 ) {
		data->endpoint = start;
		addHistory(data->endpoint);
        trigger();
        setStatus(SetStartpoint);
        //        graphicView->moveRelativeZero(start);
    } else {
        if (RS_DIALOGFACTORY) {
            RS_DIALOGFACTORY->commandMessage(
                        tr("Cannot close sequence of lines: "
                           "Not enough entities defined yet, or already closed."));
        }
    }
}

void RS_ActionDrawLine::addHistory(const RS_Vector& v){
    if(historyIndex<-1) historyIndex=-1;
    history.erase(history.begin()+historyIndex+1,history.end());
	history.push_back(v);
    historyIndex=history.size() - 1;
}
void RS_ActionDrawLine::undo() {
    if (historyIndex>0) {
        historyIndex--;
        //        history.removeLast();
        deletePreview();
        graphicView->setCurrentAction(
                    new RS_ActionEditUndo(true, *container, *graphicView));
		data->startpoint = history.at(historyIndex);
		graphicView->moveRelativeZero(data->startpoint);
    } else {
        RS_DIALOGFACTORY->commandMessage(
                    tr("Cannot undo: "
                       "Not enough entities defined yet."));
    }
    if(historyIndex>=1) {
        setStatus(SetEndpoint);
    }else{
        setStatus(SetStartpoint);
    }
}

void RS_ActionDrawLine::redo() {
	if ((int) history.size()>historyIndex+1) {
        historyIndex++;
        //        history.removeLast();
        deletePreview();
        graphicView->setCurrentAction(
                    new RS_ActionEditUndo(false, *container, *graphicView));
		data->startpoint = history.at(historyIndex);
		graphicView->moveRelativeZero(data->startpoint);
    } else {
        RS_DIALOGFACTORY->commandMessage(
                    tr("Cannot redo: "
                       "Not previous line segment defined."));
    }
    setStatus(SetEndpoint);
}

// EOF
