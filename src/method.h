/***************************************************************************
*   Copyright (C) 2007 by BOP                                             *
*   polepolek@gmail.com                                                   *
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
#ifndef METHOD_H
#define METHOD_H

#include <QStringList>

enum RET_SEARCH
{ NORMAL = 0, SEARCH_ERROR, NO_BOOK, NOT_HIT, INTERRUPTED, NOT_HIT_INTERRUPTED,
  NO_MENU, LIMIT_CHAR, LIMIT_BOOK, LIMIT_TOTAL, LIMIT_MENU };

enum SearchDirection { ExactWordSearch = 0, ForwardSearch, BackwardSearch,
                       KeywordSearch, CrossSearch, FullTextSearch,
                       WholeRead, MenuRead, BookInfo,
                       GoogleSearch, WikipediaSearch, Option1Search,
                       Option2Search };

enum NarrowingLogic { LogicAND = 0, LogicOR };

class Group;
class Book;
class QMenu;

struct SearchMethod {
    Group *         group;
    Book *          book;
    Group *         groupReader;
    Book *          bookReader;
    SearchDirection direction;
    NarrowingLogic  logic;
    int             limitTotal;
    int             limitBook;

    RET_SEARCH checkLimit(int totalCount, int matchCount, int textLength) const;
};

void addDirectionMenu(QMenu *menu);

struct Query {
    Query(const QString &query_, const SearchMethod &method_);
    QString query;
    SearchMethod method;
    QString toLogicString() const;
};

#endif
