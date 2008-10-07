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

#include "book.h"
#include "ebook.h"
#include "bookwidget.h"

BookWidget::BookWidget(Group *grp, QWidget *parent)
    : QWidget(parent)
{
    groupNameLabel = new QLabel(this);
    groupNameLabel->setFrameShape(QFrame::StyledPanel);
    groupNameLabel->setAlignment(Qt::AlignCenter);
    QHBoxLayout *h = new QHBoxLayout();
    h->setMargin(0);
    h->addStretch();
    upButton = new QPushButton(this);
    upButton->setIcon(QIcon(":images/uparrow.png"));
    connect(upButton, SIGNAL(clicked()), SLOT(upItem()));
    downButton = new QPushButton(this);
    downButton->setIcon(QIcon(":images/downarrow.png"));
    connect(downButton, SIGNAL(clicked()), SLOT(downItem()));
    h->addWidget(upButton);
    h->addWidget(downButton);
    editButton_ = new QPushButton(this);
    editButton_->setIcon(QIcon(":images/edit.png"));
    h->addWidget(editButton_);
    addButton_ = new QPushButton(this);
    addButton_->setIcon(QIcon(":images/add.png"));
    h->addWidget(addButton_);

    //connect(editButton_, SIGNAL(clicked()), this, SLOT(editItem()));
    delButton = new QPushButton(this);
    delButton->setIcon(QIcon(":images/delete.png"));
    h->addWidget(delButton);
    connect(delButton, SIGNAL(clicked()), SLOT(delItem()));
    fontButton = new QPushButton(this);
    fontButton->setIcon(QIcon(":images/font3.png"));
    h->addWidget(fontButton);
    connect(fontButton, SIGNAL(clicked()), SLOT(setFont()));
    viewButton = new QPushButton(this);
    viewButton->setIcon(QIcon(":images/find_l.png"));
    h->addWidget(viewButton);
    connect(viewButton, SIGNAL(clicked()), SLOT(viewItem()));
    bookListWidget_ = new QListWidget(this);
    connect(bookListWidget_, SIGNAL(itemSelectionChanged()),
            SLOT(changeRow()));
    connect(bookListWidget_, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            SLOT(viewItem()));
    connect(bookListWidget_, SIGNAL(itemChanged(QListWidgetItem*)),
            SLOT(changeName(QListWidgetItem*)));
    connect(bookListWidget_, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            SLOT(editItem(QListWidgetItem*)));
    connect(bookListWidget_,
            SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem*)),
            SLOT(changeSelect(QListWidgetItem *, QListWidgetItem*)));

    QVBoxLayout *v = new QVBoxLayout;
    v->setMargin(0);
    v->setSpacing(0);
    if (groupNameLabel) v->addWidget(groupNameLabel);
    v->addWidget(bookListWidget_);
    v->addLayout(h);
    setLayout(v);
    initBook(grp);


    resetButtons();
}

void BookWidget::initBook(Group *grp)
{
    group = grp;
    if (group == NULL) {
        groupNameLabel->clear();
        int cnt = bookListWidget_->count();
        disconnect(bookListWidget_, SIGNAL(itemSelectionChanged()),
                   this, SLOT(changeRow()));
        for (int i = 0; i < cnt; i++) {
            bookListWidget_->takeItem(0);
        }
        connect(bookListWidget_, SIGNAL(itemSelectionChanged()),
                SLOT(changeRow()));
    } else {
        groupNameLabel->setText(group->name());
        int cnt = bookListWidget_->count();
        //bookListWidget_->clear();
        disconnect(bookListWidget_, SIGNAL(itemSelectionChanged()),
                   this, SLOT(changeRow()));
        for (int i = 0; i < cnt; i++) {
            bookListWidget_->takeItem(0);
        }
        connect(bookListWidget_, SIGNAL(itemSelectionChanged()),
                SLOT(changeRow()));
        foreach(Book * b, group->bookList()) {
            bookListWidget_->addItem(b);
        }
        setCurrentRow(0);
    }
}

bool BookWidget::addBook(const QString &name, BookType btype,
                         const QString &path, int subbook)
{
    foreach(Book * b, group->bookList()) {
        if (b->path() == path && b->bookNo() == subbook) {
            return false;
        }
    }
    Book *book = new Book(name, btype, path, subbook, true);
    book->setData(Qt::CheckStateRole, 1);
    book->setCheckState(Qt::Checked);
    group->addBook(book);
    bookListWidget_->addItem(book);

    return true;
}


void BookWidget::resetButtons()
{
    int num = bookListWidget_->selectedItems().count();
    int row = currentRow();
    int cnt = bookListWidget_->count();

    upButton->setEnabled(num == 1 && row > 0 );
    downButton->setEnabled(num == 1 && ( cnt - row) > 1  &&  row >= 0 );
    delButton->setEnabled(row >= 0);
    viewButton->setEnabled(num == 1 && row >= 0);
    fontButton->setEnabled(num == 1 && row >= 0);
    editButton_->setEnabled(num == 1 && row >= 0);
    emit rowChanged(row);
}

void BookWidget::upItem()
{
    int row = currentRow();
    Book *book = (Book*)bookListWidget_->takeItem(row);

    bookListWidget_->insertItem(row - 1, book);
    bookListWidget_->setCurrentItem(book);
    group->takeBook(row);
    group->insBook(row - 1, book);
}

void BookWidget::downItem()
{
    int row = currentRow();
    Book *book = (Book*)bookListWidget_->takeItem(row);

    bookListWidget_->insertItem(row + 1, book);
    bookListWidget_->setCurrentItem(book);
    group->takeBook(row);
    group->insBook(row + 1, book);
}

void BookWidget::delItem()
{
    QList <QListWidgetItem *> items = bookListWidget_->selectedItems();
    foreach( QListWidgetItem *i, items) {
        int r = bookListWidget_->row(i);
        Book *book = (Book *)bookListWidget_->takeItem(r);
        group->takeBook(r);
        delete book;
    }
}

