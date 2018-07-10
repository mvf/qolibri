HEADERS += \
           src/allpage.h \
           src/book.h \
           src/bookbrowser.h \
           src/bookbrowserpopup.h \
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
           src/infopage.h \
           src/mainwindow.h \
           src/menupage.h \
           src/method.h \
           src/model.h \
           src/optiondialog.h \
           src/pageitems.h \
           src/pagewidget.h \
           src/qeb.h \
           src/referencepopup.h \
           src/searchpage.h \
           src/searchwholepage.h \
           src/server.h \
           src/ssheetsetting.h \
           src/textcodec.h \
           src/titlelabel.h \
           src/toolbar.h \
           src/treescrollpopup.h \
           src/webpage.h \

SOURCES += \
           src/allpage.cpp \
           src/book.cpp \
           src/bookbrowser.cpp \
           src/bookbrowserpopup.cpp \
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
           src/infopage.cpp \
           src/mainwindow.cpp \
           src/menupage.cpp \
           src/method.cpp \
           src/model.cpp \
           src/optiondialog.cpp \
           src/pageitems.cpp \
           src/pagewidget.cpp \
           src/qeb.cpp \
           src/qolibri.cpp \
           src/referencepopup.cpp \
           src/searchpage.cpp \
           src/searchwholepage.cpp \
           src/ssheetsetting.cpp \
           src/textcodec.cpp \
           src/toolbar.cpp \
           src/treescrollpopup.cpp \
           src/webpage.cpp \

RESOURCES += qolibri.qrc

FORMS += src/optiondialog.ui

lessThan(QT_MAJOR_VERSION, 5): error("Qt 5 or later required")

QT += multimedia network webkit webkitwidgets widgets

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
    isEmpty(INSTALL_PREFIX):INSTALL_PREFIX=/usr
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
    !exists($$QMAKE_LRELEASE) { QMAKE_LRELEASE = lrelease }
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

VERSION_STR = $$git(describe --tags --match '[0-9]*' --always)

isEmpty(VERSION_STR) {
    DEFINES += $$defStr(VERSION_STR, 2.0.2)
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
