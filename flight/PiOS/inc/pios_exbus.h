/**
 ******************************************************************************
 * @file       pios_exbus.h
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_EXBUS Jeti EX Bus protocol driver
 * @{
 * @brief Supports receivers using the Jeti EX Bus protocol
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

#ifndef PIOS_EXBUS_H
#define PIOS_EXBUS_H

#include <pios.h>
#include <pios_usart_priv.h>

#define PIOS_EXBUS_MAX_CHANNELS 24

extern const struct pios_rcvr_driver pios_exbus_rcvr_driver;

/**
 * @brief Initialize a new EX Bus device
 * @param[in] serial_id Pointer to serial device driver
 * @param[in] driver Pointer to comm driver associated with serial device
 * @param[in] lower_id Pointer to associated low-level serial device driver
 * @return 0 on success, negative on failure
 */
int32_t PIOS_EXBUS_Init(uintptr_t *exbus_id,
	const struct pios_com_driver *driver, uintptr_t lower_id, struct pios_com_dev *com_id);

#endif /* PIOS_EXBUS_H */

/**
 * @}
 * @}
 */
