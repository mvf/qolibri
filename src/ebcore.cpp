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

#include "ebcore.h"
#include "textcodec.h"
const int TextBufferSize = 4000;
const int TextSizeLimit = 2800000;

#define toUTF(q_bytearray) \
    QTextCodec::codecForLocale()->toUnicode(q_bytearray)

EbCore::EbCore(HookMode hmode)
    : QObject()
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
}

EbCore::~EbCore()
{
    eb_finalize_hookset(&hookSet);
    eb_finalize_appendix(&appendix);
    eb_finalize_book(&book);
}

void EbCore::initialize()
{
    eb_initialize_library();
    //EbCache::cachePath = QDir::homePath() + "/.ebcache";
}

void EbCore::finalize()
{
    eb_finalize_library();
}

int EbCore::initBook(const QString& path, int subbook, int refpos)
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
    if (subbook >= 0) {
        return initSubBook(subbook, refpos);
    } else {
        return sub_book_count;
    }
}

int EbCore::initSubBook(int index, int refpos)
{
    EB_Error_Code ecode;

    ebHook.refPosition = refpos;

    ecode = eb_set_subbook(&book, subBookList[index]);
    if (ecode != EB_SUCCESS) {
        ebError("eb_set_subbook", ecode);
        return -1;
    }
    ecode = eb_set_appendix_subbook(&appendix, subAppendixList[index]);
    if (ecode != EB_SUCCESS) {
        // qDebug() << "eb_set_appendix_subbook() failed";
        // return -1;
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
    if (eb_have_font(&book, EB_FONT_16)) {
        ecode = eb_set_font(&book, EB_FONT_16);
        if (ecode != EB_SUCCESS) {
            ebError("eb_set_font", ecode);
        }
    }
    return 1;
}

QString EbCore::path()
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

QString EbCore::title()
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

QString EbCore::copyright()
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
    return text(position);
}


QString EbCore::menu()
{
    if (!eb_have_menu(&book))
        return QString();

    EB_Position position;
    EB_Error_Code err;
    err = eb_menu(&book, &position);
    if (err != EB_SUCCESS)
        return QString();

    return text(position);
}

bool EbCore::menu(EB_Position *pos)
{
    EB_Error_Code err;

    err = eb_menu(&book, pos);
    if (err != EB_SUCCESS)
        return false;

    return true;
}

QStringList EbCore::candidate(const EB_Position &pos, QString *txt)          
{                                                                         
   ebHook.candList.clear(); 
   *txt = text(pos);
   return ebHook.candList;
} 

QString EbCore::text(const EB_Position &pos, bool hflag)
{
    //qDebug() << "text : " << pos->page << " " << pos->offset;
    EB_Error_Code ecode;

    EB_Hookset *hook = (hflag) ? &hookSet : NULL;

    ebHook.refList.clear();
    ecode = eb_seek_text(&book, &pos);
    if (ecode != EB_SUCCESS) {
        ebError("eb_seek_text", ecode);
        return QString();
    }

    QByteArray ret;
    char buffer[TextBufferSize + 1];
    ssize_t buffer_length;
    for (int i = 0;; i++) {
        ecode = eb_read_text(&book, &appendix, hook, (void *)&ebHook,
                             TextBufferSize, buffer, &buffer_length);
        if (ecode != EB_SUCCESS) {
            ebError("eb_read_text", ecode);
            break;
        }
        ret += buffer;
        if (eb_is_text_stopped(&book)) break;
        if (((TextBufferSize * i) + buffer_length) > TextSizeLimit) {
            qDebug() << "Data too large" << book.path
                                         << pos.page << pos.offset;
            return QString();
        }
    }

    if (hflag) {
        for (int i = 0; i < ebHook.refList.count(); i++) {
            QByteArray f = "<R" + QByteArray::number(i) + "R>";
            ret.replace(f, ebHook.refList[i]);
        }
        //for (int i = 0; i < candList.count(); i++) {
        //    int sp = ret.indexOf("<C");
        //    int ep = ret.indexOf("C>");
        //    if (ep > sp) {
        //        ret.replace(sp, ep-sp+2, "<>");
        //    }
        //}
        for (int i = 0; i < ebHook.mpegList.count(); i++) {
            QByteArray f = "<M" + QByteArray::number(i) + "M>";
            ret.replace(f, ebHook.mpegList[i]);
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

    return eucToUtf(ret).trimmed();
}

QString EbCore::heading(const EB_Position &pos, bool hflag)
{
    EB_Error_Code ecode;

    EB_Hookset *hook = (hflag) ? &hookSet : NULL;

    ecode = eb_seek_text(&book, &pos);
    if (ecode != EB_SUCCESS) {
        ebError("eb_seek_text", ecode);
        return QString();
    }

    char head_text[1024];
    ssize_t heading_length;
    ecode = eb_read_heading(&book, &appendix, hook, (void *)&ebHook,
                            1023, head_text, &heading_length);
    if (ecode != EB_SUCCESS) {
        ebError("eb_read_heading", ecode);
        return QString();
    }

    if ( heading_length == 0) {
        return QString();
    }
    QString ret = eucToUtf(head_text);
    if (hflag) {
        for (int i = 0; i < ebHook.refList.count(); i++) {
            QByteArray f = "<R" + QByteArray::number(i) + "R>";
            ret.replace(f, ebHook.refList[i]);
        }
    }
    return ret.trimmed();
}

void EbCore::ebError(const QString &func, EB_Error_Code code)
{
    qWarning() << func, toUTF(eb_error_message(code));
}

