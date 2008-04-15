/***************************************************************************
*   Copyright (C) 2007 by BOP                                             *
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

#include <QtGui>

#include "booksetting.h"
#include "book.h"
#include "bookwidget.h"
#include "groupwidget.h"
#ifdef Q_WS_MAC
#include "titlelabel.h"
#endif
#include "ebook.h"

BookSetting::BookSetting(const QList<Group*> &grp, QWidget *parent)
    : QDialog(parent), findStop(false) 
{
#ifdef Q_WS_MAC
    setWindowFlags(Qt::Sheet);
#else
    setWindowTitle(tr("Book and group settings"));
#endif
    foreach(Group * g, grp) {
        groupList_ << new Group(*g);
    }
    QVBoxLayout *v1 = new QVBoxLayout();
    {
        QHBoxLayout *h1 = new QHBoxLayout();
        {
            QLabel *l = new QLabel(tr("Search directory"));
            searchPath = new QLineEdit(QDir::homePath(), this);
            connect(searchPath, SIGNAL(textChanged(QString)),
                    this, SLOT(searchPathChanged(QString)));
            h1->addWidget(l);
            h1->addWidget(searchPath);
            pathButton = new QPushButton(QIcon(":images/open.png"),
                                         QString(), this);
            connect(pathButton, SIGNAL(clicked()), this, SLOT(setPath()));
            h1->addWidget(pathButton);
        }
        QHBoxLayout *h2 = new QHBoxLayout();
        {
            searchButton = new QPushButton(QIcon(":images/downarrow.png"),
                                           tr("Start search"), this);
            connect(searchButton, SIGNAL(clicked()), this, SLOT(searchBook()));
            cancelButton = new QPushButton(tr("Cancel"), this);
            cancelButton->setEnabled(false);
            connect(cancelButton, SIGNAL(clicked()), this,
                    SLOT(cancelSearch()));
            h2->addStretch();
            h2->addWidget(searchButton);
            h2->addWidget(cancelButton);
            h2->addStretch();
        }
        QLabel *l = new QLabel(tr("<b>SEARCH BOOKS :</b>"));
        findPaths = new QLabel();
        findPaths->setFrameShape(QFrame::StyledPanel);
        findPaths->setFixedWidth(290);
        v1->addStretch();
        v1->addWidget(l);
        v1->addLayout(h1);
        v1->addLayout(h2);
        v1->addWidget(findPaths);
        v1->addStretch();
    }
    if (groupList_.count() == 0)
        groupList_ << new Group(tr("All Books"));

    allDicWidget = new BookWidget(groupList_[0],
                                  true, false, false, true, true, this);
    connect(allDicWidget, SIGNAL(rowChanged(int)),
            this, SLOT(changeBookSelection(int)));

    groupWidget = new GroupWidget(&groupList_, this);
    connect(groupWidget, SIGNAL(rowChanged(int)),
            this, SLOT(changeGroupSelection(int)));

    dicWidget = new BookWidget(NULL, true, false, false, true, false, this);

    QVBoxLayout *v2 = new QVBoxLayout();
    {
        selButton = new QPushButton(this);
        selButton->setIcon(QIcon(":images/rightarrow.png"));
        selButton->setEnabled(false);
        connect(selButton, SIGNAL(clicked()), this, SLOT(addBook()));
        v2->addStretch();
        v2->addWidget(selButton);
        v2->addStretch();
    }

    QGridLayout*g = new QGridLayout();
    {
        g->addLayout(v1, 0, 0, 1, 1);
        g->addWidget(allDicWidget, 1, 0);
        g->addWidget(groupWidget, 0, 2);
        g->addWidget(dicWidget, 1, 2);
        g->addLayout(v2, 1, 1);
    }

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                     QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QVBoxLayout *v = new QVBoxLayout;
#ifdef Q_WS_MAC
    TitleLabel *l = new TitleLabel(tr("Book and group settings"));
    v->addWidget(l);
#endif
    v->addLayout(g);
    v->addWidget(buttonBox);
    setLayout(v);

    if (groupList_.count()  > 1)
        changeGroupSelection(0);
}

void BookSetting::searchBook()
{
    searchPath->setEnabled(false);
    pathButton->setEnabled(false);
    searchButton->setEnabled(false);
    selButton->setEnabled(false);
    allDicWidget->setEnabled(false);
    dicWidget->setEnabled(false);
    groupWidget->setEnabled(false);
    buttonBox->setEnabled(false);
    cancelButton->setEnabled(true);

    bookDirs.clear();
    findCategory(searchPath->text());


    int subbook_count = 0;
    int add_count = 0;

    foreach(QString dir, bookDirs) {
        EBook eb;
        int subbooks = eb.setBook(dir);

        subbook_count += subbooks;
        for (int i = 0; i < subbooks; i++) {
            eb.setSubBook(i);
            if (eb.isHaveText() /* && eb.isHaveWordSearch() */) {
                if (allDicWidget->addBook(eb.title(), dir, i)) {
                    add_count++;
                }
            }
        }
    }

    searchPath->setEnabled(true);
    pathButton->setEnabled(true);
    searchButton->setEnabled(true);
    selButton->setEnabled(false);
    if (groupWidget->currentRow() >= 0 && allDicWidget->currentRow() >= 0)
        selButton->setEnabled(true);
     else
        selButton->setEnabled(false);

    allDicWidget->setEnabled(true);
    dicWidget->setEnabled(true);
    groupWidget->setEnabled(true);
    buttonBox->setEnabled(true);
    cancelButton->setEnabled(false);
    QString msg = QString("%1 books(%2 subbook) found: %3 books added")
                  .arg(bookDirs.size())
                  .arg(subbook_count).arg(add_count);
    findPaths->setText(msg);
    findStop = false;
}

void BookSetting::findCategory(const QString &name)
{
    if (checkStop())
        return;

    findPaths->setText("Searching ..." + name.right(28));

    QDir dir(name);
    if (dir.entryList(QStringList() << "CATALOG" << "CATALOGS", 
                      QDir::Files).size() > 0) {
        bookDirs << dir.absoluteFilePath(name);
    } else {
        QStringList flist = dir.entryList(QStringList() << "*",
                                          QDir::Dirs | QDir::NoDotAndDotDot);
        foreach(QString file, flist) {
            findCategory(dir.absoluteFilePath(file));
        }
    }
}

void BookSetting::searchPathChanged(const QString &str)
{
    //qDebug() << "searchPathChanged";
    if (str.isEmpty() || str == "/")
        searchButton->setEnabled(false);
    else
        searchButton->setEnabled(true);
}

void BookSetting::addBook()
{
    Book *book = allDicWidget->currentBook();

    dicWidget->addBook(book->name(), book->path(), book->bookNo());
}

void BookSetting::changeBookSelection(int row)
{
    //qDebug() << "changeBookSelection row="<<row;
    if (row >= 0 && allDicWidget->currentRow() >= 0 &&
        groupWidget->currentRow() >= 0) {
        selButton->setEnabled(true);
    } else {
        selButton->setEnabled(false);
    }
}

void BookSetting::changeGroupSelection(int row)
{
    //qDebug() << "changeGroupSelection row="<<row;
    if (row >= 0 && groupWidget->currentRow() >= 0 &&
        allDicWidget->currentRow() >= 0) {
        selButton->setEnabled(true);
    } else {
        selButton->setEnabled(false);
    }
    if (row >= 0) {
        Group *grp = groupWidget->currentGroup();
        dicWidget->initBook(grp);
    } else {
        dicWidget->initBook(NULL);
    }
}

void BookSetting::setPath()
{
    QString prev = searchPath->text();
    QString next = QFileDialog::getExistingDirectory(this,
                                                     "Set Search Folder", prev);

    if (!next.isEmpty())
        searchPath->setText(next);
}

void BookSetting::keyPressEvent(QKeyEvent *event)
{
    QString cname =  focusWidget()->metaObject()->className();
    
    if (cname == "QLineEdit")
        return;

    return QDialog::keyPressEvent(event);
}
