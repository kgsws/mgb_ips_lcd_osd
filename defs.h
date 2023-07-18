
#define RGB16(r,g,b)	(((uint16_t)(r & 0xF8) << 8) | ((uint16_t)(g & 0xFC) << 3) | (uint16_t)(b >> 3))
#define RGBHTML(color)	((uint16_t)((color & 0xF80000) >> 8) | (uint16_t)((color & 0xFC00) >> 5) | (uint16_t)((color & 0xF8) >> 3))

#define ADC_MEAS_DELAY	10	// this alternates between 'battery' and 'backlight'

#define OUT_CS	P12
#define OUT_OSD	P16

#define OSD_WIDTH	160
#define OSD_HEIGHT	52

#define OSD_STRIDE	(OSD_WIDTH / 2)

#define OSD_CENTER	(OSD_WIDTH / 2)

#define OSD_ICON_GAP	70
#define OSD_DIGIT_GAP	18

#define OSD_MAIN_ICON_Y	2
#define OSD_MAIN_TEXT_Y	(OSD_HEIGHT - 13)

#define OSD_SIDE_ICON_Y	(OSD_HEIGHT - 36)
#define OSD_SIDE_TEXT_Y	1

//

extern __xdata uint16_t tick_ms;
extern __xdata uint8_t tick_tmp;
extern __xdata void (*ticker)();

//
// input

#define IN_LEFT	P36
#define IN_RIGHT	P33
#define IN_ENTER	P32
#define IN_TOUCH_L	P17
#define IN_TOUCH_R	P37

//
// customization

// hide cartridge logo
//#define CFG_HIDE_CART_LOGO

// show custom logo
#define CFG_SHOW_CUSTOM_LOGO

// custom logo start location
// when > OSD_HEIGHT it also acts as delay
#define CFG_LOGO_START_Y	90

// custom logo scroll speed
// only use (2^x)-1
#define CFG_LOGO_TICK_MASK	0x1F

// custom logo active color (from OSD palette)
#define CFG_LOGO_COLOR	4

// OSD colors
#define OSD_COLOR_BACKGROUND	4
#define OSD_COLOR_TOP	1
#define OSD_COLOR_BOT	1

// low battery raw ADC value
#define ADC_LOW_BAT	0x1A00	// 0x1A00 = ~2.3V

// use ADC for backlight level
//#define ADC_BACKLIGHT	12
#define ADC_BL_RAW_LO	0x6500
#define ADC_BL_RAW_HI	0xDB00

