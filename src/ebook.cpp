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

#include "ebook.h"
#include "ebook_hooks.h"

#define toUTF(q_bytearray) \
    QTextCodec::codecForLocale()->toUnicode(q_bytearray)

const int ImageBufferSize = 50000;
const int HitsBufferSize = 10000;
const int TextBufferSize = 4000;
const int TextSizeLimit = 2800000;

QString EBook::cachePath = QDir::homePath() + "/.ebcache";
QTextCodec * EBook::codecEuc;

EBook::EBook(HookMode hmode)
    : QObject(), fontList_(NULL), imageCount_(0), fontSize_(16),
      indentOffset_(0), indented_(false)
{
    eb_initialize_book(&book);
    eb_initialize_appendix(&appendix);
    eb_initialize_hookset(&hookSet);
    switch(hmode) {
        case HookText :
            eb_set_hooks(&hookSet, hooks);
            break;
        case HookMenu :
            eb_set_hooks(&hookSet, hooks_cand);
            break;
        case HookFont :
            eb_set_hooks(&hookSet, hooks_font);
            break;
        default:
            qWarning() << "Unrecognize Hook Mode" << hmode;
    }

    hits = new EB_Hit[HitsBufferSize];
}

EBook::~EBook()
{
    eb_finalize_hookset(&hookSet);
    eb_finalize_appendix(&appendix);
    eb_finalize_book(&book);

    delete hits;
}

int EBook::setBook(const QString& path)
{
    EB_Error_Code ecode;

    ecode = eb_bind(&book, path.toLocal8Bit());
    if (ecode != EB_SUCCESS) {
        ebError("eb_bind", ecode);
        return -1;
    }
    ecode = eb_bind_appendix(&appendix, path.toLocal8Bit());
    if (ecode != EB_SUCCESS) {
        ebError("eb_bind_appendix", ecode);
//        return -1;
    }
    int sub_book_count;
    ecode = eb_subbook_list(&book, subBookList, &sub_book_count);
    if (ecode != EB_SUCCESS) {
        ebError("eb_subbook_list", ecode);
        return -1;
    }

    ecode = eb_appendix_subbook_list(&appendix, subAppendixList,
                                     &subAppendixCount);
    if (ecode != EB_SUCCESS) {
        ebError("eb_appendix_subbook_list", ecode);
        //return -1;
    }

    return sub_book_count;
}
int EBook::setBook(const QString& path, int sub_book_no, int ref_pos)
{
    int cnt = setBook(path);
    if (cnt < 0)
        return -1;

    if (sub_book_no >= cnt)
        return -1;

    return setSubBook(sub_book_no, ref_pos);
}

int EBook::setSubBook(int index, int ref_pos)
{
    EB_Error_Code ecode;

    ecode = eb_set_subbook(&book, subBookList[index]);
    if (ecode != EB_SUCCESS) {
        ebError("eb_set_subbook", ecode);
        return -1;
    }
    ecode = eb_set_appendix_subbook(&appendix, subAppendixList[index]);
    if (ecode != EB_SUCCESS) {
//        qDebug() << "eb_set_appendix_subbook() failed";
//        return -1;
    }

    EB_Character_Code ccode;
    ecode = eb_character_code(&book, &ccode);
    if (ecode != EB_SUCCESS) {
        ebError("eb_chracter_code", ecode);
    } else {
        if (ccode == EB_CHARCODE_ISO8859_1) {
            ebError("Using ISO 8859-1", EB_SUCCESS);
        //else if (ccode == EB_CHARCODE_JISX0208) {
        //    ebError("Using JIS X 0208", EB_SUCESS);
        } else if (ccode == EB_CHARCODE_JISX0208_GB2312) {
            ebError("Using X 0208 + GB 2312", EB_SUCCESS);
        } else if (ccode == EB_CHARCODE_INVALID) {
            ebError("Using Invalid Character Code", EB_SUCCESS);
        }
    }
    //EB_Font_Code font_list[EB_MAX_FONTS];
    //int font_count;
    //if (eb_font_list(&book, font_list, &font_count) != EB_SUCCESS) {
    //    qDebug() << "eb_font_list() failed\n";
    //} else {
    //    qDebug() << "font List count = " << font_count;
    //}
    if (eb_have_font(&book, EB_FONT_16)) {
        ecode = eb_set_font(&book, EB_FONT_16);
        if (ecode != EB_SUCCESS) {
            ebError("eb_set_font", ecode);
        }
    }

    setCache(title());
    refPosition_ = ref_pos;
    firstSeek = true;

    return 0;
}

QString EBook::path()
{
    EB_Error_Code ecode;
    char str[EB_MAX_PATH_LENGTH + 1];

    ecode = eb_path(&book, str);
    if (ecode != EB_SUCCESS) {
        ebError("eb_path", ecode);
        return QString();
    }
    return QString::fromLocal8Bit(str);
}

QString EBook::copyright()
{
    if (!eb_have_copyright(&book))
        return QString();

    EB_Position position;
    EB_Error_Code ecode;
    ecode = eb_copyright(&book, &position);
    if (ecode != EB_SUCCESS) {
        ebError("eb_copyright", ecode);
        return QString();
    }
    return text(position.page, position.offset);
}

QString EBook::menu()
{
    if (!eb_have_menu(&book))
        return QString();

    EB_Position position;
    EB_Error_Code err;
    err = eb_menu(&book, &position);
    if (err != EB_SUCCESS)
        return QString();

    return text(position.page, position.offset);
}

bool EBook::menu(int *page, int *offset)
{
    EB_Position position;
    EB_Error_Code err;

    err = eb_menu(&book, &position);
    if (err != EB_SUCCESS)
        return false;

    *page = position.page;
    *offset = position.offset;
    return true;
}

QString EBook::title()
{
    EB_Error_Code ecode;
    char t[EB_MAX_TITLE_LENGTH + 1];

    ecode = eb_subbook_title(&book, t);
    if (ecode != EB_SUCCESS) {
        ebError("eb_subbook_title", ecode);
        return QString();
    }
    t[EB_MAX_TITLE_LENGTH] = 0;
    return eucToUtf(t);
}

int EBook::hitMultiWord(int maxcnt, const QStringList &words, SearchType stype)
{
    int word_num = words.count();

    char** word_list = new char*[word_num + 1];
    QList <QByteArray> bword_list;
    for (int i = 0; i < word_num; i++ ) {
	bword_list << utfToEuc(words[i]);
	word_list[i] = bword_list[i].data();
    }
    word_list[word_num] = NULL;

    if ( maxcnt <= 0 )
        maxcnt = HitsBufferSize;
    int hit_count;
    int count = 0;
    for (;;) {
        EB_Error_Code ecode;
        if (stype == SearchKeyWord) {
            ecode = eb_search_keyword(&book, word_list);
        } else {
            ecode = eb_search_cross(&book, word_list);
        }
        if (ecode != EB_SUCCESS) {
            ebError("eb_search_cross", ecode);
            break;
        }

        EB_Hit wrk[HitsBufferSize];
        ecode = eb_hit_list(&book, HitsBufferSize, wrk, &hit_count);
        if (ecode != EB_SUCCESS) {
            ebError("eb_hit_list", ecode);
            break;
        }

        for (int i = 0; i < hit_count; i++) {
            bool same_text = false;
            for (int j = 0; j < count; j++) {
                if (wrk[i].text.page == hits[j].text.page &&
                    wrk[i].text.offset == hits[j].text.offset) {
                    same_text = true;
                    break;
                }
            }
            if (same_text) continue;

            hits[count] = wrk[i];
            count++;
            if (count >= maxcnt) break;
        }

        break;
    }
    delete word_list;

    return count;
}

int EBook::hitWord(int maxcnt, const QString &word, SearchType type)
{
    if ( maxcnt <= 0 ) maxcnt = HitsBufferSize;
    EB_Error_Code ecode;
    QByteArray bword = utfToEuc(word);
    if (type == SearchWord) {
	if (!isHaveWordSearch())
	    return 0;
        ecode = eb_search_word(&book, bword);
        if (ecode != EB_SUCCESS) {
            ebError("eb_search_word", ecode);
            return -1;
        }
    } else if (type == SearchEndWord) {
	if (!isHaveEndwordSearch())
	    return 0;
        ecode = eb_search_endword(&book, bword);
        if (ecode != EB_SUCCESS) {
            ebError("eb_search_endword", ecode);
            return -1;
        }
    } else {
	if (!isHaveWordSearch())
	    return 0;
        ecode = eb_search_exactword(&book, bword);
        if (ecode != EB_SUCCESS) {
            ebError("eb_search_exactword", ecode);
            return -1;
        }
    }

    EB_Hit wrk[HitsBufferSize];
    int hit_count;
    ecode = eb_hit_list(&book, HitsBufferSize, wrk, &hit_count);
    if (ecode != EB_SUCCESS) {
        ebError("eb_hit_list", ecode);
        return -1;
    }

    int count = 0;
    for (int i = 0; i < hit_count; i++) {
        bool same_text = false;
        for (int j = 0; j < count; j++) {
            if (wrk[i].text.page == hits[j].text.page &&
                wrk[i].text.offset == hits[j].text.offset) {
                same_text = true;
                break;
            }
        }
        if (same_text)  continue;

        hits[count] = wrk[i];
        count++;
        if (count >= maxcnt) break;
    }
    return count;
}

int EBook::hitFull(int maxcnt)
{
    EB_Error_Code ecode;
    EB_Position position;
    int count = 0;

    if (firstSeek) {
        ecode = eb_text(&book, &position);
        if (ecode != EB_SUCCESS) {
            ebError("eb_text", ecode);
            return -1;
        }
        firstSeek = false;
    } else {
        position = seekPosition;
    }
    hits[count].heading = position;
    hits[count].text = position;
    count++;
    while (count <= maxcnt) {
        ecode = eb_seek_text(&book, &position);
        if (ecode != EB_SUCCESS) {
            ebError("eb_seek_text", ecode);
            break;
        }
        ecode = eb_forward_text(&book, &appendix);
        if (ecode != EB_SUCCESS) {
            if (ecode == EB_ERR_END_OF_CONTENT &&
                position.page < book.subbook_current->text.end_page) {
                //qDebug() << "hitFull : page=" << position.page
                //         << "offset=" << position.offset
                //         << "end page=" << book.subbook_current->text.end_page;
                ecode = eb_tell_text(&book, &position);
                if (ecode != EB_SUCCESS) {
                    ebError("eb_tell_text", ecode);
                    break;
                }
                if (position.page >= book.subbook_current->text.end_page) {
                    //    qDebug() << "hitFull : page=" << position.page
                    //             << "offset=" << position.offset ;
                    break;
                }
                position.offset += 2;
            } else {
                break;
            }
        } else {
            ecode = eb_tell_text(&book, &position);
            if (ecode != EB_SUCCESS) {
                ebError("eb_tell_text", ecode);
                break;
            }
        }
        if (count < maxcnt) {
            hits[count].heading = position;
            hits[count].text = position;
        }
        count++;
    }
    seekPosition = position;
    return count - 1;
}

int EBook::setStartHit(const EB_Position &text_pos)
{
    seekPosition = text_pos;
    firstSeek = false;
    return 0;
}

QStringList EBook::candidate(int page, int offset, QString *txt)
{
    candList.clear();
    *txt = text(page, offset);
    return candList;
}

QString EBook::text(int page, int offset, bool hflag)
{
    //qDebug() << "text : " << page << " " << offset;
    EB_Error_Code ecode;
    EB_Position pos;

    pos.page = page;
    pos.offset = offset;
    curPosition = pos;
    EB_Hookset *hook = (hflag) ? &hookSet : NULL;

    refList.clear();
    ecode = eb_seek_text(&book, &pos);
    if (ecode != EB_SUCCESS) {
        ebError("eb_seek_text", ecode);
        curPosition.page = 0;
        return QString();
    }

    QByteArray ret;
    char buffer[TextBufferSize + 1];
    ssize_t buffer_length;
    for (int i = 0;; i++) {
        ecode = eb_read_text(&book, &appendix, hook, (void *)this,
                             TextBufferSize, buffer, &buffer_length);
        if (ecode != EB_SUCCESS) {
            ebError("eb_read_text", ecode);
            break;
        }
        ret += buffer;
        if (eb_is_text_stopped(&book)) break;
        if (((TextBufferSize * i) + buffer_length) > TextSizeLimit) {
            qDebug() << "Data too large" << book.path << page << offset;
            curPosition.page = 0;
            return QString();
        }
    }

    if (hflag) {
        for (int i = 0; i < refList.count(); i++) {
            QByteArray f = "<R" + QByteArray::number(i) + "R>";
            ret.replace(f, refList[i]);
        }
        //for (int i = 0; i < candList.count(); i++) {
        //    int sp = ret.indexOf("<C");
        //    int ep = ret.indexOf("C>");
        //    if (ep > sp) {
        //        ret.replace(sp, ep-sp+2, "<>");
        //    }
        //}
        for (int i = 0; i < mpegList.count(); i++) {
            QByteArray f = "<M" + QByteArray::number(i) + "M>";
            ret.replace(f, mpegList[i]);
        }
        //if (!ruby_) {
        //    int sp;
        //    while((sp = ret.indexOf("<sub>")) > 0) {
        //        int ep = ret.indexOf("</sub>");
        //        if (ep < 0 || ep <= sp) {
        //            qWarning() << "Data Error : not match <sub></sub>"
        //                       << sp << ep;
        //            if (ep < 0)
        //                break;
        //            sp = ep;
        //        }
        //        ret.remove(sp, ep-sp+6);
        //    }
        //}
    }
    //QString sret = eucToUtf(ret);
    //if (sret[sret.length()-1].isSpace()) {
    //    sret.truncate(sret.length()-1);
    //}
    //return sret;

    curPosition.page = 0;
    return eucToUtf(ret).trimmed();
}

QString EBook::heading(int page, int offset, bool hflag)
{
    EB_Error_Code ecode;

    EB_Position pos;

    pos.page = page;
    pos.offset = offset;
    curPosition = pos;

    EB_Hookset *hook = (hflag) ? &hookSet : NULL;

    ecode = eb_seek_text(&book, &pos);
    if (ecode != EB_SUCCESS) {
        ebError("eb_seek_text", ecode);
        curPosition.page = 0;
        return QString();
    }

    char head_text[1024];
    ssize_t heading_length;
    ecode = eb_read_heading(&book, &appendix, hook, (void *)this,
                            1023, head_text, &heading_length);
    if (ecode != EB_SUCCESS) {
        ebError("eb_read_heading", ecode);
        curPosition.page = 0;
        return QString();
    }

    if ( heading_length == 0) {
        curPosition.page = 0;
        return QString();
    }
    QString ret = eucToUtf(head_text);
    if (hflag) {
        for (int i = 0; i < refList.count(); i++) {
            QByteArray f = "<R" + QByteArray::number(i) + "R>";
            ret.replace(f, refList[i]);
        }
    }
    curPosition.page = 0;
    return ret.trimmed();
}

void EBook::setCache(const QString &name)
{
    QString bookCachePath = cachePath + "/" + name;
    QDir rootDir(bookCachePath);

    if (!rootDir.exists()) {
        QDir().mkpath(bookCachePath);
        rootDir.mkdir("font");
        rootDir.mkdir("image");
        rootDir.mkdir("wave");
        rootDir.mkdir("mpeg");
    }
    fontCachePath_ = bookCachePath + "/font";
    fontCacheRel = utfToEuc(name) + "/font/";
    imageCachePath = bookCachePath + "/image";
    imageCacheRel = utfToEuc(name) + "/image/";
    waveCachePath = bookCachePath + "/wave";
    mpegCachePath = bookCachePath + "/mpeg";
    fontCacheList = QDir(fontCachePath_).entryList(QDir::Files);
    imageCacheList = QDir(imageCachePath).entryList(QDir::Files);
    waveCacheList = QDir(waveCachePath).entryList(QDir::Files);
}

void EBook::ebError(const QString &func, EB_Error_Code code)
{
    if (curPosition.page) {
        qWarning() << func << "failed :" << toUTF(eb_error_message(code))
                   << "(" << curPosition.page
                   << "," << curPosition.offset << ")";
    } else {
        qWarning() << func << "failed :" << toUTF(eb_error_message(code));
    }
}

QByteArray EBook::begin_decoration(int deco_code)
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

QByteArray EBook::end_decoration()
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

QByteArray EBook::set_indent(int val)
{
    QByteArray ret = QByteArray();
    if (val > 2){
        int mleft = indentOffset_ + (val * fontSize_);
        ret += "</pre><pre style=\"margin-left: " +
               QByteArray::number(mleft) + "px; \">";
        indented_ = true;
    } else {
        if (indented_) {
            ret = "</pre><pre>";
            indented_ = false;
        }
    }
    return ret;
}

QByteArray EBook::narrow_font(int code)
{
    if (!eb_have_narrow_font(&book)) {
        qDebug() << "not have narrow font";
        return errorString("narrow font error");
    }

    EB_Error_Code ecode;
    QString fcode = "n" + QString::number(code, 16);
    if (fontList_) {
        QString afont = fontList_->value(fcode);
        if (!afont.isEmpty())
            return afont.toAscii();
    }
#ifdef USE_GIF_FOR_FONT
    QByteArray fname = fcode.toAscii() + ".gif";
#else
    QByteArray fname = fcode.toAscii() + ".png";
#endif

    QByteArray out = "<img src=\"" + fontCacheRel + fname  + "\"";
    int h = fontSize();
    if (h > 17) {
        out += " height=" + QByteArray::number(h) +
               " width=" + QByteArray::number(h / 2);
    }
    out += " />";

    if (fontCacheList.contains(fname))
        return out;

    char bitmap[EB_SIZE_NARROW_FONT_16];
    if ((ecode = eb_narrow_font_character_bitmap(&book, code, bitmap))
        != EB_SUCCESS) {
        ebError("eb_narrow_font_character_bitmap", ecode);
        return errorString("narrow font error");
    }

    size_t wlen;
#ifdef USE_GIF_FOR_FONT
    char buff[EB_SIZE_NARROW_FONT_16_GIF];
    if ((ecode = eb_bitmap_to_gif(bitmap, 8, 16, buff, &wlen) != EB_SUCCESS)) {
        ebError("eb_bitmap_to_gif", ecode);
        return errorString("narrow font error");
    }
#else
    char buff[EB_SIZE_NARROW_FONT_16_PNG];
    if ((ecode = eb_bitmap_to_png(bitmap, 8, 16, buff, &wlen) != EB_SUCCESS)) {
        ebError("eb_bitmap_to_png", ecode);
        return errorString("narrow font error");
    }
#endif


    QFile f(fontCachePath_ +  '/' + fname);
    f.open(QIODevice::WriteOnly);
    f.write(buff, wlen);
    f.close();
    //qDebug() << "Output narrow_font" << fname;
    fontCacheList << fname;

    return out;
}

QByteArray EBook::begin_candidate()
{
    return "<a class=cnd href=\"<R" +
           QByteArray::number(refList.count()) +
           "R>\">";
}

QByteArray EBook::begin_candidate_menu()
{
    return "<C";
}


QByteArray EBook::begin_reference()
{
    return "<a class=ref href=\"<R" +
           QByteArray::number(refList.count()) +
           "R>\">";
}
QByteArray EBook::begin_color_jpeg(int page, int offset)
{
    imageCount_++;

    QByteArray jpgFile = makeFname("jpeg", page, offset);
    QByteArray out = "<img src=\"" + imageCacheRel +
                     jpgFile + "\"><span class=img>";

    if (imageCacheList.contains(jpgFile)) {
        return out;
    }

    //qDebug() << "Out Image " << jpgFile;

    EB_Position pos;
    EB_Error_Code err;
    pos.page = page;
    pos.offset = offset;
    if ((err = eb_set_binary_color_graphic(&book, &pos)) != EB_SUCCESS) {
        ebError("eb_set_binary_color_graphic", err);
        return errorString("image(jpeg) error");
    }


    char bitmap[ImageBufferSize];
    Q_CHECK_PTR(bitmap);
    ssize_t bitmap_length;
    QFile f(imageCachePath + '/' + jpgFile);
    f.open(QIODevice::WriteOnly);
    int flg = 0;

    for (;;)
    {
        if ((err = eb_read_binary(&book, ImageBufferSize, bitmap,
                                  &bitmap_length)) != EB_SUCCESS) {
            ebError("eb_read_binary", err);
            return errorString("image(jpeg) error");
        }
        // qDebug() << "eb_read_binary : size=" << bitmap_length;
        f.write(bitmap, bitmap_length);
        flg++;
        if (bitmap_length < ImageBufferSize) break;
    }
    f.close();
    //qDebug() << "Image Size :" << ImageBufferSize * flg;
    imageCacheList << jpgFile;

    return out;
}

QByteArray EBook::begin_color_bmp(int page, int offset)
{
    EB_Error_Code ecode;

    imageCount_++;

    QByteArray bmpFile = makeFname("bmp", page, offset);
    QByteArray out = "<img src=\"" + imageCacheRel +
                     bmpFile + "\" /><span class=img>";

    if (imageCacheList.contains(bmpFile))
        return out;

    //qDebug() << "Output Image " << bmpFile;

    EB_Position pos;
    pos.page = page;
    pos.offset = offset;
    if ((ecode = eb_set_binary_color_graphic(&book, &pos)) != EB_SUCCESS) {
        ebError("eb_set_binary_color_graphic", ecode);
        return errorString("image(bmp) error");
    }


    char bitmap[ImageBufferSize];
    Q_CHECK_PTR(bitmap);
    ssize_t bitmap_length;
    QFile f(imageCachePath + '/' + bmpFile);
    f.open(QIODevice::WriteOnly);
    int flg = 0;

    for (;;)
    {
        if ((ecode = eb_read_binary(&book, ImageBufferSize,
                                    bitmap, &bitmap_length)) != EB_SUCCESS) {
            ebError("eb_read_binary", ecode);
            return errorString("image(bmp) error");
        }
        //qDebug() << "eb_read_binary : size=" << bitmap_length;
        f.write(bitmap, bitmap_length);
        flg++;
        if (bitmap_length < ImageBufferSize) break;
    }
    f.close();
    //qDebug() << "Image Size :" << ImageBufferSize * flg;
    imageCacheList << bmpFile;

    return out;
}

QByteArray EBook::end_reference(int page, int offset)
{
    QByteArray ref = "book|" + QByteArray::number(refPosition_) + '|' +
                     QByteArray::number(page) + '|' +
                     QByteArray::number(offset);

    refList << ref;
    return "</a>";
}

QByteArray EBook::end_candidate_group(int page, int offset)
{
    QByteArray cnd = "menu|" + QByteArray::number(refPosition_) + '|' +
                     QByteArray::number(page) + '|' +
                     QByteArray::number(offset);

    refList << cnd;
    return "</a>";
}

QByteArray EBook::end_candidate_group_menu(int page, int offset)
{
    QString str = eucToUtf(eb_current_candidate(&book)) + "&|" +
                  QString::number(page) + "&|" +  QString::number(offset);

    candList << str;
    return "C>";
}

QByteArray EBook::begin_mpeg()
{
    return "<a class=mpg href=\"<M" + QByteArray::number(mpegList.count()) +
           "M>\">";
}

void EBook::end_mpeg(const unsigned int *p)
{
    EB_Error_Code ecode;
    char sfile[EB_MAX_PATH_LENGTH + 1];

    if ((ecode = eb_compose_movie_path_name(&book, p, sfile)) != EB_SUCCESS) {
        ebError("eb_compose_movie_path_name", ecode);
        return ;
    }
    QString dfile = mpegCachePath + "/" + QFileInfo(sfile).fileName() + ".mpeg";
    if (!QFile(dfile).exists())
        QFile().copy(sfile, dfile);
    mpegList << "mpeg|" + utfToEuc(dfile);
}

QByteArray EBook::wide_font(int code)
{
    EB_Error_Code ecode;

    if (!eb_have_wide_font(&book)) {
        qDebug() << "not have wide font";
        return errorString("wide font error");
    }

    QString fcode = "w" + QString::number(code, 16);

    if (fontList_) {
        QString afont = fontList_->value(fcode);
        if (!afont.isEmpty())
            return afont.toAscii();
    }

#ifdef USE_GIF_FOR_FONT
    QByteArray fname = fcode.toAscii() + ".gif";
#else
    QByteArray fname = fcode.toAscii() + ".png";
#endif

    QByteArray out = "<img src=\"" + fontCacheRel + fname  + "\"";
    int h = fontSize();
    if (h > 17)
        out += " height=" + QByteArray::number(h) +
               " width=" + QByteArray::number(h);
    out += " />";


    if (fontCacheList.contains(fname)) {
        return out;
    }

    char bitmap[EB_SIZE_WIDE_FONT_16];
    if ((ecode = eb_wide_font_character_bitmap(&book, code, bitmap))
        != EB_SUCCESS) {
        ebError("eb_wide_font_character_bitmap", ecode);
        return errorString("wide font error");
    }

    size_t wlen;
#ifdef USE_GIF_FOR_FONT
    char buff[EB_SIZE_WIDE_FONT_16_GIF];
    if ((ecode = eb_bitmap_to_gif(bitmap, 16, 16, buff, &wlen)) != EB_SUCCESS) {
        ebError("eb_bitmap_to_gif", ecode);
        return errorString("wide font error");
    }
#else
    char buff[EB_SIZE_WIDE_FONT_16_PNG];
    if ((ecode = eb_bitmap_to_png(bitmap, 16, 16, buff, &wlen)) != EB_SUCCESS) {
        ebError("eb_bitmap_to_png", ecode);
        return errorString("wide font error");
    }
#endif

    QFile f(fontCachePath_ + '/' + fname);
    f.open(QIODevice::WriteOnly);
    f.write(buff, wlen);
    f.close();
    fontCacheList << fname;
    //qDebug() << "Output wide_font" << xpmFile;
    return out;
}

QByteArray EBook::end_mono_graphic(int page, int offset)
{
    EB_Error_Code ecode;

    QByteArray bmpFile = makeFname("bmp", page, offset);

    QByteArray out = "<img src=\"" + imageCacheRel + bmpFile + "\" />\n";

    if (imageCacheList.contains(bmpFile)) {
        return out;
    }

    EB_Position pos;
    pos.page = page;
    pos.offset = offset;
    //qDebug() << eb->monoWidth << " " << eb->monoHeight;
    if ((ecode = eb_set_binary_mono_graphic(&book, &pos, monoWidth_,
                                            monoHeight_)) != EB_SUCCESS) {
        ebError("eb_set_binary_mono_graphic", ecode);
        return errorString("image(mono) error");
    }


    char bitmap[ImageBufferSize];
    Q_CHECK_PTR(bitmap);
    ssize_t bitmap_length;
    QFile f(imageCachePath + '/' + bmpFile);
    f.open(QIODevice::WriteOnly);

    int flg = 0;
    for (;;) {
        if ((ecode = eb_read_binary(&book, ImageBufferSize, bitmap,
                                    &bitmap_length)) != EB_SUCCESS) {
            ebError("eb_read_binary", ecode);
            return errorString("image(mono) error");
        }
        f.write(bitmap, bitmap_length);
        flg++;
        if (bitmap_length < ImageBufferSize) break;
    }
    f.close();
    imageCacheList << bmpFile;

    return out;
}

QByteArray EBook::begin_wave(int start_page, int start_offset, int end_page,
                             int end_offset)
{
    EB_Error_Code ecode;

    QString wavFile = QString("%1x%2.wav").arg(start_page).arg(start_offset);
    QString fname = waveCachePath + "/" + wavFile;
    QString out =  QString("<a class=snd href=\"sound|%1\">").arg(fname);

    if (waveCacheList.contains(wavFile))
        return utfToEuc(out);

    EB_Position spos, epos;
    spos.page = start_page;
    spos.offset = start_offset;
    epos.page = end_page;
    epos.offset = end_offset;
    eb_set_binary_wave(&book, &spos, &epos);

    char data[ImageBufferSize];
    Q_CHECK_PTR(data);
    ssize_t data_length;
    QFile f(fname);
    f.open(QIODevice::WriteOnly);
    int flg = 0;
    for (;;)
    {
        if ((ecode = eb_read_binary(&book, ImageBufferSize, data,
                                    &data_length)) != EB_SUCCESS) {
            ebError("eb_read_binary", ecode);
            return errorString("wave error");
        }
        //qDebug() << "eb_read_binary : size=" << data_length;
        f.write(data, data_length);
        flg++;
        if (data_length < ImageBufferSize) break;
    }
    f.close();
    waveCacheList << wavFile;
    //qDebug() << "Output wave" << fname;
    return utfToEuc(out);
}
