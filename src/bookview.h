/***************************************************************************
*   Copyright (C) 2007 by BOP                                             *
*   polepolek@gmail.com                                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef BOOKVIEW_H
#define BOOKVIEW_H

#include <QApplication>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QSplitter>
#include <eb/eb.h>

#include "method.h"

class QContextMenuEvent;
class QUrl;
class QFont;
class QKeyEvent;
#ifdef Q_WS_X11
class QMouseEvent;
#endif

class Book;
class Group;
class EBook;

//#define FIXED_POPUP

enum RET_SEARCH
{ NORMAL = 0, ERROR, NO_BOOK, NOT_HIT, INTERRUPTED, NOT_HIT_INTERRUPTED,
  NO_MENU, LIMIT_CHAR, LIMIT_BOOK, LIMIT_TOTAL, LIMIT_MENU };


class BookBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    BookBrowser(QWidget *parent = 0);
    void setSource(const QUrl &name);
    inline int fontSize() const
    {
        return document()->defaultFont().pointSize();
    }

    inline QList <Book*> bookList() const
    {
        return bookList_;
    }
    inline void addBookList(Book *book)
    {
        bookList_ << book;
    }
    void setBrowser(const QString str) 
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        setHtml(str);
        QApplication::restoreOverrideCursor();
    }

protected:
    QList <Book*> bookList_;
    void contextMenuEvent(QContextMenuEvent* event);
#ifdef Q_WS_X11
    void mouseReleaseEvent(QMouseEvent* event);
#endif

signals:
    void searchRequested(SearchDirection, const QString &);
    //void menuRequested(Book*, int, int);
    void pasteRequested(const QString&);
    void processRequested(const QString& prog);
    void soundRequested(const QString& fname);
    void selectionRequested(const QString& str);
    void statusRequested(const QString &str);

private slots:
    void changeTextSelection();
    void pasteSearchText();
};

#ifdef FIXED_POPUP

class BookBrowserPopup : public BookBrowser
{
public:
    BookBrowserPopup(QWidget *parent = 0)
        : BookBrowser(parent)
    {
    }
    inline void removeLastTitle()
    {
        titles_.removeLast();
    }
    inline void addTitle(const QString &title)
    {
        titles_ << title;
    }
    inline QStringList titles() const
    {
        return titles_;
    }

    void setSource(const QUrl &name);
private:
    QStringList titles_;

};

#endif

class ReferencePopup : public QWidget
{
public:
    ReferencePopup(Book *pbook, const EB_Position &pos, QWidget *parent, 
                   bool menu_flag=false);

    QString browserText(Book *pbook, const EB_Position &pos);

private:
    bool menuFlag;
#ifdef FIXED_POPUP
    BookBrowserPopup *bookBrowser;
#else
    BookBrowser *bookBrowser;
#endif
};


class TreeScrollPopup : public QWidget
{
    Q_OBJECT
public:
    TreeScrollPopup(QTreeWidgetItem *item, QWidget *parent);

private slots:
    void changeExpand(int value)
    {
        expand(topItem, 0, value);
    }

private:
    void countDepth(QTreeWidgetItem *item, int cur_depth, int *max_depth);
    void expand(QTreeWidgetItem *item, int cur_depth, int set_depth);
    QTreeWidgetItem *topItem;
};

class PageItems : public QObject
{

public:
    PageItems(const QString &ssheet);

    void composeHeader(const QString &ssheet);
    void composeError(const QString &anchor, const QString &text);
    void composeHLine(int num, const QString &anchor, const QString &title_l,
                      const QString &title_t, const QString &text);
    void composeHLine(int num, const QString &anchor, const QString &title,
                      const QString &text = QString());
    void composeTrail();
    void addHItem(int num, const QString &anchor, const QString &title);
    void addHtmlStr(const QString &html) { text_ += html; }
    void addTextStr(const QString &str) { text_ += str; }
    void expand(int level);
    QTreeWidgetItem* curItem() { return curItem_; }
    QTreeWidgetItem* item(int level) { return itemP_[level]; } 
    QTreeWidgetItem* topItem() { return itemP_[0]; } 

    QString text() { return text_; }
    QList <QTreeWidgetItem*> items() { return items_; }
    int textLength() { return textLength_; }

private:
    QString text_;
    QList <QTreeWidgetItem*> items_;
    QTreeWidgetItem* itemP_[7];
    QTreeWidgetItem* curItem_;
    int textLength_;

};

class PageWidget : public QSplitter
{
    Q_OBJECT
public:
    PageWidget(QWidget *parent, const SearchMethod&);

    bool isMatch(const QString &str, const QStringList &list,
                 NarrowingLogic logic);
    bool getMatch(EBook *eb, int index, const QStringList &list,
                  NarrowingLogic logic,
                  QString *head_l, QString *head_v, QString *text);
    bool getText(EBook *eb, int lndex, QString *head_l, QString *head_v,
                 QString *text);
    QString emphasize(const QString &str, const QString &word);

    inline SearchMethod method() const
    {
        return method_;
    }
    RET_SEARCH retStatus;

protected:
    inline QString toAnchor(const QString &str, int num) const
    {
        return str + QString::number(num);
    }
    RET_SEARCH checkLimit(int text_length);

    QTreeWidget *bookTree;
    BookBrowser *bookBrowser;
    SearchMethod method_;
    int totalCount;
    int matchCount;

signals:
    void statusRequested(const QString &str);
    void selectionRequested(const QString& str);

private:
    void scrollTo(QTreeWidgetItem *to);

private slots:
    void scrollTo(QTreeWidgetItem *to, int)
    {
        scrollTo(to);
    }
    void scrollTo(QTreeWidgetItem *to, QTreeWidgetItem*)
    {
        scrollTo(to);
    }
    void changeFont(const QFont &font);
    void popupSlide(const QPoint &pos);
};

class InfoPage : public PageWidget
{
    Q_OBJECT
public:
    InfoPage(QWidget *parent, const SearchMethod&);

private:
    QString convSpecialChar(const QString&) const;
};

class MenuPage : public PageWidget
{
    Q_OBJECT
public:
    MenuPage(QWidget *parent, const SearchMethod&);

private slots:
    void changePage(QTreeWidgetItem*, int column);

private:
    void fullMenuPage();
    void selectMenuPage(int index);
    void getMenus(EBook *eb, const EB_Position &pos, PageItems *items,
                  int count);
    void getTopMenu(EBook *eb, const EB_Position &pos);
    QList <EB_Position> topMenus;
    QStringList topTitles;
    int menuCount;
    bool checkMax;
};

class WholePage : public PageWidget
{
    Q_OBJECT
public:
    WholePage(QWidget *parent, const SearchMethod &);


private slots:
    void changePage(QTreeWidgetItem*, int column);

private:
    RET_SEARCH readPage(int page);
    RET_SEARCH initSeqHits();
    QList <EB_Position> seqHits;

    int curPage;
};

class SearchPage : public PageWidget
{
public:
    SearchPage(QWidget *parent, const QStringList&, const SearchMethod&);
};

class SearchWholePage : public PageWidget
{
public:
    SearchWholePage(QWidget *parent, const QStringList&, const SearchMethod&);
};


class BookView : public QTabWidget
{
    Q_OBJECT
public:
    BookView(QWidget *parent);
    RET_SEARCH newPage(const QStringList &slist,
                       const SearchMethod &method, bool newTab = true);
    inline PageWidget* currentPage() const
    {
        return (PageWidget *)currentWidget();
    }

signals:
    void fontChanged(const QFont &font);
    void tabChanged(int tab);

private slots:
    void closeTab();
    void stopSearch();
    void zoomIn();
    void zoomOut();
};

#endif

