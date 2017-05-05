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
#if QT_VERSION >= 0x050000
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QToolBar>
#include <QSound>
#include <QFontDialog>
#include <QtWebKitWidgets/QWebView>
#else
#include <QWebView>
#endif
#include <eb/eb.h>

#include "book.h"
#include "method.h"
#include "ebcore.h"

class QContextMenuEvent;
class QUrl;
class QFont;
class QKeyEvent;
//#ifdef Q_WS_X11
class QMouseEvent;
//#endif

class Book;
class Group;
class EBook;
class EbMenu;

//#define FIXED_POPUP

enum RET_SEARCH
{ NORMAL = 0, SEARCH_ERROR, NO_BOOK, NOT_HIT, INTERRUPTED, NOT_HIT_INTERRUPTED,
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

    inline void addBookList(Book *book)
    {
        bookList_ << book;
    }
    void setBrowser(const QString &str)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        setHtml(str);
        QApplication::restoreOverrideCursor();
    }

protected:
    QList <Book*> bookList_;
    void contextMenuEvent(QContextMenuEvent* event);
//#ifdef Q_WS_X11
    void mouseReleaseEvent(QMouseEvent* event);
//#endif

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
    Q_OBJECT
public:
    ReferencePopup(Book *pbook, const EB_Position &pos, QWidget *parent,
                   bool menu_flag=false);

    QString browserText(Book *pbook, const EB_Position &pos);
//private slots:
//    void resizeNoScroll();
    BookBrowser *bookBrowser() { return bookBrowser_; }

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
    Q_OBJECT
public:
    PageItems(const QString &ssheet);

    void composeHeader(const QString &ssheet);
    void composeError(const QString &anchor, const QString &text);
    QTreeWidgetItem* composeHLine(int num, const QString &anchor, const QString &title_l,
                      const QString &title_t, const QString &text);
    QTreeWidgetItem* composeHLine(int num, const QString &anchor, const QString &title,
                      const QString &text = QString());
    void composeTrail();
    QTreeWidgetItem* addHItem(int num, const QString &anchor, const QString &title);
    void addHtmlStr(const QString &html) { text_ += html; }
    void addTextStr(const QString &str) { text_ += str; }
    void expand(int level);
    QTreeWidgetItem* curItem() { return curItem_; }
    QTreeWidgetItem* item(int level) { return itemP_[level]; }

    QString text() { return text_; }
    QList <QTreeWidgetItem*> topItems() { return topItems_; }
    int textLength() { return textLength_; }

private:
    QString text_;
    QList <QTreeWidgetItem*> topItems_;
    QTreeWidgetItem* itemP_[7];
    QTreeWidgetItem* curItem_;
    int textLength_;

};

class PageWidget : public QSplitter
{
    Q_OBJECT
public:
    PageWidget(QWidget *parent, const SearchMethod&);

    virtual RET_SEARCH search(const Query&) = 0;

    void zoomIn();
    void zoomOut();
    BookBrowser* bookBrowser()
    {
        return bookBrowser_;
    }

    inline SearchMethod method() const
    {
        return method_;
    }

protected:
    BookBrowser *bookBrowser_;
    QTreeWidget *bookTree;
    SearchMethod method_;

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
    RET_SEARCH search(const Query&);

private:
    QString convSpecialChar(const QString&) const;
};

class MenuPage : public PageWidget
{
    Q_OBJECT
public:
    MenuPage(QWidget *parent, const SearchMethod&);
    RET_SEARCH search(const Query&);

private slots:
    void changePage(QTreeWidgetItem*, int column);

private:
    RET_SEARCH fullMenuPage();
    void selectMenuPage(int index);
    RET_SEARCH getMenus(EbMenu *eb, const EB_Position &pos, PageItems *items,
                  int count);
    QList <CandItem> topCands;
    int menuCount;
    bool checkMax;
};

class AllPage : public PageWidget
{
    Q_OBJECT
public:
    AllPage(QWidget *parent, const SearchMethod &);
    RET_SEARCH search(const Query&);

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
    Q_OBJECT
public:
    SearchPage(QWidget *parent, const SearchMethod&);
    RET_SEARCH search(const Query&);
};

class SearchWholePage : public PageWidget
{
    Q_OBJECT
public:
    SearchWholePage(QWidget *parent, const SearchMethod&);
    RET_SEARCH search(const Query&);
};
class WebPage : public QWebView
{
    Q_OBJECT
public:
    WebPage(QWidget *parent, const QString &url, const Query& query);
    WebPage(QWidget *parent, const QString &url);
    void zoomIn();
    void zoomOut();
    void setTabIndex(int index) { tabIndex_ = index; }
    void setTabBar(QTabBar *bar) { tabBar_ = bar; }
    SearchMethod method() { return method_; }
    bool loading() { return loading_; }

protected:
    void contextMenuEvent(QContextMenuEvent* event);

private slots:
    void progressStart();
    void progress(int pcount);
    void progressFinished(bool ok);
    void openLink(const QUrl &url);
    void openNewWin();
    void copyHoveredLink(const QString &link, const QString &title,
                         const QString &text);
    void changeFont(const QFont &font);
    void setPopupBrowser(bool);

signals:
    void linkRequested(const QString& prog);

private:
    QByteArray encString(const QString &url);
    QString setSearchString(const QString &url, const QByteArray &enc,
                            const QString &query);
    QString directionString(const QString &url);
    QString setDirectionString(const QString &url, const QString &dstr,
                               SearchDirection &direc);

    QString hoveredLink;
    bool loading_;
    bool popupBrowser_;
    SearchMethod method_;
    int progressCount_;
    int tabIndex_;
    QTabBar *tabBar_;
};

class BookView : public QTabWidget
{
    Q_OBJECT
public:
    BookView(QWidget *parent);
    RET_SEARCH newPage(QWidget *parent, const Query& query,
                       bool new_tab, bool popup_browser);
    RET_SEARCH newWebPage(const QString &name, const QString &url,
                           bool popup_browser);
    BookType pageType(int index);
    BookType currentPageType();
    SearchMethod pageMethod(int index);
    SearchMethod currentPageMethod();
    void showTabBar(int new_tab);
    void setPopupBrowser(bool popup);
    QWidget* pageWidget(int index);
    QWidget* currentPageWidget();

    void closeTab1(int index);
private:
    bool checkLoaded();
    void stopAllLoading();


signals:
    void searchRequested(SearchDirection, const QString&);
    void pasteRequested(const QString&);
    void processRequested(const QString&);
    void soundRequested(const QString&);
    void selectionRequested(const QString&);
    //void linkRequested(const QString& prog);
    void popupBrowserSet(bool);

    void fontChanged(const QFont&);
    void statusRequested(const QString&);
    void tabChanged(int tab_count);
    void allWebLoaded();

private slots:
    void showTabBarMenu(const QPoint &pnt);
    void closeTab();
    void closeAllTab();
    void stopSearch();
    void zoomIn();
    void zoomOut();
    void viewTabChanged(int index);
    void webViewFinished(bool);
};

class SearchPageBuilder : public QObject
{
    Q_OBJECT
public:
    SearchPageBuilder(BookBrowser *browser);
    RET_SEARCH search(const Query& query);
    QList <QTreeWidgetItem*> treeItems();
    QString text();
    int textLength();
    void expand();

signals:
    void statusRequested(const QString &str);

private:
    RET_SEARCH search1(const Query& query);

    int bookIndex;
    int itemIndex;
    PageItems items;
    BookBrowser *bookBrowser_;
};

#endif
