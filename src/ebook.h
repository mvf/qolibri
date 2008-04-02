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

enum HookMode { HookText, HookMenu, HookFont };
enum SearchType { SearchWord, SearchEndWord, SearchExactWord,
                  SearchKeyWord, SearchCrossWord };

class EBook : public QObject
{
public:
    EBook(HookMode menu = HookText);
    ~EBook();

    // return number of Sub Book
    int setBook(const QString &path);
    int setBook(const QString &path, int sub_book_no, int ref_pos = 0);
    int setSubBook(int sub_book_no, int ref_pos = 0);
    inline void unsetBook()
    {
        eb_unset_subbook(&book);
        //eb_finalize_book(&book);
    }
    void initSearch(int fsize, QHash<QString, QString> *flist, 
                    int indent_offset = 50, bool ruby = true)
    {   
        fontSize_ = fsize;                                                
        fontList_ = flist;
        indentOffset_ = indent_offset;
        ruby_ = ruby;
    }

    QString path();
    QString copyright();
    QString menu();
    bool menu(int *page, int *offset);

    QString title();

    inline bool isHaveText()
    {
        return (eb_have_text(&book) == 1);
    }
    inline bool isHaveWordSearch()
    {
        return (eb_have_word_search(&book) == 1);
    }
    inline bool isHaveKeywordSearch()
    {
        return (eb_have_keyword_search(&book) == 1);
    }
    inline bool isHaveEndwordSearch()
    {
        return (eb_have_endword_search(&book) == 1);
    }
    inline bool isHaveCrossSearch()
    {
        return (eb_have_cross_search(&book) == 1);
    }
    inline bool isHaveMenu()
    {
        return (eb_have_menu(&book) == 1);
    }
    inline bool isHaveCopyright()
    {
        return (eb_have_copyright(&book) == 1);
    }



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
    inline void begin_mono_graphic(int height, int width)
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

    static void initialize()
    {
        eb_initialize_library();
        codecEuc = QTextCodec::codecForName("EUC-JP");
    }
    static void finalize()
    {
        eb_finalize_library();
    }
    static QString cachePath;
    static QTextCodec *codecEuc;

private:
    void ebError(const QString &func, EB_Error_Code code);
    void setCache(const QString &name);
    static QString eucToUtf(const QByteArray &a)
    {
        return codecEuc->toUnicode(a);
    }

    static QByteArray utfToEuc(const QString &s)
    {
        return codecEuc->fromUnicode(s);
    }
    inline QByteArray makeFname(const QByteArray &type, int p1, int p2)
    {
        return QByteArray().setNum(p1) + 'x' + QByteArray().setNum(p2) +
               '.' + type;
    }

    EB_Book book;
    EB_Appendix appendix;
    EB_Hookset hookSet;
    EB_Subbook_Code subBookList[EB_MAX_SUBBOOKS];
    EB_Subbook_Code subAppendixList[EB_MAX_SUBBOOKS];
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

    int subAppendixCount;
    EB_Position seekPosition;
    EB_Position curPosition;
    bool firstSeek;
};

#endif

