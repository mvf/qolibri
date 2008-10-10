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
    EB_Hook *hooks;
    EB_Font_Code fontCode;
    EB_Error_Code ecode;

public:
    QEb() {}
    ~QEb() {}

    static void initialize();
    static void finalize();

    // Initialze Book
    void initializeBook()
    {
        eb_initialize_book(&book);
    }
    void finalizeBook()
    {
        eb_finalize_book(&book);
    }
    EB_Error_Code bind(const QString &path)
    {
        ecode = eb_bind(&book, path.toLocal8Bit());
        if (ecode != EB_SUCCESS) {
            dispError("eb_bind", ecode);
        }
        return ecode;
    }
    bool isBound()
    {
        return eb_is_bound(&book) ? true : false;
    }
    QString path()
    {
        char s[EB_MAX_PATH_LENGTH+1];
        ecode = eb_path(&book, s);
        if (ecode != EB_SUCCESS) {
            dispError("eb_path", ecode);
            return QString();
        }
        return QString::fromLocal8Bit(s);
    }
    EB_Disc_Code discType()
    {
        EB_Disc_Code dcode;
        ecode = eb_disc_type(&book, &dcode);
        if (ecode != EB_SUCCESS) {
            dispError("eb_disc_type", ecode);
            return EB_DISC_INVALID;
        }
        return dcode;
    }
    EB_Character_Code characterCode()
    {
        EB_Character_Code ccode;
        ecode = eb_character_code(&book, &ccode);
        if (ecode != EB_SUCCESS) {
            dispError("eb_character_code", ecode);
            return EB_CHARCODE_INVALID;
        }
        return ccode;
    }

    // Error 
    QString errorString(EB_Error_Code error)
    {
        return toUTF(eb_error_string(error));
    }
    QString errorMessage(EB_Error_Code error) 
    {
        return toUTF(eb_error_message(error));
    }
    void dispError(const QString &func, EB_Error_Code error)
    {
        qWarning() << func << errorMessage(error);
    }


    // Initialize Subbook
    QList <EB_Subbook_Code> subbookList()
    {
        EB_Subbook_Code codes[EB_MAX_SUBBOOKS];
        int cnt;

        QList <EB_Subbook_Code> list;
        ecode = eb_subbook_list(&book, codes, &cnt);
        if (ecode != EB_SUCCESS) {
            dispError("eb_subbook_list", ecode);
        } else {
            for(int i = 0; i < cnt; i++)
                list << codes[i];
        }
        return list;
    }
    EB_Error_Code loadAllSubbooks()
    {
        ecode = eb_load_all_subbooks(&book);
        if (ecode != EB_SUCCESS) {
            dispError("eb_subbook_list", ecode);
        }
        return ecode;
    }
    EB_Subbook_Code subbook()
    {
        EB_Subbook_Code code;
        ecode = eb_subbook(&book, &code);
        if (ecode != EB_SUCCESS) {
            dispError("eb_subbook_list", ecode);
        }
        return code;
    }
    
    QString subbookTitle()
    {
        char title[EB_MAX_TITLE_LENGTH+1];
        ecode = eb_subbook_title(&book, title);
        if (ecode != EB_SUCCESS) {
            dispError("eb_subbook_title", ecode);
            return QString();
        }
        return toUTF(title);
    }

    QString subbookTitle2(EB_Subbook_Code code)
    {
        char title[EB_MAX_TITLE_LENGTH+1];
        ecode = eb_subbook_title2(&book, code, title);
        if (ecode != EB_SUCCESS) {
            dispError("eb_subbook_title2", ecode);
            return QString();
        }
        return toUTF(title);
    }
    QString subbookDirectory()
    {
        char dir[EB_MAX_DIRECTORY_NAME_LENGTH+1];
        ecode = eb_subbook_directory(&book, dir);
        if (ecode != EB_SUCCESS) {
            dispError("eb_subbook_directory", ecode);
            return QString();
        }
        return toUTF(dir);
    }
    QString subbookDirectory2(EB_Subbook_Code code)
    {
        char dir[EB_MAX_DIRECTORY_NAME_LENGTH+1];
        ecode = eb_subbook_directory2(&book, code, dir);
        if (ecode != EB_SUCCESS) {
            dispError("eb_subbook_directory2", ecode);
            return QString();
        }
        return toUTF(dir);
    }
    EB_Error_Code setSubbook(EB_Subbook_Code code)
    {
        ecode = eb_set_subbook(&book, code);
        if (ecode != EB_SUCCESS) {
            dispError("eb_set_subbook", ecode);
        }
        return ecode;
    }
    void unsetSubbook()
    {
        eb_unset_subbook(&book);
    }

    // Search
    bool isHaveText()
    {
        return eb_have_text(&book) ? true : false;
    }
    bool isHaveWordSearch()
    {
        return eb_have_word_search(&book) ? true : false;
    }
    bool isHaveEndwordSearch()
    {
        return eb_have_endword_search(&book) ? true : false;
    }
    bool isHaveExactwordSearch()
    {
        return eb_have_exactword_search(&book) ? true : false;
    }
    bool isHaveKeywordSearch()
    {
        return eb_have_keyword_search(&book) ? true : false;
    }
    bool isHaveCrossSearch()
    {
        return eb_have_cross_search(&book) ? true : false;
    }
    bool isHaveMultiSearch()
    {
        return eb_have_multi_search(&book) ? true : false;
    }

    QList <EB_Multi_Search_Code> multiSearchList()
    {
        EB_Multi_Search_Code codes[EB_MAX_MULTI_SEARCHES];
        int cnt;
        QList <EB_Multi_Search_Code> list;

        ecode = eb_multi_search_list(&book, codes,  &cnt);
        if (ecode != EB_SUCCESS) {
            dispError("eb_multi_search_list", ecode);
        } else {
            for (int i = 0; i < cnt; i++)
                list << codes[i];
        }
        return list;
    }
    int multiEntryCount(EB_Multi_Search_Code mid)
    {
        int cnt;
        ecode = eb_multi_entry_count(&book, mid, &cnt);
        if (ecode != EB_SUCCESS) {
            dispError("eb_multi_search_list", ecode);
            return 0;
        }
        return cnt;
    }
    QString multiTitle(EB_Multi_Search_Code mid)
    {
        char s[EB_MAX_MULTI_TITLE_LENGTH+1];
        ecode = eb_multi_title(&book, mid, s);
        if (ecode != EB_SUCCESS) {
            dispError("eb_multi_title", ecode);
            return QString();
        }
        if (characterCode() == EB_CHARCODE_ISO8859_1) {
            return QString::fromLatin1(s);
        } else {
            return toUTF(s);
        }
    }
    QString multiEntryLabel(EB_Multi_Search_Code mid, int entry)
    {
        char s[EB_MAX_MULTI_LABEL_LENGTH+1];
        ecode = eb_multi_entry_label(&book, mid, entry, s);
        if (ecode != EB_SUCCESS) {
            dispError("eb_multi_entry_label", ecode);
            return QString();
        }
        if (characterCode() == EB_CHARCODE_ISO8859_1) {
            return QString::fromLatin1(s);
        } else {
            return toUTF(s);
        }
    }
    bool isMultiEntryHaveCandidates(EB_Multi_Search_Code mid, int entry)
    {
        return eb_multi_entry_have_candidates(&book, mid, entry) ? true : false;
    }
    EB_Position invalidPosition()
    {
        EB_Position pos;
        pos.page = -1;
        pos.offset = -1;
        return pos;
    }
    bool isValidPosition(const EB_Position &pos)
    {
        return (pos.offset != -1 && pos.page != -1) ? true : false;
    }
    EB_Position multiEntryCandidates(EB_Multi_Search_Code mid, int entry)
    {
        EB_Position pos;
        ecode = eb_multi_entry_candidates(&book, mid, entry, &pos);
        if (ecode != EB_SUCCESS) {
            dispError("eb_multi_entry_candidates", ecode);
            return invalidPosition();
        }
        return pos;
    }
    EB_Error_Code searchWord(const QString &word)
    {
        ecode = eb_search_word(&book, utfToEuc(word));
        if (ecode != EB_SUCCESS) {
            dispError("eb_search_word", ecode);
        }
        return ecode;
    }
    EB_Error_Code searchEndword(const QString &word)
    {
        ecode = eb_search_endword(&book, utfToEuc(word));
        if (ecode != EB_SUCCESS) {
            dispError("eb_search_endword", ecode);
        }
        return ecode;
    }
    EB_Error_Code searchExactword(const QString &word)
    {
        ecode = eb_search_exactword(&book, utfToEuc(word));
        if (ecode != EB_SUCCESS) {
            dispError("eb_search_exactword", ecode);
        }
        return ecode;
    }
    QList <QByteArray> toEucList(const QStringList &words)
    {
        QList <QByteArray> blist;
        for (int i = 0; i < words.count(); i++)
            blist << utfToEuc(words[i]);
        return blist;
    }
    EB_Error_Code searchKeyword(const QStringList &words)
    {
        QList <QByteArray> blist = toEucList(words);
        char** wlist = new char*[words.count()+1];
        for (int i = 0; i < blist.count(); i++)
            wlist[i] = blist[i].data();
        wlist[words.count()] = NULL;

        ecode = eb_search_keyword(&book, wlist);
        if (ecode != EB_SUCCESS) {
            dispError("eb_search_keyword", ecode);
        }
        delete wlist;
        return ecode;
    }
    EB_Error_Code searchCross(const QStringList &words)
    {
        QList <QByteArray> blist = toEucList(words);
        char** wlist = new char*[words.count()+1];
        for (int i = 0; i < blist.count(); i++)
            wlist[i] = blist[i].data();
        wlist[words.count()] = NULL;
        ecode = eb_search_cross(&book, wlist);
        if (ecode != EB_SUCCESS) {
            dispError("eb_search_cross", ecode);
        }
        delete wlist;
        return ecode;
    }
    EB_Error_Code searchMulti(EB_Multi_Search_Code mid,
            const QStringList &words)
    {
        QList <QByteArray> blist = toEucList(words);
        char** wlist = new char*[words.count()+1];
        for (int i = 0; i < blist.count(); i++)
            wlist[i] = blist[i].data();
        wlist[words.count()] = NULL;
        ecode = eb_search_multi(&book, mid, wlist);
        if (ecode != EB_SUCCESS) {
            dispError("eb_search_multi", ecode);
        }
        delete wlist;
        return ecode;
    }
    QList <EB_Hit> hitList(int max_count)
    {
        EB_Hit *harray = new EB_Hit[max_count];
        int cnt;
        QList <EB_Hit> hits;
        ecode = eb_hit_list(&book, max_count, harray, &cnt);
        if (ecode != EB_SUCCESS) {
            dispError("eb_hit_list", ecode);
        } else {
            for (int i = 0; i < cnt; i++) 
                hits << harray[i];
        }
        return hits;
    }

    // Text
    EB_Position startText()
    {
        EB_Position pos;
        ecode = eb_text(&book, &pos);
        if (ecode != EB_SUCCESS) {
            dispError("eb_text", ecode);
            return invalidPosition();
        }
        return pos;
    }
    EB_Error_Code seekText(const EB_Position &pos)
    {
        ecode = eb_seek_text(&book, &pos);
        if (ecode != EB_SUCCESS) {
            dispError("eb_text", ecode);
        }
        return ecode;
    }
    QString readText(bool hook_flag=true)
    {
        char buff[1024+1];
        ssize_t len;
        QByteArray b;
        for(;;) {
            if (hook_flag) {
                ecode = eb_read_text(&book, &appendix, &hookset, (void*)this,
                                1024, buff, &len);
            } else {
                ecode = eb_read_text(&book, &appendix, NULL, (void*)this,
                                1024, buff, &len);
            }
            if (ecode != EB_SUCCESS) {
                dispError("eb_read_text", ecode);
                break;
            }
            if (len > 0) {
                b += QByteArray(buff, (int)len);
            }
            if (isTextStopped()) {
                break;
            }
            if (len < 1024) {
                break;
            }
        }
        return toUTF(b);

    }
    QString readHeading(bool hook_flag=true)
    {
        char buff[1024+1];
        ssize_t len;
        QByteArray b;
        for(;;) {
            if (hook_flag) {
                ecode = eb_read_heading(&book, &appendix, &hookset, (void*)this,
                                1024, buff, &len);
            } else {
                ecode = eb_read_heading(&book, &appendix, NULL, (void*)this,
                                1024, buff, &len);
            }
            if (ecode != EB_SUCCESS) {
                dispError("eb_read_heading", ecode);
                break;
            }
            if (len > 0) {
                b += QByteArray(buff, (int)len);
            }
            if (isTextStopped()) {
                break;
            }
            if (len < 1024) {
                break;
            }
        }
        return toUTF(b);

    }
    EB_Error_Code forwardText()
    {
        ecode = eb_forward_text(&book, &appendix);
        if (ecode != EB_SUCCESS && ecode != EB_ERR_END_OF_CONTENT) {
            dispError("eb_forward_text", ecode);
        }
        return ecode;
    }
    EB_Error_Code backwardText()
    {
        ecode = eb_backward_text(&book, &appendix);
        if (ecode != EB_SUCCESS && ecode != EB_ERR_END_OF_CONTENT) {
            dispError("eb_backward_text", ecode);
        }
        return ecode;
    }
    EB_Position copyright()
    {
        EB_Position pos;
        ecode = eb_copyright(&book, &pos);
        if (ecode != EB_SUCCESS) {
            dispError("eb_copyright", ecode);
            return invalidPosition();
        }
        return pos;
    }
    bool isHaveCopyright()
    {
        return eb_have_copyright(&book) ? true : false;
    }
    EB_Position menu()
    {
        EB_Position pos;
        ecode = eb_menu(&book, &pos);
        if (ecode != EB_SUCCESS) {
            dispError("eb_menu", ecode);
            return invalidPosition();
        }
        return pos;
    }
    bool isHaveMenu()
    {
        return eb_have_menu(&book) ? true : false;
    }
    EB_Position tellText()
    {
        EB_Position pos;
        ecode = eb_tell_text(&book, &pos);
        if (ecode != EB_SUCCESS) {
            dispError("eb_tell_text", ecode);
            return invalidPosition();
        }
        return pos;
    }
    bool isTextStopped()
    {
        return eb_is_text_stopped(&book) ? true : false;
    }
    EB_Error_Code writeTextByte1(int byte1)
    {
        ecode = eb_write_text_byte1(&book, byte1);
        if (ecode != EB_SUCCESS) {
            dispError("eb_write_text_byte1", ecode);
        }
        return ecode;
    }
    EB_Error_Code writeTextByte2(int byte1, int byte2)
    {
        ecode = eb_write_text_byte2(&book, byte1, byte2);
        if (ecode != EB_SUCCESS) {
            dispError("eb_write_text_byte2", ecode);
        }
        return ecode;
    }
    EB_Error_Code writeTextString(const QString &s)
    {
        ecode = eb_write_text_string(&book, utfToEuc(s));
        if (ecode != EB_SUCCESS) {
            dispError("eb_write_text_string", ecode);
        }
        return ecode;
    }
    EB_Error_Code writeText(const QString &s)
    {
        QByteArray a = utfToEuc(s);
        ecode = eb_write_text(&book, a.data(), a.length());
        if (ecode != EB_SUCCESS) {
            dispError("eb_write_text", ecode);
        }
        return ecode;
    }
    QString currentCandidate()
    {
        const char *s = eb_current_candidate(&book);
        if (characterCode() == EB_CHARCODE_ISO8859_1) {
            return QString::fromLatin1(s);
        } else {
            return toUTF(s);
        }
    }
    

    // Initialize Hook
    void initializeHookset()
    {
        eb_initialize_hookset(&hookset);
    }
    void finalizeHookset()
    {
        eb_finalize_hookset(&hookset);
    }
    EB_Error_Code setHook(const EB_Hook hook)
    {
        ecode = eb_set_hook(&hookset, &hook);
        if (ecode != EB_SUCCESS) {
            dispError("eb_set_hooks", ecode);
        }
        return ecode;
    }
    EB_Error_Code setHooks(const EB_Hook *hooks)
    {
        ecode = eb_set_hooks(&hookset, hooks);
        if (ecode != EB_SUCCESS) {
            dispError("eb_set_hooks", ecode);
        }
        return ecode;
    }

    // Fonts
    EB_Font_Code font()
    {
        EB_Font_Code fcode;
        ecode = eb_font(&book, &fcode);
        if (ecode != EB_SUCCESS) {
            dispError("eb_font", ecode);
            // return EB_FONT_INVALID;
        }
        return fcode;
    }
    EB_Error_Code setFont(EB_Font_Code font)
    {
        ecode = eb_set_font(&book, font);
        if (ecode != EB_SUCCESS) {
            dispError("eb_set_font", ecode);
            fontCode = EB_FONT_INVALID;
        } else {
            fontCode = font;
        }
        return ecode;
    }
    void unsetFont()
    {
        eb_unset_font(&book);
    }
    QList <EB_Font_Code> fontList()
    {
        QList <EB_Font_Code> flist;
        EB_Font_Code fonts[EB_MAX_FONTS];
        int cnt;
        ecode = eb_font_list(&book, fonts, &cnt);
        if (ecode != EB_SUCCESS) {
            dispError("eb_font_list", ecode);
        } else {
            for (int i = 0; i < cnt; i++)
                flist << fonts[i];
        }
        return flist;
    }
    bool isHaveFont(EB_Font_Code font)
    {
        return eb_have_font(&book, font) ? true : false;
    }
    int fontHeight()
    {
        int height;
        ecode = eb_font_height(&book, &height);
        if (ecode != EB_SUCCESS) {
            dispError("eb_font_height", ecode);
        }
        return height;
    }
    int fontHeight(EB_Font_Code font)
    {
        int height;
        ecode = eb_font_height2(font, &height);
        if (ecode != EB_SUCCESS) {
            dispError("eb_font_height2", ecode);
        }
        return height;
    }
    bool isHaveNarrowFont()
    {
        return eb_have_narrow_font(&book) ? true : false;
    }
    bool isHaveWideFont()
    {
        return eb_have_wide_font(&book) ? true : false;
    }
    int narrowFontWidth()
    {
        int width;
        ecode = eb_narrow_font_width(&book, &width);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_font_width", ecode);
        }
        return width;
    }
    int wideFontWidth()
    {
        int width;
        ecode = eb_wide_font_width(&book, &width);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_font_width", ecode);
        }
        return width;
    }
    int wideFontWidth(EB_Font_Code font)
    {
        int width;
        ecode = eb_wide_font_width2(font, &width);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_font_width2", ecode);
        }
        return width;
    }
    int narrowFontWidth(EB_Font_Code font)
    {
        int width;
        ecode = eb_narrow_font_width2(font, &width);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_font_width", ecode);
        }
        return width;
    }
    int wideFontSize()
    {
        size_t size;
        ecode = eb_wide_font_size(&book, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_font_size", ecode);
        }
        return (int)size;
    }
    int narrowFontSize()
    {
        size_t size;
        ecode = eb_narrow_font_size(&book, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_font_size", ecode);
        }
        return (int)size;
    }
    int wideFontSize(EB_Font_Code font)
    {
        size_t size;
        ecode = eb_wide_font_size2(font, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_font_size2", ecode);
        }
        return (int)size;
    }
    int narrowFontSize(EB_Font_Code font)
    {
        size_t size;
        ecode = eb_narrow_font_size2(font, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_font_size2", ecode);
        }
        return (int)size;
    }
    int wideFontStart()
    {
        int start;
        ecode = eb_wide_font_start(&book, &start);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_font_start", ecode);
            return -1;
        }
        return start;
    }
    int narrowFontStart()
    {
        int start;
        ecode = eb_narrow_font_start(&book, &start);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_font_start", ecode);
            return -1;
        }
        return start;
    }
    int wideFontEnd()
    {
        int end;
        ecode = eb_wide_font_end(&book, &end);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_font_end", ecode);
            return -1;
        }
        return end;
    }
    int narrowFontEnd()
    {
        int end;
        ecode = eb_narrow_font_end(&book, &end);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_font_end", ecode);
            return -1;
        }
        return end;
    }
    QByteArray narrowFontCharacterBitmap(int character_number)
    {
        int size;
        switch(font()) {
            case EB_FONT_16:
                size = EB_SIZE_NARROW_FONT_16;
                break;
            case EB_FONT_24:
                size = EB_SIZE_NARROW_FONT_24;
                break;
            case EB_FONT_30:
                size = EB_SIZE_NARROW_FONT_30;
                break;
            case EB_FONT_48:
                size = EB_SIZE_NARROW_FONT_48;
                break;
            default:
                return QByteArray();
        }
        char *bits = new char[size];

        ecode = eb_narrow_font_character_bitmap(&book, character_number, bits);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_font_character_bitmap", ecode);
            delete bits;
            return QByteArray();
        }

        QByteArray b(bits, size);
        delete bits;
        return b;
    }
    QByteArray wideFontCharacterBitmap(int character_number)
    {
        int size;
        switch(font()) {
            case EB_FONT_16:
                size = EB_SIZE_WIDE_FONT_16;
                break;
            case EB_FONT_24:
                size = EB_SIZE_WIDE_FONT_24;
                break;
            case EB_FONT_30:
                size = EB_SIZE_WIDE_FONT_30;
                break;
            case EB_FONT_48:
                size = EB_SIZE_WIDE_FONT_48;
                break;
            default:
                return QByteArray();
        }
        char *bits = new char[size];

        ecode = eb_wide_font_character_bitmap(&book, character_number, bits);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_font_character_bitmap", ecode);
            delete bits;
            return QByteArray();
        }

        QByteArray b(bits, size);
        delete bits;
        return b;
    }
    int forwardNarrowFontCharacter(int n, int c_number)
    {
        int f_number = c_number;
        ecode = eb_forward_narrow_font_character(&book, n, &f_number);
        if (ecode != EB_SUCCESS) {
            dispError("eb_forward_narrow_font_character", ecode);
            return -1;
        }
        return f_number;
    }
    int forwardWideFontCharacter(int n, int c_number)
    {
        int f_number = c_number;
        ecode = eb_forward_wide_font_character(&book, n, &f_number);
        if (ecode != EB_SUCCESS) {
            dispError("eb_forward_wide_font_character", ecode);
            return -1;
        }
        return f_number;
    }
    int backwardNarrowFontCharacter(int n, int c_number)
    {
        int b_number = c_number;
        ecode = eb_backward_narrow_font_character(&book, n, &b_number);
        if (ecode != EB_SUCCESS) {
            dispError("eb_backward_narrow_font_character", ecode);
            return -1;
        }
    }
    int backwardWideFontCharacter(int n, int c_number)
    {
        int b_number = c_number;
        ecode = eb_backward_wide_font_character(&book, n, &b_number);
        if (ecode != EB_SUCCESS) {
            dispError("eb_backward_wide_font_character", ecode);
            return -1;
        }
        return b_number;
    }
    int narrowFontXbmSize(EB_Font_Code height)
    {
        size_t size;
        ecode = eb_narrow_font_xbm_size(height, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_font_xbm_size", ecode);
        }
        return (int)size;
    }
    int narrowFontXpmSize(EB_Font_Code height)
    {
        size_t size;
        ecode = eb_narrow_font_xpm_size(height, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_font_xpm_size", ecode);
        }
        return (int)size;
    }
    int narrowFontGifSize(EB_Font_Code height)
    {
        size_t size;
        ecode = eb_narrow_font_gif_size(height, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_font_gif_size", ecode);
        }
        return (int)size;
    }
    int narrowFontBmpSize(EB_Font_Code height)
    {
        size_t size;
        ecode = eb_narrow_font_bmp_size(height, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_font_bmp_size", ecode);
        }
        return (int)size;
    }
    int narrowFontPngSize(EB_Font_Code height)
    {
        size_t size;
        ecode = eb_narrow_font_png_size(height, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_font_png_size", ecode);
        }
        return (int)size;
    }
    int wideFontXbmSize(EB_Font_Code height)
    {
        size_t size;
        ecode = eb_wide_font_xbm_size(height, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_font_xbm_size", ecode);
        }
        return (int)size;
    }
    int wideFontXpmSize(EB_Font_Code height)
    {
        size_t size;
        ecode = eb_wide_font_xpm_size(height, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_font_xpm_size", ecode);
        }
        return (int)size;
    }
    int wideFontGifSize(EB_Font_Code height)
    {
        size_t size;
        ecode = eb_wide_font_gif_size(height, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_font_gif_size", ecode);
        }
        return (int)size;
    }
    int wideFontBmpSize(EB_Font_Code height)
    {
        size_t size;
        ecode = eb_wide_font_bmp_size(height, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_font_bmp_size", ecode);
        }
        return (int)size;
    }
    int wideFontPngSize(EB_Font_Code height)
    {
        size_t size;
        ecode = eb_wide_font_png_size(height, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_font_png_size", ecode);
        }
        return (int)size;
    }
    QSize narrowFontQSize()
    {
        QSize sz;
        switch(font()) {
            case EB_FONT_16:
                sz = QSize(8,16);
                break;
            case EB_FONT_24:
                sz = QSize(12,24);
                break;
            case EB_FONT_30:
                sz = QSize(15,30);
                break;
            case EB_FONT_48:
                sz = QSize(24,48);
                break;
            default:
                sz = QSize(0,0);
        }
        return sz;
    }

    QSize wideFontQSize()
    {
        QSize sz;
        switch(font()) {
            case EB_FONT_16:
                sz = QSize(16,16);
                break;
            case EB_FONT_24:
                sz = QSize(24,24);
                break;
            case EB_FONT_30:
                sz = QSize(30,30);
                break;
            case EB_FONT_48:
                sz = QSize(48,48);
                break;
            default:
                sz = QSize(0,0);
        }
        return sz;
    }
    QByteArray narrowBitmapToXbm(const QByteArray &bitmap)
    {
        QSize sz = narrowFontQSize();
        QByteArray b;
        if (sz.width() == 0) 
            return b;

        int image_size= narrowFontXbmSize(font());
        char *buff = new char[image_size];
        size_t size;
        ecode = eb_bitmap_to_xbm(bitmap, sz.width(), sz.height(), buff, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_bitmap_to_xbm", ecode);
        } else {
            b = QByteArray(buff, (int)size);
        }
        delete buff;
        return b;
    }
    QByteArray wideBitmapToXbm(const QByteArray &bitmap)
    {
        QSize sz = wideFontQSize();
        QByteArray b;
        if (sz.width() == 0) 
            return b;

        int image_size= wideFontXbmSize(font());
        char *buff = new char[image_size];
        size_t size;
        ecode = eb_bitmap_to_xbm(bitmap, sz.width(), sz.height(), buff, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_bitmap_to_xbm", ecode);
        } else {
            b = QByteArray(buff, (int)size);
        }
        delete buff;
        return b;
    }
    QByteArray narrowBitmapToXpm(const QByteArray &bitmap)
    {
        QSize sz = narrowFontQSize();
        QByteArray b;
        if (sz.width() == 0) 
            return b;

        int image_size= narrowFontXpmSize(font());
        char *buff = new char[image_size];
        size_t size;
        ecode = eb_bitmap_to_xpm(bitmap, sz.width(), sz.height(), buff, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_bitmap_to_xpm", ecode);
        } else {
            b = QByteArray(buff, (int)size);
        }
        delete buff;
        return b;
    }
    QByteArray wideBitmapToXpm(const QByteArray &bitmap)
    {
        QSize sz = wideFontQSize();
        QByteArray b;
        if (sz.width() == 0) 
            return b;

        int image_size= wideFontXpmSize(font());
        char *buff = new char[image_size];
        size_t size;
        ecode = eb_bitmap_to_xpm(bitmap, sz.width(), sz.height(), buff, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_bitmap_to_xpm", ecode);
        } else {
            b = QByteArray(buff, (int)size);
        }
        delete buff;
        return b;
    }
    QByteArray narrowBitmapToGif(const QByteArray &bitmap)
    {
        QSize sz = narrowFontQSize();
        QByteArray b;
        if (sz.width() == 0) 
            return b;

        int image_size= narrowFontGifSize(font());
        char *buff = new char[image_size];
        size_t size;
        ecode = eb_bitmap_to_gif(bitmap, sz.width(), sz.height(), buff, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_bitmap_to_gif", ecode);
        } else {
            b = QByteArray(buff, (int)size);
        }
        delete buff;
        return b;
    }
    QByteArray wideBitmapToGif(const QByteArray &bitmap)
    {
        QSize sz = wideFontQSize();
        QByteArray b;
        if (sz.width() == 0) 
            return b;

        int image_size= wideFontGifSize(font());
        char *buff = new char[image_size];
        size_t size;
        ecode = eb_bitmap_to_gif(bitmap, sz.width(), sz.height(), buff, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_bitmap_to_gif", ecode);
        } else {
            b = QByteArray(buff, (int)size);
        }
        delete buff;
        return b;
    }
    QByteArray narrowBitmapToBmp(const QByteArray &bitmap)
    {
        QSize sz = narrowFontQSize();
        QByteArray b;
        if (sz.width() == 0) 
            return b;

        int image_size= narrowFontBmpSize(font());
        char *buff = new char[image_size];
        size_t size;
        ecode = eb_bitmap_to_bmp(bitmap, sz.width(), sz.height(), buff, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_bitmap_to_bmp", ecode);
        } else {
            b = QByteArray(buff, (int)size);
        }
        delete buff;
        return b;
    }
    QByteArray wideBitmapToBmp(const QByteArray &bitmap)
    {
        QSize sz = wideFontQSize();
        QByteArray b;
        if (sz.width() == 0) 
            return b;

        int image_size= wideFontBmpSize(font());
        char *buff = new char[image_size];
        size_t size;
        ecode = eb_bitmap_to_bmp(bitmap, sz.width(), sz.height(), buff, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_bitmap_to_bmp", ecode);
        } else {
            b = QByteArray(buff, (int)size);
        }
        delete buff;
        return b;
    }
    QByteArray narrowBitmapToPng(const QByteArray &bitmap)
    {
        QSize sz = narrowFontQSize();
        QByteArray b;
        if (sz.width() == 0) 
            return b;

        int image_size= narrowFontPngSize(font());
        char *buff = new char[image_size];
        size_t size;
        ecode = eb_bitmap_to_png(bitmap, sz.width(), sz.height(), buff, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_bitmap_to_png", ecode);
        } else {
            b = QByteArray(buff, (int)size);
        }
        delete buff;
        return b;
    }
    QByteArray wideBitmapToPng(const QByteArray &bitmap)
    {
        QSize sz = wideFontQSize();
        QByteArray b;
        if (sz.width() == 0) 
            return b;

        int image_size= wideFontPngSize(font());
        char *buff = new char[image_size];
        size_t size;
        ecode = eb_bitmap_to_png(bitmap, sz.width(), sz.height(), buff, &size);
        if (ecode != EB_SUCCESS) {
            dispError("eb_bitmap_to_png", ecode);
        } else {
            b = QByteArray(buff, (int)size);
        }
        delete buff;
        return b;
    }

    // Graphics, Sounds, Movies
    EB_Error_Code setBinaryMonoGraphic(const EB_Position &position, int width,
                                       int height)
    {
        ecode = eb_set_binary_mono_graphic(&book, &position, width, height);
        if (ecode != EB_SUCCESS) {
            dispError("eb_set_binary_mono_graphic", ecode);
        }
        return ecode;
    }
    EB_Error_Code setBinaryColorGraphic(const EB_Position &position)
    {
        ecode = eb_set_binary_color_graphic(&book, &position);
        if (ecode != EB_SUCCESS) {
            dispError("eb_set_binary_color_graphic", ecode);
        }
        return ecode;
    }
    EB_Error_Code setBinaryWave(const EB_Position &start, EB_Position &end)
    {
        ecode = eb_set_binary_wave(&book, &start, &end);
        if (ecode != EB_SUCCESS) {
            dispError("eb_set_binary_wave", ecode);
        }
        return ecode;
    }
    EB_Error_Code setBinaryMpeg(const unsigned int *argv)
    {
        ecode = eb_set_binary_mpeg(&book, argv);
        if (ecode != EB_SUCCESS) {
            dispError("eb_set_binary_mpeg", ecode);
        }
        return ecode;
    }
    QByteArray readBinary()
    {
        char buff[1024];
        ssize_t len;
        QByteArray b;
        for(;;) {
            ecode = eb_read_binary(&book, 1024, buff, &len);
            if (ecode != EB_SUCCESS) {
                dispError("eb_", ecode);
                return b;
            }
            if (len > 0 )
                b += QByteArray(buff, (int)len);
            if (len < 1024)
                break;
        }
        return b;
    }
    QString composeMovieFileName(const unsigned int *argv)
    {
        char name[EB_MAX_DIRECTORY_NAME_LENGTH+1];
        ecode = eb_compose_movie_file_name(argv, name);
        if (ecode != EB_SUCCESS) {
            dispError("eb_compose_movie_file_name", ecode);
            return QString();
        }
        return toUTF(name);
    }
    QString composeMoviePathName(const unsigned int *argv)
    {
        char path[EB_MAX_PATH_LENGTH+1];
        ecode = eb_compose_movie_path_name(&book, argv, path);
        if (ecode != EB_SUCCESS) {
            dispError("eb_compose_movie_path_name", ecode);
            return QString();
        }
        return toUTF(path);
    }
    EB_Error_Code decomposeMovieFileName(unsigned int *argv,
                                         const QString &name)
    {
        ecode = eb_decompose_movie_file_name(argv, utfToEuc(name));
        if (ecode != EB_SUCCESS) {
            dispError("eb_decompose_movie_file_name", ecode);
        }
        return ecode;
    }

    // Appendix
    void initializeAppendix()
    {
        eb_initialize_appendix(&appendix);
    }
    void finalizeAppendix()
    {
        eb_finalize_appendix(&appendix);
    }
    EB_Error_Code bindAppendix(QString path)
    {
        ecode = eb_bind_appendix(&appendix, path.toLocal8Bit());
        if (ecode != EB_SUCCESS) {
            dispError("eb_bind_appendix", ecode);
        }
        return ecode;
    }
    bool isAppendixBound()
    {
        return eb_is_appendix_bound(&appendix) ? true : false;
    }
    QString appendixPath()
    {
        char path[EB_MAX_PATH_LENGTH+1];
        ecode = eb_appendix_path(&appendix, path);
        if (ecode != EB_SUCCESS) {
            dispError("eb_appendix_path", ecode);
            return QString();
        }
        return toUTF(path);
    }
    EB_Error_Code loadAllAppendixSubbooks()
    {
        ecode = eb_load_all_appendix_subbooks(&appendix);
        if (ecode != EB_SUCCESS) {
            dispError("eb_load_all_appendix_subbooks", ecode);
        }
        return ecode;
    }
    QList <EB_Subbook_Code> appendixSubbookList()
    {
        EB_Subbook_Code code[EB_MAX_SUBBOOKS];
        int cnt;
        QList <EB_Subbook_Code> list;

        ecode = eb_appendix_subbook_list(&appendix, code, &cnt);
        if (ecode != EB_SUCCESS) {
            dispError("eb_appendix_subbook_list", ecode);
        } else {
            for (int i = 0; i < cnt; i++) 
                list << code[i];
        }
        return list;
    }
    EB_Subbook_Code appendixSubbook()
    {
        EB_Subbook_Code code;
        ecode = eb_appendix_subbook(&appendix, &code);
        if (ecode != EB_SUCCESS) {
            dispError("eb_appendix_subbook", ecode);
        }
        return code;
    }
    QString appendixSubbookDirectory()
    {
        char dir[EB_MAX_DIRECTORY_NAME_LENGTH+1];
        ecode = eb_appendix_subbook_directory(&appendix, dir);
        if (ecode != EB_SUCCESS) {
            dispError("eb_appendix_subbook_directory", ecode);
            return QString();
        }
        return toUTF(dir);
    }
    QString appendixSubbookDirectory(EB_Subbook_Code code)
    {
        char dir[EB_MAX_DIRECTORY_NAME_LENGTH+1];
        ecode = eb_appendix_subbook_directory2(&appendix, code, dir);
        if (ecode != EB_SUCCESS) {
            dispError("eb_appendix_subbook_directory2", ecode);
            return QString();
        }
        return toUTF(dir);
    }
    EB_Error_Code setAppendixSubbook(EB_Subbook_Code code)
    {
        ecode = eb_set_appendix_subbook(&appendix, code);
        if (ecode != EB_SUCCESS) {
            dispError("eb_set_appendix_subbook", ecode);
        }
        return ecode;
    }
    void unsetAppendixSubbook()
    {
        eb_unset_appendix_subbook(&appendix);
    }
    bool isHaveStopCode()
    {
        return eb_have_stop_code(&appendix) ? true : false;
    }
    EB_Error_Code stopCode(int *stop1, int *stop2)
    {
        int stop_code[2];
        ecode = eb_stop_code(&appendix, stop_code);
        if (ecode != EB_SUCCESS) {
            dispError("eb_stop_code", ecode);
        }
        *stop1 = stop_code[0];
        *stop2 = stop_code[1];
        return ecode;
    }
    bool haveNarrowAlt()
    {
        return eb_have_narrow_alt(&appendix) ? true : false;
    }
    bool haveWideAlt()
    {
        return eb_have_wide_alt(&appendix) ? true : false;
    }
    int narrowAltStart()
    {
        int start;
        ecode = eb_narrow_alt_start(&appendix, &start);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_alt_start", ecode);
            return -1;
        }
        return start;
    }
    int wideAltStart()
    {
        int start;
        ecode = eb_wide_alt_start(&appendix, &start);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_alt_start", ecode);
            return -1;
        }
        return start;
    }
    int narrowAltEnd()
    {
        int end;
        ecode = eb_narrow_alt_end(&appendix, &end);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_alt_end", ecode);
            return -1;
        }
        return end;
    }
    int wideAltEnd()
    {
        int end;
        ecode = eb_wide_alt_end(&appendix, &end);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_alt_end", ecode);
            return -1;
        }
        return end;
    }
    QString narrowAltCharacterText(int character_number)
    {
        char alt[EB_MAX_ALTERNATION_TEXT_LENGTH+1];
        ecode = eb_narrow_alt_character_text(&appendix, character_number, alt);
        if (ecode != EB_SUCCESS) {
            dispError("eb_narrow_alt_character_text", ecode);
            return QString();
        }
        if (characterCode() == EB_CHARCODE_ISO8859_1) {
            return QString::fromLatin1(alt);
        } else {
            return toUTF(alt);
        }
    }
    QString wideAltCharacterText(int character_number)
    {
        char alt[EB_MAX_ALTERNATION_TEXT_LENGTH+1];
        ecode = eb_wide_alt_character_text(&appendix, character_number, alt);
        if (ecode != EB_SUCCESS) {
            dispError("eb_wide_alt_character_text", ecode);
            return QString();
        }
        if (characterCode() == EB_CHARCODE_ISO8859_1) {
            return QString::fromLatin1(alt);
        } else {
            return toUTF(alt);
        }
    }
    int backwardNarrowAltCharacter(int n, int character_number)
    {
        int next_number = character_number;
        ecode = eb_backward_narrow_alt_character(&appendix, n, &next_number);
        if (ecode != EB_SUCCESS) {
            dispError("eb_backward_narrow_alt_character", ecode);
            return -1;
        }
        return next_number;
    }
    int forwardNarrowAltCharacter(int n, int character_number)
    {
        int next_number = character_number;
        ecode = eb_forward_narrow_alt_character(&appendix, n, &next_number);
        if (ecode != EB_SUCCESS) {
            dispError("eb_forward_narrow_alt_character", ecode);
            return -1;
        }
        return next_number;
    }
    int backwardWideAltCharacter(int n, int character_number)
    {
        int next_number = character_number;
        ecode = eb_backward_wide_alt_character(&appendix, n, &next_number);
        if (ecode != EB_SUCCESS) {
            dispError("eb_backward_wide_alt_character", ecode);
            return -1;
        }
        return next_number;
    }
    int forwardWideAltCharacter(int n, int character_number)
    {
        int next_number = character_number;
        ecode = eb_forward_wide_alt_character(&appendix, n, &next_number);
        if (ecode != EB_SUCCESS) {
            dispError("eb_forward_wide_alt_character", ecode);
            return -1;
        }
        return next_number;
    }

    // Book List
    void initializeBooklist()
    {
        eb_initialize_booklist(&bookList);
    }
    EB_Error_Code  bindBooklist(const QString &path)
    {
        ecode = eb_bind_booklist(&bookList, utfToEuc(path));
        if (ecode != EB_SUCCESS) {
            dispError("eb_bind_boollist", ecode);
        }
        return ecode;
    }
    void finalizeBooklist()
    {
        eb_finalize_booklist(&bookList);
    }
    EB_Error_Code booklistBookCount()
    {
        int cnt;
        ecode = eb_booklist_book_count(&bookList, &cnt);
        if (ecode != EB_SUCCESS) {
            dispError("eb_bind_boollist", ecode);
            return -1;
        }
        return cnt;
    }
    QString booklistBookName(int idx)
    {
        char *name;
        ecode = eb_booklist_book_name(&bookList, idx, &name);
        if (ecode != EB_SUCCESS) {
            dispError("eb_booklist_book_name", ecode);
            return QString();
        }
        return toUTF(name);
    }
    QString booklistBookTitle(int idx)
    {
        char *title;
        ecode = eb_booklist_book_title(&bookList, idx, &title);
        if (ecode != EB_SUCCESS) {
            dispError("eb_booklist_book_title", ecode);
            return QString();
        }
        return toUTF(title);
    }

};

#endif
