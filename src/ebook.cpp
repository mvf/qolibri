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

#include <eb/eb.h>
#include <eb/binary.h>
#include <eb/text.h>
#include <eb/font.h>
#include <eb/appendix.h>
#include <eb/error.h>

#include "ebook.h"
#include "ebhook.h"
#include "textcodec.h"

const int HitsBufferSize = 10000;
const int TextBufferSize = 4000;
const int TextSizeLimit = 2800000;

QList <CandItems> EbMenu::topMenu()
{

    EB_Position pos = menu();
    if (!isValidPosition(pos)) {
        QList<CandItems> i;
        return i;
    }

    QString t;
    return candidate(pos, &t);

}

EBook::EBook(HookMode hmode)
    : EbCore(hmode)
{
}

EBook::~EBook()
{
}

int EBook::hitMultiWord(int maxcnt, const QStringList &words, SearchType stype)
{
    hits.clear();
    if ((stype == SearchKeyWord && !isHaveWordSearch()) ||
        (stype == SearchCrossWord && !isHaveCrossSearch()) )
        return 0;

    if ( maxcnt <= 0 )
        maxcnt = HitsBufferSize;
    int count = 0;
    for (;;) {
        EB_Error_Code ecode;
        if (stype == SearchKeyWord) {
            ecode = searchKeyword(words);
        } else {
            ecode = searchCross(words);
        }
        if (ecode != EB_SUCCESS) {
            break;
        }

        QList <EB_Hit> wrk = hitList(HitsBufferSize);
        if (wrk.count() == 0) {
            return 0;
        }

        foreach(EB_Hit w, wrk) {
            bool same_text = false;
            foreach(EB_Hit h, hits) {
                if (w.text.page == h.text.page &&
                    w.text.offset == h.text.offset) {
                    same_text = true;
                    break;
                }
            }
            if (same_text) continue;

            hits << w;
            count++;
            if (count >= maxcnt) break;
        }

        break;
    }

    return count;
}

int EBook::hitWord(int maxcnt, const QString &word, SearchType type)
{
    hits.clear();
    if ( maxcnt <= 0 ) maxcnt = HitsBufferSize;
    EB_Error_Code ecode;
    if (type == SearchWord) {
	if (!isHaveWordSearch())
	    return 0;
        ecode = searchWord(word);
        if (ecode != EB_SUCCESS) {
            return -1;
        }
    } else if (type == SearchEndWord) {
	if (!isHaveEndwordSearch())
	    return 0;
        ecode = searchEndword(word);
        if (ecode != EB_SUCCESS) {
            return -1;
        }
    } else {
	if (!isHaveExactwordSearch())
	    return 0;
        ecode = searchExactword(word);
        if (ecode != EB_SUCCESS) {
            return -1;
        }
    }

    QList <EB_Hit> wrk = hitList(HitsBufferSize);
    if (wrk.count() == 0) {
        return 0;
    }

    int count = 0;
    foreach(EB_Hit w, wrk) {
        bool same_text = false;
        foreach (EB_Hit h, hits) {
            if (w.text.page == h.text.page &&
                w.text.offset == h.text.offset) {
                same_text = true;
                break;
            }
        }
        if (same_text)  continue;
        hits << w;

        count++;
        if (count >= maxcnt) break;
    }
    return count;
}

int EbAll::hitFull(int maxcnt)
{
    hits.clear();
    EB_Error_Code ecode;
    EB_Position position;
    int count = 0;

    if (firstSeek) {
        position = startText();
        if (!isValidPosition(position)) {
            return -1;
        }
        firstSeek = false;
    } else {
        position = seekPosition;
    }
    EB_Hit hit;
    hit.heading = position;
    hit.text = position;
    hits << hit; 
    count++;
    while (count <= maxcnt) {
        ecode = seekText(position);
        if (ecode != EB_SUCCESS) {
            break;
        }
        ecode = forwardText();
        if (ecode != EB_SUCCESS) {
            if (ecode == EB_ERR_END_OF_CONTENT &&
                position.page < book.subbook_current->text.end_page) {
                //qDebug() << "hitFull : page=" << position.page
                //         << "offset=" << position.offset
                //         << "end page=" << book.subbook_current->text.end_page;
                position = tellText( );
                //qDebug() << "tell_text : page=" << position.page
                //         << "offset=" << position.offset;
                if (!isValidPosition(position)){
                    break;
                }
                if (position.page >= book.subbook_current->text.end_page) {
                    //    qDebug() << "hitFull : page=" << position.page
                    //             << "offset=" << position.offset ;
                    break;
                }
                position.offset += 2;
            } else {
                break;
            }
        } else {
            position = tellText();
            if (!isValidPosition(position)){
                break;
            }
        }
        if (count < maxcnt) {
            EB_Hit hit;
            hit.heading = position;
            hit.text = position;
            hits << hit; 
        }
        count++;
    }
    seekPosition = position;
    return count - 1;
}

int EbAll::setStartHit(const EB_Position &text_pos)
{
    seekPosition = text_pos;
    firstSeek = false;
    return 0;
}

