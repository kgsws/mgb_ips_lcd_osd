#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "reg.h"
#include "defs.h"
#include "spicmd.h"
#include "config.h"

// DO NOT FORGET:
// program_eeprom_split=16384

#define TPS_VALUE	24

__xdata config_data_t config;

// PWM gamma table
#ifdef ADC_BACKLIGHT
__code const uint8_t gamma_table[16] =
{
	0, 1, 2, 3, 4, 5, 6, 8, 10, 12, 15, 18, 21, 24, 28, 32
};
#else
__code const uint8_t gamma_table[11] =
{
	0, 1, 2, 3, 4, 6, 9, 13, 18, 25, 32
};
#endif

// stored config
static __code __at(0x4000) const config_data_t config_flash;

// default config
static __code const config_data_t config_default =
{
	.magic = CONFIG_MAGIC,
	.brightness = 0,
	.palette = 0,
	.grid = 1,
	.battery = 1,
	.pos_h = 40,
	.pos_v = 30,
	.pal =
	{
		{{ // white to black
			RGB16(255, 255, 255),
			RGB16(170, 170, 170),
			RGB16(85, 85, 85),
			RGB16(0, 0, 0),
		}},
		{{ // MGB
			RGBHTML(0xF1FAE7),
			RGBHTML(0x93A384),
			RGBHTML(0x55624C),
			RGBHTML(0x151E0F),
		}},
		{{ // MGB (alt)
			RGBHTML(0xC0FF98),
			RGBHTML(0x30B0A8),
			RGBHTML(0x187688),
			RGBHTML(0x003428),
		}},
		{{ // DMG
			RGBHTML(0xB5FF52),
			RGBHTML(0x52B24D),
			RGBHTML(0x007018),
			RGBHTML(0x002C00),
		}},
		{{ // DMG bivert (original - blue)
			RGBHTML(0xCCE7F1),
			RGBHTML(0x7E959E),
			RGBHTML(0x5F699A),
			RGBHTML(0x001090),
		}},
		{{ // DMG bivert (green)
			RGBHTML(0xCCF1E7),
			RGBHTML(0x7E9E95),
			RGBHTML(0x30703D),
			RGBHTML(0x004800),
		}},
		{{ // DMG bivert (red)
			RGBHTML(0xF1E7CC),
			RGBHTML(0x9E957E),
			RGBHTML(0x805248),
			RGBHTML(0x600000),
		}},
		{{ // emulator
			RGB16(252, 252, 124),
			RGB16(188, 188, 92),
			RGB16(124, 124, 60),
			RGB16(60, 60, 28),
		}},
		{{ // red to black
			RGB16(255, 0, 0),
			RGB16(170, 0, 0),
			RGB16(85, 0, 0),
			RGB16(0, 0, 0),
		}},
		{{ // green to black
			RGB16(0, 255, 0),
			RGB16(0, 170, 0),
			RGB16(0, 85, 0),
			RGB16(0, 0, 0),
		}},
		{{ // blue to black
			RGB16(0, 0, 255),
			RGB16(0, 0, 170),
			RGB16(0, 0, 85),
			RGB16(0, 0, 0),
		}},
		{{ // cyan to black
			RGB16(0, 255, 255),
			RGB16(0, 170, 170),
			RGB16(0, 85, 85),
			RGB16(0, 0, 0),
		}},
		{{ // magenta to black
			RGB16(255, 0, 255),
			RGB16(170, 0, 170),
			RGB16(85, 0, 85),
			RGB16(0, 0, 0),
		}},
		{{ // yellow to black
			RGB16(255, 255, 0),
			RGB16(170, 170, 0),
			RGB16(85, 85, 0),
			RGB16(0, 0, 0),
		}},
		{{ // orange to black
			RGB16(255, 128, 0),
			RGB16(170, 85, 0),
			RGB16(85, 42, 0),
			RGB16(0, 0, 0),
		}},
		{{ // violet to black
			RGB16(128, 0, 255),
			RGB16(85, 0, 170),
			RGB16(42, 0, 85),
			RGB16(0, 0, 0),
		}},
	}
};

//
// stuff

static void copy_config(__code const uint8_t *src)
{
	__xdata uint8_t *dst = (__xdata uint8_t*)&config;
	uint8_t count = sizeof(config_data_t);
	do
	{
		*dst++ = *src++;
	} while(--count);
}

static void erase_config()
{
	IAP_CONTR = 0x80;
	IAP_TPS = TPS_VALUE;
	IAP_CMD = 3;
	IAP_ADDRL = 0;
	IAP_ADDRH = 0;
	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;
	__asm
		nop
	__endasm;
	IAP_CONTR = 0x00;
	IAP_ADDRH = 0x80;
	IAP_CMD = 0;
}

static void write_config()
{
	uint16_t dst = 0;
	__xdata const uint8_t *src = (__xdata const uint8_t*)&config;
	uint8_t count = sizeof(config_data_t);

	IAP_CONTR = 0x80;
	IAP_TPS = TPS_VALUE;
	IAP_CMD = 2;

	do
	{
		IAP_ADDRH = dst >> 8;
		IAP_ADDRL = dst & 0xFF;
		dst++;
		IAP_DATA = *src++;
		IAP_TRIG = 0x5A;
		IAP_TRIG = 0xA5;
		__asm
			nop
		__endasm;
	} while(--count);

	IAP_CONTR = 0x00;
	IAP_ADDRH = 0x80;
	IAP_CMD = 0;
}

//
// API

void config_init()
{
	// check built-in config
	if(config_flash.magic == CONFIG_MAGIC)
		copy_config((__code const uint8_t*)&config_flash);
	else
		copy_config((__code const uint8_t*)&config_default);

#ifdef ADC_BACKLIGHT
	spicmd_backlight(gamma_table[0]);
#else
	spicmd_backlight(gamma_table[config.brightness]);
#endif
	spicmd_show_grid(config.grid);
	spicmd_h_position(config.pos_h);
	spicmd_v_position(config.pos_v);
}

void config_save()
{
	__code const uint8_t *src = (__code const uint8_t*)&config_flash;
	__xdata uint8_t *dst = (__xdata uint8_t*)&config;
	uint8_t count = sizeof(config_data_t);

	do
	{
		if(*dst != *src)
		{
			erase_config();
			write_config();
			break;
		}
		dst++;
		src++;
	} while(--count);

}

void config_apply_palette(uint8_t idx)
{
	__xdata uint8_t *dst = (__xdata uint8_t*)spicmd.palette.color;
	__xdata uint8_t *src = (__xdata uint8_t*)config.pal[idx];
	uint8_t count = 4 * sizeof(uint16_t);

	do
	{
		*dst++ = *src++;
	} while(--count);

	spicmd_palette_full();
}

