#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "reg.h"
#include "defs.h"
#include "spicmd.h"
#include "gfx.h"
#include "config.h"
#include "osd.h"

//
// NOTE:
// Timing is only based on SPI transfer speeds.

#define QUICK_SAVE_DELAY	2000

#define MAIN_SELECT_SCROLL	(OSD_HEIGHT - OSD_MAIN_ICON_Y)
#define GAP_MAIN_ICON_TEXT	(OSD_MAIN_TEXT_Y - OSD_MAIN_ICON_Y)
#define LEN_SELECT_SCROLL	(OSD_MAIN_TEXT_Y - OSD_SIDE_TEXT_Y)
#define LEN_HEX_SCROLL	(OSD_HEIGHT - OSD_SIDE_ICON_Y + HEX_NUMBER_OFFSET)
#define SIDE_Y_START	(OSD_SIDE_ICON_Y + 48)
#define HEX_NUMBER_OFFSET	4

#define COLOR_BUTTON_DELAY	100

#ifdef ADC_BACKLIGHT
#define MAIN_FIRST_ICON	GICON_PALETTE
#define MAIN_FIRST_TEXT	GTEXT_PALETTE
#else
#define MAIN_FIRST_ICON GICON_BRIGHTNESS
#define MAIN_FIRST_TEXT GTEXT_BRIGHTNESS
#endif

enum
{
#ifndef ADC_BACKLIGHT
	MENU_BRIGHTNESS,
#endif
	MENU_PALETTE,
	MENU_COLORS,
	MENU_GRID,
	MENU_BATTERY,
	MENU_POS_H,
	MENU_POS_V,
	//
	MAIN_COUNT
};

//

static __xdata uint8_t menu_pos;
static __xdata uint8_t menu_max;

static __xdata uint8_t main_pos;
static __xdata uint8_t color_pos;

static __xdata uint8_t menu_icon_x;
static __xdata int8_t menu_icon_y0;
static __xdata int8_t menu_icon_y1;
static __xdata uint8_t menu_icon_s;
static __xdata uint8_t menu_icon_i;

static __xdata uint16_t menu_save_wait;

static __xdata uint8_t hexnum[6];

// menu item count
static __code const uint8_t menu_side_limit[MAIN_COUNT] =
{
#ifndef ADC_BACKLIGHT
	[MENU_BRIGHTNESS] = CFG_BACKLIGHT_LEVELS,
#endif
	[MENU_PALETTE] = CFG_PALETTE_COUNT,
	[MENU_COLORS] = 4,
	[MENU_GRID] = 2,
	[MENU_BATTERY] = 2,
	[MENU_POS_H] = CFG_MAX_POS_H,
	[MENU_POS_V] = CFG_MAX_POS_V,
};

// config option pointer
static __xdata uint8_t *__code const value_ptr[MAIN_COUNT] =
{
#ifndef ADC_BACKLIGHT
	[MENU_BRIGHTNESS] = &config.brightness,
#endif
	[MENU_PALETTE] = &config.palette,
	[MENU_GRID] = &config.grid,
	[MENU_BATTERY] = &config.battery,
	[MENU_POS_H] = &config.pos_h,
	[MENU_POS_V] = &config.pos_v,
};

//
static void tick_main_menu();
static void tick_osd_idle();

//
// stuff

static void fake_palette()
{
	uint16_t color = config.pal[config.palette].color[menu_pos];

	spicmd.palette.color[0] = color;
	spicmd.palette.color[1] = color;
	spicmd.palette.color[2] = color;
	spicmd.palette.color[3] = color;
	spicmd_palette_full();
}

static void split_palette()
{
	uint16_t color = config.pal[config.palette].color[menu_pos];
	uint16_t tmp;

	tmp = color >> 11;
	tmp *= 2100;
	tmp /= 255;
	hexnum[0] = (tmp & 0xF0) >> 4;
	hexnum[1] = tmp & 0x0F;

	tmp = (color >> 5) & 0x3F;
	tmp *= 1033;
	tmp /= 255;
	hexnum[2] = (tmp & 0xF0) >> 4;
	hexnum[3] = tmp & 0x0F;

	tmp = color & 0x1F;
	tmp *= 2105;
	tmp /= 255;
	hexnum[4] = (tmp & 0xF0) >> 4;
	hexnum[5] = tmp & 0x0F;
}

static void combine_palette()
{
	uint16_t color;
	uint16_t tmp;

	tmp = (hexnum[0] << 4) | hexnum[1];
	tmp *= 31;
	tmp /= 255;
	color = tmp << 11;

	tmp = (hexnum[2] << 4) | hexnum[3];
	tmp *= 63;
	tmp /= 255;
	color |= (tmp & 0x3F) << 5;

	tmp = (hexnum[4] << 4) | hexnum[5];
	tmp *= 31;
	tmp /= 255;
	color |= tmp & 0x1F;

	config.pal[config.palette].color[color_pos] = color;

	config_apply_palette(config.palette);
}

static void slowdown(uint8_t count)
{
	do
	{
		TF0 = 0;
		while(!TF0);
	} while(--count);
}

//
// values

static void change_value()
{
	switch(main_pos)
	{
#ifndef ADC_BACKLIGHT
		case MENU_BRIGHTNESS:
			spicmd_backlight(gamma_table[menu_pos]);
		break;
#endif
		case MENU_PALETTE:
			config_apply_palette(menu_pos);
		break;
		case MENU_COLORS:
			fake_palette();
			color_pos = menu_pos;
			// do not write
			return;
		case MENU_GRID:
			spicmd_show_grid(menu_pos);
		break;
		case MENU_POS_H:
			spicmd_h_position(menu_pos);
		break;
		case MENU_POS_V:
			spicmd_v_position(menu_pos);
		break;
	}

	*value_ptr[main_pos] = menu_pos;
}

static void setup_value()
{
	menu_icon_i = 0;

	menu_max = menu_side_limit[menu_pos];

	if(main_pos == MENU_COLORS)
	{
		menu_pos = color_pos;
		fake_palette();
		return;
	}
#ifndef ADC_BACKLIGHT
	if(main_pos == MENU_BRIGHTNESS)
		menu_icon_i = 1;
#endif
	menu_pos = *value_ptr[main_pos];
}

//
// partial stuff

static void put_number(int16_t x, int8_t y, uint8_t num)
{
	uint8_t ten;

	num += menu_icon_i;

	ten = num / 10;
	num %= 10;

	if(!ten)
	{
		gfx_show_icon(x, y, GNUM_0 + num);
		// just to keep consistent draw speed
		gfx_show_icon(x, y, GNUM_0 + num);
		return;
	}

	x -= OSD_DIGIT_GAP / 2;
	gfx_show_icon(x, y, GNUM_0 + ten);
	x += OSD_DIGIT_GAP;
	gfx_show_icon(x, y, GNUM_0 + num);
}

//
// draw

static void draw_icons()
{
	uint8_t tmp = menu_pos;
	int16_t x;

	x = menu_icon_x;

	// center
	gfx_show_icon(x, menu_icon_y1, menu_icon_i + tmp);

	// right
	tmp++;
	if(tmp >= menu_max)
		tmp = 0;
	gfx_show_icon(x + menu_icon_s, menu_icon_y0, menu_icon_i + tmp);

	// left
	tmp = menu_pos - 1;
	if(tmp == 0xFF)
		tmp = menu_max - 1;
	gfx_show_icon(x - menu_icon_s, menu_icon_y0, menu_icon_i + tmp);
}

static void draw_numbers()
{
	uint8_t tmp = menu_pos;
	int16_t x;

	x = menu_icon_x;

	// center
	put_number(x, menu_icon_y1, tmp);

	// right
	tmp++;
	if(tmp >= menu_max)
		tmp = 0;
	put_number(x + menu_icon_s, menu_icon_y0, tmp);

	// left
	tmp = menu_pos - 1;
	if(tmp == 0xFF)
		tmp = menu_max - 1;
	put_number(x - menu_icon_s, menu_icon_y0, tmp);
}

static void draw_palhex()
{
	uint8_t i = 0;
	uint8_t x = OSD_CENTER - 2 * OSD_DIGIT_GAP - OSD_DIGIT_GAP / 2;

	do
	{
		uint8_t y = menu_icon_y0;
		if(i == menu_pos)
			y = menu_icon_y1;
		gfx_show_icon(x, y, GNUM_0 + hexnum[i]);
		x += OSD_DIGIT_GAP;
		i++;
	} while(i < 6);
}

static void draw_main_text()
{
	// main text Y position is bound to scroll movement
	uint8_t y;

	if(menu_icon_x < OSD_CENTER)
		y = OSD_CENTER - menu_icon_x;
	else
		y = menu_icon_x - OSD_CENTER;

	y += GAP_MAIN_ICON_TEXT;
	y += menu_icon_y1;

	gfx_show_icon(OSD_CENTER, y, MAIN_FIRST_TEXT + menu_pos);
}

//
// ticker: number menu

static void tick_color_menu()
{
	if(!IN_LEFT)
	{
		// move
		uint8_t i = HEX_NUMBER_OFFSET + 1;

		do
		{
			i--;
			menu_icon_y1 = OSD_SIDE_ICON_Y - i;
			draw_palhex();
		} while(i);

		menu_pos++;
		if(menu_pos >= 6)
			menu_pos = 0;

		do
		{
			i++;
			menu_icon_y1 = OSD_SIDE_ICON_Y - i;
			draw_palhex();
		} while(i < HEX_NUMBER_OFFSET);

		slowdown(COLOR_BUTTON_DELAY);

		return;
	}

	if(!IN_RIGHT)
	{
		// move
		uint8_t i = HEX_NUMBER_OFFSET + 1;

		do
		{
			i--;
			menu_icon_y1 = OSD_SIDE_ICON_Y - i;
			draw_palhex();
		} while(i);

		menu_pos--;
		if(menu_pos == 0xFF)
			menu_pos = 5;

		do
		{
			i++;
			menu_icon_y1 = OSD_SIDE_ICON_Y - i;
			draw_palhex();
		} while(i < HEX_NUMBER_OFFSET);

		slowdown(COLOR_BUTTON_DELAY);

		return;
	}

	if(!IN_ENTER)
	{
		// leave submenu
		uint8_t i = 0;

		do
		{
			i++;
			menu_icon_y0 = OSD_SIDE_ICON_Y + i;
			menu_icon_y1 = OSD_SIDE_ICON_Y - HEX_NUMBER_OFFSET + i;
			draw_palhex();
		} while(i < LEN_HEX_SCROLL);

		menu_pos = main_pos;
		menu_max = MAIN_COUNT;
		menu_icon_i = MAIN_FIRST_ICON;

		menu_icon_y0 = OSD_MAIN_ICON_Y;
		i = LEN_SELECT_SCROLL;
		do
		{
			i--;
			menu_icon_s = OSD_ICON_GAP + i;
			menu_icon_y1 = OSD_MAIN_ICON_Y - i;
			draw_icons();
			draw_main_text();
		} while(i);

		ticker = tick_main_menu;
		return;
	}

	if(!IN_TOUCH_L)
	{
		// increment
		uint8_t x, y, n;

		x = hexnum[menu_pos];
		x++;
		x &= 0x0F;
		hexnum[menu_pos] = x;

		x = OSD_CENTER - 2 * OSD_DIGIT_GAP - OSD_DIGIT_GAP / 2;
		x += menu_pos * OSD_DIGIT_GAP;

		n = GNUM_0 + hexnum[menu_pos];

		y = OSD_HEIGHT;
		do
		{
			y--;
			gfx_show_icon(x, y, n);
		} while(y > OSD_SIDE_ICON_Y - HEX_NUMBER_OFFSET);

		combine_palette();
		slowdown(COLOR_BUTTON_DELAY);

		return;
	}

	if(!IN_TOUCH_R)
	{
		// decrement
		uint8_t x, y, n;

		x = hexnum[menu_pos];
		x--;
		x &= 0x0F;
		hexnum[menu_pos] = x;

		x = OSD_CENTER - 2 * OSD_DIGIT_GAP - OSD_DIGIT_GAP / 2;
		x += menu_pos * OSD_DIGIT_GAP;

		n = GNUM_0 + hexnum[menu_pos];

		y = OSD_HEIGHT;
		do
		{
			y--;
			gfx_show_icon(x, y, n);
		} while(y > OSD_SIDE_ICON_Y - HEX_NUMBER_OFFSET);

		combine_palette();
		slowdown(COLOR_BUTTON_DELAY);

		return;
	}
}

//
// ticker: number menu

static void tick_number_menu()
{
	if(!IN_LEFT)
	{
		// scroll left
		uint8_t i = 0;

		do
		{
			i++;
			menu_icon_x = OSD_CENTER - i;
			draw_numbers();
		} while(i < OSD_ICON_GAP / 2);

		menu_pos++;
		if(menu_pos >= menu_max)
			menu_pos = 0;

		change_value();

		do
		{
			i--;
			menu_icon_x = OSD_CENTER + i;
			draw_numbers();
		} while(i);

		return;
	}

	if(!IN_RIGHT)
	{
		// scroll right
		uint8_t i = 0;

		do
		{
			i++;
			menu_icon_x = OSD_CENTER + i;
			draw_numbers();
		} while(i < OSD_ICON_GAP / 2);

		menu_pos--;
		if(menu_pos == 0xFF)
			menu_pos = menu_max - 1;

		change_value();

		do
		{
			i--;
			menu_icon_x = OSD_CENTER - i;
			draw_numbers();
		} while(i);

		return;
	}

	if(!IN_ENTER)
	{
		if(main_pos == MENU_COLORS)
		{
			// enter color menu
			uint8_t i;

			i = 0;
			do
			{
				i++;
				menu_icon_s = OSD_ICON_GAP + i;
				menu_icon_y1 = OSD_SIDE_ICON_Y + i;
				draw_numbers();
			} while(i < LEN_HEX_SCROLL);

			config_apply_palette(config.palette);
			split_palette();

			menu_pos = 0;

			do
			{
				i--;
				menu_icon_y0 = OSD_SIDE_ICON_Y + i;
				menu_icon_y1 = OSD_SIDE_ICON_Y - HEX_NUMBER_OFFSET + i;
				draw_palhex();
			} while(i);

			ticker = tick_color_menu;
			return;
		} else
		{
			// leave submenu
			uint8_t i;

			i = OSD_SIDE_ICON_Y;
			do
			{
				i++;
				menu_icon_y0 = i;
				menu_icon_y1 = i;
				draw_numbers();
			} while(i < SIDE_Y_START);

			menu_pos = main_pos;
			menu_max = MAIN_COUNT;
			menu_icon_i = MAIN_FIRST_ICON;

			menu_icon_y0 = OSD_MAIN_ICON_Y;
			i = LEN_SELECT_SCROLL;
			do
			{
				i--;
				menu_icon_s = OSD_ICON_GAP + i;
				menu_icon_y1 = OSD_MAIN_ICON_Y - i;
				draw_icons();
				draw_main_text();
			} while(i);

			ticker = tick_main_menu;
			return;
		}
	}
}

//
// ticker: main menu

static void tick_main_menu()
{
	if(!IN_LEFT)
	{
		// scroll left
		uint8_t i = 0;

		do
		{
			i++;
			menu_icon_x = OSD_CENTER - i;
			draw_icons();
			draw_main_text();
		} while(i < OSD_ICON_GAP / 2);

		menu_pos++;
		if(menu_pos >= menu_max)
			menu_pos = 0;

		do
		{
			i--;
			menu_icon_x = OSD_CENTER + i;
			draw_icons();
			draw_main_text();
		} while(i);

		return;
	}

	if(!IN_RIGHT)
	{
		// scroll right
		uint8_t i = 0;

		do
		{
			i++;
			menu_icon_x = OSD_CENTER + i;
			draw_icons();
			draw_main_text();
		} while(i < OSD_ICON_GAP / 2);

		menu_pos--;
		if(menu_pos == 0xFF)
			menu_pos = menu_max - 1;

		do
		{
			i--;
			menu_icon_x = OSD_CENTER - i;
			draw_icons();
			draw_main_text();
		} while(i);

		return;
	}

	if(!IN_ENTER)
	{
		// enter submenu
		uint8_t i = 0;

		do
		{
			i++;
			menu_icon_s = OSD_ICON_GAP + i;
			menu_icon_y1 = OSD_MAIN_ICON_Y - i;
			draw_icons();
			draw_main_text();
		} while(i < LEN_SELECT_SCROLL);

		main_pos = menu_pos;
		setup_value();

		menu_icon_s = OSD_ICON_GAP;
		i = SIDE_Y_START + 1;
		do
		{
			i--;
			menu_icon_y0 = i;
			menu_icon_y1 = i;
			draw_numbers();
		} while(i > OSD_SIDE_ICON_Y);

		ticker = tick_number_menu;
		return;
	}

	if(!IN_TOUCH_L && !IN_TOUCH_R)
	{
		// leave menu
		uint8_t i = 0;

		do
		{
			i++;
			menu_icon_s = OSD_ICON_GAP + i;
			menu_icon_y1 = OSD_MAIN_ICON_Y + i;
			draw_icons();
			draw_main_text();
		} while(i < MAIN_SELECT_SCROLL);

		config_save();
		slowdown(100);

		OUT_OSD = 1;

		slowdown(500);

		ticker = tick_osd_idle;
		return;
	}
}

//
// ticker: idle

static void tick_osd_idle()
{
	static __xdata uint8_t old = 0xFF;
	uint8_t in;

	if(menu_save_wait)
	{
		menu_save_wait--;
		if(!menu_save_wait)
			config_save();
	}

	in = IN_TOUCH_L;
	in |= IN_TOUCH_R << 1;

	if(old != 0xFF)
	{
		uint8_t check = in ^ old;

		if(!check)
			return;

		check &= in;
		if(check)
		{
			switch(old)
			{
				case 0:
				{
					// enter main menu
					uint8_t i = MAIN_SELECT_SCROLL;

					menu_max = MAIN_COUNT;
					menu_icon_i = MAIN_FIRST_ICON;
					menu_icon_x = OSD_CENTER;
					menu_icon_y0 = OSD_MAIN_ICON_Y;
					menu_icon_y1 = OSD_MAIN_ICON_Y;
					menu_icon_s = OSD_ICON_GAP;

					menu_save_wait = 0;
					old = 0xFF;

					OUT_OSD = 0;

					do
					{
						i--;
						menu_icon_s = OSD_ICON_GAP + i;
						menu_icon_y1 = OSD_MAIN_ICON_Y + i;
						draw_icons();
						draw_main_text();
					} while(i);

					ticker = tick_main_menu;
					return;
				}
				break;
				case 1:
					menu_save_wait = QUICK_SAVE_DELAY;
					config.palette++;
					if(config.palette >= CFG_PALETTE_COUNT)
						config.palette = 0;
					config_apply_palette(config.palette);
				break;
				case 2:
					menu_save_wait = QUICK_SAVE_DELAY;
#ifdef ADC_BACKLIGHT
					config.palette--;
					if(config.palette == 0xFF)
						config.palette = CFG_PALETTE_COUNT - 1;
					config_apply_palette(config.palette);
#else
					config.brightness++;
					if(config.brightness >= CFG_BACKLIGHT_LEVELS)
						config.brightness = 0;
					spicmd_backlight(gamma_table[config.brightness]);
#endif
				break;
			}
		}
	}

	old = in;
}

//
// API

void osd_init()
{
	// hide OSD
	OUT_OSD = 1;

	// for grayscale
	spicmd.palette.color[0] = RGB16(160, 160, 160);
	spicmd_palette_osd();

	// background input
	ticker = tick_osd_idle;

	// menu background
	spicmd_osd_clear(OSD_COLOR_BACKGROUND);
#ifdef OSD_COLOR_TOP
	spicmd_osd_line(0, OSD_COLOR_TOP);
#endif
#ifdef OSD_COLOR_BOT
	spicmd_osd_line(OSD_HEIGHT - 1, OSD_COLOR_BOT);
#endif
}

