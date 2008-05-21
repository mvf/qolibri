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

#include "ebook.h"
#include "ebhook.h"
#include "textcodec.h"
#include <eb/eb.h>
#include <eb/error.h>
#include <eb/font.h>
#include <eb/binary.h>

const int ImageBufferSize = 50000;

QByteArray EbHook::begin_decoration(int deco_code)
{
    decoStack.push(deco_code);
    if (deco_code == 1)
        return "<i>";
    else if (deco_code == 3)
        return "<b>";
    //else
    //    qWarning() << "Unrecognized decoration code" << deco_code;
    return "<i>";
}

QByteArray EbHook::end_decoration()
{
    if (!decoStack.isEmpty()) {
        int deco_code = decoStack.pop();
        if (deco_code == 1)
            return "</i>";
        else if(deco_code == 3)
            return "</b>";
        //else
            //qWarning() << "Unrecognized decoration code" << deco_code;
    }
    return "</i>";
}

QByteArray EbHook::set_indent(int val)
{
    QByteArray ret;
    if (val > 2){
        int mleft = indentOffset + (val * fontSize);
        ret += "</pre><pre style=\"margin-left: " +
               QByteArray::number(mleft) + "px; \">";
        indented = true;
    } else {
        if (indented) {
            ret = "</pre><pre>";
            indented = false;
        }
    }
    return ret;
}

QByteArray EbHook::narrow_font(EB_Book *book, int code)
{
    if (!eb_have_narrow_font(book)) {
        qDebug() << "not have narrow font";
        return errorString("narrow font error");
    }

    EB_Error_Code ecode;
    QString fcode = "n" + QString::number(code, 16);
    if (fontList) {
        QString afont = fontList->value(fcode);
        if (!afont.isEmpty())
            return afont.toAscii();
    }
#ifdef USE_GIF_FOR_FONT
    QByteArray fname = fcode.toAscii() + ".gif";
#else
    QByteArray fname = fcode.toAscii() + ".png";
#endif

    QByteArray out = "<img src=\"" + utfToEuc(ebCache->fontCacheRel) + fname  +
                     "\"";
    int h = fontSize;
    if (h > 17) {
        out += " height=" + QByteArray::number(h) +
               " width=" + QByteArray::number(h / 2);
    }
    out += " />";

    if (ebCache->fontCacheList.contains(fname))
        return out;

    char bitmap[EB_SIZE_NARROW_FONT_16];
    if ((ecode = eb_narrow_font_character_bitmap(book, code, bitmap))
        != EB_SUCCESS) {
        EbCore::ebError("eb_narrow_font_character_bitmap", ecode);
        return errorString("narrow font error");
    }

    size_t wlen;
#ifdef USE_GIF_FOR_FONT
    char buff[EB_SIZE_NARROW_FONT_16_GIF];
    if ((ecode = eb_bitmap_to_gif(bitmap, 8, 16, buff, &wlen) != EB_SUCCESS)) {
        EbCore::ebError("eb_bitmap_to_gif", ecode);
        return errorString("narrow font error");
    }
#else
    char buff[EB_SIZE_NARROW_FONT_16_PNG];
    if ((ecode = eb_bitmap_to_png(bitmap, 8, 16, buff, &wlen) != EB_SUCCESS)) {
        EbCore::ebError("eb_bitmap_to_png", ecode);
        return errorString("narrow font error");
    }
#endif


    QFile f(ebCache->fontCachePath +  '/' + fname);
    f.open(QIODevice::WriteOnly);
    f.write(buff, wlen);
    f.close();
    //qDebug() << "Output narrow_font" << fname;
    ebCache->fontCacheList << fname;

    return out;
}

QByteArray EbHook::wide_font(EB_Book *book, int code)
{
    EB_Error_Code ecode;

    if (!eb_have_wide_font(book)) {
        qDebug() << "not have wide font";
        return errorString("wide font error");
    }

    QString fcode = "w" + QString::number(code, 16);

    if (fontList) {
        QString afont = fontList->value(fcode);
        if (!afont.isEmpty())
            return afont.toAscii();
    }

#ifdef USE_GIF_FOR_FONT
    QByteArray fname = fcode.toAscii() + ".gif";
#else
    QByteArray fname = fcode.toAscii() + ".png";
#endif

    QByteArray out = "<img src=\"" + utfToEuc(ebCache->fontCacheRel) +
                     fname  + "\"";
    int h = fontSize;
    if (h > 17)
        out += " height=" + QByteArray::number(h) +
               " width=" + QByteArray::number(h);
    out += " />";


    if (ebCache->fontCacheList.contains(fname)) {
        return out;
    }

    char bitmap[EB_SIZE_WIDE_FONT_16];
    if ((ecode = eb_wide_font_character_bitmap(book, code, bitmap))
        != EB_SUCCESS) {
        EbCore::ebError("eb_wide_font_character_bitmap", ecode);
        return errorString("wide font error");
    }

    size_t wlen;
#ifdef USE_GIF_FOR_FONT
    char buff[EB_SIZE_WIDE_FONT_16_GIF];
    if ((ecode = eb_bitmap_to_gif(bitmap, 16, 16, buff, &wlen)) != EB_SUCCESS) {
        EbCore::ebError("eb_bitmap_to_gif", ecode);
        return errorString("wide font error");
    }
#else
    char buff[EB_SIZE_WIDE_FONT_16_PNG];
    if ((ecode = eb_bitmap_to_png(bitmap, 16, 16, buff, &wlen)) != EB_SUCCESS) {
        EbCore::ebError("eb_bitmap_to_png", ecode);
        return errorString("wide font error");
    }
#endif

    QFile f(ebCache->fontCachePath + '/' + fname);
    f.open(QIODevice::WriteOnly);
    f.write(buff, wlen);
    f.close();
    ebCache->fontCacheList << fname;
    //qDebug() << "Output wide_font" << xpmFile;
    return out;
}


QByteArray EbHook::begin_candidate()
{
    return "<a class=cnd href=\"<R" +
           QByteArray::number(refList.count()) +
           "R>\">";
}

QByteArray EbHook::begin_candidate_menu()
{
    return "<C";
}


QByteArray EbHook::begin_reference()
{
    return "<a class=ref href=\"<R" +
           QByteArray::number(refList.count()) +
           "R>\">";
}

QByteArray EbHook::begin_color_jpeg(EB_Book *book, int page, int offset)
{
    imageCount++;

    QByteArray jpgFile = makeFname("jpeg", page, offset);
    QByteArray out = "<img src=\"" + utfToEuc(ebCache->imageCacheRel) +
                     jpgFile + "\"><span class=img>";

    if (ebCache->imageCacheList.contains(jpgFile)) {
        return out;
    }

    //qDebug() << "Out Image " << jpgFile;

    EB_Position pos;
    EB_Error_Code err;
    pos.page = page;
    pos.offset = offset;
    if ((err = eb_set_binary_color_graphic(book, &pos)) != EB_SUCCESS) {
    EbCore::ebError("eb_set_binary_color_graphic", err);
        return errorString("image(jpeg) error");
    }


    char bitmap[ImageBufferSize];
    Q_CHECK_PTR(bitmap);
    ssize_t bitmap_length;
    QFile f(ebCache->imageCachePath + '/' + jpgFile);
    f.open(QIODevice::WriteOnly);
    int flg = 0;

    for (;;)
    {
        if ((err = eb_read_binary(book, ImageBufferSize, bitmap,
                                  &bitmap_length)) != EB_SUCCESS) {
            EbCore::ebError("eb_read_binary", err);
            return errorString("image(jpeg) error");
        }
        // qDebug() << "eb_read_binary : size=" << bitmap_length;
        f.write(bitmap, bitmap_length);
        flg++;
        if (bitmap_length < ImageBufferSize) break;
    }
    f.close();
    //qDebug() << "Image Size :" << ImageBufferSize * flg;
    ebCache->imageCacheList << jpgFile;

    return out;
}

QByteArray EbHook::begin_color_bmp(EB_Book *book, int page, int offset)
{
    EB_Error_Code ecode;

    imageCount++;

    QByteArray bmpFile = makeFname("bmp", page, offset);
    QByteArray out = "<img src=\"" + utfToEuc(ebCache->imageCacheRel) +
                     bmpFile + "\" /><span class=img>";

    if (ebCache->imageCacheList.contains(bmpFile))
        return out;

    //qDebug() << "Output Image " << bmpFile;

    EB_Position pos;
    pos.page = page;
    pos.offset = offset;
    if ((ecode = eb_set_binary_color_graphic(book, &pos)) != EB_SUCCESS) {
        EbCore::ebError("eb_set_binary_color_graphic", ecode);
        return errorString("image(bmp) error");
    }


    char bitmap[ImageBufferSize];
    Q_CHECK_PTR(bitmap);
    ssize_t bitmap_length;
    QFile f(ebCache->imageCachePath + '/' + bmpFile);
    f.open(QIODevice::WriteOnly);
    int flg = 0;

    for (;;)
    {
        if ((ecode = eb_read_binary(book, ImageBufferSize,
                                    bitmap, &bitmap_length)) != EB_SUCCESS) {
            EbCore::ebError("eb_read_binary", ecode);
            return errorString("image(bmp) error");
        }
        //qDebug() << "eb_read_binary : size=" << bitmap_length;
        f.write(bitmap, bitmap_length);
        flg++;
        if (bitmap_length < ImageBufferSize) break;
    }
    f.close();
    //qDebug() << "Image Size :" << ImageBufferSize * flg;
    ebCache->imageCacheList << bmpFile;

    return out;
}

QByteArray EbHook::end_reference(int page, int offset)
{
    QByteArray ref = "book|" + QByteArray::number(refPosition) + '|' +
                     QByteArray::number(page) + '|' +
                     QByteArray::number(offset);

    refList << ref;
    return "</a>";
}

QByteArray EbHook::end_candidate_group(int page, int offset)
{
    QByteArray cnd = "menu|" + QByteArray::number(refPosition) + '|' +
                     QByteArray::number(page) + '|' +
                     QByteArray::number(offset);

    refList << cnd;
    return "</a>";
}

QByteArray EbHook::end_candidate_group_menu(EB_Book *book, int page, int offset)
{
    QString str = eucToUtf(eb_current_candidate(book)) + "&|" +
                  QString::number(page) + "&|" +  QString::number(offset);

    candList << str;
    return "C>";
}

QByteArray EbHook::begin_mpeg()
{
    return "<a class=mpg href=\"<M" + QByteArray::number(mpegList.count()) +
           "M>\">";
}

void EbHook::end_mpeg(EB_Book *book, const unsigned int *p)
{
    EB_Error_Code ecode;
    char sfile[EB_MAX_PATH_LENGTH + 1];

    if ((ecode = eb_compose_movie_path_name(book, p, sfile)) != EB_SUCCESS) {
        EbCore::ebError("eb_compose_movie_path_name", ecode);
        return ;
    }
    QString dfile = ebCache->mpegCachePath + "/" +
                    QFileInfo(sfile).fileName() + ".mpeg";
    if (!QFile(dfile).exists())
        QFile::copy(sfile, dfile);
    mpegList << "mpeg|" + utfToEuc(dfile);
}

QByteArray EbHook::end_mono_graphic(EB_Book *book, int page, int offset)
{
    EB_Error_Code ecode;

    QByteArray bmpFile = makeFname("bmp", page, offset);

    QByteArray out = "<img src=\"" + utfToEuc(ebCache->imageCacheRel) +
                     bmpFile + "\" />\n";
    

    if (ebCache->imageCacheList.contains(bmpFile)) {
        return out;
    }

    EB_Position pos;
    pos.page = page;
    pos.offset = offset;
    //qDebug() << eb->monoWidth << " " << eb->monoHeight;
    if ((ecode = eb_set_binary_mono_graphic(book, &pos, monoWidth,
                                            monoHeight)) != EB_SUCCESS) {
        EbCore::ebError("eb_set_binary_mono_graphic", ecode);
        return errorString("image(mono) error");
    }


    char bitmap[ImageBufferSize];
    Q_CHECK_PTR(bitmap);
    ssize_t bitmap_length;
    QFile f(ebCache->imageCachePath + '/' + bmpFile);
    f.open(QIODevice::WriteOnly);

    int flg = 0;
    for (;;) {
        if ((ecode = eb_read_binary(book, ImageBufferSize, bitmap,
                                    &bitmap_length)) != EB_SUCCESS) {
            EbCore::ebError("eb_read_binary", ecode);
            return errorString("image(mono) error");
        }
        f.write(bitmap, bitmap_length);
        flg++;
        if (bitmap_length < ImageBufferSize) break;
    }
    f.close();
    ebCache->imageCacheList << bmpFile;

    return out;
}

QByteArray EbHook::begin_wave(EB_Book *book, int start_page, int start_offset,
                              int end_page, int end_offset)
{
    EB_Error_Code ecode;

    QString wavFile = QString("%1x%2.wav").arg(start_page).arg(start_offset);
    QString fname = ebCache->waveCachePath + "/" + wavFile;
    QString out =  QString("<a class=snd href=\"sound|%1\">").arg(fname);

    if (ebCache->waveCacheList.contains(wavFile))
        return utfToEuc(out);

    EB_Position spos, epos;
    spos.page = start_page;
    spos.offset = start_offset;
    epos.page = end_page;
    epos.offset = end_offset;
    eb_set_binary_wave(book, &spos, &epos);

    char data[ImageBufferSize];
    Q_CHECK_PTR(data);
    ssize_t data_length;
    QFile f(fname);
    f.open(QIODevice::WriteOnly);
    int flg = 0;
    for (;;)
    {
        if ((ecode = eb_read_binary(book, ImageBufferSize, data,
                                    &data_length)) != EB_SUCCESS) {
            EbCore::ebError("eb_read_binary", ecode);
            return errorString("wave error");
        }
        //qDebug() << "eb_read_binary : size=" << data_length;
        f.write(data, data_length);
        flg++;
        if (data_length < ImageBufferSize) break;
    }
    f.close();
    ebCache->waveCacheList << wavFile;
    //qDebug() << "Output wave" << fname;
    return utfToEuc(out);
}

/*
   EB_Error_Code hook_newline(EB_Book *book, EB_Appendix*, void*,
                           EB_Hook_Code, int, const unsigned int*)
   {
    //qDebug() << "Begin Narrow";
    eb_write_text_string(book, "<br>");
    return 0;
   }

   EB_Error_Code hook_iso8859_1(EB_Book*, EB_Appendix*, void*, EB_Hook_Code,
                             int, const unsigned int*)
   {

    qDebug() << "HOOK ISO8859_1";
    return 0;
   }
 */

EB_Error_Code hook_narrow_jisx0208(EB_Book* book, EB_Appendix* appendix,
                                   void* container, EB_Hook_Code code, int argc,
                                   const unsigned int *argv)
{
    if (*argv == 41443) {
        eb_write_text_string(book, "&lt;");
    } else if (*argv == 41444) {
        eb_write_text_string(book, "&gt;");
    } else if (*argv == 41461) {
        eb_write_text_string(book, "&amp;");
    } else {
        eb_hook_euc_to_ascii(book, appendix, container, code, argc, argv);
    }
    return 0;
}

/*
EB_Error_Code hook_wide_jisx0208(EB_Book *book, EB_Appendix*, void*,
                                    EB_Hook_Code, int,
                                    const unsigned int *argv)
{
    //qDebug() << "HOOK WIDE_JISx0208";
    return 0;
}
   EB_Error_Code hook_gb2312(EB_Book *book, EB_Appendix*, void*, EB_Hook_Code,
                          int, const unsigned int*)
   {

    qDebug() << "HOOK gb2312";
    eb_write_text_string(book,"<em class=err>gb2312</em>");
    return 0;
   }
 */
/*
   EB_Error_Code hook_begin_narrow(EB_Book*, EB_Appendix*, void*, EB_Hook_Code,
                                int, const unsigned int*)
   {
    //qDebug() << "Begin Narrow";
    //eb_write_text_string(book, "[Begin Narrow]");
    //eb_write_text_string(book, "<span>");
    return 0;
   }

   EB_Error_Code hook_end_narrow(EB_Book*, EB_Appendix*, void*, EB_Hook_Code,
   int,
                              const unsigned int*)
   {
    //qDebug() << "End Narrow";
    //eb_write_text_string(book, "[End Narrow]");
    //eb_write_text_string(book, "</span>");
    return 0;
   }
 */

EB_Error_Code hook_set_indent(EB_Book *book, EB_Appendix*, void *container,
                                   EB_Hook_Code, int, const unsigned int *argv)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->set_indent(argv[1]));
    return 0;
}

EB_Error_Code hook_begin_subscript(EB_Book *book, EB_Appendix*, void *container,
                                   EB_Hook_Code, int, const unsigned int*)
{
    EbHook *eb = static_cast<EbHook*>(container);

    //eb_write_text_string(book, "<sub>");
    eb_write_text_string(book, eb->begin_subscript());
    return 0;
}

EB_Error_Code hook_end_subscript(EB_Book *book, EB_Appendix*, void *container,
                                 EB_Hook_Code, int, const unsigned int*)
{
    EbHook *eb = static_cast<EbHook*>(container);

    //eb_write_text_string(book, "</sub>");
    eb_write_text_string(book, eb->end_subscript());
    return 0;
}

EB_Error_Code hook_begin_superscript(EB_Book *book, EB_Appendix*, void*,
                                     EB_Hook_Code, int, const unsigned int*)
{
    eb_write_text_string(book, "<sup>");
    return 0;
}

EB_Error_Code hook_end_superscript(EB_Book *book, EB_Appendix*, void*,
                                   EB_Hook_Code, int, const unsigned int*)
{
    eb_write_text_string(book, "</sup>");
    return 0;
}

EB_Error_Code hook_begin_emphasize(EB_Book *book, EB_Appendix*, void*,
                                   EB_Hook_Code, int, const unsigned int*)
{
    eb_write_text_string(book, "<em>");
    return 0;
}

EB_Error_Code hook_end_emphasize(EB_Book *book, EB_Appendix*, void*,
                                 EB_Hook_Code, int, const unsigned int*)
{
    eb_write_text_string(book, "</em>");
    return 0;
}

EB_Error_Code hook_begin_candidate(EB_Book *book, EB_Appendix*,
                                   void *container, EB_Hook_Code, int,
                                   const unsigned int*)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_candidate());
    return 0;
}

EB_Error_Code hook_begin_candidate_menu(EB_Book *book, EB_Appendix*,
                                   void *container, EB_Hook_Code, int,
                                   const unsigned int*)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_candidate_menu());
    return 0;
}

EB_Error_Code hook_end_candidate_leaf(EB_Book *book, EB_Appendix*, void*,
                                      EB_Hook_Code, int, const unsigned int*)
{
    qDebug() << "end_candidate_leaf";
    eb_write_text_string(book, "</a>");
    return 0;
}
EB_Error_Code hook_end_candidate_group(EB_Book *book, EB_Appendix*,
                                       void *container, EB_Hook_Code, int,
                                       const unsigned int *argv)
{
    //qDebug() << "end_candidate_group";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->end_candidate_group(argv[1], argv[2]));

    return 0;
}

EB_Error_Code hook_end_candidate_group_menu(EB_Book *book, EB_Appendix*,
                                            void *container, EB_Hook_Code,
                                            int, const unsigned int *argv)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book,
                         eb->end_candidate_group_menu(book, argv[1], argv[2]));

    return 0;
}


EB_Error_Code hook_begin_reference(EB_Book *book, EB_Appendix*,
                                   void *container, EB_Hook_Code, int,
                                   const unsigned int*)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_reference());
    return 0;
}
EB_Error_Code hook_end_reference(EB_Book *book, EB_Appendix*, void *container,
                                 EB_Hook_Code, int, const unsigned int *argv)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->end_reference(argv[1], argv[2]));

    return 0;
}
/*
   EB_Error_Code hook_begin_keyword(EB_Book *book, EB_Appendix*, void*,
                                 EB_Hook_Code, int, const unsigned int*)
   {
    Q_UNUSED(book);
    //eb_write_text_string(book,"<span class=key>");
    return 0;
   }
   EB_Error_Code hook_end_keyword(EB_Book *book, EB_Appendix*, void*,
                               EB_Hook_Code, int, const unsigned int*)
   {
    Q_UNUSED(book);
    //qDebug() << "End Keyword";
    //eb_write_text_string(book,"</span>");
    return 0;
   }
 */
EB_Error_Code hook_begin_decoration(EB_Book *book, EB_Appendix*,
                                    void *container, EB_Hook_Code, int argc,
                                    const unsigned int *argv)
{
    Q_UNUSED(argc);
    //qDebug() << "begin_decoration" << argc << argv[1];
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_decoration(argv[1]));

    return 0;
}
EB_Error_Code hook_end_decoration(EB_Book *book, EB_Appendix*, void *container,
                                  EB_Hook_Code, int, const unsigned int*)
{
    //qDebug() << "end_decoration" << cnt;
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->end_decoration());

    return 0;
}

EB_Error_Code hook_begin_mono_graphic(EB_Book *book, EB_Appendix*,
                                      void *container, EB_Hook_Code, int,
                                      const unsigned int *argv)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb->begin_mono_graphic(argv[2], argv[3]);
    eb_write_text_string(book, "\n");
    return 0;
}

EB_Error_Code hook_end_mono_graphic(EB_Book *book, EB_Appendix*,
                                    void *container, EB_Hook_Code, int,
                                    const unsigned int *argv)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->end_mono_graphic(book, argv[1], argv[2]));

    return 0;
}
EB_Error_Code hook_begin_color_bmp(EB_Book *book, EB_Appendix*,
                                   void *container, EB_Hook_Code, int,
                                   const unsigned int *argv)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, "\n");
    eb_write_text_string(book, eb->begin_color_bmp(book, argv[2], argv[3]));
    return 0;
}
EB_Error_Code hook_begin_color_jpeg(EB_Book *book, EB_Appendix*,
                                    void *container, EB_Hook_Code, int,
                                    const unsigned int *argv)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, "\n");
    eb_write_text_string(book, eb->begin_color_jpeg(book, argv[2], argv[3]));

    return 0;
}
EB_Error_Code hook_end_color_graphic(EB_Book *book, EB_Appendix*, void*,
                                     EB_Hook_Code, int, const unsigned int*)
{
    eb_write_text_string(book, "</span>\n");
    return 0;
}

EB_Error_Code hook_begin_in_color_bmp(EB_Book *book, EB_Appendix*,
                                      void *container, EB_Hook_Code, int,
                                      const unsigned int *argv)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_color_bmp(book, argv[2], argv[3]));
    return 0;
}

EB_Error_Code hook_begin_in_color_jpeg(EB_Book *book, EB_Appendix*,
                                       void *container, EB_Hook_Code, int,
                                       const unsigned int *argv)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_color_jpeg(book, argv[2], argv[3]));
    return 0;
}

EB_Error_Code hook_end_in_color_graphic(EB_Book *book, EB_Appendix*, void*,
                                        EB_Hook_Code, int, const unsigned int*)
{
    eb_write_text_string(book, "</span>");
    return 0;
}

EB_Error_Code hook_begin_mpeg(EB_Book *book, EB_Appendix*, void *container,
                              EB_Hook_Code, int, const unsigned int*)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_mpeg());

    return 0;
}
EB_Error_Code hook_end_mpeg(EB_Book *book, EB_Appendix*, void *container,
                            EB_Hook_Code, int, const unsigned int *argv)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb->end_mpeg(book, argv + 2);
    eb_write_text_string(book, "</a>");

    return 0;
}

EB_Error_Code hook_narrow_font(EB_Book *book, EB_Appendix*, void *container,
                               EB_Hook_Code, int, const unsigned int *argv)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->narrow_font(book, argv[0]));
    return 0;
}

EB_Error_Code hook_wide_font(EB_Book *book, EB_Appendix*, void *container,
                             EB_Hook_Code, int, const unsigned int *argv)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->wide_font(book, argv[0]));
    return 0;
}

EB_Error_Code hook_begin_wave(EB_Book *book, EB_Appendix*, void *container,
                              EB_Hook_Code, int, const unsigned int *argv)
{
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_wave(book, argv[2], argv[3],
                                              argv[4], argv[5]));

    return 0;
}
EB_Error_Code hook_end_wave(EB_Book *book, EB_Appendix*, void*, EB_Hook_Code,
                            int, const unsigned int*)
{
    eb_write_text_string(book, "</a>");
    return 0;
}

