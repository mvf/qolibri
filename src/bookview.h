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

#include "book.h"
#include "method.h"

#include <QKeySequence>
#include <QTabWidget>

class QAction;

class BookView : public QTabWidget
{
    Q_OBJECT
public:
    explicit BookView(QWidget *parent, const QKeySequence &closeCurrentTabShortcut = QKeySequence());
    RET_SEARCH newPage(QWidget *parent, const Query& query,
                       bool new_tab, bool popup_browser, bool book_tree_visible);
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
    QAction *const closeCurrentTabAction;

signals:
    void searchRequested(SearchDirection, const QString&);
    void pasteRequested(const QString&);
    void processRequested(const QString &program, const QStringList &arguments);
    void soundRequested(const QString&);
    void selectionRequested(const QString&);
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

#endif // BOOKVIEW_H
