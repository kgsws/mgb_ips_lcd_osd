
#define GFX_ICON_WIDTH	35	// last column is (kinda) ignored
#define GFX_ICON_HEIGHT	32
#define GFX_ICON_STRIDE	18	// 36 pixels

#define GFX_TEXT_HEIGHT	10

//
enum
{
	// main icons
	GICON_BRIGHTNESS,
	GICON_PALETTE,
	GICON_COLORS,
	GICON_GRID,
	GICON_BATTERY,
	GICON_HPOS,
	GICON_VPOS,
	// main strings
	GTEXT_BRIGHTNESS,
	GTEXT_PALETTE,
	GTEXT_COLORS,
	GTEXT_GRID,
	GTEXT_BATTERY,
	GTEXT_HPOS,
	GTEXT_VPOS,
	// numbers
	GNUM_0,
	GNUM_1,
	GNUM_2,
	GNUM_3,
	GNUM_4,
	GNUM_5,
	GNUM_6,
	GNUM_7,
	GNUM_8,
	GNUM_9,
	GNUM_A,
	GNUM_B,
	GNUM_C,
	GNUM_D,
	GNUM_E,
	GNUM_F,
	//
	NUM_GFX_ICONS
};

//

extern const __code uint8_t logo_pal_idx[2];
extern const __code uint8_t osd_pal_idx[4];

//

uint8_t gfx_read_2bpp();

void gfx_show_logo(uint8_t y);
void gfx_show_icon(int16_t x, int8_t y, uint8_t idx);

