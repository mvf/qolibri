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
#ifndef EBCORE_H
#define EBCORE_H

#include <QStack>
#include <QObject>

#include <eb/eb.h>
#include <eb/text.h>

#include "qeb.h"
#include "ebcache.h"
#include "ebhook.h"

enum HookMode { HookText, HookMenu, HookFont };

class EbCore : public QEb
{
public:
    EbCore(HookMode hmode=HookText);
    ~EbCore();

    QString getCopyright();
    QString getMenu();
    int initBook(const QString &path, int subbook=-1, int refpos=0);
    int initSubBook(int index, int refpos=0);
    void initHook(int fsize, QHash<QString, QString> *flist,
                  int indent_offset = 50, bool ruby = true)
    {
        ebHook.init(this, subbookTitle(), fsize, flist, indent_offset, ruby);
    }

    QString text(const EB_Position &pos, bool hflag=true);
    QString heading(const EB_Position &pos, bool hflag=true);
    QList <CandItems> candidate(const EB_Position &pos, QString *txt);

    EbHook ebHook;

protected:

    QList <EB_Subbook_Code> subBookList;
    QList <EB_Subbook_Code> subAppendixList;

};

#endif
