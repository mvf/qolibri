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
#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>

const int CONNECT_TIMEOUT = 300000;
const qint16 DEFAULT_PORT = 5626;

class QoClient : public QTcpSocket
{
    Q_OBJECT
public:
    QoClient(const QString &host, qint16 port)
        : QTcpSocket(), hostName(host), portNo(port) {}

//    ~QoClient()
//    {
//        qDebug() << "~QoClient";
//        //currentState();
//    }

//    bool portTest()
//    {
//        connectToHost(hostName, portNo, QIODevice::WriteOnly);
//        if (waitForConnected(CONNECT_TIMEOUT)){
//            disconnectFromHost();
//            return true;
//        } else {
//            return false;
//        }
//    }

    bool connectHost()
    {
        //qDebug() << "connectHost" << hostName << portNo;
        connectToHost(hostName, portNo, QIODevice::WriteOnly);
        if (waitForConnected(CONNECT_TIMEOUT)){
            return true;
        } else {
            return false;
        }
    }

    bool sendText(const QByteArray &msg)
    {
        if (state() != QAbstractSocket::ConnectedState){
            if (!connectHost()) {
                return false;
            }
        }
        write(msg);
        if (waitForBytesWritten(30000)) {
            return true;
        } else {
            qWarning() << "QoClient: write error port =" << portNo;
            //currentState();
            return false;
        }
        disconnectFromHost();
    }

//    void currentState()
//    {
//        switch(state()) {
//            case QAbstractSocket::UnconnectedState:
//                qDebug() << "The socket is not connected.";
//                break;
//            case QAbstractSocket::HostLookupState:
//                qDebug() << "The socket is performing a host name lookup.";
//                break;
//            case QAbstractSocket::ConnectingState:
//                qDebug() << "The socket has started establishing a connection.";
//                break;
//            case QAbstractSocket::ConnectedState:
//                qDebug() << "A connection is established.";
//                break;
//            case QAbstractSocket::BoundState:
//                qDebug() << "The socket is bound to an address and port.";
//                break;
//            case QAbstractSocket::ClosingState:
//                qDebug() << "The socket is about to close.";
//                break;
//            default:
//                qDebug() << "Undefined socket state =" << state();
//        }
//    }

private:

    QString hostName;
    qint16 portNo;

};

#endif

