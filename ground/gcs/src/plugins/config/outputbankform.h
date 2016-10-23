/**
 ******************************************************************************
 * @file       outputbankform.h
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

#ifndef OUTPUTBANKFORM_H
#define OUTPUTBANKFORM_H

#include <QWidget>
#include "uavobjectwidgetutils/configtaskwidget.h"
#include "outputchannelform.h"

namespace Ui {
class OutputBankForm;
}

class OutputBankForm : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief OutputBankForm constructor
     * @param parent Parent widget
     * @param idx Index of this output bank (maps to UAVO and used in UI with +1 offset)
     */
    explicit OutputBankForm(ConfigTaskWidget *parent, int idx = 0);
    ~OutputBankForm();

    /**
     * @brief addChannelWidget Add a channel form widget to this bank (will remove it from previous form)
     * @param chanForm Channel form to add to this widget
     */
    void addChannelWidget(OutputChannelForm *chanForm);

public slots:
    /**
     * @brief channelsUpdated SLOT called when the channel->bank assignment changes
     * Should be called after channel forms are moved to their new homes
     */
    void channelsUpdated();
    /**
     * @brief enableControls Enable/disable widgets (e.g. due to UAVO availability)
     * @param enable whether to enable or disable widgets
     */
    void enableControls(bool enable);

private slots:
    void setUpdateRateFromUavo(int);
    void setUpdateRateFromUi(const QString &text);
    void reconfigureBank(QAction *act);
    void setPulseLimitsFromRate(int rate);

private:
    Ui::OutputBankForm *ui;
    QSpinBox *sbUpdateRate;
    int index;
    struct BankType {
        QString name;
        int minPulse;
        int neutralPulse;
        int maxPulse;
        int updateRate;
    };
    static const QVector<BankType> bankTypes;
};

#endif // OUTPUTBANKFORM_H

/**
 * @}
 * @}
 */
