/**
 ******************************************************************************
 * @file       pios_exbus.c
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

#include "pios.h"

#if defined(PIOS_INCLUDE_EXBUS)

#if !defined(PIOS_INCLUDE_RTC)
#error PIOS_INCLUDE_RTC must be used to use EXBUS
#endif

#include "openpilot.h"
#include "misc_math.h"
#include "pios_exbus.h"

/* Private Types */
enum pios_exbus_magic {
	PIOS_EXBUS_MAGIC = 0xb35db735 
};

enum pios_exbus_state {
	PIOS_EXBUS_STATE_IDLE = 0,
	PIOS_EXBUS_STATE_BUS,
	PIOS_EXBUS_STATE_LEN,
	PIOS_EXBUS_STATE_FILLBUF,
	PIOS_EXBUS_STATE_DISCARD,
	PIOS_EXBUS_STATE_LAST,
};

enum pios_exbus_bus_state {
	PIOS_EXBUS_BUS_HOLD = 0,
	PIOS_EXBUS_BUS_RELEASE,
};

enum pios_exbus_sync {
	PIOS_EXBUS_SYNC_REPLY = 0x3b,
	PIOS_EXBUS_SYNC_REQUEST = 0x3d,
	PIOS_EXBUS_SYNC_CHAN = 0x3e,
};

struct exbus_packet {
	uint8_t sync;
	uint8_t bus_state;
	uint8_t length;
	uint8_t id;
	uint8_t data[0];
} __attribute__((packed));

struct exbus_data_block {
	uint8_t id;
	uint8_t length;
	uint8_t data[0];
} __attribute__((packed));

struct ex_telemetry_packet {
	uint8_t sync;
	uint8_t length : 6;
	uint8_t type : 2;
	uint16_t manufacturer;
	uint16_t device;
	uint8_t reserved;
	uint8_t data[0];
} __attribute__((packed));

struct ex_telemetry_data {
	uint8_t type : 4;
	uint8_t id : 4;
	uint8_t data[0];
} __attribute__((packed));

struct ex_telemetry_text {
	uint8_t id;
	uint8_t unit_length : 3;
	uint8_t desc_length : 5;
	uint8_t data[0];
} __attribute__((packed));

#define PIOS_EXBUS_RXBUF_LEN (128)

struct pios_exbus_dev {
	enum pios_exbus_magic magic;
	enum pios_exbus_state state;
	enum pios_exbus_bus_state bus_state;
	const struct pios_com_dev *com_id;
	uint32_t rx_timer;
	uint32_t failsafe_timer;
	uint16_t channels[PIOS_EXBUS_MAX_CHANNELS];
	uint16_t crc;
	uint8_t rx_buffer[PIOS_EXBUS_RXBUF_LEN];
	uint8_t rx_buffer_pos;
	uint8_t frame_length;

	volatile int error_count;
	volatile int success_count;
	int telem_count;
};

/* Private Functions */
/**
 * @brief Allocate an EX Bus device driver in memory
 * @return Pointer to dev on success, NULL on failure
 */
static struct pios_exbus_dev *PIOS_EXBUS_AllocDev(void);
/**
 * @brief Validate an EX Bus device struct
 * @param[in] dev Pointer to device structure
 * @return true if device is valid, false otherwise
 */
static bool PIOS_EXBUS_ValidateDev(struct pios_exbus_dev *dev);
/**
 * @brief Update the CRC of the current frame with a new byte
 * @param[in] dev Pointer to device structure
 * @param[in] value New byte to be added to CRC
 */
static void PIOS_EXBUS_UpdateCRC(struct pios_exbus_dev *dev, uint8_t value);
/**
 * @brief Serial receive callback
 * @param[in] context Pointer to device structure
 * @param[in] buf Pointer to receive buffer
 * @param[in] buf_len Length of receive buffer
 * @param[out] headroom Remaining headroom in local buffer
 * @param[out] task_woken Whether to force a yield
 * @return Number of bytes consumed
 */
static uint16_t PIOS_EXBUS_RxCallback(uintptr_t context, uint8_t *buf,
	uint16_t buf_len, uint16_t *headroom, bool *task_woken);
/**
 * @brief Parse the current frame (checking CRC) and unpack channel contents
 * @param[in] dev Pointer to device structure
 */
static void PIOS_EXBUS_ParseFrame(struct pios_exbus_dev *dev);
/**
 * @brief Reset all channels
 * @param[in] dev Pointer to device structure
 * @param[in] val Value each channel will be set to
 */
static void PIOS_EXBUS_ResetChannels(struct pios_exbus_dev *dev, uint16_t val);
/**
 * @brief Get a channel value
 * @param[in] id Pointer to device structure
 * @param[in] channel Channel to grab, 0 based
 */
static int32_t PIOS_EXBUS_Read(uintptr_t id, uint8_t channel);
/**
 * @brief Supervisor task to handle failsafe and frame timeout
 * @ param[in] id Pointer to device structure
 */
void PIOS_EXBUS_Supervisor(uintptr_t id);

/* Private Variables */

/* Public Variables */
const struct pios_rcvr_driver pios_exbus_rcvr_driver = {
	.read = PIOS_EXBUS_Read
};

/* Implementation */

int32_t PIOS_EXBUS_Init(uintptr_t *exbus_id,
	const struct pios_com_driver *driver, uintptr_t lower_id, struct pios_com_dev *com_id)
{
	if (!driver || !lower_id)
		return -1;
	struct pios_exbus_dev *dev = PIOS_EXBUS_AllocDev();
	if (dev == NULL)
		return -2;

	*exbus_id = (uintptr_t)dev;
	dev->com_id = com_id;

	(driver->bind_rx_cb)(lower_id, PIOS_EXBUS_RxCallback, *exbus_id);

	if (!PIOS_RTC_RegisterTickCallback(PIOS_EXBUS_Supervisor, *exbus_id)) {
		PIOS_DEBUG_Assert(0);
	}

	return 0;
}

static struct pios_exbus_dev *PIOS_EXBUS_AllocDev(void)
{
	struct pios_exbus_dev *dev = PIOS_malloc(sizeof(*dev));
	if (dev == NULL)
		return NULL;

	memset(dev, 0, sizeof(*dev));

	dev->magic = PIOS_EXBUS_MAGIC;

	return dev;
}

static bool PIOS_EXBUS_ValidateDev(struct pios_exbus_dev *dev)
{
	return dev && dev->magic == PIOS_EXBUS_MAGIC;
}

static void PIOS_EXBUS_UpdateCRC(struct pios_exbus_dev *dev, uint8_t data)
{
	if (!PIOS_EXBUS_ValidateDev(dev))
		return;

	data ^= (uint8_t)(dev->crc) & (uint8_t)(0xFF);
	data ^= data << 4;
	dev->crc = ((((uint16_t)data << 8) | ((dev->crc & 0xFF00) >> 8)) ^ (uint8_t)(data >> 4) ^ ((uint16_t)data << 3));
}

static uint16_t crc_ccitt_update(uint16_t crc, uint8_t data)
{
	data ^= (uint8_t)(crc) & (uint8_t)(0xFF);
	data ^= data << 4;
	return ((((uint16_t)data << 8) | ((crc & 0xFF00) >> 8))
	^ (uint8_t)(data >> 4)
	^ ((uint16_t)data << 3));
}

static uint16_t get_crc16z(uint8_t *p, uint16_t len)
{
	uint16_t crc16_data = 0;
	while (len--) {
		crc16_data = crc_ccitt_update(crc16_data, p[0]);
		p++;
	}
	return(crc16_data);
}

#define POLY 0x07
static uint8_t update_crc(uint8_t crc, uint8_t crc_seed)
{
	uint8_t crc_u = crc;
	crc_u ^= crc_seed;

	for (int i = 0; i < 8; i++)
		crc_u = ( crc_u & 0x80 ) ? POLY ^ ( crc_u << 1 ) : ( crc_u << 1 );

	return crc_u;
}

static uint8_t crc8(uint8_t *crc, uint8_t crc_lenght)
{
	uint8_t crc_up = 0;

	for (int c = 0; c < crc_lenght; c++)
		crc_up = update_crc(crc[c], crc_up);

	return crc_up;
}

static void PIOS_EXBUS_SendTelem(struct pios_exbus_dev *dev, uint8_t packet_id)
{
	uint8_t buf[128] = {0, };
	struct exbus_packet *packet = (struct exbus_packet *)buf;
	packet->sync = PIOS_EXBUS_SYNC_REPLY;
	packet->bus_state = PIOS_EXBUS_BUS_RELEASE;
	packet->length = sizeof(struct exbus_packet);
	packet->id = packet_id;

	struct exbus_data_block *data = (struct exbus_data_block *)packet->data;
	data->id = 0x3a;

	struct ex_telemetry_packet *telem = (struct ex_telemetry_packet *)data->data;
	telem->sync = 0x1f;
	telem->manufacturer = 0xa410;
	telem->device = 0xf3fc;
	telem->reserved = 0;
	telem->length = sizeof(struct ex_telemetry_packet) - 2; /* excluding first 2 bytes */
	data->length = sizeof(struct ex_telemetry_packet);

	if (dev->telem_count++ < 10 || !(dev->telem_count % 10)) {
		static int desc_num = 0;
		// send text
		telem->type = 0;

		struct ex_telemetry_text *text = (struct ex_telemetry_text *)telem->data;
		if (desc_num++ % 2) {
			text->id = 1;
			text->desc_length = 3;
			text->unit_length = 1;
			text->data[0] = 'L';
			text->data[1] = 'o';
			text->data[2] = 'L';
			text->data[3] = 'm'; // unit
		} else {
			text->id = 0;
			text->desc_length = 6;
			text->unit_length = 0;
			text->data[0] = 'd';
			text->data[1] = 'R';
			text->data[2] = 'o';
			text->data[3] = 'n';
			text->data[4] = 'i';
			text->data[5] = 'n';
		}

		telem->length += sizeof(struct ex_telemetry_text) + text->desc_length + text->unit_length;
		data->length += sizeof(struct ex_telemetry_text) + text->desc_length + text->unit_length;
	} else {
		// send data
		telem->type = 1;

		struct ex_telemetry_data *field = (struct ex_telemetry_data *)telem->data;
		field->id = 1;
		field->type = 8; // int30_t
		uint32_t *val = (uint32_t *)field->data;
		// TODO: handle sign properly (just mask out the scale bits)
		//*val = dev->telem_count & 0x1fff;
		*val = 1000;
		/*if (dev->telem_count < 0)
			*val |= 1 << 31;*/

		telem->length += sizeof(struct ex_telemetry_data) + 4;
		data->length += sizeof(struct ex_telemetry_data) + 4;
	}
	/* CRC includes length byte (which is not included in the length value) */
	data->data[data->length] = crc8(&data->data[1], telem->length + 1);
	data->length += 1;

	packet->length += sizeof(struct exbus_data_block) + data->length;

	uint16_t *crc = (uint16_t *)&buf[packet->length];
	packet->length += sizeof(uint16_t);
	*crc = get_crc16z(buf, packet->length - sizeof(uint16_t));

	PIOS_COM_SendBufferNonBlocking((uintptr_t)dev->com_id, buf, packet->length);
}

static void PIOS_EXBUS_UnpackPacket(struct pios_exbus_dev *dev)
{
	struct exbus_packet *packet = (struct exbus_packet *)dev->rx_buffer;
	struct exbus_data_block *data = (struct exbus_data_block *)packet->data;
	switch (data->id) {
	case 0x31: // channels
		PIOS_EXBUS_ParseFrame(dev);
		break;
	case 0x3a: // telemetry request
		PIOS_EXBUS_SendTelem(dev, packet->id);
		break;
	default:
		break;
	}
}

static uint16_t PIOS_EXBUS_RxCallback(uintptr_t context, uint8_t *buf,
	uint16_t buf_len, uint16_t *headroom, bool *task_woken)
{
	*task_woken = false;

	struct pios_exbus_dev *dev = (struct pios_exbus_dev *)context;
	if (!PIOS_EXBUS_ValidateDev(dev))
		return PIOS_RCVR_INVALID;

	dev->rx_timer = 0;
	uint16_t consumed = 0;

	for (int i = 0; i < buf_len; i++) {
		if (dev->state != PIOS_EXBUS_STATE_DISCARD)
			dev->rx_buffer[dev->rx_buffer_pos++] = buf[i];
		PIOS_EXBUS_UpdateCRC(dev, buf[i]);
		consumed++;

		switch (dev->state) {
		case PIOS_EXBUS_STATE_IDLE:
			switch (buf[i]) {
			case PIOS_EXBUS_SYNC_REPLY:
			case PIOS_EXBUS_SYNC_REQUEST:
			case PIOS_EXBUS_SYNC_CHAN:
				dev->state = PIOS_EXBUS_STATE_BUS;
				break;
			default:
				dev->rx_buffer_pos = 0;
				dev->crc = 0;
				continue;
			}
			break;
		case PIOS_EXBUS_STATE_BUS:
			if (buf[i] == 0x01)
				dev->bus_state = PIOS_EXBUS_BUS_RELEASE;
			else
				dev->bus_state = PIOS_EXBUS_BUS_HOLD;
			dev->state = PIOS_EXBUS_STATE_LEN;
			break;
		case PIOS_EXBUS_STATE_LEN:
			dev->frame_length = buf[i];
			if (dev->frame_length > PIOS_EXBUS_RXBUF_LEN || dev->rx_buffer[0] == PIOS_EXBUS_SYNC_REPLY)
				dev->state = PIOS_EXBUS_STATE_DISCARD;
			else
				dev->state = PIOS_EXBUS_STATE_FILLBUF;
			break;
		case PIOS_EXBUS_STATE_FILLBUF:
			if (dev->rx_buffer_pos >= (dev->frame_length - 1))
				dev->state = PIOS_EXBUS_STATE_LAST;
			break;
		case PIOS_EXBUS_STATE_DISCARD:
			if (dev->rx_buffer_pos++ >= dev->frame_length)
				dev->state = PIOS_EXBUS_STATE_IDLE;
			break;
		case PIOS_EXBUS_STATE_LAST:
			if (dev->crc) {
				dev->error_count++;
			} else {
				dev->success_count++;
				PIOS_EXBUS_UnpackPacket(dev);
				if (!dev->failsafe_timer) // an rx packet
					*task_woken = true;
			}
			dev->rx_buffer_pos = 0;
			dev->crc = 0;
			dev->state = PIOS_EXBUS_STATE_IDLE;
			break;
		}
	}

	if (headroom) {
		if (dev->state == PIOS_EXBUS_STATE_DISCARD)
			*headroom = PIOS_EXBUS_RXBUF_LEN;
		else
			*headroom = PIOS_EXBUS_RXBUF_LEN - dev->rx_buffer_pos;
	}

	return consumed;
}

static void PIOS_EXBUS_ParseFrame(struct pios_exbus_dev *dev)
{
	const uint8_t nchans = dev->rx_buffer[5] / 2;
	uint16_t *chan = (uint16_t *)&dev->rx_buffer[6];
	for (int i = 0; i < nchans; i++)
		dev->channels[i] = *chan++;
	dev->failsafe_timer = 0;
}

static int32_t PIOS_EXBUS_Read(uintptr_t id, uint8_t channel)
{
	struct pios_exbus_dev *dev = (struct pios_exbus_dev *)id;
	if (!PIOS_EXBUS_ValidateDev(dev))
		return PIOS_RCVR_INVALID;
	return dev->channels[channel];
}

static void PIOS_EXBUS_ResetChannels(struct pios_exbus_dev *dev, uint16_t val)
{
	if (!PIOS_EXBUS_ValidateDev(dev))
		return;

	for (int i = 0; i < NELEMENTS(dev->channels); i++)
		dev->channels[i] = val;
}

void PIOS_EXBUS_Supervisor(uintptr_t id)
{
	struct pios_exbus_dev *dev = (struct pios_exbus_dev *)id;
 	if (!PIOS_EXBUS_ValidateDev(dev))
		return;
	/*
	 * RTC runs this callback at 625 Hz (T=1.6 ms)
	 * Frame period is up to 30 ms
	 * We will reset the rx buffer if nothing is received for 8 ms (5 ticks)
	 * We will failsafe if 4 frames (low rate) are lost
	 * (4 * 21) / 1.6 ~ 53 ticks
	 */
	if (++dev->rx_timer >= 5)
		dev->rx_buffer_pos = 0;
	if (++dev->failsafe_timer >= 75)
		PIOS_EXBUS_ResetChannels(dev, PIOS_RCVR_TIMEOUT);
}

#endif /* defined(PIOS_INCLUDE_EXBUS) */

/**
 * @}
 * @}
 */
