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

#include <QtGui>

#include "mainwindow.h"
#include "ssheet.h"
#include "ebook.h"
#include "book.h"
#include "bookview.h"
#include "statusbutton.h"
#include "groupdock.h"
#include "booksetting.h"
#include "fontsetting.h"
#include "ssheetsetting.h"
#include "configure.h"
#include "configsetting.h"

const char *Program = { "qolibri" };

#define CONNECT_BUSY(widget) \
    connect(this, SIGNAL(nowBusy(bool)), widget, SLOT(setDisabled(bool)))

MainWindow::MainWindow()
{
#ifdef Q_WS_MAC
    //setUnifiedTitleAndToolBarOnMac(true);
    //setAttribute(Qt::WA_MacBrushedMetal);
#endif
    //qDebug() << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    //qDebug() << QCoreApplication::applicationDirPath();

    CONF->load();

    EBook::initialize();

    bookView = new BookView(this);

    groupDock = new GroupDock(this);
//#if defined (Q_WS_X11) || defined (Q_WS_WIN)
//    groupDock->hide();
//#endif

    createMenus();
    createToolBars();
    createStatusBar();

    CONNECT_BUSY(bookView);
    CONNECT_BUSY(groupDock);

    readSettings();


    setCentralWidget(bookView);
    Group *g = (bookMode == ModeBook) ? method.groupReader : method.group;

    groupDock->changeGroupList(&groupList);
    changeGroup(groupList.indexOf(g));

    setTitle();
    qApp->setWindowIcon(QIcon(":/images/title.png"));

    changeViewTabCount(0);

    sound = NULL;
    timer = NULL;

#if defined (Q_WS_X11)
    timerDock = NULL;
    timerDock = new QTimer(this);
    timerDock->setSingleShot(true);
    connect(timerDock, SIGNAL(timeout()), this, SLOT(showDock()));
    timerDock->start(500);
#elif defined (Q_WS_WIN)
    showDock();
#endif

    if (groupList[0]->bookList().count() == 0) {
        timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, SIGNAL(timeout()), this, SLOT(setBooks()));
        timer->start(300);
    }
    statusBar()->setStyleSheet(CONF->statusBarSheet);
}

void MainWindow::createMenus()
{
    QMenu *fmenu = menuBar()->addMenu(tr("&File"));

    enterAct = fmenu->addAction(QIcon(":/images/key_enter.png"), tr("&Search"),
                                this, SLOT(viewSearch()), tr("Ctrl+F"));
    CONNECT_BUSY(enterAct);
    openBookAct = fmenu->addAction(QIcon(":/images/bookopen.png"),
                                   tr("&Open Book"),
                                   this, SLOT(viewMenu()), tr("Ctrl+O"));
    CONNECT_BUSY(openBookAct);

    viewAllAct = fmenu->addAction(QIcon(":/images/find.png"),
                                  tr("&View All Data"),
                                  this, SLOT(viewFull()), tr("Ctrl+D"));
    CONNECT_BUSY(viewAllAct);

    stopAct = fmenu->addAction(QIcon(":/images/stop.png"), tr("&Cancel"),
                               bookView, SLOT(stopSearch()), Qt::Key_Escape);
    connect(stopAct, SIGNAL(triggered()), this, SLOT(stopSound()));
    stopAct->setEnabled(false);
    exitAct = fmenu->addAction(tr("E&xit"), this, SLOT(close()), tr("Ctrl+Q"));
    CONNECT_BUSY(exitAct);

    QMenu *emenu = menuBar()->addMenu(tr("&Edit"));
    clearEditAct = emenu->addAction(QIcon(":/images/clear.png"),
                                    tr("clear search text"));


    QMenu *vmenu = menuBar()->addMenu(tr("&View"));
    toggleDockAct = new QAction(QIcon(":/images/dock_mac.png"),
                                tr("Dock On/Off"), this);
    toggleDockAct->setCheckable(true);
    connect(toggleDockAct, SIGNAL(triggered(bool)),
            this, SLOT(toggleDock(bool)));
    vmenu->addAction(toggleDockAct);
    toggleBarAct = vmenu->addAction(QIcon(":/images/find_l.png"),
                                    tr("search/read book"),
                                    this, SLOT(toggleBar()));
    zoomInAct = vmenu->addAction(QIcon(":/images/zoomin.png"), tr("zoom &out"),
                                 bookView, SLOT(zoomIn()), QString("Ctrl+-"));
    zoomOutAct = vmenu->addAction(QIcon(":/images/zoomout.png"), tr("zoom &in"),
                                  bookView, SLOT(zoomOut()), QString("Ctrl++"));


    QMenu *smenu = menuBar()->addMenu(tr("&Setting"));
    booksAct = smenu->addAction(QIcon(":/images/books.png"), tr("Group set"),
                                this, SLOT(setBooks()));
    CONNECT_BUSY(booksAct);
    configAct = smenu->addAction(QIcon(":/images/setting.png"),
                                 tr("Option setting"), this, SLOT(setConfig()));
    CONNECT_BUSY(configAct);
    addMarkAct = smenu->addAction(QIcon(":/images/bookmark.png"),
                                  tr("Book Mark"),
                                  this, SLOT(addMark()));
    CONNECT_BUSY(addMarkAct);
    toggleTabsAct = smenu->addAction(QIcon(":/images/tabs.png"),
                                     tr("tab on/off"));
    toggleTabsAct->setCheckable(true);
    CONNECT_BUSY(toggleTabsAct);
    QMenu *mf = smenu->addMenu(QIcon(":/images/font1.png"), tr("Font Set"));
    fontAct = mf->menuAction();
    mf->addAction(QIcon(":/images/font1.png"), tr("Browser Font"),
                  this, SLOT(setViewFont()));
    mf->addAction(QIcon(":/images/font2.png"), tr("Application Font"),
                  this, SLOT(setAppFont()));

    QMenu *ms = smenu->addMenu(QIcon(":/images/stylesheet.png"),
                               tr("Style Sheet"));
    sSheetAct = ms->menuAction();
    ms->addAction(QIcon(":/images/stylesheet.png"),
                  tr("Dictionary Style Sheet"), this, SLOT(setDictSheet()));
    ms->addAction(QIcon(":/images/stylesheet2.png"),
                  tr("Book Style Sheet"), this, SLOT(setBookSheet()));
    ms->addAction(QIcon(":/images/stylesheet3.png"),
                  tr("Internal Style Sheet 1"), this, SLOT(setStatusBarSheet()));
    optDirectionMenu = smenu->addMenu(QIcon(":/images/find_l.png"),
                                   tr("Option Search"));
    optDirectionGroup = new QActionGroup(this);
    QAction *act;
    act = optDirectionMenu->addAction(QObject::tr("&Exact Word Search"));
    act->setData(ExactWordSearch);
    optDirectionGroup->addAction(act);
    act = optDirectionMenu->addAction(QObject::tr("&Forward Search"));
    act->setData(ForwardSearch);
    optDirectionGroup->addAction(act);
    act = optDirectionMenu->addAction(QObject::tr("&Keyword Search"));
    act->setData(KeywordSearch);
    optDirectionGroup->addAction(act);
    act = optDirectionMenu->addAction(QObject::tr("&Cross Search"));
    act->setData(CrossSearch);
    optDirectionGroup->addAction(act);
    optDirectionMenu->addSeparator();
    act = optDirectionMenu->addAction(QObject::tr("&Google Search"));
    act->setData(GoogleSearch);
    optDirectionGroup->addAction(act);
    act = optDirectionMenu->addAction(QObject::tr("&WikiPedia Search"));
    act->setData(WikipediaSearch);
    optDirectionGroup->addAction(act);
    act = optDirectionMenu->addAction(QObject::tr("&User Defined URL Search"));
    act->setData(Option1Search);
    optDirectionGroup->addAction(act);
    foreach(QAction * a, optDirectionMenu->actions()) {
        a->setCheckable(true);
    }
    connect(optDirectionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(changeOptDirection(QAction*)));


#if !defined (Q_WS_WIN)
    smenu->addAction(QIcon(":/images/delete.png"), tr("&Clear Cashe"),
                     this, SLOT(clearCashe()));
#endif
    toggleRubyAct = smenu->addAction(QIcon(":/images/ruby.png"),
                                     tr("&Suppress Subscription"), this,
                                     SLOT(toggleRuby()));
    toggleRubyAct->setCheckable(true);

    QMenu *hmenu = menuBar()->addMenu(tr("&Help"));
    hmenu->addAction(tr("about QT"), qApp, SLOT(aboutQt()));
}

void MainWindow::createToolBars()
{
    setIconSize(QSize(32, 32));
    QToolBar *bar1 = addToolBar("Search/Read Book");
    bar1->setMovable(false);
    bar1->addAction(toggleBarAct);
#ifdef Q_WS_MAC
    bar1->addSeparator();
#endif
    //bar1->addWidget(new QLabel("  "));
    searchBar = addToolBar("Search");
    searchBar->setMovable(false);
    StatusButton *gbutton1 = new StatusButton(groupDock->groupCombo(),
                                              "group", this, true);
    gbutton1->setStyleSheet("QPushButton { font-weight:bold; }");
    searchBar->addWidget(gbutton1);

    searchBar->addAction(clearEditAct);
    searchTextEdit = new QLineEdit(this);
    connect(searchTextEdit, SIGNAL(returnPressed()), this, SLOT(viewSearch()));
    connect(searchTextEdit, SIGNAL(textChanged(QString)),
            this, SLOT(changeSearchText(QString)));
    connect(clearEditAct, SIGNAL(triggered()), searchTextEdit, SLOT(clear()));
    connect(clearEditAct, SIGNAL(triggered()), searchTextEdit, SLOT(setFocus()));
    CONNECT_BUSY(searchTextEdit);
    searchBar->addWidget(searchTextEdit);
    searchBar->addAction(enterAct);
    searchBar->addAction(stopAct);

    bookBar = addToolBar("Read Book");
    bookBar->setMovable(false);
    StatusButton *gbutton2 = new StatusButton(groupDock->groupCombo(),
                                              "group", this, true);
    gbutton2->setStyleSheet("QPushButton { font-weight:bold; }");
    bookBar->addWidget(gbutton2);

    StatusButton *bbutton = new StatusButton(groupDock->bookListWidget(),
                                             "book", this, true);
    bbutton->setStyleSheet("QPushButton { font-weight:bold;  }");
    bookBar->addWidget(bbutton);
    bookBar->addAction(openBookAct);
    bookBar->hide();

    QToolBar *bar2 = addToolBar("Options 1");

    bar2->setMovable(false);
#ifdef Q_WS_MAC
    bar2->addSeparator();
#endif
    //bar2->addAction(booksAct);
    bar2->addAction(addMarkAct);
    bar2->addAction(zoomInAct);
    bar2->addAction(zoomOutAct);

    bar2->addAction(toggleTabsAct);

    addToolBarBreak();
    methodBar = addToolBar(tr("Search Methods"));
    methodBar->setMovable(false);
    //methodBar->addWidget(new QLabel("  "));

    methodCombo = new QComboBox(this);
    methodCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    methodCombo->addItem(QObject::tr("Exact Word Search"));
    methodCombo->addItem(QObject::tr("Forward Search"));
    methodCombo->addItem(QObject::tr("Backward Search"));
    methodCombo->addItem(QObject::tr("Keyword Search"));
    methodCombo->addItem(QObject::tr("Cross Search"));
    methodCombo->addItem(QObject::tr("Full Text Search"));
    methodCombo->setCurrentIndex(-1);
    connect(methodCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeDirection(int)));
    methodBar->addWidget(methodCombo);
    methodBar->addWidget(new QLabel(tr(" Logic:")));
    logicCombo = new QComboBox(this);
    logicCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    logicCombo->addItem(tr("AND"));
    logicCombo->addItem(tr("OR"));
    logicCombo->setCurrentIndex(-1);
    connect(logicCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeLogic(int)));
    methodBar->addWidget(logicCombo);

    methodBar->addWidget(new QLabel(tr(" Hit Limit (Book/Total):")));
    limitBookSpin = new QSpinBox();
    limitBookSpin->setRange(CONF->stepBookHitMax, CONF->maxLimitBookHit);
    limitBookSpin->setSingleStep(CONF->stepBookHitMax);
    connect(limitBookSpin, SIGNAL(valueChanged(int)),
            this, SLOT(changeLimitBook(int)));
    methodBar->addWidget(limitBookSpin);
    methodBar->addWidget(new QLabel(tr("/")));
    limitTotalSpin = new QSpinBox();
    limitTotalSpin->setRange(CONF->stepTotalHitMax, CONF->maxLimitTotalHit);
    limitTotalSpin->setSingleStep(CONF->stepTotalHitMax);
    connect(limitTotalSpin, SIGNAL(valueChanged(int)),
            this, SLOT(changeLimitTotal(int)));
    methodBar->addWidget(limitTotalSpin);
    methodBar->addSeparator();
    methodBar->addAction(booksAct);
    methodBar->addAction(sSheetAct);
    methodBar->addAction(fontAct);

    toggleMethodBarAct = methodBar->toggleViewAction();
    toggleMethodBarAct->setText(tr("option bar On/Off"));
    toggleMethodBarAct->setIcon(QIcon(":/images/configure.png"));

    bar2->addAction(toggleMethodBarAct);
    bar2->addAction(toggleDockAct);
}

void MainWindow::createStatusBar()
{
    QStatusBar *bar = statusBar();

    bar->setStyleSheet(statusBarStyleSheet);

    StatusButton *mbutton = new StatusButton(methodCombo, "direction");
    mbutton->setFlat(true);
    bar->addWidget(mbutton);

    StatusButton *lbutton = new StatusButton(logicCombo, "logic");
    lbutton->setFlat(true);
    bar->addWidget(lbutton);

    StatusButton *bbutton = new StatusButton(limitBookSpin, "limit-book");
    bbutton->setFlat(true);
    bar->addWidget(bbutton);

    StatusButton *tbutton = new StatusButton(limitTotalSpin, "limit-total");
    tbutton->setFlat(true);
    bar->addWidget(tbutton);

    processLabel = new QLabel(this);
    processLabel->setObjectName("process");
    bar->addWidget(processLabel, 1);

    StatusButton *sbutton = new StatusButton(optDirectionMenu, "search", this);
    sbutton->setFlat(true);
    bar->addWidget(sbutton);

    optSearchButton = new QPushButton(this);
    optSearchButton->setObjectName("selection");
    optSearchButton->setFlat(true);
    connect(optSearchButton, SIGNAL(clicked()), this, SLOT(doSearch()));
    CONNECT_BUSY(optSearchButton);
    bar->addWidget(optSearchButton);
}

void MainWindow::readSettings()
{
    QSettings settings("QOLIBRI_1_0", "EpwingViewer");
    QSize size = settings.value("size", QSize(800, 600)).toSize();

    resize(size);
    bool dock = settings.value("dock", 1).toBool();
    toggleDockAct->setChecked(dock);

    bool search_bar = settings.value("search_bar", 1).toBool();
    if (!search_bar) {
        searchBar->hide();
        bookBar->show();
        bookMode = ModeBook;
        toggleBarAct->setIcon(QIcon(":images/book.png"));
    } else {
        bookMode = ModeDictionary;
    }
    bool method_bar = settings.value("method_bar", 1).toBool();
    if (!method_bar) {
        methodBar->hide();
    }
    bool tab = settings.value("newtab", 1).toBool();
    toggleTabsAct->setChecked(tab);
    int searchsel = settings.value("searchsel", (int)WikipediaSearch).toInt();
    optDirection = (SearchDirection)searchsel;
    foreach(QAction * act, optDirectionGroup->actions()) {
        if (act->data().toInt() == searchsel) {
            //act->setChecked(true);
            act->trigger();
        }
    }

    QSettings groups("QOLIBRI_1_0", "EpwingGroups");
    int gcnt = groups.beginReadArray("DictionaryGroups");
    for (int i = 0; i < gcnt; i++) {
        groups.setArrayIndex(i);
        QString name = groups.value("name").toString();
        QString use = groups.value("use").toString();
        bool bUse = false;
        if (use == "ON") bUse = true;
        Group *g = new Group(name, bUse);
        groupList << g;
        int dcnt = groups.beginReadArray("Dictionaries");
        for (int j = 0; j < dcnt; j++) {
            groups.setArrayIndex(j);
            name = groups.value("name").toString();
            QString path = groups.value("path").toString();
            int subbook = groups.value("subbook").toInt();
            use = groups.value("use").toString();
            bUse = false;
            if (use == "ON") bUse = true;
            Book *d = new Book(name, path, subbook, bUse);
            g->addBook(d);
        }
        groups.endArray();
    }
    groups.endArray();
    if (groupList.count() == 0) {
        groupList << new Group(tr("All Books"));
    }

    method = readMethodSetting(settings);

    methodCombo->setCurrentIndex(method.direction);
    changeDirection(method.direction);
    logicCombo->setCurrentIndex(method.logic);
    changeLogic((int)method.logic);
    limitBookSpin->setValue(method.limitBook);
    changeLimitBook(method.limitBook);
    limitTotalSpin->setValue(method.limitTotal);
    changeLimitTotal(method.limitTotal);
    toggleRubyAct->setChecked(!method.ruby);

    QSettings hist("QOLIBRI_1_0", "EpwingHistory");
    int hcnt = hist.beginReadArray("History");
    for (int i = 0; i < hcnt && i < CONF->historyMax; i++) {
        hist.setArrayIndex(i);
        QString str = hist.value("name").toString();
        SearchMethod m = readMethodSetting(hist);
        if (!m.group || !m.book) continue;

        groupDock->addHistory(str, m);
    }
    hist.endArray();

    QSettings mark("QOLIBRI_1_0", "EpwingBookmark");
    int mcnt = mark.beginReadArray("Bookmark");
    for (int i = 0; i < mcnt; i++) {
        mark.setArrayIndex(i);
        QString str = mark.value("name").toString();
        SearchMethod method = readMethodSetting(mark);
        if (!method.group || !method.book) continue;

        groupDock->addMark(str, method);
    }
    mark.endArray();
}

void MainWindow::writeSettings()
{
    QSettings settings("QOLIBRI_1_0", "EpwingViewer");

    settings.setValue("size", size());
    settings.setValue("dock", toggleDockAct->isChecked());
    settings.setValue("search_bar", searchBar->isVisible());
    settings.setValue("method_bar", toggleMethodBarAct->isChecked());

    settings.setValue("newtab", toggleTabsAct->isChecked());
    settings.setValue("searchsel", optDirection);
    writeMethodSetting(method, &settings);

    QSettings groups("QOLIBRI_1_0", "EpwingGroups");
    groups.beginWriteArray("DictionaryGroups");
    for (int i = 0; i < groupList.count(); i++) {
        Group *g = groupList[i];
        groups.setArrayIndex(i);
        groups.setValue("name", g->name());
        if (g->checkState() == Qt::Checked) {
            groups.setValue("use", "ON");
        } else {
            groups.setValue("use", "OFF");
        }
        groups.beginWriteArray("Dictionaries");
        for (int j = 0; j < g->bookList().count(); j++) {
            Book *d = g->bookList()[j];
            groups.setArrayIndex(j);
            groups.setValue("name", d->name());
            groups.setValue("path", d->path());
            groups.setValue("subbook", d->bookNo());
            if (d->checkState() == Qt::Checked) {
                groups.setValue("use", "ON");
            } else {
                groups.setValue("use", "OFF");
            }
        }
        groups.endArray();
    }
    groups.endArray();

    QSettings history("QOLIBRI_1_0", "EpwingHistory");
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

    QSettings mark("QOLIBRI_1_0", "EpwingBookmark");
    mark.beginWriteArray("Bookmark");
    QListWidget *mlist = groupDock->markListWidget();
    for (int i = 0; i < mlist->count(); i++) {
        mark.setArrayIndex(i);
        SearchItem *item = (SearchItem *)mlist->item(mlist->count() - i - 1);
        mark.setValue("name", item->searchStr());
        writeMethodSetting(item->method(), &mark);
    }
    mark.endArray();
}

void MainWindow::setTitle()
{
    QString title = Program;

    if (method.group) {
        if (bookMode == ModeDictionary) {
            title += " - " + method.group->name();
        } else {
            title += " - " + method.groupReader->name();
            if (method.bookReader) {
                title += " - " + method.bookReader->name();
            }
        }
    }
    setWindowTitle(title);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (stopAct->isEnabled()) {
        connect(this, SIGNAL(searchFinished()), this, SLOT(close()));
        stopAct->trigger();
        event->ignore();
        return;
    }

    writeSettings();

    groupDock->close();

    CONF->save();

    EBook::finalize();

    event->accept();
}

#if defined (Q_WS_X11) || defined (Q_WS_WIN)

bool MainWindow::event(QEvent *ev)
{
    if (ev->type() == QEvent::WindowActivate) {
        setDockPosition();
        moveDock();
        groupDock->raise();
        return false;
    } else if (ev->type() == QEvent::Move) {
        moveDock();
        return false;
    } else if (ev->type() == QEvent::Resize) {
        resizeDock();
        moveDock();
        return false;
    } else if (ev->type() == QEvent::WindowStateChange) {
        if (windowState() & Qt::WindowMinimized) {
            groupDock->hide();
        } else if (windowState() & Qt::WindowActive) {
            setDockPosition();
            moveDock();
            if (toggleDockAct->isChecked()) {
                groupDock->show();
            } else {
            }
        } else if (windowState() == Qt::WindowNoState) {
            if (toggleDockAct->isChecked()) {
                groupDock->show();
            }
        }
        return true;
    }
    return QWidget::event(ev);
}


#endif


Group *MainWindow::groupFromName(const QString &name)
{
    if (groupList.count() == 0) {
        return NULL;
    }
    foreach(Group * g, groupList) {
        if (g->name() == name) {
            return g;
        }
    }

    return groupList[0];
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
    return grp->bookList()[0];
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
    m.ruby = set.value("ruby", true).toBool();

    return m;
}

void MainWindow::writeMethodSetting(const SearchMethod &m, QSettings *set)
{
    set->setValue("search_method", m.direction);
    set->setValue("narrowing_logic", m.logic);
    set->setValue("limit_book", m.limitBook);
    set->setValue("limit_total", m.limitTotal);
    set->setValue("ruby", m.ruby);
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

    if (!str.isEmpty()) msg = " :";
    if (!stopAct->isEnabled()) {
        if (groupList.count() > 0 && groupList[0]->bookList().count() == 0) {
            msg += tr(" No Search Book");
        } else if (bookMode == ModeDictionary &&
                   searchTextEdit->text().isEmpty()) {
            //msg += tr(" Input Search Text");
        } else {
            msg += tr(" ready");
        }
    } else {
        msg += tr(" retrieving . . .");
    }
    processLabel->setText(str + msg);
}


void MainWindow::toggleDock(bool check)
{
    if (check) {
#if !defined (Q_WS_MAC)
        DockPosition p = dockPosition;
        setDockPosition();
        if (p != dockPosition) {
            moveDock();
        }
#endif
        groupDock->show();
    } else {
        groupDock->hide();
    }
}

void MainWindow::closedDock()
{
    toggleDockAct->setChecked(false);
}


#if defined (Q_WS_X11) || defined (Q_WS_WIN)
void MainWindow::setDockPosition()
{
    int left = mapToGlobal(QPoint(0, 0)).x();
    int right = QDesktopWidget().screenGeometry(1).width() - width() -
                left - 2;

    //
    //qDebug() << "left" << left << "right" << right
    //         << "width" << width() << "screen width"
    //         << QDesktopWidget().screenGeometry(this).width();
    //
    // "screenGeometry()" returns wrong desktop size(width),
    // when gave parameter ("this") or ("-1").
    // Possibly this occurred on linux workstation of macbook.
    //
    dockPosition =  (left > right) ? DockLeft : DockRight;

}
void MainWindow::resizeDock()
{
    int height = size().height() -
                 (frameSize().height() - size().height());
    //int height = size().height() -
    //             ( groupDock->frameSize().height() - groupDock->size().height());
    //int width = 280 +
    //             ( groupDock->frameSize().width() - groupDock->size().width());
    //qDebug() << "frameSize width" << groupDock->frameSize().width()
    //         << "width" << groupDock->size().width();
    //qDebug() << "frameSize height" << groupDock->frameSize().height()
    //         << "height" << groupDock->size().height();

    QSize sz(280, height);

    //QSize sz = groupDock->size();
    //sz.setHeight(size().height());
    //sz.setWidth(280);
    groupDock->resize(sz);
}
void MainWindow::moveDock()
{
    QPoint pnt;

    if (dockPosition == DockLeft) {
        int w = 280 + (groupDock->frameSize().width() - groupDock->size().width());
        pnt = mapToGlobal(QPoint(-w, 0));
        if (pnt.x() < 0) {
            pnt.setX(0);
        }
    } else {
        pnt = mapToGlobal(QPoint(width(), 0));
    }
    groupDock->move(pnt);
}

void MainWindow::showDock()
{
    //
    // qDebug() << ((isVisible()) ? "Visible" : "Unvisible");
    //
    // Delay resizing (after MainWindow shown?).
    // This stupid code is meant to get real MainWindow height.
    // And this may not get acurate size yet.
    //
    setDockPosition();
    resizeDock();
    //connect(toggleDockAct, SIGNAL(triggered(bool)),
    //        this, SLOT(toggleDock(bool)));
    moveDock();
    if (!toggleDockAct->isChecked()) {
        groupDock->hide();
    } else {
        groupDock->show();
    }
#if defined (Q_WS_X11)
    delete timerDock;
#endif
}

#endif

void MainWindow::toggleBar()
{
    Group *group;

    if (searchBar->isVisible()) {
        searchBar->hide();
        bookBar->show();
        bookMode = ModeBook;
        group = method.groupReader;
        toggleBarAct->setIcon(QIcon(":images/book.png"));
    } else {
        searchBar->show();
        bookBar->hide();
        bookMode = ModeDictionary;
        group = method.group;
        toggleBarAct->setIcon(QIcon(":images/find_l.png"));
    }
    changeGroup(groupList.indexOf(group));
}

void MainWindow::toggleRuby()
{
    method.ruby = ! toggleRubyAct->isChecked();
}

void MainWindow::changeGroup(int group_index)
{
    if (groupList.count() == 0) {
        method.group = NULL;
        method.groupReader = NULL;
        changeBook(-1);
        return;
    }
    if (group_index < 0) {
        group_index = 0;
    }

    int book_index = 0;
    Group *group = groupList[group_index];
    if (bookMode == ModeDictionary) {
        method.group = group;
        book_index = group->bookList().indexOf(method.book);
    } else {
        method.groupReader = group;
        book_index = group->bookList().indexOf(method.bookReader);
    }
    groupDock->changeGroup(group_index);
    changeBook(book_index);
}

void MainWindow::changeBook(int index)
{
    Group *group = (bookMode == ModeBook) ? method.groupReader : method.group;

    if (!group || group->bookList().count() == 0) {
        if (bookMode == ModeDictionary) {
            method.book = NULL;
        } else {
            method.bookReader = NULL;
        }
        return;
    }

    if (index < 0) {
        index = 0;
    }

    if (bookMode == ModeDictionary) {
        method.book = group->bookList()[index];
    } else {
        method.bookReader = group->bookList()[index];
    }

    groupDock->setCurrentBook(index);

    setTitle();
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
    SearchMethod m = method;

    m.book = book;
    m.direction = BookInfo;
    viewSearch(m.book->name(), m);
}

void MainWindow::setBookFont(Book *book)
{
    QString cashe_path = loadAllExternalFont(book);
    FontSetting dlg(book, this);
    if (dlg.setupTreeWidget(cashe_path) < 1) {
        QMessageBox::information(this, "Font Setting", tr("No External Font"));
        showStatus(tr("No External Font"));
        return;
    }
    int w = dlg.size().width();
    int h = size().height() - 50;

    dlg.resize(QSize(w, h));
    bool del_flag = true;
    if (dlg.exec() == QDialog::Accepted) {
        QHash <QString, QString> *flist =  dlg.newAlternateFontList();
        foreach(Group * g, groupList) {
            foreach(Book * b, g->bookList()) {
                if (b->path() != book->path()) continue;

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
    BookSetting dlg(groupList, this);

    if (dlg.exec() == QDialog::Accepted) {
        QList <Group*> grp = dlg.groupList();

        groupList.clear();
        foreach(Group * g, grp) {
            groupList << new Group(*g);
        }

        int idx = 0;
        Book *book;
        int book_idx = 0;
        if (groupList.count() == 0) {
            method.group = NULL;
            method.groupReader = NULL;
            book = NULL;
            idx = -1;
            book_idx = -1;
        } else {
            method.group = groupList[0];
            method.groupReader = groupList[0];
            if (groupList[0]->bookList().count() == 0) {
                book = NULL;
                book_idx = -1;
            } else {
                book = groupList[0]->bookList()[0];
            }
        }
        groupDock->changeGroupList(&groupList);
        //qDebug() << method.book->name();
        changeGroup(idx);
        method.book = book;
        method.bookReader = book;
        changeBook(book_idx);
        QString msg;
        msg = "Group=" + QString().setNum(groupList.count()) +
              ", Book=" + QString().setNum(groupList[0]->bookList().count());

        showStatus(msg);
    } else {
        showStatus("Group Setting Cancelled");
    }
    if (timer) {
        delete timer;
        timer = NULL;
    }
}

void MainWindow::viewSearch()
{
    QString str = searchTextEdit->text();

//    if (str.isEmpty()) {
//        qDebug() << "MassageBox( Input Search Text)";
//        return;
//    }
    viewSearch(str, method );
    searchTextEdit->setFocus();
    searchTextEdit->selectAll();
}

void MainWindow::viewMenu()
{
    SearchMethod m = method;

    m.direction = MenuRead;
    if (bookMode == ModeDictionary) {
        m.groupReader = method.group;
        m.bookReader = method.book;
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

    if (bookMode == ModeDictionary) {
        searchTextEdit->setFocus();
        searchTextEdit->selectAll();
    }
}

void MainWindow::viewFull()
{
    SearchMethod m = method;

    m.direction = WholeRead;
    if (bookMode == ModeDictionary) {
        m.groupReader = method.group;
        m.bookReader = method.book;
    }
    viewSearch(m.bookReader->name(), m);

    searchTextEdit->setFocus();
    searchTextEdit->selectAll();
}

void MainWindow::viewSearch(const QString &name, const SearchMethod &mthd)
{
    QTime pTime;

    pTime.start();

    QStringList list;
    if ( mthd.direction == WholeRead || mthd.direction == BookInfo ||
        mthd.direction == MenuRead ) {
       list << name;
    } else {
        list = name.split(QRegExp("\\W+"), QString::SkipEmptyParts);
        if (list.count() == 0) {
            showStatus("<b><font color=#993333>Input search word</font></b>");
            return;
        }
    }
    optSearchButton->setText(list[0]);
    emit nowBusy(true);
    stopAct->setEnabled(true);

    bool ntab = toggleTabsAct->isChecked();

    RET_SEARCH ret = bookView->newPage(list, mthd, ntab);
    QString sstr = toLogicString(list, mthd);
    groupDock->addHistory(toLogicString(list, mthd), mthd);

    emit nowBusy(false);
    stopAct->setEnabled(false);
    QString msg;
    QString msg_b = QString();
    if (ret == NORMAL) {
        msg = "OK : ";
    } else {
        msg = "<b><font color=#993333>";
        switch (ret) {
            case NOT_HIT:
                msg_b = tr("Word not found.");
                msg += msg_b;
                break;
            case NO_MENU:
                msg_b = tr("Not have menu data.");
                msg += msg_b;
                break;
            case NO_BOOK:
                msg_b = tr("Can't open the book.");
                msg += msg_b;
                break;
            case NOT_HIT_INTERRUPTED:
            case INTERRUPTED:
                msg_b = tr("Interrupted.");
                msg += msg_b;
                break;
            case LIMIT_CHAR:
                msg += tr("Stopped (Character size limit[") +
                       QString().setNum(CONF->limitBrowserChar) + "])";
                break;
            case LIMIT_IMAGE:
                msg += tr("Stopped (Image limit[") +
                       QString().setNum(CONF->limitImageNum) + "])";
                break;
            case LIMIT_TOTAL:
                msg += tr("Stopped (Item iimit[") +
                       QString().setNum(mthd.limitTotal) + "])";
                break;
            default:
                msg += "Error";
                qWarning() << "Unrecognized Error Code" << ret;
                break;
        }
        msg += "</font></b> : ";
    }
    if (CONF->beepSound) {
        qApp->beep();
    }
    int etime = pTime.elapsed();
    msg += QString().setNum((double)etime / 1000.0, 'f', 2);
    showStatus(msg);
    if (!msg_b.isEmpty()) {
        QMessageBox::warning(this, Program, msg_b);
    }
    emit searchFinished();
}
void MainWindow::pasteMethod(const QString &str, const SearchMethod &m)
{
    if (m.direction == WholeRead || m.direction == MenuRead ||
        m.direction == BookInfo ) {
        if (bookMode != ModeBook) {
            toggleBar();
        }
        method.bookReader = m.bookReader;
        changeGroup(groupList.indexOf(m.groupReader));
    } else {
        if (bookMode != ModeDictionary) {
            toggleBar();
        }
        methodCombo->setCurrentIndex(m.direction);
        changeDirection(m.direction);
        logicCombo->setCurrentIndex(m.logic);
        changeLogic((int)m.logic);
        limitBookSpin->setValue(m.limitBook);
        changeLimitBook(m.limitBook);
        limitTotalSpin->setValue(m.limitTotal);
        changeLimitTotal(m.limitTotal);
        changeGroup(groupList.indexOf(m.group));
        QStringList list = str.split(QRegExp("\\W+"), QString::SkipEmptyParts);
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
            addr += "%" + QString().setNum((ushort)((uchar)c), 16);
        }
        execProcess(CONF->browserProcess + ' ' + addr);
    }
}

void MainWindow::viewSearch(SearchDirection direction, const QString &text)
{
    bool ntab = toggleTabsAct->isChecked();

    toggleTabsAct->setChecked(true);

    SearchMethod m = method;
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

void MainWindow::execProcess(const QString &prog)
{
    QProcess *proc = new QProcess(this);
    connect(proc, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(execError(QProcess::ProcessError)));
    proc->start(prog);

    QString msg = "Execute :" + prog;
    showStatus(msg);
}
void MainWindow::execError(QProcess::ProcessError e)
{
    QString msg;
    if (e == QProcess::FailedToStart) {
        msg = tr("Failed to start process.");
    } else {
        msg = QString(tr("Error occured during staring process(code=%1)."))
                      .arg((int)e);
    }
    QMessageBox::warning(this, Program, msg );
    showStatus(msg);
}

void MainWindow::execSound(const QString &fname)
{
    stopSound();
    checkSound();
    stopAct->setEnabled(true);
    sound = new QSound(fname, this);
    sound->play();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkSound()));
    timer->start(1000);
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
    SearchMethod m = bookView->currentPage()->method();
    QString sstr = bookView->tabText(bookView->currentIndex());
    sstr.replace("&&", "&");
    groupDock->addMark(sstr, m);

    toggleDockAct->setChecked(true);
    groupDock->show();
    groupDock->changeTabToMark();
}

void MainWindow::setDictSheet()
{
    SSheetSetting dlg(CONF->dictSheet, dictStyleSheet, true, this);
    QSize sz = size() - QSize(50, 50);

    dlg.resize(sz);
    if (dlg.exec() == QDialog::Accepted) {
        CONF->dictSheet = dlg.text();
        showStatus("Stylesheet Changed");
    } else {
        showStatus("Cancelled StyleSheel");
    }
}

void MainWindow::setBookSheet()
{
    SSheetSetting dlg(CONF->bookSheet, bookStyleSheet, false, this);
    QSize sz = size() - QSize(50, 50);

    dlg.resize(sz);
    if (dlg.exec() == QDialog::Accepted) {
        CONF->bookSheet = dlg.text();
        showStatus("Stylesheet Changed");
    } else {
        showStatus("Cancelled StyleSheel");
    }
}

void MainWindow::setStatusBarSheet()
{
    SSheetOptSetting dlg(CONF->statusBarSheet, statusBarStyleSheet, this);

    dlg.resize(QSize(450, 550));
    if (dlg.exec() == QDialog::Accepted) {
        CONF->statusBarSheet = dlg.text();
        statusBar()->setStyleSheet(CONF->statusBarSheet);
        showStatus("Stylesheet Changed");
    } else {
        showStatus("Cancelled StyleSheel");
    }
}

void MainWindow::setViewFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, CONF->browserFont, this,
                                      tr("Set Browser Font"));

    if (ok) {
        CONF->browserFont = font;
        QString str = "font-family=" + font.family() +
                      " font-size=" + QString().setNum(font.pointSize());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        emit viewFontChanged(font);
        QApplication::restoreOverrideCursor();
        showStatus(str);
        //} else {
        //showStatus("Cancelled Font Dialog");
    }
}

void MainWindow::setAppFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, qApp->font(),
                                      this, tr("Set Application Font"));

    if (ok) {
        qApp->setFont(font);
        QString str = "font-family=" + font.family() +
                      " font-size=" + QString().setNum(font.pointSize());
        showStatus(str);
    }
}

#if !defined (Q_WS_WIN)
void MainWindow::clearCashe()
{
    int ret = QMessageBox::question(this, Program,
                                    tr("Do you want to remove all cashe?\n\"")
                                    +
                                    EBook::cashePath + "/*\"",
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
//#ifdef Q_WS_WIN
//        execProcess("del /F /Q\ " + EBook::cashePath);
//#else
        execProcess("rm -rf " + EBook::cashePath);
//#endif
    }
}
#endif

void MainWindow::setConfig()
{
    ConfigSetting dlg(this);

    if (dlg.exec() == QDialog::Accepted) {
        dlg.update();
    }
}

void MainWindow::changeSearchText(const QString&)
{
    showStatus();
}

void MainWindow::changeViewTabCount(int tab_count)
{
    if (tab_count == 0 ) {
        addMarkAct->setEnabled(false);
        zoomInAct->setEnabled(false);
        zoomOutAct->setEnabled(false);
    } else {
        addMarkAct->setEnabled(true);
        zoomInAct->setEnabled(true);
        zoomOutAct->setEnabled(true);
    }
}

QString MainWindow::loadAllExternalFont(Book *pbook)
{
    EBook eb(HookFont);
    if (eb.setBook(pbook->path(), pbook->bookNo()) < 0) {
        QMessageBox::warning(this, Program, tr("Can't open the book.") );
    }
    eb.initSearch(16, NULL);
    QFile loadf(eb.fontCashePath() + "/loaded");
    if (loadf.exists()) {
        return eb.fontCashePath();
    }
    emit nowBusy(true);
    //stopAct->setEnabled(true);
    int total = 0;
    for(;;) {
        int hit_num = eb.hitFull(1000);
        total += hit_num;
        QString msg = QString("Please wait... "
                              "Loading all external fonts (%1).").arg(total);
        showStatus(msg);
        for(int i = 0; i < hit_num; i++){
            eb.text(i);
        }

        QEventLoop event;
        event.processEvents();

        if (hit_num < 1000) break;
    }
    loadf.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&loadf);
    out << "All font loaded";
    emit nowBusy(false);

    return eb.fontCashePath();
}
