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

#include "rs_actionselectsingle.h"

#include <QAction>
#include <QMouseEvent>
#include "rs_dialogfactory.h"
#include "rs_selection.h"



RS_ActionSelectSingle::RS_ActionSelectSingle(RS_EntityContainer& container,
											 RS_GraphicView& graphicView,
											 RS_ActionInterface* actionSelect,
											 std::set<RS2::EntityType> const& entityTypeList)
	:RS_ActionInterface("Select Entities", container, graphicView)
	,entityTypeList(entityTypeList)
	,en(nullptr)
{
	actionType=RS2::ActionSelectSingle;
    if(actionSelect != NULL){
        if(actionSelect->rtti() == RS2::ActionSelect) {
            this->actionSelect=static_cast<RS_ActionSelect*>(actionSelect);
                this->actionSelect->requestFinish(true);
        }else{
			this->actionSelect=nullptr;
        }
    }
}


void RS_ActionSelectSingle::trigger() {
	if (en && (entityTypeList.empty() ||
			   entityTypeList.count(en->rtti())
			   )
	){
        RS_Selection s(*container, graphicView);
        s.selectSingle(en);

        RS_DIALOGFACTORY->updateSelectionWidget(container->countSelected(),container->totalSelectedLength());
    } else {
        RS_DEBUG->print("RS_ActionSelectSingle::trigger: Entity is NULL\n");
    }
}


void RS_ActionSelectSingle::keyPressEvent(QKeyEvent* e)
{
    if (container->countSelected() > 0 && e->key()==Qt::Key_Enter)
    {
        finish(false);
        actionSelect->keyPressEvent(e);
    }
}


void RS_ActionSelectSingle::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button()==Qt::RightButton) {
        //need to finish the parent RS_ActionSelect as well, bug#3437138
            //need to check actionSelect is set, bug#3437138
        if(actionSelect) {
            actionSelect->requestFinish(false);
        }
        init(getStatus()-1);
    } else {
        if(entityTypeList.size()) {
//            std::cout<<"RS_ActionSelectSingle::mouseReleaseEvent(): entityTypeList->size()="<< entityTypeList->size()<<std::endl;
            en = catchEntity(e, entityTypeList);
        }else{
            en = catchEntity(e);
        }
        trigger();
    }
}



void RS_ActionSelectSingle::updateMouseCursor() {
    graphicView->setMouseCursor(RS2::SelectCursor);
}

// EOF
