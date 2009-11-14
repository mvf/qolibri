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

#include <QSettings>
#include "model.h"

Model::Model()
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
            Book *d = g->bookList()[j];
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
}

void Model::setGroupList(const QList <Group*> &groups)
{
    groupList.clear();
    foreach(Group *g, groups) {
        groupList << new Group(*g);
    }
    emit dictionaryGroupsChanged();
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
    
