
#define SFR(x, y)	__sfr __at(y) x
#define SBIT(x, y, z)	__sbit __at(y+z) x

// GPIO modes:
// 0) 8051 bidir
// 1) push pull
// 2) input
// 3) open drain

//
// PORT 0
SFR(P0M0, 0x94);
SFR(P0M1, 0x93);
SFR(P0, 0x80);
SBIT(P00, 0x80, 0);
SBIT(P01, 0x80, 1);
SBIT(P02, 0x80, 2);
SBIT(P03, 0x80, 3);
SBIT(P04, 0x80, 4);
SBIT(P05, 0x80, 5);
SBIT(P06, 0x80, 6);
SBIT(P07, 0x80, 7);

//
// PORT 1
SFR(P1M0, 0x92);
SFR(P1M1, 0x91);
SFR(P1, 0x90);
SBIT(P10, 0x90, 0);
SBIT(P11, 0x90, 1);
SBIT(P12, 0x90, 2);
SBIT(P13, 0x90, 3);
SBIT(P14, 0x90, 4);
SBIT(P15, 0x90, 5);
SBIT(P16, 0x90, 6);
SBIT(P17, 0x90, 7);

//
// PORT 2
SFR(P2M0, 0x96);
SFR(P2M1, 0x95);
SFR(P2, 0xA0);
SBIT(P20, 0xA0, 0);
SBIT(P21, 0xA0, 1);
SBIT(P22, 0xA0, 2);
SBIT(P23, 0xA0, 3);
SBIT(P24, 0xA0, 4);
SBIT(P25, 0xA0, 5);
SBIT(P26, 0xA0, 6);
SBIT(P27, 0xA0, 7);

//
// PORT 3
SFR(P3M0, 0xB2);
SFR(P3M1, 0xB1);
SFR(P3, 0xB0);
SBIT(P30, 0xB0, 0);
SBIT(P31, 0xB0, 1);
SBIT(P32, 0xB0, 2);
SBIT(P33, 0xB0, 3);
SBIT(P34, 0xB0, 4);
SBIT(P35, 0xB0, 5);
SBIT(P36, 0xB0, 6);
SBIT(P37, 0xB0, 7);

// SPI status
// 6) WCOL
// 7) SPIF

// SPI config
// 0:1) SPR; clock div: 4, 8, 16, 32
// 2) CPHA
// 3) CPOL
// 4) MSTR
// 5) DORD; 1 = LSB first
// 6) SPEN
// 7) SSIG; 1 = ignore SS

//
// SPI
SFR(SPSTAT, 0xCD);
SFR(SPCTL, 0xCE);
SFR(SPDAT, 0xCF);
#define IRQ_SPI	9

//
// IRQ
SBIT(EX0, 0xA8, 0);
SBIT(ET0, 0xA8, 1);
SBIT(EX1, 0xA8, 2);
SBIT(ET1, 0xA8, 3);
SBIT(ES, 0xA8, 4);
SBIT(EADC, 0xA8, 5);
SBIT(ELVD, 0xA8, 6);
SBIT(EA, 0xA8, 7);
SFR(IE, 0xA8);
SFR(IE2, 0xAF);

//
// power
SFR(PCON, 0x87);

//
// Timer 0 and 1
SFR(TCON, 0x88);
SFR(TMOD, 0x89);

//
// Timer 0
SFR(TL0, 0x8A);
SFR(TH0, 0x8C);
SBIT(TR0, 0x88, 4);
SBIT(TF0, 0x88, 5);
#define IRQ_TIMER0	1

//
// Timer 1
SFR(TL1, 0x8B);
SFR(TH1, 0x8D);
SBIT(TR1, 0x88, 6);
SBIT(TF1, 0x88, 7);
#define IRQ_TIMER1	3

//
// AUX
SFR(AUXR, 0x8E);

//
// ADC
SFR(ADC_CONTR, 0xBC);
SFR(ADC_RES, 0xBD);
SFR(ADC_RESL, 0xBE);
SFR(ADCCFG, 0xDE);
#define ADCTIM	*((__xdata uint8_t*)0xFEA8)
#define IRQ_ADC	5

//
// IAP
SFR(IAP_DATA, 0xC2);
SFR(IAP_ADDRH, 0xC3);
SFR(IAP_ADDRL, 0xC4);
SFR(IAP_CMD, 0xC5);
SFR(IAP_TRIG, 0xC6);
SFR(IAP_CONTR, 0xC7);
SFR(IAP_TPS, 0xF5);

