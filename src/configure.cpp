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

#include "configure.h"

#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QTextStream>

const bool highlightMatch_Def = true;
const bool beepSound_Def = true;
const bool serverMode_Def = false;
const bool convertFullwidth_Def = true;
const int maxLimitBookHit_Def = 15000;
const int maxLimitTotalHit_Def = 15000;
const int historyMax_Def = 500;
const int limitBrowserChar_Def = 1000000;
const int limitMenuHit_Def = 1000;
const int indentOffset_Def = 30;
const int portNo_Def = 5626;

const char* googleUrl_Def =
    "https://www.google.com/search?rls=ja-jp&ie=UTF-8&oe=UTF-8&q=";
const char* wikipediaUrl_Def = "https://ja.wikipedia.org/wiki/";
const char* userDefUrl_Def = "https://en.wikipedia.org/wiki/";
const char* settingOrg_Def = "qolibri";

Configure *configure_s;

Configure::Configure()
{
    settingOrg = settingOrg_Def;
}

Configure *Configure::configure()
{
    return configure_s;
}

void Configure::load()
{
    QSettings conf(CONF->settingOrg, "EpwingConfigure");

    highlightMatch = conf.value("highlight_match", highlightMatch_Def).toBool();
    beepSound = conf.value("beep_sound", beepSound_Def).toBool();
    serverMode = conf.value("server_mode", serverMode_Def).toBool();
    convertFullwidth = conf.value("convert_fullwidth", convertFullwidth_Def).toBool();
    historyMax = conf.value("hist_max", historyMax_Def).toInt();
    waveProcess = conf.value("wave_proc").toString();
    mpegProcess = conf.value("mpeg_proc").toString();
    browserProcess = conf.value("browser_proc").toString();
    googleUrl = conf.value("google_url", googleUrl_Def).toString();
    wikipediaUrl = conf.value("wikipedia_url", wikipediaUrl_Def).toString();
    userDefUrl = conf.value("userdef_url", userDefUrl_Def).toString();
    limitBrowserChar = conf.value("limit_char", limitBrowserChar_Def).toInt();
    limitMenuHit = conf.value("limit_menu", limitMenuHit_Def).toInt();
    indentOffset = conf.value("indent_offset", indentOffset_Def).toInt();
    portNo = conf.value("port_no", portNo_Def).toInt();
    maxLimitBookHit = conf.value("limt_book", maxLimitBookHit_Def).toInt();
    maxLimitTotalHit = conf.value("limit_total", maxLimitTotalHit_Def).toInt();
    browserFont.fromString(conf.value("browser_font", qApp->font()).toString());
    dictionarySearchPath = conf.value("dictionary_search_path", QDir::homePath()).toString();

    QFile file(":/data/book-style.css");
    QTextStream stream(&file);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    defBookStyle = stream.readAll();
    file.close();
    file.setFileName(":/data/dict-style.css");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    defDictStyle = stream.readAll();
    file.close();
    file.setFileName(":/data/statusbar-style.css");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    defStatusbarStyle = stream.readAll();
    file.close();

    QSettings ssheets(CONF->settingOrg, "EpwingStyleSheet");
    dictSheet = ssheets.value("dictionary", defDictStyle).toString();
    bookSheet = ssheets.value("book", defBookStyle).toString();
    statusBarSheet = ssheets.value("widgets1", defStatusbarStyle).toString();
}

void Configure::save()
{
    QSettings conf(CONF->settingOrg, "EpwingConfigure");

    conf.setValue("hightlight_match", highlightMatch);
    conf.setValue("beep_sound", beepSound);
    conf.setValue("server_mode", serverMode);
    conf.setValue("convert_fullwidth", convertFullwidth);
    conf.setValue("hist_max", historyMax);
    conf.setValue("wave_proc", waveProcess);
    conf.setValue("mpeg_proc", mpegProcess);
    conf.setValue("browser_proc", browserProcess);
    conf.setValue("google_url", googleUrl);
    conf.setValue("wikipedia_url", wikipediaUrl);
    conf.setValue("userdef_url", userDefUrl);
    conf.setValue("limit_char", limitBrowserChar);
    conf.setValue("limit_menu", limitMenuHit);
    conf.setValue("limt_book", maxLimitBookHit);
    conf.setValue("limit_total", maxLimitTotalHit);
    conf.setValue("indent_offset", indentOffset);
    conf.setValue("port_no", portNo);
    conf.setValue("browser_font", browserFont.toString());
    conf.setValue("dictionary_search_path", dictionarySearchPath);

    QSettings ssheets(settingOrg, "EpwingStyleSheet");
    ssheets.setValue("dictionary", dictSheet);
    ssheets.setValue("book", bookSheet);
    ssheets.setValue("widgets1", statusBarSheet );
}

void Configure::setDefault()
{
    highlightMatch = highlightMatch_Def;
    beepSound = beepSound_Def;
    serverMode = serverMode_Def;
    historyMax = historyMax_Def;
    waveProcess.clear();
    mpegProcess.clear();
    browserProcess.clear();
    googleUrl = googleUrl_Def;
    wikipediaUrl = wikipediaUrl_Def;
    userDefUrl = userDefUrl_Def;
    limitBrowserChar = limitBrowserChar_Def;
    limitMenuHit = limitMenuHit_Def;
    indentOffset = indentOffset_Def;
    portNo = portNo_Def;
    maxLimitBookHit = maxLimitBookHit_Def;
    maxLimitTotalHit = maxLimitTotalHit_Def;
    browserFont = qApp->font();
}

