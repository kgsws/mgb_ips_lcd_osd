
#define CFG_BACKLIGHT_LEVELS	sizeof(gamma_table)
#define CFG_PALETTE_COUNT	16
#define CFG_MAX_POS_H	80
#define CFG_MAX_POS_V	72

#define CONFIG_MAGIC	0x2AB439C7	// increment with 'config_data_t' changes

typedef struct
{
	uint16_t color[4];
} cfg_palette_t;

typedef struct
{
	uint32_t magic;
	uint8_t brightness;
	uint8_t palette;
	uint8_t grid;
	uint8_t battery;
	uint8_t pos_h;
	uint8_t pos_v;
	cfg_palette_t pal[CFG_PALETTE_COUNT];
} config_data_t;

//

extern __xdata config_data_t config;

#ifdef ADC_BACKLIGHT
extern __code const uint8_t gamma_table[16];
#else
extern __code const uint8_t gamma_table[11];
#endif

//

void config_init();
void config_save();
void config_apply_palette(uint8_t idx);
