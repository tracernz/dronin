#ifndef _PIOS_FLIGHT_CONFIG_H
#define _PIOS_FLIGHT_CONFIG_H

// Top level PIOS Subsystems
#define PIOS_INCLUDE_ADC
#define PIOS_INCLUDE_ANNUNC
#define PIOS_INCLUDE_BL_HELPER
#define PIOS_INCLUDE_COM
#define PIOS_INCLUDE_EXTI
#define PIOS_INCLUDE_GPS
#define PIOS_INCLUDE_GPS_NMEA_PARSER
#define PIOS_INCLUDE_GPS_UBX_PARSER
#define PIOS_INCLUDE_IAP
#define PIOS_INCLUDE_INITCALL
#define PIOS_INCLUDE_PPM
#define PIOS_INCLUDE_RCVR
#define PIOS_INCLUDE_RTC
#define PIOS_INCLUDE_SERVO
#define PIOS_INCLUDE_SYS
#define PIOS_INCLUDE_TIM
#define PIOS_INCLUDE_USART
#define PIOS_INCLUDE_USB
#define PIOS_INCLUDE_USB_CDC
#define PIOS_INCLUDE_USB_HID
#define PIOS_INCLUDE_WDG

#define WDG_STATS_DIAGNOSTICS

#define PIOS_INCLUDE_FLASH
#define PIOS_INCLUDE_FLASH_INTERNAL
#define PIOS_INCLUDE_LOGFS_SETTINGS

#define PIOS_INCLUDE_CHIBIOS

// Resource limits
#define CPULOAD_LIMIT_WARNING		80
#define CPULOAD_LIMIT_CRITICAL		95
#define IRQSTACK_LIMIT_WARNING		150
#define IRQSTACK_LIMIT_CRITICAL		80
#define HEAP_LIMIT_WARNING		1000
#define HEAP_LIMIT_CRITICAL		500

// Serial receiver protocols
#define PIOS_INCLUDE_CROSSFIRE
#define PIOS_INCLUDE_DSM
#define PIOS_INCLUDE_GCSRCVR
#define PIOS_INCLUDE_HSUM
#define PIOS_INCLUDE_HOTT
#define PIOS_INCLUDE_IBUS
#define PIOS_INCLUDE_SBUS
#define PIOS_INCLUDE_SRXL

// Other protocols and things
#define PIOS_INCLUDE_FRSKY_SENSOR_HUB
#define PIOS_INCLUDE_FRSKY_SPORT_TELEMETRY
#define PIOS_INCLUDE_OPENLOG
#define PIOS_INCLUDE_STORM32BGC
#define PIOS_INCLUDE_MSP_BRIDGE
#define PIOS_INCLUDE_4WAY
#define PIOS_INCLUDE_ACTUATOR_ANNUNCIATOR

#endif /* _PIOS_FLIGHT_CONFIG_H */
