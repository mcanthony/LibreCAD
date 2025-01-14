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

#ifndef RS_ACTIONDRAWARC3P_H
#define RS_ACTIONDRAWARC3P_H

#include "rs_previewactioninterface.h"

struct RS_ArcData;

/**
 * This action class can handle user events to draw 
 * arcs with three points given.
 *
 * @author Andrew Mustun
 */
class RS_ActionDrawArc3P : public RS_PreviewActionInterface {
	Q_OBJECT
public:
    /**
     * Action States.
     */
    enum Status {
        SetPoint1,       /**< Setting the 1st point. */
        SetPoint2,       /**< Setting the 2nd point. */
        SetPoint3        /**< Setting the 3rd point. */
    };

public:
    RS_ActionDrawArc3P(RS_EntityContainer& container,
                       RS_GraphicView& graphicView);
    ~RS_ActionDrawArc3P();

    void reset();

    virtual void init(int status=0);
	
    virtual void trigger();
	void preparePreview();
	
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);

	virtual void coordinateEvent(RS_CoordinateEvent* e);
    virtual void commandEvent(RS_CommandEvent* e);
        virtual QStringList getAvailableCommands();
	
    virtual void updateMouseButtonHints();
    virtual void updateMouseCursor();

protected:
    /**
     * Arc data defined so far.
     */
	std::unique_ptr<RS_ArcData> data;
    /**
     * 1st point.
     */
    RS_Vector point1;
    /**
     * 2nd point.
     */
    RS_Vector point2;
    /**
     * 3nd point.
     */
    RS_Vector point3;
};

#endif
