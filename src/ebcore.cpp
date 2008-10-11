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
const int TextBufferSize = 4000;
const int TextSizeLimit = 2800000;

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

    ebHook.refPosition = refpos;

    if (index >= subBookList.count()){
        return -1;
    }
    ecode = setSubbook(subBookList[index]);
    if (ecode != EB_SUCCESS) {
        return -1;
    }
    setAppendixSubbook(subAppendixList[index]);

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
    if (seekText(position) != EB_SUCCESS) {
        return QString();
    }
    return readText((void*)&ebHook);
}


QString EbCore::getMenu()
{
    if (!isHaveMenu())
        return QString();

    EB_Position position = menu();
    if (!isValidPosition(position)) {
        return QString();
    }
    if (seekText(position) != EB_SUCCESS) {
        return QString();
    }

    return readText((void*)&ebHook);
}

QList <CandItems> EbCore::candidate(const EB_Position &pos, QString *txt)          
{                                                                         
   ebHook.candList.clear(); 
   *txt = text(pos);
   return ebHook.candList;
} 

QString EbCore::text(const EB_Position &pos, bool hflag)
{

    //EB_Hookset *hook = (hflag) ? &hookSet : NULL;

    ebHook.refList.clear();

    if (seekText(pos) != EB_SUCCESS) {
        return QString();
    }

    QString str = readText((void*)&ebHook,hflag);
    if (hflag) {
    qDebug() << 4;
        for (int i = 0; i < ebHook.refList.count(); i++) {
            QString f = "<R" + QString::number(i) + "R>";
            str.replace(f, ebHook.refList[i]);
        }
        for (int i = 0; i < ebHook.mpegList.count(); i++) {
            QString f = "<M" + QString::number(i) + "M>";
            str.replace(f, ebHook.mpegList[i]);
        }
    }

    return str.trimmed();
}

QString EbCore::heading(const EB_Position &pos, bool hflag)
{

    //EB_Hookset *hook = (hflag) ? &hookSet : NULL;

    if (seekText(pos) != EB_SUCCESS) {
        return QString();
    }

    QString str = readHeading((void*)&ebHook, hflag);
    if (hflag) {
        for (int i = 0; i < ebHook.refList.count(); i++) {
            QString f = "<R" + QString::number(i) + "R>";
            str.replace(f, ebHook.refList[i]);
        }
    }
    return str.trimmed();
}

