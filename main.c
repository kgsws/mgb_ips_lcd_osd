#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "reg.h"
#include "defs.h"
#include "spicmd.h"
#include "gfx.h"
#include "osd.h"
#include "config.h"

// timing is not exact due to RC oscillator and rounding
// timer clock: 23.520 MHz / 12
#define TIMER_MS	63576

__xdata uint16_t tick_ms;
__xdata uint8_t tick_tmp;
__xdata void (*ticker)();

static uint8_t adc_tick = ADC_MEAS_DELAY - 1;

#ifdef ADC_BACKLIGHT
static __xdata uint16_t adc_val[2] = {0xFFFF, 0}; // battery starts as 'charged', brightness starts at 'lowest'
static __xdata uint8_t adc_idx;
static __xdata uint16_t adc_old;
static __xdata uint8_t pwm_now = 0xFF;
static __xdata uint8_t pwm_old = 0xFF;
#else
static __xdata uint16_t adc_val[1] = {0xFFFF};
#endif

static __xdata uint8_t adc_lowbat;

//
// IRQ

void timer0_irq() __naked __interrupt(IRQ_TIMER0)
{
	// This function is set 'naked'!
	// That means this can be the only active code!

	if(adc_tick >= 2)
	{
		adc_tick--;
		if(adc_tick == 1)
		{
			// enable ADC
			uint8_t tmp = 0b10000000;
#ifdef ADC_BACKLIGHT
			if(adc_idx)
				tmp |= ADC_BACKLIGHT;
#endif
			ADC_CONTR = tmp;
		}
	} else
	if(adc_tick == 1)
	{
		// start ADC measure
		uint8_t tmp = 0b11000000;
#ifdef ADC_BACKLIGHT
		if(adc_idx)
			tmp |= ADC_BACKLIGHT;
#endif
		ADC_CONTR = tmp;
		adc_tick = 0;
	} else
	if(ADC_CONTR & 0b00100000)
	{
#ifdef ADC_BACKLIGHT
		__xdata uint16_t *ptr = adc_val + adc_idx;
#else
		__xdata uint16_t *ptr = adc_val;
#endif
		uint16_t value;

		value = (uint16_t)ADC_RES << 8;
		value |= ADC_RESL;

		*ptr -= *ptr / 4;
		*ptr += value / 4;
		value = *ptr;
#ifdef ADC_BACKLIGHT
		if(adc_idx)
		{
			uint16_t diff;

			if(adc_old < value)
				diff = value - adc_old;
			else
				diff = adc_old - value;

			if(diff >= 0x0100)
			{
				adc_old = value;

				if(value <= ADC_BL_RAW_LO)
					pwm_now = gamma_table[0];
				else
				if(value >= ADC_BL_RAW_HI)
					pwm_now = gamma_table[sizeof(gamma_table) - 1];
				else
				{
					value = ((value - ADC_BL_RAW_LO) >> 8) * (sizeof(gamma_table) - 2);
					value /= (uint8_t)((ADC_BL_RAW_HI - ADC_BL_RAW_LO) >> 8);
					pwm_now = gamma_table[1 + value];
				}
			}
		} else
#endif
		{
			if(value <= ADC_LOW_BAT)
				// this flag is never unset
				adc_lowbat = 1;
		}

		ADC_CONTR = 0;
		adc_tick = ADC_MEAS_DELAY;
#ifdef ADC_BACKLIGHT
		adc_idx = !adc_idx;
#endif
	}

#ifdef ADC_BACKLIGHT
	if(pwm_now != pwm_old)
	{
		pwm_old = pwm_now;
		spicmd_backlight(pwm_now);
	}
#endif

	if(adc_lowbat && config.battery)
	{
		if(!(tick_ms & 0x1FF))
			spicmd_show_battery(!(tick_ms & 0x200));
	}

	// tick
	tick_ms++;
	ticker();

	// return opcode due to 'naked' function
	__asm
		reti
	__endasm;
}

//
// logo scroll

static void tick_logo_wait()
{
	if(tick_ms < 5100)
		return;

	// set correct palette
	config_apply_palette(config.palette);

	// init OSD
	osd_init();
}

static void tick_logo_up()
{
	// scroll custom logo up

	if(tick_ms & CFG_LOGO_TICK_MASK)
		// this is logo scroll speed
		return;

	tick_tmp--;
	gfx_show_logo(tick_tmp);

	if(tick_tmp)
		return;

	ticker = tick_logo_wait;
}

static inline void setup_logo()
{
	uint16_t *pc;

#ifdef CFG_SHOW_CUSTOM_LOGO
	ticker = tick_logo_up;
	tick_tmp = CFG_LOGO_START_Y;
#else
	ticker = tick_logo_wait;
#endif

	// FPGA shows number '1' in top left corner every boot
	// hide this by faking palette color
	pc = config.pal[config.palette].color;

	spicmd.palette.color[0] = pc[0];
	spicmd.palette.color[1] = pc[0];
	spicmd.palette.color[2] = pc[0];
#ifdef CFG_HIDE_CART_LOGO
	spicmd.palette.color[3] = pc[0];
#else
	spicmd.palette.color[3] = pc[3];
#endif
	spicmd_palette_full();

	// set OSD color to logo background color
	spicmd_palette_osd();

	// clear OSD with custom color
	spicmd_osd_clear(0xFF);

#ifdef CFG_SHOW_CUSTOM_LOGO
	// show OSD
	OUT_OSD = 0;
#endif

	// start timer
	TR0 = 1;
}

//
// setup

static inline void init_io()
{
	P0M0 = 0b00000000;
	P0M1 = 0b11111111;

	// !IN_COLOR; !OSD; CLK; MISO; MOSI; !CS; PWR?; ADC_BAT
	P1M0 = 0b01111110;
	P1M1 = 0b10000001;

	P2M0 = 0b00000000;
	P2M1 = 0b11111111;

	// !IN_LIGHT; !IN_A; -; -; !IN_B; !IN_SELECT; -; -
	P3M0 = 0b00000000;
	P3M1 = 0b11111111;

	// some kind of power supply enable?
	P11 = 0;

	// hide OSD
	OUT_OSD = 1;

	// SPI
	OUT_CS = 1;
	SPCTL = 0b11110000;

	// ADC setup
	ADCTIM = 0x3F;
	ADCCFG = 0b00001111;
}

static inline void init_system()
{
	// setup logo scroll
	setup_logo();

	// timer
	TMOD = 0b00000000;
	TL0 = TIMER_MS & 0xFF;
	TH0 = TIMER_MS >> 8;
	ET0 = 1;

	// enable IRQ
	EA = 1;
}

//
// MAIN

void main()
{
	init_io();
	config_init();
	init_system();

	while(1)
	{
		// enter IDLE
		PCON = 1;
	}
}
