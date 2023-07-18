
typedef struct
{
	uint8_t cmd;
	union
	{
		struct
		{
			uint8_t value;
		} simple;
		struct
		{
			uint16_t color[4];
		} palette;
		struct
		{
			uint16_t address;
			// uint8_t data[80];
		} osd;
	};
} spi_cmd_t;

//

extern __xdata spi_cmd_t spicmd;

extern __xdata uint8_t spicmd_osd_line_offs;
extern __xdata uint8_t spicmd_osd_line_len;

//
void spi_send(uint8_t len);
void spicmd_osd_clear(uint8_t color);
void spicmd_osd_line(uint8_t y, uint8_t color);

void spicmd_xbm_line(const __code uint8_t *);
void spicmd_img_line();

// macros

#define spicmd_backlight(val)	do{spicmd.cmd = 0x01; spicmd.simple.value = val; spi_send(2);}while(0)
#define spicmd_temp_num(val)	do{spicmd.cmd = 0x02; spicmd.simple.value = val; spi_send(2);}while(0)
#define spicmd_palette_full()	do{spicmd.cmd = 0x03; spi_send(1+sizeof(uint16_t)*4);}while(0)
#define spicmd_palette_0()	do{spicmd.cmd = 0x04; spi_send(1+sizeof(uint16_t));}while(0)
#define spicmd_palette_1()	do{spicmd.cmd = 0x05; spi_send(1+sizeof(uint16_t));}while(0)
#define spicmd_palette_2()	do{spicmd.cmd = 0x06; spi_send(1+sizeof(uint16_t));}while(0)
#define spicmd_palette_3()	do{spicmd.cmd = 0x07; spi_send(1+sizeof(uint16_t));}while(0)
#define spicmd_palette_osd()	do{spicmd.cmd = 0x08; spi_send(1+sizeof(uint16_t));}while(0)
#define spicmd_v_position(val)	do{spicmd.cmd = 0x09; spicmd.simple.value = val; spi_send(2);}while(0)
#define spicmd_h_position(val)	do{spicmd.cmd = 0x0A; spicmd.simple.value = val; spi_send(2);}while(0)
#define spicmd_show_grid(val)	do{spicmd.cmd = 0x0B; spicmd.simple.value = val; spi_send(2);}while(0)
#define spicmd_show_battery(val)	do{spicmd.cmd = 0x0C; spicmd.simple.value = val; spi_send(2);}while(0)
// 0x0D: OSD data

