#
# Rules to (help) build the F1xx device support.
#

#
# Directory containing this makefile
#
PIOS_DEVLIB			:=	$(dir $(lastword $(MAKEFILE_LIST)))

#
# Hardcoded linker script names for now
#
LINKER_SCRIPTS_APP	 =	$(PIOS_DEVLIB)/sections_chibios.ld

#
# Compiler options implied by the F1xx
#
CDEFS 				+= -DUSE_STDPERIPH_DRIVER
ARCHFLAGS			+= -mcpu=cortex-m3 -march=armv7e-m -mfloat-abi=soft
FLOATABI			+= soft

#
# PIOS device library source and includes
#
SRC					+=	$(filter-out $(PIOS_DEVLIB)vectors_stm32f1xx.c $(PIOS_DEVLIB)startup.c $(PIOS_DEVLIB)pios_i2c.c $(PIOS_DEVLIB)pios_i2c_alt.c, $(wildcard $(PIOS_DEVLIB)*.c))
EXTRAINCDIRS		+=	$(PIOS_DEVLIB)/inc

#
# ST Peripheral library
#
PERIPHLIB			 =	$(PIOS_DEVLIB)/Libraries/STM32F10x_StdPeriph_Driver
EXTRAINCDIRS		+=	$(PERIPHLIB)/inc
SRC					+=	$(wildcard $(PERIPHLIB)/src/*.c)

#
# ST USB library
#
USBLIB				 =	$(PIOS_DEVLIB)/Libraries/STM32_USB-FS-Device_Driver
USBLIB_SRC			 =	usb_core.c usb_init.c usb_int.c usb_mem.c usb_regs.c usb_sil.c
EXTRAINCDIRS		+=	$(USBLIB)/inc
SRC					+=	$(addprefix $(USBLIB)/src/,$(USBLIB_SRC))
