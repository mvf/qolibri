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


BookWidget::BookWidget(Group *grp, bool bDel, bool bView, bool bFont,
                       bool titleDisp, bool bEdit, QWidget *parent)
    : QWidget(parent)
{
    groupNameEdit = NULL;
    if (titleDisp) {
        groupNameEdit = new QLineEdit(this);
        connect(groupNameEdit, SIGNAL(textChanged(QString)),
                this, SLOT(changeGroupName(QString)));
    }
    QHBoxLayout *h = new QHBoxLayout();
    h->setMargin(0);
    h->addStretch();
    upButton = new QPushButton(this);
    upButton->setIcon(QIcon(":images/uparrow.png"));
    connect(upButton, SIGNAL(clicked()), this, SLOT(upItem()));
    downButton = new QPushButton(this);
    downButton->setIcon(QIcon(":images/downarrow.png"));
    connect(downButton, SIGNAL(clicked()), this, SLOT(downItem()));
    h->addWidget(upButton);
    h->addWidget(downButton);
    editButton = NULL;
    if (bEdit) {
        editButton = new QPushButton(this);
        editButton->setIcon(QIcon(":images/edit.png"));
        h->addWidget(editButton);
        connect(editButton, SIGNAL(clicked()), this, SLOT(editItem()));
    }
    delButton = NULL;
    if (bDel) {
        delButton = new QPushButton(this);
        delButton->setIcon(QIcon(":images/delete.png"));
        h->addWidget(delButton);
        connect(delButton, SIGNAL(clicked()), this, SLOT(delItem()));
    }
    fontButton = NULL;
    if (bFont) {
        fontButton = new QPushButton(this);
        fontButton->setIcon(QIcon(":images/font3.png"));
        h->addWidget(fontButton);
        connect(fontButton, SIGNAL(clicked()), this, SLOT(setFont()));
    }
    viewButton = NULL;
    if (bView) {
        viewButton = new QPushButton(this);
        viewButton->setIcon(QIcon(":images/find_l.png"));
        h->addWidget(viewButton);
        connect(viewButton, SIGNAL(clicked()), this, SLOT(viewItem()));
    }
    bookListWidget_ = new QListWidget(this);
    connect(bookListWidget_, SIGNAL(currentRowChanged(int)),
            this, SLOT(changeRow(int)));
    if (bView) {
        connect(bookListWidget_, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                this, SLOT(viewItem()));
    }
    if (bEdit) {
        connect(bookListWidget_, SIGNAL(itemChanged(QListWidgetItem*)),
                this, SLOT(changeName(QListWidgetItem*)));
        connect(bookListWidget_, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                this, SLOT(editItem(QListWidgetItem*)));
        connect(bookListWidget_,
                SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem*)),
                this, SLOT(changeSelect(QListWidgetItem *, QListWidgetItem*)));
    }

    QVBoxLayout *v = new QVBoxLayout;
    v->setMargin(0);
    v->setSpacing(0);
    if (groupNameEdit) v->addWidget(groupNameEdit);
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
        if (groupNameEdit) groupNameEdit->clear();
        int cnt = bookListWidget_->count();
        disconnect(bookListWidget_, SIGNAL(currentRowChanged(int)),
                   this, SLOT(changeRow(int)));
        for (int i = 0; i < cnt; i++) {
            bookListWidget_->takeItem(0);
        }
        connect(bookListWidget_, SIGNAL(currentRowChanged(int)),
                this, SLOT(changeRow(int)));
    } else {
        if (groupNameEdit) groupNameEdit->setText(group->name());
        int cnt = bookListWidget_->count();
        //bookListWidget_->clear();
        disconnect(bookListWidget_, SIGNAL(currentRowChanged(int)),
                   this, SLOT(changeRow(int)));
        for (int i = 0; i < cnt; i++) {
            bookListWidget_->takeItem(0);
        }
        connect(bookListWidget_, SIGNAL(currentRowChanged(int)),
                this, SLOT(changeRow(int)));
        foreach(Book * b, group->bookList()) {
            bookListWidget_->addItem(b);
        }
        setCurrentRow(0);
    }
}

bool BookWidget::addBook(const QString &name, const QString &path, int subbook)
{
    foreach(Book * b, group->bookList()) {
        if (b->path() == path && b->bookNo() == subbook) {
            return false;
        }
    }
    Book *book = new Book(name, path, subbook, true);
    book->setData(Qt::CheckStateRole, 1);
    book->setCheckState(Qt::Checked);
    group->addBook(book);
    bookListWidget_->addItem(book);

    return true;
}


void BookWidget::resetButtons()
{
    int row = currentRow();
    int cnt = bookListWidget_->count();

    upButton->setEnabled(row > 0 );
    downButton->setEnabled((( cnt - row) > 1 ) && ( row >= 0 ));
    if (delButton) delButton->setEnabled(row >= 0);
    if (viewButton) viewButton->setEnabled(row >= 0);
    if (fontButton) fontButton->setEnabled(row >= 0);
    if (editButton) editButton->setEnabled(row >= 0);
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
    int row = currentRow();
    Book *book = (Book *)bookListWidget_->takeItem(row);

    group->takeBook(row);
    delete book;
    //qDebug() << "BookWidget::delItem(int row) " << row;
    //resetButtons();
}

