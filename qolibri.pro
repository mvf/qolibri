HEADERS += \
           src/book.h \
           src/booksetting.h \
           src/bookview.h \
           src/bookwidget.h \
           src/client.h \
           src/configure.h \
           src/ebcache.h \
           src/ebcore.h \
           src/ebook.h \
           src/fontsetting.h  \
           src/globaleventfilter.h \
           src/groupdock.h \
           src/groupwidget.h \
           src/mainwindow.h \
           src/method.h \
           src/model.h \
           src/optiondialog.h \
           src/qeb.h \
           src/server.h \
           src/ssheetsetting.h \
           src/textcodec.h \
           src/titlelabel.h \
           src/toolbar.h \

SOURCES += \
           src/book.cpp \
           src/booksetting.cpp \
           src/bookview.cpp \
           src/bookwidget.cpp \
           src/configure.cpp \
           src/ebcache.cpp \
           src/ebcore.cpp \
           src/ebhook.cpp \
           src/ebook.cpp \
           src/fontsetting.cpp \
           src/globaleventfilter.cpp \
           src/groupdock.cpp \
           src/groupwidget.cpp \
           src/mainwindow.cpp \
           src/method.cpp \
           src/model.cpp \
           src/optiondialog.cpp \
           src/qeb.cpp \
           src/qolibri.cpp \
           src/ssheetsetting.cpp \
           src/textcodec.cpp \
           src/toolbar.cpp \

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

defineReplace(defStr) {
    return(QOLIBRI_$$1='"\\\"$$2\\\""')
}

defineReplace(git) {
    return($$system(git --git-dir "$$PWD/.git" --work-tree "$$PWD" $$1))
}

TRANSLATIONS = translations/qolibri_ja_JP.ts

#DEFINES += USE_GIF_FOR_FONT
#DEFINES += FIXED_POPUP
#DEFINES += RUBY_ON_TOOLBAR

unix:!macx {
    isEmpty(INSTALL_PREFIX):INSTALL_PREFIX=/usr/local
    isEmpty(INSTALL_BINDIR):INSTALL_BINDIR=$$INSTALL_PREFIX/bin
    isEmpty(INSTALL_DATADIR):INSTALL_DATADIR=$$INSTALL_PREFIX/share
    isEmpty(INSTALL_PKGDATADIR):INSTALL_PKGDATADIR=$$INSTALL_DATADIR/$$TARGET

    DEFINES += $$defStr(PKGDATADIR, $$INSTALL_PKGDATADIR)

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

# This #defines the following preprocessor string tokens with the empty string as a fallback:
# QOLIBRI_GIT_COMMIT_DATE - HEAD's commit date in ISO format, e.g. "2017-01-13 13:52:55 +0100"
# QOLIBRI_VERSION_STR     - Version name, e.g. 1.0.4-3-g4dead, or the hard-coded version number
# QOLIBRI_WEBSITE         - The contents of the WEBSITE variable

VERSION_STR = $$git(describe --always --tags)

isEmpty(VERSION_STR) {
    DEFINES += $$defStr(VERSION_STR, 1.0.4)
} else {
    DEFINES += $$defStr(VERSION_STR, $$VERSION_STR)
}

DEFINES += $$defStr(GIT_COMMIT_DATE, $$git(log -1 --format=%ci))
DEFINES += $$defStr(WEBSITE, $$WEBSITE)

message(Version = $$QT_VERSION)
message(Config = $$CONFIG)
message(Plugins = $$QTPLUGIN)
message(Defines  = $$DEFINES)
message(Libs = $$LIBS)
message(Include Path = $$INCLUDEPATH)
message(Installs = $$INSTALLS)
message(Target  = $$TARGET)
equals(WEBSITE, "") {
    message("Define the WEBSITE variable to put a hyperlink into the about dialog.")
}
