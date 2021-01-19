#include "bookbrowserpopup.h"
#include "referencepopup.h"

#include <QDebug>

#include <ebu/eb.h>

BookBrowserPopup::BookBrowserPopup(QWidget *parent)
: BookBrowser(parent)
{
}

void BookBrowserPopup::removeLastTitle()
{
    titles_.removeLast();
}

void BookBrowserPopup::addTitle(const QString &title)
{
    titles_ << title;
}

void BookBrowserPopup::setSource(const QUrl &name)
{
    QStringList args = name.toString().split('|');

    if ((args[0] == "book" || args[0] == "menu") && args.count() <= 5) {
        // args[1] : book index
        // args[2] : page
        // args[3] : offset
        // args[4] : title index
        int index = args[1].toInt();
        if (index >= bookList_.count()) {
            qWarning() << "Invalid book index" << index;
            return;
        }
        if (args.count() == 5) {
            for (int i = titles_.count(); i > args[4].toInt(); i--) {
                titles_.removeLast();
            }
        }
        EB_Position pos;
        pos.page = args[2].toInt();
        pos.offset = args[3].toInt();
        ReferencePopup *popup = (ReferencePopup*)parentWidget();
        QString str = popup->browserText(bookList_[index], pos );
        setBrowser(str);
        return;
    }
    BookBrowser::setSource(name);
}
