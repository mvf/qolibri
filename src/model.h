/***************************************************************************
*   Copyright (C) 2007 by BOP                                             *
*   Copyright (C) 2009 Fujii Hironori                                     *
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
#ifndef MODEL_H
#define MODEL_H

#include "method.h"
#include "book.h"

#include <QObject>
#include <QList>

class QSettings;

enum BookMode { ModeDictionary, ModeBook };

class Model : public QObject {
    Q_OBJECT

public:
    Model();
    ~Model();
    void load();
    void save();

    int dictionaryGroupIndex() { return groupList.indexOf(method.group); };
    int readerGroupIndex() { return groupList.indexOf(method.groupReader); };
    int readerBookIndex() { return method.groupReader->bookList().indexOf(method.bookReader); };

    void setGroupList(const QList <Group*> &groups);
    void setLocalBooks(const Group *);
    void setWebSites(const Group *);

    QList <Group*> groupList;
    Group* localBooks;
    Group* webSites;

    BookMode bookMode;
    SearchMethod method;

public slots:
    void setBookMode(BookMode);
    void setMethod(const SearchMethod &);
    void setDirection(int direction);
    void setLogic(int logic);
    void setGroupIndex(int index);
    void setDictionaryGroupIndex(int index);
    void setReaderGroupIndex(int index);
    void setBookIndex(int index);
    void setDictionaryBookIndex(int index);
    void setReaderBookIndex(int index);
    void setLimitBook(int val);
    void setLimitTotal(int val);

signals:
    void bookModeChanged(BookMode);
    void dictionaryGroupsChanged();
    void directionChanged(int direction);
    void logicChanged(int logic);
    void groupIndexChanged(int index);
    void dictionaryGroupIndexChanged(int index);
    void readerGroupIndexChanged(int index);
    void bookIndexChanged(int index);
    void dictionaryBookIndexChanged(int index);
    void readerBookIndexChanged(int index);
    void limitBookChanged(int val);
    void limitTotalChanged(int val);

private:
    Group *groupFromName(const QString &name);
    Book *bookFromName(Group* group, const QString &name);
    SearchMethod readMethodSetting(const QSettings &);
    void writeMethodSetting(const SearchMethod&, QSettings*);
};

#endif // MODEL_H
