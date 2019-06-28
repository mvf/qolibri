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
#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QComboBox>

class Model;

class DirectionComboBox : public QComboBox
{
    Q_OBJECT

public:
    DirectionComboBox(QWidget *parent, Model *);

private:
    Model *model;
};

class LogicComboBox : public QComboBox
{
    Q_OBJECT

public:
    LogicComboBox(QWidget *parent, Model *);

private:
    Model *model;
};

class DictionaryGroupComboBox : public QComboBox
{
    Q_OBJECT

public:
    DictionaryGroupComboBox(QWidget *parent, Model *);

public slots:
    void update();

private:
    Model *model;
};

class ReaderGroupComboBox : public QComboBox
{
    Q_OBJECT

public:
    ReaderGroupComboBox(QWidget *parent, Model *);

public slots:
    void update();

private:
    Model *model;
};

class ReaderBookComboBox : public QComboBox
{
    Q_OBJECT

public:
    ReaderBookComboBox(QWidget *parent, Model *);

public slots:
    void update();

private:
    Model *model;
};

#endif // TOOLBAR_H
