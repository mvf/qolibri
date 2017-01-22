#ifndef BOOKBROWSERPOPUP_H
#define BOOKBROWSERPOPUP_H

#include "bookbrowser.h"

#include <QStringList>

class BookBrowserPopup : public BookBrowser
{
public:
    BookBrowserPopup(QWidget *parent = 0);
    void removeLastTitle();
    void addTitle(const QString &title);
    QStringList titles() const { return titles_; }

    void setSource(const QUrl &name);
private:
    QStringList titles_;
};

#endif // BOOKBROWSERPOPUP_H
