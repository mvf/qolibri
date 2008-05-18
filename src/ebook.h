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
    int setBook(const QString &path, int sub_book_no, int ref_pos = 0);
    int setSubBook(int sub_book_no, int ref_pos = 0);
    void initSearch(int fsize, QHash<QString, QString> *flist, 
                    int indent_offset = 50, bool ruby = true)
    {   
        fontSize_ = fsize;                                                
        fontList_ = flist;
        indentOffset_ = indent_offset;
        ruby_ = ruby;
        ebCache.init(title());
        ebHook.init(&ebCache, fsize, flist, indent_offset, ruby);
    }

    QString copyright();
    QString menu();
    bool menu(int *page, int *offset);

    QString heading(int page, int offset, bool hook = true);
    QString text(int page, int offset, bool hook = true);
    QString heading(int index, bool hook = true)
    {
        return heading(hits[index].heading.page,
                       hits[index].heading.offset, hook);
    }
    QString text(int index, bool hook = true)
    {
        return text(hits[index].text.page, hits[index].text.offset, hook);
    }
    QStringList candidate(int page, int offset, QString *txt);

    int hitWord(int maxcnt, const QString &word, SearchType type);
    int hitMultiWord(int maxcnt, const QStringList &word_list, SearchType type);
    int hitFull(int maxcnt);
    inline EB_Hit hit(int index)
    {
        return hits[index];
    }
    int setStartHit(const EB_Position &text_pos);

    QByteArray begin_decoration(int deco_code);
    QByteArray end_decoration();
    QByteArray set_indent(int val);
    QByteArray begin_reference();
    QByteArray end_reference(int page, int offset);
    QByteArray begin_candidate();
    QByteArray begin_candidate_menu();
    QByteArray end_candidate_group(int page, int offset);
    QByteArray end_candidate_group_menu(int page, int offset);
    QByteArray narrow_font(int code);
    QByteArray wide_font(int code);
    QByteArray end_mono_graphic(int page, int offset);
    QByteArray begin_mpeg();
    QByteArray begin_wave(int start_page, int start_offset,
                          int end_page, int end_offset);
    QByteArray begin_color_bmp(int page, int offset);
    QByteArray begin_color_jpeg(int page, int offset);
    QByteArray errorString(const QByteArray &error_string)
    {
        return "<em class=err>" + error_string + "</em>";
    }
    QByteArray begin_subscript()
    {
	return (ruby_) ? "<sub>" : QByteArray();
    }
    QByteArray end_subscript()
    {
	return (ruby_) ? "</sub>" : QByteArray();
    }
    void end_mpeg(const unsigned int *p);
    void begin_mono_graphic(int height, int width)
    {
        monoHeight_ = height;
        monoWidth_ = width;
        imageCount_++;
    }

    inline int imageCount() const
    {
        return imageCount_;
    }
    inline int fontSize() const
    {
        return fontSize_;
    }
    inline QHash <QString, QString> *fontList() const
    {
        return fontList_;
    }
    QString fontCachePath()
    {
        return fontCachePath_;
    }

    //QString loadAllFonts();

    static QString cachePath;


private:
    void setCache(const QString &name);
    inline QByteArray makeFname(const QByteArray &type, int p1, int p2)
    {
        return QByteArray::number(p1) + 'x' + QByteArray::number(p2) +
               '.' + type;
    }

    EB_Hit *hits;
    QHash <QString, QString> *fontList_;
    QStack <int> decoStack;

    QString fontCachePath_;
    QString imageCachePath;
    QString waveCachePath;
    QString mpegCachePath;
    QByteArray fontCacheRel;
    QByteArray imageCacheRel;

    QStringList candList;
    QStringList waveCacheList;
    QStringList fontCacheList;
    QStringList imageCacheList;
    QList <QByteArray> mpegList;
    QList <QByteArray> refList;

    int monoHeight_;
    int monoWidth_;
    int imageCount_;
    int fontSize_;
    int refPosition_;
    int indentOffset_;
    bool ruby_;
    bool indented_;

    //int subAppendixCount;
    EB_Position seekPosition;
    EB_Position curPosition;
    bool firstSeek;
};

#endif

