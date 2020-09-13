/***************************************************************************
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

#include "mainwindow.h"
#include "ebook.h"
#include "book.h"
#include "bookview.h"
#include "clipboardoptionswidget.h"
#include "toolbar.h"
#include "globaleventfilter.h"
#include "groupdock.h"
#include "booksetting.h"
#include "fontsetting.h"
#include "ssheetsetting.h"
#include "configure.h"
#include "optiondialog.h"
#include "webview.h"

#include <QApplication>
#include <QAudioDeviceInfo>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFontDialog>
#include <QLocale>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QSound>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QWebEngineHistory>

const char *Program = { "qolibri" };

#define CONNECT_BUSY(widget) \
    connect(this, SIGNAL(nowBusy(bool)), widget, SLOT(setDisabled(bool)))

MainWindow::MainWindow(Model *model_, const QString &s_text)
    : model(model_)
{
    QEb::initialize();

    bookView = new BookView(this, tr("Ctrl+W"));

    groupDock = new GroupDock(this, model);
    groupDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    CONNECT_BUSY(bookView);
    CONNECT_BUSY(groupDock);

    readSettings();


    setCentralWidget(bookView);

    connect(model, &Model::groupIndexChanged, this, &MainWindow::setTitle);
    setTitle();
    qApp->setWindowIcon(QIcon(":/images/title.png"));

    changeViewTabCount(0);

    addDockWidget(dockPosition, groupDock);
    if (!toggleDockAct->isChecked()) {
        groupDock->hide();
    }

    sound = NULL;
    timer = NULL;
    if (model->groupList[0]->bookList().count() == 0) {
        QTimer::singleShot(0, this, SLOT(setBooks()));
    } else if (!s_text.isEmpty()) {
        clientText << s_text;
        QTimer::singleShot(0, this, SLOT(checkNextSearch()));
    }

    connect(this, SIGNAL(searchFinished()), SLOT(checkNextSearch()));
    QTimer::singleShot(0, searchTextEdit, SLOT(setFocus()));

    clipboardsearchtimer = new QTimer;
    clipboardsearchtimer->setSingleShot(true);
    connect(clipboardsearchtimer, &QTimer::timeout, [this] {
        searchClientText(QApplication::clipboard()->text(QClipboard::Selection), watchClipboardRaiseWindow);
    });

    GlobalEventFilter *gef = new GlobalEventFilter();
    qApp->installEventFilter(gef);
    connect(gef, SIGNAL(focusSearch()), this, SLOT(focusSearch()));
    bookViewSlots();
    groupDockSlots();
}

void MainWindow::createActions()
{
    toggleWatchClipboardAct = new QAction(tr("Watch clipboard"), this);
    toggleWatchClipboardAct->setCheckable(true);
    toggleWatchClipboardAct->setIconVisibleInMenu(false);
    toggleWatchClipboardAct->setIcon(QIcon(":/images/paste.png"));
    connect(toggleWatchClipboardAct, &QAction::toggled, this, &MainWindow::connectClipboard);
}

void MainWindow::createMenus()
{
    QMenu *fmenu = menuBar()->addMenu(tr("&File"));

    enterAct = fmenu->addAction(QIcon(":/images/key_enter.png"), tr("&Search"),
                                this, SLOT(viewSearch()), tr("Ctrl+F"));
    CONNECT_BUSY(enterAct);
    openBookAct = fmenu->addAction(QIcon(":/images/bookopen.png"),
                                   tr("&Open book"),
                                   this, SLOT(viewMenu()), tr("Ctrl+O"));
    CONNECT_BUSY(openBookAct);

    viewAllAct = fmenu->addAction(QIcon(":/images/find.png"),
                                  tr("&View all data"),
                                  this, SLOT(viewFull()), tr("Ctrl+D"));
    CONNECT_BUSY(viewAllAct);

    stopAct = fmenu->addAction(QIcon(":/images/stop.png"), tr("&Cancel"),
                               bookView, SLOT(stopSearch()), Qt::Key_Escape);
    connect(stopAct, SIGNAL(triggered()), SLOT(stopSound()));
    stopAct->setEnabled(false);
    exitAct = fmenu->addAction(tr("E&xit"), this, SLOT(close()), tr("Ctrl+Q"));
    CONNECT_BUSY(exitAct);

    QMenu *emenu = menuBar()->addMenu(tr("&Edit"));
    clearEditAct = emenu->addAction(QIcon(":/images/clear.png"),
                                    tr("Clear search text"));

    vmenu = menuBar()->addMenu(tr("&View"));
    toggleDockAct = new QAction(QIcon(":/images/dock_mac.png"),
                                tr("Dock on/off"), this);
    toggleDockAct->setCheckable(true);
    toggleDockAct->setIconVisibleInMenu(false);
    connect(toggleDockAct, SIGNAL(triggered(bool)),
            SLOT(toggleDock(bool)));
    vmenu->addAction(toggleDockAct);
    toggleBarAct = vmenu->addAction(QIcon(":/images/find_l.png"),
                                    tr("Search/read book"),
                                    this, SLOT(toggleBar()));
    zoomInAct = vmenu->addAction(QIcon(":/images/zoomin.png"), tr("Zoom &in"),
                                 bookView, SLOT(zoomIn()));
    zoomInAct->setShortcuts(QList<QKeySequence>() << tr("Ctrl++") << tr("Ctrl+="));
    zoomOutAct = vmenu->addAction(QIcon(":/images/zoomout.png"), tr("Zoom &out"),
                                  bookView, SLOT(zoomOut()), tr("Ctrl+-"));
    goPrevAct = vmenu->addAction(QIcon(":/images/goprev.png"),
                                 tr("Go &Previous"), this, SLOT(goPrev()));
    goNextAct = vmenu->addAction(QIcon(":/images/gonext.png"),
                                 tr("Go &Next"), this, SLOT(goNext()));
    reloadAct = vmenu->addAction(QIcon(":/images/reload.png"),
                                 tr("&Reload"), this, SLOT(reload()));
    goPrevAct->setEnabled(false);
    goNextAct->setEnabled(false);
    reloadAct->setEnabled(false);


    QMenu *smenu = menuBar()->addMenu(tr("&Settings"));
    booksAct = smenu->addAction(QIcon(":/images/books.png"), tr("Book and group settings..."),
                                this, SLOT(setBooks()));
    CONNECT_BUSY(booksAct);
    configAct = smenu->addAction(QIcon(":/images/setting.png"),
                                 tr("Options..."), this, SLOT(setConfig()));
    CONNECT_BUSY(configAct);
    addMarkAct = smenu->addAction(QIcon(":/images/bookmark.png"),
                                  tr("Bookmark"),
                                  this, SLOT(addMark()));
    smenu->addAction(toggleWatchClipboardAct);

    CONNECT_BUSY(addMarkAct);
    toggleTabsAct = smenu->addAction(QIcon(":/images/tabs.png"),
                                     tr("Search result tabs on/off"),
                                     this, SLOT(toggleNewTab(bool)));
    toggleTabsAct->setCheckable(true);
    toggleTabsAct->setIconVisibleInMenu(false);
    CONNECT_BUSY(toggleTabsAct);
    toggleBrowserAct = smenu->addAction(QIcon(":/images/new_browser.png"),
                                     tr("Popup New Browser on/off"),
                                     this, SLOT(toggleNewBrowser(bool)));
    toggleBrowserAct->setCheckable(true);
    toggleBrowserAct->setIconVisibleInMenu(false);
    CONNECT_BUSY(toggleBrowserAct);
    fontAct = smenu->addAction(QIcon(":/images/font1.png"),
                               tr("Set browser font..."),
                               this, SLOT(setViewFont()));

    QMenu *ms = smenu->addMenu(QIcon(":/images/stylesheet.png"),
                               tr("Edit style sheet"));
    sSheetAct = ms->menuAction();
    connect(sSheetAct, SIGNAL(triggered(bool)), SLOT(setDictSheet()));
    ms->addAction(QIcon(":/images/stylesheet.png"),
                  tr("Dictionary style sheet..."), this, SLOT(setDictSheet()));
    ms->addAction(QIcon(":/images/stylesheet2.png"),
                  tr("Book style sheet..."), this, SLOT(setBookSheet()));
    ms->addAction(QIcon(":/images/stylesheet3.png"),
                  tr("Internal style sheet 1..."), this, SLOT(setStatusBarSheet()));
    optDirectionMenu = smenu->addMenu(QIcon(":/images/find_l.png"),
                                   tr("Search options"));
    addDirectionMenu(optDirectionMenu);
    QActionGroup *actg = new QActionGroup(this);
    foreach(QAction * a, optDirectionMenu->actions()) {
        actg->addAction(a);
        a->setCheckable(true);
    }
    connect(optDirectionMenu, SIGNAL(triggered(QAction*)),
            SLOT(changeOptDirection(QAction*)));

    smenu->addAction(QIcon(":/images/delete.png"), tr("&Clear cache"),
                     this, SLOT(clearCache()));

    QMenu *hmenu = menuBar()->addMenu(tr("&Help"));
    hmenu->addAction(QIcon(
        ":/qt-project.org/"
        "qmessagebox/images/qtlogo-64.png"), tr("About &QT..."), qApp, SLOT(aboutQt()));
    hmenu->addAction(QIcon(":/images/title.png"), tr("About q&olibri..."), this, SLOT(aboutQolibri()));
}

void MainWindow::createToolBars()
{
    setIconSize(QSize(16, 16));
    QToolBar *bar1 = addToolBar("Search/Read Book");
    bar1->setMovable(false);
    bar1->addAction(toggleBarAct);

    searchBar = addToolBar("Search");
    searchBar->setMovable(false);
    searchBar->addWidget(new DictionaryGroupComboBox(this, model));

    searchBar->addAction(clearEditAct);
    searchTextEdit = new QLineEdit(this);
    connect(searchTextEdit, SIGNAL(returnPressed()), SLOT(viewSearch()));
    connect(searchTextEdit, SIGNAL(textChanged(QString)),
            SLOT(changeSearchText(QString)));
    connect(clearEditAct, SIGNAL(triggered()), searchTextEdit, SLOT(clear()));
    connect(clearEditAct, SIGNAL(triggered()), searchTextEdit, SLOT(setFocus()));
    CONNECT_BUSY(searchTextEdit);
    searchBar->addWidget(searchTextEdit);
    searchBar->addAction(enterAct);
    searchBar->addAction(stopAct);

    bookBar = addToolBar("Read Book");
    bookBar->setMovable(false);
    bookBar->addWidget(new ReaderGroupComboBox(this, model));
    bookBar->addWidget(new ReaderBookComboBox(this, model));
    bookBar->addAction(openBookAct);
    bookBar->hide();

    methodBar = addToolBar(tr("Search methods"));
    methodBar->setMovable(false);

    methodBar->addWidget(new DirectionComboBox(this, model));
    methodBar->addWidget(new LogicComboBox(this, model));

    QToolBar *bar2 = addToolBar("Options 1");

    bar2->setMovable(false);


    bar2->addAction(addMarkAct);
    bar2->addAction(toggleWatchClipboardAct);
    bar2->addAction(toggleDockAct);

    webBar = addToolBar("Web");
    webBar->setMovable(false);
    webBar->addSeparator();

    webBar->addAction(goPrevAct);
    webBar->addAction(goNextAct);
    webBar->addAction(reloadAct);
    webBar->hide();

#if 0
    methodBar->addSeparator();
    methodBar->addAction(booksAct);
    methodBar->addAction(fontAct);
    methodBar->addAction(sSheetAct);
    methodBar->addAction(toggleTabsAct);
    methodBar->addAction(toggleBrowserAct);
#endif

    toggleMethodBarAct = methodBar->toggleViewAction();
    toggleMethodBarAct->setText(tr("Option bar on/off"));
    toggleMethodBarAct->setIcon(QIcon(":/images/configure.png"));
    toggleMethodBarAct->setIconVisibleInMenu(false);
    vmenu->addAction(toggleMethodBarAct);

    toggleBookTreeAct = new QAction(tr("&Book Tree Panel"));
    toggleBookTreeAct->setCheckable(true);
    connect(toggleBookTreeAct, SIGNAL(toggled(bool)), SIGNAL(bookTreeToggled(bool)));
    vmenu->addAction(toggleBookTreeAct);
}

void MainWindow::createStatusBar()
{
    QStatusBar *bar = statusBar();

    bar->setStyleSheet(CONF->statusBarSheet);

    processLabel = new QLabel(this);
    processLabel->setObjectName("process");
    bar->addWidget(processLabel, 1);

    optSearchButton = new QPushButton(this);
    optSearchButton->setObjectName("selection");
    optSearchButton->setFlat(true);
    connect(optSearchButton, SIGNAL(clicked()), SLOT(doSearch()));
    CONNECT_BUSY(optSearchButton);
    bar->addWidget(optSearchButton);
}

void MainWindow::readSettings()
{
    QSettings settings(CONF->settingOrg, "EpwingViewer");
    QSize size = settings.value("size", QSize(800, 600)).toSize();

    resize(size);
    bool dock = settings.value("dock", 1).toBool();
    toggleDockAct->setChecked(dock);
    dockPosition = (Qt::DockWidgetArea) settings.value("dockarea", Qt::RightDockWidgetArea).toInt();

    bool search_bar = settings.value("search_bar", 1).toBool();
    if (!search_bar) {
        searchBar->hide();
        bookBar->show();
        model->setBookMode(ModeBook);
        toggleBarAct->setIcon(QIcon(":images/book.png"));
    } else {
        model->setBookMode(ModeDictionary);
    }
    bool method_bar = settings.value("method_bar", 1).toBool();
    if (!method_bar) {
        methodBar->hide();
    }
    bool book_tree = settings.value("book_tree", true).toBool();
    toggleBookTreeAct->setChecked(book_tree);
    bool tab = settings.value("newtab", 1).toBool();
    toggleTabsAct->setChecked(tab);
    bool browser = settings.value("newbrowser", 1).toBool();
    toggleBrowserAct->setChecked(browser);
    int searchsel = settings.value("searchsel", (int)WikipediaSearch).toInt();
    optDirection = (SearchDirection)searchsel;
    foreach(QAction * act, optDirectionMenu->actions()) {
        if (act->data().toInt() == searchsel) {
            //act->setChecked(true);
            act->trigger();
        }
    }
    toggleWatchClipboardAct->setChecked(settings.value("watch_clipboard", false).toBool());
    watchClipboardMode = QClipboard::Mode(settings.value("watch_clipboard_mode", QClipboard::Clipboard).toInt());
    watchClipboardSelectionDelay = settings.value("watch_clipboard_selection_delay", 300).toInt();
    watchClipboardRaiseWindow = settings.value("watch_clipboard_raise_window", true).toBool();

    QSettings hist(CONF->settingOrg, "EpwingHistory");
    int hcnt = hist.beginReadArray("History");
    for (int i = 0; i < hcnt && i < CONF->historyMax; i++) {
        hist.setArrayIndex(i);
        QString str = hist.value("name").toString();
        SearchMethod m = readMethodSetting(hist);
        if (!m.group || !m.book) continue;

        groupDock->addHistory(str, m, CONF->historyMax);
    }
    hist.endArray();

    QSettings mark(CONF->settingOrg, "EpwingBookmark");
    int mcnt = mark.beginReadArray("Bookmark");
    for (int i = 0; i < mcnt; i++) {
        mark.setArrayIndex(i);
        QString str = mark.value("name").toString();
        QString u = mark.value("url").toString();
        //qDebug() << str << u;
        if (u.isEmpty()) {
            SearchMethod method = readMethodSetting(mark);
            if (!method.group || !method.book) continue;
            groupDock->addMark(str, method);
        } else {
            groupDock->addMark(str, u);
        }

    }
    mark.endArray();
}

void MainWindow::writeSettings()
{
    QSettings settings(CONF->settingOrg, "EpwingViewer");

    settings.setValue("size", size());
    settings.setValue("dock", toggleDockAct->isChecked());
    settings.setValue("dockarea", dockWidgetArea(groupDock));
    settings.setValue("search_bar", searchBar->isVisible());
    settings.setValue("method_bar", toggleMethodBarAct->isChecked());
    settings.setValue("book_tree", toggleBookTreeAct->isChecked());

    settings.setValue("newtab", toggleTabsAct->isChecked());
    settings.setValue("newbrowser", toggleBrowserAct->isChecked());
    settings.setValue("searchsel", optDirection);
    settings.setValue("watch_clipboard", toggleWatchClipboardAct->isChecked());
    settings.setValue("watch_clipboard_mode", watchClipboardMode);
    settings.setValue("watch_clipboard_selection_delay", watchClipboardSelectionDelay);
    settings.setValue("watch_clipboard_raise_window", watchClipboardRaiseWindow);

    QSettings history(CONF->settingOrg, "EpwingHistory");
    history.beginWriteArray("History");
    QListWidget *hlist = groupDock->historyListWidget();
    for (int i = 0; i < hlist->count(); i++) {
        history.setArrayIndex(i);
        SearchItem *item =
            (SearchItem *)hlist->item(hlist->count() - i - 1);
        history.setValue("name", item->searchStr());
        writeMethodSetting(item->method(), &history);
    }
    history.endArray();

    QSettings mark(CONF->settingOrg, "EpwingBookmark");
    mark.beginWriteArray("Bookmark");
    QListWidget *mlist = groupDock->markListWidget();
    for (int i = 0; i < mlist->count(); i++) {
        mark.setArrayIndex(i);
        SearchItem *item = (SearchItem *)mlist->item(mlist->count() - i - 1);
        mark.setValue("name", item->searchStr());
        if (item->url().isEmpty()) {
            writeMethodSetting(item->method(), &mark);
        } else {
            mark.setValue("url", item->url());
        }
    }
    mark.endArray();
}

void MainWindow::setTitle()
{
    QString title = Program;

    if (model->method.group) {
        if (model->bookMode == ModeDictionary) {
            title += " - " + model->method.group->name();
        } else {
            title += " - " + model->method.groupReader->name();
            if (model->method.bookReader) {
                title += " - " + model->method.bookReader->name();
            }
        }
    }
    setWindowTitle(title);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (isBusy()) {
        clientText.clear();
        connect(this, SIGNAL(searchFinished()), SLOT(close()));
        stopAct->trigger();
        event->ignore();
        return;
    }

    writeSettings();

    groupDock->close();

    CONF->save();

    QEb::finalize();

    event->accept();
}

Group *MainWindow::groupFromName(const QString &name)
{
    if (model->groupList.count() == 0) {
        return NULL;
    }
    foreach(Group * g, model->groupList) {
        if (g->name() == name) {
            return g;
        }
    }

    return model->groupList[0];
}

Book *MainWindow::bookFromName(Group *grp, const QString &name)
{
    if (grp->bookList().count() == 0) {
        return NULL;
    }
    foreach(Book * b, grp->bookList()) {
        if (b->name() == name) {
            return b;
        }
    }
    return grp->bookList().at(0);
}

SearchMethod MainWindow::readMethodSetting(const QSettings &set)
{
    SearchMethod m;

    m.group = groupFromName(set.value("group").toString());
    m.groupReader = groupFromName(set.value("group_reader").toString());
    m.book = bookFromName(m.group, set.value("book").toString());
    m.bookReader = bookFromName(m.groupReader,
                                set.value("book_reader").toString());
    m.direction = (SearchDirection)set.value("search_method",
                                             ExactWordSearch).toInt();
    m.logic  = (NarrowingLogic)set.value("narrowing_logic",
                                         LogicAND).toInt();
    m.limitBook = set.value("limit_book", 100).toInt();
    m.limitTotal = set.value("limit_total", 1000).toInt();

    return m;
}

void MainWindow::writeMethodSetting(const SearchMethod &m, QSettings *set)
{
    set->setValue("search_method", m.direction);
    set->setValue("narrowing_logic", m.logic);
    set->setValue("limit_book", m.limitBook);
    set->setValue("limit_total", m.limitTotal);
    if (m.group) {
        set->setValue("group", m.group->name());
    }
    if (m.groupReader) {
        set->setValue("group_reader", m.groupReader->name());
    }
    if (m.book) {
        set->setValue("book", m.book->name());
    }
    if (m.bookReader) {
        set->setValue("book_reader", m.bookReader->name());
    }
}

void MainWindow::showStatus(const QString &str)
{
    QString msg;

    if (!isBusy()) {
        if (model->groupList.count() > 0 && model->groupList[0]->bookList().count() == 0) {
            if (!str.isEmpty()) msg = ": ";
            msg += tr("No search book");
        } else if (model->bookMode == ModeDictionary &&
                   searchTextEdit->text().isEmpty()) {
            //msg += tr(" Input search text");
        } else {
            if (!str.isEmpty()) msg = ": ";
            msg += tr("ready");
        }
    } else {
        if (!str.isEmpty()) msg = ": ";
        msg += tr("retrieving . . .");
    }
    processLabel->setText(str + msg);
}

void MainWindow::showTabInfo(int index)
{
    if (index < 0) {
        return;
    }
    BookType t = bookView->pageType(index);
    if (t == BookLocal) {
        SearchMethod m = bookView->pageMethod(index);
        QString s = bookView->tabText(index);
        s.replace("&&", "&");
        SearchItem i(s, m);
        webBar->hide();
        goPrevAct->setEnabled(false);
        goNextAct->setEnabled(false);
        reloadAct->setEnabled(false);
        showStatus(i.text());
    } else if (t == BookWeb) {
        WebView *w = (WebView*)(bookView->pageWidget(index));
        showStatus(w->url().toString());
        webBar->show();
        reloadAct->setEnabled(true);
        if (w->history()->canGoBack()) {
            goPrevAct->setEnabled(true);
        } else {
            goPrevAct->setEnabled(false);
        }
        if (w->history()->canGoForward()) {
            goNextAct->setEnabled(true);
        } else {
            goNextAct->setEnabled(false);
        }
    } else {
        webBar->hide();
        //showStatus("");
    }

}

void MainWindow::toggleDock(bool check)
{
    if (check) {
        groupDock->show();
    } else {
        groupDock->hide();
    }
}

void MainWindow::toggleNewTab(bool check)
{
    bookView->showTabBar(check);
}

void MainWindow::toggleNewBrowser(bool check)
{
    //qDebug() << "toggleNewBrowser";
    bookView->setPopupBrowser(check);
}

void MainWindow::toggleBar()
{
    if (searchBar->isVisible()) {
        searchBar->hide();
        bookBar->show();
        model->setBookMode(ModeBook);
        toggleBarAct->setIcon(QIcon(":images/book.png"));
    } else {
        searchBar->show();
        bookBar->hide();
        model->setBookMode(ModeDictionary);
        toggleBarAct->setIcon(QIcon(":images/find_l.png"));
    }
}

void MainWindow::changeOptSearchButtonText(const QString &str)
{
    QStringList list = str.split(QRegExp("\\W+"), QString::SkipEmptyParts);

    if (list.count() > 0) {
        optSearchButton->setText(list[0]);
    }
}

void MainWindow::changeOptDirection(QAction *act)
{
    optDirection = (SearchDirection)act->data().toInt();
}

void MainWindow::viewInfo(Book *book)
{
    //qDebug() << "MainWindow::viewInfo(Book *dic) Name=" << book->name();
    //bookView->newInfoPage(book, toggleTabsAct->isChecked());
    if (book->bookType() == BookLocal) {
        SearchMethod m = model->method;
        m.book = book;
        m.direction = BookInfo;
        viewSearch(m.book->name(), m);
    } else {
        QString u = book->path();
        int index = u.indexOf('?');
        if (index < 0) {
            index = u.indexOf('{');
            if (index < 0) {
                index = u.length();
            }
        }
        QString s = u.left(index);
        viewWeb(book->name(), s);
    }
}

void MainWindow::setBookFont(Book *book)
{
    QString cache_path = loadAllExternalFont(book);
    FontSetting dlg(book, this);
    if (dlg.setupTreeWidget(cache_path) < 1) {
        QMessageBox::information(this, "Font Setting", tr("No external font"));
        showStatus(tr("No external font"));
        return;
    }
    int w = dlg.size().width();
    int h = size().height() - 50;

    dlg.resize(QSize(w, h));
    bool del_flag = true;
    if (dlg.exec() == QDialog::Accepted) {
        QHash <QString, QString> *flist =  dlg.newAlternateFontList();
        foreach(Group * g, model->groupList) {
            foreach(Book * b, g->bookList()) {
                if (b->path() != book->path() ||
                    b->bookNo() != book->bookNo()) continue;

                if (del_flag) {
                    b->removeFontList();
                    del_flag = false;
                }
                b->setFontList(flist);
            }
        }
        book->saveAlterFont();
    }
    showStatus();
}

void MainWindow::setBooks()
{
//    qDebug() << "MainWindow::setBook() " << currentGroup->name;
    BookSetting dlg(model, this);

    if (dlg.exec() == QDialog::Accepted) {
        QString msg;
        msg = "Group=" + QString::number(model->groupList.count()) +
              ", Book=" + QString::number(model->groupList[0]->bookList().count());

        showStatus(msg);
    } else {
        showStatus("Group Setting Cancelled");
    }
}

void MainWindow::viewWeb(const QString &name, const QString &url)
{
    bool pbrowser = toggleBrowserAct->isChecked();
    RET_SEARCH ret = bookView->newWebPage(name, url, pbrowser);
    if (ret != NORMAL) {
        showStatus("Error: " + url);
    } else {
        showStatus(url);
    }
}

void MainWindow::viewSearch()
{
    QString str = searchTextEdit->text();

//    if (str.isEmpty()) {
//        qDebug() << "MassageBox( Input Search Text)";
//        return;
//    }
    viewSearch(str, model->method);
    focusSearch();
}

void MainWindow::focusSearch()
{
    searchTextEdit->setFocus();
    searchTextEdit->selectAll();
}

void MainWindow::viewMenu()
{
    SearchMethod m = model->method;

    m.direction = MenuRead;
    if (model->bookMode == ModeDictionary) {
        m.groupReader = model->method.group;
        m.bookReader = model->method.book;
    }
    if (m.groupReader == 0){
        qWarning() << "Error: No Group";
        return;
    }
    if (m.bookReader == 0){
        QString msg = "Select book.";
        QMessageBox::warning(this, Program, msg);
        showStatus(msg);
        return;
    }

    viewSearch(m.bookReader->name(), m);

    if (model->bookMode == ModeDictionary) {
        focusSearch();
    }
}

void MainWindow::viewFull()
{
    SearchMethod m = model->method;

    m.direction = WholeRead;
    if (model->bookMode == ModeDictionary) {
        m.groupReader = model->method.group;
        m.bookReader = model->method.book;
    }
    viewSearch(m.bookReader->name(), m);
    focusSearch();
}

void MainWindow::viewSearch(const QString &queryStr, const SearchMethod &mthd)
{
    if (queryStr.isEmpty())
        return;

    QElapsedTimer pTime;

    pTime.start();

    optSearchButton->setText(queryStr);
    emit nowBusy(true);
    stopAct->setEnabled(true);

    bool ntab = toggleTabsAct->isChecked();
    bool pbrowser = toggleBrowserAct->isChecked();

    Query query(queryStr, mthd);
    RET_SEARCH ret = bookView->newPage(this, query, ntab, pbrowser, toggleBookTreeAct->isChecked());
    groupDock->addHistory(query.toLogicString(), mthd, CONF->historyMax);

    emit nowBusy(false);
    //stopAct->setEnabled(false);
    QString msg;
    QString msg_b;
    if (ret == NORMAL) {
        msg = "OK ";
    } else {
        msg = "<b><font color=#993333>";
        switch (ret) {
            case NOT_HIT:
                msg += tr("No results for \"%1\".").arg(queryStr);
                break;
            case NO_MENU:
                msg_b = tr("Not have menu data.");
                msg += msg_b;
                break;
            case NO_BOOK:
                msg_b = tr("Cannot open the book.");
                msg += msg_b;
                break;
            case NOT_HIT_INTERRUPTED:
            case INTERRUPTED:
                msg += tr("Interrupted.");
                break;
            case LIMIT_CHAR:
                msg += tr("Stopped (Character size limit[") +
                       QString::number(CONF->limitBrowserChar) + "])";
                break;
            case LIMIT_TOTAL:
                msg += tr("Stopped (Item limit[") +
                       QString::number(mthd.limitTotal) + "])";
                break;
            case LIMIT_MENU:
                msg += tr("Stopped (Menu limit[") +
                       QString::number(CONF->limitMenuHit) + "])";
                break;
            default:
                msg += "Error";
                qWarning() << "Unrecognized Error Code" << ret;
                break;
        }
        msg += "</font></b> ";
    }
    if (CONF->beepSound) {
        qApp->beep();
    }
    int etime = pTime.elapsed();
    msg += "(" + QString::number((double)etime / 1000.0, 'f', 1) + " seconds)";
    showStatus(msg);
    if (!msg_b.isEmpty()) {
        QMessageBox::warning(this, Program, msg_b);
    }
    emit searchFinished();
}
void MainWindow::pasteMethod(const QString &str, const SearchMethod &m)
{
    model->setMethod(m);
    if (m.direction == WholeRead || m.direction == MenuRead ||
        m.direction == BookInfo ) {
    } else {
        QStringList list = str.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        searchTextEdit->setText(list.join(" "));
    }
}

void MainWindow::doSearch()
{
    QString str = optSearchButton->text();

    if (str.isEmpty()) {
        return;
    }
    if (optDirection >= ExactWordSearch && optDirection <= MenuRead) {
        viewSearch(optDirection, str);
    } else if (optDirection >= GoogleSearch && optDirection <= Option1Search) {
        QString addr;
        if (optDirection == GoogleSearch) {
            addr = CONF->googleUrl;
        } else if (optDirection == WikipediaSearch) {
            addr = CONF->wikipediaUrl;
        } else {
            addr = CONF->userDefUrl;
        }
        foreach(const char c, str.toUtf8()) {
            addr += "%" + QString::number((ushort)((uchar)c), 16);
        }
        openExternalLink(addr);
    }
}

void MainWindow::viewSearch(SearchDirection direction, const QString &text)
{
    bool ntab = toggleTabsAct->isChecked();

    toggleTabsAct->setChecked(true);

    SearchMethod m = model->method;
    m.direction = direction;
    viewSearch(text, m);

    toggleTabsAct->setChecked(ntab);
}

void MainWindow::pasteSearchText(const QString &text)
{
    if (!searchBar->isVisible()) {
        toggleBar();
    }
    searchTextEdit->setText(text);
}

void MainWindow::startProcess(const QString &program, const QStringList &arguments)
{
    QProcess *const proc = new QProcess();
    connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), proc, SLOT(deleteLater()));
    connect(proc, SIGNAL(error(QProcess::ProcessError)), SLOT(processError()));
    proc->start(program, arguments);
    showStatus(tr("Execute: %1 %2").arg(program, arguments.join(' ')));
}

void MainWindow::processError()
{
    if (QProcess *const proc = qobject_cast<QProcess *>(sender())) {
        QMessageBox::warning(this, Program,
                             tr("Error running external program:\n\n%1 %2\n\n%3").arg(
                                 proc->program(),
                                 proc->arguments().join(' '),
                                 proc->errorString()));
    }
}

void MainWindow::playSound(const QString &fileName)
{
    if (!CONF->waveProcess.isEmpty()) {
        startProcess(CONF->waveProcess, QStringList(fileName));
    } else if (!QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).isEmpty()) {
        stopSound();
        checkSound();
        stopAct->setEnabled(true);
        sound = new QSound(fileName, this);
        sound->play();
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), SLOT(checkSound()));
        timer->start(1000);
    } else {
        qWarning() << "Can't play sound" << CONF->waveProcess << fileName;
        showStatus("Can't play sound");
    }
}

void MainWindow::playVideo(const QString &fileName)
{
    if (!CONF->mpegProcess.isEmpty()) {
        startProcess(CONF->mpegProcess, QStringList(fileName));
        return;
    }

    if (QDesktopServices::openUrl(QUrl::fromLocalFile(fileName)))
        return;

    QMessageBox::warning(this, Program,
                         tr("Failed to start system default application to play video file:\n\n"
                            "%1\n\n"
                            "Please check the system's default application for this file extension "
                            "or set a custom video player in the qolibri options.").arg(fileName));
}

void MainWindow::openExternalLink(const QString &url)
{
    if (!CONF->browserProcess.isEmpty()) {
        startProcess(CONF->browserProcess, QStringList(url));
        return;
    }

    if (QDesktopServices::openUrl(url))
        return;

    QMessageBox::warning(this, Program,
                         tr("Failed to start system default application to open URL:\n\n"
                            "%1\n\n"
                            "Please check the system's default browser settings "
                            "or set a custom browser in the qolibri options.").arg(url));
}

void MainWindow::checkSound()
{
    if (sound) {
        if (sound->isFinished()) {
            delete sound;
            sound = NULL;
            stopAct->setEnabled(false);
        }
    }
    if (!sound && timer) {
        timer->stop();
        delete timer;
        timer = NULL;
    }
}

void MainWindow::stopSound()
{
    if (sound) {
        sound->stop();
    }
}

void MainWindow::addMark()
{
    if (bookView->currentPageType() == BookLocal) {
        SearchMethod m = bookView->currentPageMethod();
        QString s = bookView->tabText(bookView->currentIndex());
        s.replace("&&", "&");
        groupDock->addMark(s, m);
    } else {
        WebView *w = (WebView*)(bookView->currentPageWidget());
        groupDock->addMark(w->title(), w->url().toString());

//        QTabWidget *v = bookView;
//        if (bookView->currentWidget()->objectName() == "bookview") {
//            v = (QTabWidget*)bookView->currentWidget();
//            s += " (" + v->tabText(v->currentIndex()) + ")";
//        } else {
//            s = v->tabText(v->currentIndex());
//        }
//        QString s = w->title();
//        QString u = w->url().toString();
//        groupDock->addMark(s, u);
    }

    toggleDockAct->setChecked(true);
    groupDock->show();
    groupDock->changeTabToMark();
}

void MainWindow::setDictSheet()
{
    SSheetSetting dlg(CONF->dictSheet, CONF->defDictStyle, true, this);
    QSize sz = size() - QSize(50, 50);

    dlg.resize(sz);
    if (dlg.exec() == QDialog::Accepted) {
        CONF->dictSheet = dlg.text();
        showStatus("Stylesheet changed");
    } else {
        showStatus("Stylesheet unchanged");
    }
}

void MainWindow::setBookSheet()
{
    SSheetSetting dlg(CONF->bookSheet, CONF->defBookStyle, false, this);
    QSize sz = size() - QSize(50, 50);

    dlg.resize(sz);
    if (dlg.exec() == QDialog::Accepted) {
        CONF->bookSheet = dlg.text();
        showStatus("Stylesheet changed");
    } else {
        showStatus("Stylesheet unchanged");
    }
}

void MainWindow::setStatusBarSheet()
{
    SSheetOptSetting dlg(CONF->statusBarSheet, CONF->defStatusbarStyle, this);

    dlg.resize(QSize(450, 550));
    if (dlg.exec() == QDialog::Accepted) {
        CONF->statusBarSheet = dlg.text();
        statusBar()->setStyleSheet(CONF->statusBarSheet);
        showStatus("Stylesheet changed");
    } else {
        showStatus("Stylesheet unchanged");
    }
}

void MainWindow::setViewFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, CONF->browserFont, this,
                                      tr("Set browser font"));

    if (ok) {
        CONF->browserFont = font;
        QString str = "font-family=" + font.family() +
                      " font-size=" + QString::number(font.pointSize());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        emit viewFontChanged(font);
        QApplication::restoreOverrideCursor();
        showStatus(str);
        //} else {
        //showStatus("Cancelled Font Dialog");
    }
}

void MainWindow::clearCache()
{
    int ret = QMessageBox::question(this, Program,
                                    tr("Are you sure you want to remove all cache data?\n\"")
                                    +
                                    EbCache::cachePath + "/*\"",
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        QDir dir(EbCache::cachePath);
        dir.removeRecursively();
    }
}

void MainWindow::setConfig()
{
    OptionDialog dlg(this);
    auto *const cow = ClipboardOptionsWidget::maybeCreate(this);
    if (cow) {
        cow->setMode(watchClipboardMode);
        cow->setRaiseWindowEnabled(watchClipboardRaiseWindow);
        cow->setSelectionDelay(watchClipboardSelectionDelay);
        dlg.insertTab(1, cow, tr("Clipboard"));
    }
    if (dlg.exec() == QDialog::Accepted && cow) {
        watchClipboardMode = cow->mode();
        watchClipboardRaiseWindow = cow->isRaiseWindowEnabled();
        watchClipboardSelectionDelay = cow->selectionDelay();
    }
}

void MainWindow::changeSearchText(const QString&)
{
    showStatus();
}

void MainWindow::changeViewTabCount(int tab_count)
{
    if (tab_count == 0 ) {
        showStatus();
        addMarkAct->setEnabled(false);
        zoomInAct->setEnabled(false);
        zoomOutAct->setEnabled(false);
    } else {
        bookView->showTabBar(toggleTabsAct->isChecked());
        addMarkAct->setEnabled(true);
        zoomInAct->setEnabled(true);
        zoomOutAct->setEnabled(true);
    }
}

QString MainWindow::loadAllExternalFont(Book *pbook)
{
    EbAll eb(HookFont);
    if (eb.initBook(pbook->path(), pbook->bookNo()) < 0) {
        QMessageBox::warning(this, Program, tr("Cannot open the book.") );
    }
    eb.initHook(16, NULL);
    QFile loadf(eb.ebCache.fontCachePath + "/loaded");
    if (loadf.exists()) {
        return eb.ebCache.fontCachePath;
    }
    emit nowBusy(true);

    if (eb.isHaveNarrowFont())
    {
        int i = eb.narrowFontStart();
        while (i >= 0)
        {
            QByteArray fname = 'n' + eb.numToBStr(i, 16) + ".png";
            QByteArray bitmap = eb.narrowFontCharacterBitmap(i);
            if (bitmap.isEmpty())
            {
                qDebug() << "Font Extract Error" << fname;
                continue;
            }

            QByteArray cnv = eb.narrowBitmapToPng(bitmap);
            if (cnv.isEmpty())
            {
                qDebug() << "Font Conversion Error" << fname;
                continue;
            }

            if(!eb.makeBinaryFile(eb.ebCache.fontCachePath + '/' + fname, cnv)) {
                qDebug() << "Font Write Error" << fname;
                continue;
            }
            eb.ebCache.fontCacheList << fname;

            if (i == eb.narrowFontEnd())
                break;
            i = eb.forwardNarrowFontCharacter(1, i);
        }
    }

    if (eb.isHaveWideFont())
    {
        int i = eb.wideFontStart();
        while (i >= 0)
        {
            QByteArray fname = 'w' + eb.numToBStr(i, 16) + ".png";
            QByteArray bitmap = eb.wideFontCharacterBitmap(i);
            if (bitmap.isEmpty())
            {
                qDebug() << "Font Extract Error" << fname;
                continue;
            }

            QByteArray cnv = eb.wideBitmapToPng(bitmap);
            if (cnv.isEmpty())
            {
                qDebug() << "Font Conversion Error" << fname;
                continue;
            }

            if(!eb.makeBinaryFile(eb.ebCache.fontCachePath + '/' + fname, cnv)) {
                qDebug() << "Font Write Error" << fname;
                continue;
            }
            eb.ebCache.fontCacheList << fname;

            if (i == eb.wideFontEnd())
                break;
            i = eb.forwardWideFontCharacter(1, i);
        }
    }

    loadf.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&loadf);
    out << "All font loaded";
    emit nowBusy(false);

    return eb.ebCache.fontCachePath;
}

void MainWindow::checkNextSearch()
{
    if (!clientText.count())
        return;
    QString str = clientText[0];
    clientText.removeAt(0);

    searchClientText(str);

}

void MainWindow::searchClientText(const QString &str, bool raiseWindow)
{
    if (raiseWindow)
        raise();
    if (str.isEmpty()) {
        return;
    }
    if (isBusy()) {
        clientText << str;
        return;
    }
    //showStatus("written");
    pasteSearchText(str);
    viewSearch();
}

void MainWindow::connectClipboard(bool enable)
{
    auto *const clipboard = QApplication::clipboard();
    if (enable) {
        connect(clipboard, &QClipboard::changed, [this, clipboard](QClipboard::Mode mode) {
            if (mode != watchClipboardMode)
                return;

            if (QApplication::activeModalWidget())
                return;

            if (mode == QClipboard::Selection && watchClipboardSelectionDelay)
                clipboardsearchtimer->start(watchClipboardSelectionDelay);
            else
                searchClientText(clipboard->text(mode), watchClipboardRaiseWindow);
        });
    } else
        clipboard->disconnect(SIGNAL(changed(QClipboard::Mode)));
}

void MainWindow::aboutQolibri()
{
    const QString gitCommitDate(QLocale().toString(QDateTime::fromString(QOLIBRI_GIT_COMMIT_DATE, Qt::ISODate), QLocale::ShortFormat));
    const QUrl website(QOLIBRI_WEBSITE, QUrl::StrictMode);
    QString msg(tr("<h2>qolibri</h2>"
                   "<h3>EPWING Dictionary/Book Viewer</h3>"
                   "<h4>Version %1</h4>").arg(QOLIBRI_VERSION_STR));

    if (!gitCommitDate.isEmpty())
        msg += tr("<p>Commit date: %1</p>").arg(gitCommitDate);
   
    if (website.isValid())
        msg += QString("<p><a href='%1'>%1</a></p>").arg(website.toString());
    
    msg += tr("<p>Based on <a href='%1'>%1</a></p>").arg("http://qolibri.sourceforge.jp");
    msg += tr("<p>Compiled against Qt version %1</p>").arg(QT_VERSION_STR);

    QMessageBox::about(this, tr("About qolibri"), msg);
}

void MainWindow::goPrev()
{
    WebView *w = (WebView*)bookView->currentPageWidget();
    w->back();
    if (w->history()->canGoBack()) {
        goPrevAct->setEnabled(true);
    } else {
        goPrevAct->setEnabled(false);
    }
    if (w->history()->canGoForward()) {
        goNextAct->setEnabled(true);
    } else {
        goNextAct->setEnabled(false);
    }
}

void MainWindow::goNext()
{
    WebView *w = (WebView*)bookView->currentPageWidget();
    w->forward();
    if (w->history()->canGoBack()) {
        goPrevAct->setEnabled(true);
    } else {
        goPrevAct->setEnabled(false);
    }
    if (w->history()->canGoForward()) {
        goNextAct->setEnabled(true);
    } else {
        goNextAct->setEnabled(false);
    }
}

void MainWindow::reload()
{
    WebView *w = (WebView*)bookView->currentPageWidget();
    w->reload();
}

void MainWindow::bookViewSlots()
{
    connect(bookView, SIGNAL(tabChanged(int)), SLOT(changeViewTabCount(int)));
    connect(bookView, SIGNAL(currentChanged(int)), SLOT(showTabInfo(int)));
    connect(bookView, SIGNAL(statusRequested(QString)),
            SLOT(showStatus(QString)));
    connect(bookView, SIGNAL(searchRequested(SearchDirection,QString)),
            SLOT(viewSearch(SearchDirection,QString)));
    connect(bookView, SIGNAL(pasteRequested(QString)),
            SLOT(pasteSearchText(QString)));
    connect(bookView, SIGNAL(soundRequested(QString)),
            SLOT(playSound(QString)));
    connect(bookView, SIGNAL(videoRequested(QString)),
            SLOT(playVideo(QString)));
    connect(bookView, SIGNAL(externalLinkRequested(QString)),
            SLOT(openExternalLink(QString)));
    connect(bookView, SIGNAL(selectionRequested(QString)),
            SLOT(changeOptSearchButtonText(QString)));
    connect(bookView, SIGNAL(allWebLoaded()),
            SLOT(setWebLoaded()));
}

void MainWindow::groupDockSlots()
{
    connect(groupDock, &QDockWidget::visibilityChanged, toggleDockAct, &QAction::setChecked);
    connect(groupDock, SIGNAL(searchRequested(QString, SearchMethod)),
            SLOT(viewSearch(QString, SearchMethod)));
    connect(groupDock, SIGNAL(webRequested(QString, QString)),
            SLOT(viewWeb(QString, QString)));
    connect(groupDock, SIGNAL(pasteRequested(QString, SearchMethod)),
            SLOT(pasteMethod(QString, SearchMethod)));
    connect(groupDock, SIGNAL(bookViewRequested(Book*)),
            SLOT(viewInfo(Book*)));
    connect(groupDock, SIGNAL(fontViewRequested(Book*)),
            SLOT(setBookFont(Book*)));
    connect(groupDock, SIGNAL(menuRequested()), SLOT(viewMenu()));
    connect(groupDock, SIGNAL(fullRequested()), SLOT(viewFull()));
}

void MainWindow::setWebLoaded()
{
    stopAct->setEnabled(false);
}
