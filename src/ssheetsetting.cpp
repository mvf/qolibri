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

#include "ssheetsetting.h"
#ifdef Q_WS_MAC
#include "titlelabel.h"
#endif


const char *view1 =
    "<html>\n"
    "<head>\n"
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>\n"
    "<style type=\"text/css\">\n";

const char *view2 =
    "</style>\n"
    "</head>\n"
    "<body>\n";

const char *view3 =
    "</body></html>";


SSheetSetting::SSheetSetting(const QString &current, const QString &defsheet,
                             bool dic, QWidget *parent)
    : QDialog(parent), initialSheet(current), defaultSheet(defsheet)
{
#ifdef Q_WS_MAC
    setWindowFlags(Qt::Sheet);
#else
    setWindowTitle(tr("Browser style sheet setting"));
#endif

    QFile file((dic) ? ":/data/dict-style.html" : ":/data/book-style.html");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    testText = in.readAll();

    QVBoxLayout *v = new QVBoxLayout;
#ifdef Q_WS_MAC
    TitleLabel *l = new TitleLabel(tr("Browser style sheet setting"));
    v->addWidget(l);
#endif
    QSplitter *splitter = new QSplitter(this);
    edit = new QTextEdit();
    edit->setPlainText(current);
    view = new QTextEdit();
    view->setReadOnly(true);
    view->setHtml(setViewText());
    splitter->addWidget(edit);
    splitter->addWidget(view);
    splitter->setStretchFactor(splitter->indexOf(view),  1);
    v->addWidget(splitter);
    v->setStretchFactor(splitter, 1);

    QDialogButtonBox *bBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
        QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Reset);
    QPushButton *save = bBox->addButton(tr("Save to file..."),
                                        QDialogButtonBox::ActionRole);
    QPushButton *load = bBox->addButton(tr("Load from file..."),
                                        QDialogButtonBox::ActionRole);
    QPushButton *aply = bBox->addButton(tr("Test"),
                                        QDialogButtonBox::ActionRole);
    connect(bBox, SIGNAL(accepted()), SLOT(accept()));
    connect(bBox, SIGNAL(rejected()), SLOT(reject()));
    connect(bBox->button(QDialogButtonBox::RestoreDefaults),
            SIGNAL(clicked()), SLOT(defaultReset()));
    connect(bBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
            SLOT(reset()));
    connect(save, SIGNAL(clicked()), SLOT(save()));
    connect(load, SIGNAL(clicked()), SLOT(load()));
    connect(aply, SIGNAL(clicked()), SLOT(apply()));
    v->addWidget(bBox);

    setLayout(v);
}

// private slot:
//

void SSheetSetting::save()
{
    QString file = QFileDialog::getSaveFileName(this,
                                                tr("Save style sheet"),
                                                QDir::homePath());

    if (file.isEmpty()) {
        return;
    }
    QFile f(file);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&f);
    out << edit->toPlainText();
}

void SSheetSetting::load()
{
    QString file = QFileDialog::getOpenFileName(this,
                                                tr("Load style sheet"),
                                                QDir::homePath());

    if (file.isEmpty()) {
        return;
    }
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QTextStream in(&f);
    edit->setPlainText(in.readAll());
    setViewText();
}



QString SSheetSetting::setViewText()
{
    return view1 + edit->toPlainText() + view2 + testText + view3;
}

SSheetOptSetting::SSheetOptSetting(const QString &current,
                                   const QString &defsheet, QWidget *parent)
    : QDialog(parent), initialSheet(current), defaultSheet(defsheet)
{
#ifndef Q_WS_MAC
    setWindowFlags(Qt::Sheet);
#else
    setWindowTitle(tr("Style sheet setting"));
#endif

    QVBoxLayout *v = new QVBoxLayout;
#ifdef Q_WS_MAC
    TitleLabel *l = new TitleLabel(tr("Style sheet setting"));
    v->addWidget(l);
#endif
    edit = new QTextEdit();
    edit->setPlainText(current);

    QDialogButtonBox *bBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
        QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Reset);
    connect(bBox, SIGNAL(accepted()), SLOT(accept()));
    connect(bBox, SIGNAL(rejected()), SLOT(reject()));
    connect(bBox->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()),
            SLOT(defaultReset()));
    connect(bBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
            SLOT(reset()));
    v->addWidget(edit);
    v->setStretchFactor(edit, 1);
    v->addWidget(bBox);

    setLayout(v);
}
