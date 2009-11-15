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
            s_method = QObject::tr("Read book");
            break;
        case MenuRead:
            s_method = QObject::tr("Read menu");
            break;
        case BookInfo:
            s_method = QObject::tr("Book information");
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
SearchItem::SearchItem(const QString &name, const QString &url)
    : QListWidgetItem(), searchStr_(name), url_(url)
{
    setText(name);
}

GTab::GTab(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *v = new QVBoxLayout();
    listWidget_ = new QListWidget(this);
    listWidget_->setContextMenuPolicy(Qt::CustomContextMenu);
    v->addWidget(listWidget_);
    connect(listWidget_, SIGNAL(currentRowChanged(int)),
            SLOT(resetButtons()));

    buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    upButton = new QPushButton(QIcon(":images/uparrow.png"), QString(), this);
    buttonLayout->addWidget(upButton);
    downButton = new QPushButton(QIcon(":images/downarrow.png"), QString(),
                                 this);
    buttonLayout->addWidget(downButton);
    delButton = new QPushButton(QIcon(":images/delete.png"), QString(), this);
    buttonLayout->addWidget(delButton);
    viewButton = new QPushButton(QIcon(":images/find_l.png"), QString(), this);
    buttonLayout->addWidget(viewButton);
    v->addLayout(buttonLayout);
    v->setSpacing(0);
    v->setMargin(0);
    setLayout(v);

    connect(listWidget_, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(popupMenu(QPoint)));
    connect(listWidget_, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            SLOT(viewCurrent()));

    connect(upButton, SIGNAL(clicked()), SLOT(upCurrent()));
    connect(downButton, SIGNAL(clicked()), SLOT(downCurrent()));
    connect(delButton, SIGNAL(clicked()), SLOT(delCurrent()));

}

void GTab::changeGroupList(QList<Group*> *gList)
{
    for (int i = listWidget_->count(); i > 0; i--) {
        bool flg = false;
        foreach(Group * g, (*gList)) {
            SearchItem *item = (SearchItem*)listWidget_->item(i - 1);
            if (!item->url().isEmpty()) {
                flg = true;
                break;
            } else if (item->method().group->name() == g->name()) {
                flg = true;
                break;
            }
        }
        if (!flg) {
            SearchItem *item = (SearchItem*)listWidget_->takeItem(i - 1);
            delete item;
        }
    }
}


void GTab::upCurrent()
{
    int row = listWidget_->currentRow();
    SearchItem *item = (SearchItem*)listWidget_->takeItem(row);

    listWidget_->insertItem(row - 1, item);
    listWidget_->setCurrentItem(item);
}

void GTab::downCurrent()
{
    int row = listWidget_->currentRow();
    SearchItem *item = (SearchItem*)listWidget_->takeItem(row);

    listWidget_->insertItem(row + 1, item);
    listWidget_->setCurrentItem(item);
}


void GTab::delCurrent()
{
    int row = listWidget_->currentRow();
    SearchItem *item = (SearchItem*)listWidget_->takeItem(row);

    delete item;
}

void GTab::delAll()
{
    if (QMessageBox::question(this, "qolibri",                        
                              tr("Are you sure you want to remove all list?"),
                              QMessageBox::Yes | QMessageBox::No) ==
            QMessageBox::Yes) {  
        listWidget_->clear();
    }
}

void GTab::viewCurrent()
{
    SearchItem *item = (SearchItem*)listWidget_->currentItem();
    if (!item->url().isEmpty()) {
        emit webRequested(item->searchStr(), item->url());
    } else {
        emit searchRequested(item->searchStr(), item->method());
    }
}

void GTab::pasteCurrent()
{
    SearchItem *item = (SearchItem*)listWidget_->currentItem();
    emit pasteRequested(item->searchStr(), item->method());
}

void GTab::popupMenu(const QPoint &pos)
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
                   tr("&Set to current search condition"),
                   this, SLOT(pasteCurrent()));
    menu.addAction(QIcon(":images/delete.png"), tr("&Delete"),
                   this, SLOT(delCurrent()));
    menu.addAction(QIcon(":images/delete.png"), tr("Delete &All"),
                   this, SLOT(delAll()));
    menu.exec(listWidget_->viewport()->mapToGlobal(pos));
}

void GTab::resetButtons()
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


GroupTab::GroupTab(QWidget *parent)
    : QWidget(parent), groupList(NULL), group(NULL)
{
    QVBoxLayout *v = new QVBoxLayout();

    groupWidget_ = new QListWidget(this);
    groupWidget_->setResizeMode(QListView::Adjust);
    connect(groupWidget_, SIGNAL(currentRowChanged(int)),
            SLOT(changeGroup(int)));
    bookWidget_ = new BookWidget(group, this);
    bookWidget_->hideDelButton();
    bookWidget_->hideAddButton();
    bookWidget_->hideEditButton();
    bookWidget_->hideGroupName();
    bookWidget_->bookListWidget()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bookWidget_, SIGNAL(bookViewRequested(Book*)),
            SIGNAL(bookViewRequested(Book*)));
    connect(bookWidget_, SIGNAL(fontViewRequested(Book*)),
            SIGNAL(fontViewRequested(Book*)));
    connect(bookWidget_, SIGNAL(rowChanged(int)),
            SIGNAL(bookChanged(int)));
    v->addWidget(new QLabel(tr("Groups:")));
    v->addWidget(groupWidget_);
    v->addSpacing(10);
    v->addWidget(new QLabel(tr("Books:")));
    v->addWidget(bookWidget_);

    v->setMargin(0);
    v->setSpacing(0);

    QVBoxLayout *v2 = new QVBoxLayout();
    v2->addLayout(v);
    v2->setMargin(0);
    v2->setSpacing(0);
    setLayout(v2);

    connect(bookWidget_->bookListWidget(),
            SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(popupMenu(QPoint)));
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
    disconnect(groupWidget_, SIGNAL(currentRowChanged(int)),
               this, SLOT(changeGroup(int)));
    group = (*groupList)[index];
    groupWidget_->setCurrentRow(index);
    bookWidget_->initBook(group);
    connect(groupWidget_, SIGNAL(currentRowChanged(int)),
            SLOT(changeGroup(int)));
}

void GroupTab::changeGroupList(QList<Group*> *gList)
{
    groupList = gList;
    bookWidget_->initBook(NULL);
    disconnect(groupWidget_, SIGNAL(currentRowChanged(int)),
               this, SLOT(changeGroup(int)));
    groupWidget_->clear();
    foreach(Group * g, (*groupList)) {
        groupWidget_->addItem(g->name());
    }
    groupWidget_->setCurrentRow(-1);
    connect(groupWidget_, SIGNAL(currentRowChanged(int)),
            SLOT(changeGroup(int)));
    int h = groupWidget_->sizeHintForRow(1);
    int c = groupList->count() <= 8 ? groupList->count() : 8;
    if (h == -1) h = 16;

    groupWidget_->setFixedHeight((h * c) + 5);

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
                                   tr("Show book &Information"));
    QAction *rAct = menu.addAction(QIcon(":images/bookopen.png"),
                                   tr("&Read book"));
    QAction *aAct = menu.addAction(QIcon(":images/bookopen.png"),
                                   tr("Read &All cata"));
    QAction *fAct = menu.addAction(QIcon(":images/font3.png"),
                                   tr("&Font setting"));
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
void MarkTab::addMark(const QString &str, const QString &url)
{

    listWidget_->insertItem(0, new SearchItem(str, url));
}


void HistoryTab::addHistory(const QString &str, const SearchMethod &method,
                            int max_hist)
{
    if (listWidget_->count() >= max_hist) {
        QListWidgetItem *rem = listWidget_->takeItem(listWidget_->count() - 1);
        delete rem;
    }
    listWidget_->insertItem(0, new SearchItem(str, method));
}

GroupDock::GroupDock(QWidget *parent, Model *model_)
#if defined (Q_WS_MAC)
    : QTabWidget(parent)
#else
    : QWidget(parent)
#endif
    , model(model_)
{
    connect(this, SIGNAL(groupChanged(int)), model, SLOT(setGroupIndex(int)));
    connect(model, SIGNAL(groupIndexChanged(int)), SLOT(changeGroup(int)));
    connect(this, SIGNAL(bookChanged(int)), model, SLOT(setBookIndex(int)));
    connect(model, SIGNAL(bookIndexChanged(int)), SLOT(setCurrentBook(int)));
    connect(model, SIGNAL(dictionaryGroupsChanged()), SLOT(changeGroupList()));

#if defined (Q_WS_MAC)
    QTabWidget* tabWidget = this;
#else
    tabWidget = new QTabWidget(this);
#endif
    groupTab = new GroupTab(this);
    tabWidget->addTab(groupTab, QIcon(":/images/group.png"), tr("Group"));
    connect(groupTab, SIGNAL(bookViewRequested(Book*)),
            SIGNAL(bookViewRequested(Book*)));
    connect(groupTab, SIGNAL(fontViewRequested(Book*)),
            SIGNAL(fontViewRequested(Book*)));
    connect(groupTab, SIGNAL(menuRequested()), SIGNAL(menuRequested()));
    connect(groupTab, SIGNAL(fullRequested()), SIGNAL(fullRequested()));
    connect(groupTab, SIGNAL(groupChanged(int)), SIGNAL(groupChanged(int)));
    connect(groupTab, SIGNAL(bookChanged(int)), SIGNAL(bookChanged(int)));

    markTab = new MarkTab(this);
    connect(markTab, SIGNAL(searchRequested(QString, SearchMethod)),
            SIGNAL(searchRequested(QString, SearchMethod)));
    connect(markTab, SIGNAL(webRequested(QString, QString)),
            SIGNAL(webRequested(QString, QString)));
    connect(markTab, SIGNAL(pasteRequested(QString, SearchMethod)),
            SIGNAL(pasteRequested(QString, SearchMethod)));
    tabWidget->addTab(markTab, QIcon(":/images/bookmark.png"), tr("Mark"));

    historyTab = new HistoryTab(this);
    connect(historyTab, SIGNAL(searchRequested(QString, SearchMethod)),
            SIGNAL(searchRequested(QString, SearchMethod)));
    connect(historyTab, SIGNAL(webRequested(QString, QString)),
            SIGNAL(webRequested(QString, QString)));
    connect(historyTab, SIGNAL(pasteRequested(QString, SearchMethod)),
            SIGNAL(pasteRequested(QString, SearchMethod)));
    tabWidget->addTab(historyTab, QIcon(":/images/history.png"), tr("History"));

#if defined (Q_WS_MAC)
    setWindowFlags(Qt::Drawer);
#else
    QHBoxLayout *h = new QHBoxLayout();
    h->setAlignment(0);
    h->setMargin(0);
    h->setSpacing(0);
    h->addWidget(tabWidget);
    setLayout(h);

    setWindowFlags(Qt::Tool);
#endif

    changeGroupList();
    //connect(this, SIGNAL(closed()), parent, SLOT(closedDock()));
}
