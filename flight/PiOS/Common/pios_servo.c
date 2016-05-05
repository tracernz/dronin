/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_SERVO RC Servo Functions
 * @brief Code to do set RC servo output
 * @{
 *
 * @file       pios_servo.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2014-2015
 * @author     dRonin, http://dRonin.org, Copyright (C) 2016
 * @brief      RC Servo routines (STM32 dependent)
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

/* Project Includes */
#include "pios.h"
#include "pios_servo_priv.h"
#include "pios_tim_priv.h"
#include "misc_math.h"

/* Private variables */
static const struct pios_servo_cfg *servo_cfg;

//! The counter rate for the channel, used to calculate compare values.
static uint32_t *output_channel_resolution;  // The clock rate for that timer
enum channel_mode {REGULAR_PWM, SYNC_PWM} *output_channel_mode;

/* Private function prototypes */
static uint32_t max_timer_clock(TIM_TypeDef *timer);

/**
* Initialise Servos
*/
int32_t PIOS_Servo_Init(const struct pios_servo_cfg *cfg)
{
	uintptr_t tim_id;
	if (PIOS_TIM_InitChannels(&tim_id, cfg->channels, cfg->num_channels, NULL, 0)) {
		return -1;
	}

	/* Store away the requested configuration */
	servo_cfg = cfg;

	/* Configure the channels to be in output compare mode */
	for (uint8_t i = 0; i < cfg->num_channels; i++) {
		const struct pios_tim_channel *chan = &cfg->channels[i];

		/* Set up for output compare function */
		switch(chan->timer_chan) {
			case TIM_Channel_1:
				TIM_OC1Init(chan->timer, (TIM_OCInitTypeDef *)&cfg->tim_oc_init);
				TIM_OC1PreloadConfig(chan->timer, TIM_OCPreload_Enable);
				break;
			case TIM_Channel_2:
				TIM_OC2Init(chan->timer, (TIM_OCInitTypeDef *)&cfg->tim_oc_init);
				TIM_OC2PreloadConfig(chan->timer, TIM_OCPreload_Enable);
				break;
			case TIM_Channel_3:
				TIM_OC3Init(chan->timer, (TIM_OCInitTypeDef *)&cfg->tim_oc_init);
				TIM_OC3PreloadConfig(chan->timer, TIM_OCPreload_Enable);
				break;
			case TIM_Channel_4:
				TIM_OC4Init(chan->timer, (TIM_OCInitTypeDef *)&cfg->tim_oc_init);
				TIM_OC4PreloadConfig(chan->timer, TIM_OCPreload_Enable);
				break;
		}

		TIM_ARRPreloadConfig(chan->timer, ENABLE);
		TIM_CtrlPWMOutputs(chan->timer, ENABLE);
		TIM_Cmd(chan->timer, ENABLE);
	}

	output_channel_resolution = PIOS_malloc(servo_cfg->num_channels * sizeof(typeof(output_channel_resolution)));
	if (output_channel_resolution == NULL) {
		return -1;
	}
	memset(output_channel_resolution, 0, servo_cfg->num_channels * sizeof(typeof(output_channel_resolution)));

	/* Allocate memory for frequency table */
	output_channel_mode = PIOS_malloc(servo_cfg->num_channels * sizeof(typeof(output_channel_mode)));
	if (output_channel_mode == NULL) {
		return -1;
	}
	memset(output_channel_mode, 0, servo_cfg->num_channels * sizeof(typeof(output_channel_mode)));


	return 0;
}

/**
 * @brief PIOS_Servo_SetMode Sets the PWM output frequency and resolution.
 * An output rate of 0 indicates Synchronous updates (e.g. SyncPWM/OneShot), otherwise
 * normal PWM at the specified output rate. SyncPWM uses hardware one-pulse mode.
 * Timer prescaler and related parameters are calculated based on the channel
 * with highest max pulse length on each timer. A deadtime is provided to
 * ensure SyncPWM pulses cannot merge.
 * The information required to convert from us to compare value is cached
 * for each channel (not timer) to facilitate PIOS_Servo_Set
 * @param out_rate array of output rate in Hz, banks elements
 * @param banks maximum number of banks
 * @param channel_max array of max pulse lengths, number of channels elements
 */
void PIOS_Servo_SetMode(const uint16_t *out_rate, const int banks, const uint16_t *channel_max)
{
	if (!servo_cfg || banks > PIOS_SERVO_MAX_BANKS)
		return;

	struct timer_bank {
		TIM_TypeDef *timer;
		uint32_t clk_rate;
		uint16_t max_pulse;
		uint16_t prescaler;
		uint16_t period;
	} timer_banks[PIOS_SERVO_MAX_BANKS];

	memset(&timer_banks, 0, sizeof(timer_banks));
	int bank = 0;

	// find max pulse length for each bank and channel allocation
	for (int i = 0; i < servo_cfg->num_channels && bank < banks; i++) {
		bool new = i > 0;
		const struct pios_tim_channel *chan = &servo_cfg->channels[i];
		/* See if any previous channels use that same timer */
		for (int j = 0; j < i && new; j++) {
			if (chan->timer == servo_cfg->channels[j].timer)
				new = false;
		}
		if (new)
			++bank;

		timer_banks[bank].timer = chan->timer;
		timer_banks[bank].max_pulse = MAX(timer_banks[bank].max_pulse, channel_max[i]);
	}

	// configure timers/banks
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = servo_cfg->tim_base_init;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	for (int i = 0; i < banks; i++) {
		/* Calculate the maximum clock frequency for the timer */
		volatile uint32_t max_tim_clock = max_timer_clock(timer_banks[i].timer);
		// check if valid timer
		if (!max_tim_clock)
			return;

		// output rate of 0 means SyncPWM
		if (out_rate[i] == 0) {
			/* 
			 * Ensure a dead time of 2% + 10 us, e.g. 15us for 250us
			 * long pulses, 50 us for 2000us long pulses
			 */
			volatile float period = 1.02f * timer_banks[i].max_pulse + 10.0f;
			volatile float num_ticks = period / 1e6f * max_tim_clock;
			// assume 16-bit timer
			timer_banks[i].prescaler = num_ticks / 0xffff + 0.5f;
			timer_banks[i].clk_rate = max_tim_clock / (timer_banks[i].prescaler + 1);
			timer_banks[i].period = period / 1e6f * timer_banks[i].clk_rate;

			// select one pulse mode for SyncPWM
			TIM_SelectOnePulseMode(timer_banks[i].timer, TIM_OPMode_Single);
			// Need to invert output polarity for one-pulse mode
			uint16_t inverted_polarity = (servo_cfg->tim_oc_init.TIM_OCPolarity == TIM_OCPolarity_Low) ?
					TIM_OCPolarity_High : TIM_OCPolarity_Low;
			TIM_OC1PolarityConfig(timer_banks[i].timer, inverted_polarity);
			TIM_OC2PolarityConfig(timer_banks[i].timer, inverted_polarity);
			TIM_OC3PolarityConfig(timer_banks[i].timer, inverted_polarity);
			TIM_OC4PolarityConfig(timer_banks[i].timer, inverted_polarity);
		} else {
			volatile float num_ticks = (float)max_tim_clock / (float)out_rate[i];
			// assume 16-bit timer
			timer_banks[i].prescaler = num_ticks / 0xffff + 0.5f;
			timer_banks[i].clk_rate = max_tim_clock / (timer_banks[i].prescaler + 1);
			timer_banks[i].period = (float)timer_banks[i].clk_rate / (float)out_rate[i];

			// de-select one pulse mode in case SyncPWM was previously used
			TIM_SelectOnePulseMode(timer_banks[i].timer, TIM_OPMode_Repetitive);
			// restore polarity in case one-pulse was used
			TIM_OC1PolarityConfig(timer_banks[i].timer, servo_cfg->tim_oc_init.TIM_OCPolarity);
			TIM_OC2PolarityConfig(timer_banks[i].timer, servo_cfg->tim_oc_init.TIM_OCPolarity);
			TIM_OC3PolarityConfig(timer_banks[i].timer, servo_cfg->tim_oc_init.TIM_OCPolarity);
			TIM_OC4PolarityConfig(timer_banks[i].timer, servo_cfg->tim_oc_init.TIM_OCPolarity);
		}

		TIM_TimeBaseStructure.TIM_Prescaler = timer_banks[i].prescaler;
		TIM_TimeBaseStructure.TIM_Period = timer_banks[i].period;

		// Configure this timer appropriately.
		TIM_TimeBaseInit(timer_banks[i].timer, &TIM_TimeBaseStructure);

		/* Configure frequency scaler for all channels that use the same timer */
		for (uint8_t j = 0; j < servo_cfg->num_channels; j++) {
			const struct pios_tim_channel *chan = &servo_cfg->channels[j];
			if (timer_banks[i].timer == chan->timer) {
				/* save the frequency for these channels */
				output_channel_mode[j] = (out_rate[i] == 0) ? SYNC_PWM : REGULAR_PWM;
				output_channel_resolution[j] = timer_banks[i].clk_rate;
			}
		}
	}
}

/**
* Set servo position
* \param[in] Servo Servo number (0->num_channels-1)
* \param[in] Position Servo position in microseconds
*/
void PIOS_Servo_Set(uint8_t servo, float position)
{
	/* Make sure servo exists */
	if (!servo_cfg || servo >= servo_cfg->num_channels) {
		return;
	}

	const struct pios_tim_channel *chan = &servo_cfg->channels[servo];

	/* recalculate the position value based on timer clock rate */
	/* position is in us. */
	float us_to_count = output_channel_resolution[servo] / 1000000.0f;
	position = position * us_to_count;

	// in one-pulse mode, the pulse length is ARR - CCR
	if (output_channel_mode[servo] == SYNC_PWM) {
		position = chan->timer->ARR - position;
	}

	/* Update the position */
	switch(chan->timer_chan) {
		case TIM_Channel_1:
			TIM_SetCompare1(chan->timer, position);
			break;
		case TIM_Channel_2:
			TIM_SetCompare2(chan->timer, position);
			break;
		case TIM_Channel_3:
			TIM_SetCompare3(chan->timer, position);
			break;
		case TIM_Channel_4:
			TIM_SetCompare4(chan->timer, position);
			break;
	}
}

/**
* Update the timer for HPWM/OneShot
*/
void PIOS_Servo_Update(void)
{
	if (!servo_cfg) {
		return;
	}

	for (uint8_t i = 0; i < servo_cfg->num_channels; i++) {
		const struct pios_tim_channel * chan = &servo_cfg->channels[i];

		/* Check for channels that are using synchronous output */
		/* Look for a disabled timer using synchronous output */
		if (!(chan->timer->CR1 & TIM_CR1_CEN)) {
			/* enable it again and reinitialize it */
			TIM_GenerateEvent(chan->timer, TIM_EventSource_Update);
			TIM_Cmd(chan->timer, ENABLE);
		}
	}
}

/**
 * @brief Determines the maximum clock rate for a given timer (i.e. no prescale)
 * @param[in] timer Pointer to the base register of the timer to check
 * @retval Clock frequency in Hz
 */
#if defined(STM32F10X_MD)

static uint32_t max_timer_clock(TIM_TypeDef *timer)
{
	// "The timer clock frequencies are automatically fixed by hardware. There are two cases:
	//    1. if the APB prescaler is 1, the timer clock frequencies are set to the same frequency as
	//    that of the APB domain to which the timers are connected.
	//    2. otherwise, they are set to twice (*2) the frequency of the APB domain to which the
	//    timers are connected."
	if (timer == TIM1 || timer == TIM8) {
		if (PIOS_PERIPHERAL_APB2_CLOCK == PIOS_SYSCLK)
			return PIOS_PERIPHERAL_APB2_CLOCK;
		else
			return PIOS_PERIPHERAL_APB2_CLOCK * 2;
	} else {
		if (PIOS_PERIPHERAL_APB1_CLOCK == PIOS_SYSCLK)
			return PIOS_PERIPHERAL_APB1_CLOCK;
		else
			return PIOS_PERIPHERAL_APB1_CLOCK * 2;
	}

	// TIM6 and TIM7 cannot be used
	return 0;
}

#elif defined(STM32F30X) /* F3 */

static uint32_t max_timer_clock(TIM_TypeDef *timer)
{
	if (timer == TIM2 || timer == TIM3 || timer == TIM4) {
		if (PIOS_PERIPHERAL_APB1_CLOCK == PIOS_SYSCLK) {
			return PIOS_PERIPHERAL_APB1_CLOCK;
		} else {
			//those timers run at double APB1 speed if APB1 prescaler is != 1 which is usually the case
			return PIOS_PERIPHERAL_APB1_CLOCK * 2;
		}
	} else {
		return PIOS_PERIPHERAL_APB2_CLOCK;
	}

	// TIM6 and TIM7 cannot be used
	return 0;
}

#elif defined(STM32F40_41xxx) || defined(STM32F446xx)/*  F4 */

static uint32_t max_timer_clock(TIM_TypeDef *timer)
{
	if (timer == TIM1 || timer == TIM8 || timer == TIM9 || timer == TIM10 || timer == TIM11 )
		return PIOS_PERIPHERAL_APB2_CLOCK;
	else
		return PIOS_PERIPHERAL_APB1_CLOCK;

	// TIM6 and TIM7 cannot be used
	return 0;
}

#else
#error Unsupported microcontroller
#endif

/*
 * @}
 * @}
 */
