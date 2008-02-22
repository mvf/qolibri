HEADERS += src/mainwindow.h \
           src/method.h \
           src/book.h \
           src/ebook.h \
           src/ebook_hooks.h \
           src/bookview.h \
           src/groupdock.h \
           src/groupwidget.h \
           src/bookwidget.h \
           src/configure.h \
           src/booksetting.h \
           src/fontsetting.h  \
           src/ssheetsetting.h \
           src/configsetting.h \
           src/ssheet.h \
           src/titlelabel.h \
           src/statusbutton.h

SOURCES += src/qolibri.cpp \
           src/mainwindow.cpp \
           src/method.cpp \
           src/book.cpp \
           src/ebook.cpp \
           src/ebook_hooks.cpp \
           src/bookview.cpp \
           src/groupdock.cpp \
           src/groupwidget.cpp \
           src/bookwidget.cpp \
           src/configure.cpp \
           src/booksetting.cpp \
           src/fontsetting.cpp \
           src/ssheetsetting.cpp \
           src/configsetting.cpp \
           src/statusbutton.cpp

INCLUDEPATH += src
RESOURCES += src/qolibri.qrc
OBJECTS_DIR += build
MOC_DIR += build
RCC_DIR += build

TARGET = qolibri
DESTDIR = ./bin

#DEFINES += USE_GIF_FOR_FONT

macx {
    !exists($$[QT_INSTALL_LIBS]/QtCore.framework) {
        CONFIG += static
        QTPLUGIN += qjpcodecs qjpeg
        DEFINES += USE_STATIC_PLUGIN
        LIBS += /usr/lib/libeb.a
        LIBS += -dead_strip
    } else {
        LIBS += -leb
    }
    i18n.path = $$DESTDIR/"$$TARGET".app/Contents/MacOS/i18n
    i18n_s.path = $$DESTDIR/"$$TARGET".app/Contents/MacOS/i18n/qolibri
    ICON = src/qolibri.icns
}
win32 {
    INCLUDEPATH += "C:\Program Files\EB Library\include"
    LIBS += -L"C:\Program Files\EB Library\lib"
    !exists($$[QT_INSTALL_LIBS]/QtCore4.dll) {
        CONFIG += static
        QTPLUGIN += qjpcodecs qjpeg
        DEFINES += USE_STATIC_PLUGIN
        LIBS += -dead_strip
    }
    #debug {
    #    CONFIG += console
    #}
    LIBS += -leb

    #
    # Error will be occurred, when you try to "mingw32-make install".
    # This may be BUG. (see Makefile.Debug or MakeFile.Release).
    # Then please copy translation files to target directory manually.
    #
    i18n.path = $$DESTDIR/i18n
    i18n_s.path = $$DESTDIR/i18n/qolibri
    RC_FILE = src/qolibri.rc
} 
unix:!macx {
    i18n.path = $$[QT_INSTALL_TRANSLATIONS]
    i18n_s.path = $$[QT_INSTALL_TRANSLATIONS]/qolibri
    LIBS += -leb
}

i18n.files = src/i18n/*.qm
i18n_s.files = src/i18n/qolibri/*sample*
INSTALLS += i18n

TRANSLATIONS = src/qolibri_ja_JP.ts

message(Version = $$QT_VERSION)
macx {
    message(Translations = ./src/bin/qolibri.app/Contents/MacOS/i18n)
} else {
    message(Translations = $$[QT_INSTALL_TRANSLATIONS])
}
message(Config = $$CONFIG)
message(Plugins = $$QTPLUGIN)
message(Defines  = $$DEFINES)
message(Libs = $$LIBS)
message(Include Path = $$INCLUDEPATH)
message(Installs = $$INSTALLS)
message(Target  = $$TARGET)
