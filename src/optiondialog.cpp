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
#include "optiondialog.h"
#include "configure.h"

OptionDialog::OptionDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    waveProcEdit->setPlaceholderText(tr("(internal audio player)"));
    waveProcEdit->setFileDialogTitle(tr("Browse for audio player"));
    mpegProcEdit->setPlaceholderText(tr("(system default video player)"));
    mpegProcEdit->setFileDialogTitle(tr("Browse for video player"));
    browserProcEdit->setPlaceholderText(tr("(system default web browser)"));
    browserProcEdit->setFileDialogTitle(tr("Browse for web browser"));
    reset();
}

void OptionDialog::insertTab(int index, QWidget *page, const QString &label)
{
    tabWidget->insertTab(index, page, label);
}

void OptionDialog::reset()
{
    Configure *d = CONF;

    highlightCheck->setChecked(d->highlightMatch);
    beepSoundCheck->setChecked(d->beepSound);
    serverModeCheck->setChecked(d->serverMode);
    convertFullwidthCheck->setChecked(d->convertFullwidth);
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
    limitBookBox->setValue(d->limitBookHit);
    limitTotalBox->setValue(d->limitTotalHit);
}

void OptionDialog::accept()
{
    QDialog::accept();
    Configure *d = CONF;

    d->highlightMatch = highlightCheck->isChecked();
    d->beepSound = beepSoundCheck->isChecked();
    d->serverMode = serverModeCheck->isChecked();
    d->convertFullwidth = convertFullwidthCheck->isChecked();
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
    d->limitBookHit = limitBookBox->value();
    d->limitTotalHit = limitTotalBox->value();
}
