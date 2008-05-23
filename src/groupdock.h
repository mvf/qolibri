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
#ifndef GROUPDOCK_H
#define GROUPDOCK_H

#include <QListWidgetItem>

#include "bookwidget.h"
#include "method.h"

class QPushButton;
class QListWidget;
class QTabWidget;
class QComboBox;

class SearchItem : public QListWidgetItem
{
public:
    SearchItem(const QString &str, const SearchMethod &method);
    inline QString searchStr() const
    {
        return searchStr_;
    }
    inline SearchMethod method() const
    {
        return method_;
    }

protected:
    QString searchStr_;
    SearchMethod method_;
};

class GroupTab : public QWidget
{
    Q_OBJECT
public:
    GroupTab(QWidget *parent);
    inline BookWidget *bookWidget() const
    {
        return bookWidget_;
    }
    inline QComboBox *groupCombo() const
    {
        return groupCombo_;
    }
    void changeGroupList(QList<Group*> *gList);
    void changeGroupNoSignal(int index);

signals:
    void groupChanged(int grp);
    void bookChanged(int index);
    void bookViewRequested(Book *book);
    void fontViewRequested(Book *book);
    void menuRequested();
    void fullRequested();

private slots:
    void changeGroup(int index);
    void popupMenu(const QPoint &pos);

private:
    QList<Group*> *groupList;
    Group *group;
    QComboBox *groupCombo_;
    BookWidget *bookWidget_;
};

class MarkTab : public QWidget
{
    Q_OBJECT
public:
    MarkTab(QWidget *parent);
    void addMark(const QString &str, const SearchMethod &method);
    inline QListWidget* listWidget() const
    {
        return listWidget_;
    }
    void changeGroupList(QList<Group*> *gList);

signals:
    void searchRequested(const QString &name, const SearchMethod &method );
    void pasteRequested(const QString &name, const SearchMethod &method );

private slots:
    void delCurrent();
    void viewCurrent();
    void upCurrent();
    void downCurrent();
    void pasteCurrent();
    void delAll();
    void resetButtons();
    void popupMenu(const QPoint &pos);

private:
    QListWidget *listWidget_;
    QPushButton *upButton;
    QPushButton *downButton;
    QPushButton *delButton;
    QPushButton *viewButton;
};

class HistoryTab : public QWidget
{
    Q_OBJECT
public:
    HistoryTab(QWidget *parent);
    void addHistory(const QString &str, const SearchMethod &method);
    inline QListWidget *listWidget() const
    {
        return listWidget_;
    }
    void changeGroupList(QList<Group*> *gList);

signals:
    void searchRequested(const QString &name, const SearchMethod &method );
    void pasteRequested(const QString &name, const SearchMethod &method );

private slots:
    void delCurrent();
    void viewCurrent();
    void resetButtons();
    void pasteCurrent();
    void delAll();
    void popupMenu(const QPoint &pos);

private:
    QListWidget *listWidget_;
    QPushButton *delButton;
    QPushButton *viewButton;
};

#if defined (Q_WS_MAC)
class GroupDock : public QTabWidget
#else
class GroupDock : public QWidget
#endif
{
    Q_OBJECT
public:
    GroupDock(QWidget *parent);

#if defined (Q_WS_MAC)
    inline void changeCurrentTab(QWidget *w)
    {
        setCurrentWidget(w);
    }
#else
    inline void changeCurrentTab(QWidget *w)
    {
        tabWidget->setCurrentWidget(w);
    }
#endif
    inline QListWidget *bookListWidget() const
    {
        return groupTab->bookWidget()->bookListWidget();
    }

    inline void setCurrentBook(int index)
    {
        groupTab->bookWidget()->setCurrentRow(index);
    }

    inline void changeGroup(int index)
    {
        groupTab->changeGroupNoSignal(index);
    }
    inline void changeGroupList(QList<Group*> *group_list)
    {
        groupTab->changeGroupList(group_list);
        markTab->changeGroupList(group_list);
        historyTab->changeGroupList(group_list);
    }
    inline QComboBox *groupCombo() const
    {
        return groupTab->groupCombo();
    }
    inline QListWidget *historyListWidget() const
    {
        return historyTab->listWidget();
    }
    inline QListWidget *markListWidget() const
    {
        return markTab->listWidget();
    }
    inline void addMark(const QString &str, const SearchMethod &method)
    {
        markTab->addMark(str, method);
    }
    inline void addHistory(const QString &str, const SearchMethod &method)
    {
        historyTab->addHistory(str, method);
    }
    inline void changeTabToMark()
    {
        changeCurrentTab(markTab);
    }

protected:
    void closeEvent(QCloseEvent*)
    {
        emit closed();
    }

signals:
    void closed();

private:
#if !defined (Q_WS_MAC)
    QTabWidget * tabWidget;
#endif
    GroupTab * groupTab;
    MarkTab *markTab;
    HistoryTab *historyTab;
};

#endif

