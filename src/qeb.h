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

#include <QDebug>
#include <QList>
#include <QTextCodec>

#include <ebu/eb.h>
#include <ebu/text.h>
#include <ebu/binary.h>
#include <ebu/font.h>
#include <ebu/appendix.h>
#include <ebu/booklist.h>
#include <ebu/error.h>

#define toUTF(q_bytearray) \
    QTextCodec::codecForLocale()->toUnicode(q_bytearray)

class QEb
{
protected:
    EB_Book book;
    EB_Appendix appendix;
    EB_Hookset hookset;
    EB_BookList bookList;
    EB_Error_Code lastError;

    static QTextCodec *const eucCodec;

public:
    QEb() {}
    ~QEb() {}

    static void initialize() { eb_initialize_library(); }
    static void finalize() { eb_finalize_library(); }

    // Initialze Book
    void initializeBook() { eb_initialize_book(&book); }
    void finalizeBook() { eb_finalize_book(&book); }
    EB_Error_Code bind(const QString &path);
    QString path();
    EB_Character_Code characterCode();

    // Error
    QString errorMessage(EB_Error_Code error)
        { return toUTF(eb_error_message(error)); }
    void dispError(const QString &func, EB_Error_Code error)
        { qWarning() << func << errorMessage(error);  lastError = error; }


    // Initialize Subbook
    QList <EB_Subbook_Code> subbookList();
    QString subbookTitle();
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
    EB_Position invalidPosition()
        { EB_Position pos = { -1, -1 }; return pos; }
    bool isValidPosition(const EB_Position &pos)
        { return (pos.offset != -1 && pos.page != -1); }
    EB_Error_Code searchWord(const QString &word);
    EB_Error_Code searchEndword(const QString &word);
    EB_Error_Code searchExactword(const QString &word);
    QList <QByteArray> toEucList(const QStringList &words);
    EB_Error_Code searchKeyword(const QStringList &words);
    EB_Error_Code searchCross(const QStringList &words);
    QList <EB_Hit> hitList(int max_count);

    // Text
    EB_Position startText();
    EB_Error_Code seekText(const EB_Position &pos);
    QString readText(void *para, bool hook_flag=true);
    QString readHeading(void *para);
    EB_Error_Code forwardText();
    EB_Position copyright();
    bool isHaveCopyright() { return (eb_have_copyright(&book) == 1); }
    EB_Position menu();
    bool isHaveMenu() { return (eb_have_menu(&book) == 1); }
    EB_Position tellText();
    bool isTextStopped() { return (eb_is_text_stopped(&book) == 1); }
    QString currentCandidate();

    // Initialize Hook
    void initializeHookset() { eb_initialize_hookset(&hookset); }
    void finalizeHookset() { eb_finalize_hookset(&hookset); }
    EB_Error_Code setHooks(const EB_Hook *hooks);

    // Fonts
    EB_Font_Code font();
    EB_Error_Code setFont(EB_Font_Code font);
    QList <EB_Font_Code> fontList();
    bool isHaveFont(EB_Font_Code font)
        { return (eb_have_font(&book, font) == 1); }
    bool isHaveNarrowFont() { return (eb_have_narrow_font(&book) == 1); }
    bool isHaveWideFont() { return (eb_have_wide_font(&book) == 1); }
    int wideFontStart();
    int narrowFontStart();
    int wideFontEnd();
    int narrowFontEnd();
    QByteArray narrowFontCharacterBitmap(int c_num);
    QByteArray wideFontCharacterBitmap(int c_num);
    int forwardNarrowFontCharacter(int n, int c_num);
    int forwardWideFontCharacter(int n, int c_num);
    int narrowFontPngSize(EB_Font_Code height);
    int wideFontPngSize(EB_Font_Code height);
    QSize narrowFontQSize();
    QSize wideFontQSize();
    QByteArray narrowBitmapToPng(const QByteArray &bitmap);
    QByteArray wideBitmapToPng(const QByteArray &bitmap);

    // Graphics, Sounds, Movies
    EB_Error_Code setBinaryMonoGraphic(const EB_Position &position, int width,
                                       int height);
    EB_Error_Code setBinaryColorGraphic(const EB_Position &position);
    EB_Error_Code setBinaryWave(const EB_Position &start, EB_Position &end);
    QByteArray readBinary();
    QString composeMoviePathName(const unsigned int *argv);

    // Appendix
    void initializeAppendix() { eb_initialize_appendix(&appendix); }
    void finalizeAppendix() { eb_finalize_appendix(&appendix); }
    EB_Error_Code bindAppendix(const QString &path);
    QString appendixPath();
    QList <EB_Subbook_Code> appendixSubbookList();
    QString appendixSubbookDirectory();
    EB_Error_Code setAppendixSubbook(EB_Subbook_Code code);
    bool isHaveAppendixSubbook(EB_Subbook_Code code);
    EB_Error_Code stopCode(int *stop1, int *stop2);
    bool isHaveNarrowAlt() { return (eb_have_narrow_alt(&appendix) == 1); }
    bool isHaveWideAlt() { return (eb_have_wide_alt(&appendix) == 1); }
    int narrowAltStart();
    int wideAltStart();
    int narrowAltEnd();
    int wideAltEnd();
};

#endif
