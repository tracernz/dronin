/**
 ******************************************************************************
 * @addtogroup dRonin Targets
 * @{
 * @addtogroup BrainRE1 bootloader
 * @{
 *
 * @file       brainre1/bl/pios_config.h 
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2012-2013
 * @brief      Board specific bootloader configuration file for PiOS
 * @see        The GNU Public License (GPL) Version 3
 *
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

#ifndef PIOS_CONFIG_H
#define PIOS_CONFIG_H

/* Enable/Disable PiOS Modules */
#define PIOS_INCLUDE_LED
#define PIOS_INCLUDE_SPI
#define PIOS_INCLUDE_SYS
#define PIOS_INCLUDE_IAP
#define PIOS_INCLUDE_USB
#define PIOS_INCLUDE_USB_HID
#define PIOS_INCLUDE_COM_MSG
#define PIOS_INCLUDE_FLASH
#define PIOS_INCLUDE_FLASH_INTERNAL
#define PIOS_INCLUDE_FLASH_JEDEC

#define BOOTLOADER_PAUSE_DELAY 2

#define PIOS_INCLUDE_RE1_FPGA

#endif /* PIOS_CONFIG_H */

/**
 * @}
 * @}
 */