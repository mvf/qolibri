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
#ifndef SSHEETSETTING_H
#define SSHEETSETTING_H

#include <QDialog>
#include <QTextEdit>


class SSheetSetting : public QDialog
{
    Q_OBJECT

public:
    SSheetSetting(const QString &current, const QString &defsheet,
                  bool dic, QWidget *parent);
    inline QString text() const
    {
        return edit->toPlainText();
    }

private slots:
    void save();
    void load();
    void apply()
    {
        view->setHtml(setViewText());
    }
    void reset()
    {
        edit->setPlainText(initialSheet);
        view->setHtml(setViewText());
    }
    void defaultReset()
    {
        edit->setPlainText(defaultSheet);
        view->setHtml(setViewText());
    }

private:
    QString setViewText();

    QTextEdit *edit;
    QTextEdit *view;
    QString initialSheet;
    QString defaultSheet;
    QString testText;
};

class SSheetOptSetting : public QDialog
{
    Q_OBJECT

public:
    SSheetOptSetting(const QString &current, const QString &defsheet,
                     QWidget *parent);
    inline QString text() const
    {
        return edit->toPlainText();
    }

private slots:
    void reset()
    {
        edit->setPlainText(initialSheet);
    }
    void defaultReset()
    {
        edit->setPlainText(defaultSheet);
    }

private:
    QTextEdit *edit;
    QString initialSheet;
    QString defaultSheet;
};

#endif
