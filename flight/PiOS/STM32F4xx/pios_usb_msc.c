/**
 ******************************************************************************
 * @file       STM32F4xx/pios_usb_msc.c
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2012-2014
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_USB_COM USB COM Functions
 * @{
 * @brief USB COM functions (STM32 dependent code)
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_USB_MSC)

#include "pios_usb.h"
#include "pios_usb_msc_priv.h"
#include "pios_usb_board_data.h" /* PIOS_BOARD_*_DATA_LENGTH */
#include "pios_usbhook.h"	 /* PIOS_USBHOOK_* */

#define BOT_GET_MAX_LUN              0xFE
#define BOT_RESET                    0xFF

/* Implement COM layer driver API */
static void PIOS_USB_MSC_RegisterTxCallback(uintptr_t usbmsc_id, pios_com_callback tx_out_cb, uintptr_t context);
static void PIOS_USB_MSC_RegisterRxCallback(uintptr_t usbmsc_id, pios_com_callback rx_in_cb, uintptr_t context);
static void PIOS_USB_MSC_TxStart(uintptr_t usbmsc_id, uint16_t tx_bytes_avail);
static void PIOS_USB_MSC_RxStart(uintptr_t usbmsc_id, uint16_t rx_bytes_avail);
static bool PIOS_USB_MSC_Available (uintptr_t usbmsc_id);

const struct pios_com_driver pios_usb_msc_com_driver = {
	.tx_start    = PIOS_USB_MSC_TxStart,
	.rx_start    = PIOS_USB_MSC_RxStart,
	.bind_tx_cb  = PIOS_USB_MSC_RegisterTxCallback,
	.bind_rx_cb  = PIOS_USB_MSC_RegisterRxCallback,
	.available   = PIOS_USB_MSC_Available,
};

enum pios_usb_msc_dev_magic {
	PIOS_USB_MSC_DEV_MAGIC = 0x554d5343, /* "UMSC" */
};

struct pios_usb_msc_dev {
	enum pios_usb_msc_dev_magic     magic;
	const struct pios_usb_msc_cfg * cfg;

	void *lower_id;

	pios_com_callback rx_in_cb;
	uintptr_t rx_in_context;
	pios_com_callback tx_out_cb;
	uintptr_t tx_out_context;

	bool usb_data_if_enabled;

	uint8_t rx_packet_buffer[PIOS_USB_BOARD_MSC_DATA_LENGTH] __attribute__ ((aligned(4)));
	volatile bool rx_active;

	/*
	 * NOTE: This is -1 as somewhat of a hack.  It ensures that we always send packets
	 * that are strictly < maxPacketSize for this interface which means we never have
	 * to bother with zero length packets (ZLP).
	 */
	uint8_t tx_packet_buffer[PIOS_USB_BOARD_MSC_DATA_LENGTH - 1] __attribute__ ((aligned(4)));
	volatile bool tx_active;

	uint32_t rx_dropped;
	uint32_t rx_oversize;
};

static bool PIOS_USB_MSC_validate(struct pios_usb_msc_dev *usb_msc_dev)
{
	return (usb_msc_dev && (usb_msc_dev->magic == PIOS_USB_MSC_DEV_MAGIC));
}

static struct pios_usb_msc_dev *PIOS_USB_MSC_alloc(void)
{
	struct pios_usb_msc_dev *usb_msc_dev;

	usb_msc_dev = (struct pios_usb_msc_dev *)PIOS_malloc(sizeof(*usb_msc_dev));
	if (!usb_msc_dev)
		return(NULL);

	memset(usb_msc_dev, 0, sizeof(*usb_msc_dev));
	usb_msc_dev->magic = PIOS_USB_MSC_DEV_MAGIC;
	return usb_msc_dev;
}

/* Implement USB_IFOPS for MSC Data Interface */
static void PIOS_USB_MSC_DATA_IF_Init(uintptr_t usb_msc_id);
static void PIOS_USB_MSC_DATA_IF_DeInit(uintptr_t usb_msc_id);
static bool PIOS_USB_MSC_DATA_IF_Setup(uintptr_t usb_msc_id, struct usb_setup_request *req);

static struct pios_usb_ifops usb_msc_data_ifops = {
	.init          = PIOS_USB_MSC_DATA_IF_Init,
	.deinit        = PIOS_USB_MSC_DATA_IF_DeInit,
	.setup         = PIOS_USB_MSC_DATA_IF_Setup,
};

static void *pios_usb_msc_id;

int32_t PIOS_USB_MSC_Init(void **usbmsc_id, const struct pios_usb_msc_cfg *cfg, void *lower_id)
{
	PIOS_Assert(usbmsc_id);
	PIOS_Assert(cfg);

	struct pios_usb_msc_dev *usb_msc_dev;

	usb_msc_dev = PIOS_USB_MSC_alloc();
	if (!usb_msc_dev)
		goto out_fail;

	/* Bind the configuration to the device instance */
	usb_msc_dev->cfg = cfg;
	usb_msc_dev->lower_id = lower_id;

	pios_usb_msc_id = usb_msc_dev;

	/* Rx and Tx are not active yet */
	usb_msc_dev->rx_active = false;
	usb_msc_dev->tx_active = false;

	/* Clear stats */
	usb_msc_dev->rx_dropped = 0;
	usb_msc_dev->rx_oversize = 0;

	/* Register class specific interface callbacks with the USBHOOK layer */
	usb_msc_dev->usb_data_if_enabled = false;
	PIOS_USBHOOK_RegisterIfOps(cfg->data_if, &usb_msc_data_ifops, (uintptr_t) usb_msc_dev);

	*usbmsc_id = usb_msc_dev;

	return 0;

out_fail:
	return -1;
}

static bool PIOS_USB_MSC_SendData(struct pios_usb_msc_dev *usb_msc_dev)
{
	uint16_t bytes_to_tx;

	if (!usb_msc_dev->tx_out_cb) {
		return false;
	}

	bool need_yield = false;
	bytes_to_tx = (usb_msc_dev->tx_out_cb)(usb_msc_dev->tx_out_context,
					       usb_msc_dev->tx_packet_buffer,
					       sizeof(usb_msc_dev->tx_packet_buffer),
					       NULL,
					       &need_yield);
	if (bytes_to_tx == 0) {
		return false;
	}

	/* 
	 * Mark this endpoint as being tx active _before_ actually transmitting
	 * to make sure we don't race with the Tx completion interrupt
	 */
	usb_msc_dev->tx_active = true;

	PIOS_USBHOOK_EndpointTx(usb_msc_dev->cfg->data_tx_ep,
				usb_msc_dev->tx_packet_buffer,
				bytes_to_tx);

	return true;
}

static void PIOS_USB_MSC_RxStart(uintptr_t usbmsc_id, uint16_t rx_bytes_avail) {
	struct pios_usb_msc_dev *usb_msc_dev = (struct pios_usb_msc_dev *)usbmsc_id;

	bool valid = PIOS_USB_MSC_validate(usb_msc_dev);
	PIOS_Assert(valid);

	/* Make sure this USB interface has been initialized */
	if (!usb_msc_dev->usb_data_if_enabled) {
		return;
	}

	if (!PIOS_USB_CheckAvailable((uintptr_t)usb_msc_dev->lower_id)) {
		return;
	}

	// If endpoint was stalled and there is now space make it valid
	if (!usb_msc_dev->rx_active && (rx_bytes_avail >= PIOS_USB_BOARD_MSC_DATA_LENGTH)) {
		PIOS_USBHOOK_EndpointRx(usb_msc_dev->cfg->data_rx_ep,
					usb_msc_dev->rx_packet_buffer,
					sizeof(usb_msc_dev->rx_packet_buffer));
		usb_msc_dev->rx_active = true;
	}
}

static void PIOS_USB_MSC_TxStart(uintptr_t usbmsc_id, uint16_t tx_bytes_avail)
{
	struct pios_usb_msc_dev * usb_msc_dev = (struct pios_usb_msc_dev *)usbmsc_id;

	bool valid = PIOS_USB_MSC_validate(usb_msc_dev);
	PIOS_Assert(valid);

	/* Make sure this USB interface has been initialized */
	if (!usb_msc_dev->usb_data_if_enabled) {
		return;
	}

	if (!PIOS_USB_CheckAvailable((uintptr_t)usb_msc_dev->lower_id)) {
		return;
	}

	if (!usb_msc_dev->tx_active) {
		/* Transmitter is not currently active, send a report */
		PIOS_USB_MSC_SendData(usb_msc_dev);
	}
}

static void PIOS_USB_MSC_RegisterRxCallback(uintptr_t usbmsc_id, pios_com_callback rx_in_cb, uintptr_t context)
{
	struct pios_usb_msc_dev * usb_msc_dev = (struct pios_usb_msc_dev *)usbmsc_id;

	bool valid = PIOS_USB_MSC_validate(usb_msc_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	usb_msc_dev->rx_in_context = context;
	usb_msc_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_USB_MSC_RegisterTxCallback(uintptr_t usbmsc_id, pios_com_callback tx_out_cb, uintptr_t context)
{
	struct pios_usb_msc_dev * usb_msc_dev = (struct pios_usb_msc_dev *)usbmsc_id;

	bool valid = PIOS_USB_MSC_validate(usb_msc_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	usb_msc_dev->tx_out_context = context;
	usb_msc_dev->tx_out_cb = tx_out_cb;
}

static bool PIOS_USB_MSC_Available (uintptr_t usbmsc_id)
{
	struct pios_usb_msc_dev *usb_msc_dev = (struct pios_usb_msc_dev *)usbmsc_id;

	bool valid = PIOS_USB_MSC_validate(usb_msc_dev);
	PIOS_Assert(valid);

	return (PIOS_USB_CheckAvailable((uintptr_t)usb_msc_dev->lower_id));
}

static bool PIOS_USB_MSC_DATA_EP_IN_Callback(uintptr_t usb_msc_id, uint8_t epnum, uint16_t len);
static bool PIOS_USB_MSC_DATA_EP_OUT_Callback(uintptr_t usb_msc_id, uint8_t epnum, uint16_t len);

static void PIOS_USB_MSC_DATA_IF_Init(uintptr_t usb_msc_id)
{
	struct pios_usb_msc_dev * usb_msc_dev = (struct pios_usb_msc_dev *)usb_msc_id;

	if (!PIOS_USB_MSC_validate(usb_msc_dev)) {
		return;
	}

	/* Register endpoint specific callbacks with the USBHOOK layer */
	PIOS_USBHOOK_RegisterEpInCallback(usb_msc_dev->cfg->data_tx_ep,
					  sizeof(usb_msc_dev->tx_packet_buffer),
					  PIOS_USB_MSC_DATA_EP_IN_Callback,
					  (uintptr_t) usb_msc_dev);
	PIOS_USBHOOK_RegisterEpOutCallback(usb_msc_dev->cfg->data_rx_ep,
					   sizeof(usb_msc_dev->rx_packet_buffer),
					   PIOS_USB_MSC_DATA_EP_OUT_Callback,
					   (uintptr_t) usb_msc_dev);
	usb_msc_dev->usb_data_if_enabled = true;
}

static void PIOS_USB_MSC_DATA_IF_DeInit(uintptr_t usb_msc_id)
{
	struct pios_usb_msc_dev * usb_msc_dev = (struct pios_usb_msc_dev *)usb_msc_id;

	if (!PIOS_USB_MSC_validate(usb_msc_dev)) {
		return;
	}

	/* reset state of the usb hid device structure */
	usb_msc_dev->rx_active = false;
	usb_msc_dev->rx_dropped = 0;
	usb_msc_dev->rx_oversize = 0;
	usb_msc_dev->tx_active = false;
	usb_msc_dev->usb_data_if_enabled = false;

	/* DeRegister endpoint specific callbacks with the USBHOOK layer */
	PIOS_USBHOOK_DeRegisterEpInCallback(usb_msc_dev->cfg->data_tx_ep);
	PIOS_USBHOOK_DeRegisterEpOutCallback(usb_msc_dev->cfg->data_rx_ep);
}

static bool PIOS_USB_MSC_DATA_IF_Setup(uintptr_t usb_msc_id, struct usb_setup_request *req)
{
	struct pios_usb_msc_dev *usb_msc_dev = (struct pios_usb_msc_dev *)pios_usb_msc_id;

	bool valid = PIOS_USB_MSC_validate(usb_msc_dev);
	PIOS_Assert(valid);

	/* Make sure this is a request for an interface we know about */
	uint8_t ifnum = req->wIndex & 0xFF;
	if (ifnum != usb_msc_dev->cfg->data_if) {
		return false;
	}

	switch (req->bmRequestType & USB_REQ_TYPE_MASK) {
	/* Class request */
	case USB_REQ_TYPE_CLASS :
		switch (req->bRequest) {
		case BOT_GET_MAX_LUN:
			if((req->wValue  == 0) && (req->wLength == 1) &&
					((req->bRequest & 0x80) == 0x80)) {
				uint8_t USBD_MSC_MaxLun = /*USBD_STORAGE_fops->GetMaxLun()*/0;
				if (USBD_MSC_MaxLun > 0) {
					PIOS_USBHOOK_CtrlTx(&USBD_MSC_MaxLun, 1);
				} else {
					return false;
				}
			} else {
				return false;
			}
			break;

		case BOT_RESET:
			if ((req->wValue  == 0) && (req->wLength == 0) &&
					((req->bRequest & 0x80) != 0x80)) {
				//MSC_BOT_Reset(pdev);
			} else {
				return false;
			}
			break;

		default:
			return false;
		}
		break;

	/* Interface & Endpoint request */
	case USB_REQ_TYPE_STANDARD:
		switch (req->bRequest) {
		case USB_REQ_GET_INTERFACE:
			{
				uint8_t USBD_MSC_AltSet = 0;
				PIOS_USBHOOK_CtrlTx(&USBD_MSC_AltSet, 1);
			}
			break;

		case USB_REQ_SET_INTERFACE:
			// TODO
			//USBD_MSC_AltSet = (uint8_t)(req->wValue);
			break;

		case USB_REQ_CLEAR_FEATURE:
			/* Flush the FIFO and Clear the stall status */
			PIOS_USBHOOK_EndpointFlush((uint8_t)req->wIndex);

			/* Re-activate the EP */
			if ((((uint8_t)req->wIndex) & 0x80) == 0x80) {
				PIOS_USBHOOK_DeRegisterEpInCallback(usb_msc_dev->cfg->data_tx_ep);
				PIOS_USBHOOK_RegisterEpInCallback(usb_msc_dev->cfg->data_tx_ep,
					  sizeof(usb_msc_dev->tx_packet_buffer),
					  PIOS_USB_MSC_DATA_EP_IN_Callback,
					  (uintptr_t) usb_msc_dev);
			} else {
				PIOS_USBHOOK_DeRegisterEpOutCallback(usb_msc_dev->cfg->data_rx_ep);
				PIOS_USBHOOK_RegisterEpOutCallback(usb_msc_dev->cfg->data_rx_ep,
					   sizeof(usb_msc_dev->rx_packet_buffer),
					   PIOS_USB_MSC_DATA_EP_OUT_Callback,
					   (uintptr_t) usb_msc_dev);
			}

			/* TODO: Handle BOT error */
			//MSC_BOT_CplClrFeature(pdev, (uint8_t)req->wIndex);
			break;

		}
		break;

	default:
		break;
	}

	return true;
}

/**
 * @brief Callback used to indicate a transmission from device INto host completed
 * Checks if any data remains, pads it into HID packet and sends.
 */
static bool PIOS_USB_MSC_DATA_EP_IN_Callback(uintptr_t usb_msc_id, uint8_t epnum, uint16_t len)
{
	struct pios_usb_msc_dev * usb_msc_dev = (struct pios_usb_msc_dev *)pios_usb_msc_id;

	bool valid = PIOS_USB_MSC_validate(usb_msc_dev);
	PIOS_Assert(valid);

	bool rc = PIOS_USB_MSC_SendData(usb_msc_dev);
	if (!rc) {
		/* No additional data was transmitted, note that tx is no longer active */
		usb_msc_dev->tx_active = false;
	}

	return rc;
}

static bool PIOS_USB_MSC_DATA_EP_OUT_Callback(uintptr_t usb_msc_id, uint8_t epnum, uint16_t len)
{
	struct pios_usb_msc_dev * usb_msc_dev = (struct pios_usb_msc_dev *)pios_usb_msc_id;

	if (!PIOS_USB_MSC_validate(usb_msc_dev)) {
		return false;
	}

	if (len > sizeof(usb_msc_dev->rx_packet_buffer)) {
		len = sizeof(usb_msc_dev->rx_packet_buffer);
	}

	if (!usb_msc_dev->rx_in_cb) {
		/* No Rx call back registered, disable the receiver */
		usb_msc_dev->rx_active = false;
		return false;
	}

	uint16_t headroom;
	bool need_yield = false;
	uint16_t bytes_rxed;
	bytes_rxed = (usb_msc_dev->rx_in_cb)(usb_msc_dev->rx_in_context,
					usb_msc_dev->rx_packet_buffer,
					len,
					&headroom,
					&need_yield);

	if (bytes_rxed < len) {
		/* Lost bytes on rx */
		usb_msc_dev->rx_dropped += (len - bytes_rxed);
	}

	bool rc;
	if (headroom >= sizeof(usb_msc_dev->rx_packet_buffer)) {
		/* We have room for a maximum length message */
		PIOS_USBHOOK_EndpointRx(usb_msc_dev->cfg->data_rx_ep,
					usb_msc_dev->rx_packet_buffer,
					sizeof(usb_msc_dev->rx_packet_buffer));
		rc = true;
	} else {
		/* Not enough room left for a message, apply backpressure */
		usb_msc_dev->rx_active = false;
		rc = false;
	}

	return rc;
}

#endif	/* PIOS_INCLUDE_USB_MSC */

/**
 * @}
 * @}
 */
