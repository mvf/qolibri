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

#include "ebook.h"

#include <ebu/eb.h>
#include <ebu/binary.h>
#include <ebu/text.h>
#include <ebu/font.h>
#include <ebu/appendix.h>
#include <ebu/error.h>

const int HitsBufferSize = 10000;

QList <CandItem> EbMenu::topMenu()
{

    EB_Position pos = menu();
    if (!isValidPosition(pos)) {
        QList<CandItem> i;
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

int EBook::searchQuery(int maxcnt, const QString& query, SearchType type)
{
    switch (type) {
    case SearchKeyWord:
    case SearchCrossWord:
    {
        words = query.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        return hitMultiWord(maxcnt, words, type);
    }
    default:
        words = QStringList(query);
        return hitWord(maxcnt, query, type);
    }
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

static QString emphasize(const QString &str, const QString &word)
{
    enum { NO_SKIP=0, SKIP_TAG=0x01, SKIP_ENT=0x02 };
    QString ret;
    int slen = str.length();
    int wlen = word.length();
    unsigned int skip = NO_SKIP;

    for (int i = 0; i < slen; i++) {
        QChar a = str[i];
        if ((slen - i) < wlen) {
            ret += a;
            continue;
        }
        if (a == '<') {
            skip |= SKIP_TAG;
            ret += a;
            continue;
        }
        if (a == '&') {
            skip |= SKIP_ENT;
            ret += a;
            continue;
        }
        if (skip) {
            if (a == '>')
                skip &= ~SKIP_TAG;
	    else if (a == ';')
		skip &= ~SKIP_ENT;
            ret += a;
            continue;
        }
        if (a.isSpace()) {
            ret += a;
            continue;
        }
        QString cmp = str.mid(i, wlen);
        if (!QString::compare(cmp, word, Qt::CaseInsensitive)) {
            ret += "<span class=sel>" + cmp + "</span>";
            i += wlen - 1;
        } else {
            ret += a;
        }
    }

    return ret;
}

void EBook::getMatch(int index, QString *head_l, QString *head_v, QString *text, bool highlightMatches)
{
    getText(index, head_l, head_v, text);
    if (highlightMatches) {
        foreach(QString s, words) {
            *head_v = emphasize(*head_v, s);
            *text = emphasize(*text, s);
        }
    }
}

void EBook::getText(int index, QString *head_l, QString *head_v, QString *text)
{
    QString t_v = hitText(index);
    QString h_v = hitHeading(index);

    int p = t_v.indexOf('\n');
    if (h_v.isEmpty()) {
        h_v = t_v.left(p);
        t_v = t_v.mid(p+1);
    } else if (h_v == t_v.left(p)) {
        t_v = t_v.mid(p+1);
    }
    QString h_l = h_v;

    if (h_l.contains('<')) {
        h_l.replace(QRegExp("<img[^>]+>"), "?");
        //h_l.replace(regRep1, "?");
        if (h_l.contains('<')) {
            h_l.replace(QRegExp("<[^>]+>"), "");
            //h_l.replace(regRep2, "");
	}
    }

    int sp = 0;
    while((sp = h_l.indexOf('&', sp)) >= 0) {
	if (h_l.mid(sp+1, 3) == "lt;")
            h_l.replace(sp, 4, '<');
        else if (h_l.mid(sp+1, 3) == "gt;")
            h_l.replace(sp, 4, '>');
        else if(h_l.mid(sp+1, 4) == "amp;")
            h_l.replace(sp, 5, '&');
        else {
            int ep = h_l.indexOf(';', sp+1);
            if (ep < 0) break;
            h_l.replace(sp, ep-sp+1, '?');
        }
        sp++;
    }

    *head_l = h_l;
    *head_v = h_v;
    *text = t_v;
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

