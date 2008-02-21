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
#ifndef BOOK_H
#define BOOK_H

#include <QListWidgetItem>

class Book : public QListWidgetItem
{
public:
    Book(const QString &nam, const QString &pth, int bno, bool bUse = true)
        : QListWidgetItem(nam), path_(pth), bookNo_(bno), fontList_(NULL)
    {
        setData(Qt::CheckStateRole, true);
        if (bUse) {
            setCheckState(Qt::Checked);
        } else {
            setCheckState(Qt::Unchecked);
        }
        loadAlterFont();
    }
    Book(const Book &that)
        : QListWidgetItem(that), path_(that.path_),
        bookNo_(that.bookNo_), fontList_(that.fontList_)
    {
    }
    void loadAlterFont();
    void saveAlterFont();
    inline QString name() const
    {
        return text();
    }
    inline void setName(const QString name)
    {
        setText(name);
    }
    inline QString path() const
    {
        return path_;
    }
    inline int bookNo() const
    {
        return bookNo_;
    }
    inline QHash<QString, QString> *fontList() const
    {
        return fontList_;
    }
    inline void removeFontList()
    {
        if (fontList_) delete fontList_;
        fontList_ = NULL;
    }
    inline void setFontList(QHash<QString, QString> *flist)
    {
        fontList_ = flist;
    }

private:
    QString path_;
    int bookNo_;
    QHash<QString, QString> *fontList_;
};

class Group : public QListWidgetItem
{
public:
    Group(const QString &nam, bool bUse = true)
        : QListWidgetItem(nam)
    {
        setData(Qt::CheckStateRole, bUse);
        if (bUse) setCheckState(Qt::Checked);
        else setCheckState(Qt::Unchecked);
    }
    Group(const Group &that)
        : QListWidgetItem(that)
    {
        for (int i = 0; i < that.bookList_.count(); i++) {
            bookList_ << new Book(*that.bookList_[i]);
        }
    }
    inline QString name() const
    {
        return text();
    }
    inline void setName(const QString name)
    {
        setText(name);
    }
    inline QList <Book*> bookList() const
    {
        return bookList_;
    }
    inline void addBook(Book* book)
    {
        bookList_ << book;
    }
    inline void takeBook(int idx)
    {
        bookList_.takeAt(idx);
    }
    inline void insBook(int idx, Book *book)
    {
        bookList_.insert(idx, book);
    }
private:
    QList <Book*> bookList_;
};
#endif
