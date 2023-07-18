TARGET   = flash.ihx
CFLAGS   = --model-small -I../include
LFLAGS   = --code-loc 0x0000 --code-size 0x4000 --xram-loc 0x0000 --xram-size 0x400
ASFLAGS  = -plosgff
RELFILES = main.rel spicmd.rel gfx.rel osd.rel config.rel

$(TARGET): gfx_osd.h $(RELFILES)
	sdcc $(CFLAGS) $(LFLAGS) $(RELFILES) -o $(TARGET)

%.rel: %.c
	sdcc $(CFLAGS) -c $<

%.rel: %.asm
	sdas8051 $(ASFLAGS) $@ $<

gfx_osd.h:
	./imgconv.py images gfx_osd.h

clean:
	rm --force *.ihx *.lnk *.lst *.map *.rel *.rst *.sym *.mem *.asm *.lk flash.cdb flash.omf gfx_osd.h

all: clean main

