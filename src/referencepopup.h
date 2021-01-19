#ifndef REFERENCEPOPUP_H
#define REFERENCEPOPUP_H

#include <QWidget>

#include <ebu/eb.h>

class Book;
class BookBrowser;
class BookBrowserPopup;

class ReferencePopup : public QWidget
{
    Q_OBJECT
public:
    ReferencePopup(Book *pbook, const EB_Position &pos, QWidget *parent,
                   bool menu_flag=false);

    QString browserText(Book *pbook, const EB_Position &pos);
    BookBrowser *bookBrowser();

protected:
    void showEvent(QShowEvent*);

private:
    bool menuFlag;
#ifdef FIXED_POPUP
    BookBrowserPopup *bookBrowser_;
#else
    BookBrowser *bookBrowser_;
#endif
};

#endif // REFERENCEPOPUP_H
