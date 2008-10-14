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

#include "qeb.h"
#include "textcodec.h"

EB_Error_Code QEb::bind(const QString &path)
{
    EB_Error_Code ecode = eb_bind(&book, path.toLocal8Bit());
    if (ecode != EB_SUCCESS)
        dispError("eb_bind", ecode);
    return ecode;
}
QString QEb::path()
{
    char s[EB_MAX_PATH_LENGTH+1];
    EB_Error_Code ecode = eb_path(&book, s);
    if (ecode != EB_SUCCESS) {
        dispError("eb_path", ecode);
        return QString();
    }
    return QString::fromLocal8Bit(s);
}
EB_Disc_Code QEb::discType()
{
    EB_Disc_Code dcode;
    EB_Error_Code ecode = eb_disc_type(&book, &dcode);
    if (ecode != EB_SUCCESS) {
        dispError("eb_disc_type", ecode);
        return EB_DISC_INVALID;
    }
    return dcode;
}
EB_Character_Code QEb::characterCode()
{
    EB_Character_Code ccode;
    EB_Error_Code ecode = eb_character_code(&book, &ccode);
    if (ecode != EB_SUCCESS) {
        dispError("eb_character_code", ecode);
        return EB_CHARCODE_INVALID;
    }
    return ccode;
}

// Initialize Subbook
QList <EB_Subbook_Code> QEb::subbookList()
{
    EB_Subbook_Code codes[EB_MAX_SUBBOOKS];
    int cnt;

    QList <EB_Subbook_Code> list;
    EB_Error_Code ecode = eb_subbook_list(&book, codes, &cnt);
    if (ecode != EB_SUCCESS)
        dispError("eb_subbook_list", ecode);
    else
        for(int i = 0; i < cnt; i++)
            list << codes[i];
    return list;
}
EB_Error_Code QEb::loadAllSubbooks()
{
    EB_Error_Code ecode = eb_load_all_subbooks(&book);
    if (ecode != EB_SUCCESS)
        dispError("eb_subbook_list", ecode);
    return ecode;
}
EB_Subbook_Code QEb::subbook()
{
    EB_Subbook_Code code;
    EB_Error_Code ecode = eb_subbook(&book, &code);
    if (ecode != EB_SUCCESS)
        dispError("eb_subbook_list", ecode);
    return code;
}
    
QString QEb::subbookTitle()
{
    char title[EB_MAX_TITLE_LENGTH+1];
    EB_Error_Code ecode = eb_subbook_title(&book, title);
    if (ecode != EB_SUCCESS) {
        dispError("eb_subbook_title", ecode);
        return QString();
    }
    return eucToUtf(title);
}

QString QEb::subbookTitle2(EB_Subbook_Code code)
{
    char title[EB_MAX_TITLE_LENGTH+1];
    EB_Error_Code ecode = eb_subbook_title2(&book, code, title);
    if (ecode != EB_SUCCESS) {
        dispError("eb_subbook_title2", ecode);
        return QString();
    }
    return eucToUtf(title);
}
QString QEb::subbookDirectory()
{
    char dir[EB_MAX_DIRECTORY_NAME_LENGTH+1];
    EB_Error_Code ecode = eb_subbook_directory(&book, dir);
    if (ecode != EB_SUCCESS) {
        dispError("eb_subbook_directory", ecode);
        return QString();
    }
    return eucToUtf(dir);
}
QString QEb::subbookDirectory2(EB_Subbook_Code code)
{
    char dir[EB_MAX_DIRECTORY_NAME_LENGTH+1];
    EB_Error_Code ecode = eb_subbook_directory2(&book, code, dir);
    if (ecode != EB_SUCCESS) {
        dispError("eb_subbook_directory2", ecode);
        return QString();
    }
    return eucToUtf(dir);
}
EB_Error_Code QEb::setSubbook(EB_Subbook_Code code)
{
    EB_Error_Code ecode = eb_set_subbook(&book, code);
    if (ecode != EB_SUCCESS)
        dispError("eb_set_subbook", ecode);
    return ecode;
}

QList <EB_Multi_Search_Code> QEb::multiSearchList()
{
    EB_Multi_Search_Code codes[EB_MAX_MULTI_SEARCHES];
    int cnt;
    QList <EB_Multi_Search_Code> list;

    EB_Error_Code ecode = eb_multi_search_list(&book, codes,  &cnt);
    if (ecode != EB_SUCCESS)
        dispError("eb_multi_search_list", ecode);
    else
        for (int i = 0; i < cnt; i++)
            list << codes[i];
    return list;
}
int QEb::multiEntryCount(EB_Multi_Search_Code mid)
{
    int cnt;
    EB_Error_Code ecode = eb_multi_entry_count(&book, mid, &cnt);
    if (ecode != EB_SUCCESS) {
        dispError("eb_multi_search_list", ecode);
        return 0;
    }
    return cnt;
}
QString QEb::multiTitle(EB_Multi_Search_Code mid)
{
    char s[EB_MAX_MULTI_TITLE_LENGTH+1];
    EB_Error_Code ecode = eb_multi_title(&book, mid, s);
    if (ecode != EB_SUCCESS) {
        dispError("eb_multi_title", ecode);
        return QString();
    }
    if (characterCode() == EB_CHARCODE_ISO8859_1)
        return QString::fromLatin1(s);
    else
        return eucToUtf(s);
}
QString QEb::multiEntryLabel(EB_Multi_Search_Code mid, int entry)
{
    char s[EB_MAX_MULTI_LABEL_LENGTH+1];
    EB_Error_Code ecode = eb_multi_entry_label(&book, mid, entry, s);
    if (ecode != EB_SUCCESS) {
        dispError("eb_multi_entry_label", ecode);
        return QString();
    }
    if (characterCode() == EB_CHARCODE_ISO8859_1)
        return QString::fromLatin1(s);
    else
        return eucToUtf(s);
    
}
EB_Position QEb::multiEntryCandidates(EB_Multi_Search_Code mid, int entry)
{
    EB_Position pos;
    EB_Error_Code ecode = eb_multi_entry_candidates(&book, mid, entry, &pos);
    if (ecode != EB_SUCCESS) {
        dispError("eb_multi_entry_candidates", ecode);
        return invalidPosition();
    }
    return pos;
}
EB_Error_Code QEb::searchWord(const QString &word)
{
    EB_Error_Code ecode = eb_search_word(&book, utfToEuc(word));
    if (ecode != EB_SUCCESS)
        dispError("eb_search_word", ecode);
    return ecode;
}
EB_Error_Code QEb::searchEndword(const QString &word)
{
    EB_Error_Code ecode = eb_search_endword(&book, utfToEuc(word));
    if (ecode != EB_SUCCESS)
        dispError("eb_search_endword", ecode);
    return ecode;
}
EB_Error_Code QEb::searchExactword(const QString &word)
{
    EB_Error_Code ecode = eb_search_exactword(&book, utfToEuc(word));
    if (ecode != EB_SUCCESS)
        dispError("eb_search_exactword", ecode);
    return ecode;
}
QList <QByteArray> QEb::toEucList(const QStringList &words)
{
    QList <QByteArray> blist;
    for (int i = 0; i < words.count(); i++)
        blist << utfToEuc(words[i]);
    return blist;
}
EB_Error_Code QEb::searchKeyword(const QStringList &words)
{
    QList <QByteArray> blist = toEucList(words);
    char** wlist = new char*[words.count()+1];
    for (int i = 0; i < blist.count(); i++)
        wlist[i] = blist[i].data();
    wlist[words.count()] = NULL;

    EB_Error_Code ecode = eb_search_keyword(&book, wlist);
    if (ecode != EB_SUCCESS)
        dispError("eb_search_keyword", ecode);
    delete wlist;
    return ecode;
}
EB_Error_Code QEb::searchCross(const QStringList &words)
{
    QList <QByteArray> blist = toEucList(words);
    char** wlist = new char*[words.count()+1];
    for (int i = 0; i < blist.count(); i++)
        wlist[i] = blist[i].data();
    wlist[words.count()] = NULL;
    EB_Error_Code ecode = eb_search_cross(&book, wlist);
    if (ecode != EB_SUCCESS)
        dispError("eb_search_cross", ecode);
    delete wlist;
    return ecode;
}
EB_Error_Code QEb::searchMulti(EB_Multi_Search_Code mid,
        const QStringList &words)
{
    QList <QByteArray> blist = toEucList(words);
    char** wlist = new char*[words.count()+1];
    for (int i = 0; i < blist.count(); i++)
        wlist[i] = blist[i].data();
    wlist[words.count()] = NULL;
    EB_Error_Code ecode = eb_search_multi(&book, mid, wlist);
    if (ecode != EB_SUCCESS)
        dispError("eb_search_multi", ecode);
    delete wlist;
    return ecode;
}
QList <EB_Hit> QEb::hitList(int max_count)
{
    EB_Hit *harray = new EB_Hit[max_count];
    int cnt;
    QList <EB_Hit> hits;
    EB_Error_Code ecode = eb_hit_list(&book, max_count, harray, &cnt);
    if (ecode != EB_SUCCESS)
        dispError("eb_hit_list", ecode);
    else
        for (int i = 0; i < cnt; i++) 
            hits << harray[i];
    return hits;
}


EB_Position QEb::startText()
{
    EB_Position pos;
    EB_Error_Code ecode = eb_text(&book, &pos);
    if (ecode != EB_SUCCESS) {
        dispError("eb_text", ecode);
        return invalidPosition();
    }
    return pos;
}
EB_Error_Code QEb::seekText(const EB_Position &pos)
{
    EB_Error_Code ecode = eb_seek_text(&book, &pos);
    if (ecode != EB_SUCCESS)
        dispError("eb_text", ecode);
    return ecode;
}
QString QEb::readText(void *para, bool hook_flag)
{
    char buff[1024+1];
    ssize_t len;
    QByteArray b;
    for(;;) {
        EB_Error_Code ecode;
        if (hook_flag)
            ecode = eb_read_text(&book, &appendix, &hookset, para,
                                 1024, buff, &len);
        else
            ecode = eb_read_text(&book, &appendix, NULL, para,
                                 1024, buff, &len);
        if (ecode != EB_SUCCESS) {
            dispError("eb_read_text", ecode);
            break;
        }
        if (len > 0)
            b += QByteArray(buff, (int)len);
        if (isTextStopped())
            break;
        //if (len < 1024)
        //    break;
    }
    return eucToUtf(b);

}
QString QEb::readHeading(void *para, bool hook_flag)
{
    char buff[1024+1];
    ssize_t len;
    QByteArray b;
    for(;;) {
        EB_Error_Code ecode;
        if (hook_flag)
            ecode = eb_read_heading(&book, &appendix, &hookset, para,
                                    1024, buff, &len);
        else
            ecode = eb_read_heading(&book, &appendix, NULL, para,
                                    1024, buff, &len);
        if (ecode != EB_SUCCESS) {
            dispError("eb_read_heading", ecode);
            break;
        }
        if (len > 0)
            b += QByteArray(buff, (int)len);
        if (isTextStopped())
            break;
        //if (len < 1024)
        //    break;
    }
    return eucToUtf(b);

}
EB_Error_Code QEb::forwardText()
{
    EB_Error_Code ecode = eb_forward_text(&book, &appendix);
    if (ecode != EB_SUCCESS && ecode != EB_ERR_END_OF_CONTENT)
        dispError("eb_forward_text", ecode);
    return ecode;
}
EB_Error_Code QEb::backwardText()
{
    EB_Error_Code ecode = eb_backward_text(&book, &appendix);
    if (ecode != EB_SUCCESS && ecode != EB_ERR_END_OF_CONTENT)
        dispError("eb_backward_text", ecode);
    return ecode;
}
EB_Position QEb::copyright()
{
    EB_Position pos;
    EB_Error_Code ecode = eb_copyright(&book, &pos);
    if (ecode != EB_SUCCESS) {
        dispError("eb_copyright", ecode);
        return invalidPosition();
    }
    return pos;
}
EB_Position QEb::menu()
{
    EB_Position pos;
    EB_Error_Code ecode = eb_menu(&book, &pos);
    if (ecode != EB_SUCCESS) {
        dispError("eb_menu", ecode);
        return invalidPosition();
    }
    return pos;
}
EB_Position QEb::tellText()
{
    EB_Position pos;
    EB_Error_Code ecode = eb_tell_text(&book, &pos);
    if (ecode != EB_SUCCESS) {
        dispError("eb_tell_text", ecode);
        return invalidPosition();
    }
    return pos;
}
EB_Error_Code QEb::writeTextByte1(int byte1)
{
    EB_Error_Code ecode = eb_write_text_byte1(&book, byte1);
    if (ecode != EB_SUCCESS)
        dispError("eb_write_text_byte1", ecode);
    return ecode;
}
EB_Error_Code QEb::writeTextByte2(int byte1, int byte2)
{
    EB_Error_Code ecode = eb_write_text_byte2(&book, byte1, byte2);
    if (ecode != EB_SUCCESS)
        dispError("eb_write_text_byte2", ecode);
    return ecode;
}
EB_Error_Code QEb::writeTextString(const QString &s)
{
    EB_Error_Code ecode = eb_write_text_string(&book, utfToEuc(s));
    if (ecode != EB_SUCCESS)
        dispError("eb_write_text_string", ecode);
    return ecode;
}
EB_Error_Code QEb::writeText(const QString &s)
{
    QByteArray a = utfToEuc(s);
    EB_Error_Code ecode = eb_write_text(&book, a.data(), a.length());
    if (ecode != EB_SUCCESS)
        dispError("eb_write_text", ecode);
    return ecode;
}
QString QEb::currentCandidate()
{
    const char *s = eb_current_candidate(&book);
    if (characterCode() == EB_CHARCODE_ISO8859_1)
        return QString::fromLatin1(s);
    else
        return eucToUtf(s);
}

EB_Error_Code QEb::setHook(const EB_Hook hook)
{
    EB_Error_Code ecode = eb_set_hook(&hookset, &hook);
    if (ecode != EB_SUCCESS)
        dispError("eb_set_hooks", ecode);
    return ecode;
}
EB_Error_Code QEb::setHooks(const EB_Hook *hooks)
{
    EB_Error_Code ecode = eb_set_hooks(&hookset, hooks);
    if (ecode != EB_SUCCESS)
        dispError("eb_set_hooks", ecode);
    return ecode;
}


EB_Font_Code QEb::font()
{
    EB_Font_Code fcode;
    EB_Error_Code ecode = eb_font(&book, &fcode);
    if (ecode != EB_SUCCESS)
        dispError("eb_font", ecode);
    return fcode;
}
EB_Error_Code QEb::setFont(EB_Font_Code font)
{
    EB_Error_Code ecode = eb_set_font(&book, font);
    if (ecode != EB_SUCCESS) {
        dispError("eb_set_font", ecode);
        fontCode = EB_FONT_INVALID;
    } else {
        fontCode = font;
    }
    return ecode;
}
QList <EB_Font_Code> QEb::fontList()
{
    QList <EB_Font_Code> flist;
    EB_Font_Code fonts[EB_MAX_FONTS];
    int cnt;
    EB_Error_Code ecode = eb_font_list(&book, fonts, &cnt);
    if (ecode != EB_SUCCESS)
        dispError("eb_font_list", ecode);
    else
        for (int i = 0; i < cnt; i++)
            flist << fonts[i];
    return flist;
}
int QEb::fontHeight()
{
    int height;
    EB_Error_Code ecode = eb_font_height(&book, &height);
    if (ecode != EB_SUCCESS) {
        dispError("eb_font_height", ecode);
        return 0;
    }
    return height;
}
int QEb::fontHeight(EB_Font_Code font)
{
    int height;
    EB_Error_Code ecode = eb_font_height2(font, &height);
    if (ecode != EB_SUCCESS) {
        dispError("eb_font_height2", ecode);
        return 0;
    }
    return height;
}
int QEb::narrowFontWidth()
{
    int width;
    EB_Error_Code ecode = eb_narrow_font_width(&book, &width);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_font_width", ecode);
        return 0;
    }
    return width;
}
int QEb::wideFontWidth()
{
    int width;
    EB_Error_Code ecode = eb_wide_font_width(&book, &width);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_font_width", ecode);
        return 0;
    }
    return width;
}
int QEb::wideFontWidth(EB_Font_Code font)
{
    int width;
    EB_Error_Code ecode = eb_wide_font_width2(font, &width);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_font_width2", ecode);
        return 0;
    }
    return width;
}
int QEb::narrowFontWidth(EB_Font_Code font)
{
    int width;
    EB_Error_Code ecode = eb_narrow_font_width2(font, &width);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_font_width", ecode);
        return 0;
    }
    return width;
}
int QEb::wideFontSize()
{
    size_t size;
    EB_Error_Code ecode = eb_wide_font_size(&book, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_font_size", ecode);
        return 0;
    }
    return (int)size;
}
int QEb::narrowFontSize()
{
    size_t size;
    EB_Error_Code ecode = eb_narrow_font_size(&book, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_font_size", ecode);
        return 0;
    }
    return (int)size;
}
int QEb::wideFontSize(EB_Font_Code font)
{
    size_t size;
    EB_Error_Code ecode = eb_wide_font_size2(font, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_font_size2", ecode);
        return 0;
    }
    return (int)size;
}
int QEb::narrowFontSize(EB_Font_Code font)
{
    size_t size;
    EB_Error_Code ecode = eb_narrow_font_size2(font, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_font_size2", ecode);
        return 0;
    }
    return (int)size;
}
int QEb::wideFontStart()
{
    int start;
    EB_Error_Code ecode = eb_wide_font_start(&book, &start);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_font_start", ecode);
        return -1;
    }
    return start;
}
int QEb::narrowFontStart()
{
    int start;
    EB_Error_Code ecode = eb_narrow_font_start(&book, &start);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_font_start", ecode);
        return -1;
    }
    return start;
}
int QEb::wideFontEnd()
{
    int end;
    EB_Error_Code ecode = eb_wide_font_end(&book, &end);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_font_end", ecode);
        return -1;
    }
    return end;
}
int QEb::narrowFontEnd()
{
    int end;
    EB_Error_Code ecode = eb_narrow_font_end(&book, &end);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_font_end", ecode);
        return -1;
    }
    return end;
}
QByteArray QEb::narrowFontCharacterBitmap(int c_num)
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

    EB_Error_Code ecode = eb_narrow_font_character_bitmap(&book, c_num, bits);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_font_character_bitmap", ecode);
        delete bits;
        return QByteArray();
    }

    QByteArray b(bits, size);
    delete bits;
    return b;
}
QByteArray QEb::wideFontCharacterBitmap(int c_num)
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

    EB_Error_Code ecode = eb_wide_font_character_bitmap(&book, c_num, bits);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_font_character_bitmap", ecode);
        delete bits;
        return QByteArray();
    }

    QByteArray b(bits, size);
    delete bits;
    return b;
}
int QEb::forwardNarrowFontCharacter(int n, int c_num)
{
    int f_num = c_num;
    EB_Error_Code ecode = eb_forward_narrow_font_character(&book, n, &f_num);
    if (ecode != EB_SUCCESS) {
        dispError("eb_forward_narrow_font_character", ecode);
        return -1;
    }
    return f_num;
}
int QEb::forwardWideFontCharacter(int n, int c_num)
{
    int f_num = c_num;
    EB_Error_Code ecode = eb_forward_wide_font_character(&book, n, &f_num);
    if (ecode != EB_SUCCESS) {
        dispError("eb_forward_wide_font_character", ecode);
        return -1;
    }
    return f_num;
}
int QEb::backwardNarrowFontCharacter(int n, int c_num)
{
    int b_num = c_num;
    EB_Error_Code ecode = eb_backward_narrow_font_character(&book, n, &b_num);
    if (ecode != EB_SUCCESS) {
        dispError("eb_backward_narrow_font_character", ecode);
        return -1;
    }
    return b_num;
}
int QEb::backwardWideFontCharacter(int n, int c_num)
{
    int b_num = c_num;
    EB_Error_Code ecode = eb_backward_wide_font_character(&book, n, &b_num);
    if (ecode != EB_SUCCESS) {
        dispError("eb_backward_wide_font_character", ecode);
        return -1;
    }
    return b_num;
}
int QEb::narrowFontXbmSize(EB_Font_Code height)
{
    size_t size;
    EB_Error_Code ecode = eb_narrow_font_xbm_size(height, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_font_xbm_size", ecode);
        return 0;
    }
    return (int)size;
}
int QEb::narrowFontXpmSize(EB_Font_Code height)
{
    size_t size;
    EB_Error_Code ecode = eb_narrow_font_xpm_size(height, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_font_xpm_size", ecode);
        return 0;
    }
    return (int)size;
}
int QEb::narrowFontGifSize(EB_Font_Code height)
{
    size_t size;
    EB_Error_Code ecode = eb_narrow_font_gif_size(height, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_font_gif_size", ecode);
        return 0;
    }
    return (int)size;
}
int QEb::narrowFontBmpSize(EB_Font_Code height)
{
    size_t size;
    EB_Error_Code ecode = eb_narrow_font_bmp_size(height, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_font_bmp_size", ecode);
        return 0;
    }
    return (int)size;
}
int QEb::narrowFontPngSize(EB_Font_Code height)
{
    size_t size;
    EB_Error_Code ecode = eb_narrow_font_png_size(height, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_font_png_size", ecode);
    }
    return (int)size;
}
int QEb::wideFontXbmSize(EB_Font_Code height)
{
    size_t size;
    EB_Error_Code ecode = eb_wide_font_xbm_size(height, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_font_xbm_size", ecode);
        return 0;
    }
    return (int)size;
}
int QEb::wideFontXpmSize(EB_Font_Code height)
{
    size_t size;
    EB_Error_Code ecode = eb_wide_font_xpm_size(height, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_font_xpm_size", ecode);
        return 0;
    }
    return (int)size;
}
int QEb::wideFontGifSize(EB_Font_Code height)
{
    size_t size;
    EB_Error_Code ecode = eb_wide_font_gif_size(height, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_font_gif_size", ecode);
        return 0;
    }
    return (int)size;
}
int QEb::wideFontBmpSize(EB_Font_Code height)
{
    size_t size;
    EB_Error_Code ecode = eb_wide_font_bmp_size(height, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_font_bmp_size", ecode);
        return 0;
    }
    return (int)size;
}
int QEb::wideFontPngSize(EB_Font_Code height)
{
    size_t size;
    EB_Error_Code ecode = eb_wide_font_png_size(height, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_font_png_size", ecode);
        return 0;
    }
    return (int)size;
}
QSize QEb::narrowFontQSize()
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

QSize QEb::wideFontQSize()
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
QByteArray QEb::narrowBitmapToXbm(const QByteArray &bitmap)
{
    QSize sz = narrowFontQSize();
    QByteArray b;
    if (sz.width() == 0) 
        return b;

    int image_size= narrowFontXbmSize(font());
    char *buff = new char[image_size];
    size_t size;
    EB_Error_Code ecode = eb_bitmap_to_xbm(bitmap, sz.width(), sz.height(),
                                           buff, &size);
    if (ecode != EB_SUCCESS)
        dispError("eb_bitmap_to_xbm", ecode);
    else
        b = QByteArray(buff, (int)size);
    delete buff;
    return b;
}
QByteArray QEb::wideBitmapToXbm(const QByteArray &bitmap)
{
    QSize sz = wideFontQSize();
    QByteArray b;
    if (sz.width() == 0) 
        return b;

    int image_size= wideFontXbmSize(font());
    char *buff = new char[image_size];
    size_t size;
    EB_Error_Code ecode = eb_bitmap_to_xbm(bitmap, sz.width(), sz.height(),
                                           buff, &size);
    if (ecode != EB_SUCCESS)
        dispError("eb_bitmap_to_xbm", ecode);
    else
        b = QByteArray(buff, (int)size);
    delete buff;
    return b;
}
QByteArray QEb::narrowBitmapToXpm(const QByteArray &bitmap)
{
    QSize sz = narrowFontQSize();
    QByteArray b;
    if (sz.width() == 0) 
        return b;

    int image_size= narrowFontXpmSize(font());
    char *buff = new char[image_size];
    size_t size;
    EB_Error_Code ecode = eb_bitmap_to_xpm(bitmap, sz.width(), sz.height(),
                                           buff, &size);
    if (ecode != EB_SUCCESS)
        dispError("eb_bitmap_to_xpm", ecode);
    else
        b = QByteArray(buff, (int)size);
    delete buff;
    return b;
}
QByteArray QEb::wideBitmapToXpm(const QByteArray &bitmap)
{
    QSize sz = wideFontQSize();
    QByteArray b;
    if (sz.width() == 0) 
        return b;

    int image_size= wideFontXpmSize(font());
    char *buff = new char[image_size];
    size_t size;
    EB_Error_Code ecode = eb_bitmap_to_xpm(bitmap, sz.width(), sz.height(),
                                           buff, &size);
    if (ecode != EB_SUCCESS)
        dispError("eb_bitmap_to_xpm", ecode);
    else
        b = QByteArray(buff, (int)size);
    
    delete buff;
    return b;
}
QByteArray QEb::narrowBitmapToGif(const QByteArray &bitmap)
{
    QSize sz = narrowFontQSize();
    QByteArray b;
    if (sz.width() == 0) 
        return b;

    int image_size= narrowFontGifSize(font());
    char *buff = new char[image_size];
    size_t size;
    EB_Error_Code ecode = eb_bitmap_to_gif(bitmap, sz.width(), sz.height(),
                                           buff, &size);
    if (ecode != EB_SUCCESS)
        dispError("eb_bitmap_to_gif", ecode);
    else
        b = QByteArray(buff, (int)size);
    delete buff;
    return b;
}
QByteArray QEb::wideBitmapToGif(const QByteArray &bitmap)
{
    QSize sz = wideFontQSize();
    QByteArray b;
    if (sz.width() == 0) 
        return b;

    int image_size= wideFontGifSize(font());
    char *buff = new char[image_size];
    size_t size;
    EB_Error_Code ecode = eb_bitmap_to_gif(bitmap, sz.width(), sz.height(),
                                           buff, &size);
    if (ecode != EB_SUCCESS)
        dispError("eb_bitmap_to_gif", ecode);
    else
        b = QByteArray(buff, (int)size);
    delete buff;
    return b;
}
QByteArray QEb::narrowBitmapToBmp(const QByteArray &bitmap)
{
    QSize sz = narrowFontQSize();
    QByteArray b;
    if (sz.width() == 0) 
        return b;

    int image_size= narrowFontBmpSize(font());
    char *buff = new char[image_size];
    size_t size;
    EB_Error_Code ecode = eb_bitmap_to_bmp(bitmap, sz.width(), sz.height(),
                                           buff, &size);
    if (ecode != EB_SUCCESS)
        dispError("eb_bitmap_to_bmp", ecode);
    else
        b = QByteArray(buff, (int)size);
    
    delete buff;
    return b;
}
QByteArray QEb::wideBitmapToBmp(const QByteArray &bitmap)
{
    QSize sz = wideFontQSize();
    QByteArray b;
    if (sz.width() == 0) 
        return b;

    int image_size= wideFontBmpSize(font());
    char *buff = new char[image_size];
    size_t size;
    EB_Error_Code ecode = eb_bitmap_to_bmp(bitmap, sz.width(), sz.height(),
                                           buff, &size);
    if (ecode != EB_SUCCESS)
        dispError("eb_bitmap_to_bmp", ecode);
    else
        b = QByteArray(buff, (int)size);

    delete buff;
    return b;
}
QByteArray QEb::narrowBitmapToPng(const QByteArray &bitmap)
{
    QSize sz = narrowFontQSize();
    QByteArray b;
    if (sz.width() == 0) 
        return b;

    int image_size= narrowFontPngSize(font());
    char *buff = new char[image_size];
    size_t size;
    EB_Error_Code ecode = eb_bitmap_to_png(bitmap, sz.width(), sz.height(),
                                           buff, &size);
    if (ecode != EB_SUCCESS)
        dispError("eb_bitmap_to_png", ecode);
    else
        b = QByteArray(buff, (int)size);
    
    delete buff;
    return b;
}
QByteArray QEb::wideBitmapToPng(const QByteArray &bitmap)
{
    QSize sz = wideFontQSize();
    QByteArray b;
    if (sz.width() == 0) 
        return b;

    int image_size= wideFontPngSize(font());
    char *buff = new char[image_size];
    size_t size;
    EB_Error_Code ecode = eb_bitmap_to_png(bitmap, sz.width(), sz.height(),
                                           buff, &size);
    if (ecode != EB_SUCCESS)
        dispError("eb_bitmap_to_png", ecode);
    else
        b = QByteArray(buff, (int)size);
    
    delete buff;
    return b;
}

// Graphics, Sounds, Movies
EB_Error_Code QEb::setBinaryMonoGraphic(const EB_Position &position, int width,
                                   int height)
{
    EB_Error_Code ecode = eb_set_binary_mono_graphic(&book, &position,
                                                     width, height);
    if (ecode != EB_SUCCESS)
        dispError("eb_set_binary_mono_graphic", ecode);
    return ecode;
}
EB_Error_Code QEb::setBinaryColorGraphic(const EB_Position &position)
{
    EB_Error_Code ecode = eb_set_binary_color_graphic(&book, &position);
    if (ecode != EB_SUCCESS)
        dispError("eb_set_binary_color_graphic", ecode);
    return ecode;
}
EB_Error_Code QEb::setBinaryWave(const EB_Position &start, EB_Position &end)
{
    EB_Error_Code ecode = eb_set_binary_wave(&book, &start, &end);
    if (ecode != EB_SUCCESS)
        dispError("eb_set_binary_wave", ecode);
    return ecode;
}
EB_Error_Code QEb::setBinaryMpeg(const unsigned int *argv)
{
    EB_Error_Code ecode = eb_set_binary_mpeg(&book, argv);
    if (ecode != EB_SUCCESS)
        dispError("eb_set_binary_mpeg", ecode);
    return ecode;
}
QByteArray QEb::readBinary()
{
    char buff[1024];
    ssize_t len;
    QByteArray b;
    for(;;) {
        EB_Error_Code ecode = eb_read_binary(&book, 1024, buff, &len);
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
QString QEb::composeMovieFileName(const unsigned int *argv)
{
    char name[EB_MAX_DIRECTORY_NAME_LENGTH+1];
    EB_Error_Code ecode = eb_compose_movie_file_name(argv, name);
    if (ecode != EB_SUCCESS) {
        dispError("eb_compose_movie_file_name", ecode);
        return QString();
    }
    return eucToUtf(name);
}
QString QEb::composeMoviePathName(const unsigned int *argv)
{
    char path[EB_MAX_PATH_LENGTH+1];
    EB_Error_Code ecode = eb_compose_movie_path_name(&book, argv, path);
    if (ecode != EB_SUCCESS) {
        dispError("eb_compose_movie_path_name", ecode);
        return QString();
    }
    return eucToUtf(path);
}
EB_Error_Code QEb::decomposeMovieFileName(unsigned int *argv,
                                     const QString &name)
{
    EB_Error_Code ecode = eb_decompose_movie_file_name(argv, utfToEuc(name));
    if (ecode != EB_SUCCESS)
        dispError("eb_decompose_movie_file_name", ecode);
    return ecode;
}

EB_Error_Code QEb::bindAppendix(QString path)
{
    EB_Error_Code ecode = eb_bind_appendix(&appendix, path.toLocal8Bit());
    if (ecode != EB_SUCCESS)
        dispError("eb_bind_appendix", ecode);
    return ecode;
}
QString QEb::appendixPath()
{
    char path[EB_MAX_PATH_LENGTH+1];
    EB_Error_Code ecode = eb_appendix_path(&appendix, path);
    if (ecode != EB_SUCCESS) {
        dispError("eb_appendix_path", ecode);
        return QString();
    }
    return eucToUtf(path);
}
EB_Error_Code QEb::loadAllAppendixSubbooks()
{
    EB_Error_Code ecode = eb_load_all_appendix_subbooks(&appendix);
    if (ecode != EB_SUCCESS)
        dispError("eb_load_all_appendix_subbooks", ecode);
    return ecode;
}
QList <EB_Subbook_Code> QEb::appendixSubbookList()
{
    EB_Subbook_Code code[EB_MAX_SUBBOOKS];
    int cnt;
    QList <EB_Subbook_Code> list;

    EB_Error_Code ecode = eb_appendix_subbook_list(&appendix, code, &cnt);
    if (ecode != EB_SUCCESS)
        dispError("eb_appendix_subbook_list", ecode);
    else
        for (int i = 0; i < cnt; i++)
            list << code[i];
    return list;
}
EB_Subbook_Code QEb::appendixSubbook()
{
    EB_Subbook_Code code;
    EB_Error_Code ecode = eb_appendix_subbook(&appendix, &code);
    if (ecode != EB_SUCCESS)
        dispError("eb_appendix_subbook", ecode);
    return code;
}
QString QEb::appendixSubbookDirectory()
{
    char dir[EB_MAX_DIRECTORY_NAME_LENGTH+1];
    EB_Error_Code ecode = eb_appendix_subbook_directory(&appendix, dir);
    if (ecode != EB_SUCCESS) {
        dispError("eb_appendix_subbook_directory", ecode);
        return QString();
    }
    return eucToUtf(dir);
}
QString QEb::appendixSubbookDirectory(EB_Subbook_Code code)
{
    char dir[EB_MAX_DIRECTORY_NAME_LENGTH+1];
    EB_Error_Code ecode = eb_appendix_subbook_directory2(&appendix, code, dir);
    if (ecode != EB_SUCCESS) {
        dispError("eb_appendix_subbook_directory2", ecode);
        return QString();
    }
    return eucToUtf(dir);
}
bool QEb::isHaveAppendixSubbook(EB_Subbook_Code code)
{
    EB_Error_Code ecode = eb_set_appendix_subbook(&appendix, code);
    if (ecode != EB_SUCCESS) {
        return false;
    }
    eb_unset_appendix_subbook(&appendix);
    return true;
}
EB_Error_Code QEb::setAppendixSubbook(EB_Subbook_Code code)
{
    EB_Error_Code ecode = eb_set_appendix_subbook(&appendix, code);
    if (ecode != EB_SUCCESS)
        dispError("eb_set_appendix_subbook", ecode);

    return ecode;
}
EB_Error_Code QEb::stopCode(int *stop1, int *stop2)
{
    int stop_code[2];
    EB_Error_Code ecode = eb_stop_code(&appendix, stop_code);
    if (ecode != EB_SUCCESS) {
        dispError("eb_stop_code", ecode);
        *stop1 = *stop2 = -1;
    } else {
        *stop1 = stop_code[0];
        *stop2 = stop_code[1];
    }
    return ecode;
}
int QEb::narrowAltStart()
{
    int start;
    EB_Error_Code ecode = eb_narrow_alt_start(&appendix, &start);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_alt_start", ecode);
        return -1;
    }
    return start;
}
int QEb::wideAltStart()
{
    int start;
    EB_Error_Code ecode = eb_wide_alt_start(&appendix, &start);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_alt_start", ecode);
        return -1;
    }
    return start;
}
int QEb::narrowAltEnd()
{
    int end;
    EB_Error_Code ecode = eb_narrow_alt_end(&appendix, &end);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_alt_end", ecode);
        return -1;
    }
    return end;
}
int QEb::wideAltEnd()
{
    int end;
    EB_Error_Code ecode = eb_wide_alt_end(&appendix, &end);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_alt_end", ecode);
        return -1;
    }
    return end;
}
QString QEb::narrowAltCharacterText(int c_num)
{
    char alt[EB_MAX_ALTERNATION_TEXT_LENGTH+1];
    EB_Error_Code ecode = eb_narrow_alt_character_text(&appendix, c_num, alt);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_alt_character_text", ecode);
        return QString();
    }
    if (characterCode() == EB_CHARCODE_ISO8859_1)
        return QString::fromLatin1(alt);
    else
        return eucToUtf(alt);
}
QString QEb::wideAltCharacterText(int c_num)
{
    char alt[EB_MAX_ALTERNATION_TEXT_LENGTH+1];
    EB_Error_Code ecode = eb_wide_alt_character_text(&appendix, c_num, alt);
    if (ecode != EB_SUCCESS) {
        dispError("eb_wide_alt_character_text", ecode);
        return QString();
    }
    if (characterCode() == EB_CHARCODE_ISO8859_1)
        return QString::fromLatin1(alt);
    else
        return eucToUtf(alt);
}
int QEb::backwardNarrowAltCharacter(int n, int c_num)
{
    int n_num = c_num;
    EB_Error_Code ecode = eb_backward_narrow_alt_character(&appendix, n, &n_num);
    if (ecode != EB_SUCCESS) {
        dispError("eb_backward_narrow_alt_character", ecode);
        return -1;
    }
    return n_num;
}
int QEb::forwardNarrowAltCharacter(int n, int c_num)
{
    int n_num = c_num;
    EB_Error_Code ecode = eb_forward_narrow_alt_character(&appendix, n, &n_num);
    if (ecode != EB_SUCCESS) {
        dispError("eb_forward_narrow_alt_character", ecode);
        return -1;
    }
    return n_num;
}
int QEb::backwardWideAltCharacter(int n, int c_num)
{
    int n_num = c_num;
    EB_Error_Code ecode = eb_backward_wide_alt_character(&appendix, n, &n_num);
    if (ecode != EB_SUCCESS) {
        dispError("eb_backward_wide_alt_character", ecode);
        return -1;
    }
    return n_num;
}
int QEb::forwardWideAltCharacter(int n, int c_num)
{
    int n_num = c_num;
    EB_Error_Code ecode = eb_forward_wide_alt_character(&appendix, n, &n_num);
    if (ecode != EB_SUCCESS) {
        dispError("eb_forward_wide_alt_character", ecode);
        return -1;
    }
    return n_num;
}

EB_Error_Code  QEb::bindBooklist(const QString &path)
{
    EB_Error_Code ecode = eb_bind_booklist(&bookList, utfToEuc(path));
    if (ecode != EB_SUCCESS)
        dispError("eb_bind_boollist", ecode);
    return ecode;
}
EB_Error_Code QEb::booklistBookCount()
{
    int cnt;
    EB_Error_Code ecode = eb_booklist_book_count(&bookList, &cnt);
    if (ecode != EB_SUCCESS) {
        dispError("eb_bind_boollist", ecode);
        return -1;
    }
    return cnt;
}
QString QEb::booklistBookName(int idx)
{
    char *name;
    EB_Error_Code ecode = eb_booklist_book_name(&bookList, idx, &name);
    if (ecode != EB_SUCCESS) {
        dispError("eb_booklist_book_name", ecode);
        return QString();
    }
    return eucToUtf(name);
}
QString QEb::booklistBookTitle(int idx)
{
    char *title;
    EB_Error_Code ecode = eb_booklist_book_title(&bookList, idx, &title);
    if (ecode != EB_SUCCESS) {
        dispError("eb_booklist_book_title", ecode);
        return QString();
    }
    return eucToUtf(title);
}
