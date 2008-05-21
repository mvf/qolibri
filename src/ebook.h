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

#include <QTextCodec>
#include <eb/eb.h>

#include "ebcore.h"

enum HookMode { HookText, HookMenu, HookFont };
enum SearchType { SearchWord, SearchEndWord, SearchExactWord,
                  SearchKeyWord, SearchCrossWord };

class EBook : public EbCore
{
public:
    EBook(HookMode menu = HookText);
    ~EBook();

    // return number of Sub Book
    void initSearch(int fsize, QHash<QString, QString> *flist, 
                    int indent_offset = 50, bool ruby = true)
    {   
        ebCache.init(title());
        ebHook.init(&ebCache, fsize, flist, indent_offset, ruby);
        firstSeek = true;
    }

    QString hitText(int index, bool hook = true)
    {
        return EbCore::text(hits[index].text, hook);
    }

    QString hitHeading(int index, bool hook = true)
    {
        return EbCore::heading(hits[index].heading, hook);
    }

    int hitWord(int maxcnt, const QString &word, SearchType type);
    int hitMultiWord(int maxcnt, const QStringList &word_list, SearchType type);
    int hitFull(int maxcnt);
    inline EB_Hit hit(int index)
    {
        return hits[index];
    }
    int setStartHit(const EB_Position &text_pos);


    inline int imageCount() const
    {
        return ebHook.imageCount;
    }

private:

    EB_Hit *hits;

    EB_Position seekPosition;
    bool firstSeek;
};

#endif

