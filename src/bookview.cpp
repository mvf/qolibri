/**************************************************************************
*   Copyright (C) 2007 by BOP                                             *
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
#include "bookview.h"
#include "bookbrowser.h"
#include "configure.h"
#include "searchwholepage.h"
#include "allpage.h"
#include "menupage.h"
#include "infopage.h"
#include "searchpage.h"
#include "webview.h"

#include <QContextMenuEvent>
#include <QDir>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

extern bool stopFlag;

BookView::BookView(QWidget *parent, const QKeySequence &closeCurrentTabShortcut)
    : QTabWidget(parent)
    , closeCurrentTabAction(new QAction(tr("Close This Page"), this))
{
    setObjectName("bookview");
    setUsesScrollButtons(true);

    closeCurrentTabAction->setIcon(QIcon(":images/closetab.png"));
    closeCurrentTabAction->setToolTip(tr("Close page"));
    if (!closeCurrentTabShortcut.isEmpty())
        closeCurrentTabAction->setShortcut(closeCurrentTabShortcut);
    connect(closeCurrentTabAction, SIGNAL(triggered()), SLOT(closeTab()));

    QToolButton *close_button = new QToolButton(this);
    setCornerWidget(close_button, Qt::TopRightCorner);
    close_button->setDefaultAction(closeCurrentTabAction);
    close_button->setCursor(Qt::ArrowCursor);
    close_button->setAutoRaise(true);

    tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar(), SIGNAL(customContextMenuRequested(const QPoint&)),
            SLOT(showTabBarMenu(const QPoint&)));
}

RET_SEARCH BookView::newPage(QWidget *parent, const Query& query, bool newTab,
                             bool popup_browser, bool book_tree_visible)
{
    stopFlag = false;

    int dcount = 0;
    if (query.method.direction != MenuRead && query.method.direction != BookInfo) {
        foreach(Book *book, query.method.group->bookList()) {
            if (book->bookType() == BookWeb &&
                    book->checkState() == Qt::Checked) {
                dcount++;
            }
        }
    }

    PageWidget *page = 0;
    switch (query.method.direction) {
    case FullTextSearch:
        page = new SearchWholePage(this, query.method);
        break;
    case WholeRead:
        page = new AllPage(this, query.method);
        break;
    case MenuRead:
        page = new MenuPage(this, query.method);
        break;
    case BookInfo:
        page = new InfoPage(this, query.method);
        break;
    case ExactWordSearch:
    case ForwardSearch:
    case BackwardSearch:
    case KeywordSearch:
    case CrossSearch:
        page = new SearchPage(this, query.method);
        break;
    default:
        Q_ASSERT(0);
    }

    page->setBookTreeVisible(book_tree_visible);
    connect(page, SIGNAL(statusRequested(QString)),
            SIGNAL(statusRequested(QString)));
    connect(page->bookBrowser(), SIGNAL(statusRequested(QString)),
            SIGNAL(statusRequested(QString)));
    connect(page->bookBrowser(),
            SIGNAL(searchRequested(SearchDirection,QString)),
            SIGNAL(searchRequested(SearchDirection,QString)));
    connect(page->bookBrowser(), SIGNAL(pasteRequested(QString)),
            SIGNAL(pasteRequested(QString)));
    connect(page->bookBrowser(), SIGNAL(soundRequested(QString)),
            SIGNAL(soundRequested(QString)));
    connect(page->bookBrowser(), SIGNAL(videoRequested(QString)),
            SIGNAL(videoRequested(QString)));
    connect(page->bookBrowser(), SIGNAL(externalLinkRequested(QString)),
            SIGNAL(externalLinkRequested(QString)));
    connect(page, SIGNAL(selectionRequested(QString)),
            SIGNAL(selectionRequested(QString)));
    connect(page->bookBrowser(), SIGNAL(selectionRequested(QString)),
            SIGNAL(selectionRequested(QString)));
    connect(parent, SIGNAL(viewFontChanged(QFont)), page,
            SLOT(changeFont(QFont)));
    connect(parent, SIGNAL(bookTreeToggled(bool)), page,
            SLOT(setBookTreeVisible(bool)));

    RET_SEARCH retStatus = page->search(query);

    // Collapse the book tree by default.
    // This makes jumping between books easier.
    page->collapseBookTree();

    QWidget *focus_page = 0;
    BookView *view = this;
    if (retStatus == NORMAL) {
        dcount++;
    }
    if (dcount > 1) {
        view = new BookView(parent);
        QString tab_title = query.toLogicString();
        if (newTab || count() == 0) {
            addTab(view, QIcon(":/images/stabs.png"), tab_title);
        } else {
            int index = currentIndex();
            closeTab();
            insertTab(index, view, QIcon(":/images/stabs.png"),
                      tab_title);
        }
        connect(view, SIGNAL(currentChanged(int)), SLOT(viewTabChanged(int)));
        setCurrentWidget(view);
    } else {
        if (checkLoaded()) {
            emit allWebLoaded();
        }
    }
    if (retStatus != NORMAL) {
        delete page;
        if (dcount == 0) return retStatus;
        retStatus = NORMAL;
    } else {
        QString tab_title = query.toLogicString();
        if (dcount > 1 || newTab || view->count() == 0) {
            view->addTab(page, QIcon(":/images/sbook.png"), tab_title);
        } else {
            int index = view->currentIndex();
            view->closeTab();
            view->insertTab(index, page, QIcon(":/images/sbook.png"),
                            tab_title);
        }
        focus_page = (QWidget*)page;
        connect(this, SIGNAL(fontChanged(QFont)), page,
                SLOT(changeFont(QFont)));
        if (query.method.direction == MenuRead || query.method.direction == BookInfo) {
            view->setCurrentWidget(focus_page);
            return NORMAL;
        }
    }

    foreach(Book *book, query.method.group->bookList()) {
        if (book->bookType() == BookWeb &&
                book->checkState() == Qt::Checked) {
            WebView *wpage = new WebView(this);
            connect(wpage, SIGNAL(loadFinished(bool)),
                    SLOT(webViewFinished(bool)));
            connect(wpage, SIGNAL(externalLinkRequested(QString)),
                    SIGNAL(externalLinkRequested(QString)));
            connect(parent, SIGNAL(viewFontChanged(QFont)), wpage,
                    SLOT(changeFont(QFont)));
            connect(this, SIGNAL(popupBrowserSet(bool)), wpage,
                    SLOT(setPopupBrowser(bool)));
            int idx = view->addTab(wpage, QIcon(":/images/web2.png"),
                    book->name());
            wpage->setTabIndex(idx);
            wpage->setTabBar(view->tabBar());
            if (!focus_page) {
                focus_page = (QWidget *)wpage;
            }
            wpage->load(book->path(), query);
        }
    }
    emit popupBrowserSet(popup_browser);
    if (focus_page) {
        view->setCurrentWidget(focus_page);
    }
    emit tabChanged(count());

    return retStatus;
}

RET_SEARCH BookView::newWebPage(const QString &name, const QString &url,
                                bool popup_browser)
{
    WebView *wpage = new WebView(this);
    connect(wpage, SIGNAL(loadFinished(bool)), SLOT(webViewFinished(bool)));
    connect(wpage, SIGNAL(externalLinkRequested(QString)),
            SIGNAL(externalLinkRequested(QString)));
    connect(this, SIGNAL(popupBrowserSet(bool)), wpage,
            SLOT(setPopupBrowser(bool)));
    emit popupBrowserSet(popup_browser);
    int idx = addTab(wpage, QIcon(":/images/web2.png"), name);
    wpage->setTabIndex(idx);
    wpage->setTabBar(tabBar());
    setCurrentWidget(wpage);
    wpage->load(url);
    emit tabChanged(count());

    return NORMAL;
}

void BookView::showTabBarMenu(const QPoint& pnt)
{
    QTabBar *bar = tabBar();
    int index = 0;
    int tabnum = bar->count();
    for (; index < tabnum; index++) {
        if (bar->tabRect(index).contains(pnt))
            break;
    }
    QMenu menu("", bar);
    QAction *close_other_page = 0;
    QAction *close_left_page = 0;
    QAction *close_right_page = 0;
    QAction *close_all_page = 0;
    menu.addAction(closeCurrentTabAction);
    if (tabnum > 1)
        close_other_page = menu.addAction(tr("Close Other Pages"));
    if (index > 0)
        close_left_page = menu.addAction(tr("Close Left Pages"));
    if (index < (tabnum-1))
        close_right_page = menu.addAction(tr("Close Right Pages"));
    if (tabnum > 1)
        close_all_page = menu.addAction(tr("Close All Pages"));

    QAction *sel_act = menu.exec(bar->mapToGlobal(pnt));
    if (!sel_act)
        return;
    if (sel_act == close_other_page) {
        for (int i=tabnum-1; i>index; i--) {
            closeTab1(i);
        }
        for (int i=index-1; i>=0; i--) {
            closeTab1(i);
        }
    } else if (sel_act == close_left_page) {
        for (int i=index-1; i>=0; i--) {
            closeTab1(i);
        }
    } else if (sel_act == close_right_page) {
        for (int i=tabnum-1; i>index; i--) {
            closeTab1(i);
        }
    } else if (sel_act == close_all_page) {
        for (int i=tabnum-1; i>=0; i--) {
            closeTab1(i);
        }
    }
    emit tabChanged(count());
}

void BookView::closeTab1(int index)
{
    QWidget *wgt = widget(index);
    removeTab(index);
    //wgt->close();
    QTimer::singleShot(0, wgt, SLOT(deleteLater()));

}

void BookView::closeAllTab()
{
    QTabBar *bar = tabBar();
    for (int i=bar->count()-1;  i >= 0; i--) {
        closeTab1(i);
    }

    emit tabChanged(count());
}

void BookView::closeTab()
{
//    int idx = currentIndex();
//    QWidget *wgt = currentWidget();

//    removeTab(idx);
//    wgt->close();
//    delete wgt;

    closeTab1(currentIndex());

    emit tabChanged(count());
}
BookType BookView::pageType(int index)
{
    if (index < 0) {
        return BookUnset;
    }
    QString oname = widget(index)->objectName();
    if (oname == "dicpage"){
        return BookLocal;
    } else if (oname == "webpage") {
        return BookWeb;
    } else if (oname == "bookview") {
        BookView *v = (BookView*)widget(index);
        return v->currentPageType();
    } else {
        return BookUnset;
    }

}
BookType BookView::currentPageType()
{
    return pageType(currentIndex());
}

QWidget* BookView::pageWidget(int index)
{
    if (index < 0) {
        return 0;
    }
    QString oname = widget(index)->objectName();
    if (oname == "bookview") {
            BookView *v = (BookView*)widget(index);
            return v->pageWidget(v->currentIndex());
    } else {
        return widget(index);
    }

}
QWidget* BookView::currentPageWidget()
{
    return pageWidget(currentIndex());
}

SearchMethod BookView::pageMethod(int index)
{
    if (index < 0) {
        return SearchMethod();
    }
    QWidget *w = widget(index);
    QString oname = w->objectName();
    if (oname == "dicpage"){
        return ((PageWidget*)w)->method();
    } else if (oname == "webpage") {
        return ((WebView*)w)->method();
    } else if (oname == "bookview") {
        BookView *v = (BookView*)widget(index);
        return v->currentPageMethod();
    } else {
        return SearchMethod();
    }
}

SearchMethod BookView::currentPageMethod()
{
    return pageMethod(currentIndex());
}

void BookView::stopSearch()
{
    stopFlag = true;
    stopAllLoading();
}

void BookView::zoomIn()
{
    //int fsize = CONF->browserFont.pointSize();

    //CONF->browserFont.setPointSize(fsize + 1);
    //emit fontChanged(CONF->browserFont);
    QWidget *w = currentPageWidget();
    if (w->objectName() == "dicpage") {
        ((PageWidget*)w)->zoomIn();
    }else{
        ((WebView*)w)->zoomIn();
    }
}

void BookView::zoomOut()
{
    //int fsize = CONF->browserFont.pointSize();

    //if (fsize > 5) {
    //    CONF->browserFont.setPointSize(fsize - 1);
    //    emit fontChanged(CONF->browserFont);
    //}
    QWidget *w = currentPageWidget();
    if (w->objectName() == "dicpage") {
        ((PageWidget*)w)->zoomOut();
    }else{
        ((WebView*)w)->zoomOut();
    }
}

void BookView::showTabBar(int new_tab)
{
    int tcount = count();
    if (!new_tab) {
        if (tcount > 0) {
            if (tcount > 1){
                int index = currentIndex();
                for (int i=tcount-1; i>index; i--) {
                    closeTab1(i);
                }
                for (int i=index-1; i>=0; i--) {
                    closeTab1(i);
                }
            }
            tabBar()->hide();
        }
    } else {
        if (tcount > 0) {
            tabBar()->show();
        }
    }
}

void BookView::viewTabChanged(int index) {
    if (index < 0) {
        closeTab1(currentIndex());
        emit tabChanged(count());
    } else {
        emit currentChanged(currentIndex());
    }
}

void BookView::stopAllLoading()
{
    for(int i=0; i < count(); i++) {
        if (widget(i)->objectName() == "webpage") {
            WebView *w = (WebView*)widget(i);
            w->stop();
        } else if (widget(i)->objectName() == "bookview") {
            BookView *w = (BookView*)widget(i);
            w->stopAllLoading();
        }
    }
}

bool BookView::checkLoaded() {
    bool all_loaded = true;
    for(int i=0; i < count(); i++) {
        if (widget(i)->objectName() == "webpage") {
            WebView *w = (WebView*)widget(i);
            if (w->loading()) {
                all_loaded = false;
                break;
            }
        } else if (widget(i)->objectName() == "bookview") {
            BookView *w = (BookView*)widget(i);
            if (!w->checkLoaded()){
                all_loaded = false;
                break;
            }
        }
    }
    return all_loaded;
}

void BookView::webViewFinished(bool) {
    if (checkLoaded()) {
        emit allWebLoaded();
    }
    emit currentChanged(currentIndex());
}

void BookView::setPopupBrowser(bool popup)
{
    emit popupBrowserSet(popup);
}
