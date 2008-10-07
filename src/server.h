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
#ifndef SERVER_H
#define SERVER_H

#include <QtGui>
#include <QTcpServer>
#include <QTcpSocket>

class QoServer : public QTcpServer
{

   Q_OBJECT

public:
    QoServer(qint16 port)
        : QTcpServer(0)
    {

        //if (port == 0)
        //    port = 5626;
    
        if (!listen(QHostAddress::Any, port)) {
            qWarning() << "Server Listen Error : port =" << port;
            return;
        }
    
        connect(this, SIGNAL(newConnection()), SLOT(getClientText()));

    }

//    QoServer::~QoServer()
//    {
//        qDebug() << "~QoServer()";
//    }

    void slotShowStatus(const QObject *receiver, const char *member)
    {
        connect(this, SIGNAL(statusRequested(const QString&)),
                receiver, member); 
    }

    void slotSearchText(const QObject *receiver, const char *member)
    {
        connect(this, SIGNAL(searchRequested(const QString&)),
                receiver, member); 
    }

    void showStatus(const QString &str)
    {
        emit statusRequested(str);
    }
    
signals:
    void searchRequested(const QString &str);
    void statusRequested(const QString &msg);

private slots:
    
    void getClientText()
    {

        QTcpSocket *c = nextPendingConnection();
        //connect(c, SIGNAL(disconnected()), this, SLOT(disconnectTest()));
        connect(c, SIGNAL(disconnected()), c, SLOT(deleteLater()));
        c->waitForReadyRead(30000);
        QString str = c->read(1000);
        emit searchRequested(str);

    }

//    void disconnectTest()
//    {
//        qDebug() << "disconnectTest";
//    }

};

#endif

