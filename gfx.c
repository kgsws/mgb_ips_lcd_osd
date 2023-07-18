#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "reg.h"
#include "defs.h"
#include "spicmd.h"
#include "gfx.h"

// images + logo
#include "gfx_osd.h"

#ifdef OSD_COLOR_TOP
#define CLIP_TOP	1
#else
#define CLIP_TOP	0
#endif

#ifdef OSD_COLOR_BOT
#define CLIP_BOT	(OSD_HEIGHT - 1)
#else
#define CLIP_BOT	OSD_HEIGHT
#endif

//

const __code uint8_t logo_pal_idx[2] = {15, CFG_LOGO_COLOR};
const __code uint8_t osd_pal_idx[4] = {4, 14, 15, 3};

// these are not in XRAM
static const __code uint8_t *img_ptr;
static uint8_t img_left;
static uint8_t img_byte;
static uint8_t img_bits;

// icon list
static __code const uint8_t *__code const gfx_icon_ptr[NUM_GFX_ICONS] =
{
	[GICON_BRIGHTNESS] = i_brightness,
	[GICON_PALETTE] = i_palette,
	[GICON_COLORS] = i_colors,
	[GICON_GRID] = i_grid,
	[GICON_BATTERY] = i_battery,
	[GICON_HPOS] = i_hpos,
	[GICON_VPOS] = i_vpos,

	[GTEXT_BRIGHTNESS] = t_brightness,
	[GTEXT_PALETTE] = t_palette,
	[GTEXT_COLORS] = t_colors,
	[GTEXT_GRID] = t_grid,
	[GTEXT_BATTERY] = t_battery,
	[GTEXT_HPOS] = t_hpos,
	[GTEXT_VPOS] = t_vpos,

	[GNUM_0] = i_num0,
	[GNUM_1] = i_num1,
	[GNUM_2] = i_num2,
	[GNUM_3] = i_num3,
	[GNUM_4] = i_num4,
	[GNUM_5] = i_num5,
	[GNUM_6] = i_num6,
	[GNUM_7] = i_num7,
	[GNUM_8] = i_num8,
	[GNUM_9] = i_num9,
	[GNUM_A] = i_numA,
	[GNUM_B] = i_numB,
	[GNUM_C] = i_numC,
	[GNUM_D] = i_numD,
	[GNUM_E] = i_numE,
	[GNUM_F] = i_numF,
};

//
// API

void gfx_show_logo(uint8_t y)
{
	const __code uint8_t *data = logo;

	spicmd.cmd = 0x0D;
	spicmd.osd.address = (uint16_t)y * OSD_STRIDE;

	for( ; y < OSD_HEIGHT; y++)
	{
		spicmd_xbm_line(data);
		spicmd.osd.address += OSD_STRIDE;
		data += OSD_WIDTH / 8;
	}
}

void gfx_show_icon(int16_t x, int8_t y, uint8_t idx)
{
	const __code uint8_t *data;
	uint8_t width, stride, skip;
	int8_t ye;
	int16_t xe;

	if(y >= CLIP_BOT)
		return;

	data = gfx_icon_ptr[idx];

	stride = *data++;

	x -= stride / 2; // autocenter

	if(x >= OSD_WIDTH - 1) // last column can't be used
		return;

	ye = *data++;
	ye += y;

	if(y < CLIP_TOP)
	{
		data += (CLIP_TOP - y) * stride / 4;
		y = CLIP_TOP;
	}

	if(ye > CLIP_BOT)
		ye = CLIP_BOT;

	xe = x + stride;

	if(xe <= 0)
		return;

	if(xe > OSD_WIDTH)
		xe = OSD_WIDTH;

	width = xe - x;

	spicmd.cmd = 0x0D;
	spicmd.osd.address = (uint16_t)y * OSD_STRIDE;

	if(x < 0)
	{
		skip = -x & 3;
		x /= 4;
		data -= x;
		width += x * 4;
	} else
	{
		skip = x & 1;
		spicmd.osd.address += (x + 1) / 2;
	}

	stride /= 4;

	for( ; y < ye; y++)
	{
		img_ptr = data;
		img_left = width;
		img_bits = 0;

		data += stride;

		for(uint8_t tmp = 0; tmp < skip; tmp++)
			gfx_read_2bpp();

		spicmd_img_line();

		spicmd.osd.address += OSD_STRIDE;
	}
}

uint8_t gfx_read_2bpp()
{
	uint8_t ret;

	if(!img_left)
		return 0xFF;

	if(!img_bits)
	{
		img_byte = *img_ptr;
		img_ptr++;
		img_bits = 4;
	}

	ret = img_byte & 3;
	img_byte >>= 2;

	img_bits--;
	img_left--;

	return ret;
}

