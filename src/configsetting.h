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
#ifndef CONFIGSETTING_H
#define CONFIGSETTING_H

#include <QDialog>

class QCheckBox;
class QSpinBox;
class QLineEdit;

class ConfigSetting : public QDialog
{
    Q_OBJECT
public:
    ConfigSetting(QWidget *parent);

    void update();

private slots:
    void setWaveProcess();
    void setMpegProcess();
    void setBrowserProcess();
    void reset();
    void defaultReset();

private:
    QCheckBox *highlightCheck;
    QCheckBox *beepSoundCheck;
    QSpinBox *historyBox;
    QSpinBox *limitImageBox;
    QSpinBox *limitCharBox;
    QSpinBox *limitMenuBox;
    QSpinBox *limitMaxBookBox;
    QSpinBox *limitMaxTotalBox;
    QSpinBox *indentOffsetBox;
    QSpinBox *stepBookHitBox;
    QSpinBox *stepTotalHitBox;
    QLineEdit *waveProcEdit;
    QLineEdit *mpegProcEdit;
    QLineEdit *browserProcEdit;
    QLineEdit *googleUrlEdit;
    QLineEdit *wikipediaUrlEdit;
    QLineEdit *userDefUrlEdit;
};

#endif

