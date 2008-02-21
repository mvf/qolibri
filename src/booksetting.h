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
#ifndef BOOKSETTING_H
#define BOOKSETTING_H

#include <QDialog>
#include <QLabel>
#include <QEventLoop>

class QLineEdit;
class QPushButton;
class QDialogButtonBox;
class QKeyEvent;

class Book;
class Group;
class BookWidget;
class GroupWidget;

class BookSetting : public QDialog
{
    Q_OBJECT
public:
    BookSetting(const QList<Group*> &grp, QWidget *parent);
    QList <Group*> groupList()
    {
        return groupList_;
    }

private slots:
    void searchBook();
    void cancelSearch()
    {
        findStop = true;
    }

    void searchPathChanged(const QString &str);
    void addBook();
    void changeGroupSelection(int row);
    void changeBookSelection(int row);
    void setPath();

protected:
    void keyPressEvent(QKeyEvent*);

private:
    void findCategory(const QString &name);
    inline bool checkStop()
    {
        QEventLoop event;

        event.processEvents();
        return findStop;
    }

    bool findStop;
    QList <Group*> groupList_;
    QLineEdit *searchPath;
    QLabel *findPaths;

    QPushButton *pathButton;
    QPushButton *searchButton;
    QPushButton *cancelButton;
    QPushButton *selButton;

    BookWidget *allDicWidget;
    BookWidget *dicWidget;
    GroupWidget *groupWidget;

    QDialogButtonBox *buttonBox;
    QStringList bookDirs;
};

#endif

