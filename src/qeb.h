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
#ifndef QEB_H
#define QEB_H

#include <QtCore>
#include <QObject>
#include <QList>

#include <eb/eb.h>
#include <eb/text.h>
#include <eb/binary.h>
#include <eb/font.h>
#include <eb/appendix.h>
#include <eb/booklist.h>
#include <eb/error.h>

#include "textcodec.h"

#define toUTF(q_bytearray) \
    QTextCodec::codecForLocale()->toUnicode(q_bytearray)

#define HOOK_DEF(code,class_name,function) \
    EB_Error_Code function(int argc, unsigned int* argv); \
    static EB_Error_Code Hook##code(EB_Book*,EB_Appendix*,void *classp, \
            EB_Hook_Code, int argc, unsigned int* argv) \
    { \
        class_name *p = static_cast<class_name*>(classp); \
        return p->function(argc,argv); \
    }

class QEb : public QObject
{

protected:
    EB_Book book;
    EB_Appendix appendix;
    EB_Hookset hookset;
    EB_BookList bookList;
    EB_Font_Code fontCode;
    EB_Error_Code lastError;

public:
    QEb() {}
    ~QEb() {}

    static void initialize() { eb_initialize_library(); }
    static void finalize() { eb_finalize_library(); }

    // Initialze Book
    void initializeBook() { eb_initialize_book(&book); }
    void finalizeBook() { eb_finalize_book(&book); }
    EB_Error_Code bind(const QString &path);
    bool isBound() { return (eb_is_bound(&book) == 1); }
    QString path();
    EB_Disc_Code discType();
    EB_Character_Code characterCode();

    // Error 
    QString errorString(EB_Error_Code error)
        { return toUTF(eb_error_string(error)); }
    QString errorMessage(EB_Error_Code error)
        { return toUTF(eb_error_message(error)); }
    void dispError(const QString &func, EB_Error_Code error)
        { qWarning() << func << errorMessage(error);  lastError = error; }


    // Initialize Subbook
    QList <EB_Subbook_Code> subbookList();
    EB_Error_Code loadAllSubbooks();
    EB_Subbook_Code subbook();
    QString subbookTitle();
    QString subbookTitle2(EB_Subbook_Code code);
    QString subbookDirectory();
    QString subbookDirectory2(EB_Subbook_Code code);
    EB_Error_Code setSubbook(EB_Subbook_Code code);
    void unsetSubbook() { eb_unset_subbook(&book); }

    // Search
    bool isHaveText() { return (eb_have_text(&book) == 1); }
    bool isHaveWordSearch() { return (eb_have_word_search(&book) == 1); }
    bool isHaveEndwordSearch() { return (eb_have_endword_search(&book) == 1); }
    bool isHaveExactwordSearch()
        { return (eb_have_exactword_search(&book) == 1); }
    bool isHaveKeywordSearch() { return (eb_have_keyword_search(&book) == 1); }
    bool isHaveCrossSearch() { return (eb_have_cross_search(&book) == 1); }
    bool isHaveMultiSearch() { return (eb_have_multi_search(&book) == 1); }

    QList <EB_Multi_Search_Code> multiSearchList();
    int multiEntryCount(EB_Multi_Search_Code mid);
    QString multiTitle(EB_Multi_Search_Code mid);
    QString multiEntryLabel(EB_Multi_Search_Code mid, int entry);
    bool isMultiEntryHaveCandidates(EB_Multi_Search_Code mid, int entry)
        { return (eb_multi_entry_have_candidates(&book, mid, entry) == 1); }
    EB_Position invalidPosition()
        { EB_Position pos = { -1, -1 }; return pos; }
    bool isValidPosition(const EB_Position &pos)
        { return (pos.offset != -1 && pos.page != -1); }
    EB_Position multiEntryCandidates(EB_Multi_Search_Code mid, int entry);
    EB_Error_Code searchWord(const QString &word);
    EB_Error_Code searchEndword(const QString &word);
    EB_Error_Code searchExactword(const QString &word);
    QList <QByteArray> toEucList(const QStringList &words);
    EB_Error_Code searchKeyword(const QStringList &words);
    EB_Error_Code searchCross(const QStringList &words);
    EB_Error_Code searchMulti(EB_Multi_Search_Code mid,
            const QStringList &words);
    QList <EB_Hit> hitList(int max_count);

    // Text
    EB_Position startText();
    EB_Error_Code seekText(const EB_Position &pos);
    QString readText(void *para, bool hook_flag=true);
    QString readHeading(void *para, bool hook_flag=true);
    EB_Error_Code forwardText();
    EB_Error_Code backwardText();
    EB_Position copyright();
    bool isHaveCopyright() { return (eb_have_copyright(&book) == 1); }
    EB_Position menu();
    bool isHaveMenu() { return (eb_have_menu(&book) == 1); }
    EB_Position tellText();
    bool isTextStopped() { return (eb_is_text_stopped(&book) == 1); }
    EB_Error_Code writeTextByte1(int byte1);
    EB_Error_Code writeTextByte2(int byte1, int byte2);
    EB_Error_Code writeTextString(const QString &s);
    EB_Error_Code writeText(const QString &s);
    QString currentCandidate();

    // Initialize Hook
    void initializeHookset() { eb_initialize_hookset(&hookset); }
    void finalizeHookset() { eb_finalize_hookset(&hookset); }
    EB_Error_Code setHook(const EB_Hook hook);
    EB_Error_Code setHooks(const EB_Hook *hooks);

    // Fonts
    EB_Font_Code font();
    EB_Error_Code setFont(EB_Font_Code font);
    void unsetFont() { eb_unset_font(&book); }
    QList <EB_Font_Code> fontList();
    bool isHaveFont(EB_Font_Code font)
        { return (eb_have_font(&book, font) == 1); }
    int fontHeight();
    int fontHeight(EB_Font_Code font);
    bool isHaveNarrowFont() { return (eb_have_narrow_font(&book) == 1); }
    bool isHaveWideFont() { return (eb_have_wide_font(&book) == 1); }
    int narrowFontWidth();
    int wideFontWidth();
    int wideFontWidth(EB_Font_Code font);
    int narrowFontWidth(EB_Font_Code font);
    int wideFontSize();
    int narrowFontSize();
    int wideFontSize(EB_Font_Code font);
    int narrowFontSize(EB_Font_Code font);
    int wideFontStart();
    int narrowFontStart();
    int wideFontEnd();
    int narrowFontEnd();
    QByteArray narrowFontCharacterBitmap(int c_num);
    QByteArray wideFontCharacterBitmap(int c_num);
    int forwardNarrowFontCharacter(int n, int c_num);
    int forwardWideFontCharacter(int n, int c_num);
    int backwardNarrowFontCharacter(int n, int c_num);
    int backwardWideFontCharacter(int n, int c_num);
    int narrowFontXbmSize(EB_Font_Code height);
    int narrowFontXpmSize(EB_Font_Code height);
    int narrowFontGifSize(EB_Font_Code height);
    int narrowFontBmpSize(EB_Font_Code height);
    int narrowFontPngSize(EB_Font_Code height);
    int wideFontXbmSize(EB_Font_Code height);
    int wideFontXpmSize(EB_Font_Code height);
    int wideFontGifSize(EB_Font_Code height);
    int wideFontBmpSize(EB_Font_Code height);
    int wideFontPngSize(EB_Font_Code height);
    QSize narrowFontQSize();
    QSize wideFontQSize();
    QByteArray narrowBitmapToXbm(const QByteArray &bitmap);
    QByteArray wideBitmapToXbm(const QByteArray &bitmap);
    QByteArray narrowBitmapToXpm(const QByteArray &bitmap);
    QByteArray wideBitmapToXpm(const QByteArray &bitmap);
    QByteArray narrowBitmapToGif(const QByteArray &bitmap);
    QByteArray wideBitmapToGif(const QByteArray &bitmap);
    QByteArray narrowBitmapToBmp(const QByteArray &bitmap);
    QByteArray wideBitmapToBmp(const QByteArray &bitmap);
    QByteArray narrowBitmapToPng(const QByteArray &bitmap);
    QByteArray wideBitmapToPng(const QByteArray &bitmap);

    // Graphics, Sounds, Movies
    EB_Error_Code setBinaryMonoGraphic(const EB_Position &position, int width,
                                       int height);
    EB_Error_Code setBinaryColorGraphic(const EB_Position &position);
    EB_Error_Code setBinaryWave(const EB_Position &start, EB_Position &end);
    EB_Error_Code setBinaryMpeg(const unsigned int *argv);
    QByteArray readBinary();
    QString composeMovieFileName(const unsigned int *argv);
    QString composeMoviePathName(const unsigned int *argv);
    EB_Error_Code decomposeMovieFileName(unsigned int *argv,
                                         const QString &name);

    // Appendix
    void initializeAppendix() { eb_initialize_appendix(&appendix); }
    void finalizeAppendix() { eb_finalize_appendix(&appendix); }
    EB_Error_Code bindAppendix(QString path);
    bool isAppendixBound() { return (eb_is_appendix_bound(&appendix) == 1); }
    QString appendixPath();
    EB_Error_Code loadAllAppendixSubbooks();
    QList <EB_Subbook_Code> appendixSubbookList();
    EB_Subbook_Code appendixSubbook();
    QString appendixSubbookDirectory();
    QString appendixSubbookDirectory(EB_Subbook_Code code);
    EB_Error_Code setAppendixSubbook(EB_Subbook_Code code);
    void unsetAppendixSubbook() { eb_unset_appendix_subbook(&appendix); }
    bool isHaveStopCode() { return (eb_have_stop_code(&appendix) == 1); }
    EB_Error_Code stopCode(int *stop1, int *stop2);
    bool haveNarrowAlt() { return (eb_have_narrow_alt(&appendix) == 1); }
    bool haveWideAlt() { return (eb_have_wide_alt(&appendix) == 1); }
    int narrowAltStart();
    int wideAltStart();
    int narrowAltEnd();
    int wideAltEnd();
    QString narrowAltCharacterText(int c_num);
    QString wideAltCharacterText(int c_num);
    int backwardNarrowAltCharacter(int n, int c_num);
    int forwardNarrowAltCharacter(int n, int c_num);
    int backwardWideAltCharacter(int n, int c_num);
    int forwardWideAltCharacter(int n, int c_num);

    // Book List
    void initializeBooklist() { eb_initialize_booklist(&bookList); }
    EB_Error_Code  bindBooklist(const QString &path);
    void finalizeBooklist() { eb_finalize_booklist(&bookList); }
    EB_Error_Code booklistBookCount();
    QString booklistBookName(int idx);
    QString booklistBookTitle(int idx);

};

#endif
