/**
 ******************************************************************************
 * @file       mtk_cfg.h
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @addtogroup Modules
 * @{
 * @addtogroup GSPModule GPS Module
 * @{
 * @brief Process GPS information
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

#ifndef MTK_CFG_H_
#define MTK_CFG_H_

#include "openpilot.h"
#include "modulesettings.h"

void mtk_cfg_autoconfigure(uintptr_t gps_port, ModuleSettingsGPSSpeedOptions baud_rate,
	bool sbas_enabled);
void mtk_cfg_set_baudrate(uintptr_t gps_port, ModuleSettingsGPSSpeedOptions baud_rate);
void mtk_cfg_set_messages(uintptr_t gps_port);
void mtk_cfg_set_fix_period(uintptr_t gps_port, uint16_t period);
void mtk_cfg_set_sbas(uintptr_t gps_port, bool enabled);

#endif /* MTK_CFG_H_ */

/**
 * @}
 * @}
 */
