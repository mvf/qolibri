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
#include "toolbar.h"

DirectionComboBox::DirectionComboBox(QWidget *parent, Model *model_)
  : QComboBox(parent)
  , model(model_)
{
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    addItem(QObject::tr("Exact word search"));
    addItem(QObject::tr("Forward search"));
    addItem(QObject::tr("Backward search"));
    addItem(QObject::tr("Keyword search"));
    addItem(QObject::tr("Cross search"));
    addItem(QObject::tr("Full text search"));
    setCurrentIndex(model->method.direction);
    connect(this, SIGNAL(currentIndexChanged(int)), model, SLOT(setDirection(int)));
    connect(model, SIGNAL(directionChanged(int)), SLOT(setCurrentIndex(int)));
}

LogicComboBox::LogicComboBox(QWidget *parent, Model *model_)
  : QComboBox(parent)
  , model(model_)
{
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    addItem(tr("AND"));
    addItem(tr("OR"));
    setCurrentIndex(model->method.logic);
    connect(this, SIGNAL(currentIndexChanged(int)), model, SLOT(setLogic(int)));
    connect(model, SIGNAL(logicChanged(int)), SLOT(setCurrentIndex(int)));
}
