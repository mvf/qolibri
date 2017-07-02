HEADERS += src/mainwindow.h \
           src/method.h \
           src/book.h \
           src/qeb.h \
           src/ebcore.h \
           src/ebcache.h \
           src/ebook.h \
           src/bookview.h \
           src/globaleventfilter.h \
           src/groupdock.h \
           src/groupwidget.h \
           src/bookwidget.h \
           src/configure.h \
           src/booksetting.h \
           src/optiondialog.h \
           src/fontsetting.h  \
           src/model.h \
           src/ssheetsetting.h \
           src/titlelabel.h \
           src/toolbar.h \
           src/server.h \
           src/client.h \
           src/textcodec.h

SOURCES += src/qolibri.cpp \
           src/mainwindow.cpp \
           src/mainwindow-about.cpp \
           src/method.cpp \
           src/book.cpp \
           src/qeb.cpp \
           src/ebcore.cpp \
           src/ebcache.cpp \
           src/ebhook.cpp \
           src/ebook.cpp \
           src/bookview.cpp \
           src/globaleventfilter.cpp \
           src/groupdock.cpp \
           src/groupwidget.cpp \
           src/bookwidget.cpp \
           src/configure.cpp \
           src/booksetting.cpp \
           src/optiondialog.cpp \
           src/fontsetting.cpp \
           src/model.cpp \
           src/ssheetsetting.cpp \
           src/textcodec.cpp \
           src/toolbar.cpp

RESOURCES += qolibri.qrc

FORMS += src/optiondialog.ui

QT += network
QT += webkit
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets webkitwidgets multimedia
} else {
    DEFINES += QStringLiteral=QString::fromUtf8
}

include("config")

TARGET = qolibri
DESTDIR = .
OBJECTS_DIR = src/.build
MOC_DIR = src/.build
RCC_DIR = src/.build
UI_DIR = src/.build
LIBS += -leb -lz

TRANSLATIONS = translations/qolibri_ja_JP.ts

#DEFINES += USE_GIF_FOR_FONT
#DEFINES += FIXED_POPUP
#DEFINES += RUBY_ON_TOOLBAR

unix:!macx {
    isEmpty(INSTALL_PREFIX):INSTALL_PREFIX=/usr/local
    isEmpty(INSTALL_BINDIR):INSTALL_BINDIR=$$INSTALL_PREFIX/bin
    isEmpty(INSTALL_DATADIR):INSTALL_DATADIR=$$INSTALL_PREFIX/share
    isEmpty(INSTALL_PKGDATADIR):INSTALL_PKGDATADIR=$$INSTALL_DATADIR/$$TARGET

    DEFINES += PKGDATADIR=\\\"$$INSTALL_PKGDATADIR\\\"

    target.path = $$INSTALL_BINDIR
    INSTALLS += target
    translations.path = $$INSTALL_PKGDATADIR
    translations.files = translations
    samples.path = $$INSTALL_PKGDATADIR
    samples.files = i18n/qolibri/*
}

INSTALLS += translations samples

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
    !exists($$QMAKE_LRELEASE) { QMAKE_LRELEASE = lrelease-qt4 }
}

updateqm.input = TRANSLATIONS
updateqm.output = translations/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE -silent ${QMAKE_FILE_IN} -qm translations/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm

gitversion.target = src/gitversion.h
gitversion.commands = src/gitversion.sh > src/gitversion.h
gitversion.depends = ${SOURCES} ${HEADERS}
QMAKE_EXTRA_TARGETS += gitversion
PRE_TARGETDEPS += src/gitversion.h

message(Version = $$QT_VERSION)
message(Config = $$CONFIG)
message(Plugins = $$QTPLUGIN)
message(Defines  = $$DEFINES)
message(Libs = $$LIBS)
message(Include Path = $$INCLUDEPATH)
message(Installs = $$INSTALLS)
message(Target  = $$TARGET)
