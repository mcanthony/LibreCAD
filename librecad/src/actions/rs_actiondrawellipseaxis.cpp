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
#include "rs_actiondrawellipseaxis.h"

#include "rs_dialogfactory.h"
#include "rs_graphicview.h"
#include "rs_commandevent.h"
#include "rs_ellipse.h"
#include "rs_line.h"
#include "rs_coordinateevent.h"
#include "rs_math.h"

/**
 * Constructor.
 *
 * @param isArc true if this action will produce an ellipse arc.
 *              false if it will produce a full ellipse.
 */
RS_ActionDrawEllipseAxis::RS_ActionDrawEllipseAxis(
		RS_EntityContainer& container,
		RS_GraphicView& graphicView,
		bool isArc)
	:RS_PreviewActionInterface("Draw ellipse with axis",
							   container, graphicView)
	,center(false)
	,m_vMajorP(false)
	,ratio(0.5)
	,angle1(0.)
	,angle2(isArc?2.*M_PI:0.0)
	,isArc(isArc)
{
	actionType=isArc?RS2::ActionDrawEllipseArcAxis:RS2::ActionDrawEllipseAxis;
}


void RS_ActionDrawEllipseAxis::init(int status) {
    RS_PreviewActionInterface::init(status);

    if (status==SetCenter) {
        center = RS_Vector(false);
    }
    if (status<=SetMajor) {
		m_vMajorP = RS_Vector(false);
    }
    if (status<=SetMinor) {
        ratio = 0.5;
    }
    if (status<=SetAngle1) {
        angle1 = 0.0;
    }
    if (status<=SetAngle2) {
        angle2 = 0.0;
    }
}



void RS_ActionDrawEllipseAxis::trigger() {
    RS_PreviewActionInterface::trigger();

	RS_Ellipse* ellipse = new RS_Ellipse{container,
		center, m_vMajorP, ratio, angle1, angle2
	};
    if (ratio > 1.){
        ellipse->switchMajorMinor();
    }
    ellipse->setLayerToActive();
    ellipse->setPenToActive();

    container->addEntity(ellipse);

    // upd. undo list:
    if (document) {
        document->startUndoCycle();
        document->addUndoable(ellipse);
        document->endUndoCycle();
    }

    RS_Vector rz = graphicView->getRelativeZero();
    graphicView->redraw(RS2::RedrawDrawing);
    graphicView->moveRelativeZero(rz);
    drawSnapper();

    setStatus(SetCenter);

    RS_DEBUG->print("RS_ActionDrawEllipseAxis::trigger():"
                    " entity added: %d", ellipse->getId());
}



void RS_ActionDrawEllipseAxis::mouseMoveEvent(QMouseEvent* e) {
    RS_DEBUG->print("RS_ActionDrawEllipseAxis::mouseMoveEvent begin");

    RS_Vector mouse = snapPoint(e);

    switch (getStatus()) {
//    case SetCenter:
//        break;

    case SetMajor:
        if (center.valid) {
            deletePreview();
			preview->addEntity(new RS_Ellipse{preview.get(),
				center, mouse-center, 0.5, 0.0, isArc?2.*M_PI:0.
							   });
            drawPreview();
        }
        break;

    case SetMinor:
		if (center.valid && m_vMajorP.valid) {
            deletePreview();
			RS_Line line{center-m_vMajorP, center+m_vMajorP};
            double d = line.getDistanceToPoint(mouse);
            ratio = d/(line.getLength()/2);
			preview->addEntity(new RS_Ellipse{preview.get(),
				center, m_vMajorP, ratio, 0., isArc?2.*M_PI:0.
							   });
            drawPreview();
        }
        break;

    case SetAngle1:
		if (center.valid && m_vMajorP.valid) {
            deletePreview();

            //angle1 = center.angleTo(mouse);

                        RS_Vector m = mouse;
						m.rotate(center, -m_vMajorP.angle());
                        RS_Vector v = m-center;
						v.y /= ratio;
						angle1 = v.angle(); // + m_vMajorP.angle();

			preview->addEntity(new RS_Line{preview.get(), center, mouse});

			preview->addEntity(new RS_Ellipse{preview.get(),
				center, m_vMajorP, ratio, angle1, angle1+1.0
							   });
            drawPreview();
        }
        break;

    case SetAngle2:
		if (center.valid && m_vMajorP.valid) {
            deletePreview();
            //angle2 = center.angleTo(mouse);

                        RS_Vector m = mouse;
						m.rotate(center, -m_vMajorP.angle());
                        RS_Vector v = m-center;
						v.y /= ratio;
						angle2 = v.angle(); // + m_vMajorP.angle();

			preview->addEntity(new RS_Line{preview.get(), center, mouse});

			preview->addEntity(new RS_Ellipse{preview.get(),
											  center, m_vMajorP,
												  ratio,
												  angle1, angle2
							   });
            drawPreview();
        }

    default:
        break;
    }

    RS_DEBUG->print("RS_ActionDrawEllipseAxis::mouseMoveEvent end");
}



void RS_ActionDrawEllipseAxis::mouseReleaseEvent(QMouseEvent* e) {
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


void RS_ActionDrawEllipseAxis::coordinateEvent(RS_CoordinateEvent* e) {
	if (!e) return;
	RS_Vector const& mouse = e->getCoordinate();

    switch (getStatus()) {
    case SetCenter:
        center = mouse;
        graphicView->moveRelativeZero(mouse);
        setStatus(SetMajor);
        break;

    case SetMajor:
		m_vMajorP = mouse-center;
        setStatus(SetMinor);
        break;

    case SetMinor: {
			RS_Line line{center-m_vMajorP, center+m_vMajorP};
            double d = line.getDistanceToPoint(mouse);
            ratio = d/(line.getLength()/2);
            if (!isArc) {
                trigger();
                setStatus(SetCenter);
            } else {
                setStatus(SetAngle1);
            }
        }
        break;

    case SetAngle1: {
        //angle1 = center.angleTo(mouse);
                RS_Vector m = mouse;
				m.rotate(center, -m_vMajorP.angle());
                RS_Vector v = m-center;
				v.y /= ratio;
                angle1 = v.angle();
        setStatus(SetAngle2);
                } break;

    case SetAngle2: {
        //angle2 = center.angleTo(mouse);
                RS_Vector m = mouse;
				m.rotate(center, -m_vMajorP.angle());
                RS_Vector v = m-center;
				v.y /= ratio;
                angle2 = v.angle();
        trigger();
		}
		break;

    default:
        break;
    }
}



void RS_ActionDrawEllipseAxis::commandEvent(RS_CommandEvent* e) {
    QString c = e->getCommand().toLower();

    if (checkCommand("help", c)) {
        if (RS_DIALOGFACTORY) {
            RS_DIALOGFACTORY->commandMessage(msgAvailableCommands()
                                             + getAvailableCommands().join(", "));
        }
        return;
    }

    switch (getStatus()) {
    case SetMinor: {
            bool ok;
            double m = RS_Math::eval(c, &ok);
            if (ok) {
                e->accept();
				ratio = m / m_vMajorP.magnitude();
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
                e->accept();
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
                e->accept();
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



QStringList RS_ActionDrawEllipseAxis::getAvailableCommands() {
    QStringList cmd;
    return cmd;
}



void RS_ActionDrawEllipseAxis::updateMouseButtonHints() {
    if (RS_DIALOGFACTORY) {
        switch (getStatus()) {
        case SetCenter:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify ellipse center"),
                                                tr("Cancel"));
            break;

        case SetMajor:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify endpoint of major axis"),
                                                tr("Back"));
            break;

        case SetMinor:
            RS_DIALOGFACTORY->updateMouseWidget(
                tr("Specify endpoint or length of minor axis:"),
                tr("Back"));
            break;

        case SetAngle1:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify start angle"),
                                                tr("Back"));
            break;

        case SetAngle2:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify end angle"),
                                                tr("Back"));
            break;

        default:
			RS_DIALOGFACTORY->updateMouseWidget();
            break;
        }
    }
}



void RS_ActionDrawEllipseAxis::updateMouseCursor() {
    graphicView->setMouseCursor(RS2::CadCursor);
}

// EOF
