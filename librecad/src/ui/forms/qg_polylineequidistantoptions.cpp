/****************************************************************************
**
  * Create option widget used to draw equidistant polylines

Copyright (C) 2011 Dongxu Li (dongxuli2011@gmail.com)
Copyright (C) 2011 R. van Twisk (librecad@rvt.dds.nl)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**********************************************************************/
#include "qg_polylineequidistantoptions.h"

#include "rs_actionpolylineequidistant.h"
#include "rs_settings.h"
#include "rs_math.h"

/*
  * Create option widget used to draw equidistant polylines
  *
  *@Author Dongxu Li
 */
QG_PolylineEquidistantOptions::QG_PolylineEquidistantOptions(QWidget* parent, Qt::WindowFlags fl)
    : QWidget(parent, fl)
{
    setupUi(this);

}

/*
 *  Destroys the object and frees any allocated resources
 */
QG_PolylineEquidistantOptions::~QG_PolylineEquidistantOptions()
{
    saveOptions();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QG_PolylineEquidistantOptions::languageChange()
{
    retranslateUi(this);
}

void QG_PolylineEquidistantOptions::setAction(RS_ActionInterface* a, bool update) {
	if (a && a->rtti()==RS2::ActionPolylineEquidistant) {
        action = (RS_ActionPolylineEquidistant*)a;

        QString sd;
        QString sn;

        // settings from action:
        if (update) {
            sd = QString("%1").arg(action->getDist());
            sn = QString("%1").arg(action->getNumber());
        }
        // settings from config file:
        else {
            RS_SETTINGS->beginGroup("/Draw");
            sd = RS_SETTINGS->readEntry("/PolylineEquidistantDist", "10.0");
            sn = RS_SETTINGS->readEntry("/PolylineEquidistantCopies", "1");
            RS_SETTINGS->endGroup();
        }

        leDist->setText(sd);
        leNumber->setText(sn);
    } else {
        RS_DEBUG->print(RS_Debug::D_ERROR,
                        "QG_PolylineEquidistantOptions::setAction: wrong action type");
        this->action = NULL;
    }
}

void QG_PolylineEquidistantOptions::saveOptions() {
    RS_SETTINGS->beginGroup("/Draw");
//    std::cout<<"QG_PolylineEquidistantOptions::saveOptions(): saving /PolylineEquidistantDist="<<qPrintable(leDist->text())<<std::endl;
    RS_SETTINGS->writeEntry("/PolylineEquidistantDist", leDist->text());
    RS_SETTINGS->writeEntry("/PolylineEquidistantCopies", leNumber->text());
    RS_SETTINGS->endGroup();
}

void QG_PolylineEquidistantOptions::updateDist(const QString& l) {
	if (action) {
        bool ok;
        double dist=RS_Math::eval(l,&ok);
		if(!ok){
            dist=10.;
            leDist->setText(QString::number(dist,'g',3));
        }
		if (action) {
            action->setDist(dist);
        }
    }
}

void QG_PolylineEquidistantOptions::updateNumber(const QString& l) {
    bool ok;
    unsigned i=static_cast<unsigned>(RS_Math::eval(l,&ok)+0.5);
	if (!ok || (i==0 && i>100)) {
        i=1;
        leNumber->setText(QString::number(i));
    }
	if (action) {
        action->setNumber(i);
    }
}
