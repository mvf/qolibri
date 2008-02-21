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
#ifndef STATUSBUTTON_H
#define STATUSBUTTON_H

#include <QPushButton>

class StatusButton : public QPushButton
{
    Q_OBJECT
public:
    StatusButton(QWidget *enter, const QString &name, QWidget *parent = 0,
                 bool deco = false);

protected:
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

signals:
    void updown(int next);

private slots:
    void popupMenuCombo();
    void popupMenuSpin();
    void popupMenuList();
    void popupMenuMenu();
    void changeValueSpin(int value);
    void changeTextCombo(const QString &text);
    void changeTextList(int row);
    void changeTextMenu(QAction* );
    void updownCombo(int next);
    void updownSpin(int next);
    void updownList(int next);
    void updownMenu(int next);

private:
    void nextValue(int updown);
    QWidget *enterFocus;
    QWidget *leaveFocus;
    QAction *actionSave;
    bool decolation;
};

#endif

