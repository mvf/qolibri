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

#include "qeb.h"
#include "ebcache.h"

#include <QStack>

#include <ebu/eb.h>
#include <ebu/text.h>

extern EB_Hook hooks[];
extern EB_Hook hooks_cand[];
extern EB_Hook hooks_font[];

enum NarrowOrWide { NarrowCode, WideCode };
enum HookMode { HookText, HookMenu, HookFont };

struct CandItem
{
    QString title;
    EB_Position position;
};

class EbCore : public QEb
{
public:
    EbCore(HookMode hmode=HookText);
    ~EbCore();

    void initializeEucWideToUtfNarrow();
    QString getCopyright();
    QString getMenu();
    int initBook(const QString &path, int subbook=-1, int refpos=0);
    int initSubBook(int index, int refpos=0);
    void initHook(int fsize, QHash<QString, QString> *flist)
    {
        ebCache.init(subbookTitle());
        fontSize = fsize;
        fontList = flist;
    }

    QString text(const EB_Position &pos, bool hflag=true);
    QString heading(const EB_Position &pos);
    QList <CandItem> candidate(const EB_Position &pos, QString *txt);

    QByteArray errorBStr(const QByteArray &error_message);
    QByteArray fontToBStr(int code, NarrowOrWide n_or_w);
    QByteArray fontToHtmlBStr(const QByteArray &fname, NarrowOrWide w_or_n);
    QByteArray numToBStr(int n, int b=10) { return QByteArray::number(n, b); }
    QString numToStr(int n, int b=10) { return QString::number(n, b); }
    bool makeBinaryFile(const QString &fname, const QByteArray &data);
    QByteArray binaryFname(const QByteArray &ftype, int p1, int p2)
        { return numToBStr(p1) + 'x' + numToBStr(p2) + '.' + ftype; }

// Hook Callbacks
    QByteArray hookBeginSubscript(int, const unsigned int*);
    QByteArray hookEndSubscript(int, const unsigned int*);
    QByteArray hookBeginSuperscript(int, const unsigned int*);
    QByteArray hookEndSuperscript(int, const unsigned int*);
    QByteArray hookBeginEmphasis(int, const unsigned int*);
    QByteArray hookEndEmphasis(int, const unsigned int*);
    QByteArray hookBeginCandidate(int, const unsigned int*);
    QByteArray hookEndCandidateGroup(int, const unsigned int*);
    QByteArray hookEndCandidateGroupMENU(int, const unsigned int*);
    QByteArray hookEndCandidateLeaf(int, const unsigned int*);
    QByteArray hookBeginReference(int, const unsigned int*);
    QByteArray hookEndReference(int, const unsigned int*);
    QByteArray hookNarrowFont(int, const unsigned int*);
    QByteArray hookWideFont(int, const unsigned int*);
    QByteArray hookISO8859_1(int, const unsigned int*);
    QByteArray hookNarrowJISX0208(int, const unsigned int*);
    QByteArray hookWideJISX0208(int, const unsigned int*);
    QByteArray hookGB2312(int, const unsigned int*);
    QByteArray hookBeginMonoGraphic(int, const unsigned int*);
    QByteArray hookEndMonoGraphic(int, const unsigned int*);
    QByteArray hookBeginGrayGraphic(int, const unsigned int*);
    QByteArray hookEndGrayGraphic(int, const unsigned int*);
    QByteArray hookBeginColorBmp(int, const unsigned int*);
    QByteArray hookBeginColorJpeg(int, const unsigned int*);
    QByteArray hookBeginInColorBmp(int, const unsigned int*);
    QByteArray hookBeginInColorJpeg(int, const unsigned int*);
    QByteArray hookEndColorGraphic(int, const unsigned int*);
    QByteArray hookEndInColorGraphic(int, const unsigned int*);
    QByteArray hookBeginWave(int, const unsigned int*);
    QByteArray hookEndWave(int, const unsigned int*);
    QByteArray hookBeginMpeg(int, const unsigned int*);
    QByteArray hookEndMpeg(int, const unsigned int*);
    QByteArray hookBeginGraphicReference(int, const unsigned int*);
    QByteArray hookEndGraphicReference(int, const unsigned int*);
    QByteArray hookGraphicReference(int, const unsigned int*);
    QByteArray hookBeginDecoration(int, const unsigned int*);
    QByteArray hookEndDecoration(int, const unsigned int*);
    QByteArray hookBeginImagePage(int, const unsigned int*);
    QByteArray hookEndImagePage(int, const unsigned int*);
    QByteArray hookBeginClickableArea(int, const unsigned int*);
    QByteArray hookEndClickableArea(int, const unsigned int*);

    EbCache ebCache;
    QStack <int> decorationStack;
    QMap<uint,QString> eucWideToUtfNarrow;
    QHash<QString, QString> *fontList;
    QList <QByteArray> refList;
    QList <CandItem> candList;
    int refPosition;
    int fontSize;
    int monoWidth;
    int monoHeight;

public:

    QList <EB_Subbook_Code> subBookList;
    QList <EB_Subbook_Code> subAppendixList;
};

#endif
