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
#include "ebcore.h"

WebSetting::WebSetting(QWidget *parent, const QString &name, const QString &url)
    : QDialog(parent)
{

   QHBoxLayout *h1 = new QHBoxLayout;
   {
       h1->addWidget(new QLabel(tr("Name:")));
       nameEdit_ = new QLineEdit(name, this);
       connect(nameEdit_, SIGNAL(textChanged(const QString&)),
               SLOT(setOkButton(const QString&)));
       connect(nameEdit_, SIGNAL(textEdited(const QString&)),
               SLOT(setOkButton(const QString&)));
       h1->addWidget(nameEdit_);
   }
   QHBoxLayout *h2 = new QHBoxLayout;
   {
       h2->addWidget(new QLabel(tr("URL:")));
       urlEdit_ = new QLineEdit(url, this);
       connect(urlEdit_, SIGNAL(textChanged(const QString&)),
               SLOT(setOkButton(const QString&)));
       connect(urlEdit_, SIGNAL(textEdited(const QString&)),
               SLOT(setOkButton(const QString&)));
       h2->addWidget(urlEdit_);
   }

   buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok |
                                     QDialogButtonBox::Cancel);
   connect(buttonBox_, SIGNAL(accepted()), SLOT(accept()));
   connect(buttonBox_, SIGNAL(rejected()), SLOT(reject()));

   QVBoxLayout *v = new QVBoxLayout;
   v->addWidget(new QLabel(tr("Web Site Setting")));
   v->addLayout(h1);
   v->addLayout(h2);
   v->addWidget(buttonBox_);
   setLayout(v);
   setFixedWidth(500);
}

void WebSetting::setOkButton(const QString&)
{

    QPushButton *b = buttonBox_->button(QDialogButtonBox::Ok);

    if (nameEdit_->text().isEmpty() || urlEdit_->text().isEmpty()) {
        b->setEnabled(false);
    } else {
        b->setEnabled(true);
    }

}

EpwingFileSetting::EpwingFileSetting(QWidget *parent, const QString &name,
                                     const QString &path, int book_no)
    : QDialog(parent)
{

   QHBoxLayout *h1 = new QHBoxLayout;
   {
       h1->addWidget(new QLabel(tr("Name:")));
       nameEdit_ = new QLineEdit(name, this);
       connect(nameEdit_, SIGNAL(textChanged(const QString&)),
               SLOT(setOkButton(const QString&)));
       connect(nameEdit_, SIGNAL(textEdited(const QString&)),
               SLOT(setOkButton(const QString&)));
       h1->addWidget(nameEdit_);
   }
   QHBoxLayout *h2 = new QHBoxLayout;
   {
       h2->addWidget(new QLabel(tr("Path:")));
       pathEdit_ = new QLineEdit(path, this);
       connect(pathEdit_, SIGNAL(textChanged(const QString&)),
               SLOT(setOkButton(const QString&)));
       connect(pathEdit_, SIGNAL(textEdited(const QString&)),
               SLOT(setOkButton(const QString&)));
       h2->addWidget(pathEdit_);
   }
   QHBoxLayout *h3 = new QHBoxLayout;
   {
       h3->addWidget(new QLabel(tr("Book No:")));
       bookNoEdit_ = new QLineEdit(QString::number(book_no), this);
       bookNoEdit_->setInputMask("9");
       connect(bookNoEdit_, SIGNAL(textChanged(const QString&)),
               SLOT(setOkButton(const QString&)));
       connect(bookNoEdit_, SIGNAL(textEdited(const QString&)),
               SLOT(setOkButton(const QString&)));
       h3->addWidget(bookNoEdit_);
   }


   buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok |
                                     QDialogButtonBox::Cancel);
   connect(buttonBox_, SIGNAL(accepted()), SLOT(accept()));
   connect(buttonBox_, SIGNAL(rejected()), SLOT(reject()));

   QVBoxLayout *v = new QVBoxLayout;
   v->addWidget(new QLabel(tr("Epwing File Setting")));
   v->addLayout(h1);
   v->addLayout(h2);
   v->addLayout(h3);
   v->addWidget(buttonBox_);
   setLayout(v);
   setFixedWidth(500);
}

void EpwingFileSetting::setOkButton(const QString&)
{

    QPushButton *b = buttonBox_->button(QDialogButtonBox::Ok);

    if (nameEdit_->text().isEmpty() || pathEdit_->text().isEmpty() ||
        bookNoEdit_->text().isEmpty()) {
        b->setEnabled(false);
    } else {
        b->setEnabled(true);
    }

}
BookSetting::BookSetting(Model *model_, QWidget *parent)
    : QDialog(parent)
    , findStop(false)
    , model(model_)
{
#ifdef Q_WS_MAC
    setWindowFlags(Qt::Sheet);
#else
    setWindowTitle(tr("Book and group settings"));
#endif
    localBooks_ = new Group(*(model->localBooks));
    webSites_ = new Group(*(model->webSites));

    foreach(Group * g, model->groupList) {
        groupList_ << new Group(*g);
    }
    QVBoxLayout *v1 = new QVBoxLayout();
    {
        QHBoxLayout *h1 = new QHBoxLayout();
        {
            //QLabel *l = new QLabel(tr("Search directory"));
            searchPath = new QLineEdit(QDir::homePath(), this);
            connect(searchPath, SIGNAL(textChanged(QString)),
                    SLOT(searchPathChanged(QString)));
            //h1->addWidget(l);
            pathButton = new QPushButton(QIcon(":images/open.png"),
                                         QString(), this);
            connect(pathButton, SIGNAL(clicked()), SLOT(setPath()));
            h1->addWidget(searchPath);
            h1->addWidget(pathButton);
        }
        QHBoxLayout *h2 = new QHBoxLayout();
        {
            searchButton = new QPushButton(QIcon(":images/downarrow.png"),
                                           tr("Start search"), this);
            connect(searchButton, SIGNAL(clicked()), SLOT(searchBook()));
            cancelButton = new QPushButton(tr("Cancel"), this);
            cancelButton->setEnabled(false);
            connect(cancelButton, SIGNAL(clicked()), SLOT(cancelSearch()));
            h2->addStretch();
            h2->addWidget(searchButton);
            h2->addWidget(cancelButton);
            h2->addStretch();
        }
        QLabel *l = new QLabel(tr("<b>Search books:</b>"));
        findPaths = new QLabel();
        findPaths->setFrameShape(QFrame::StyledPanel);
        //findPaths->setFixedWidth(290);
        v1->addStretch();
        v1->addWidget(l);
        v1->addLayout(h1);
        v1->addLayout(h2);
        v1->addWidget(findPaths);
        v1->addStretch();
    }
    QVBoxLayout *v2 = new QVBoxLayout();
    {
        QLabel *l = new QLabel(tr("<b>Internet Search:</b>"));
        QHBoxLayout *h1 = new QHBoxLayout();
        {
            QLabel *n = new QLabel(tr("Name:"));
            webNameEdit = new QLineEdit("",this);
            connect(webNameEdit, SIGNAL(textChanged(QString)),
                    SLOT(webNameChanged(QString)));
            h1->addWidget(n);
            h1->addWidget(webNameEdit);
        }
        QHBoxLayout *h2 = new QHBoxLayout();
        {
            QLabel *s = new QLabel(tr("Site:"));
            webSiteEdit = new QLineEdit("http://",this);
            connect(webSiteEdit, SIGNAL(textChanged(QString)),
                    SLOT(webSiteChanged(QString)));
            h2->addWidget(s);
            h2->addWidget(webSiteEdit);
        }
        QHBoxLayout *h3 = new QHBoxLayout();
        {
            webAddButton = new QPushButton(
                    QIcon(":images/downarrow.png"), tr("Add"), this);
            connect(webAddButton, SIGNAL(clicked()),
                    SLOT(addWebSite()));
            h3->addStretch();
            h3->addWidget(webAddButton);
            h3->addStretch();
        }
        //v2->addStretch();
        v2->addWidget(l);
        v2->addLayout(h1);
        v2->addLayout(h2);
        v2->addLayout(h3);
        //v2->addStretch();
        webNameChanged("");
    }
    QWidget *dicSearch = new QWidget(this);
    dicSearch->setLayout(v1);
    QWidget *webSearch = new QWidget(this);
    webSearch->setLayout(v2);
    tab = new QTabWidget(this);
    tab->addTab(dicSearch, tr("Add Dictionary"));
    tab->addTab(webSearch, tr("Add Web Site"));
    connect(tab, SIGNAL(currentChanged(int)), SLOT(changeTab(int)));

    allDicWidget = new BookWidget(localBooks_, this);
    allDicWidget->hideViewButton();
    allDicWidget->hideFontButton();
    allDicWidget->setFixedWidth(320);
    allDicWidget->bookListWidget()->setSelectionMode(
                                      QAbstractItemView::ExtendedSelection);
    connect(allDicWidget, SIGNAL(rowChanged(int)),
            SLOT(changeBookSelection(int)));
    connect(allDicWidget->editButton(), SIGNAL(clicked()), SLOT(editItem()));
    connect(allDicWidget->addButton(), SIGNAL(clicked()), SLOT(addItem()));

    groupWidget = new GroupWidget(&groupList_, this);
    connect(groupWidget, SIGNAL(rowChanged(int)),
            SLOT(changeGroupSelection(int)));

    dicWidget = new BookWidget(NULL, this);
    dicWidget->bookListWidget()->setSelectionMode(
                                      QAbstractItemView::ExtendedSelection);
    dicWidget->setFixedWidth(320);
    dicWidget->hideViewButton();
    dicWidget->hideFontButton();
    dicWidget->hideAddButton();
    dicWidget->hideEditButton();

    QVBoxLayout *va = new QVBoxLayout();
    {
        selButton = new QPushButton(this);
        selButton->setIcon(QIcon(":images/rightarrow.png"));
        selButton->setEnabled(false);
        connect(selButton, SIGNAL(clicked()), SLOT(addBook()));
        va->addStretch();
        va->addWidget(selButton);
        va->addStretch();
    }

    QGridLayout*g = new QGridLayout();
    {
        g->addWidget(tab, 0, 0, 1, 2);
        //g->addLayout(v1, 0, 0, 1, 2);
        g->addWidget(allDicWidget, 1, 0);
        g->addWidget(groupWidget, 0, 2);
        g->addWidget(dicWidget, 1, 2);
        g->addLayout(va, 1, 1);
        g->setColumnStretch(0, 0);
    }

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                     QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
    QVBoxLayout *v = new QVBoxLayout;
#ifdef Q_WS_MAC
    TitleLabel *l = new TitleLabel(tr("Book and group settings"));
    v->addWidget(l);
#endif
    v->addLayout(g);
    v->addWidget(buttonBox);
    setLayout(v);

    if (groupList_.count()  > 0)
        changeGroupSelection(0);
}

void BookSetting::accept()
{
    QDialog::accept();
    model->setLocalBooks(localBooks_);
    model->setWebSites(webSites_);
    model->setGroupList(groupList_);
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
        EbCore eb;
        int subbooks = eb.initBook(dir);

        subbook_count += subbooks;
        for (int i = 0; i < subbooks; i++) {
            eb.initSubBook(i);
            if (eb.isHaveText() /* && eb.isHaveWordSearch() */) {
                if (allDicWidget->addBook(eb.subbookTitle(), BookLocal,
                                          dir, i)) {
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

    //findPaths->setText("Searching ..." + name.right(28));

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
    QString msg = QString("Found %1 books [%2]")
                  .arg(bookDirs.size()).arg(dir.dirName().right(20));
    findPaths->setText(msg);
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
    foreach(QListWidgetItem *i, allDicWidget->selectedBooks()) {
        Book *book = (Book*)i;
        dicWidget->addBook(book->name(), book->bookType(),
                           book->path(), book->bookNo());
    }

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

void BookSetting::editItem()
{
    Book *b = (Book *)allDicWidget->currentItem();
    if (b->bookType() == BookLocal) {
        EpwingFileSetting dlg(this, b->name(), b->path(), b->bookNo());
        if (dlg.exec() == QDialog::Accepted){
            b->setName(dlg.name());
            b->setPath(dlg.path());
            b->setBookNo(dlg.bookNo());
        }
    } else if (b->bookType() == BookWeb) {
        WebSetting dlg(this, b->name(), b->path());
        if (dlg.exec() == QDialog::Accepted){
            b->setName(dlg.name());
            b->setPath(dlg.url());
        }
    }

}

void BookSetting::addItem()
{
    if (tab->currentIndex() == 0) {
        EpwingFileSetting dlg(this, "", "", 0);
        if (dlg.exec() == QDialog::Accepted){
            allDicWidget->addBook(dlg.name(), BookLocal, dlg.path(),
                                  dlg.bookNo());
        }
    } else if (tab->currentIndex() == 1) {
        WebSetting dlg(this, "", "");
        if (dlg.exec() == QDialog::Accepted){
            allDicWidget->addBook(dlg.name(), BookWeb, dlg.url(),
                                  0);
        }
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

void BookSetting::webNameChanged(const QString &str)
{
    QString stext = webSiteEdit->text();
    if (!str.isEmpty() && !stext.isEmpty() && stext != "http://"){
        webAddButton->setEnabled(true);
    } else {
        webAddButton->setEnabled(false);
    }
}

void BookSetting::webSiteChanged(const QString &str)
{
    QString ntext = webNameEdit->text();
    if (!ntext.isEmpty() && !str.isEmpty() && str != "http://"){
        webAddButton->setEnabled(true);
    } else {
        webAddButton->setEnabled(false);
    }
}

void BookSetting::addWebSite()
{
    allDicWidget->addBook(webNameEdit->text(), BookWeb,
                          webSiteEdit->text(), 0);
}

void BookSetting::changeTab(int index)
{
    if (index == 0) {
        allDicWidget->initBook(localBooks_);
    } else {
        allDicWidget->initBook(webSites_);
    }

}
