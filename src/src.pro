HEADERS += mainwindow.h \
           method.h \
           book.h \
           qeb.h \
           ebcore.h \
           ebcache.h \
           ebook.h \
           bookview.h \
           groupdock.h \
           groupwidget.h \
           bookwidget.h \
           configure.h \
           booksetting.h \
           optiondialog.h \
           fontsetting.h  \
           ssheetsetting.h \
           ssheet.h \
           titlelabel.h \
           statusbutton.h \
           server.h \
           client.h \
           textcodec.h

SOURCES += qolibri.cpp \
           mainwindow.cpp \
           method.cpp \
           book.cpp \
           qeb.cpp \
           ebcore.cpp \
           ebcache.cpp \
           ebhook.cpp \
           ebook.cpp \
           bookview.cpp \
           groupdock.cpp \
           groupwidget.cpp \
           bookwidget.cpp \
           configure.cpp \
           booksetting.cpp \
           optiondialog.cpp \
           fontsetting.cpp \
           ssheetsetting.cpp \
           statusbutton.cpp \
           textcodec.cpp

QT += network
QT += webkit
RESOURCES += qolibri.qrc
FORMS += optiondialog.ui

TARGET = qolibri
DESTDIR = ./bin
OBJECTS_DIR = ./build
MOC_DIR = ./build/moc

#DEFINES += USE_GIF_FOR_FONT
#DEFINES += FIXED_POPUP
#DEFINES += RUBY_ON_TOOLBAR

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

    ICON = qolibri.icns

    i18n.path = $$DESTDIR/"$$TARGET".app/Contents/MacOS/i18n
    i18n.extra = cp -rf i18n/*.qm $$DESTDIR/"$$TARGET".app/Contents/MacOS/i18n
    i18n_s.path = $$DESTDIR/"$$TARGET".app/Contents/MacOS/i18n/qolibri
    i18n_s.extra = cp -rf i18n/qolibri/*sample* $$DESTDIR/"$$TARGET".app/Contents/MacOS/i18n/qolibri
}
win32 {
    INCLUDEPATH += "C:\Program Files\EB Library\include"
    LIBS += -L"C:\Program Files\EB Library\lib"
    #debug {
    #    CONFIG += console
    #}
    LIBS += -leb
    RC_FILE = qolibri.rc

    i18n.path = $$DESTDIR/i18n
    i18n_s.path = $$DESTDIR/i18n/qolibri
    i18n.files = i18n/*.qm
    i18n_s.files = i18n/qolibri/*sample*

} 
unix:!macx {

    LIBS += -leb

    target.path = /usr/bin
    target.files = bin/qolibri
    INSTALLS += target
    i18n.path = $$[QT_INSTALL_TRANSLATIONS]
    i18n.extra = cp -rf i18n/*.qm $$[QT_INSTALL_TRANSLATIONS]
    i18n_s.path = $$[QT_INSTALL_TRANSLATIONS]/qolibri
    i18n_s.extra = cp -rf i18n/qolibri/*sample* $$[QT_INSTALL_TRANSLATIONS]/qolibri
}

INSTALLS += i18n i18n_s

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
