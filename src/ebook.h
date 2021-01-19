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
#ifndef EBOOK_H
#define EBOOK_H

#include "ebcore.h"

#include <QTextCodec>

#include <ebu/eb.h>

enum SearchType { SearchWord, SearchEndWord, SearchExactWord,
                  SearchKeyWord, SearchCrossWord };

class EbMenu : public EbCore
{
public:
    EbMenu() : EbCore(HookMenu) {}
    ~EbMenu() {}

    QList <CandItem> topMenu();
};

class EBook : public EbCore
{
public:
    EBook(HookMode hmode);
    ~EBook();

    int searchQuery(int maxcnt, const QString &query, SearchType type);

    // return number of Sub Book
    QString hitText(int index)
    {
        return EbCore::text(hits[index].text);
    }

    QString hitHeading(int index)
    {
        return EbCore::heading(hits[index].heading);
    }

    inline EB_Hit hit(int index)
    {
        return hits[index];
    }

    void getMatch(int index, QString *head_l, QString *head_v, QString *text, bool highlightMatches);
    void getText(int lndex, QString *head_l, QString *head_v, QString *text);

protected:

    QList <EB_Hit> hits;

private:
    QStringList words;
    int hitWord(int maxcnt, const QString &word, SearchType type);
    int hitMultiWord(int maxcnt, const QStringList &word_list, SearchType type);

};

class EbAll : public EBook
{
public:
    EbAll(HookMode mode) : EBook(mode) {}
    ~EbAll() {}

    int hitFull(int maxcnt);
    int setStartHit(const EB_Position &text_pos);
    void initSeek() { firstSeek = true; }

private:
    EB_Position seekPosition;
    bool firstSeek;
};

#endif
