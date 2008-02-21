HEADERS += mainwindow.h \
           method.h \
           book.h \
           ebook.h \
           ebook_hooks.h \
           bookview.h \
           groupdock.h \
           groupwidget.h \
           bookwidget.h \
           configure.h \
           booksetting.h \
           fontsetting.h  \
           ssheetsetting.h \
           configsetting.h \
           ssheet.h \
           titlelabel.h \
           statusbutton.h

SOURCES += qolibri.cpp \
           mainwindow.cpp \
           method.cpp \
           book.cpp \
           ebook.cpp \
           ebook_hooks.cpp \
           bookview.cpp \
           groupdock.cpp \
           groupwidget.cpp \
           bookwidget.cpp \
           configure.cpp \
           booksetting.cpp \
           fontsetting.cpp \
           ssheetsetting.cpp \
           configsetting.cpp \
           statusbutton.cpp

RESOURCES += qolibri.qrc

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
    i18n.extra = cp -f i18n/* $$DESTDIR/"$$TARGET".app/Contents/MacOS/i18n
    ICON = qolibri.icns
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
    i18n.path = $$DESTDIR/i18n
    i18n.extra = copy i18n/* $$DESTDIR/i18n
    RC_FILE = qolibri.rc
} 
unix:!macx {
    i18n.path = $$[QT_INSTALL_TRANSLATIONS]
    i18n.extra = cp -f i18n/* $$[QT_INSTALL_TRANSLATIONS]
    LIBS += -leb
}

INSTALLS += i18n

TRANSLATIONS = qolibri_ja_JP.ts

message(Version = $$QT_VERSION)
macx {
    message(Translations = ./bin/qolibri.app/Contents/MacOS/i18n)
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
