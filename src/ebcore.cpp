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
#include <eb/binary.h>
#include <eb/text.h>
#include <eb/font.h>
#include <eb/appendix.h>
#include <eb/error.h>

#include "qeb.h"
#include "ebcore.h"
#include "textcodec.h"

EbCore::EbCore(HookMode hmode) : QEb()
{
    initializeBook();
    initializeAppendix();
    initializeHookset();
    switch(hmode) {
        case HookText :
            setHooks(hooks);
            break;
        case HookMenu :
            setHooks(hooks_cand);
            break;
        case HookFont :
            setHooks(hooks_font);
            break;
        default:
            qWarning() << "Unrecognize Hook Mode" << hmode;
    }
}

EbCore::~EbCore()
{
    finalizeHookset();
    finalizeAppendix();
    finalizeBook();
}

int EbCore::initBook(const QString& path, int subbook, int refpos)
{
    EB_Error_Code ecode;

    ecode = bind(path);
    if (ecode != EB_SUCCESS) {
        return -1;
    }
    bindAppendix(path);

    subBookList =  subbookList();
    if (subBookList.count() == 0) {
        return -1;
    }

    subAppendixList = appendixSubbookList();

    if (subbook >= 0) {
        return initSubBook(subbook, refpos);
    } else {
        return subBookList.count();
    }
}

int EbCore::initSubBook(int index, int refpos)
{
    EB_Error_Code ecode;

    refPosition = refpos;

    if (index >= subBookList.count()){
        return -1;
    }
    ecode = setSubbook(subBookList[index]);
    if (ecode != EB_SUCCESS) {
        return -1;
    }
    if (isHaveAppendixSubbook(subAppendixList[index])) {
        setAppendixSubbook(subAppendixList[index]);
    }

    EB_Character_Code ccode = characterCode();
    if (ccode == EB_CHARCODE_ISO8859_1) {
        qWarning() << "Using ISO 8859-1";
    //else if (ccode == EB_CHARCODE_JISX0208) {
        //    qWarning() << "Using JIS X 0208";
    } else if (ccode == EB_CHARCODE_JISX0208_GB2312) {
        qWarning() << "Using X 0208 + GB 2312";
    } else if (ccode == EB_CHARCODE_INVALID) {
        qWarning() << "Using Invalid Character Code";
    }
    if (isHaveFont(EB_FONT_16)) {
        setFont(EB_FONT_16);
    }
    return 1;
}

QString EbCore::getCopyright()
{
    if (!isHaveCopyright())
        return QString();

    EB_Position position = copyright();
    if (!isValidPosition(position)) {
        return QString();
    }
    return text(position);
}


QString EbCore::getMenu()
{
    if (!isHaveMenu())
        return QString();

    EB_Position position = menu();
    if (!isValidPosition(position)) {
        return QString();
    }
    return text(position);
}

QList <CandItem> EbCore::candidate(const EB_Position &pos, QString *txt)
{
   candList.clear();
   *txt = text(pos);
   return candList;
}

QString EbCore::text(const EB_Position &pos, bool hflag)
{

    refList.clear();

    if (seekText(pos) != EB_SUCCESS) {
        return QString();
    }

    currentIndent = false;
    QString str = readText((void*)this,hflag);
    if (hflag) {
        if (currentIndent)
            str = str + "</pre>";
        for (int i = 0; i < refList.count(); i++) {
            QString f = "<R" + numToStr(i) + "R>";
            str.replace(f, refList[i]);
        }
        for (int i = 0; i < mpegList.count(); i++) {
            QString f = "<M" + numToStr(i) + "M>";
            str.replace(f, mpegList[i]);
        }
    }
    str = str.trimmed();

    return str;
}

QString EbCore::heading(const EB_Position &pos, bool hflag)
{

    if (seekText(pos) != EB_SUCCESS) {
        return QString();
    }

    currentIndent = false;
    QString str = readHeading((void*)this, hflag);
    if (hflag) {
        for (int i = 0; i < refList.count(); i++) {
            QString f = "<R" + numToStr(i) + "R>";
            str.replace(f, refList[i]);
        }
    }
    return str.trimmed();
}

QByteArray EbCore::errorBStr(const QByteArray &error_message)
{
    qWarning() << subbookTitle() << ':' << error_message;
    return "<em class=err>" + error_message + "</em>";
}

QByteArray EbCore::fontToBStr(int code, NarrowOrWide n_or_w)
{
    if (!fontList)
        return QByteArray();

    QChar c = (n_or_w == NarrowCode) ? 'n' : 'w';
    QString afont = fontList->value(c + numToStr(code, 16));

    if (!afont.isEmpty())
        return afont.toUtf8();
    else
        return QByteArray();
}
QByteArray EbCore::fontToHtmlBStr(const QByteArray &fname,
                                    NarrowOrWide)
{
    QByteArray ret = "<img src=\"" + ebCache.fontCacheRel.toUtf8() + fname  +
                     "\"";

#if 0
    if (fontSize != 16) {
        int h = fontSize;
        int w = (n_or_w == NarrowCode) ? (h / 2) : h;
        ret += " height=" + numToBStr(h) +
               " width=" + numToBStr(w);
    }
#endif
    ret += " />";
    return ret;
}

bool EbCore::makeBinaryFile(const QString &fname, const QByteArray &data)
{
    QFile f(fname);
    if (!f.open(QIODevice::WriteOnly)) {
        qWarning() << "File Open Error: " << fname << " code: " << f.error();
        return false;
    }
    if (f.write(data) == -1) {
        qWarning() << "File Write Error: " << fname << " code: " << f.error();
        return false;
    }
    return true;
}

// Hook Callbacks
QByteArray EbCore::hookInitialize(int, const unsigned int*)
{
    qDebug() << "HOOK_INITIALIZE";
    return QByteArray();
}
QByteArray EbCore::hookBeginNarrow(int, const unsigned int*)
{
    qDebug() << "HOOK_BEGIN_NARROW";
    return QByteArray();
}
QByteArray EbCore::hookEndNarrow(int, const unsigned int*)
{
    qDebug() << "HOOK_END_NARROW";
    return QByteArray();
}
QByteArray EbCore::hookBeginSubscript(int, const unsigned int*)
{
    return (ruby) ? "<sub>" : QByteArray();
}
QByteArray EbCore::hookEndSubscript(int, const unsigned int*)
{
    return (ruby) ? "</sub>" : QByteArray();
}
QByteArray EbCore::hookSetIndent(int, const unsigned int* argv)
{
    int indent = argv[1];
    QByteArray ret;
    if (indent > 1) {
        if (currentIndent)
            ret = "</pre>";
        int left = (indent-1) * fontSize;
        ret = ret + "<pre style=\"margin-left:" + numToBStr(left) + "px;\">";
        currentIndent = true;
    } else {
        if (currentIndent) {
            ret = "</pre>";
        }
        currentIndent = false;
    }

    return ret;
}

QByteArray EbCore::hookNewline(int, const unsigned int*)
{
    return "<br>";
}
QByteArray EbCore::hookBeginSuperscript(int, const unsigned int*)
{
    return "<sup>";
}
QByteArray EbCore::hookEndSuperscript(int, const unsigned int*)
{
    return "</sup>";
}
QByteArray EbCore::hookBeginNoNewline(int, const unsigned int*)
{
    qDebug() << "HOOK_BEGIN_NO_NEWLINE";
    return QByteArray();
}
QByteArray EbCore::hookEndNoNewline(int, const unsigned int*)
{
    qDebug() << "HOOK_END_NO_NEWLINE";
    return QByteArray();
}
QByteArray EbCore::hookBeginEmphasis(int, const unsigned int*)
{
    return "<em>";
}
QByteArray EbCore::hookEndEmphasis(int, const unsigned int*)
{
    return "</em>";
}
QByteArray EbCore::hookBeginCandidate(int, const unsigned int*)
{
    return "<a class=cnd href=\"menu?<R" + numToBStr(refList.count()) +
           "R>\">";
}
QByteArray EbCore::hookEndCandidateGroup(int, const unsigned int *argv)
{
    refList << numToBStr(refPosition) + '?' + numToBStr(argv[1]) + '?' +
               numToBStr(argv[2]);
    return "</a>";
}
QByteArray EbCore::hookEndCandidateGroupMENU(int, const unsigned int *argv)
{
    CandItem cItem;
    cItem.title = currentCandidate();
    cItem.position.page = argv[1];
    cItem.position.offset = argv[2];
    candList << cItem;
    return QByteArray();
}
QByteArray EbCore::hookEndCandidateLeaf(int, const unsigned int*)
{
    return errorBStr("HOOK END_CANDIDATE_LEF Not Supported.");
}
QByteArray EbCore::hookBeginReference(int, const unsigned int*)
{
    return "<a class=ref href=\"<R" + numToBStr(refList.count()) +
           "R>\">";
}
QByteArray EbCore::hookEndReference(int, const unsigned int *argv)
{
    QByteArray ref = "book?" + numToBStr(refPosition) + '?' +
                     numToBStr(argv[1]) + '?' + numToBStr(argv[2]);
    refList << ref;
    return "</a>";
}
QByteArray EbCore::hookBeginKeyword(int, const unsigned int*)
{
    return "<span class=key>";
}
QByteArray EbCore::hookEndKeyword(int, const unsigned int*)
{
    return "</span>";
}
QByteArray EbCore::hookNarrowFont(int, const unsigned int *argv)
{
    int fcode = argv[0];
    QByteArray fstr = fontToBStr(fcode, NarrowCode);
    if (!fstr.isEmpty())
        return fstr;

    if (!isHaveNarrowFont())
        return errorBStr("No Narrow Font");

#ifdef USE_GIF_FOR_FONT
    QByteArray fname = 'n' + numToBStr(fcode,16) + ".gif";
#else
    QByteArray fname = 'n' + numToBStr(fcode,16) + ".png";
#endif

    QByteArray bitmap = narrowFontCharacterBitmap(fcode);
    if (bitmap.isEmpty())
        return errorBStr("Font Extract Error");

#ifdef USE_GIF_FOR_FONT
    QByteArray cnv = narrowBitmapToGif(bitmap);
#else
    QByteArray cnv = narrowBitmapToPng(bitmap);
#endif
    if (cnv.isEmpty())
        return errorBStr("Font Conversion Error");

    if(!makeBinaryFile(ebCache.fontCachePath + '/' + fname, cnv)) {
        return errorBStr("Font Write Error");
    }
    ebCache.fontCacheList << fname;
    //qDebug() << "Output narrow_font" << ebCache.fontCachePath << fname;

    return fontToHtmlBStr(fname, NarrowCode);

}

QByteArray EbCore::hookWideFont(int, const unsigned int *argv)
{
    int fcode = argv[0];
    QByteArray fstr = fontToBStr(fcode, WideCode);
    if (!fstr.isEmpty())
        return fstr;

    if (!isHaveWideFont())
        return errorBStr("No Wide Font");

#ifdef USE_GIF_FOR_FONT
    QByteArray fname = 'w' + numToBStr(fcode,16) + ".gif";
#else
    QByteArray fname = 'w' + numToBStr(fcode,16) + ".png";
#endif

    QByteArray bitmap = wideFontCharacterBitmap(fcode);
    if (bitmap.isEmpty())
        return errorBStr("Font Extract Error");

#ifdef USE_GIF_FOR_FONT
    QByteArray cnv = wideBitmapToGif(bitmap);
#else
    QByteArray cnv = wideBitmapToPng(bitmap);
#endif
    if (cnv.isEmpty())
        return errorBStr("Font Conversion Error");

    if(!makeBinaryFile(ebCache.fontCachePath + '/' + fname, cnv)) {
        return errorBStr("Font Write Error");
    }
    ebCache.fontCacheList << fname;
    //qDebug() << "Output wide" << ebCache.fontCachePath << fname;

    return fontToHtmlBStr(fname, WideCode);
}
QByteArray EbCore::hookISO8859_1(int, const unsigned int*)
{
    qWarning() << "HOOK_ISO08858_1";
    return QByteArray();
}
QByteArray EbCore::hookNarrowJISX0208(int argc, const unsigned int *argv)
{
    if (*argv == 41443) {
        return "&lt;";
    } else if (*argv == 41444) {
        return "&gt;";
    } else if (*argv == 41461) {
        return "&amp;";
    } else if (*argv == 41382) {
        // gets converted to garbage otherwise
        return "·";
    } else {
        eb_hook_euc_to_ascii(&book, &appendix, (void*)this,
                             EB_HOOK_NARROW_JISX0208, argc, argv);
        return QByteArray();
    }

}
QByteArray EbCore::hookWideJISX0208(int, const unsigned int *argv)
{
    char code[3];
    code[0] = argv[0] >> 8;
    code[1] = argv[0] & 0xff;
    code[2] = '\0';
    return QByteArray(eucToUtf(code).toUtf8());
}
QByteArray EbCore::hookGB2312(int, const unsigned int*)
{
    return errorBStr("HOOK_GB2312 Not Supported");
}
QByteArray EbCore::hookBeginMonoGraphic(int, const unsigned int *argv)
{
    monoWidth = argv[2];
    monoHeight = argv[3];
    return "\n";
}
QByteArray EbCore::hookEndMonoGraphic(int, const unsigned int *argv)
{
    QByteArray fname = binaryFname("bmp", argv[1], argv[2]);
    QByteArray out = "<img src=\"" + ebCache.imageCacheRel.toUtf8() +
                     fname + "\" />\n";
    if (ebCache.imageCacheList.contains(fname))
        return out;

    EB_Position pos;
    pos.page = argv[1];
    pos.offset = argv[2];
    if (setBinaryMonoGraphic(pos, monoWidth, monoHeight) != EB_SUCCESS)
        return errorBStr("Image(Mono) Error");

    QByteArray image = readBinary();
    if (image.isEmpty())
        return errorBStr("Image(Mono) Error");

    if(!makeBinaryFile(ebCache.imageCachePath + '/' + fname, image))
        return errorBStr("Image(Mono) Write Error");

    ebCache.imageCacheList << fname;

    return out;
}
QByteArray EbCore::hookBeginGrayGraphic(int, const unsigned int*)
{
    return errorBStr("HOOK_BEGIN_GRAY_GRAPHIC Not Supported");
}
QByteArray EbCore::hookEndGrayGraphic(int, const unsigned int*)
{
    return errorBStr("HOOK_END_GRAY_GRAPHIC Not Supported");
}
QByteArray EbCore::hookBeginColorBmp(int, const unsigned int *argv)
{
    QByteArray fname = binaryFname("bmp", argv[2], argv[3]);
    QByteArray out = "<img src=\"" + ebCache.imageCacheRel.toUtf8() +
                     fname + "\" /><span class=img>";
    if (ebCache.imageCacheList.contains(fname))
        return out;

    EB_Position pos;
    pos.page = argv[2];
    pos.offset = argv[3];
    if (setBinaryColorGraphic(pos) != EB_SUCCESS)
        return errorBStr("Image(bmp) Error") + "<span class=img>";

    QByteArray image = readBinary();
    if (image.isEmpty())
        return errorBStr("Image(bmp) Error") + "<span class=img>";

    if(!makeBinaryFile(ebCache.imageCachePath + '/' + fname, image))
        return errorBStr("Image(bmp) Write Error") + "<span class=img>";

    ebCache.imageCacheList << fname;

    return out;

}
QByteArray EbCore::hookBeginColorJpeg(int, const unsigned int *argv)
{
    QByteArray fname = binaryFname("jpeg", argv[2], argv[3]);
    QByteArray out = "<img src=\"" + ebCache.imageCacheRel.toUtf8() +
                     fname + "\" /><span class=img>";
    if (ebCache.imageCacheList.contains(fname))
        return out;

    EB_Position pos;
    pos.page = argv[2];
    pos.offset = argv[3];
    if (setBinaryColorGraphic(pos) != EB_SUCCESS)
        return errorBStr("Image(jpeg) Error") + "<span class=img>";

    QByteArray image = readBinary();
    if (image.isEmpty())
        return errorBStr("Image(jpeg) Error") + "<span class=img>";

    if(!makeBinaryFile(ebCache.imageCachePath + '/' + fname, image))
        return errorBStr("Image(jpeg) Write Error") + "<span class=img>";

    ebCache.imageCacheList << fname;

    return out;
}
QByteArray EbCore::hookBeginInColorBmp(int argc, const unsigned int *argv)
{
    QByteArray b = hookBeginColorBmp(argc, argv);

    return '\n' + b;
}
QByteArray EbCore::hookBeginInColorJpeg(int argc, const unsigned int *argv)
{
    QByteArray b = hookBeginColorJpeg(argc, argv);

    return '\n' + b;
}
QByteArray EbCore::hookEndColorGraphic(int, const unsigned int*)
{
    return "</span>\n";
}
QByteArray EbCore::hookEndInColorGraphic(int, const unsigned int*)
{
    return "</span>\n";
}
QByteArray EbCore::hookBeginWave(int, const unsigned int *argv)
{
    // argv[2] : start page
    // qrgv[3] : start offset
    // qrgv[4] : end page
    // argv[5] : end offset
    QByteArray fname = binaryFname("wav", argv[2], argv[3]);
    QByteArray out = "<a class=snd href=\"sound?" +
                     ebCache.waveCachePath.toUtf8() + "\">";
    if (ebCache.waveCacheList.contains(fname))
        return out;

    EB_Position spos;
    spos.page = argv[2];
    spos.offset = argv[3];
    EB_Position epos;
    epos.page = argv[4];
    epos.offset = argv[5];
    if (setBinaryWave(spos,epos) != EB_SUCCESS)
        return errorBStr("Image(wave) Error") + "<a>";

    QByteArray image = readBinary();
    if (image.isEmpty())
        return errorBStr("Image(wave) Error") + "<a>";

    if(!makeBinaryFile(ebCache.waveCachePath + '/' + fname, image))
        return errorBStr("Image(wave) Write Error") + "<a>";

    ebCache.waveCacheList << fname;

    return out;
}
QByteArray EbCore::hookEndWave(int, const unsigned int*)
{
    return "</a>";
}
QByteArray EbCore::hookBeginMpeg(int, const unsigned int*)
{
    return "<a class=mpg href=\"mpeg?<M" + numToBStr(mpegList.count()) +
           "M>\">";
}
QByteArray EbCore::hookEndMpeg(int, const unsigned int *argv)
{
    QString path = composeMoviePathName(argv+2);
    if (path.isEmpty())
        return errorBStr("Image(mpeg) Error");

    QString fname = QFileInfo(path).fileName() + ".mpeg";
    QString dst_file = ebCache.mpegCachePath + '/' + fname;

    if (!ebCache.mpegCacheList.contains(fname)) {
        if (!QFile(dst_file).exists())
            QFile::copy(path, dst_file);
        ebCache.mpegCacheList << fname;
    }
    mpegList << dst_file.toUtf8();
    return QByteArray();
}
QByteArray EbCore::hookBeginGraphicReference(int, const unsigned int*)
{
    return errorBStr("HOOK_BEGIN_GRAPHIC_REFERECE Not Supported");
}
QByteArray EbCore::hookEndGraphicReference(int, const unsigned int*)
{
    return errorBStr("HOOK_END_GRAPHIC_REFERECE Not Supported");
}
QByteArray EbCore::hookGraphicReference(int, const unsigned int*)
{
    return errorBStr("HOOK_GRAPHIC_REFERECE Not Supported");
}
QByteArray EbCore::hookBeginDecoration(int, const unsigned int* argv)
{
    int code = argv[1];
    decorationStack.push(code);
    if (code == 1) {
        return "<i>";
    } else if (code == 3) {
        return "<b>";
    } else {
        qWarning() << "Unrecognized Decoration Code" << code;
        return "<i>";
    }
}
QByteArray EbCore::hookEndDecoration(int, const unsigned int*)
{
    if (decorationStack.isEmpty()) {
        return QByteArray();
    }
    int code = decorationStack.pop();
    if (code == 1) {
        return "</i>";
    } else if (code == 3) {
        return "</b>";
    } else {
        return "</i>";
    }
}
QByteArray EbCore::hookBeginImagePage(int, const unsigned int*)
{
    return errorBStr("HOOK_BEGIN_IMAGE_PAGE Not Supported");
}
QByteArray EbCore::hookEndImagePage(int, const unsigned int*)
{
    return errorBStr("HOOK_END_IMAGE_PAGE Not Supported");
}
QByteArray EbCore::hookBeginClickableArea(int, const unsigned int*)
{
    return errorBStr("HOOK_BEGIN_CLICKABLE_AREA Not Supported");
}
QByteArray EbCore::hookEndClickableArea(int, const unsigned int*)
{
    return errorBStr("HOOK_END_CLICKABLE_AREA Not Supported");
}

