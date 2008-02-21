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

#include "groupdock.h"
#include "book.h"
#include "groupwidget.h"
#include "configure.h"

SearchItem::SearchItem(const QString &str, const SearchMethod &method)
    : QListWidgetItem(), searchStr_(str), method_(method)
{
    QString s_method;

    switch (method.direction) {
        case ExactWordSearch:
            s_method = QObject::tr("Exact");
            break;
        case ForwardSearch:
            s_method = QObject::tr("Forward");
            break;
        case BackwardSearch:
            s_method = QObject::tr("Backward");
            break;
        case FullTextSearch:
            s_method = QObject::tr("Full");
            break;
        case KeywordSearch:
            s_method = QObject::tr("Keyword");
            break;
        case CrossSearch:
            s_method = QObject::tr("Cross");
            break;
        case WholeRead:
            s_method = QObject::tr("Read Book");
            break;
        case MenuRead:
            s_method = QObject::tr("Read Menu");
            break;
        case BookInfo:
            s_method = QObject::tr("Book Information");
            break;
        default:
            s_method = QObject::tr("-----");
            qWarning() << "Unrecognized search method" << method.direction;
    }

    if (method.direction == WholeRead || method.direction == MenuRead ||
        method.direction == BookInfo ) {
        setText(str + ", " + s_method);
    } else {
        QString text = "\"" + str + "\", ";
        text += s_method + ", " + method.group->name();
        setText(text);
    }
}


GroupTab::GroupTab(QWidget *parent)
    : QWidget(parent), groupList(NULL), group(NULL)
{
    QVBoxLayout *v = new QVBoxLayout();

    groupCombo_ = new QComboBox(this);
    groupCombo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(groupCombo_, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeGroup(int)));
    bookWidget_ = new BookWidget(group, false, true, true, false, false, this);
    bookWidget_->bookListWidget()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bookWidget_, SIGNAL(rowChanged(int)),
            this, SIGNAL(bookChanged(int)));
    v->addWidget(groupCombo_);
    v->addWidget(bookWidget_);

    v->setMargin(0);
    v->setSpacing(0);

    QVBoxLayout *v2 = new QVBoxLayout();
    v2->addLayout(v);
    v2->setMargin(3);
    v2->setSpacing(0);
    setLayout(v2);


    QWidget *mainwin = parent->parentWidget();
    connect(bookWidget_, SIGNAL(bookViewRequested(Book*)),
            mainwin, SLOT(viewInfo(Book*)));
    connect(bookWidget_, SIGNAL(fontViewRequested(Book*)),
            mainwin, SLOT(setBookFont(Book*)));
    connect(this, SIGNAL(bookViewRequested(Book*)),
            mainwin, SLOT(viewInfo(Book*)));
    connect(this, SIGNAL(fontViewRequested(Book*)),
            mainwin, SLOT(setBookFont(Book*)));
    connect(this, SIGNAL(menuRequested()), mainwin, SLOT(viewMenu()));
    connect(this, SIGNAL(fullRequested()), mainwin, SLOT(viewFull()));
    connect(this, SIGNAL(groupChanged(int)), mainwin, SLOT(changeGroup(int)));
    connect(this, SIGNAL(bookChanged(int)), mainwin, SLOT(changeBook(int)));
    connect(bookWidget_->bookListWidget(),
            SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(popupMenu(QPoint)));
}

void GroupTab::changeGroup(int index)
{
    if (index < 0) return;

    group = (*groupList)[index];
    bookWidget_->initBook(group);
    emit groupChanged(index);
    if (group->bookList().count()) {
        emit bookChanged(0);
    } else {
        emit bookChanged(-1);
    }
}

void GroupTab::changeGroupNoSignal(int index)
{
    disconnect(groupCombo_, SIGNAL(currentIndexChanged(int)),
               this, SLOT(changeGroup(int)));
    group = (*groupList)[index];
    groupCombo_->setCurrentIndex(index);
    bookWidget_->initBook(group);
    connect(groupCombo_, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeGroup(int)));
}

void GroupTab::changeGroupList(QList<Group*> *gList)
{
    groupList = gList;
    bookWidget_->initBook(NULL);
    disconnect(groupCombo_, SIGNAL(currentIndexChanged(int)),
               this, SLOT(changeGroup(int)));
    groupCombo_->clear();
    foreach(Group * g, (*groupList)) {
        groupCombo_->addItem(g->name());
    }
    groupCombo_->setCurrentIndex(-1);
    connect(groupCombo_, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeGroup(int)));
}

void GroupTab::popupMenu(const QPoint &pos)
{
    QListWidget *widget = bookWidget_->bookListWidget();
    QListWidgetItem *item = widget->itemAt(pos);

    if (!item) {
        return;
    }
    int index = widget->row(item);
    widget->setCurrentRow(index);

    QMenu menu;
    QAction *iAct = menu.addAction(QIcon(":images/find_l.png"),
                                   tr("Show Book &Information"));
    QAction *rAct = menu.addAction(QIcon(":images/bookopen.png"),
                                   tr("&Read Book"));
    QAction *aAct = menu.addAction(QIcon(":images/bookopen.png"),
                                   tr("Read &All Data"));
    QAction *fAct = menu.addAction(QIcon(":images/font3.png"),
                                   tr("&Font Setting"));
    QAction *a = menu.exec(widget->viewport()->mapToGlobal(pos));
    if (a == iAct) {
        emit bookViewRequested(bookWidget_->currentBook());
    } else if (a == rAct) {
        emit menuRequested();
    } else if (a == aAct) {
        emit fullRequested();
    } else if (a == fAct) {
        emit fontViewRequested(bookWidget_->currentBook());
    }
}

MarkTab::MarkTab(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *v = new QVBoxLayout();

    listWidget_ = new QListWidget(this);
    listWidget_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(listWidget_, SIGNAL(currentRowChanged(int)),
            this, SLOT(resetButtons()));
    connect(listWidget_, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(viewCurrent()));
    connect(listWidget_, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(popupMenu(QPoint)));
    v->addWidget(listWidget_);

    QHBoxLayout *h = new QHBoxLayout();
    h->addStretch();
    upButton = new QPushButton(QIcon(":images/uparrow.png"), QString(), this);
    h->addWidget(upButton);
    downButton = new QPushButton(QIcon(":images/downarrow.png"), QString(),
                                 this);
    h->addWidget(downButton);
    delButton = new QPushButton(QIcon(":images/delete.png"), QString(), this);
    h->addWidget(delButton);
    viewButton = new QPushButton(QIcon(":images/find_l.png"), QString(), this);
    h->addWidget(viewButton);
    connect(upButton, SIGNAL(clicked()), this, SLOT(upCurrent()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(downCurrent()));
    connect(delButton, SIGNAL(clicked()), this, SLOT(delCurrent()));
    connect(viewButton, SIGNAL(clicked()), this, SLOT(viewCurrent()));
    v->addLayout(h);

    setLayout(v);
    v->setSpacing(0);
    v->setMargin(3);
    resetButtons();

    QWidget *mainwin = parent->parentWidget();
    connect(this, SIGNAL(searchRequested(QString, SearchMethod)),
            mainwin, SLOT(viewSearch(QString, SearchMethod)));
    connect(this, SIGNAL(pasteRequested(QString, SearchMethod)),
            mainwin, SLOT(pasteMethod(QString, SearchMethod)));
}

void MarkTab::addMark(const QString &str, const SearchMethod &method)
{
    int i;

    for (i = 0; i < listWidget_->count(); i++) {
        SearchItem *item = (SearchItem*)listWidget_->item(i);
        if (item->searchStr() == str &&
            item->method().group == method.group &&
            item->method().direction == method.direction &&
            item->method().logic == method.logic) {
            return;
        }
    }

    listWidget_->insertItem(0, new SearchItem(str, method));
}

void MarkTab::resetButtons()
{
    upButton->setEnabled(false);
    downButton->setEnabled(false);
    delButton->setEnabled(false);
    viewButton->setEnabled(false);

    int row = listWidget_->currentRow();
    if (row < 0) {
        return;
    }

    if ((row + 1) < listWidget_->count()) {
        downButton->setEnabled(true);
    }
    if (row > 0) {
        upButton->setEnabled(true);
    }

    delButton->setEnabled(true);
    viewButton->setEnabled(true);
}

void MarkTab::delCurrent()
{
    int row = listWidget_->currentRow();
    SearchItem *mrk = (SearchItem*)listWidget_->takeItem(row);

    delete mrk;
}

void MarkTab::viewCurrent()
{
    SearchItem *mrk = (SearchItem*)listWidget_->currentItem();
    emit searchRequested(mrk->searchStr(), mrk->method());
}

void MarkTab::pasteCurrent()
{
    SearchItem *mrk = (SearchItem*)listWidget_->currentItem();
    emit pasteRequested(mrk->searchStr(), mrk->method());
}

void MarkTab::upCurrent()
{
    int row = listWidget_->currentRow();
    SearchItem *mrk = (SearchItem*)listWidget_->takeItem(row);

    listWidget_->insertItem(row - 1, mrk);
    listWidget_->setCurrentItem(mrk);
}

void MarkTab::downCurrent()
{
    int row = listWidget_->currentRow();
    SearchItem *mrk = (SearchItem*)listWidget_->takeItem(row);

    listWidget_->insertItem(row + 1, mrk);
    listWidget_->setCurrentItem(mrk);
}

void MarkTab::changeGroupList(QList<Group*> *gList)
{
    for (int i = listWidget_->count(); i > 0; i--) {
        bool flg = false;
        foreach(Group * g, (*gList)) {
            SearchItem *item = (SearchItem*)listWidget_->item(i - 1);

            if (item->method().group->name() == g->name()) {
                flg = true;
                break;
            }
        }
        if (!flg) {
            SearchItem *mrk = (SearchItem*)listWidget_->takeItem(i - 1);
            delete mrk;
        }
    }
}

void MarkTab::popupMenu(const QPoint &pos)
{
    QListWidgetItem *item = listWidget_->itemAt(pos);

    if (!item) {
        return;
    }
    int index = listWidget_->row(item);
    listWidget_->setCurrentRow(index);
    QMenu menu;
    menu.addAction(QIcon(":images/bookopen.png"), tr("&Execute"),
                   this, SLOT(viewCurrent()));
    menu.addAction(QIcon(":images/copy.png"),
                   tr("&Copy to Current Search Condition"),
                   this, SLOT(pasteCurrent()));
    menu.addAction(QIcon(":images/delete.png"), tr("&Delete"),
                   this, SLOT(deleteCurrnt()));
    menu.exec(listWidget_->viewport()->mapToGlobal(pos));
}

HistoryTab::HistoryTab(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *v = new QVBoxLayout();

    listWidget_ = new QListWidget(this);
    listWidget_->setContextMenuPolicy(Qt::CustomContextMenu);
    v->addWidget(listWidget_);
    connect(listWidget_, SIGNAL(currentRowChanged(int)),
            this, SLOT(resetButtons()));
    connect(listWidget_, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(viewCurrent()));
    connect(listWidget_, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(popupMenu(QPoint)));

    QHBoxLayout *h = new QHBoxLayout();
    h->addStretch();
    delButton = new QPushButton(QIcon(":images/delete.png"), "", this);
    h->addWidget(delButton);
    viewButton = new QPushButton(QIcon(":images/find_l.png"), "", this);
    h->addWidget(viewButton);
    connect(delButton, SIGNAL(clicked()), this, SLOT(delCurrent()));
    connect(viewButton, SIGNAL(clicked()), this, SLOT(viewCurrent()));
    v->addLayout(h);

    v->setSpacing(0);
    v->setMargin(3);
    setLayout(v);

    resetButtons();

    QWidget *mainwin = parent->parentWidget();
    connect(this, SIGNAL(searchRequested(QString, SearchMethod)),
            mainwin, SLOT(viewSearch(QString, SearchMethod)));
    connect(this, SIGNAL(pasteRequested(QString, SearchMethod)),
            mainwin, SLOT(pasteMethod(QString, SearchMethod)));
}

void HistoryTab::addHistory(const QString &str, const SearchMethod &method)
{
    if (listWidget_->count() >= CONF->historyMax) {
        QListWidgetItem *rem = listWidget_->takeItem(listWidget_->count() - 1);
        delete rem;
    }
    listWidget_->insertItem(0, new SearchItem(str, method));
}

void HistoryTab::resetButtons()
{
    if (listWidget_->currentRow() < 0) {
        delButton->setEnabled(false);
        viewButton->setEnabled(false);
    } else {
        delButton->setEnabled(true);
        viewButton->setEnabled(true);
    }
}
void HistoryTab::delCurrent()
{
    int row = listWidget_->currentRow();
    SearchItem *hst = (SearchItem*)listWidget_->takeItem(row);

    delete hst;
}

void HistoryTab::viewCurrent()
{
    SearchItem *item = (SearchItem*)listWidget_->currentItem();
    emit searchRequested(item->searchStr(), item->method());
}

void HistoryTab::pasteCurrent()
{
    SearchItem *mrk = (SearchItem*)listWidget_->currentItem();
    emit pasteRequested(mrk->searchStr(), mrk->method());
}

void HistoryTab::changeGroupList(QList<Group*> *gList)
{
    for (int i = listWidget_->count(); i > 0; i--) {
        bool flg = false;
        foreach(Group * g, (*gList)) {
            SearchItem *item = (SearchItem*)listWidget_->item(i - 1);

            if (item->method().group->name() == g->name()) {
                flg = true;
                break;
            }
        }
        if (!flg) {
            SearchItem *hst = (SearchItem*)listWidget_->takeItem(i - 1);
            delete hst;
        }
    }
}

void HistoryTab::popupMenu(const QPoint &pos)
{
    QListWidgetItem *item = listWidget_->itemAt(pos);

    if (!item) {
        return;
    }
    int index = listWidget_->row(item);
    listWidget_->setCurrentRow(index);
    QMenu menu;
    menu.addAction(QIcon(":images/bookopen.png"), tr("&Execute"),
                   this, SLOT(viewCurrent()));
    menu.addAction(QIcon(":images/copy.png"),
                   tr("&Copy to Current Search Condition"),
                   this, SLOT(pasteCurrent()));
    menu.addAction(QIcon(":images/delete.png"), tr("&Delete"),
                   this, SLOT(delCurrent()));
    menu.exec(listWidget_->viewport()->mapToGlobal(pos));
}

#if defined (Q_WS_MAC)
GroupDock::GroupDock(QWidget *parent)
    : QTabWidget(parent)
#else
GroupDock::GroupDock(QWidget * parent)
    : QWidget(parent)
#endif
{
#if defined (Q_WS_MAC)

    groupTab = new GroupTab(this);
    addTab(groupTab, QIcon(":/images/group.png"), tr("Group"));

    markTab = new MarkTab(this);
    addTab(markTab, QIcon(":/images/bookmark.png"), tr("Mark"));

    historyTab = new HistoryTab(this);
    addTab(historyTab, QIcon(":/images/history.png"), tr("History"));

    setWindowFlags(Qt::Drawer);

#else

    tabWidget = new QTabWidget(this);

    groupTab = new GroupTab(this);
    tabWidget->addTab(groupTab, QIcon(":/images/group.png"), tr("Group"));

    markTab = new MarkTab(this);
    tabWidget->addTab(markTab, QIcon(":/images/marktab.png"), tr("Mark"));

    historyTab = new HistoryTab(this);
    tabWidget->addTab(historyTab, QIcon(":/images/history.png"), tr("History"));

    QHBoxLayout *h = new QHBoxLayout();
    h->setAlignment(0);
    h->setMargin(0);
    h->setSpacing(0);
    h->addWidget(tabWidget);
    setLayout(h);

    setWindowFlags(Qt::Tool);

#endif

    connect(this, SIGNAL(closed()), parent, SLOT(closedDock()));
}
