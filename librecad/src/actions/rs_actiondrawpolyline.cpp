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
#include "rs_actiondrawpolyline.h"

#include "rs_dialogfactory.h"
#include "rs_graphicview.h"
#include "rs_commands.h"
#include "rs_commandevent.h"
#include "rs_arc.h"
#include "rs_line.h"
#include "rs_polyline.h"
#include "rs_coordinateevent.h"
#include "rs_math.h"

#ifdef EMU_C99
#include "emu_c99.h"
#endif

RS_ActionDrawPolyline::RS_ActionDrawPolyline(RS_EntityContainer& container,
                                     RS_GraphicView& graphicView)
        :RS_PreviewActionInterface("Draw polylines",
						   container, graphicView)
		,m_Reversed(1)
{
	actionType=RS2::ActionDrawPolyline;
    reset();
}



RS_ActionDrawPolyline::~RS_ActionDrawPolyline() {}


void RS_ActionDrawPolyline::reset() {
		polyline = nullptr;
	data.reset(new RS_PolylineData(RS_Vector(false), RS_Vector(false), false));
    start = RS_Vector(false);
    history.clear();
    bHistory.clear();
}



void RS_ActionDrawPolyline::init(int status) {
    reset();
    RS_PreviewActionInterface::init(status);

}



void RS_ActionDrawPolyline::trigger() {
    RS_PreviewActionInterface::trigger();

	if (!polyline) return;

        // add the entity
    //RS_Polyline* polyline = new RS_Polyline(container, data);
    //polyline->setLayerToActive();
    //polyline->setPenToActive();
    //container->addEntity(polyline);

    // upd. undo list:
    if (document) {
        document->startUndoCycle();
        document->addUndoable(polyline);
        document->endUndoCycle();
    }

        // upd view
    deleteSnapper();
	graphicView->moveRelativeZero(RS_Vector{0.,0.});
    graphicView->drawEntity(polyline);
    graphicView->moveRelativeZero(polyline->getEndpoint());
    drawSnapper();
    RS_DEBUG->print("RS_ActionDrawLinePolyline::trigger(): polyline added: %d",
                    polyline->getId());

		polyline = nullptr;
}



void RS_ActionDrawPolyline::mouseMoveEvent(QMouseEvent* e) {
    RS_DEBUG->print("RS_ActionDrawLinePolyline::mouseMoveEvent begin");

    RS_Vector mouse = snapPoint(e);
    double bulge=solveBulge(mouse);
    if (getStatus()==SetNextPoint && point.valid) {
        deletePreview();
        // clearPreview();

                //RS_Polyline* p = polyline->clone();
                //p->reparent(preview);
                //preview->addEntity(p);
        if (fabs(bulge)<RS_TOLERANCE || Mode==Line) {
			preview->addEntity(new RS_Line{preview.get(), point, mouse});
        } else
			preview->addEntity(new RS_Arc(preview.get(), *arc_data));
        drawPreview();
    }

    RS_DEBUG->print("RS_ActionDrawLinePolyline::mouseMoveEvent end");
}



void RS_ActionDrawPolyline::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button()==Qt::LeftButton) {
        RS_CoordinateEvent ce(snapPoint(e));
        coordinateEvent(&ce);
    } else if (e->button()==Qt::RightButton) {
                if (getStatus()==SetNextPoint) {
                        trigger();
                }
        deletePreview();
        // clearPreview();
        deleteSnapper();
        init(getStatus()-1);
    }
}

double RS_ActionDrawPolyline::solveBulge(RS_Vector mouse) {

    double b(0.);
    bool suc;
	RS_Arc arc(nullptr, RS_ArcData());
	RS_Line line(nullptr,RS_LineData());
	double direction;
    RS_AtomicEntity* lastentity;
    calculatedSegment=false;

    switch (Mode){
//     case Line:
//        b=0.0;
//        break;
     case Tangential:
        if (polyline){
            lastentity = (RS_AtomicEntity*)polyline->lastEntity();
            direction = RS_Math::correctAngle(
                lastentity->getDirection2()+M_PI);
            line.setStartpoint(point);
            line.setEndpoint(mouse);
			double const direction2=RS_Math::correctAngle(line.getDirection2()+M_PI);
			double const delta=direction2-direction;
            if( fabs(remainder(delta,M_PI))>RS_TOLERANCE_ANGLE ) {
                b=tan(delta/2);
                suc = arc.createFrom2PBulge(point,mouse,b);
                if (suc)
					arc_data.reset(new RS_ArcData(arc.getData()));
                else
                    b=0;
            }
            break;
//            if(delta<RS_TOLERANCE_ANGLE ||
//                (delta<M_PI+RS_TOLERANCE_ANGLE &&
//                delta>M_PI-RS_TOLERANCE_ANGLE))
//                b=0;
//            else{
//                b=tan((direction2-direction)/2);
//                suc = arc.createFrom2PBulge(point,mouse,b);
//                if (suc)
//                    arc_data = arc.getData();
//                else
//                    b=0;
//            }
        }
//        else
//            b=0;
//        break;
     case TanRad:
        if (polyline){
            lastentity = (RS_AtomicEntity*)polyline->lastEntity();
            direction = RS_Math::correctAngle(
                lastentity->getDirection2()+M_PI);
            suc = arc.createFrom2PDirectionRadius(point, mouse,
                direction,Radius);
            if (suc){
				arc_data.reset(new RS_ArcData(arc.getData()));
                b=arc.getBulge();
                calculatedEndpoint = arc.getEndpoint();
                calculatedSegment=true;

            }
//            else
//                b=0;
        }
//        else
//          b=0;
        break;
/*     case TanAng:
        b=tan(Reversed*Angle*M_PI/720.0);
        break;
     case TanRadAng:
        b=tan(Reversed*Angle*M_PI/720.0);
        break;*/
    case Ang:
		b=tan(m_Reversed*Angle*M_PI/720.0);
        suc = arc.createFrom2PBulge(point,mouse,b);
        if (suc)
			arc_data.reset(new RS_ArcData(arc.getData()));
		else
            b=0;
        break;
    default:
        break;
        /*     case RadAngEndp:
        b=tan(Reversed*Angle*M_PI/720.0);
        break;
     case RadAngCenp:
        b=tan(Reversed*Angle*M_PI/720.0);*/
    }
    return b;
}

void RS_ActionDrawPolyline::coordinateEvent(RS_CoordinateEvent* e) {
	if (!e) {
        return;
    }

    RS_Vector mouse = e->getCoordinate();
    double bulge=solveBulge(mouse);
    if (calculatedSegment)
        mouse=calculatedEndpoint;

    switch (getStatus()) {
    case SetStartpoint:
        //	data.startpoint = mouse;
        //printf ("SetStartpoint\n");
        point = mouse;
        history.clear();
        history.append(mouse);
        bHistory.clear();
        bHistory.append(0.0);
        start = point;
        setStatus(SetNextPoint);
        graphicView->moveRelativeZero(mouse);
        updateMouseButtonHints();
        break;

    case SetNextPoint:
        graphicView->moveRelativeZero(mouse);
        point = mouse;
        history.append(mouse);
        bHistory.append(bulge);
				if (!polyline) {
						polyline = new RS_Polyline(container, *data);
                        polyline->addVertex(start, 0.0);
                }
                if (polyline) {
                        polyline->setNextBulge(bulge);
                        polyline->addVertex(mouse, 0.0);
                        polyline->setEndpoint(mouse);
                        if (polyline->count()==1) {
                        polyline->setLayerToActive();
                        polyline->setPenToActive();
                                container->addEntity(polyline);
                        }
                        deletePreview();
                        // clearPreview();
                        deleteSnapper();
                        graphicView->drawEntity(polyline);
                        drawSnapper();
                }
        //trigger();
        //data.startpoint = data.endpoint;
        updateMouseButtonHints();
        //graphicView->moveRelativeZero(mouse);
        break;

    default:
        break;
    }
}

void RS_ActionDrawPolyline::setMode(SegmentMode m) {
	Mode=m;
}

int RS_ActionDrawPolyline::getMode() const{
	return Mode;
}

void RS_ActionDrawPolyline::setRadius(double r) {
	Radius=r;
}

double RS_ActionDrawPolyline::getRadius() const{
	return Radius;
}

void RS_ActionDrawPolyline::setAngle(double a) {
	Angle=a;
}

double RS_ActionDrawPolyline::getAngle() const{
	return Angle;
}

void RS_ActionDrawPolyline::setReversed( bool c) {
	m_Reversed=c?-1:1;
}

bool RS_ActionDrawPolyline::isReversed() const{
	return m_Reversed==-1;
}


void RS_ActionDrawPolyline::commandEvent(RS_CommandEvent* e) {
    QString c = e->getCommand().toLower();

    switch (getStatus()) {
    case SetStartpoint:
        if (checkCommand("help", c)) {
            RS_DIALOGFACTORY->commandMessage(msgAvailableCommands()
                                             + getAvailableCommands().join(", "));
            return;
        }
        break;

    case SetNextPoint:
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
        break;
    }
}



QStringList RS_ActionDrawPolyline::getAvailableCommands() {
    QStringList cmd;

    switch (getStatus()) {
    case SetStartpoint:
        break;
    case SetNextPoint:
        if (history.size()>=2) {
            cmd += command("undo");
        }
        if (history.size()>=3) {
            cmd += command("close");
        }
        break;
    default:
        break;
    }

    return cmd;
}



void RS_ActionDrawPolyline::updateMouseButtonHints() {
    switch (getStatus()) {
    case SetStartpoint:
        RS_DIALOGFACTORY->updateMouseWidget(tr("Specify first point"),
                                            tr("Cancel"));
        break;
    case SetNextPoint: {
            QString msg = "";

            if (history.size()>=3) {
                msg += RS_COMMANDS->command("close");
                msg += "/";
            }
            if (history.size()>=2) {
                msg += RS_COMMANDS->command("undo");
            }

            if (history.size()>=2) {
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


void RS_ActionDrawPolyline::showOptions() {
    RS_ActionInterface::showOptions();

    RS_DIALOGFACTORY->requestOptions(this, true);
}



void RS_ActionDrawPolyline::hideOptions() {
    RS_ActionInterface::hideOptions();

    RS_DIALOGFACTORY->requestOptions(this, false);
}


void RS_ActionDrawPolyline::updateMouseCursor() {
    graphicView->setMouseCursor(RS2::CadCursor);
}

void RS_ActionDrawPolyline::close() {
    if (history.size()>2 && start.valid) {
        //data.endpoint = start;
        //trigger();
                if (polyline) {
                        if (Mode==TanRad)
                                Mode=Line;
                        RS_CoordinateEvent e(polyline->getStartpoint());
                        coordinateEvent(&e);
                }
        polyline->setClosed(true);
                trigger();
        setStatus(SetStartpoint);
        graphicView->moveRelativeZero(start);
    } else {
        RS_DIALOGFACTORY->commandMessage(
            tr("Cannot close sequence of lines: "
               "Not enough entities defined yet."));
    }
}

void RS_ActionDrawPolyline::undo() {
    if (history.size()>1) {
        history.removeLast();
        bHistory.removeLast();
        deletePreview();
        point = history.last();

        if(history.size()==1){
            graphicView->moveRelativeZero(history.at(0));
            //remove polyline from container,
            //container calls delete over polyline
            container->removeEntity(polyline);
			polyline = nullptr;
            graphicView->drawEntity(polyline);
        }
        if (polyline) {
            polyline->removeLastVertex();
            graphicView->moveRelativeZero(polyline->getEndpoint());
            graphicView->drawEntity(polyline);
        }
    } else {
        RS_DIALOGFACTORY->commandMessage(
            tr("Cannot undo: "
               "Not enough entities defined yet."));
    }
}

// EOF
