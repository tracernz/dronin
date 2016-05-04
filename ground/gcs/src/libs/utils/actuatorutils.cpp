/**
 ******************************************************************************
 * @file       actuatorutils.h
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @addtogroup GCSLibraries GCS Libraries
 * @{
 * @addtogroup Utilities
 * @{
 * @brief Actuator configuration utilities
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

#include "actuatorutils.h"

ActuatorUtils::ActuatorUtils()
{

}

/**
 * @brief Gets the default/suggested output rate in Hz
 * @param[in] type Actuator type
 * @return Suggested rate in Hz, or -1 on error;
 */
int ActuatorUtils::outputRate(ActuatorType type)
{
    switch (type) {
    case TYPE_ANALOGSERVO:
        return 50;
    case TYPE_DIGITALSERVO:
    case TYPE_PWMESC:
        return 400;
    case TYPE_MULTISHOT:
    case TYPE_ONESHOT42:
    case TYPE_ONESHOT125:
        return 0; // syncpwm
    }
    return -1;
}

/**
 * @brief Gets the default/suggested minimum pulse in us
 * @param[in] type Actuator type
 * @return Suggested pulse in us
 */
double ActuatorUtils::minPulse(ActuatorType type)
{
    switch (type) {
    case TYPE_ANALOGSERVO:
    case TYPE_DIGITALSERVO:
    case TYPE_PWMESC:
        return 1000.0;
    case TYPE_MULTISHOT:
        return 5.0;
    case TYPE_ONESHOT42:
        return 125.0 / 3.0; // oneshot42 is actually oneshot 125 / 3
    case TYPE_ONESHOT125:
        return 125;
    }
    return 0.0;
}

/**
 * @brief Gets the default/suggested neutral pulse in us
 * @param[in] type Actuator type
 * @return Suggested pulse in us
 */
double ActuatorUtils::neutralPulse(ActuatorType type)
{
    switch (type) {
    case TYPE_ANALOGSERVO:
    case TYPE_DIGITALSERVO:
        return 1500.0;
    default:
        return minPulse(type);
    }
    return 0.0;
}

/**
 * @brief Gets the default/suggested maximum pulse in us
 * @param[in] type Actuator type
 * @return Suggested pulse in us
 */
double ActuatorUtils::maxPulse(ActuatorType type)
{
    switch (type) {
    case TYPE_ANALOGSERVO:
    case TYPE_DIGITALSERVO:
    case TYPE_PWMESC:
        return 2000.0;
    case TYPE_MULTISHOT:
        return 25.0;
    case TYPE_ONESHOT42:
        return 250.0 / 3.0; // oneshot42 is actually oneshot 125 / 3
    case TYPE_ONESHOT125:
        return 250;
    }
    return 0.0;
}

/**
 * @}
 * @}
 */
