/***************************************************************************
*   Copyright (C) 2007 by BOP                                             *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include <QtCore>
#include <QMenu>

#include "method.h"

QString toLogicString(const QStringList &list, const SearchMethod &method,
                      bool and_flag)
{

    if (list.count() <= 0) {
        qWarning() << "No Search String List";
        return QString();
    }
    QString str = list[0];
    if (list.count() == 1) {
        return str;
    }
    bool narrow = (method.direction == KeywordSearch ||
                   method.direction == CrossSearch) ? false : true;

    QString slogic = " | ";

    if (!narrow || method.logic == LogicAND) {
        if (and_flag) {
            slogic = " & ";
        } else {
            slogic = " && ";
        }
    }

    if (narrow && method.direction != FullTextSearch) {
        str += " ( " + list[1];
        for (int i = 2; i < list.count(); i++) {
            str += slogic + list[i];
        }
        str += " )";
    } else {
        for (int i = 1; i < list.count(); i++) {
            str += slogic + list[i];
        }
    }

    return str;
}

void addDirectionAct(QMenu *menu, QString title, SearchDirection direc)
{
    QAction *act =  menu->addAction(title);
    act->setData(direc);
}

void addDirectionMenu(QMenu *menu)
{
    addDirectionAct(menu, QObject::tr("&Exact word search"), ExactWordSearch);
    addDirectionAct(menu, QObject::tr("&Forward search"), ForwardSearch);
    addDirectionAct(menu, QObject::tr("&Keyword search"), KeywordSearch);
    addDirectionAct(menu, QObject::tr("&Cross search"), CrossSearch);
    menu->addSeparator();
    addDirectionAct(menu, QObject::tr("&Google search"), GoogleSearch);
    addDirectionAct(menu, QObject::tr("&WikiPedia search"), WikipediaSearch);
    addDirectionAct(menu, QObject::tr("&User defined URL search"),
                    Option1Search);
}
