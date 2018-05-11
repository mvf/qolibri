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

#include "ebcache.h"

#include <QDir>
#include <QStandardPaths>

QString EbCache::cachePath = "uninitialized";

void EbCache::initialize()
{
    cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

void EbCache::init(const QString &title)
{
    //qDebug () << "Cache path =" << cachePath;
    QString bookCachePath = cachePath + "/" + title;
    QDir rootDir(bookCachePath);
    if (!rootDir.exists()) {
        QDir().mkpath(bookCachePath);
        rootDir.mkdir("font");
        rootDir.mkdir("image");
        rootDir.mkdir("wave");
        rootDir.mkdir("mpeg");
    }

    fontCachePath = bookCachePath + "/font";
    fontCacheRel   = title + "/font/";
    imageCachePath = bookCachePath + "/image";
    imageCacheRel  = title + "/image/";
    waveCachePath  = bookCachePath + "/wave";
    mpegCachePath  = bookCachePath + "/mpeg";
    fontCacheList  = QDir(fontCachePath).entryList(QDir::Files);
    imageCacheList = QDir(imageCachePath).entryList(QDir::Files);
    waveCacheList  = QDir(waveCachePath).entryList(QDir::Files);
    mpegCacheList  = QDir(mpegCachePath).entryList(QDir::Files);
}

