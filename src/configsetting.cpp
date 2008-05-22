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

#include "configure.h"
#include "configsetting.h"
#ifdef Q_WS_MAC
#include "titlelabel.h"
#endif

ConfigSetting::ConfigSetting(QWidget *parent)
    : QDialog(parent)
{
#ifdef Q_WS_MAC
    setWindowFlags(Qt::Sheet);
#else
    setWindowTitle(tr("Preference settings"));
#endif

    QVBoxLayout *vl = new QVBoxLayout;
    vl->addStretch();
    highlightCheck = new QCheckBox(tr("Emphasize match string"));
    vl->addWidget(highlightCheck);
    beepSoundCheck = new QCheckBox(tr("Beep sound"));
    vl->addWidget(beepSoundCheck);
    serverModeCheck = new QCheckBox(tr("Server mode(must restart)"));
    vl->addWidget(serverModeCheck);
    {
        QGridLayout *g = new QGridLayout;
        {
            g->addWidget(new QLabel(tr("Max history")), 0, 0);
            historyBox = new QSpinBox();
            historyBox->setRange(10, 1000);
            historyBox->setSingleStep(10);
            g->addWidget(historyBox, 0, 1);
        }
        {
            g->addWidget(new QLabel(tr("Indent offset")), 1, 0);
            indentOffsetBox = new QSpinBox();
            indentOffsetBox->setRange(0, 100);
            indentOffsetBox->setSingleStep(10);
            g->addWidget(indentOffsetBox, 1, 1);
        }
        {
            g->addWidget(new QLabel(tr("Port No (server mode)")), 2, 0);
            portNoBox = new QSpinBox();
            portNoBox->setRange(5000, 9999);
            portNoBox->setSingleStep(1);
            g->addWidget(portNoBox, 2, 1);
        }
        vl->addLayout(g);
    } {
        QGridLayout *g = new QGridLayout;
        {
            g->addWidget(new QLabel(tr("External program")), 0, 0, 1, 3);
        } {
            g->addWidget(new QLabel(tr("Sound(wave)")), 1, 0);
            waveProcEdit = new QLineEdit();
            QPushButton *b = new QPushButton(QIcon(":images/open.png"), 
                                             QString(), this);
            connect(b, SIGNAL(clicked()), this, SLOT(setWaveProcess()));
            g->addWidget(waveProcEdit, 1, 1);
            g->addWidget(b, 1, 2);
        } {
            g->addWidget(new QLabel(tr("Movie(mpeg)")), 2, 0);
            mpegProcEdit = new QLineEdit();
            QPushButton *b = new QPushButton(QIcon(":images/open.png"), 
                                             QString(), this);
            connect(b, SIGNAL(clicked()), this, SLOT(setMpegProcess()));
            g->addWidget(mpegProcEdit, 2, 1);
            g->addWidget(b, 2, 2);
        } {
            g->addWidget(new QLabel(tr("Web browser")), 3, 0);
            browserProcEdit = new QLineEdit();
            QPushButton *b = new QPushButton(QIcon(":images/open.png"),
                                             QString(), this);
            connect(b, SIGNAL(clicked()), this, SLOT(setBrowserProcess()));
            g->addWidget(browserProcEdit, 3, 1);
            g->addWidget(b, 3, 2);
        } {
            g->addWidget(new QLabel(tr("Google URL")), 4, 0);
            googleUrlEdit = new QLineEdit();
            g->addWidget(googleUrlEdit, 4, 1, 1, 2);
        } {
            g->addWidget(new QLabel(tr("Wikipedia URL")), 5, 0);
            wikipediaUrlEdit = new QLineEdit();
            g->addWidget(wikipediaUrlEdit, 5, 1, 1, 2);
        } {
            g->addWidget(new QLabel(tr("User defined URL")), 6, 0);
            userDefUrlEdit = new QLineEdit();
            g->addWidget(userDefUrlEdit, 6, 1, 1, 2);
        }
        vl->addLayout(g);
    }
    vl->addStretch();
    QGridLayout *g = new QGridLayout;
    {
        g->addWidget(new QLabel(tr("Limit of characters")), 0, 0);
        limitCharBox = new QSpinBox();
        limitCharBox->setRange(100000, 7000000);
        limitCharBox->setSingleStep(10000);
        g->addWidget(limitCharBox, 0, 1);
    } {
        g->addWidget(new QLabel(tr("Limit of menu items")), 1, 0);
        limitMenuBox = new QSpinBox();
        limitMenuBox->setRange(100, 7000);
        limitMenuBox->setSingleStep(100);
        g->addWidget(limitMenuBox, 1, 1);
    } {
        g->addWidget(new QLabel(tr("Limit of hits per book")), 2, 0);
        limitMaxBookBox = new QSpinBox();
        limitMaxBookBox->setRange(1000, 100000);
        limitMaxBookBox->setSingleStep(1000);
        g->addWidget(limitMaxBookBox, 2, 1);
    } {
        g->addWidget(new QLabel(tr("Limit of total hits")), 3, 0);
        limitMaxTotalBox = new QSpinBox();
        limitMaxTotalBox->setRange(1000, 100000);
        limitMaxTotalBox->setSingleStep(1000);
        g->addWidget(limitMaxTotalBox, 3, 1);
    } {
        g->addWidget(new QLabel(tr("Step of hits per book")), 4, 0);
        stepBookHitBox = new QSpinBox();
        stepBookHitBox->setRange(10, 1000);
        stepBookHitBox->setSingleStep(10);
        g->addWidget(stepBookHitBox, 4, 1);
    } {
        g->addWidget(new QLabel(tr("Step of total hits")), 5, 0);
        stepTotalHitBox = new QSpinBox();
        stepTotalHitBox->setRange(100, 10000);
        stepTotalHitBox->setSingleStep(100);
        g->addWidget(stepTotalHitBox, 5, 1);
    }
    QGroupBox *gb = new QGroupBox(tr("Change carefully these parameters"));
    gb->setLayout(g);
    gb->setCheckable(true);
    gb->setChecked(false);
    QHBoxLayout *h = new QHBoxLayout;
    h->addLayout(vl);
    h->addWidget(gb);
    QVBoxLayout *v = new QVBoxLayout;

    {
#ifdef Q_WS_MAC
        v->addWidget(new TitleLabel(tr("Preference settings")));
#endif
        v->addLayout(h);
        QDialogButtonBox *bBox;
        bBox  = new QDialogButtonBox(QDialogButtonBox::Ok |
                                     QDialogButtonBox::Cancel);

        QPushButton *def = bBox->addButton(QDialogButtonBox::RestoreDefaults);
        QPushButton *res = bBox->addButton(tr("Reset"),
                                           QDialogButtonBox::ActionRole);
        connect(bBox, SIGNAL(accepted()), this, SLOT(accept()));
        connect(bBox, SIGNAL(rejected()), this, SLOT(reject()));
        connect(def, SIGNAL(clicked()), this, SLOT(defaultReset()));
        connect(res, SIGNAL(clicked()), this, SLOT(reset()));
        v->addWidget(bBox);
    }

    reset();

    setLayout(v);
}


void ConfigSetting::setWaveProcess()
{
    QString prev = waveProcEdit->text();
    QString next = QFileDialog::getOpenFileName(this,
                                                tr("Set wave player"), prev );

    if (!next.isEmpty()) waveProcEdit->setText(next);
}

void ConfigSetting::setMpegProcess()
{
    QString prev = mpegProcEdit->text();
    QString next = QFileDialog::getOpenFileName(this,
                                                "Set Movie Player", prev );

    if (!next.isEmpty()) mpegProcEdit->setText(next);
}

void ConfigSetting::setBrowserProcess()
{
    QString prev = browserProcEdit->text();
    QString next = QFileDialog::getOpenFileName(this,
                                                "Set Movie Player", prev );

    if (!next.isEmpty()) browserProcEdit->setText(next);
}

void ConfigSetting::reset()
{
    Configure *d = CONF;

    highlightCheck->setChecked(d->highlightMatch);
    beepSoundCheck->setChecked(d->beepSound);
    serverModeCheck->setChecked(d->serverMode);
    historyBox->setValue(d->historyMax);
    indentOffsetBox->setValue(d->indentOffset);
    portNoBox->setValue(d->portNo);
    waveProcEdit->setText(d->waveProcess);
    mpegProcEdit->setText(d->mpegProcess);
    browserProcEdit->setText(d->browserProcess);
    googleUrlEdit->setText(d->googleUrl);
    wikipediaUrlEdit->setText(d->wikipediaUrl);
    userDefUrlEdit->setText(d->userDefUrl);
    limitCharBox->setValue(d->limitBrowserChar);
    limitMenuBox->setValue(d->limitMenuHit);
    limitMaxBookBox->setValue(d->maxLimitBookHit);
    limitMaxTotalBox->setValue(d->maxLimitTotalHit);
    stepBookHitBox->setValue(d->stepBookHitMax);
    stepTotalHitBox->setValue(d->stepTotalHitMax);
}

void ConfigSetting::update()
{
    Configure *d = CONF;

    d->highlightMatch = highlightCheck->isChecked();
    d->beepSound = beepSoundCheck->isChecked();
    d->serverMode = serverModeCheck->isChecked();
    d->historyMax = historyBox->value();
    d->indentOffset = indentOffsetBox->value();
    d->portNo = portNoBox->value();
    d->waveProcess = waveProcEdit->text();
    d->mpegProcess = mpegProcEdit->text();
    d->browserProcess = browserProcEdit->text();
    d->googleUrl = googleUrlEdit->text();
    d->wikipediaUrl = wikipediaUrlEdit->text();
    d->userDefUrl = userDefUrlEdit->text();
    d->limitBrowserChar = limitCharBox->value();
    d->limitMenuHit = limitMenuBox->value();
    d->maxLimitBookHit = limitMaxBookBox->value();
    d->maxLimitTotalHit = limitMaxTotalBox->value();
    d->stepBookHitMax = stepBookHitBox->value();
    d->stepTotalHitMax = stepTotalHitBox->value();
}

void ConfigSetting::defaultReset()
{
    Configure save = *CONF;

    CONF->setDefault();
    reset();
    *CONF = save;
}

