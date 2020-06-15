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

#include "qeb.h"

#include <QSize>
#include <QStringList>

namespace {

QString decodePath(const char *path)
{
#ifdef QOLIBRI_EB_UTF8_PATHS
    return QString::fromUtf8(path);
#else
    return QString::fromLocal8Bit(path);
#endif
}

QByteArray encodePath(const QString &path)
{
#ifdef QOLIBRI_EB_UTF8_PATHS
    return path.toUtf8();
#else
    return path.toLocal8Bit();
#endif
}

} // anonymous namespace

QTextCodec *const QEb::eucCodec = QTextCodec::codecForName("EUC-JP");

EB_Error_Code QEb::bind(const QString &path)
{
    EB_Error_Code ecode = eb_bind(&book, encodePath(path));
    if (ecode != EB_SUCCESS)
        dispError("eb_bind(" + path + ")", ecode);
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
    return decodePath(s);
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

QString QEb::subbookTitle()
{
    char title[EB_MAX_TITLE_LENGTH+1];
    EB_Error_Code ecode = eb_subbook_title(&book, title);
    if (ecode != EB_SUCCESS) {
        dispError("eb_subbook_title", ecode);
        return QString();
    }
    return eucCodec->toUnicode(title);
}

EB_Error_Code QEb::setSubbook(EB_Subbook_Code code)
{
    EB_Error_Code ecode = eb_set_subbook(&book, code);
    if (ecode != EB_SUCCESS)
        dispError("eb_set_subbook", ecode);
    return ecode;
}

EB_Error_Code QEb::searchWord(const QString &word)
{
    EB_Error_Code ecode = eb_search_word(&book, eucCodec->fromUnicode(word));
    if (ecode != EB_SUCCESS)
        dispError("eb_search_word", ecode);
    return ecode;
}
EB_Error_Code QEb::searchEndword(const QString &word)
{
    EB_Error_Code ecode = eb_search_endword(&book, eucCodec->fromUnicode(word));
    if (ecode != EB_SUCCESS)
        dispError("eb_search_endword", ecode);
    return ecode;
}
EB_Error_Code QEb::searchExactword(const QString &word)
{
    EB_Error_Code ecode = eb_search_exactword(&book, eucCodec->fromUnicode(word));
    if (ecode != EB_SUCCESS)
        dispError("eb_search_exactword", ecode);
    return ecode;
}
QList <QByteArray> QEb::toEucList(const QStringList &words)
{
    QList <QByteArray> blist;
    for (int i = 0; i < words.count(); i++)
        blist << eucCodec->fromUnicode(words[i]);
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
    delete[] wlist;
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
    delete[] wlist;
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
    delete[] harray;
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
        dispError("eb_seek_text", ecode);
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
    return b;

}
QString QEb::readHeading(void *para)
{
    char buff[1024+1];
    ssize_t len;
    QByteArray b;
    for(;;) {
        EB_Error_Code ecode;
        ecode = eb_read_heading(&book, &appendix, &hookset, para, 1024, buff, &len);
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
    return b;

}
EB_Error_Code QEb::forwardText()
{
    EB_Error_Code ecode = eb_forward_text(&book, &appendix);
    if (ecode != EB_SUCCESS && ecode != EB_ERR_END_OF_CONTENT)
        dispError("eb_forward_text", ecode);
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
QString QEb::currentCandidate()
{
    const char *s = eb_current_candidate(&book);
    if (characterCode() == EB_CHARCODE_ISO8859_1)
        return QString::fromLatin1(s);
    else
        return eucCodec->toUnicode(s);
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
        delete[] bits;
        return QByteArray();
    }

    QByteArray b(bits, size);
    delete[] bits;
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
        delete[] bits;
        return QByteArray();
    }

    QByteArray b(bits, size);
    delete[] bits;
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
int QEb::narrowFontPngSize(EB_Font_Code height)
{
    size_t size;
    EB_Error_Code ecode = eb_narrow_font_png_size(height, &size);
    if (ecode != EB_SUCCESS) {
        dispError("eb_narrow_font_png_size", ecode);
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

    delete[] buff;
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

    delete[] buff;
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
QByteArray QEb::readBinary()
{
    char buff[1024];
    ssize_t len;
    QByteArray b;
    for(;;) {
        EB_Error_Code ecode = eb_read_binary(&book, 1024, buff, &len);
        if (ecode != EB_SUCCESS) {
            dispError("eb_read_binary", ecode);
            return b;
        }
        if (len > 0 )
            b += QByteArray(buff, (int)len);
        if (len < 1024)
            break;
    }
    return b;
}
QString QEb::composeMoviePathName(const unsigned int *argv)
{
    char path[EB_MAX_PATH_LENGTH+1];
    EB_Error_Code ecode = eb_compose_movie_path_name(&book, argv, path);
    if (ecode != EB_SUCCESS) {
        dispError("eb_compose_movie_path_name", ecode);
        return QString();
    }
    return path;
}

EB_Error_Code QEb::bindAppendix(const QString &path)
{
    EB_Error_Code ecode = eb_bind_appendix(&appendix, encodePath(path));
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
    return eucCodec->toUnicode(path);
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
QString QEb::appendixSubbookDirectory()
{
    char dir[EB_MAX_DIRECTORY_NAME_LENGTH+1];
    EB_Error_Code ecode = eb_appendix_subbook_directory(&appendix, dir);
    if (ecode != EB_SUCCESS) {
        dispError("eb_appendix_subbook_directory", ecode);
        return QString();
    }
    return eucCodec->toUnicode(dir);
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
