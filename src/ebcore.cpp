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
#include "ebcore.h"

#include <QFile>
#include <QFileInfo>

#include <ebu/eb.h>
#include <ebu/binary.h>
#include <ebu/text.h>
#include <ebu/font.h>
#include <ebu/appendix.h>
#include <ebu/error.h>


EbCore::EbCore(HookMode hmode) : QEb()
{
    initializeBook();
    initializeAppendix();
    initializeHookset();
    initializeEucWideToUtfNarrow();
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

void EbCore::initializeEucWideToUtfNarrow()
{
    QFile file(":/data/euc-wide-to-utf-narrow");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString line = in.readLine();
    while (!line.isNull()) {
        QStringList list = line.remove('\n').split(' ');
        if (list.count() == 2) {
            eucWideToUtfNarrow.insert(list[0].toUInt(NULL, 16), list[1]);
        }
        line = in.readLine();
    }
    // special case for space since the file is space separated
    eucWideToUtfNarrow.insert(0xa1a1, " ");
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

    QString str = readText((void*)this,hflag);
    if (hflag) {
        int x = 0;
        for (int i = 0; i < refList.count(); i++) {
            const QString f = "<R" + numToStr(i) + "R>";
            x = str.indexOf(f, x);
            str.replace(x, f.length(), refList[i]);
        }
    }

    return str.trimmed();
}

QString EbCore::heading(const EB_Position &pos)
{

    if (seekText(pos) != EB_SUCCESS) {
        return QString();
    }

    QString str = readHeading((void*)this);
    for (int i = 0; i < refList.count(); i++) {
        QString f = "<R" + numToStr(i) + "R>";
        str.replace(f, refList[i]);
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
QByteArray EbCore::hookBeginSubscript(int, const unsigned int*)
{
    return QByteArrayLiteral("<sub>");
}
QByteArray EbCore::hookEndSubscript(int, const unsigned int*)
{
    return QByteArrayLiteral("</sub>");
}
QByteArray EbCore::hookBeginSuperscript(int, const unsigned int*)
{
    return "<sup>";
}
QByteArray EbCore::hookEndSuperscript(int, const unsigned int*)
{
    return "</sup>";
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
QByteArray EbCore::hookNarrowFont(int, const unsigned int *argv)
{
    int fcode = argv[0];
    QByteArray fstr = fontToBStr(fcode, NarrowCode);
    if (!fstr.isEmpty())
        return fstr;

    QByteArray fname = 'n' + numToBStr(fcode,16);
    if (!isHaveNarrowFont())
        return errorBStr("外字(" + fname + ")");

    fname += ".png";

    QByteArray bitmap = narrowFontCharacterBitmap(fcode);
    if (bitmap.isEmpty())
        return errorBStr("Font Extract Error");

    QByteArray cnv = narrowBitmapToPng(bitmap);
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

    QByteArray fname = 'w' + numToBStr(fcode,16);
    if (!isHaveWideFont())
        return errorBStr("外字(" + fname + ")");

    fname += ".png";

    QByteArray bitmap = wideFontCharacterBitmap(fcode);
    if (bitmap.isEmpty())
        return errorBStr("Font Extract Error");

    QByteArray cnv = wideBitmapToPng(bitmap);
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
    QString str;

    if (eucWideToUtfNarrow.contains(argv[0]))
        str = eucWideToUtfNarrow.value(argv[0]);

    if (!str.isNull())
    {
        str = str.toHtmlEscaped();
        return str.toUtf8();
    }

    return hookWideJISX0208(argc, argv);
}
QByteArray EbCore::hookWideJISX0208(int, const unsigned int *argv)
{
    const char code[] = { char(argv[0] >> 8), char(argv[0] & 0xff) };
    return eucCodec->toUnicode(code, std::size(code)).toUtf8();
}
QByteArray EbCore::hookGB2312(int, const unsigned int*)
{
    return errorBStr("HOOK_GB2312 Not Supported");
}
QByteArray EbCore::hookBeginMonoGraphic(int, const unsigned int *argv)
{
    monoHeight = argv[2];
    monoWidth = argv[3];
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
QByteArray EbCore::hookBeginInColorBmp(int, const unsigned int *argv)
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
QByteArray EbCore::hookBeginInColorJpeg(int, const unsigned int *argv)
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
QByteArray EbCore::hookBeginColorBmp(int argc, const unsigned int *argv)
{
    return "<br>" + hookBeginInColorBmp(argc, argv);
}
QByteArray EbCore::hookBeginColorJpeg(int argc, const unsigned int *argv)
{
    return "<br>" + hookBeginInColorJpeg(argc, argv);
}
QByteArray EbCore::hookEndColorGraphic(int, const unsigned int*)
{
    return "</span><br>";
}
QByteArray EbCore::hookEndInColorGraphic(int, const unsigned int*)
{
    return "</span>";
}
QByteArray EbCore::hookBeginWave(int, const unsigned int *argv)
{
    // argv[2] : start page
    // qrgv[3] : start offset
    // qrgv[4] : end page
    // argv[5] : end offset
    QByteArray fname = binaryFname("wav", argv[2], argv[3]);
    QByteArray out = "<a class=snd href=\"sound?" +
                     ebCache.waveCachePath.toUtf8() + "/" + fname + "\">";
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
    return "<a class=mpg href=\"mpeg?<R" + numToBStr(refList.count()) +
           "R>\">";
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
    refList << dst_file.toUtf8();
    return QByteArray();
}
QByteArray EbCore::hookBeginGraphicReference(int, const unsigned int*)
{
    return errorBStr("HOOK_BEGIN_GRAPHIC_REFERENCE Not Supported");
}
QByteArray EbCore::hookEndGraphicReference(int, const unsigned int*)
{
    return errorBStr("HOOK_END_GRAPHIC_REFERENCE Not Supported");
}
QByteArray EbCore::hookGraphicReference(int, const unsigned int*)
{
    return errorBStr("HOOK_GRAPHIC_REFERENCE Not Supported");
}
QByteArray EbCore::hookBeginDecoration(int, const unsigned int* argv)
{
    int code = argv[1];
    decorationStack.push(code);
    switch (code) {
    case 1:
    case 0x1101:
        return "<i>";
    case 3:
    case 0x1103:
        return "<b>";
    default:
        qWarning() << subbookTitle() << "Unrecognized Decoration Code" << code;
        return "<i class=\"decoration0x" + numToBStr(code, 16) + "\">";
    }
}
QByteArray EbCore::hookEndDecoration(int, const unsigned int*)
{
    if (decorationStack.isEmpty()) {
        return QByteArray();
    }
    int code = decorationStack.pop();
    switch (code) {
    case 1:
    case 0x1101:
        return "</i>";
    case 3:
    case 0x1103:
        return "</b>";
    default:
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

