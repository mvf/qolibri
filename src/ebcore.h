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

#include <qdebug>
#include <QObject>

#include <eb/eb.h>
#include <eb/text.h>

#include "ebcache.h"
#include "ebhook.h"

class EbCore : public QObject
{
public:
    EbCore();
    ~EbCore();

    static void initialize();
    static void finalize();

    int initBook(const QString &path);
    int initSubBook(int index);
    void unsetBook() { eb_unset_subbook(&book); }

    QString path();
    QString title();
/*
    QString copyright();
    QString menu();
    bool menu(EB_Position *pos);
    QString text(EB_Position *pos, bool hflag=true);
    QString heading(EB_Position *pos, bool hflag=true);
*/

    bool isHaveText() { return (eb_have_text(&book) == 1); }
    bool isHaveWordSearch() { return (eb_have_word_search(&book) == 1); }
    bool isHaveKeywordSearch() { return (eb_have_keyword_search(&book) == 1); }
    bool isHaveEndwordSearch() { return (eb_have_endword_search(&book) == 1); }
    bool isHaveCrossSearch() { return (eb_have_cross_search(&book) == 1); }
    bool isHaveMenu() { return (eb_have_menu(&book) == 1); }
    bool isHaveCopyright() { return (eb_have_copyright(&book) == 1); }

protected:

    void ebError(const QString &func, EB_Error_Code code);

    EB_Book book;
    EB_Appendix appendix;
    EB_Hookset hookSet;
    EB_Subbook_Code subBookList[EB_MAX_SUBBOOKS];
    EB_Subbook_Code subAppendixList[EB_MAX_SUBBOOKS];
    int subAppendixCount;

    EbHook ebHook;
    EbCache ebCache;

};

#endif
