/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 *
 * @file       vector_stm32f1xx.c
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      C based vectors for F4
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
 *
 * Additional note on redistribution: The copyright and license notices above
 * must be maintained in each individual source file that is a derivative work
 * of this source file; otherwise redistribution is prohibited.
 */

/** interrupt handler */
typedef const void	(vector)(void);

/** default interrupt handler */
static void
default_io_handler(void)
{
	for (;;);
}

/** prototypes an interrupt handler */
#define HANDLER(_name)	extern vector _name __attribute__((weak, alias("default_io_handler")))

HANDLER(WWDG_IRQHandler);                 // Window WatchDog
HANDLER(PVD_IRQHandler);                  // PVD through EXTI Line detection
HANDLER(TAMPER_IRQHandler);               // Tamper
HANDLER(RTC_IRQHandler);                  // RTC global
HANDLER(FLASH_IRQHandler);                // FLASH global
HANDLER(RCC_IRQHandler);                  // RCC global
HANDLER(EXTI0_IRQHandler);                // EXTI Line0
HANDLER(EXTI1_IRQHandler);                // EXTI Line1
HANDLER(EXTI2_IRQHandler);                // EXTI Line2
HANDLER(EXTI3_IRQHandler);                // EXTI Line3
HANDLER(EXTI4_IRQHandler);                // EXTI Line4
HANDLER(DMA1_Channel1_IRQHandler);        // DMA1 Channel1 global
HANDLER(DMA1_Channel2_IRQHandler);        // DMA1 Channel2 global
HANDLER(DMA1_Channel3_IRQHandler);        // DMA1 Channel3 global
HANDLER(DMA1_Channel4_IRQHandler);        // DMA1 Channel4 global
HANDLER(DMA1_Channel5_IRQHandler);        // DMA1 Channel5 global
HANDLER(DMA1_Channel6_IRQHandler);        // DMA1 Channel6 global
HANDLER(DMA1_Channel7_IRQHandler);        // DMA1 Channel7 global
HANDLER(ADC1_2_IRQHandler);               // ADC1 and ADC2 global
HANDLER(USB_HP_CAN1_TX_IRQHandler);       // USB High Priority or CAN TX
HANDLER(USB_LP_CAN1_RX0_IRQHandler);      // USB Low Priority or CAN RX0
HANDLER(CAN1_RX1_IRQHandler);             // CAN1 RX1
HANDLER(CAN1_SCE_IRQHandler);             // CAN1 SCE
HANDLER(EXTI9_5_IRQHandler);              // EXTI Line[9:5]
HANDLER(TIM1_BRK_IRQHandler);             // TIM1 Break
HANDLER(TIM1_UP_IRQHandler);              // TIM1 Update
HANDLER(TIM1_TRG_COM_IRQHandler);         // TIM1 Trigger and Commutation
HANDLER(TIM1_CC_IRQHandler);              // TIM1 Capture Compare
HANDLER(TIM2_IRQHandler);                 // TIM2 global
HANDLER(TIM3_IRQHandler);                 // TIM3 global
HANDLER(TIM4_IRQHandler);                 // TIM4 global
HANDLER(I2C1_EV_IRQHandler);              // I2C1 event
HANDLER(I2C1_ER_IRQHandler);              // I2C1 error
HANDLER(I2C2_EV_IRQHandler);              // I2C2 event
HANDLER(I2C2_ER_IRQHandler);              // I2C2 error
HANDLER(SPI1_IRQHandler);                 // SPI1 global
HANDLER(SPI2_IRQHandler);                 // SPI2 global
HANDLER(USART1_IRQHandler);               // USART1 global
HANDLER(USART2_IRQHandler);               // USART2 global
HANDLER(USART3_IRQHandler);               // USART3 global
HANDLER(EXTI15_10_IRQHandler);            // EXTI Line[15:10]
HANDLER(RTCAlarm_IRQHandler);             // RTC alarm through EXTI line
HANDLER(USBWakeUp_IRQHandler);            // USB wakeup from suspend through EXTI line interrupt

/** stm32f1xx interrupt vector table */
vector *io_vectors[] __attribute__((section(".io_vectors"))) = {
	WWDG_IRQHandler,                   // 0:  Window WatchDog
	PVD_IRQHandler,                    // 1:  PVD through EXTI Line detection
	TAMPER_IRQHandler,                 // 2:  Tamper
	RTC_IRQHandler,                    // 3:  RTC global
	FLASH_IRQHandler,                  // 4:  FLASH global
	RCC_IRQHandler,                    // 5:  RCC global
	EXTI0_IRQHandler,                  // 6:  EXTI Line0
	EXTI1_IRQHandler,                  // 7:  EXTI Line1
	EXTI2_IRQHandler,                  // 8:  EXTI Line2
	EXTI3_IRQHandler,                  // 9:  EXTI Line3
	EXTI4_IRQHandler,                  // 10: EXTI Line4
	DMA1_Channel1_IRQHandler,          // 11: DMA1 Channel1 global
	DMA1_Channel2_IRQHandler,          // 12: DMA1 Channel2 global
	DMA1_Channel3_IRQHandler,          // 13: DMA1 Channel3 global
	DMA1_Channel4_IRQHandler,          // 14: DMA1 Channel4 global
	DMA1_Channel5_IRQHandler,          // 15: DMA1 Channel5 global
	DMA1_Channel6_IRQHandler,          // 16: DMA1 Channel6 global
	DMA1_Channel7_IRQHandler,          // 17: DMA1 Channel7 global
	ADC1_2_IRQHandler,                 // 18: ADC1 and ADC2 global
	USB_HP_CAN1_TX_IRQHandler,         // 19: USB High Priority or CAN TX
	USB_LP_CAN1_RX0_IRQHandler,        // 20: USB Low Priority or CAN RX0
	CAN1_RX1_IRQHandler,               // 21: CAN RX1
	CAN1_SCE_IRQHandler,               // 22: CAN SCE
	EXTI9_5_IRQHandler,                // 23: EXTI Line[9:5]
	TIM1_BRK_IRQHandler,               // 24: TIM1 Break
	TIM1_UP_IRQHandler,                // 25: TIM1 Update
	TIM1_TRG_COM_IRQHandler,           // 26: TIM1 Trigger and Commutation
	TIM1_CC_IRQHandler,                // 27: TIM1 Capture Compare
	TIM2_IRQHandler,                   // 28: TIM2 global
	TIM3_IRQHandler,                   // 29: TIM3 global
	TIM4_IRQHandler,                   // 30: TIM4 global
	I2C1_EV_IRQHandler,                // 31: I2C1 event
	I2C1_ER_IRQHandler,                // 32: I2C1 error
	I2C2_EV_IRQHandler,                // 33: I2C2 event
	I2C2_ER_IRQHandler,                // 34: I2C2 error
	SPI1_IRQHandler,                   // 35: SPI1 global
	SPI2_IRQHandler,                   // 36: SPI2 global
	USART1_IRQHandler,                 // 37: USART1 global
	USART2_IRQHandler,                 // 38: USART2 global
	USART3_IRQHandler,                 // 39: USART3 global
	EXTI15_10_IRQHandler,              // 40: EXTI Line[15:10]
	RTCAlarm_IRQHandler,               // 41: RTC alarm through EXTI line
	USBWakeUp_IRQHandler,              // 42: USB wakeup from suspend through EXTI line interrupt
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	(vector *)0xF108F85F,              // 50: BootRAM according to ST startup file, undocumented magic :(
};

/**
 * @}
 */