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

#include "method.h"
#include "configure.h"

#include <QMenu>
#include <QRegularExpression>

namespace {
void addDirectionAct(QMenu *menu, const QString &title, SearchDirection direc)
{
    QAction *act =  menu->addAction(title);
    act->setData(direc);
}

QStringList makeWords(const QString &query, SearchDirection direction)
{
    return query.split(QRegularExpression{QStringLiteral(u"[\\s　]+")}, Qt::SkipEmptyParts);
}
} // anonymous namespace

RET_SEARCH SearchMethod::checkLimit(int totalCount, int matchCount, int textLength) const
{
    if (totalCount >= limitTotal)
        return LIMIT_TOTAL;
    if (matchCount >= limitBook)
        return LIMIT_BOOK;
    if (textLength >= CONF->limitBrowserChar)
        return LIMIT_CHAR;

    return NORMAL;
}

void addDirectionMenu(QMenu *menu)
{
    addDirectionAct(menu, QObject::tr("&Exact word search"), ExactWordSearch);
    addDirectionAct(menu, QObject::tr("&Forward search"), ForwardSearch);
    addDirectionAct(menu, QObject::tr("&Backward search"), BackwardSearch);
    addDirectionAct(menu, QObject::tr("&Keyword search"), KeywordSearch);
    addDirectionAct(menu, QObject::tr("&Cross search"), CrossSearch);
    addDirectionAct(menu, QObject::tr("Full &text search"), FullTextSearch);
    menu->addSeparator();
    addDirectionAct(menu, QObject::tr("&Google search"), GoogleSearch);
    addDirectionAct(menu, QObject::tr("&Wikipedia search"), WikipediaSearch);
    addDirectionAct(menu, QObject::tr("&User defined URL search"),
                    Option1Search);
}

Query::Query(const QString &query_, const SearchMethod &method_)
    : words(makeWords(query_, method_.direction))
    , method(method_)
{
}

QString Query::toLogicString() const
{
    return words.join(' ');
}
