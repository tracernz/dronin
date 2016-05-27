/**
 ******************************************************************************
 * @file       
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @addtogroup Modules dRonin Modules
 * @{
 * @addtogroup EscBlHeliBridge ESC to BlHeliSuite Bridge
 * @{
 * @brief BlHeliSuite 4w-if protocol bridge
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

#include "openpilot.h"
#include "pios_hal.h"
#include "pios_thread.h"
#include "pios_modules.h"
#include "pios_crc.h"

#if defined(PIOS_INCLUDE_ESCBLHELIBRIDGE)

#define ntohs(v) (				\
	(((v) & 0xFF00) >> 8) |			\
	(((v) & 0x00FF) << 8))

#define STACK_SIZE_BYTES  1024
#define TASK_PRIORITY     PIOS_THREAD_PRIO_LOW

#define BLHELI_INTERFACE_NAME "dRonin"
#define BLHELI_PROTOCOL_VER   106

enum blheli_syncbyte {
	BLHELI_SYNCBYTE_RX = 0x2f,
	BLHELI_SYNCBYTE_TX = 0x2e,
};

enum blheli_command {
	BLHELI_CMD_IF_KEEPALIVE =   0x30,
	BLHELI_CMD_IF_PROTO_VER =   0x31,
	BLHELI_CMD_IF_NAME =        0x32,
	BLHELI_CMD_IF_VER =         0x33,
	BLHELI_CMD_IF_EXIT =        0x34,
	BLHELI_CMD_DEV_RESET =      0x35,
	BLHELI_CMD_DEV_INITFLASH =  0x37,
	BLHELI_CMD_DEV_ERASEALL =   0x38,
	BLHELI_CMD_DEV_ERASEPAGE =  0x39,
	BLHELI_CMD_DEV_READ =       0x3a,
	BLHELI_CMD_DEV_WRITE =      0x3b,
	BLHELI_CMD_DEV_CLKLOW =     0x3c,
	BLHELI_CMD_DEV_EEP_READ =   0x3d,
	BLHELI_CMD_DEV_EEP_WRITE =  0x3e,
	BLHELI_CMD_IF_SETMODE =     0x3f,
};

enum blheli_ack {
	BLHELI_ACK_OK           = 0x00,
	BLHELI_ERR_IF_UNKNOWN =   0x01,
	BLHELI_ERR_IF_BADCMD =    0x02,
	BLHELI_ERR_IF_BADCRC =    0x03,
	BLHELI_ERR_IF_VERIFY =    0x04,
	BLHELI_ERR_DEV_BADCMD =   0x05,
	BLHELI_ERR_DEV_FAILCMD =  0x06,
	BLHELI_ERR_DEV_UNKNOWN =  0x07,
	BLHELI_ACK_IF_BADCHAN =   0x08,
	BLHELI_ACK_IF_BADPARAM =  0X09,
	BLHELI_ACK_DEV_ERROR =    0X0f,
};

enum blheli_mode {
	BLHELI_MODE_SILC2 =  0,
	BLHELI_MODE_SILBLB = 1,
	BLHELI_MODE_ATMBLB = 2,
	BLHELI_MODE_ATMSK =  3,
	BLHELI_MODE_NUM
};

struct blheli_header {
	uint8_t sync_byte;
	uint8_t command;
	uint16_t address;
	uint8_t payload_len;
	uint8_t payload[0];
} __attribute__((packed));

static bool module_enabled = false;

static uint8_t rxbuf[256 + sizeof(struct blheli_header) + 2];
static uint8_t txbuf[256 + sizeof(struct blheli_header) + 2];

static enum blheli_mode current_mode;

static int32_t escBlHeliBridge_Initialise(void);
static int32_t escBlHeliBridge_Start(void);
MODULE_INITCALL(escBlHeliBridge_Initialise, escBlHeliBridge_Start);

static void escBlHeliBridge_Task(void *parameters);
static void send_error(enum blheli_command cmd, enum blheli_ack err);
static void send_protocol_version(void);
static void send_interface_name(void);
static void send_interface_version(void);
static void send_interface_mode(uint8_t mode);
static void send_keepalive(void);
static void send_exit(void);
static void process_dev_init(int esc);

static int32_t escBlHeliBridge_Initialise(void)
{
	if (pios_com_escbridge_id && PIOS_Modules_IsEnabled(PIOS_MODULE_ESCBLHELIBRIDGE))
		module_enabled = true;

	return 0;
}

static int32_t escBlHeliBridge_Start(void)
{
	if (!module_enabled)
		return -1;

	struct pios_thread *task = PIOS_Thread_Create(escBlHeliBridge_Task, "escBlHeliBridge", STACK_SIZE_BYTES, NULL, TASK_PRIORITY);
	if (!task)
		return -2;
	TaskMonitorAdd(TASKINFO_RUNNING_ESCBLHELIBRIDGE, task);

	return 0;
}

static void escBlHeliBridge_Task(void *parameters)
{
	(void)parameters;

	while (true) {
		uint16_t len = PIOS_COM_ReceiveBuffer(pios_com_escbridge_id, &rxbuf[0], 1, PIOS_QUEUE_TIMEOUT_MAX);
		if (!len)
			continue;

		if (rxbuf[0] != BLHELI_SYNCBYTE_RX)
			continue;

		len = 0;
		while (len < sizeof(struct blheli_header) - 1)
			len += PIOS_COM_ReceiveBuffer(pios_com_escbridge_id, &rxbuf[1 + len], sizeof(struct blheli_header) - 1 - len, PIOS_QUEUE_TIMEOUT_MAX);


		struct blheli_header *msg = (struct blheli_header *)rxbuf;
		int payload_len = msg->payload_len == 0 ? 256 : msg->payload_len;

		DEBUG_PRINTF(2, "cmd: 0x%x, len: %d\n", msg->command, payload_len);

		len = 0;
		while (len < payload_len + 2)
			len += PIOS_COM_ReceiveBuffer(pios_com_escbridge_id, msg->payload + len, payload_len + 2 - len, PIOS_QUEUE_TIMEOUT_MAX);
		
		/*uint16_t *rx_crc = (uint16_t *)&msg->payload[payload_len];
		*rx_crc = ntohs(*rx_crc);
		uint16_t crc = PIOS_CRC16_CCITT_updateCRC(0, txbuf, sizeof(struct blheli_header) + payload_len + 2);
		if (crc != 0) {
			send_error(rxbuf[1], BLHELI_ERR_IF_BADCRC);
			DEBUG_PRINTF(2, "bad crc: 0x%x\n", crc);
			continue;
		}*/

		switch (msg->command) {
		case BLHELI_CMD_IF_KEEPALIVE:
			send_keepalive();
			break;
		case BLHELI_CMD_IF_PROTO_VER:
			send_protocol_version();
			break;
		case BLHELI_CMD_IF_NAME:
			send_interface_name();
			break;
		case BLHELI_CMD_IF_VER:
			send_interface_version();
			break;
		case BLHELI_CMD_IF_EXIT:
			send_exit();
			break;
		case BLHELI_CMD_DEV_RESET:
			send_error(rxbuf[1], BLHELI_ERR_IF_BADCMD);
			break;
		case BLHELI_CMD_DEV_INITFLASH:
			process_dev_init(msg->payload[0]);
			break;
		case BLHELI_CMD_DEV_ERASEALL:
		case BLHELI_CMD_DEV_ERASEPAGE:
		case BLHELI_CMD_DEV_READ:
		case BLHELI_CMD_DEV_WRITE:
		case BLHELI_CMD_DEV_CLKLOW:
		case BLHELI_CMD_DEV_EEP_READ:
		case BLHELI_CMD_DEV_EEP_WRITE:
			send_error(rxbuf[1], BLHELI_ERR_IF_BADCMD);
			break;
		case BLHELI_CMD_IF_SETMODE:
			send_interface_mode(msg->payload[0]);
			break;
		default:
			send_error(rxbuf[1], BLHELI_ERR_IF_BADCMD);
		}
	}
}

static void send_message(enum blheli_command cmd, enum blheli_ack ack, uint16_t payload_len)
{
	struct blheli_header *msg = (struct blheli_header *)txbuf;
	msg->sync_byte = BLHELI_SYNCBYTE_TX;
	msg->command = cmd;
	msg->address = 0;
	msg->payload_len = payload_len >= 256 ? 0 : payload_len;
	uint8_t *footer = &msg->payload[payload_len];
	*footer++ = ack;
	uint16_t crc = PIOS_CRC16_CCITT_updateCRC(0, txbuf, sizeof(struct blheli_header) + payload_len + 1);
	*footer++ = crc >> 8;
	*footer++ = crc & 0xff;
	PIOS_COM_SendBuffer(pios_com_escbridge_id, txbuf, sizeof(struct blheli_header) + payload_len + 3);
}

static void send_error(enum blheli_command cmd, enum blheli_ack err)
{
	struct blheli_header *msg = (struct blheli_header *)txbuf;
	msg->payload[0] = 0;
	send_message(cmd, err, 1);
}

static void send_protocol_version(void)
{
	struct blheli_header *msg = (struct blheli_header *)txbuf;
	msg->payload[0] = BLHELI_PROTOCOL_VER;
	send_message(BLHELI_CMD_IF_PROTO_VER, BLHELI_ACK_OK, 1);
}

static void send_interface_name(void)
{
	struct blheli_header *msg = (struct blheli_header *)txbuf;
	int len = strlen(BLHELI_INTERFACE_NAME);
	if (len >= 256)
		return;
	memcpy(msg->payload, BLHELI_INTERFACE_NAME, len);
	send_message(BLHELI_CMD_IF_NAME, BLHELI_ACK_OK, len);
}

static void send_interface_version(void)
{
	struct blheli_header *msg = (struct blheli_header *)txbuf;
	msg->payload[0] = 1;
	msg->payload[1] = 0;
	send_message(BLHELI_CMD_IF_VER, BLHELI_ACK_OK, 2);
}

static void send_interface_mode(uint8_t mode)
{
	switch (mode) {
	case BLHELI_MODE_SILBLB:
		current_mode = mode;
		break;
	case BLHELI_MODE_ATMBLB:
	case BLHELI_MODE_ATMSK:
	default:
		DEBUG_PRINTF(2, "bad mode: %d\n", mode);
		send_error(BLHELI_CMD_IF_SETMODE, BLHELI_ACK_IF_BADPARAM);
		return;
	}

	struct blheli_header *msg = (struct blheli_header *)txbuf;
	msg->payload[0] = mode;
	send_message(BLHELI_CMD_IF_SETMODE, BLHELI_ACK_OK, 1);
}

static void send_keepalive(void)
{
	struct blheli_header *msg = (struct blheli_header *)txbuf;
	msg->payload[0] = 0;
	send_message(BLHELI_CMD_IF_KEEPALIVE, BLHELI_ACK_OK, 1);
}

static void send_exit(void)
{
	struct blheli_header *msg = (struct blheli_header *)txbuf;
	msg->payload[0] = 0;
	send_message(BLHELI_CMD_IF_EXIT, BLHELI_ACK_OK, 1);
}

static void process_dev_init(int esc)
{
	struct blheli_header *msg = (struct blheli_header *)txbuf;
	msg->payload[0] = 0x30;
	msg->payload[1] = 0xf3;
	msg->payload[2] = 0;
	msg->payload[3] = current_mode;
	send_message(BLHELI_CMD_DEV_INITFLASH, BLHELI_ACK_OK, 4);
}

#endif // defined(PIOS_INCLUDE_ESCBLHELIBRIDGE)

/**
 * @}
 * @}
 */
