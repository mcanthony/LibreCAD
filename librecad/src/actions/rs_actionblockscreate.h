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

#ifndef RS_ACTIONBLOCKSCREATE_H
#define RS_ACTIONBLOCKSCREATE_H

#include "rs_previewactioninterface.h"

/**
 * This action class can handle user events for creating blocks from
 * existing entities.
 *
 * @author Andrew Mustun
 */
class RS_ActionBlocksCreate : public RS_PreviewActionInterface {
        Q_OBJECT
public:
    /**
     * Action States.
     */
    enum Status {
        SetReferencePoint,    /**< Setting the reference point. */
                ShowDialog            /**< Showing dialog for name. */
    };

public:
    RS_ActionBlocksCreate(RS_EntityContainer& container,
                        RS_GraphicView& graphicView);
	~RS_ActionBlocksCreate() = default;

    virtual void init(int status=0);

    virtual void trigger();

    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);

    virtual void coordinateEvent(RS_CoordinateEvent* e);
    //virtual void commandEvent(RS_CommandEvent* e);
        //virtual QStringList getAvailableCommands();

        //virtual void showOptions();
        //virtual void hideOptions();

    virtual void updateMouseButtonHints();
    virtual void updateMouseCursor();

protected:
        RS_Vector referencePoint;
};

#endif
