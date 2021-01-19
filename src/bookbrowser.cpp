#include "bookbrowser.h"
#include "configure.h"
#include "ebcache.h"
#include "referencepopup.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>

#include <ebu/eb.h>

BookBrowser::BookBrowser(QWidget *parent)
: QTextBrowser(parent)
{
    setSearchPaths(QStringList() << EbCache::cachePath);
    document()->setDefaultFont(CONF->browserFont);

    connect(this, SIGNAL(selectionChanged()), SLOT(changeTextSelection()));
}

void BookBrowser::setSource(const QUrl &name)
{
    const QString path = name.path();
    const QStringList args = name.query().split('?');

    if (path == "sound") {
        // args[0] : wave file
        emit soundRequested(args[0]);
    } else if (path == "book" || path == "menu") {
        // args[0] : book index
        // args[1] : page
        // args[2] : offset
        if (args.count() == 3) {
            const int index = args[0].toInt();
            if (index >= bookList_.count()) {
                qWarning() << "Invalid book index" << args[0];
                emit statusRequested("ERROR: Invalid book index: " + args[0]);
                return;
            }
            EB_Position pos;
            pos.page = args[1].toInt();
            pos.offset = args[2].toInt();

            ReferencePopup *popup =
                new ReferencePopup(bookList_[index], pos, this, path == "menu");
            connect(popup->bookBrowser(), SIGNAL(statusRequested(QString)),
                    SIGNAL(statusRequested(QString)));
            connect(popup->bookBrowser(),
                    SIGNAL(searchRequested(SearchDirection,QString)),
                    SIGNAL(searchRequested(SearchDirection,QString)));
            connect(popup->bookBrowser(), SIGNAL(pasteRequested(QString)),
                    SIGNAL(pasteRequested(QString)));
            connect(popup->bookBrowser(), SIGNAL(soundRequested(QString)),
                    SIGNAL(soundRequested(QString)));
            connect(popup->bookBrowser(), SIGNAL(videoRequested(QString)),
                    SIGNAL(videoRequested(QString)));
            connect(popup->bookBrowser(), SIGNAL(externalLinkRequested(QString)),
                    SIGNAL(externalLinkRequested(QString)));
            connect(popup->bookBrowser(), SIGNAL(selectionRequested(QString)),
                    SIGNAL(selectionRequested(QString)));
            popup->show();
        } else {
            qWarning() << "Invalid Reference Parameter" << args.count();
        }
    } else if (path == "mpeg") {
        // args[0] : mpeg file
        emit videoRequested(args[0]);
    } else if (path == "close") {
        parentWidget()->close();
    } else {
        qWarning() << "Invalid Command" << path;
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
            emit externalLinkRequested(addr);
        }
    }
    delete menu;
}

// Implement for linux.
// For "selectionChanged" SIGNAL not allways invoked at mouse move and
// release timing.
void BookBrowser::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        changeTextSelection();

    QTextEdit::mouseReleaseEvent(ev);
}

void BookBrowser::changeTextSelection()
{
    emit selectionRequested(textCursor().selectedText());
}

void BookBrowser::pasteSearchText()
{
    emit pasteRequested(textCursor().selectedText());
}

