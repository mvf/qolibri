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
#include <eb/error.h>
#include <eb/font.h>
#include <eb/binary.h>
#include "qeb.h"
#include "ebook.h"
#include "ebhook.h"
#include "textcodec.h"

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

QByteArray EbHook::narrow_font(int code)
{
    if (!eb->isHaveNarrowFont()) {
        qDebug() << "not have narrow font";
        return errorString("narrow font error");
    }

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

    QByteArray out = "<img src=\"" + utfToEuc(ebCache.fontCacheRel) + fname  +
                     "\"";
    int h = fontSize;
    if (h > 17) {
        out += " height=" + QByteArray::number(h) +
               " width=" + QByteArray::number(h / 2);
    }
    out += " />";

    if (ebCache.fontCacheList.contains(fname))
        return out;

    QByteArray bitmap = eb->narrowFontCharacterBitmap(code);
    if (bitmap.isNull())
        return errorString("narrow font error");

#ifdef USE_GIF_FOR_FONT
    QByteArray cnv = eb->narrowBitmapToGif(bitmap);
    if (cnv.isNull())
        return errorString("narrow font error");

#else
    QByteArray cnv = eb->narrowBitmapToPng(bitmap);
    if (cnv.isNull())
        return errorString("narrow font error");
#endif


    QFile f(ebCache.fontCachePath +  '/' + fname);
    f.open(QIODevice::WriteOnly);
    f.write(cnv);
    f.close();
    //qDebug() << "Output narrow_font" << fname;
    ebCache.fontCacheList << fname;

    return out;
}

QByteArray EbHook::wide_font(int code)
{

    if (!eb->isHaveWideFont()) {
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

    QByteArray out = "<img src=\"" + utfToEuc(ebCache.fontCacheRel) +
                     fname  + "\"";
    int h = fontSize;
    if (h > 17)
        out += " height=" + QByteArray::number(h) +
               " width=" + QByteArray::number(h);
    out += " />";


    if (ebCache.fontCacheList.contains(fname)) {
        return out;
    }

    QByteArray bitmap = eb->wideFontCharacterBitmap(code);
    if (bitmap.isNull())
        return errorString("wide font error");

#ifdef USE_GIF_FOR_FONT
    QByteArray cnv = eb->wideBitmapToGif(bitmap);
    if (cnv.isNull()) {
        return errorString("wide font error");
    }
#else
    QByteArray cnv = eb->wideBitmapToPng(bitmap);
    if (cnv.isNull()) {
        return errorString("wide font error");
    }
#endif

    QFile f(ebCache.fontCachePath + '/' + fname);
    f.open(QIODevice::WriteOnly);
    f.write(cnv);
    f.close();
    ebCache.fontCacheList << fname;
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

QByteArray EbHook::begin_color_jpeg(int page, int offset)
{

    QByteArray jpgFile = makeFname("jpeg", page, offset);
    QByteArray out = "<img src=\"" + utfToEuc(ebCache.imageCacheRel) +
                     jpgFile + "\"><span class=img>";

    if (ebCache.imageCacheList.contains(jpgFile)) {
        return out;
    }

    //qDebug() << "Out Image " << jpgFile;

    EB_Position pos;
    EB_Error_Code err;
    pos.page = page;
    pos.offset = offset;

    if ((err = eb->setBinaryColorGraphic(pos)) != EB_SUCCESS) {
        return errorString("image(jpeg) error");
    }

    QByteArray b = eb->readBinary();
    if (b.isNull()) {
        return errorString("read binary error");
    }
    
    QFile f(ebCache.imageCachePath + '/' + jpgFile);
    f.open(QIODevice::WriteOnly);
    f.write(b);
    f.close();

    ebCache.imageCacheList << jpgFile;

    return out;
}

QByteArray EbHook::begin_color_bmp(int page, int offset)
{
    EB_Error_Code ecode;

    QByteArray bmpFile = makeFname("bmp", page, offset);
    QByteArray out = "<img src=\"" + utfToEuc(ebCache.imageCacheRel) +
                     bmpFile + "\" /><span class=img>";

    if (ebCache.imageCacheList.contains(bmpFile))
        return out;

    //qDebug() << "Output Image " << bmpFile;

    EB_Position pos;
    pos.page = page;
    pos.offset = offset;

    if ((ecode = eb->setBinaryColorGraphic(pos)) != EB_SUCCESS) {
        return errorString("image(bmp) error");
    }

    QByteArray b = eb->readBinary();
    if (b.isNull()) {
        return errorString("read binary error");
    }

    QFile f(ebCache.imageCachePath + '/' + bmpFile);
    f.open(QIODevice::WriteOnly);
    f.write(b);
    f.close();

    //qDebug() << "Image Size :" << ImageBufferSize * flg;
    ebCache.imageCacheList << bmpFile;

    return out;
}

QByteArray EbHook::end_reference(int page, int offset)
{
    QByteArray ref = "book?" + QByteArray::number(refPosition) + '?' +
                     QByteArray::number(page) + '?' +
                     QByteArray::number(offset);

    refList << ref;
    return "</a>";
}

QByteArray EbHook::end_candidate_group(int page, int offset)
{
    QByteArray cnd = "menu?" + QByteArray::number(refPosition) + '?' +
                     QByteArray::number(page) + '?' +
                     QByteArray::number(offset);

    refList << cnd;
    return "</a>";
}

void EbHook::end_candidate_group_menu(int page, int offset)
{
    CandItems cItem;
    cItem.title = eb->currentCandidate();
    cItem.position.page = page;
    cItem.position.offset = offset;
    candList << cItem;

    return;
}

QByteArray EbHook::begin_mpeg()
{
    return "<a class=mpg href=\"<M" + QByteArray::number(mpegList.count()) +
           "M>\">";
}

void EbHook::end_mpeg(const unsigned int *p)
{

    QString path = eb->composeMoviePathName(p);
    if (path.isEmpty()) {
        return;
    }
    QString dfile = ebCache.mpegCachePath + "/" +
                    QFileInfo(path).fileName() + ".mpeg";
    if (!QFile(dfile).exists())
        QFile::copy(path, dfile);
    mpegList << "mpeg?" + utfToEuc(dfile);
}

QByteArray EbHook::end_mono_graphic(int page, int offset)
{
    EB_Error_Code ecode;

    QByteArray bmpFile = makeFname("bmp", page, offset);

    QByteArray out = "<img src=\"" + utfToEuc(ebCache.imageCacheRel) +
                     bmpFile + "\" />\n";
    

    if (ebCache.imageCacheList.contains(bmpFile)) {
        return out;
    }

    EB_Position pos;
    pos.page = page;
    pos.offset = offset;
    //qDebug() << eb->monoWidth << " " << eb->monoHeight;
    if ((ecode = eb->setBinaryMonoGraphic(pos, monoWidth,
                                            monoHeight)) != EB_SUCCESS) {
        return errorString("image(mono) error");
    }


    QByteArray b = eb->readBinary();
    if (b.isNull()) {
        return errorString("read binary error");
    }

    QFile f(ebCache.imageCachePath + '/' + bmpFile);
    f.open(QIODevice::WriteOnly);
    f.write(b);
    f.close();
    ebCache.imageCacheList << bmpFile;

    return out;
}

QByteArray EbHook::begin_wave(int start_page, int start_offset,
                              int end_page, int end_offset)
{

    QString wavFile = QString("%1x%2.wav").arg(start_page).arg(start_offset);
    QString fname = ebCache.waveCachePath + "/" + wavFile;
    QString out =  QString("<a class=snd href=\"sound?%1\">").arg(fname);

    if (ebCache.waveCacheList.contains(wavFile))
        return utfToEuc(out);

    EB_Position spos, epos;
    spos.page = start_page;
    spos.offset = start_offset;
    epos.page = end_page;
    epos.offset = end_offset;
    if (eb->setBinaryWave(spos, epos) != EB_SUCCESS) {
        return errorString("image(wave) error");
    }

    QByteArray b = eb->readBinary();
    if (b.isNull()) {
        return errorString("read binary error");
    }

    QFile f(fname);
    f.open(QIODevice::WriteOnly);
    f.write(b);
    f.close();
    ebCache.waveCacheList << wavFile;
    //qDebug() << "Output wave" << fname;
    return utfToEuc(out);
}

/*
   EB_Error_Code hook_newline(EB_Book *book, EB_Appendix*, void*,
                           EB_Hook_Code, int, const unsigned int*)
   {
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
    qDebug() << "1";
    if (*argv == 41443) {
        eb_write_text_string(book, "&lt;");
    } else if (*argv == 41444) {
        eb_write_text_string(book, "&gt;");
    } else if (*argv == 41461) {
        eb_write_text_string(book, "&amp;");
    } else {
        eb_hook_euc_to_ascii(book, appendix, container, code, argc, argv);
    }
    qDebug() << "2";
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
    qDebug() << "3";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->set_indent(argv[1]));
    qDebug() << "4";
    return 0;
}

EB_Error_Code hook_begin_subscript(EB_Book *book, EB_Appendix*, void *container,
                                   EB_Hook_Code, int, const unsigned int*)
{
    qDebug() << "5";
    EbHook *eb = static_cast<EbHook*>(container);

    //eb_write_text_string(book, "<sub>");
    eb_write_text_string(book, eb->begin_subscript());
    qDebug() << "6";
    return 0;
}

EB_Error_Code hook_end_subscript(EB_Book *book, EB_Appendix*, void *container,
                                 EB_Hook_Code, int, const unsigned int*)
{
    qDebug() << "7";
    EbHook *eb = static_cast<EbHook*>(container);

    //eb_write_text_string(book, "</sub>");
    eb_write_text_string(book, eb->end_subscript());
    qDebug() << "8";
    return 0;
}

EB_Error_Code hook_begin_superscript(EB_Book *book, EB_Appendix*, void*,
                                     EB_Hook_Code, int, const unsigned int*)
{
    qDebug() << "9";
    eb_write_text_string(book, "<sup>");
    qDebug() << "a";
    return 0;
}

EB_Error_Code hook_end_superscript(EB_Book *book, EB_Appendix*, void*,
                                   EB_Hook_Code, int, const unsigned int*)
{
    qDebug() << "b";
    eb_write_text_string(book, "</sup>");
    qDebug() << "c";
    return 0;
}

EB_Error_Code hook_begin_emphasize(EB_Book *book, EB_Appendix*, void*,
                                   EB_Hook_Code, int, const unsigned int*)
{
    qDebug() << "d";
    eb_write_text_string(book, "<em>");
    qDebug() << "e";
    return 0;
}

EB_Error_Code hook_end_emphasize(EB_Book *book, EB_Appendix*, void*,
                                 EB_Hook_Code, int, const unsigned int*)
{
    qDebug() << "f";
    eb_write_text_string(book, "</em>");
    qDebug() << "g";
    return 0;
}

EB_Error_Code hook_begin_candidate(EB_Book *book, EB_Appendix*,
                                   void *container, EB_Hook_Code, int,
                                   const unsigned int*)
{
    qDebug() << "-7";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_candidate());
    qDebug() << "-7";
    return 0;
}

EB_Error_Code hook_begin_candidate_menu(EB_Book *, EB_Appendix*,
                                   void *, EB_Hook_Code, int,
                                   const unsigned int*)
{
    qDebug() << "-7";
    //EbHook *eb = static_cast<EbHook*>(container);

    //eb_write_text_string(book, eb->begin_candidate_menu());
    qDebug() << "-7";
    return 0;
}

EB_Error_Code hook_end_candidate_leaf(EB_Book *book, EB_Appendix*, void*,
                                      EB_Hook_Code, int, const unsigned int*)
{
    qDebug() << "-7";
    qDebug() << "end_candidate_leaf";
    eb_write_text_string(book, "</a>");
    qDebug() << "-7";
    return 0;
}
EB_Error_Code hook_end_candidate_group(EB_Book *book, EB_Appendix*,
                                       void *container, EB_Hook_Code, int,
                                       const unsigned int *argv)
{
    //qDebug() << "end_candidate_group";
    qDebug() << "-7";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->end_candidate_group(argv[1], argv[2]));
    qDebug() << "-7";

    return 0;
}

EB_Error_Code hook_end_candidate_group_menu(EB_Book *, EB_Appendix*,
                                            void *container, EB_Hook_Code,
                                            int, const unsigned int *argv)
{
    qDebug() << "-7";
    EbHook *eb = static_cast<EbHook*>(container);

    eb->end_candidate_group_menu(argv[1], argv[2]);
    qDebug() << "-7";

    return 0;
}


EB_Error_Code hook_begin_reference(EB_Book *book, EB_Appendix*,
                                   void *container, EB_Hook_Code, int,
                                   const unsigned int*)
{
    qDebug() << "-7";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_reference());
    qDebug() << "-7";
    return 0;
}
EB_Error_Code hook_end_reference(EB_Book *book, EB_Appendix*, void *container,
                                 EB_Hook_Code, int, const unsigned int *argv)
{
    qDebug() << "-7";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->end_reference(argv[1], argv[2]));
    qDebug() << "-7";

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
    qDebug() << "-7";
    Q_UNUSED(argc);
    //qDebug() << "begin_decoration" << argc << argv[1];
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_decoration(argv[1]));
    qDebug() << "-7";

    return 0;
}
EB_Error_Code hook_end_decoration(EB_Book *book, EB_Appendix*, void *container,
                                  EB_Hook_Code, int, const unsigned int*)
{
    //qDebug() << "end_decoration" << cnt;
    qDebug() << "-7";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->end_decoration());
    qDebug() << "-7";

    return 0;
}

EB_Error_Code hook_begin_mono_graphic(EB_Book *book, EB_Appendix*,
                                      void *container, EB_Hook_Code, int,
                                      const unsigned int *argv)
{
    qDebug() << "-7";
    EbHook *eb = static_cast<EbHook*>(container);

    eb->begin_mono_graphic(argv[2], argv[3]);
    eb_write_text_string(book, "\n");
    qDebug() << "-7";
    return 0;
}

EB_Error_Code hook_end_mono_graphic(EB_Book *book, EB_Appendix*,
                                    void *container, EB_Hook_Code, int,
                                    const unsigned int *argv)
{
    qDebug() << "-7";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->end_mono_graphic(argv[1], argv[2]));
    qDebug() << "-7";

    return 0;
}
EB_Error_Code hook_begin_color_bmp(EB_Book *book, EB_Appendix*,
                                   void *container, EB_Hook_Code, int,
                                   const unsigned int *argv)
{
    qDebug() << "-7";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, "\n");
    eb_write_text_string(book, eb->begin_color_bmp(argv[2], argv[3]));
    qDebug() << "-7";
    return 0;
}
EB_Error_Code hook_begin_color_jpeg(EB_Book *book, EB_Appendix*,
                                    void *container, EB_Hook_Code, int,
                                    const unsigned int *argv)
{
    qDebug() << "-7";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, "\n");
    eb_write_text_string(book, eb->begin_color_jpeg(argv[2], argv[3]));
    qDebug() << "-7";

    return 0;
}
EB_Error_Code hook_end_color_graphic(EB_Book *book, EB_Appendix*, void*,
                                     EB_Hook_Code, int, const unsigned int*)
{
    qDebug() << "-7";
    eb_write_text_string(book, "</span>\n");
    qDebug() << "-7";
    return 0;
}

EB_Error_Code hook_begin_in_color_bmp(EB_Book *book, EB_Appendix*,
                                      void *container, EB_Hook_Code, int,
                                      const unsigned int *argv)
{
    qDebug() << "-7";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_color_bmp(argv[2], argv[3]));
    qDebug() << "-7";
    return 0;
}

EB_Error_Code hook_begin_in_color_jpeg(EB_Book *book, EB_Appendix*,
                                       void *container, EB_Hook_Code, int,
                                       const unsigned int *argv)
{
    qDebug() << "-9";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_color_jpeg(argv[2], argv[3]));
    qDebug() << "-7";
    return 0;
}

EB_Error_Code hook_end_in_color_graphic(EB_Book *book, EB_Appendix*, void*,
                                        EB_Hook_Code, int, const unsigned int*)
{
    qDebug() << "-8";
    eb_write_text_string(book, "</span>");
    qDebug() << "-7";
    return 0;
}

EB_Error_Code hook_begin_mpeg(EB_Book *book, EB_Appendix*, void *container,
                              EB_Hook_Code, int, const unsigned int*)
{
    qDebug() << "-6";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_mpeg());
    qDebug() << "-7";

    return 0;
}
EB_Error_Code hook_end_mpeg(EB_Book *book, EB_Appendix*, void *container,
                            EB_Hook_Code, int, const unsigned int *argv)
{
    qDebug() << "-5";
    EbHook *eb = static_cast<EbHook*>(container);

    eb->end_mpeg(argv + 2);
    eb_write_text_string(book, "</a>");
    qDebug() << "-7";

    return 0;
}

EB_Error_Code hook_narrow_font(EB_Book *book, EB_Appendix*, void *container,
                               EB_Hook_Code, int, const unsigned int *argv)
{
    qDebug() << "-4";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->narrow_font(argv[0]));
    qDebug() << "-7";
    return 0;
}

EB_Error_Code hook_wide_font(EB_Book *book, EB_Appendix*, void *container,
                             EB_Hook_Code, int, const unsigned int *argv)
{
    qDebug() << "-3";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->wide_font(argv[0]));
    qDebug() << "-7";
    return 0;
}

EB_Error_Code hook_begin_wave(EB_Book *book, EB_Appendix*, void *container,
                              EB_Hook_Code, int, const unsigned int *argv)
{
    qDebug() << "-2";
    EbHook *eb = static_cast<EbHook*>(container);

    eb_write_text_string(book, eb->begin_wave(argv[2], argv[3],
                                              argv[4], argv[5]));
    qDebug() << "-7";

    return 0;
}
EB_Error_Code hook_end_wave(EB_Book *book, EB_Appendix*, void*, EB_Hook_Code,
                            int, const unsigned int*)
{
    qDebug() << "-1";
    eb_write_text_string(book, "</a>");
    qDebug() << "-7";
    return 0;
}

