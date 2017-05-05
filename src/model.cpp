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

#include "model.h"

Model::Model()
    : bookMode(ModeDictionary)
    , m_scanClipboard(false)
{
}

Model::~Model()
{
}

void Model::load()
{
    QSettings groups(CONF->settingOrg, "EpwingGroups");
    localBooks = new Group("Local Books");
    webSites = new Group("Web Sites");
    int gcnt = groups.beginReadArray("DictionaryGroups");
    for (int i = 0; i < gcnt; i++) {
        groups.setArrayIndex(i);
        QString name = groups.value("name").toString();
        Group *g;
        if (i == 0) {
            g = localBooks;
        } else if (i == 1) {
            g = webSites;
        } else {
            g = new Group(name);
            groupList << g;
        }
        int dcnt = groups.beginReadArray("Dictionaries");
        for (int j = 0; j < dcnt; j++) {
            groups.setArrayIndex(j);
            name = groups.value("name").toString();
            int booktype = groups.value("booktype").toInt();
            QString path = groups.value("path").toString();
            int subbook = groups.value("subbook").toInt();
            QString use = groups.value("use").toString();
            bool bUse = false;
            if (use == "ON") bUse = true;
            Book *d = new Book(name, (BookType)booktype, path, subbook, bUse);
            g->addBook(d);
        }
        groups.endArray();
    }
    groups.endArray();
    if (groupList.count() == 0) {
        groupList << new Group(tr("All books"));
    }
    emit dictionaryGroupsChanged();

    {
        QSettings settings(CONF->settingOrg, "EpwingViewer");
        method = readMethodSetting(settings);
        setMethod(method);
    }
    {
        QSettings settings(CONF->settingOrg, "option");
        m_scanClipboard = settings.value("scan_clipboard", true).toBool();
    }
}

void Model::save()
{
    QSettings groups(CONF->settingOrg, "EpwingGroups");

    groups.beginWriteArray("DictionaryGroups");
    for (int i = 0; i < groupList.count()+2 ; i++) {
        Group *g;
        if (i == 0) {
            g = localBooks;
        } else if (i == 1) {
            g = webSites;
        } else {
            g = groupList[i-2];
        }
        groups.setArrayIndex(i);
        groups.setValue("name", g->name());
        if (g->checkState() == Qt::Checked) {
            groups.setValue("use", "ON");
        } else {
            groups.setValue("use", "OFF");
        }
        groups.beginWriteArray("Dictionaries");
        for (int j = 0; j < g->bookList().count(); j++) {
            Book *d = g->bookList().at(j);
            groups.setArrayIndex(j);
            groups.setValue("booktype", (int)(d->bookType()));
            groups.setValue("name", d->name());
            groups.setValue("path", d->path());
            groups.setValue("subbook", d->bookNo());
            if (d->checkState() == Qt::Checked) {
                groups.setValue("use", "ON");
            } else {
                groups.setValue("use", "OFF");
            }
        }
        groups.endArray();
    }
    groups.endArray();

    {
        QSettings settings(CONF->settingOrg, "EpwingViewer");
        writeMethodSetting(method, &settings);
    }
    {
        QSettings settings(CONF->settingOrg, "option");
        settings.setValue("scan_clipboard", m_scanClipboard);
    }
}

void Model::setGroupList(const QList <Group*> &groups)
{
    groupList.clear();
    foreach(Group *g, groups) {
        groupList << new Group(*g);
    }
    emit dictionaryGroupsChanged();

    if (groupList.count() == 0) {
        setDictionaryGroupIndex(-1);
        setReaderGroupIndex(-1);
    } else {
        setDictionaryGroupIndex(0);
        setReaderGroupIndex(0);
    }
}

void Model::setLocalBooks(const Group *g)
{
    delete localBooks;
    localBooks = new Group(*g);
}

void Model::setWebSites(const Group *g)
{
    delete webSites;
    webSites = new Group(*g);
}

void Model::setBookMode(BookMode mode)
{
    if (bookMode == mode)
        return;
    bookMode = mode;
    emit bookModeChanged(mode);
    //Group *group = mode == ModeBook ? method.groupReader : method.group;
    //setGroup(groupList.indexOf(group));
}

void Model::setMethod(const SearchMethod &m)
{
    switch (m.direction) {
    case WholeRead:
    case MenuRead:
    case BookInfo:
        setBookMode(ModeBook);
        setGroupIndex(groupList.indexOf(m.groupReader));
        break;
    default:
        setBookMode(ModeDictionary);
        setGroupIndex(groupList.indexOf(m.group));
        setDirection(m.direction);
        setLogic((int)m.logic);
        setLimitBook(m.limitBook);
        setLimitTotal(m.limitTotal);
        break;
    }
}

void Model::setDirection(int direction)
{
    if (method.direction == (SearchDirection)direction)
        return;
    method.direction = (SearchDirection)direction;
    emit directionChanged(direction);
}

void Model::setLogic(int logic)
{
    if (method.logic == (NarrowingLogic)logic)
        return;
    method.logic = (NarrowingLogic)logic;
    emit logicChanged(logic);
}

void Model::setGroupIndex(int index)
{
    if (bookMode == ModeDictionary)
        setDictionaryGroupIndex(index);
    else
        setReaderGroupIndex(index);
}

void Model::setDictionaryGroupIndex(int index)
{
    if (groupList.count() == 0)
        index = -1;
    else if (index < 0)
        index = 0;
    if (index == -1) {
        if (method.group == NULL)
            return;
        method.group = NULL;
    } else {
        Group *group = groupList[index];
        if (method.group == group)
            return;
        method.group = group;
    }
    emit dictionaryGroupIndexChanged(index);
    if (bookMode == ModeDictionary)
        emit groupIndexChanged(index);
    setDictionaryBookIndex(-1);
}

void Model::setReaderGroupIndex(int index)
{
    if (index < 0)
        index = 0;
    Group *group = groupList[index];
    if (method.groupReader == group)
        return;
    method.groupReader = group;
    emit readerGroupIndexChanged(index);
    if (bookMode == ModeBook)
        emit groupIndexChanged(index);
    setReaderBookIndex(-1);
}

void Model::setBookIndex(int index)
{
    if (bookMode == ModeDictionary)
        setDictionaryBookIndex(index);
    else
        setReaderBookIndex(index);
}

void Model::setDictionaryBookIndex(int index)
{
    if (method.group->bookList().count() == 0)
        index = -1;
    else if (index < 0)
        index = 0;
    if (index == -1) {
        method.book = NULL;
    } else {
        method.book = method.group->bookList().at(index);
    }
    emit dictionaryBookIndexChanged(index);
    if (bookMode == ModeDictionary)
        emit bookIndexChanged(index);
}

void Model::setReaderBookIndex(int index)
{
    if (method.groupReader->bookList().count() == 0)
        index = -1;
    else if (index < 0)
        index = 0;
    if (index == -1) {
        method.bookReader = NULL;
    } else {
        method.bookReader = method.groupReader->bookList().at(index);
    }
    emit readerBookIndexChanged(index);
    if (bookMode == ModeBook)
        emit bookIndexChanged(index);
}

void Model::setLimitBook(int val)
{
    if (method.limitBook == val)
        return;
    method.limitBook = val;
    emit limitBookChanged(val);
}

void Model::setLimitTotal(int val)
{
    if (method.limitTotal == val)
        return;
    method.limitTotal = val;
    emit limitTotalChanged(val);
}

Group *Model::groupFromName(const QString &name)
{
    if (groupList.count() == 0) {
        return NULL;
    }
    foreach(Group * g, groupList) {
        if (g->name() == name) {
            return g;
        }
    }

    return groupList[0];
}

Book *Model::bookFromName(Group *grp, const QString &name)
{
    if (grp->bookList().count() == 0) {
        return NULL;
    }
    foreach(Book * b, grp->bookList()) {
        if (b->name() == name) {
            return b;
        }
    }
    return grp->bookList().at(0);
}

SearchMethod Model::readMethodSetting(const QSettings &set)
{
    SearchMethod m;

    m.group = groupFromName(set.value("group").toString());
    m.groupReader = groupFromName(set.value("group_reader").toString());
    m.book = bookFromName(m.group, set.value("book").toString());
    m.bookReader = bookFromName(m.groupReader,
                                set.value("book_reader").toString());
    m.direction = (SearchDirection)set.value("search_method",
                                             ExactWordSearch).toInt();
    m.logic  = (NarrowingLogic)set.value("narrowing_logic",
                                         LogicAND).toInt();
    m.limitBook = set.value("limit_book", 100).toInt();
    m.limitTotal = set.value("limit_total", 1000).toInt();
    m.ruby = set.value("ruby", true).toBool();

    return m;
}

void Model::writeMethodSetting(const SearchMethod &m, QSettings *set)
{
    set->setValue("search_method", m.direction);
    set->setValue("narrowing_logic", m.logic);
    set->setValue("limit_book", m.limitBook);
    set->setValue("limit_total", m.limitTotal);
    set->setValue("ruby", m.ruby);
    if (m.group) {
        set->setValue("group", m.group->name());
    }
    if (m.groupReader) {
        set->setValue("group_reader", m.groupReader->name());
    }
    if (m.book) {
        set->setValue("book", m.book->name());
    }
    if (m.bookReader) {
        set->setValue("book_reader", m.bookReader->name());
    }
}

bool Model::scanClipboard()
{
    return m_scanClipboard;
}

void Model::setScanClipboard(bool v)
{
    if (m_scanClipboard == v)
        return;
    m_scanClipboard = v;
    emit scanClipboardChanged(v);
}
