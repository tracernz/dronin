/**
 ******************************************************************************
 * @file       outputbankform.cpp
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Servo output configuration form for the config output gadget
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Additional note on redistribution: The copyright and license notices above
 * must be maintained in each individual source file that is a derivative work
 * of this source file; otherwise redistribution is prohibited.
 */

#include "outputbankform.h"
#include "ui_outputbankform.h"

#include "extensionsystem/pluginmanager.h"
#include "actuatorsettings.h"

#include <QMenu>
#include <QMessageBox>

const QVector<OutputBankForm::BankType> OutputBankForm::bankTypes = {
    { "Analog Servo (1500 \u03bcs, 50 Hz)", 1000, 1500, 2000, 50 },
    { "Digital Servo (1500 \u03bcs, 330 Hz)", 1000, 1500, 2000, 330 },
    { "Fast Tail Servo (760 \u03bcs, 560 Hz)", 460, 760, 1060, 560 },
    { "Fast ESC (400 Hz)", 1000, 1030, 2000, 400 },
    { "Oneshot125", 125, 130, 250, 0 },
    { "Oneshot42", 42, 45, 83, 0 },
    { "Multishot", 5, 7, 25, 0 },
    { "Brushed Motor (16.67 kHz)", 0, 5, 60, 16666 },
};

OutputBankForm::OutputBankForm(ConfigTaskWidget *parent, int idx) :
    QWidget(parent),
    ui(new Ui::OutputBankForm)
{
    ui->setupUi(this);
    index = idx;

    ui->gbBank->setTitle(tr("Output Bank %0").arg(index + 1));

    // hidden widget to handle translation from UAVO int field to combobox
    sbUpdateRate = new QSpinBox(this);
    sbUpdateRate->setVisible(false);
    sbUpdateRate->setMaximum(INT16_MAX);
    sbUpdateRate->setObjectName("sbUpdateRate");
    parent->addUAVObjectToWidgetRelation("ActuatorSettings", "TimerUpdateFreq", sbUpdateRate, index);
    connect(sbUpdateRate, SIGNAL(valueChanged(int)), this, SLOT(setUpdateRateFromUavo(int)));
    connect(ui->cbUpdateRate, &QComboBox::currentTextChanged, this, &OutputBankForm::setUpdateRateFromUi);

    // reconfigure bank dropdown
    QMenu *menu = new QMenu(ui->btnBankConfigure);
    for (int i = 0; i < bankTypes.length(); i++) {
        const BankType &bankType = bankTypes.at(i);
        QAction *act = new QAction(bankType.name, ui->btnBankConfigure);
        act->setData(i);
        act->setText(bankType.name);
        menu->addAction(act);
    }
    ui->btnBankConfigure->setMenu(menu);
    connect(menu, &QMenu::triggered, this, &OutputBankForm::reconfigureBank);
}

OutputBankForm::~OutputBankForm()
{
    delete ui;
}

void OutputBankForm::addChannelWidget(OutputChannelForm *chanForm)
{
    ui->gbBank->layout()->addWidget(chanForm);
}

void OutputBankForm::channelsUpdated()
{
    setVisible(findChildren<OutputChannelForm *>().length() > 0);
}

void OutputBankForm::enableControls(bool enable)
{
    ui->cbUpdateRate->setEnabled(enable);
    ui->btnBankConfigure->setEnabled(enable);
}

void OutputBankForm::setUpdateRateFromUavo(int rate)
{
    QString option = QString::number(rate);
    if (rate == 0)
        option = "SyncPWM";
    ui->cbUpdateRate->setCurrentText(option);
}

void OutputBankForm::setUpdateRateFromUi(const QString &text)
{
    int value = sbUpdateRate->value();
    if (text == "SyncPWM") {
        value = 0;
    } else {
        bool ok;
        value = text.toInt(&ok, 10);
        if (!ok) {
            qWarning() << "Setting update rate failed!!";
            return;
        }
        if (value == 0)
            ui->cbUpdateRate->setCurrentText("SyncPWM");
    }
    sbUpdateRate->setValue(value);
}

void OutputBankForm::reconfigureBank(QAction *act)
{
    bool okay;
    int index = act->data().toInt(&okay);
    if (!okay || index >= bankTypes.size()) {
        Q_ASSERT(false);
        return;
    }
    const BankType &type = bankTypes.at(index);

    QList<int> alreadyConfigured;
    QList<int> unusedConfigured;
    for (OutputChannelForm *chan : findChildren<OutputChannelForm *>()) {
        if (chan->min() || chan->neutral() || chan->max()) {
            if (chan->isAssigned())
                alreadyConfigured << chan->index();
            else
                unusedConfigured << chan->index();
        }
    }

    if (unusedConfigured.length()) {
        QMessageBox::StandardButton res = QMessageBox::question(this,
                                                                tr("Unused Channels Configured"),
                                                                tr("One or more of the channels in this bank appears to be unused (not assigned to a motor/servo), "
                                                                   "but they are configured with min, neutral or max values. "
                                                                   "It is recommended to set all unused channels to 0/0/0. "
                                                                   "Do you wish to reset these channels?"));
        if (res != QMessageBox::Yes)
            unusedConfigured.clear();
    }

    if (alreadyConfigured.length()) {
        QMessageBox::StandardButton res = QMessageBox::question(this,
                                                                tr("Bank Already Configured"),
                                                                tr("One or more of the channels in this bank appears to be configured already. "
                                                                   "Do you wish to overwrite these channels with default settings for %0?").arg(type.name));
        if (res != QMessageBox::Yes)
            return;
    }

    int chansSet = 0;
    for (OutputChannelForm *chan : findChildren<OutputChannelForm *>()) {
        if (unusedConfigured.contains(chan->index())) {
            chan->setMin(0);
            chan->setMax(0);
            chan->setNeutral(0);
            chan->setType(0); // PWM
        } else if (chan->isAssigned()) {
            chan->setMin(type.minPulse);
            chan->setMax(type.maxPulse);
            chan->setNeutral(type.neutralPulse);
            chan->setType(0); // PWM
            chansSet++;
        }
    }
    if (chansSet)
        setUpdateRateFromUavo(type.updateRate);
    else
        QMessageBox::information(this, tr("Bank Not Reconfigured"), tr("No channels in this bank are assigned, so the bank was not reconfigured."));
}

void OutputBankForm::setPulseLimitsFromRate(int rate)
{
    double maxPulseWidth, timerPeriodUs = INT16_MAX;

    if (rate != 0) {
        maxPulseWidth = 1000000 / rate;

        // Should never happen, but here in case we ever have a
        // < 16Hz PWM mode.
        if (maxPulseWidth > timerPeriodUs)
            maxPulseWidth = timerPeriodUs;
    } else {
        // SyncPWM has been selected, only the timer period
        // provides bounding
        maxPulseWidth = timerPeriodUs;
    }

    // Find its form line and set the limit.
    for (OutputChannelForm *chanForm : findChildren<OutputChannelForm *>())
        chanForm->setMax(static_cast<int>(maxPulseWidth));
}

/**
 * @}
 * @}
 */
