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

#include "statusbutton.h"

static const char downArrow_utf8[] = { 0xe2, 0x96, 0xbC, 0x00 };

StatusButton::StatusButton(QWidget *enter, const QString &name,
                           QWidget *parent, bool deco )
    : QPushButton(parent), enterFocus(enter), decolation(deco)
{
    setObjectName(name);
    leaveFocus = 0;
    QString class_name = enter->metaObject()->className();
    if (class_name == "QComboBox") {
        connect(this, SIGNAL(pressed()), SLOT(popupMenuCombo()));
        connect((QComboBox*)enter, SIGNAL(currentIndexChanged(QString)),
                SLOT(changeTextCombo(QString)));
        connect(this, SIGNAL(updown(int)), SLOT(updownCombo(int)));
    } else if (class_name == "QSpinBox") {
        connect(this, SIGNAL(pressed()), SLOT(popupMenuSpin()));
        connect((QSpinBox*)enter, SIGNAL(valueChanged(int)),
                SLOT(changeValueSpin(int)));
        connect(this, SIGNAL(updown(int)), SLOT(updownSpin(int)));
    } else if (class_name == "QListWidget") {
        setText(" - ");
        connect(this, SIGNAL(pressed()), SLOT(popupMenuList()));
        connect((QListWidget*)enter, SIGNAL(currentRowChanged(int)),
                SLOT(changeTextList(int)));
        connect(this, SIGNAL(updown(int)), SLOT(updownList(int)));
    } else if (class_name == "QMenu") {
        connect(this, SIGNAL(pressed()), SLOT(popupMenuMenu()));
        connect((QMenu*)enter, SIGNAL(triggered(QAction*)),
                SLOT(changeTextMenu(QAction*)));
        connect(this, SIGNAL(updown(int)), SLOT(updownMenu(int)));
    } else {
        qWarning()<< "Unrecognized class Name" << class_name;
    }
}

void StatusButton::popupMenuCombo()
{
    QMenu memu;
    QActionGroup grp(this);
    QComboBox *w = (QComboBox*)enterFocus;
    QAction *a;

    for (int i = 0; i < w->count(); i++) {
        a = memu.addAction(w->itemText(i));
        grp.addAction(a);
        a->setCheckable(true);
        a->setData(i);
        if (i == w->currentIndex())
            a->setChecked(true);
    }
    a = memu.exec(QCursor::pos());
    if (a)
        w->setCurrentIndex(a->data().toInt());
}

void StatusButton::popupMenuSpin()
{
    QMenu menu;
    QActionGroup grp(this);
    QAction *a;
    QSpinBox *w = (QSpinBox*)enterFocus;
    int step = w->singleStep();
    int sp = w->value() + (step * 5);
    int ep = w->value() - (step * 5);

    if (sp > w->maximum()) {
        for (; sp > w->maximum(); sp -= step) ;
    } else if (ep < w->minimum()) {
        for (; ep < w->minimum(); ep += step) {
            sp += step;
        }
    }
    for (int i = 0; i < 11; i++) {
        a = menu.addAction(QString::number(sp));
        grp.addAction(a);
        a->setCheckable(true);
        if (sp == w->value())
            a->setChecked(true);
        sp -= step;
    }
    a = menu.exec(QCursor::pos());
    if (a)
        w->setValue(a->text().toInt());
}

void StatusButton::popupMenuList()
{
    QListWidget *w = (QListWidget*)enterFocus;
    if (!w->count()) {
        qWarning() << "No list data";
        return;
    }
    QMenu *m = new QMenu();
    QActionGroup *g = new QActionGroup(this);
    QAction *a;

    for (int i = 0; i < w->count(); i++) {
        a = m->addAction(w->item(i)->text());
        g->addAction(a);
        a->setCheckable(true);
        if (i == w->currentRow())
            a->setChecked(true);
        a->setData(i);
    }
    a = m->exec(QCursor::pos());
    if (a)
        w->setCurrentRow(a->data().toInt());
    delete m;
    delete g;
}

void StatusButton::popupMenuMenu()
{
    QMenu *m = (QMenu*)enterFocus;

    m->exec(QCursor::pos());
}


void StatusButton::changeValueSpin(int value)
{
    QString str = QString::number(value);

    setText(str);
}

void StatusButton::changeTextCombo(const QString &text)
{
    QString str = text;

    if (decolation)
        str += " " + QString::fromUtf8(downArrow_utf8);
    setText(str);
}

void StatusButton::changeTextList(int row)
{
    if (row < 0) {
        setText(" - ");
        return;
    } else {
        QListWidgetItem *item = ((QListWidget *)enterFocus)->item(row);
        if (!item) {
           setText(" - ");
           return;
        }
        QString str = item->text();
        if (decolation)
            str += " " + QString::fromUtf8(downArrow_utf8);
	    //str += " " + QString(downArrow_utf8, 3);
        setText(str);
    }
}

void StatusButton::changeTextMenu(QAction *act)
{
    actionSave = act;
    QString str = act->text();
    int i = str.indexOf('&');
    if (i >= 0)
        setText(str.mid(i + 1, 1));
}

void StatusButton::updownCombo(int updown)
{
    QComboBox *w = (QComboBox*)enterFocus;
    int nxt = w->currentIndex() + updown;

    if (nxt >= 0 && nxt < w->count())
        w->setCurrentIndex(nxt);
}

void StatusButton::updownSpin(int updown)
{
    QSpinBox *w = (QSpinBox*)enterFocus;
    int nxt = w->value() + (w->singleStep() * updown);
    int mx = w->maximum();
    int mn = w->minimum();

    if (nxt > mx) nxt = mx;
    if (nxt < mn) nxt = mn;
    w->setValue(nxt);
}

void StatusButton::updownList(int updown)
{
    QListWidget *w = (QListWidget*)enterFocus;
    int nxt = w->currentRow() + updown;

    if (nxt >= 0 && nxt < w->count())
        w->setCurrentRow(nxt);
}

void StatusButton::updownMenu(int updown)
{
    QList<QAction*> acts = actionSave->actionGroup()->actions();

    for (int i = 0; i < acts.count(); i++) {
        if (acts[i]->isChecked()) {
            i += updown;
            if (i >= 0 || i < acts.count())
                acts[i]->trigger();
            break;
        }
    }
}

void StatusButton::wheelEvent(QWheelEvent* event)
{
    setFocus();
    if (event->delta() > 0)
        emit updown(1);
    else
        emit updown(-1);
    //nextValue((event->delta() > 0) ? 1 : -1);
}

void StatusButton::keyPressEvent(QKeyEvent *event)
{
    setFocus();
    if (event->key() == Qt::Key_Up)
        emit updown(1);
    else if (event->key() == Qt::Key_Down)
        emit updown(-1);
    else
        QPushButton::keyPressEvent(event);
}

void StatusButton::enterEvent(QEvent*)
{
    leaveFocus = QApplication::focusWidget();
    setFocus();
}

void StatusButton::leaveEvent(QEvent*)
{
    if (leaveFocus)
        leaveFocus->setFocus();
}
