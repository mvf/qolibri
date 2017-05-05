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


void addDirectionAct(QMenu *menu, const QString &title, SearchDirection direc)
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

Query::Query(const QString &query_, const SearchMethod &method_)
    : query(query_.simplified())
    , method(method_)
{
}

QString Query::toLogicString() const
{
    return query;
}

QStringList stemWords(const QString &words)
{
    struct {
        const char* suffix;
        const char* stem;
    } dict[] = {
        {"ies",  "y"},
        {"ied",  "y"},
        {"es",  ""},
        {"ting",  "te"},
        {"ing",  ""},
        {"ing",  "e"},
        {"ed",  "e"},
        {"ed",  ""},
        {"id",  "y"},
        {"ices",  "ex"},
        {"ves",  "fe"},
        {"s",  ""},
    };

    QStringList list;
    for (size_t i=0; i<sizeof(dict)/sizeof(dict[0]); i++) {
        QString suffix(dict[i].suffix);
        if (words.endsWith(suffix, Qt::CaseInsensitive)) {
            QString stem(dict[i].stem);
            QString t(words);
            t.chop(suffix.length());
            t.append(stem);
            list << t;
        }
    }
    return list;
}
