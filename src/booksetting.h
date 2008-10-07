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
#include <QLineEdit>

class QLineEdit;
class QPushButton;
class QDialogButtonBox;
class QTabWidget;
class QKeyEvent;

class Book;
class Group;
class BookWidget;
class GroupWidget;

class WebSetting : public QDialog
{
    Q_OBJECT
public:
    WebSetting(QWidget *parent, const QString &name, const QString &url);

public:

    QString name() { return nameEdit_->text(); }
    QString url() { return urlEdit_->text(); }

private slots:
    void setOkButton(const QString &text);

private:
    QLineEdit *nameEdit_;
    QLineEdit *urlEdit_;
    QDialogButtonBox *buttonBox_;

};

class EpwingFileSetting : public QDialog
{
    Q_OBJECT
public:
    EpwingFileSetting(QWidget *parent, const QString &name, const QString &path,
                      int book_no);

public:

    QString name() { return nameEdit_->text(); }
    QString path() { return pathEdit_->text(); }
    int bookNo() { return bookNoEdit_->text().toInt(); }

private slots:
    void setOkButton(const QString &text);

private:
    QLineEdit *nameEdit_;
    QLineEdit *pathEdit_;
    QLineEdit *bookNoEdit_;
    QDialogButtonBox *buttonBox_;

};


class BookSetting : public QDialog
{
    Q_OBJECT
public:
    BookSetting(Group *lbook, Group *wsite,
                const QList<Group*> &grp, QWidget *parent);
    QList <Group*> groupList()
    {
        return groupList_;
    }
    Group* localBooks()
    {
        return localBooks_;
    }
    Group* webSites()
    {
        return webSites_;
    }

private slots:
    void searchBook();
    void cancelSearch()
    {
        findStop = true;
    }

    void changeTab(int index);
    void searchPathChanged(const QString &str);
    void webNameChanged(const QString &str);
    void webSiteChanged(const QString &str);
    void editItem();
    void addItem();
    void addBook();
    void addWebSite();
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
    Group* localBooks_;
    Group* webSites_;
    QLineEdit *searchPath;
    QLineEdit *webNameEdit;
    QLineEdit *webSiteEdit;
    QLabel *findPaths;
    QTabWidget *tab;

    QPushButton *pathButton;
    QPushButton *searchButton;
    QPushButton *cancelButton;
    QPushButton *selButton;
    QPushButton *webAddButton;

    BookWidget *allDicWidget;
    BookWidget *dicWidget;
    GroupWidget *groupWidget;

    QDialogButtonBox *buttonBox;
    QStringList bookDirs;
};

#endif

