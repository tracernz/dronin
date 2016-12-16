/**
 ******************************************************************************
 * @file       mtk_cfg.c
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

#include "openpilot.h"
#include "pios_com.h"
#include "pios_thread.h"

#if !defined(PIOS_GPS_MINIMAL)

#include "GPS.h"
#include "NMEA.h"

#include "gpsposition.h"
#include "modulesettings.h"

enum mtk_command {
	PMTK_CMD_HOT_START = 101,
	PMTK_CMD_WARM_START = 102,
	PMTK_CMD_COLD_START = 103,
	PMTK_CMD_FULL_COLD_START = 104,
	PMTK_SET_NMEA_BAUDRATE = 251,
	PMTK_API_SET_FIX_CTL = 300,
	PMTK_API_SET_SBAS_ENABLED = 313,
	PMTK_API_SET_NMEA_OUTPUT = 314,
	PMTK_API_SET_USER_OPTION = 390,
	PMTK_API_GET_USER_OPTION = 490,
};

enum mtk_sentence {
	NMEA_SEN_GLL = 0, /* GPGLL interval - Geographic Position - Latitude longitude */
	NMEA_SEN_RMC = 1, /* GPRMC interval - Recomended Minimum Specific GNSS Sentence */
	NMEA_SEN_VTG = 2, /* GPVTG interval - Course Over Ground and Ground Speed */
	NMEA_SEN_GGA = 3, /* GPGGA interval - GPS Fix Data */
	NMEA_SEN_GSA = 4, /* GPGSA interval - GNSS DOPS and Active Satellites */
	NMEA_SEN_GSV = 5, /* GPGSV interval - GNSS Satellites in View */
	NMEA_SEN_GRS = 6, /* GPGRS interval - GNSS Range Residuals */
	NMEA_SEN_GST = 7, /* GPGST interval - GNSS Pseudorange Erros Statistics */
	NMEA_SEN_MALM = 13, /* PMTKALM interval - GPS almanac information */
	NMEA_SEN_MEPH = 14, /* PMTKEPH interval - GPS ephmeris information */
	NMEA_SEN_MDGP = 15, /* PMTKDGP interval - GPS differential correction information */
	NMEA_SEN_MDBG = 16, /* PMTKDBG interval – MTK debug information */
	NMEA_SEN_ZDA = 17, /* GPZDA interval – Time & Date */
	NMEA_SEN_MCHN = 18, /* PMTKCHN interval – GPS channel status  */
};

enum mtk_sentence_frequency {
	MTK_FREQ_DISABLED = 0,
	MTK_FREQ_EVERY_FIX = 1,
	MTK_FREQ_EVERY_2FIX = 2,
	MTK_FREQ_EVERY_3FIX = 3,
	MTK_FREQ_EVERY_4FIX = 4,
	MTK_FREQ_EVERY_5FIX = 5,
};

char *write_int(char *buf, unsigned val, unsigned base, unsigned min_digits)
{
	unsigned t = val;
	unsigned digits = 1;
	while (t /= base)
		digits++;
	if (digits < min_digits) {
		memset(buf, '0', min_digits - digits);
		digits = min_digits;
	}
	
	char *p = buf + digits - 1;
	do {
		int d = val % base;
		*p-- = (d < 10) ? '0' + d : 'A' - 10 + d;
		val /= base;
	} while (val);

	return buf + digits;
}

void mtk_cfg_set_fix_period(uintptr_t gps_port, unsigned period)
{
	if (period < 200)
		period = 200; /* hardware limitation */
	else if (period > 1000)
		period = 1000; /* arbitrary limitation */

	char buf[50] = "$PMTK300,\0";
	char *ptr = buf + strlen(buf);

	ptr = write_int(ptr, period, 10, 0);
	
	const char *end = ",0,0,0,0*";
	strcpy(ptr, end);
	ptr += strlen(end);
	
	uint8_t checksum = NMEA_checksum(buf, NULL);
	ptr = write_int(ptr, checksum, 16, 2);
	*ptr = '\0';

	PIOS_COM_SendString(gps_port, buf);
}

void mtk_cfg_set_messages(uintptr_t gps_port)
{
	char buf[50] = "$PMTK314\0";
	char *ptr = buf + strlen(buf);

	const uint8_t rates[] = {
		MTK_FREQ_DISABLED, /* NMEA_SEN_GLL */
		MTK_FREQ_EVERY_FIX, /* NMEA_SEN_RMC */
		MTK_FREQ_EVERY_FIX, /* NMEA_SEN_VTG */
		MTK_FREQ_EVERY_FIX, /* NMEA_SEN_GGA */
		MTK_FREQ_EVERY_FIX, /* NMEA_SEN_GSA */
		MTK_FREQ_EVERY_5FIX, /* NMEA_SEN_GSV */
		MTK_FREQ_DISABLED, /* NMEA_SEN_GRS */
		MTK_FREQ_DISABLED, /* NMEA_SEN_GST */
		MTK_FREQ_DISABLED, /* NMEA_SEN_MALM */
		MTK_FREQ_DISABLED, /* NMEA_SEN_MEPH */
		MTK_FREQ_DISABLED, /* NMEA_SEN_MDGP */
		MTK_FREQ_DISABLED, /* NMEA_SEN_MDBG */
		MTK_FREQ_EVERY_5FIX, /* NMEA_SEN_ZDA */
		MTK_FREQ_DISABLED, /* NMEA_SEN_MCHN */
	};

	for (int i = 0; i < NELEMENTS(rates); i++) {
		*ptr++ = ',';
		ptr = write_int(ptr, rates[i], 10, 0);
	}

	*ptr++ = '*';

	uint8_t checksum = NMEA_checksum(buf, NULL);
	ptr = write_int(ptr, checksum, 16, 2);
	*ptr = '\0';

	PIOS_COM_SendString(gps_port, buf);
}

void mtk_cfg_set_baudrate(uintptr_t gps_port, ModuleSettingsGPSSpeedOptions baudrate)
{
	const char *msg_4800 = "$PMTK251,4800*14\r\n";
	const char *msg_9600 = "$PMTK251,9600*17\r\n";
	const char *msg_19200 = "$PMTK251,19200*22\r\n";
	const char *msg_38400 = "$PMTK251,38400*27\r\n";
	const char *msg_57600 = "$PMTK251,57600*2C\r\n";
	const char *msg_115200 = "$PMTK251,115200*1F\r\n";
	
	const char *msg;
	uint32_t baud;

	switch (baudrate) {
	case MODULESETTINGS_GPSSPEED_4800:
		msg = msg_4800;
		baud = 4800;
		break;
	case MODULESETTINGS_GPSSPEED_9600:
		msg = msg_9600;
		baud = 9600;
		break;
	case MODULESETTINGS_GPSSPEED_19200:
		msg = msg_19200;
		baud = 19200;
		break;
	case MODULESETTINGS_GPSSPEED_38400:
		msg = msg_38400;
		baud = 38400;
		break;
	case MODULESETTINGS_GPSSPEED_57600:
		msg = msg_57600;
		baud = 57600;
		break;
	case MODULESETTINGS_GPSSPEED_115200:
		msg = msg_115200;
		baud = 115200;
		break;
	default:
		return;
	}

	/* send the message at all common baudrates */
	GPSSendStringAtAllBaudrates(msg);
	
	/* better change the port now we're done */
	PIOS_COM_ChangeBaud(gps_port, baud);
}

#endif /* !defined(PIOS_GPS_MINIMAL) */

/**
 * @}
 * @}
 */
