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

#include <QtGui>
#include <QtNetwork>

#include "mainwindow.h"
#include "server.h"

QoServer::QoServer(MainWindow *mainw, int port)
    : QTcpServer(0)
{
    if (!listen(QHostAddress::Any, port)) {
        qDebug() << "Server Listen Error";
        return;
    }
    if (port == 0){
        port = serverPort();
        qDebug() << "Server Port No =" << port;
    }
    connect(this, SIGNAL(newConnection()), this, SLOT(getClientText()));
    connect(this, SIGNAL(searchRequested(const QString&)),
            mainw, SLOT(searchClientText(const QString&))); 
    connect(this, SIGNAL(statusRequested(const QString&)),
            mainw, SLOT(showStatus(const QString&))); 
    QString msg = "Server Port No = " + QString::number(port);
    emit statusRequested(msg);
    return;
}
void QoServer::getClientText()
{
    QTcpSocket *c = nextPendingConnection();
    connect(c, SIGNAL(disconnected()), c, SLOT(deleteLater()));
    c->waitForReadyRead();
    QString str;
    str = c->read(1000);
    emit searchRequested(str);
}
