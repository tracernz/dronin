/**
 ******************************************************************************
 * @addtogroup TauLabsTargets Tau Labs Targets
 * @{
 * @addtogroup Sparky Tau Labs Sparky support files
 * @{
 *
 * @file       pios_board.c
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2012-2014
 * @author     dRonin, http://dronin.org Copyright (C) 2015
 * @brief      The board specific initialization routines
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
 */

/* Pull in the board-specific static HW definitions.
 * Including .c files is a bit ugly but this allows all of
 * the HW definitions to be const and static to limit their
 * scope.
 *
 * NOTE: THIS IS THE ONLY PLACE THAT SHOULD EVER INCLUDE THIS FILE
 */

#include "board_hw_defs.c"

#include <pios.h>
#include <pios_hal.h>
#include <openpilot.h>
#include <uavobjectsinit.h>
#include "hwsparky.h"
#include "manualcontrolsettings.h"
#include "modulesettings.h"


#define PIOS_COM_CAN_RX_BUF_LEN 256
#define PIOS_COM_CAN_TX_BUF_LEN 256

bool external_mag_fail;

uintptr_t pios_com_aux_id;
uintptr_t pios_com_can_id;
uintptr_t pios_uavo_settings_fs_id;
uintptr_t pios_waypoints_settings_fs_id;
uintptr_t pios_internal_adc_id;
uintptr_t pios_can_id;
uintptr_t pios_com_openlog_logging_id;

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */

#include <pios_board_info.h>

void PIOS_Board_Init(void)
{

	/* Delay system */
	PIOS_DELAY_Init();

	const struct pios_board_info *bdinfo = &pios_board_info_blob;

#if defined(PIOS_INCLUDE_LED)
	const struct pios_led_cfg *led_cfg = PIOS_BOARD_HW_DEFS_GetLedCfg(bdinfo->board_rev);
	PIOS_Assert(led_cfg);
	PIOS_LED_Init(led_cfg);
#endif	/* PIOS_INCLUDE_LED */

#if defined(PIOS_INCLUDE_CAN)
	if (PIOS_CAN_Init(&pios_can_id, &pios_can_cfg) != 0)
		PIOS_HAL_Panic(PIOS_LED_ALARM, PIOS_HAL_PANIC_CAN);

	uint8_t *rx_buffer = (uint8_t *) PIOS_malloc(PIOS_COM_CAN_RX_BUF_LEN);
	uint8_t *tx_buffer = (uint8_t *) PIOS_malloc(PIOS_COM_CAN_TX_BUF_LEN);
	PIOS_Assert(rx_buffer);
	PIOS_Assert(tx_buffer);
	if (PIOS_COM_Init(&pios_com_can_id, &pios_can_com_driver, pios_can_id,
	                  rx_buffer, PIOS_COM_CAN_RX_BUF_LEN,
	                  tx_buffer, PIOS_COM_CAN_TX_BUF_LEN))
		PIOS_HAL_Panic(PIOS_LED_ALARM, PIOS_HAL_PANIC_CAN);

	pios_com_bridge_id = pios_com_can_id;
#endif

#if defined(PIOS_INCLUDE_FLASH)
	/* Inititialize all flash drivers */
	if (PIOS_Flash_Internal_Init(&pios_internal_flash_id, &flash_internal_cfg) != 0)
		PIOS_HAL_Panic(PIOS_LED_ALARM, PIOS_HAL_PANIC_FLASH);

	/* Register the partition table */
	const struct pios_flash_partition *flash_partition_table;
	uint32_t num_partitions;
	flash_partition_table = PIOS_BOARD_HW_DEFS_GetPartitionTable(bdinfo->board_rev, &num_partitions);
	PIOS_FLASH_register_partition_table(flash_partition_table, num_partitions);

	/* Mount all filesystems */
	if (PIOS_FLASHFS_Logfs_Init(&pios_uavo_settings_fs_id, &flashfs_internal_settings_cfg, FLASH_PARTITION_LABEL_SETTINGS) != 0)
		PIOS_HAL_Panic(PIOS_LED_ALARM, PIOS_HAL_PANIC_FILESYS);
	if (PIOS_FLASHFS_Logfs_Init(&pios_waypoints_settings_fs_id, &flashfs_internal_waypoints_cfg, FLASH_PARTITION_LABEL_WAYPOINTS) != 0)
		PIOS_HAL_Panic(PIOS_LED_ALARM, PIOS_HAL_PANIC_FILESYS);

#if defined(ERASE_FLASH)
	PIOS_FLASHFS_Format(pios_uavo_settings_fs_id);
#endif

#endif	/* PIOS_INCLUDE_FLASH */

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

	/* Initialize UAVObject libraries */
	UAVObjInitialize();

	/* Initialize the alarms library. Reads RCC reset flags */
	AlarmsInitialize();
	PIOS_RESET_Clear(); // Clear the RCC reset flags after use.

	/* Initialize the hardware UAVOs */
	HwSparkyInitialize();
	ModuleSettingsInitialize();

#if defined(PIOS_INCLUDE_RTC)
	/* Initialize the real-time clock and its associated tick */
	PIOS_RTC_Init(&pios_rtc_main_cfg);
#endif

	/* Initialize watchdog as early as possible to catch faults during init
	 * but do it only if there is no debugger connected
	 */
	if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) == 0) {
		PIOS_WDG_Init();
	}

	/* Set up pulse timers */
	//inputs

	//outputs
	PIOS_TIM_InitClock(&tim_1_cfg);
	PIOS_TIM_InitClock(&tim_2_cfg);
	PIOS_TIM_InitClock(&tim_3_cfg);
	PIOS_TIM_InitClock(&tim_15_cfg);
	PIOS_TIM_InitClock(&tim_16_cfg);
	PIOS_TIM_InitClock(&tim_17_cfg);

	/* IAP System Setup */
	PIOS_IAP_Init();
	uint16_t boot_count = PIOS_IAP_ReadBootCount();
	if (boot_count < 3) {
		PIOS_IAP_WriteBootCount(++boot_count);
		AlarmsClear(SYSTEMALARMS_ALARM_BOOTFAULT);
	} else {
		/* Too many failed boot attempts, force hw config to defaults */
		HwSparkySetDefaults(HwSparkyHandle(), 0);
		ModuleSettingsSetDefaults(ModuleSettingsHandle(), 0);
		AlarmsSet(SYSTEMALARMS_ALARM_BOOTFAULT, SYSTEMALARMS_ALARM_CRITICAL);
	}

#if defined(PIOS_INCLUDE_USB)
	/* Initialize board specific USB data */
	PIOS_USB_BOARD_DATA_Init();

	/* Flags to determine if various USB interfaces are advertised */
	bool usb_hid_present = false;

#if defined(PIOS_INCLUDE_USB_CDC)
	bool usb_cdc_present = false;
	if (PIOS_USB_DESC_HID_CDC_Init()) {
		PIOS_Assert(0);
	}
	usb_hid_present = true;
	usb_cdc_present = true;
#else
	if (PIOS_USB_DESC_HID_ONLY_Init()) {
		PIOS_Assert(0);
	}
	usb_hid_present = true;
#endif

	uintptr_t pios_usb_id;
	PIOS_USB_Init(&pios_usb_id, PIOS_BOARD_HW_DEFS_GetUsbCfg(bdinfo->board_rev));

#if defined(PIOS_INCLUDE_USB_CDC)

	uint8_t hw_usb_vcpport;
	/* Configure the USB VCP port */
	HwSparkyUSB_VCPPortGet(&hw_usb_vcpport);

	if (!usb_cdc_present) {
		/* Force VCP port function to disabled if we haven't advertised VCP in our USB descriptor */
		hw_usb_vcpport = HWSPARKY_USB_VCPPORT_DISABLED;
	}

	PIOS_HAL_ConfigureCDC(hw_usb_vcpport, pios_usb_id, &pios_usb_cdc_cfg);
#endif	/* PIOS_INCLUDE_USB_CDC */

#if defined(PIOS_INCLUDE_USB_HID)
	/* Configure the usb HID port */
	uint8_t hw_usb_hidport;
	HwSparkyUSB_HIDPortGet(&hw_usb_hidport);

	if (!usb_hid_present) {
		/* Force HID port function to disabled if we haven't advertised HID in our USB descriptor */
		hw_usb_hidport = HWSPARKY_USB_HIDPORT_DISABLED;
	}

	PIOS_HAL_ConfigureHID(hw_usb_hidport, pios_usb_id, &pios_usb_hid_cfg);

#endif	/* PIOS_INCLUDE_USB_HID */
#endif	/* PIOS_INCLUDE_USB */

	/* Configure the IO ports */
	
	PIOS_HAL_ConfigurePort(HWSHARED_PORTTYPES_I2C,  // port type protocol
	        NULL,                                   // usart_port_cfg
	        NULL,                                   // com_driver
	        &pios_i2c_internal_id,                  // i2c_id
	        &pios_i2c_internal_cfg,                 // i2c_cfg 
	        NULL,                                   // ppm_cfg
	        NULL,                                   // pwm_cfg
	        PIOS_LED_ALARM,                         // led_id
	        NULL,                                   // dsm_cfg
	        0,                                      // dsm_mode
	        NULL);                                  // sbus_cfg

	HwSparkyDSMxModeOptions hw_DSMxMode;
	HwSparkyDSMxModeGet(&hw_DSMxMode);

	/* Configure main USART port */
	uint8_t hw_mainport;
	HwSparkyMainPortGet(&hw_mainport);
	
	PIOS_HAL_ConfigurePort(hw_mainport,          // port type protocol
	        &pios_main_usart_cfg,                // usart_port_cfg
	        &pios_usart_com_driver,              // com_driver
	        NULL,                                // i2c_id
	        NULL,                                // i2c_cfg 
	        NULL,                                // ppm_cfg
	        NULL,                                // pwm_cfg
	        PIOS_LED_ALARM,                      // led_id
	        &pios_main_dsm_aux_cfg,              // dsm_cfg
	        hw_DSMxMode,                         // dsm_mode
	        NULL);                               // sbus_cfg

	/* Configure FlexiPort */
	uint8_t hw_flexiport;
	HwSparkyFlexiPortGet(&hw_flexiport);
	
	PIOS_HAL_ConfigurePort(hw_flexiport,         // port type protocol
	        &pios_flexi_usart_cfg,               // usart_port_cfg
	        &pios_usart_com_driver,              // com_driver
	        &pios_i2c_flexi_id,                  // i2c_id
	        &pios_i2c_flexi_cfg,                 // i2c_cfg 
	        NULL,                                // ppm_cfg
	        NULL,                                // pwm_cfg
	        PIOS_LED_ALARM,                      // led_id
	        &pios_flexi_dsm_aux_cfg,             // dsm_cfg
	        hw_DSMxMode,                         // dsm_mode
	        NULL);                               // sbus_cfg

	/* Configure the rcvr port */
	uint8_t hw_rcvrport;
	HwSparkyRcvrPortGet(&hw_rcvrport);
	
	PIOS_HAL_ConfigurePort(hw_rcvrport,          // port type protocol
	        &pios_rcvr_usart_cfg,                // usart_port_cfg
	        &pios_usart_com_driver,              // com_driver
	        NULL,                                // i2c_id 
	        NULL,                                // i2c_cfg
	        &pios_ppm_cfg,                       // ppm_cfg
	        NULL,                                // pwm_cfg
	        PIOS_LED_ALARM,                      // led_id
	        &pios_rcvr_dsm_aux_cfg,              // dsm_cfg
	        hw_DSMxMode,                         // dsm_mode
	        NULL);                               // sbus_cfg

#if defined(PIOS_INCLUDE_GCSRCVR)
	GCSReceiverInitialize();
	uintptr_t pios_gcsrcvr_id;
	PIOS_GCSRCVR_Init(&pios_gcsrcvr_id);
	uintptr_t pios_gcsrcvr_rcvr_id;
	if (PIOS_RCVR_Init(&pios_gcsrcvr_rcvr_id, &pios_gcsrcvr_rcvr_driver, pios_gcsrcvr_id)) {
		PIOS_Assert(0);
	}
	pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_GCS] = pios_gcsrcvr_rcvr_id;
#endif	/* PIOS_INCLUDE_GCSRCVR */

	uint8_t hw_outport;
	uint8_t number_of_pwm_outputs;
	uint8_t number_of_adc_ports;
	bool use_pwm_in;
	HwSparkyOutPortGet(&hw_outport);
	switch (hw_outport) {
	case HWSPARKY_OUTPORT_PWM10:
		number_of_pwm_outputs = 10;
		number_of_adc_ports = 0;
		use_pwm_in = false;
		break;
	case HWSPARKY_OUTPORT_PWM82ADC:
		number_of_pwm_outputs = 8;
		number_of_adc_ports = 2;
		use_pwm_in = false;
		break;
	case HWSPARKY_OUTPORT_PWM73ADC:
		number_of_pwm_outputs = 7;
		number_of_adc_ports = 3;
		use_pwm_in = false;
		break;
	case HWSPARKY_OUTPORT_PWM9PWM_IN:
		number_of_pwm_outputs = 9;
		use_pwm_in = true;
		number_of_adc_ports = 0;
		break;
	case HWSPARKY_OUTPORT_PWM72ADCPWM_IN:
		number_of_pwm_outputs = 7;
		use_pwm_in = true;
		number_of_adc_ports = 2;
		break;
	default:
		PIOS_Assert(0);
		break;
	}
#ifndef PIOS_DEBUG_ENABLE_DEBUG_PINS
#ifdef PIOS_INCLUDE_SERVO
	pios_servo_cfg.num_channels = number_of_pwm_outputs;
	PIOS_Servo_Init(&pios_servo_cfg);
#endif
#else
	PIOS_DEBUG_Init(&pios_tim_servo_all_channels, NELEMENTS(pios_tim_servo_all_channels));
#endif

#if defined(PIOS_INCLUDE_ADC)
	if (number_of_adc_ports > 0) {
		internal_adc_cfg.adc_pin_count = number_of_adc_ports;
		uint32_t internal_adc_id;
		if (PIOS_INTERNAL_ADC_Init(&internal_adc_id, &internal_adc_cfg) < 0)
			PIOS_Assert(0);
		PIOS_ADC_Init(&pios_internal_adc_id, &pios_internal_adc_driver, internal_adc_id);
	}
#endif /* PIOS_INCLUDE_ADC */
#if defined(PIOS_INCLUDE_PWM)
	if (use_pwm_in) {
		if (number_of_adc_ports > 0)
			pios_pwm_cfg.channels = &pios_tim_rcvrport_pwm[1];
		uintptr_t pios_pwm_id;
		PIOS_PWM_Init(&pios_pwm_id, &pios_pwm_cfg);

		uintptr_t pios_pwm_rcvr_id;
		if (PIOS_RCVR_Init(&pios_pwm_rcvr_id, &pios_pwm_rcvr_driver, pios_pwm_id)) {
			PIOS_Assert(0);
		}
		pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_PWM] = pios_pwm_rcvr_id;
	}
#endif	/* PIOS_INCLUDE_PWM */
	PIOS_WDG_Clear();
	PIOS_DELAY_WaitmS(200);
	PIOS_WDG_Clear();

#if defined(PIOS_INCLUDE_MPU)

	uint8_t selected_mag;
	HwSparkyMagnetometerGet(&selected_mag);

	if (selected_mag == HWSPARKY_MAGNETOMETER_INTERNAL)
		pios_mpu9150_cfg.use_internal_mag = true;
	else
		pios_mpu9150_cfg.use_internal_mag = false;


	pios_mpu_dev_t mpu_dev = NULL;
	int retval = PIOS_MPU_I2C_Init(&mpu_dev, pios_i2c_internal_id, &pios_mpu9150_cfg);
	if (retval != 0)
		PIOS_HAL_Panic(PIOS_LED_ALARM, PIOS_HAL_PANIC_IMU);

	// To be safe map from UAVO enum to driver enum
	uint8_t hw_gyro_range;
	HwSparkyGyroRangeGet(&hw_gyro_range);
	switch (hw_gyro_range) {
	case HWSPARKY_GYRORANGE_250:
		PIOS_MPU_SetGyroRange(PIOS_MPU_SCALE_250_DEG);
		break;
	case HWSPARKY_GYRORANGE_500:
		PIOS_MPU_SetGyroRange(PIOS_MPU_SCALE_500_DEG);
		break;
	case HWSPARKY_GYRORANGE_1000:
		PIOS_MPU_SetGyroRange(PIOS_MPU_SCALE_1000_DEG);
		break;
	case HWSPARKY_GYRORANGE_2000:
		PIOS_MPU_SetGyroRange(PIOS_MPU_SCALE_2000_DEG);
		break;
	}

	uint8_t hw_accel_range;
	HwSparkyAccelRangeGet(&hw_accel_range);
	switch (hw_accel_range) {
	case HWSPARKY_ACCELRANGE_2G:
		PIOS_MPU_SetAccelRange(PIOS_MPU_SCALE_2G);
		break;
	case HWSPARKY_ACCELRANGE_4G:
		PIOS_MPU_SetAccelRange(PIOS_MPU_SCALE_4G);
		break;
	case HWSPARKY_ACCELRANGE_8G:
		PIOS_MPU_SetAccelRange(PIOS_MPU_SCALE_8G);
		break;
	case HWSPARKY_ACCELRANGE_16G:
		PIOS_MPU_SetAccelRange(PIOS_MPU_SCALE_16G);
		break;
	}

	uint8_t hw_mpu9150_dlpf;
	HwSparkyMPU9150DLPFGet(&hw_mpu9150_dlpf);
	uint16_t bandwidth = \
                                (hw_mpu9150_dlpf == HWSPARKY_MPU9150DLPF_256) ? 256 : \
                                (hw_mpu9150_dlpf == HWSPARKY_MPU9150DLPF_188) ? 188 : \
                                (hw_mpu9150_dlpf == HWSPARKY_MPU9150DLPF_98) ? 68 : \
                                (hw_mpu9150_dlpf == HWSPARKY_MPU9150DLPF_42) ? 42 : \
                                (hw_mpu9150_dlpf == HWSPARKY_MPU9150DLPF_20) ? 20 : \
                                (hw_mpu9150_dlpf == HWSPARKY_MPU9150DLPF_10) ? 10 : \
                                (hw_mpu9150_dlpf == HWSPARKY_MPU9150DLPF_5) ? 5 : \
                                188;
	PIOS_MPU_SetGyroBandwidth(bandwidth);

	uint8_t hw_mpu9150_samplerate;
	HwSparkyMPU9150RateGet(&hw_mpu9150_samplerate);
	uint16_t mpu9150_samplerate = \
	                              (hw_mpu9150_samplerate == HWSPARKY_MPU9150RATE_200) ? 200 : \
	                              (hw_mpu9150_samplerate == HWSPARKY_MPU9150RATE_333) ? 333 : \
	                              (hw_mpu9150_samplerate == HWSPARKY_MPU9150RATE_500) ? 500 : \
	                              (hw_mpu9150_samplerate == HWSPARKY_MPU9150RATE_666) ? 666 : \
	                              (hw_mpu9150_samplerate == HWSPARKY_MPU9150RATE_1000) ? 1000 : \
	                              (hw_mpu9150_samplerate == HWSPARKY_MPU9150RATE_2000) ? 2000 : \
	                              (hw_mpu9150_samplerate == HWSPARKY_MPU9150RATE_4000) ? 4000 : \
	                              (hw_mpu9150_samplerate == HWSPARKY_MPU9150RATE_8000) ? 8000 : \
	                              pios_mpu9150_cfg.default_samplerate;
	PIOS_MPU_SetSampleRate(mpu9150_samplerate);

#endif /* PIOS_INCLUDE_MPU */

	//I2C is slow, sensor init as well, reset watchdog to prevent reset here
	PIOS_WDG_Clear();

#if defined(PIOS_INCLUDE_HMC5883)
	{
		uint8_t selected_mag;
		HwSparkyMagnetometerGet(&selected_mag);

		external_mag_fail = false;
		
		if (selected_mag == HWSPARKY_MAGNETOMETER_EXTERNALI2CFLEXIPORT) {
			if (PIOS_HMC5883_Init(pios_i2c_flexi_id, &pios_hmc5883_external_cfg) == 0) {
				if (PIOS_HMC5883_Test() == 0) {
					// External mag configuration was successful

					// setup sensor orientation
					uint8_t ext_mag_orientation;
					HwSparkyExtMagOrientationGet(&ext_mag_orientation);

					enum pios_hmc5883_orientation hmc5883_orientation = \
						(ext_mag_orientation == HWSPARKY_EXTMAGORIENTATION_TOP0DEGCW)      ? PIOS_HMC5883_TOP_0DEG      : \
						(ext_mag_orientation == HWSPARKY_EXTMAGORIENTATION_TOP90DEGCW)     ? PIOS_HMC5883_TOP_90DEG     : \
						(ext_mag_orientation == HWSPARKY_EXTMAGORIENTATION_TOP180DEGCW)    ? PIOS_HMC5883_TOP_180DEG    : \
						(ext_mag_orientation == HWSPARKY_EXTMAGORIENTATION_TOP270DEGCW)    ? PIOS_HMC5883_TOP_270DEG    : \
						(ext_mag_orientation == HWSPARKY_EXTMAGORIENTATION_BOTTOM0DEGCW)   ? PIOS_HMC5883_BOTTOM_0DEG   : \
						(ext_mag_orientation == HWSPARKY_EXTMAGORIENTATION_BOTTOM90DEGCW)  ? PIOS_HMC5883_BOTTOM_90DEG  : \
						(ext_mag_orientation == HWSPARKY_EXTMAGORIENTATION_BOTTOM180DEGCW) ? PIOS_HMC5883_BOTTOM_180DEG : \
						(ext_mag_orientation == HWSPARKY_EXTMAGORIENTATION_BOTTOM270DEGCW) ? PIOS_HMC5883_BOTTOM_270DEG : \
						pios_hmc5883_external_cfg.Default_Orientation;
					PIOS_HMC5883_SetOrientation(hmc5883_orientation);
				}
				else
					external_mag_fail = true;  // External HMC5883 Test Failed
			}
			else
				external_mag_fail = true;  // External HMC5883 Init Failed
		}
	}
#endif /* PIOS_INCLUDE_HMC5883 */

	//I2C is slow, sensor init as well, reset watchdog to prevent reset here
	PIOS_WDG_Clear();

#if defined(PIOS_INCLUDE_MS5611)
	PIOS_MS5611_Init(&pios_ms5611_cfg, pios_i2c_internal_id);
	if (PIOS_MS5611_Test() != 0)
		PIOS_HAL_Panic(PIOS_LED_ALARM, PIOS_HAL_PANIC_BARO);
#endif

#if defined(PIOS_INCLUDE_GPIO)
	PIOS_GPIO_Init();
#endif

	/* Make sure we have at least one telemetry link configured or else fail initialization */
	PIOS_Assert(pios_com_telem_serial_id || pios_com_telem_usb_id);
}

/**
 * @}
 */
