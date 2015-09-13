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

#include "book.h"

const char *FontFile = { "gaiji-table_" };

void Book::loadAlterFont()
{
    if (fontList_) {
        delete fontList_;
        fontList_ = NULL;
    }
    QString fname = path_ + "/" + FontFile + QString::number(bookNo_);
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //qWarning() << "Cannot open for read" << fname;
        return;
    }

    fontList_ = new QHash<QString, QString>;
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString line = in.readLine();
    while (!line.isNull()) {
        QStringList list = line.remove('\n').split(' ');
        if (list.count() == 2) {
            fontList_->insert(list[0], list[1]);
        }
        line = in.readLine();
    }
}

void Book::saveAlterFont()
{
    QString fname = path_ + "/" + FontFile + QString::number(bookNo_);
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open for write" << fname;
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    QHashIterator<QString, QString> i(*fontList_);
    while (i.hasNext()) {
        i.next();
        out << i.key() << " " << i.value() << "\n";
    }
}

