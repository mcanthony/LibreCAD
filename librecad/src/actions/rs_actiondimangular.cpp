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
#include "rs_actiondimangular.h"
#include "rs_dimangular.h"

#include "rs_dialogfactory.h"
#include "rs_graphicview.h"
#include "rs_commandevent.h"
#include "rs_information.h"
#include "rs_line.h"
#include "rs_coordinateevent.h"

RS_ActionDimAngular::RS_ActionDimAngular(
    RS_EntityContainer& container,
    RS_GraphicView& graphicView)
        :RS_ActionDimension("Draw Angular Dimensions",
                    container, graphicView) {
	actionType= RS2::ActionDimAngular;
    reset();
}


RS_ActionDimAngular::~RS_ActionDimAngular(){}

void RS_ActionDimAngular::reset() {
    RS_ActionDimension::reset();

	edata.reset(new RS_DimAngularData(RS_Vector{false},
									  RS_Vector{false},
									  RS_Vector{false},
									  RS_Vector{false})
				);
	line1 = nullptr;
	line2 = nullptr;
	center = RS_Vector{}; //default to invalid vector
    RS_DIALOGFACTORY->requestOptions(this, true, true);
}



void RS_ActionDimAngular::trigger() {
    RS_PreviewActionInterface::trigger();

    if (line1 && line2) {
		RS_DimAngular* newEntity = new RS_DimAngular(container,
									  *data,
									  *edata);

        newEntity->setLayerToActive();
        newEntity->setPenToActive();
        newEntity->update();
        container->addEntity(newEntity);

        // upd. undo list:
        if (document) {
            document->startUndoCycle();
            document->addUndoable(newEntity);
            document->endUndoCycle();
        }
        RS_Vector rz = graphicView->getRelativeZero();
		setStatus(SetLine1);
		graphicView->redraw(RS2::RedrawDrawing);
        graphicView->moveRelativeZero(rz);
		RS_Snapper::finish();

    } else {
        RS_DEBUG->print("RS_ActionDimAngular::trigger:"
						" Entity is nullptr\n");
    }
}



void RS_ActionDimAngular::mouseMoveEvent(QMouseEvent* e) {
    RS_DEBUG->print("RS_ActionDimAngular::mouseMoveEvent begin");

	switch (getStatus()) {

    case SetPos:
        if (line1 && line2 && center.valid) {
			edata->definitionPoint4 = snapPoint(e);

			RS_DimAngular* d = new RS_DimAngular(preview.get(), *data, *edata);

            deletePreview();
            preview->addEntity(d);
            d->update();
            drawPreview();
        }
        break;

    default:
        break;
    }

    RS_DEBUG->print("RS_ActionDimAngular::mouseMoveEvent end");
}

void RS_ActionDimAngular::mouseReleaseEvent(QMouseEvent* e) {

    if (e->button()==Qt::LeftButton) {
        switch (getStatus()) {
        case SetLine1: {
                RS_Entity* en = catchEntity(e, RS2::ResolveAll);
                if (en &&
                        en->rtti()==RS2::EntityLine) {
                    line1 = (RS_Line*)en;
                    setStatus(SetLine2);
                }
            }
            break;

        case SetLine2: {
                RS_Entity* en = catchEntity(e, RS2::ResolveAll);
                if (en &&
                        en->rtti()==RS2::EntityLine) {
                    line2 = (RS_Line*)en;

                    RS_VectorSolutions sol =
                        RS_Information::getIntersectionLineLine(line1, line2);

                    if (sol.get(0).valid) {
                        center = sol.get(0);

                        if (center.distanceTo(line1->getStartpoint()) <
                                center.distanceTo(line1->getEndpoint())) {
							edata->definitionPoint1 = line1->getStartpoint();
							edata->definitionPoint2 = line1->getEndpoint();
                        } else {
							edata->definitionPoint1 = line1->getEndpoint();
							edata->definitionPoint2 = line1->getStartpoint();
                        }

                        if (center.distanceTo(line2->getStartpoint()) <
                                center.distanceTo(line2->getEndpoint())) {
							edata->definitionPoint3 = line2->getStartpoint();
							data->definitionPoint = line2->getEndpoint();
                        } else {
							edata->definitionPoint3 = line2->getEndpoint();
							data->definitionPoint = line2->getStartpoint();
                        }
                        graphicView->moveRelativeZero(center);
                        setStatus(SetPos);
                    }
                }
            }
            break;

        case SetPos: {
                RS_CoordinateEvent ce(snapPoint(e));
                coordinateEvent(&ce);
            }
            break;
        }
    } else if (e->button()==Qt::RightButton) {
        deletePreview();
        init(getStatus()-1);
    }

}



void RS_ActionDimAngular::coordinateEvent(RS_CoordinateEvent* e) {
	if (!e) {
        return;
    }

    switch (getStatus()) {
    case SetPos:
		edata->definitionPoint4 = e->getCoordinate();
        trigger();
        reset();
        setStatus(SetLine1);
        break;

    default:
        break;
    }
}


void RS_ActionDimAngular::commandEvent(RS_CommandEvent* e) {
    QString c = e->getCommand().toLower();

    if (checkCommand("help", c)) {
        RS_DIALOGFACTORY->commandMessage(msgAvailableCommands()
                                         + getAvailableCommands().join(", "));
        return;
    }

    // setting new text label:
    if (getStatus()==SetText) {
        setText(c);
        RS_DIALOGFACTORY->requestOptions(this, true, true);
        graphicView->enableCoordinateInput();
        setStatus(lastStatus);
        return;
    }

    // command: text
    if (checkCommand("text", c)) {
        lastStatus = (Status)getStatus();
        graphicView->disableCoordinateInput();
        setStatus(SetText);
    }
}



QStringList RS_ActionDimAngular::getAvailableCommands() {
    QStringList cmd;

    switch (getStatus()) {
    case SetLine1:
    case SetLine2:
    case SetPos:
        cmd += command("text");
        break;

    default:
        break;
    }

    return cmd;
}



void RS_ActionDimAngular::showOptions() {
    RS_ActionInterface::showOptions();

    RS_DIALOGFACTORY->requestOptions(this, true);
}



void RS_ActionDimAngular::hideOptions() {
    RS_ActionInterface::hideOptions();

    RS_DIALOGFACTORY->requestOptions(this, false);
}



void RS_ActionDimAngular::updateMouseButtonHints() {
    switch (getStatus()) {
    case SetLine1:
        RS_DIALOGFACTORY->updateMouseWidget(tr("Select first line"),
                                            tr("Cancel"));
        break;
    case SetLine2:
        RS_DIALOGFACTORY->updateMouseWidget(tr("Select second line"),
                                            tr("Cancel"));
        break;
    case SetPos:
        RS_DIALOGFACTORY->updateMouseWidget(
            tr("Specify dimension arc line location"), tr("Cancel"));
        break;
    case SetText:
        RS_DIALOGFACTORY->updateMouseWidget(tr("Enter dimension text:"), "");
        break;
    default:
        RS_DIALOGFACTORY->updateMouseWidget();
        break;
    }
}

// EOF
