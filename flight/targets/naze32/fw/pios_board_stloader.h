/**
 ******************************************************************************
 * @file       pios_board_stloader.h
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @addtogroup Targets Target Flight Controllers
 * @{
 * @addtogroup Naze Naze32/OpenNaze family support files
 * @{
 * @brief Board specific ST Microelectronics bootloader routines
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

#ifndef PIOS_BOARD_STLOADER_H_
#define PIOS_BOARD_STLOADER_H_

/**
 * Prepares board for entry to ST bootloader (e.g. enable LEDs etc.)
 */
inline void PIOS_Board_STLoader_Setup(void)
{
	// we're going to bootloader, enable LEDs. don't reset these clocks or the LEDs go off
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
	// remap JTAG pins to GPIO
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	// set LEDs as low outputs (on)
	GPIOB->CRL = (uint32_t)(0x77 << 12);
	GPIOB->BRR = (uint16_t)(GPIO_Pin_3 | GPIO_Pin_4);
}

#endif // PIOS_BOARD_STLOADER_H_
