/**
 ******************************************************************************
 * @file       pios_uavo_serial.c
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_UAVO_SERIAL Serial-over-UAVO Functions
 * @{
 * @brief Provides serial emulation through UAVOs/UAVTalk
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
 */

#include "pios.h"

#if defined(PIOS_INCLUDE_UAVO_SERIAL)

#if defined(PIOS_INCLUDE_FREERTOS)
#include "FreeRTOS.h"
#endif /* defined(PIOS_INCLUDE_FREERTOS) */

#include "uavobjectmanager.h"
#include "uavoserialtx.h"
#include "uavoserialrx.h"

/* Provide a COM driver */
static void PIOS_UAVO_SERIAL_RegisterRxCallback(uintptr_t usart_id, pios_com_callback rx_in_cb, uintptr_t context);
static void PIOS_UAVO_SERIAL_RegisterTxCallback(uintptr_t usart_id, pios_com_callback tx_out_cb, uintptr_t context);
static void PIOS_UAVO_SERIAL_TxStart(uintptr_t usart_id, uint16_t tx_bytes_avail);
static void PIOS_UAVO_SERIAL_RxStart(uintptr_t usart_id, uint16_t rx_bytes_avail);

const struct pios_com_driver pios_uavo_serial_com_driver = {
	.tx_start   = PIOS_UAVO_SERIAL_TxStart,
	.rx_start   = PIOS_UAVO_SERIAL_RxStart,
	.bind_tx_cb = PIOS_UAVO_SERIAL_RegisterTxCallback,
	.bind_rx_cb = PIOS_UAVO_SERIAL_RegisterRxCallback,
};

enum pios_uavo_serial_dev_magic {
	PIOS_UAVO_SERIAL_DEV_MAGIC = 0xc431ac68,
};

struct pios_uavo_serial_dev {
	enum pios_uavo_serial_dev_magic     magic;
	pios_com_callback rx_in_cb;
	uintptr_t rx_in_context;
	pios_com_callback tx_out_cb;
	uintptr_t tx_out_context;
	bool rx_enabled;
	bool tx_enabled;
	UavoSerialRxData rx_data;
	UavoSerialTxData tx_data;
};

/* Private functions */
static void PIOS_UAVO_SERIAL_RxHandler(UAVObjEvent* ev, void* cb_ctx,
	void *uavo_data, int uavo_len);

static bool PIOS_UAVO_SERIAL_Validate(struct pios_uavo_serial_dev *uavo_serial_dev)
{
	return (uavo_serial_dev->magic == PIOS_UAVO_SERIAL_DEV_MAGIC);
}

static struct pios_uavo_serial_dev *PIOS_UAVO_SERIAL_Alloc(void)
{
	struct pios_uavo_serial_dev *uavo_serial_dev;

	uavo_serial_dev = (struct pios_uavo_serial_dev *)PIOS_malloc(sizeof(*uavo_serial_dev));
	if (!uavo_serial_dev)
		return(NULL);

	memset(uavo_serial_dev, 0, sizeof(*uavo_serial_dev));
	uavo_serial_dev->magic = PIOS_UAVO_SERIAL_DEV_MAGIC;
	return(uavo_serial_dev);
}

/**
* Initialise a single UAVO Serial device
*/
int32_t PIOS_UAVO_SERIAL_Init(uintptr_t *uavo_serial_id)
{
	PIOS_DEBUG_Assert(uavo_serial_id);

	struct pios_uavo_serial_dev *uavo_serial_dev;

	uavo_serial_dev = (struct pios_uavo_serial_dev *)PIOS_UAVO_SERIAL_Alloc();
	if (!uavo_serial_dev)
		return -1;

	UavoSerialTxInitialize();
	UavoSerialRxInitialize();

	// bind UAVO callback
	UAVObjConnectCallback(UavoSerialRxHandle(), PIOS_UAVO_SERIAL_RxHandler, 
							uavo_serial_dev, EV_UPDATED | EV_UNPACKED);

	*uavo_serial_id = (uintptr_t)uavo_serial_dev;

	return 0;
}

static void PIOS_UAVO_SERIAL_RxStart(uintptr_t uavo_serial_id, uint16_t rx_bytes_avail)
{
	struct pios_uavo_serial_dev *uavo_serial_dev = (struct pios_uavo_serial_dev *)uavo_serial_id;
	PIOS_Assert(PIOS_UAVO_SERIAL_Validate(uavo_serial_dev));
	
	uavo_serial_dev->rx_enabled = true;
}
static void PIOS_UAVO_SERIAL_TxStart(uintptr_t uavo_serial_id, uint16_t tx_bytes_avail)
{
	struct pios_uavo_serial_dev *uavo_serial_dev = (struct pios_uavo_serial_dev *)uavo_serial_id;
	PIOS_Assert(PIOS_UAVO_SERIAL_Validate(uavo_serial_dev));
	
	uavo_serial_dev->tx_enabled = true;
}

static void PIOS_UAVO_SERIAL_RegisterRxCallback(uintptr_t uavo_serial_id, pios_com_callback rx_in_cb, uintptr_t context)
{
	struct pios_uavo_serial_dev *uavo_serial_dev = (struct pios_uavo_serial_dev *)uavo_serial_id;
	PIOS_Assert(PIOS_UAVO_SERIAL_Validate(uavo_serial_dev));
	
	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	uavo_serial_dev->rx_in_context = context;
	uavo_serial_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_UAVO_SERIAL_RegisterTxCallback(uintptr_t uavo_serial_id, pios_com_callback tx_out_cb, uintptr_t context)
{
	struct pios_uavo_serial_dev *uavo_serial_dev = (struct pios_uavo_serial_dev *)uavo_serial_id;
	PIOS_Assert(PIOS_UAVO_SERIAL_Validate(uavo_serial_dev));
	
	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	uavo_serial_dev->tx_out_context = context;
	uavo_serial_dev->tx_out_cb = tx_out_cb;
}

static void PIOS_UAVO_SERIAL_RxHandler(UAVObjEvent* ev, void* cb_ctx,
	void *uavo_data, int uavo_len)
{
	struct pios_uavo_serial_dev *uavo_serial_dev = (struct pios_uavo_serial_dev *)cb_ctx;
	PIOS_Assert(PIOS_UAVO_SERIAL_Validate(uavo_serial_dev));

	if (uavo_serial_dev->rx_in_cb) {
		memcpy(&uavo_serial_dev->rx_data, uavo_data, sizeof(uavo_serial_dev->rx_data));
		uavo_serial_dev->rx_in_cb(uavo_serial_dev->rx_in_context, uavo_serial_dev->rx_data.Payload,
				uavo_serial_dev->rx_data.PayloadLength, NULL, NULL);
	}

}

#endif /* PIOS_INCLUDE_UAVO_SERIAL */

/**
 * @}
 * @}
 */
