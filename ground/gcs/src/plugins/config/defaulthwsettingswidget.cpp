/**
 ******************************************************************************
 * @file       DefaultHwSettingsWidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2012-2013
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Placeholder for attitude panel until board is connected.
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
#include "defaulthwsettingswidget.h"
#include <QErrorMessage>
#include <QDebug>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>

/**
 * @brief DefaultHwSettingsWidget::DefaultHwSettingsWidget Constructed when either a new
 * board connection is established or when there is no board
 * @param parent The main configuration widget
 */
DefaultHwSettingsWidget::DefaultHwSettingsWidget(QSharedPointer<UAVObject> settingsObj, QWidget *parent) :
        ConfigTaskWidget(parent),
        defaultHWSettingsWidget(new Ui_defaulthwsettings)
{
    if (!settingsObj) {
        Q_ASSERT(false);
        qWarning() << "Invalid object!";
        return;
    }

    defaultHWSettingsWidget->setupUi(this);

    QFormLayout *layout = qobject_cast<QFormLayout *>(defaultHWSettingsWidget->portSettingsFrame->layout());
    QWidget *wdg;
    QLabel *lbl;

    const auto &fields = settingsObj->getFields();
    for (auto field : fields) {
        if (!field)
            continue;
        switch (field->getType()) {
        case UAVObjectField::BITFIELD:
        case UAVObjectField::ENUM:
            wdg = new QComboBox(this);
            break;
        case UAVObjectField::INT8:
        case UAVObjectField::INT16:
        case UAVObjectField::INT32:
        case UAVObjectField::UINT8:
        case UAVObjectField::UINT16:
        case UAVObjectField::UINT32: {
            wdg = new QSpinBox(this);
            break;
        }
        case UAVObjectField::FLOAT32: {
            wdg = new QDoubleSpinBox(this);
            break;
        }
        case UAVObjectField::STRING:
            wdg = new QLineEdit(this);
            break;
        default:
            continue;
        }

        QStringList objRelation;
        objRelation.append(QString("objname:%1").arg(settingsObj->getName()));
        objRelation.append(QString("fieldname:%1").arg(fields[i]->getName()));
        objRelation.append(QString("buttongroup:1"));
        objRelation.append(QString("haslimits:yes"));
        objRelation.append(QString("useunits:yes"));
        wdg->setProperty("objrelation", objRelation);

        lbl = new QLabel(field->getName(), this);
        layout->addRow(lbl, wdg);
    }

    autoLoadWidgets();
    loadAllLimits();
    populateWidgets();
    refreshWidgetsValues();
    enableControls(true);

    // Have to force the form as clean (unedited by user) since refreshWidgetsValues forces it to dirty.
    forceConnectedState();

    disableMouseWheelEvents();
}

DefaultHwSettingsWidget::~DefaultHwSettingsWidget()
{
    delete defaultHWSettingsWidget;
}
