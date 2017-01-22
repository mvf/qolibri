#include "bookbrowser.h"
#include "configure.h"
#include "ebcache.h"
#include "referencepopup.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QAudioDeviceInfo>
#else
#include <QSound>
#endif

#include <eb/eb.h>

BookBrowser::BookBrowser(QWidget *parent)
: QTextBrowser(parent)
{
    setSearchPaths(QStringList() << EbCache::cachePath);
    document()->setDefaultFont(CONF->browserFont);

    connect(this, SIGNAL(selectionChanged()), SLOT(changeTextSelection()));
}

void BookBrowser::setSource(const QUrl &name)
{

    QStringList args = name.toString().split('?');

    if (args[0] == "sound") {
        // args[1] : wave file
        if (!CONF->waveProcess.isEmpty()) {
            emit processRequested(CONF->waveProcess + ' ' + args[1]);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        } else if (!QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).isEmpty()) {
#else
        } else if (QSound::isAvailable()) {
#endif
            emit soundRequested(args[1]);
        } else {
            qWarning() << "Can't play sound" << CONF->waveProcess << args[1];
            emit statusRequested("Can't play sound");
        }
    } else if (args[0] == "book" || args[0] == "menu") {
        // args[1] : book index
        // args[2] : page
        // args[3] : offset
        if ( args.count() == 4) {
            int index = args[1].toInt();
            if (index >= bookList_.count()) {
                qWarning() << "Invalid book index" << args[1];
                emit statusRequested("ERROR: Invalid book index: " + args[1]);
                return;
            }
            bool mflag = (args[0] == "menu") ? true : false;
            EB_Position pos;
            pos.page = args[2].toInt();
            pos.offset = args[3].toInt();

            ReferencePopup *popup =
                new ReferencePopup(bookList_[index], pos, this, mflag);
            connect(popup->bookBrowser(), SIGNAL(statusRequested(QString)),
                    SIGNAL(statusRequested(QString)));
            connect(popup->bookBrowser(),
                    SIGNAL(searchRequested(SearchDirection,QString)),
                    SIGNAL(searchRequested(SearchDirection,QString)));
            connect(popup->bookBrowser(), SIGNAL(pasteRequested(QString)),
                    SIGNAL(pasteRequested(QString)));
            connect(popup->bookBrowser(), SIGNAL(processRequested(QString)),
                    SIGNAL(processRequested(QString)));
            connect(popup->bookBrowser(), SIGNAL(soundRequested(QString)),
                    SIGNAL(soundRequested(QString)));
            connect(popup->bookBrowser(), SIGNAL(selectionRequested(QString)),
                    SIGNAL(selectionRequested(QString)));
            popup->show();
        } else {
            qWarning() << "Invalid Reference Parameter" << args.count();
        }
    } else if (args[0] == "mpeg") {
        // args[1] : mpeg file
        if (!CONF->mpegProcess.isEmpty()) {
            emit processRequested(CONF->mpegProcess + ' ' + args[1]);
        } else {
            qWarning() << "Can't play moview";
            emit statusRequested("Can't play Movie");
        }
    } else if (args[0] == "close") {
        parentWidget()->close();
    } else {
        qWarning() << "Invalid Command" << args[0];
    }
}

int BookBrowser::fontSize() const
{
    return document()->defaultFont().pointSize();
}

void BookBrowser::addBookList(Book *book)
{
    bookList_ << book;
}

void BookBrowser::setBrowser(const QString &str)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    setHtml(str);
    QApplication::restoreOverrideCursor();
}

void BookBrowser::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu *menu = createStandardContextMenu();

    //if (textCursor().hasSelection() &&
    //    parent()->parent()->objectName() == "main_browser") {
    if (textCursor().hasSelection()) {
        menu->addSeparator();
        addDirectionMenu(menu);
        menu->addSeparator();
        menu->addAction(QObject::tr("&Paste selected string to edit line"),
                this, SLOT(pasteSearchText()));
    }
    QAction *a = menu->exec(event->globalPos());
    if (a && a->data().isValid()){
       // qDebug() << a->data().typeName();
        SearchDirection d = (SearchDirection)a->data().toInt();
        if (d <= MenuRead) {
            emit searchRequested(d, textCursor().selectedText());
        } else {
            QString addr;
            if (d == GoogleSearch) {
                addr = CONF->googleUrl;
            } else if (d == WikipediaSearch) {
                addr = CONF->wikipediaUrl;
            } else if (d == Option1Search) {
                addr = CONF->userDefUrl;
            } else {
                qWarning() << "Selected Invalid Menu?";
                delete menu;
                return;
            }
            foreach(const char c, textCursor().selectedText().toUtf8()) {
                addr += "%" + QString::number((ushort)((uchar)c), 16);
            }
            emit processRequested(CONF->browserProcess + ' ' + addr);
        }
    }
    delete menu;
}

//#ifdef Q_WS_X11
// Implement for linux.
// For "selectionChanged" SIGNAL not allways invoked at mouse move and
// release timing.
void BookBrowser::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        changeTextSelection();

    QTextEdit::mouseReleaseEvent(ev);
}
//#endif

void BookBrowser::changeTextSelection()
{
    emit selectionRequested(textCursor().selectedText());
}

void BookBrowser::pasteSearchText()
{
    emit pasteRequested(textCursor().selectedText());
}

