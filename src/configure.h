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
#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <QString>

#define CONF (static_cast<Configure *>(Configure::configure()))

class Configure
{
public:
    Configure();
    static Configure *configure();
    void load();
    void save();
    void setDefault();

    bool highlightMatch;
    bool beepSound;
    bool serverMode;
    int stepBookHitMax;
    int stepTotalHitMax;
    int maxLimitBookHit;
    int maxLimitTotalHit;
    int historyMax;
    int limitImageNum;
    int limitBrowserChar;
    int limitMenuHit;
    int indentOffset;
    int portNo;
    QString dictSheet;
    QString bookSheet;
    QString statusBarSheet;
    QString waveProcess;
    QString mpegProcess;
    QString browserProcess;
    QString googleUrl;
    QString wikipediaUrl;
    QString userDefUrl;
    QString settingOrg;
    QFont browserFont;
};

#endif
