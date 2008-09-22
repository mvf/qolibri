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
#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include <QListWidget>
#include "book.h"

class QLineEdit;
class QPushButton;
class QDialogButtonBox;
class QLabel;

class GroupWidget : public QWidget
{
    Q_OBJECT
public:
    GroupWidget(QList <Group*> *group, QWidget *parent);
    inline int currentRow() const
    {
        return groupListWidget->currentRow();
    }
    inline Group *currentGroup() const
    {
        return (*groupList)[currentRow()];
    }
    inline QListWidgetItem* currentItem() const
    {
        return groupListWidget->currentItem();
    }

private slots:
    void initGroup();
    void changeAddGroup(const QString &str);
    void createGroup();
    void upItem();
    void downItem();
    void delItem();
    void changeRow(int)
    {
        resetButtons();
    }
    void editItem(QListWidgetItem *item)
    {
        groupListWidget->openPersistentEditor(item);
        groupListWidget->editItem(item);
    }
    void editItem()
    {
        editItem(currentItem());
    }

    void changeName(QListWidgetItem *item)
    {
        Group *group = (Group *)item;

        group->setName(item->text());
    }

    void changeSelect(QListWidgetItem*, QListWidgetItem *prev)
    {
        groupListWidget->closePersistentEditor(prev);
    }


signals:
    void rowChanged(int row);

private:
    void resetButtons();

    QList <Group*> *groupList;
    QListWidget *groupListWidget;
    QLineEdit *addGroupEdit;
    QPushButton *addButton;
    QPushButton *upButton;
    QPushButton *downButton;
    QPushButton *delButton;
    QPushButton *editButton;
};

#endif

