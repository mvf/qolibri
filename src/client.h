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

//#include <QtCore>
#include <QTcpSocket>

const int CONNECT_TIMEOUT = 300000;
const int DEFAULT_PORT = 5626;

class QoClient : public QTcpSocket
{
   Q_OBJECT
public:
    QoClient(const QString &host, int port)
        : QTcpSocket(), hostName(host)
   {
       portNo = port ? port : DEFAULT_PORT;
   }


    bool connectHost()
    {
        //qDebug() << "connectHost" << hostName << portNo;
        connectToHost(hostName, portNo);
        if (waitForConnected(CONNECT_TIMEOUT)){
            return true;
        } else {
            return false;
        }
    }
    bool sendText(const QByteArray &msg)
    {
        write(msg);
        if (waitForBytesWritten()) {
            return true;
        } else {
            return false;
        }
    }
    
private:
    QString hostName;
    int portNo;
};

#endif

