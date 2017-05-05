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

#include <QDockWidget>
#include <QListWidgetItem>
#include <QPushButton>

#include "bookwidget.h"
#include "method.h"
#include "model.h"

class QListWidget;
class QTabWidget;
class QComboBox;
class QHBoxLayout;

class SearchItem : public QListWidgetItem
{
public:
    SearchItem(const QString &str, const SearchMethod &method);
    SearchItem(const QString &name, const QString &url);
    inline QString searchStr() const
    {
        return searchStr_;
    }
    inline SearchMethod method() const
    {
        return method_;
    }
    inline QString url() const
    {
        return url_;
    }

protected:
    QString searchStr_;
    QString url_;
    SearchMethod method_;
};

class GTab : public QWidget
{
    Q_OBJECT
public:
    GTab(QWidget *parent);
    void changeGroupList(QList<Group*> *gList);
    QListWidget *listWidget() { return listWidget_; }

signals:
    void searchRequested(const QString &name, const SearchMethod &method );
    void webRequested(const QString &name, const QString &url );
    void pasteRequested(const QString &name, const SearchMethod &method );

private slots:
    void upCurrent();
    void downCurrent();
    void delCurrent();
    void delAll();
    void viewCurrent();
    void pasteCurrent();
    void popupMenu(const QPoint &pos);
    void resetButtons();

protected:
    QListWidget *listWidget_;
    QHBoxLayout *buttonLayout;
    QPushButton *upButton;
    QPushButton *downButton;
    QPushButton *delButton;
    QPushButton *viewButton;
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
    inline QListWidget *groupWidget() const
    {
        return groupWidget_;
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
    QListWidget *groupWidget_;
    BookWidget *bookWidget_;
};

class MarkTab : public GTab
{
    Q_OBJECT
public:
    MarkTab(QWidget *parent) : GTab(parent) {}
    void addMark(const QString &str, const SearchMethod &method);
    void addMark(const QString &str, const QString &url);

};

class HistoryTab : public GTab
{
    Q_OBJECT
public:
    HistoryTab(QWidget *parent) : GTab(parent)
    {
        upButton->hide();
        downButton->hide();
    }
    void addHistory(const QString &str, const SearchMethod &method,
                    int max_hist);

};

#if defined (Q_WS_MAC)
class GroupDock : public QTabWidget
#else
class GroupDock : public QDockWidget
#endif
{
    Q_OBJECT
public:
    GroupDock(QWidget *parent, Model *model_);

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

    inline QListWidget *groupWidget() const
    {
        return groupTab->groupWidget();
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
    inline void addMark(const QString &str, const QString &url)
    {
        markTab->addMark(str, url);
    }
    inline void addHistory(const QString &str, const SearchMethod &method,
                           int max_hist)
    {
        historyTab->addHistory(str, method, max_hist);
    }
    inline void changeTabToMark()
    {
        changeCurrentTab(markTab);
    }

public slots:
    void changeGroupList()
    {
        groupTab->changeGroupList(&model->groupList);
        markTab->changeGroupList(&model->groupList);
        historyTab->changeGroupList(&model->groupList);
    }

    inline void setCurrentBook(int index)
    {
        groupTab->bookWidget()->setCurrentRow(index);
    }

    inline void changeGroup(int index)
    {
        groupTab->changeGroupNoSignal(index);
    }

protected:
    void closeEvent(QCloseEvent*)
    {
        emit closed();
    }

signals:
    void closed();
    // from Gtab
    void searchRequested(const QString &name, const SearchMethod &method );
    void webRequested(const QString &name, const QString &url );
    void pasteRequested(const QString &name, const SearchMethod &method );
    // from GroupTab
    void groupChanged(int grp);
    void bookChanged(int index);
    void bookViewRequested(Book *book);
    void fontViewRequested(Book *book);
    void menuRequested();
    void fullRequested();

private:
#if !defined (Q_WS_MAC)
    QTabWidget * tabWidget;
#endif
    GroupTab * groupTab;
    MarkTab *markTab;
    HistoryTab *historyTab;
    Model *model;
};

#endif

