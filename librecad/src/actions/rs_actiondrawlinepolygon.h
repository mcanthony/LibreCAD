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

#ifndef RS_ACTIONDRAWLINEPOLYGON_H
#define RS_ACTIONDRAWLINEPOLYGON_H

#include "rs_previewactioninterface.h"


/**
 * This action class can handle user events to draw polygons.
 *
 * @author Andrew Mustun
 */
class RS_ActionDrawLinePolygonCenCor : public RS_PreviewActionInterface {
	Q_OBJECT
    enum Status {
        SetCenter,    /**< Setting center. */
		SetCorner,    /**< Setting corner. */
		SetNumber     /**< Setting number in the command line. */
	};
	
public:
    RS_ActionDrawLinePolygonCenCor(RS_EntityContainer& container,
                              RS_GraphicView& graphicView);
    ~RS_ActionDrawLinePolygonCenCor() = default;

    virtual void trigger();
	
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void updateMouseButtonHints();
	
	virtual void coordinateEvent(RS_CoordinateEvent* e);
    virtual void commandEvent(RS_CommandEvent* e);
        virtual QStringList getAvailableCommands();
	
    virtual void hideOptions();
    virtual void showOptions();

    virtual void updateMouseCursor();

	int getNumber() const{
		return number;
	}

	void setNumber(int n) {
		number = n;
	}

private:
	/** Center of polygon */
	RS_Vector center;
	/** Edge */
	RS_Vector corner;
	/** Number of edges. */
	int number;
	/** Last status before entering text. */
	Status lastStatus;
};

#endif
