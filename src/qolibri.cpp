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
#include "mainwindow.h"
#include "configure.h"
#include "method.h"
#include "model.h"
#include "server.h"
#include "client.h"
#include "textcodec.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#ifdef USE_STATIC_PLUGIN
#include <QtPlugin>
#endif

#if defined (USE_STATIC_PLUGIN)
Q_IMPORT_PLUGIN(qjpcodecs)
Q_IMPORT_PLUGIN(qjpeg)
#endif

const char * const usage =
    "\n"
    "Usage:\n"
    "   qolibri [argument] [search text...] \n"
    "\n"
    "Arguments:\n"
    "   -s                Client/Server mode\n"
    "   -p <port-no>      Server Port no (default:5626)\n"
    "   -c <session name> Configuration session name\n"
    "   -h  or  --help    Print Help (this message) and exit\n"
    "   --version         Print version information and exit";

const char *version =
    "qolibri - EPWING Dictionary/Book Viewer " QOLIBRI_VERSION_STR;

QoServer *server;

extern Configure *configure_s;

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(qolibri);

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    configure_s = new Configure();
    CONF->load();
    EbCache::initialize();

    Model model;
    model.load();

    QString searchText;
    qint16 port = CONF->portNo;
    bool qserv = CONF->serverMode;

    for(int i=1; i<argc; i++){
        QString str = QString::fromLocal8Bit(argv[i]);
        if (str == "-c" && (i+1) < argc) {
            CONF->settingOrg = QString::fromLocal8Bit(argv[i+1]);
            i++;

        } else if (str == "-p" && (i+1) < argc) {
            bool ok;
            port = QString::fromLocal8Bit(argv[i+1]).toInt(&ok);
            if (ok) {
                i++;
            } else {
                qWarning() << "can't convert port no (" << argv[i+1] << ")";
            }
        } else if (str == "-s") {
            qserv = true;
        } else if (str == "-h" || str == "--help") {
            qDebug() << version;
            qDebug() << usage;
            return 1;
        } else if (str == "--version") {
            qDebug() << version;
            return 1;
        } else {
            searchText += str;
            if ( (i+1) < argc) searchText += " ";
        }
    }

    if (qserv) {
        QoClient client("localhost", port);
        if (client.connectHost()) {
            client.sendText(searchText.toLocal8Bit());
            //client.disconnectFromHost();
            return 1;
        }
    }

    QTranslator qtTranslator;
    if (qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(&qtTranslator);

    QTranslator trans;
    QString thename(QStringLiteral(":/translations/qolibri_") + QLocale::system().name());
    if (trans.load(thename))
        QApplication::installTranslator(&trans);

    MainWindow mainWin(&model, searchText);

    mainWin.show();

    if (qserv) {
        server = new QoServer(port);
        server->slotSearchText(&mainWin,
                               SLOT(searchClientText(const QString&)));
        server->slotShowStatus(&mainWin,
                               SLOT(showStatus(const QString&)));
        server->showStatus(QString("Start as server (port = %1)")
                                  .arg(server->serverPort()));
    }
    //if (!searchText.isEmpty()) {
    //    emit mainWin.searchClientText(searchText);
    //}


    int ret = app.exec();

    delete server;
    model.save();

    return ret;
}

