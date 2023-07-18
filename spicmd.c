#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "reg.h"
#include "spicmd.h"
#include "gfx.h"

__xdata spi_cmd_t spicmd;

//
// waiting

static void spi_wait(uint8_t ttt)
{
	while(--ttt);
}

//
// data transfer

void spi_send(uint8_t len)
{
	__xdata uint8_t *ptr = (__xdata uint8_t*)&spicmd;

	OUT_CS = 0;

	do
	{
		SPDAT = *ptr;
		ptr++;
		spi_wait(2);
	} while(--len);

	spi_wait(3);
	OUT_CS = 1;
}

//
// API

void spicmd_osd_clear(uint8_t color)
{
	color |= color << 4;

	for(uint8_t y = 0; y < 65; y++)
	{
		uint8_t x = 64;

		OUT_CS = 0;

		SPDAT = 0x0D;
		spi_wait(4);
		SPDAT = y << 6;
		spi_wait(4);
		SPDAT = y >> 2;
		spi_wait(4);

		do
		{
			SPDAT = color;
			spi_wait(4);
		} while(--x);

		spi_wait(3);
		OUT_CS = 1;
		spi_wait(3);
	}
}

void spicmd_osd_line(uint8_t y, uint8_t color)
{
	uint16_t addr;

	if(y >= OSD_HEIGHT)
		return;

	color |= color << 4;

	addr = (uint16_t)y * OSD_STRIDE;

	OUT_CS = 0;

	SPDAT = 0x0D;
	spi_wait(4);
	SPDAT = addr & 0xFF;
	spi_wait(4);
	SPDAT = addr >> 8;
	spi_wait(5);

	y = OSD_STRIDE;
	do
	{
		SPDAT = color;
		spi_wait(4);
	} while(--y);

	spi_wait(3);
	OUT_CS = 1;
}

void spicmd_xbm_line(const __code uint8_t *data)
{
	__xdata uint8_t *ptr = (__xdata uint8_t*)&spicmd;
	uint8_t len = 3;

	OUT_CS = 0;

	// command
	do
	{
		SPDAT = *ptr;
		ptr++;
		spi_wait(2);
	} while(--len);

	// data
	len = OSD_WIDTH / 8;
	do
	{
		register uint8_t bits = *data++;
		register uint8_t out;

		out = logo_pal_idx[bits & 1];
		out |= logo_pal_idx[(bits >> 1) & 1] << 4;
		SPDAT = out;
		spi_wait(1);

		out = logo_pal_idx[(bits >> 2) & 1];
		out |= logo_pal_idx[(bits >> 3) & 1] << 4;
		SPDAT = out;
		spi_wait(1);

		out = logo_pal_idx[(bits >> 4) & 1];
		out |= logo_pal_idx[(bits >> 5) & 1] << 4;
		SPDAT = out;
		spi_wait(1);

		out = logo_pal_idx[(bits >> 6) & 1];
		out |= logo_pal_idx[(bits >> 7) & 1] << 4;
		SPDAT = out;
		spi_wait(1);
	} while(--len);

	spi_wait(4);
	OUT_CS = 1;
}

void spicmd_img_line()
{
	__xdata uint8_t *ptr = (__xdata uint8_t*)&spicmd;
	uint8_t len = 3;

	OUT_CS = 0;

	// command
	do
	{
		SPDAT = *ptr;
		ptr++;
		spi_wait(2);
	} while(--len);

	// data
	while(1)
	{
		uint8_t out;
		uint8_t idx;

		idx = gfx_read_2bpp();
		if(idx & 0x80)
			break;

		out = osd_pal_idx[idx];

		idx = gfx_read_2bpp();
		if(idx & 0x80)
			out |= out << 4;
		else
			out |= osd_pal_idx[idx] << 4;

		SPDAT = out;
		// no delay here
	}

	spi_wait(3);
	OUT_CS = 1;
}

