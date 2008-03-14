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


#include "book.h"
#include "fontsetting.h"
#include "configure.h"

#ifdef Q_WS_MAC
FontSetting::FontSetting(Book *pbook, QWidget *parent)
    : QDialog(parent, Qt::Tool), book(pbook)
#else
FontSetting::FontSetting(Book *pbook, QWidget *parent)
    : QDialog(parent), book(pbook)
#endif
{
    setWindowTitle(tr("Alternate font setting"));

    QHBoxLayout *h = new QHBoxLayout();
    {
        fontTreeWidget = new QTreeWidget(this);
        fontTreeWidget->setColumnCount(3);
        QStringList hlabels;
        hlabels << tr("Font") << tr("Code") << tr("Alternate font");
        fontTreeWidget->setHeaderLabels(hlabels);
        h->addWidget(fontTreeWidget);
        h->setStretchFactor(fontTreeWidget, 2);
        connect(fontTreeWidget,
                SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem*)),
                this, SLOT(selectFont(QTreeWidgetItem *, QTreeWidgetItem*)));
    }
    {
        QVBoxLayout *v = new QVBoxLayout();
        {
            QHBoxLayout *h2 = new QHBoxLayout();
            {
                fontCodeLabel = new QLabel();
                fontCodeLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
                fontButton = new QPushButton();
                fontButton->setFlat(true);
                h2->addWidget(fontCodeLabel);
                h2->addWidget(fontButton);
                h2->setStretchFactor(fontCodeLabel, 1);
            }
            v->addLayout(h2);
        } {
            QLabel *l = new QLabel(tr("Alternate font code"));
            v->addWidget(l);
        } {
            fontCodeEdit = new QLineEdit(this);
            connect(fontCodeEdit, SIGNAL(textChanged(QString)),
                    this, SLOT(changeFontCode(QString)));
            v->addWidget(fontCodeEdit);
        } {
            fontLabel = new QLabel(" \n \n ");
            fontLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
            fontLabel->setTextFormat(Qt::RichText);
            //fontLabel->setAlignment(Qt::AlignCenter);
            //fontLabel->setScaledContents(true);
            //fontLabel->setMargin(5);
            //fontLabel->setFixedHeight(50);
            QFont font(CONF->browserFont);
            font.setPointSize(32);
            fontLabel->setFont(font);
            v->addWidget(fontLabel);
        }
        v->addStretch();
        h->addLayout(v);
    }

    QVBoxLayout *v = new QVBoxLayout();
    QDialogButtonBox *bBox;
    bBox  = new QDialogButtonBox(QDialogButtonBox::Ok |
                                 QDialogButtonBox::Cancel);
    QPushButton *save = bBox->addButton(tr("Save to file..."),
                                        QDialogButtonBox::ActionRole);
    QPushButton *load = bBox->addButton(tr("Load from file..."),
                                        QDialogButtonBox::ActionRole);
    bBox->addButton(load, QDialogButtonBox::ActionRole);
    connect(save, SIGNAL(clicked()), this, SLOT(save()));
    connect(load, SIGNAL(clicked()), this, SLOT(load()));
    connect(bBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(bBox, SIGNAL(rejected()), this, SLOT(reject()));
    v->addLayout(h);
    v->addWidget(bBox);
    setLayout(v);

}

int FontSetting::setupTreeWidget(const QString &font_path)
{
    QList<QTreeWidgetItem *> items;
#ifdef USE_GIF_FOR_FONT
    QStringList flist = QDir(font_path).entryList(QStringList() << "*.gif",
                                                  QDir::Files, QDir::Name);
#else
    QStringList flist = QDir(font_path).entryList(QStringList() << "*.png",
                                                  QDir::Files, QDir::Name);
#endif
    cacheDir = font_path;
    if (flist.count() < 1) {
        return 0;
    }
    foreach(QString s, flist) {
        QStringList nlist = s.split(QChar('.'));
        QStringList tlist = QStringList() << QString() << nlist[0];

        if (book->fontList()) {
            tlist << book->fontList()->value(nlist[0]);
        } else {
            tlist << QString();
        }

        QTreeWidgetItem *font = new QTreeWidgetItem((QTreeWidget*)0, tlist);
        QString fname = font_path + "/" + s;
        font->setIcon(0, QIcon(fname));
        items << font;
    }
    fontTreeWidget->insertTopLevelItems(0, items);
    fontTreeWidget->setCurrentItem(items[0]);
    return flist.count();
}

void FontSetting::selectFont(QTreeWidgetItem *current, QTreeWidgetItem*)
{
    if (!current) {
        return;
    }
#ifdef USE_GIF_FOR_FONT
    QString fname = cacheDir + "/" + current->text(1) + ".gif";
#else
    QString fname = cacheDir + "/" + current->text(1) + ".png";
#endif
    fontButton->setIcon(QIcon(fname));
    fontCodeLabel->setText(current->text(1));
    fontCodeEdit->setText(current->text(2));
    fontLabel->setText("\n" + current->text(2) + "\n");
    fontCodeEdit->setFocus();
}

void FontSetting::changeFontCode(const QString &str)
{
    fontLabel->setText(str);
    QTreeWidgetItem *w = fontTreeWidget->currentItem();
    if (w) {
        w->setText(2, fontCodeEdit->text());
    }
}

QHash <QString, QString> *FontSetting::newAlternateFontList()
{
    QHash <QString, QString> *flist = new QHash <QString, QString>;
    for (int i = 0; i < fontTreeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *w = fontTreeWidget->topLevelItem(i);
        if (!w->text(2).isEmpty() ) {
            flist->insert(w->text(1), w->text(2));
        }
    }

    return flist;
}
void FontSetting::save()
{
    QString fname = QFileDialog::getSaveFileName(this,
                                                tr("Save current font setting"),
                                                QDir::homePath() );

    if (fname.isEmpty()) {
        return;
    }
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Can't open for write" << fname;
        return;
    }
    QTextStream out(&file);
    for (int i = 0; i < fontTreeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *w = fontTreeWidget->topLevelItem(i);
        QString f = w->text(2);
        f.trimmed();
        if (!f.isEmpty()) {
            out << w->text(1) << " " << f << "\n";
        }
    }
}

void FontSetting::load()
{
    QString fname = QFileDialog::getOpenFileName(this,
                                                tr("Load font setting"),
                                                QDir::homePath() );

    if (fname.isEmpty())
        return;
    
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    while (!file.atEnd()) {
        QString line = file.readLine();
        QStringList list = line.remove('\n').split(' ');
        if (list.count() != 2) continue;

        for (int j = 0; j < fontTreeWidget->topLevelItemCount(); j++) {
            QTreeWidgetItem *w = fontTreeWidget->topLevelItem(j);
            if (w->text(1) == list[0]) {
                w->setText(2, list[1]);
                break;
            }
        }
    }
}
