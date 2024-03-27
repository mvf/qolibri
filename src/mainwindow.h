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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "method.h"

#include <QMainWindow>
#include <QAction>
#include <QClipboard>

class QComboBox;
class QLineEdit;
class QLabel;
class QMediaPlayer;
class QSpinBox;
class QSettings;
class QTimer;
class QPushButton;
class QMenu;

class GroupDock;
class BookView;
class Model;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Model *, const QString &s_text);

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void searchFinished();
    void nowBusy(bool enable);
    void viewFontChanged(const QFont &font);
    void bookTreeToggled(bool visible);

public slots:
    //void searchClientText(const QString &str);

private slots:
    void connectClipboard(bool enable);
    void viewInfo(Book *book);
    void viewPseudoSearch(SearchDirection direction);
    void viewSearch(const QString &str, const SearchMethod &method);
    void viewSearch(SearchDirection d, const QString &str);
    void viewWeb(const QString &name, const QString &url);
    void doSearch();                            // by Menu
    void showStatus(const QString &str = QString());
    void showTabInfo(int index);
    void pasteMethod(const QString &str, const SearchMethod &method);
    void pasteSearchText(const QString&);
    void setBooks();
    void setBookFont(Book *book);
    void setViewFont();
    void setDictSheet();
    void setBookSheet();
    void setStatusBarSheet();
    void setConfig();
    void toggleBar();
    void toggleDock(bool check);
    void toggleNewTab(bool check);
    void toggleNewBrowser(bool check);
    void goNext();
    void goPrev();
    void reload();
    void setWebLoaded();
    void focusSearch();

    void changeSearchText(const QString&);
    void changeOptDirection(QAction*);
    void changeViewTabCount(int tab_count);
    void changeOptSearchButtonText(const QString &str);

    void addMark();
    void startProcess(const QString &program, const QStringList &arguments);
    void playSound(const QString &fileName);
    void playVideo(const QString &fileName);
    void openExternalLink(const QString &url);
    void processError();
    void clearCache();

    void checkNextSearch();
    void searchClientText(const QString &str, bool raiseWindow=true);

    void aboutQolibri();


private:
    void viewSearch();
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();

    void bookViewSlots();
    void groupDockSlots();

    void setTitle();

    bool isBusy()
    {
        return stopAct->isEnabled();
    }

    Group *groupFromName(const QString &name);
    Book *bookFromName(Group* group, const QString &name);
    SearchMethod readMethodSetting(const QSettings &);
    void writeMethodSetting(const SearchMethod&, QSettings*);
    QString loadAllExternalFont(Book *pbook);

    Model *model;

    GroupDock *groupDock;
    BookView *bookView;

    QMenu   *vmenu;
    QMenu   *optDirectionMenu;
    QAction *addMarkAct;
    QAction *exitAct;
    QAction *enterAct;
    QAction *stopAct;
    QAction *openBookAct;
    QAction *clearEditAct;
    QAction *toggleTabsAct;
    QAction *toggleBrowserAct;
    QAction *toggleDockAct;
    QAction *toggleBarAct;
    QAction *toggleMethodBarAct;
    QAction *toggleBookTreeAct;
    QAction *toggleWatchClipboardAct;
    QAction *booksAct;
    QAction *configAct;
    QAction *sSheetAct;
    QAction *fontAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *viewAllAct;
    QAction *goPrevAct;
    QAction *goNextAct;
    QAction *reloadAct;

    QToolBar *searchBar;
    QToolBar *bookBar;
    QToolBar *methodBar;
    QToolBar *webBar;

    QLineEdit *searchTextEdit;

    QLabel *processLabel;

    QPushButton *optSearchButton;

    QMediaPlayer *sound;
    SearchDirection optDirection;
    QTimer *clipboardsearchtimer;
    QStringList  clientText;

    Qt::DockWidgetArea dockPosition;

    QClipboard::Mode watchClipboardMode;
    int watchClipboardSelectionDelay;
    bool watchClipboardRaiseWindow;
    QString lastClipboardText;
};

#endif
