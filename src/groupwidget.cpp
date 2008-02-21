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
#include "groupwidget.h"
#include "ebook.h"

GroupWidget::GroupWidget(QList <Group*> *group, QWidget *parent)
    : QWidget(parent), groupList(group)
{
    QHBoxLayout *h1 = new QHBoxLayout();

    {
        QLabel *l = new QLabel(tr("Group: "));
        addGroupEdit = new QLineEdit(this);
        connect(addGroupEdit, SIGNAL(editingFinished()),
                this, SLOT(createGroup()));
        addButton = new QPushButton(tr("Create"), this);
        connect(addGroupEdit, SIGNAL(textChanged(QString)),
                this, SLOT(changeAddGroup(QString)));
        connect(addButton, SIGNAL(clicked()), this, SLOT(createGroup()));
        addButton->setEnabled(false);
        h1->addWidget(l);
        h1->addWidget(addGroupEdit);
        h1->addWidget(addButton);
    }
    QHBoxLayout *h2 = new QHBoxLayout();
    {
        h2->setMargin(0);
        upButton = new QPushButton(this);
        upButton->setIcon(QIcon(":images/uparrow.png"));
        downButton = new QPushButton(this);
        downButton->setIcon(QIcon(":images/downarrow.png"));
        h2->addStretch();
        h2->addWidget(upButton);
        h2->addWidget(downButton);
        editButton = new QPushButton(this);
        editButton->setIcon(QIcon(":images/edit.png"));
        h2->addWidget(editButton);
        delButton = new QPushButton(this);
        delButton->setIcon(QIcon(":images/delete.png"));
        h2->addWidget(delButton);
    }
    groupListWidget = new QListWidget(this);
    QVBoxLayout *v = new QVBoxLayout;
    v->setMargin(0);
    v->setSpacing(0);
    v->addLayout(h1);
    v->addWidget(groupListWidget);
    v->addLayout(h2);
    setLayout(v);
    //qDebug() << "BookWidget 3";
    connect(upButton, SIGNAL(clicked()), this, SLOT(upItem()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(downItem()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editItem()));
    connect(delButton, SIGNAL(clicked()), this, SLOT(delItem()));
    connect(groupListWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(changeRow(int)));
    connect(groupListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(changeName(QListWidgetItem*)));
    connect(groupListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(editItem(QListWidgetItem*)));
    connect(groupListWidget,
            SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem*)),
            this, SLOT(changeSelect(QListWidgetItem *, QListWidgetItem*)));
    initGroup();
    resetButtons();
}

void GroupWidget::initGroup()
{
    int cnt = groupListWidget->count();
    for (int i = 0; i < cnt; i++) {
        groupListWidget->takeItem(0);
    }
    for (int i = 1; i < groupList->count(); i++) {
        groupListWidget->addItem((*groupList)[i]);
    }
    if ( groupListWidget->count() > 0 ) {
        groupListWidget->setCurrentRow(0);
        emit rowChanged(0);
        //qDebug() << "emit changeRow";
    }
}

void GroupWidget::resetButtons()
{
    int row = currentRow();
    int cnt = groupListWidget->count();

    upButton->setEnabled(row > 0 );
    downButton->setEnabled((( cnt - row) > 1 ) && ( row >= 0 ));
    editButton->setEnabled(row >= 0);
    if (delButton) delButton->setEnabled(row >= 0);
    emit rowChanged(row);
}

void GroupWidget::upItem()
{
    int row = currentRow();
    Group *grp = (Group*)groupListWidget->takeItem(row);

    groupListWidget->insertItem(row - 1, grp);
    groupListWidget->setCurrentItem(grp);
    groupList->takeAt(row + 1);
    groupList->insert(row, grp);
}

void GroupWidget::downItem()
{
    int row = currentRow();
    Group *grp = (Group*)groupListWidget->takeItem(row);

    groupListWidget->insertItem(row + 1, grp);
    groupListWidget->setCurrentItem(grp);
    groupList->takeAt(row + 1);
    groupList->insert(row + 2, grp);
}

void GroupWidget::delItem()
{
    int row = currentRow();
    Group *grp = (Group *)groupListWidget->takeItem(row);

    groupList->takeAt(row + 1);
    delete grp;
//    resetButtons();
}

void GroupWidget::changeAddGroup(const QString &str)
{
    if (str.isEmpty() || str == "/")
        addButton->setEnabled(false);
    else
        addButton->setEnabled(true);
}

void GroupWidget::createGroup()
{
    if (addGroupEdit->text().isEmpty()){
        return;
    }
    Group *grp = new Group(addGroupEdit->text());

    *groupList << grp;
    groupListWidget->addItem(grp);
    groupListWidget->setCurrentItem(grp);
    addGroupEdit->clear();
}

