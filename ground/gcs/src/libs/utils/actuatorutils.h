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

#ifndef ACTUATORUTILS_H
#define ACTUATORUTILS_H


class ActuatorUtils
{
public:
    enum ActuatorType {
        TYPE_PWMESC,
        TYPE_ANALOGSERVO,
        TYPE_DIGITALSERVO,
        TYPE_ONESHOT42,
        TYPE_ONESHOT125,
        TYPE_MULTISHOT,
    };

    ActuatorUtils();

    static int outputRate(ActuatorType type);
    static double minPulse(ActuatorType type);
    static double neutralPulse(ActuatorType type);
    static double maxPulse(ActuatorType type);
};

#endif // ACTUATORUTILS_H

/**
 * @}
 * @}
 */
