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
#ifndef TEXTCODEC_H
#define TEXTCODEC_H

#include <QTextCodec>

extern QTextCodec *codecEuc;

// EUC-JP(b) -> UTF-8(s)
QString eucToUtf(const QByteArray &ba);

// UTF-8(s) -> EUC-JP(b)
QByteArray utfToEuc(const QString &s);

//
// UTF-8(s) -> Local Codec(b)
// QByteArray QString::toLocal8Bit()
//
// Local Codec(b) -> UTF-8(s)
// QString QString::fromLocal8Bit();
//
// UTF-8(s) -> Ascii(b)
// QByteArray QString::toAscii()
//
// Ascii(b) -> UTF-8(s)
// QString QString::fromAscii();
//
// UTF-8(s) -> UTF-8(b)
// QByteArray QString::toUtf8()
//
// UTF-8(b) -> UTF-8(s)
// QString QString::fromUtf8();
//

#endif
