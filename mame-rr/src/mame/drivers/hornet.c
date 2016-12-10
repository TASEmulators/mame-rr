/*  Konami Hornet System
    Driver by Ville Linde


    Konami 'Hornet' Hardware
    Konami, 1997-1999

    Known games on this hardware include....

    Game                             (C)      Year
    ----------------------------------------------
    Gradius 4 : Fukkatsu             Konami   1999
    NBA Play by Play                 Konami   1998
    Silent Scope                     Konami   1999
    Silent Scope 2 : Fatal Judgement Konami   2000
    Silent Scope 2 : Dark Silhouette Konami   2000
    Terraburst                       Konami   1998

    Hardware overview:

    GN715 CPU Board:
    ----------------
        IBM PowerPC 403GA at 32MHz (main CPU)
        Motorola MC68EC000 at 16MHz (sound CPU)
        Konami K056800 (MIRAC), sound system interface
        Ricoh RF5c400 sound chip

    GN715 GFX Board:
    ----------------
        Analog Devices ADSP-21062 SHARC DSP at 36MHz
        Konami 0000037122 (2D Tilemap)
        Konami 0000033906 (PCI bridge)
        3DFX 500-0003-03 (Voodoo) FBI with 2MB RAM
        3DFX 500-0004-02 (Voodoo) TMU with 2MB RAM

    GQ871 GFX Board:
    ----------------
        Analog Devices ADSP-21062 SHARC DSP at 36MHz
        Konami 0000037122 (2D Tilemap)
        Konami 0000033906 (PCI bridge)
        3DFX 500-0009-01 (Voodoo 2) FBI with 2MB RAM
        3DFX 500-0010-01 (Voodoo 2) TMU with 4MB RAM


    Hardware configurations:
    ------------------------

    Game              KONAMI ID  CPU PCB    GFX Board(s)  LAN PCB
    --------------------------------------------------------------
    Gradius 4         GX837      GN715(A)   GN715(B)
    NBA Play By Play  GX778      GN715(A)   GN715(B)
    Silent Scope      GQ830      GN715(A)   2x GN715(B)
    Silent Scope 2    GQ931      GN715(A)   2x GQ871(B)   GQ931(H)


    PCB Layouts
    -----------

    Top Board
    GN715 PWB(A)A
    |--------------------------------------------------------------|
    | SP485CS CN10       CN11        CN9          JP8 JP9 JP10 JP11|
    |CN19                                                    PAL1  |
    |CN21       JP13 PAL2             68EC000          EPROM.7S    |
    |   NE5532       PAL3                                      CN12|
    |           JP12  JP16    DRM1M4SJ8                        CN13|
    |   NE5532                            MASKROM.9P    MASKROM.9T |
    |     SM5877 JP15         RF5C400                              |
    |CN18                                 MASKROM.12P   MASKROM.12T|
    |     SM5877     16.9344MHz                                    |
    |CN14            SRAM256K             MASKROM.14P   MASKROM.14T|
    |                                                              |
    |CN16            SRAM256K             MASKROM.16P   MASKROM.16T|
    |  ADC12138                                                    |
    |             056800            JP5                            |
    |                               JP4                            |
    |                      MACH111  JP3                |---------| |
    |   TEST_SW                         EPROM.22P      |         | |
    |CN1                   DRAM16X16                   |PPC403GA | |
    |                                   EPROM.25P      |         | |
    |                                                  |         | |
    |                      DRAM16X16    EPROM.27P      |---------| |
    | 4AK16                                                     JP6|
    |                                                              |
    |CN3                                                           |
    | 0038323  PAL4                                       7.3728MHz|
    | E9825    058232           CN2                                |
    |                                                     50.000MHz|
    |    RESET_SW               CN5                    JP1  JP2    |
    |M48T58Y-70PC1  CN4                 CN6               64.000MHz|
    |--------------------------------------------------------------|
    Notes:
          DRM1M4SJ8 - Fujitsu 81C4256 256Kx4 DRAM (SOJ24)
           SRAM256K - Cypress CY7C199 32kx8 SRAM (SOJ28)
          DRAM16X16 - Fujitsu 8118160A-60 16megx16 DRAM (SOJ42)
      0038323 E9825 - SOIC8 (Secured PIC?). I've seen a similar chip in the security cart of System573
      M48T58Y-70PC1 - ST Timekeeper RAM
            RF5C400 - Ricoh RF5C400 PCM 32Ch, 44.1 kHz Stereo, 3D Effect Spatializer, clock input 16.9344MHz
             056800 - Konami Custom (QFP80)
             058232 - Konami Custom Ceramic Package (SIL14)
           ADC12138 - National Semiconductor ADC12138 A/D Converter, 12-bit + Serial I/O With MUX (SOP28)
            MACH111 - AMD MACH111 CPLD (Stamped 'N676A1', PLCC44)
            68EC000 - Motorola MC68EC000, running at 16.0MHz (64/4)
           PPC403GA - IBM PowerPC 403GA CPU, clock input 32.0MHz (QFP160)
           SM5877AM - Nippon Precision Circuits 3rd Order 2-Channel D/A Converter (SOIC24)
              4AK16 - Hitachi 4AK16 Silicon N-Channel Power MOS FET Array (SIL10)
           NE5532AN - Philips, Dual Low-Noise High-Speed Audio OP Amp (DIP8)
            SP485CS - Sipex SP485CS Low Power Half Duplex RS485 Transceiver (DIP8)
               PAL1 - AMD PALCE16V8 (stamped 'N676A4', DIP20)
               PAL2 - AMD PALCE16V8 (stamped 'N676A2', DIP20)
               PAL3 - AMD PALCE16V8 (stamped 'N676A3', DIP20)
               PAL4 - AMD PALCE16V8 (stamped 'N676A5', DIP20)
                JP1 -       25M O O-O 32M
                JP2 -       25M O O-O 32M
                JP3 -        RW O O O RO
                JP4 - PROG  32M O O-O 16M
                JP5 - DATA  32M O-O O 16M
                JP6 - BOOT   16 O-O O 32
                JP7 - SRC DOUT2 O O-O 0
                JP8 -   64M&32M O-O O 16M
                JP9 -       64M O O-O 32M&16M
               JP10 -   64M&32M O-O O 16M
               JP11 -       64M O O-O 32M&16M
               JP12 -      THRU O-O O SP
               JP13 -      THRU O-O O SP
               JP14 - WDT       O O
               JP15 -      MONO O-O O SURR
               JP16 -      HIGH O O O MID (N/C LOW)
       CN1 THRU CN3 - Multi-pin Flat Cable Connector
                CN4 - Multi-pin Connector for Network PCB
                CN5 - Multi-pin Flat Cable Connector
                CN6 - 96-Pin To Lower PCB, Joining Connector
       CN7 THRU CN8 - Not used
      CN9 THRU CN11 - 6-Pin Power Connectors
               CN19 - USB Connector
               CN21 - 5-Pin Analog Controls Connector (Tied to USB Connector via the Filter Board)
               CN18 - RCA Mono Audio OUT
        CN14 & CN16 - RCA Stereo Audio OUT


    ROM Usage
    ---------
                 |------------------------------- ROM Locations ----------------------------------|
    Game         27P     25P  22P   16P     14P     12P     9P      16T     14T     12T  9T  7S
    -----------------------------------------------------------------------------------------------
    Gradius 4    837C01  -    -     837A09  837A10  -       778A12  837A04  837A05  -    -   837A08
    NBA P/Play   778A01  -    -     778A09  778A10  778A11  778A12  778A04  778A05  -    -   778A08
    S/Scope      830B01  -    -     830A09  830A10  -       -       -       -       -    -   830A08
    S/Scope 2    931D01  -    -     931A09  931A10  931A11  -       931A04  -       -    -   931A08
    Terraburst


    Bottom Board
    GN715 PWB(B)A
    |--------------------------------------------------------------|
    |CN4        CN2    CN8               CN6                    CN5|
    |JP1                 |---------|              4M_EDO   4M_EDO  |
    |                    |         |     |----------|              |
    |  4M_EDO 4M_EDO     | TEXELFX |     |          |              |
    |                    |         |     | PIXELFX  |       4M_EDO |
    |  4M_EDO 4M_EDO     |         |     |          |       4M_EDO |
    |                    |---------|     |          | |--------|   |
    |  4M_EDO 4M_EDO                     |----------| |KONAMI  |   |
    |CN3                                50MHz JP7     |33906   |   |
    |  4M_EDO 4M_EDO                          JP6     |        |   |
    |                       256KSRAM 256KSRAM         |--------|   |
    |CN7                                                           |
    |         AV9170                     1MSRAM 1MSRAM             |
    | MC44200                                                      |
    |                       256KSRAM 256KSRAM                      |
    |                                    1MSRAM 1MSRAM             |
    |  |-------|                                    MASKROM.24U    |
    |  |KONAMI |  MACH111  |-------------|              MASKROM.24V|
    |  |37122  |           |ANALOG       |   1MSRAM 1MSRAM         |
    |  |       |           |DEVICES      |                         |
    |  |-------|       JP5 |ADSP-21062   |   36.00MHz              |
    |1MSRAM                |SHARC        |   1MSRAM 1MSRAM         |
    |                      |             |                         |
    |1MSRAM                |             |                         |
    |           256KSRAM   |-------------|          MASKROM.32U    |
    |1MSRAM     256KSRAM                                MASKROM.32V|
    |           256KSRAM     PAL1  PAL2            JP4             |
    |1MSRAM                                                        |
    |           JP2                 CN1            JP3             |
    |--------------------------------------------------------------|
    Notes:
          4M_EDO - Silicon Magic SM81C256K16CJ-35 EDO DRAM 66MHz (SOJ40)
          1MSRAM - Cypress CY7C109-25VC 1Meg SRAM (SOJ32)
        256KSRAM - Winbond W24257AJ-15 256K SRAM (SOJ28)
         TEXELFX - 3DFX 500-0004-02 BD0665.1 TMU (QFP208)
         PIXELFX - 3DFX 500-0003-03 F001701.1 FBI (QFP240)
      0000037122 - Konami Custom (QFP208)
       MC44200FT - Motorola MC44200FT 3 Channel Video D/A Converter (QFP44)
         MACH111 - AMD MACH111 CPLD (Stamped 'N715B1', PLCC44)
          AV9170 - Integrated Circuit Systems Inc. Clock Multiplier (SOIC8)
            PAL1 - AMD PALCE16V8 (stamped 'N676B4', DIP20)
            PAL2 - AMD PALCE16V8 (stamped 'N676B5', DIP20)
             JP1 -    SCR O O-O TWN
             JP2 - MASTER O-O O SLAVE
             JP3 -    16M O O-O 32M
             JP4 -    32M O-O O 16M
             JP5 -  ASYNC O O-O SYNC
             JP6 -    DSP O O-O ADCK
             JP7 -    MCK O-O O SCK
             CN1 - 96 Pin To Lower PCB, Joining Connector
             CN2 - 8-Pin RGB OUT
             CN3 - 15-Pin DSUB VGA Video MAIN OUT
             CN4 - 6-Pin Power Connector
             CN5 - 4-Pin Power Connector
             CN6 - 2-Pin Connector (Not Used)
             CN7 - 15-Pin DSUB VGA Video MAIN OUT
             CN8 - 6-Pin Connector (Not Used)

    ROM Usage
    ---------
                 |------ ROM Locations -------|
    Game         24U     24V     32U     32V
    -------------------------------------------
    Gradius 4    837A13  837A15  837A14  837A16
    NBA P/Play   778A13  778A15  778A14  778A16
    S/Scope      -       -       -       -          (no ROMs, not used)
    S/Scope 2    -       -       -       -          (no ROMs, not used)
    Terraburst



    LAN PCB: GQ931 PWB(H)      (C) 1999 Konami
    ------------------------------------------

    2 x LAN ports, LANC(1) & LANC(2)
    1 x 32.0000MHz Oscillator

         HYC2485S  SMC ARCNET Media Transceiver, RS485 5Mbps-2.5Mbps
    8E   931A19    Konami 32meg masked ROM, ROM0 (compressed GFX data?)
    6E   931A20    Konami 32meg masked ROM, ROM1 (compressed GFX data?)
    12F  XC9536    Xilinx  CPLD,  44 pin PLCC, Konami no. Q931H1
    12C  XCS10XL   Xilinx  FPGA, 100 pin PQFP, Konami no. 4C
    12B  CY7C199   Cypress 32kx8 SRAM
    8B   AT93C46   Atmel 1K serial EEPROM, 8 pin SOP
    16G  DS2401    Dallas Silicon Serial Number IC, 6 pin SOP



    GFX PCB:  GQ871 PWB(B)A    (C) 1999 Konami
    ------------------------------------------

    There are no ROMs on the two GFX PCBs, all sockets are empty.
    Prior to the game starting there is a message saying downloading data.


    Jumpers set on GFX PCB to main monitor:
    4A   set to TWN (twin GFX PCBs)
    37D  set to Master


    Jumpers set on GFX PCB to scope monitor:
    4A   set to TWN (twin GFX PCBs)
    37D  set to Slave


    1 x 64.0000MHz
    1 x 36.0000MHz  (to 27L, ADSP)

    21E  AV9170           ICS, Clock synchroniser and multiplier

    27L  ADSP-21062       Analog Devices SHARC ADSP, 512k flash, Konami no. 022M16C
    15T  0000033906       Konami Custom, 160 pin PQFP
    19R  W241024AI-20     Winbond, 1Meg SRAM
    22R  W241024AI-20     Winbond, 1Meg SRAM
    25R  W241024AI-20     Winbond, 1Meg SRAM
    29R  W241024AI-20     Winbond, 1Meg SRAM
    19P  W241024AI-20     Winbond, 1Meg SRAM
    22P  W241024AI-20     Winbond, 1Meg SRAM
    25P  W241024AI-20     Winbond, 1Meg SRAM
    29P  W241024AI-20     Winbond, 1Meg SRAM
    18N  W24257AJ-15      Winbond, 256K SRAM
    14N  W24257AJ-15      Winbond, 256K SRAM
    18M  W24257AJ-15      Winbond, 256K SRAM
    14M  W24257AJ-15      Winbond, 256K SRAM

    28D  000037122        Konami Custom, 208 pin PQFP
    33E  W24257AJ-15      Winbond, 256K SRAM
    33D  W24257AJ-15      Winbond, 256K SRAM
    33C  W24257AJ-15      Winbond, 256K SRAM
    27A  W241024AI-20     Winbond, 1Meg SRAM
    30A  W241024AI-20     Winbond, 1Meg SRAM
    32A  W241024AI-20     Winbond, 1Meg SRAM
    35A  W241024AI-20     Winbond, 1Meg SRAM

    7K   500-0010-01      3DFX, Texture processor
    16F  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    13F  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    9F   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    5F   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    16D  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    13D  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    9D   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    5D   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg

    9P   500-0009-01      3DFX, Pixel processor
    10U  SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    7U   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    3S   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
    3R   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg

    27G  XC9536           Xilinx, CPLD, Konami no. Q830B1
    21C  MC44200FT        Motorola, 3 Channel video D/A converter
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "cpu/sharc/sharc.h"
#include "machine/adc1213x.h"
#include "machine/eeprom.h"
#include "machine/k033906.h"
#include "machine/konppc.h"
#include "machine/timekpr.h"
#include "sound/rf5c400.h"
#include "sound/k056800.h"
#include "video/voodoo.h"
#include "video/konicdev.h"
#include "rendlay.h"

static UINT8 led_reg0, led_reg1;
static UINT32 *workram;
static UINT32 *sharc_dataram[2];
static UINT8 *jvs_sdata;
static UINT32 jvs_sdata_ptr = 0;


static READ32_HANDLER( hornet_k037122_sram_r )
{
	running_device *k037122 = space->machine->device(get_cgboard_id() ? "k037122_2" : "k037122_1");
	return k037122_sram_r(k037122, offset, mem_mask);
}

static WRITE32_HANDLER( hornet_k037122_sram_w )
{
	running_device *k037122 = space->machine->device(get_cgboard_id() ? "k037122_2" : "k037122_1");
	k037122_sram_w(k037122, offset, data, mem_mask);
}


static READ32_HANDLER( hornet_k037122_char_r )
{
	running_device *k037122 = space->machine->device(get_cgboard_id() ? "k037122_2" : "k037122_1");
	return k037122_char_r(k037122, offset, mem_mask);
}

static WRITE32_HANDLER( hornet_k037122_char_w )
{
	running_device *k037122 = space->machine->device(get_cgboard_id() ? "k037122_2" : "k037122_1");
	k037122_char_w(k037122, offset, data, mem_mask);
}

static READ32_HANDLER( hornet_k037122_reg_r )
{
	running_device *k037122 = space->machine->device(get_cgboard_id() ? "k037122_2" : "k037122_1");
	return k037122_reg_r(k037122, offset, mem_mask);
}

static WRITE32_HANDLER( hornet_k037122_reg_w )
{
	running_device *k037122 = space->machine->device(get_cgboard_id() ? "k037122_2" : "k037122_1");
	k037122_reg_w(k037122, offset, data, mem_mask);
}

static void voodoo_vblank_0(running_device *device, int param)
{
	cputag_set_input_line(device->machine, "maincpu", INPUT_LINE_IRQ0, ASSERT_LINE);
}

static void voodoo_vblank_1(running_device *device, int param)
{
	cputag_set_input_line(device->machine, "maincpu", INPUT_LINE_IRQ1, ASSERT_LINE);
}

static VIDEO_UPDATE( hornet )
{
	running_device *voodoo = screen->machine->device("voodoo0");
	running_device *k037122 = screen->machine->device("k037122_1");

	voodoo_update(voodoo, bitmap, cliprect);

	k037122_tile_draw(k037122, bitmap, cliprect);

	draw_7segment_led(bitmap, 3, 3, led_reg0);
	draw_7segment_led(bitmap, 9, 3, led_reg1);
	return 0;
}

static VIDEO_UPDATE( hornet_2board )
{
	if (strcmp(screen->tag(), "lscreen") == 0)
	{
		running_device *k037122 = screen->machine->device("k037122_1");
		running_device *voodoo = screen->machine->device("voodoo0");
		voodoo_update(voodoo, bitmap, cliprect);

		/* TODO: tilemaps per screen */
		k037122_tile_draw(k037122, bitmap, cliprect);
	}
	else if (strcmp(screen->tag(), "rscreen") == 0)
	{
		running_device *k037122 = screen->machine->device("k037122_2");
		running_device *voodoo = screen->machine->device("voodoo1");
		voodoo_update(voodoo, bitmap, cliprect);

		/* TODO: tilemaps per screen */
		k037122_tile_draw(k037122, bitmap, cliprect);
	}

	draw_7segment_led(bitmap, 3, 3, led_reg0);
	draw_7segment_led(bitmap, 9, 3, led_reg1);
	return 0;
}

/*****************************************************************************/

static READ8_HANDLER( sysreg_r )
{
	UINT8 r = 0;
	static const char *const portnames[] = { "IN0", "IN1", "IN2" };
	running_device *adc12138 = space->machine->device("adc12138");
	running_device *eeprom = space->machine->device("eeprom");

	switch (offset)
	{
		case 0:	/* I/O port 0 */
		case 1:	/* I/O port 1 */
		case 2:	/* I/O port 2 */
			r = input_port_read(space->machine, portnames[offset]);
			break;

		case 3:	/* I/O port 3 */
			/*
                0x80 = JVSINIT (JAMMA I/F SENSE)
                0x40 = COMMST
                0x20 = GSENSE
                0x08 = EEPDO (EEPROM DO)
                0x04 = ADEOC (ADC EOC)
                0x02 = ADDOR (ADC DOR)
                0x01 = ADDO (ADC DO)
            */
			r = 0xf0 | (eeprom_read_bit(eeprom) << 3);
			r |= adc1213x_do_r(adc12138, 0) | (adc1213x_eoc_r(adc12138, 0) << 2);
			break;

		case 4:	/* I/O port 4 - DIP switches */
			r = input_port_read(space->machine, "DSW");
			break;
	}
	return r;
}

static WRITE8_HANDLER( sysreg_w )
{
	running_device *adc12138 = space->machine->device("adc12138");

	switch (offset)
	{
		case 0:	/* LED Register 0 */
			led_reg0 = data;
			break;

		case 1:	/* LED Register 1 */
			led_reg1 = data;
			break;

		case 2: /* Parallel data register */
			mame_printf_debug("Parallel data = %02X\n", data);
			break;

		case 3:	/* System Register 0 */
			/*
                0x80 = EEPWEN (EEPROM write enable)
                0x40 = EEPCS (EEPROM CS)
                0x20 = EEPSCL (EEPROM SCL?)
                0x10 = EEPDT (EEPROM data)
                0x08 = JVSTXEN / LAMP3 (something about JAMMA interface)
                0x04 = LAMP2
                0x02 = LAMP1
                0x01 = LAMP0
            */
			input_port_write(space->machine, "EEPROMOUT", data, 0xff);
			mame_printf_debug("System register 0 = %02X\n", data);
			break;

		case 4:	/* System Register 1 */
			/*
                0x80 = SNDRES (sound reset)
                0x40 = COMRES (COM reset)
                0x20 = COINRQ2 (EEPROM SCL?)
                0x10 = COINRQ1 (EEPROM data)
                0x08 = ADCS (ADC CS)
                0x04 = ADCONV (ADC CONV)
                0x02 = ADDI (ADC DI)
                0x01 = ADDSCLK (ADC SCLK)
            */
			adc1213x_cs_w(adc12138, 0, (data >> 3) & 0x1);
			adc1213x_conv_w(adc12138, 0, (data >> 2) & 0x1);
			adc1213x_di_w(adc12138, 0, (data >> 1) & 0x1);
			adc1213x_sclk_w(adc12138, 0, data & 0x1);

			cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
			mame_printf_debug("System register 1 = %02X\n", data);
			break;

		case 5:	/* Sound Control Register */
			/*
                0x80 = MODE1
                0x40 = MUTE1
                0x20 = DEEN1
                0x10 = ATCK1
                0x08 = MODE0
                0x04 = MUTE0
                0x02 = DEEN0
                0x01 = ATCK0
            */
			mame_printf_debug("Sound control register = %02X\n", data);
			break;

		case 6:	/* WDT Register */
			/*
                0x80 = WDTCLK
            */
			if (data & 0x80)
				watchdog_reset(space->machine);
			break;

		case 7:	/* CG Control Register */
			/*
                0x80 = EXRES1
                0x40 = EXRES0
                0x20 = EXID1
                0x10 = EXID0
                0x01 = EXRGB
            */
			if (data & 0x80)
				cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_IRQ1, CLEAR_LINE);
			if (data & 0x40)
				cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_IRQ0, CLEAR_LINE);
			set_cgboard_id((data >> 4) & 3);
			break;
	}
}

/*****************************************************************************/

static WRITE32_HANDLER( comm1_w )
{
	printf("comm1_w: %08X, %08X, %08X\n", offset, data, mem_mask);
}

static WRITE32_HANDLER( comm_rombank_w )
{
	int bank = data >> 24;
	UINT8 *usr3 = memory_region(space->machine, "user3");
	if (usr3 != NULL)
		memory_set_bank(space->machine, "bank1", bank & 0x7f);
}

static READ32_HANDLER( comm0_unk_r )
{
//  printf("comm0_unk_r: %08X, %08X\n", offset, mem_mask);
	return 0xffffffff;
}

static UINT16 gn680_latch, gn680_ret0, gn680_ret1;

static READ32_HANDLER(gun_r)
{
	return gn680_ret0<<16 | gn680_ret1;
}

static WRITE32_HANDLER(gun_w)
{
	if (mem_mask == 0xffff0000)
	{
		gn680_latch = data>>16;
		cputag_set_input_line(space->machine, "gn680", M68K_IRQ_6, HOLD_LINE);
	}
}

/*****************************************************************************/

static ADDRESS_MAP_START( hornet_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_BASE(&workram)		/* Work RAM */
	AM_RANGE(0x74000000, 0x740000ff) AM_READWRITE(hornet_k037122_reg_r, hornet_k037122_reg_w)
	AM_RANGE(0x74020000, 0x7403ffff) AM_READWRITE(hornet_k037122_sram_r, hornet_k037122_sram_w)
	AM_RANGE(0x74040000, 0x7407ffff) AM_READWRITE(hornet_k037122_char_r, hornet_k037122_char_w)
	AM_RANGE(0x74080000, 0x7408000f) AM_READWRITE(gun_r, gun_w)
	AM_RANGE(0x78000000, 0x7800ffff) AM_READWRITE(cgboard_dsp_shared_r_ppc, cgboard_dsp_shared_w_ppc)
	AM_RANGE(0x780c0000, 0x780c0003) AM_READWRITE(cgboard_dsp_comm_r_ppc, cgboard_dsp_comm_w_ppc)
	AM_RANGE(0x7d000000, 0x7d00ffff) AM_READ8(sysreg_r, 0xffffffff)
	AM_RANGE(0x7d010000, 0x7d01ffff) AM_WRITE8(sysreg_w, 0xffffffff)
	AM_RANGE(0x7d020000, 0x7d021fff) AM_DEVREADWRITE8("m48t58", timekeeper_r, timekeeper_w, 0xffffffff)	/* M48T58Y RTC/NVRAM */
	AM_RANGE(0x7d030000, 0x7d030007) AM_DEVREADWRITE("k056800", k056800_host_r, k056800_host_w)
	AM_RANGE(0x7d042000, 0x7d043fff) AM_RAM				/* COMM BOARD 0 */
	AM_RANGE(0x7d044000, 0x7d044007) AM_READ(comm0_unk_r)
	AM_RANGE(0x7d048000, 0x7d048003) AM_WRITE(comm1_w)
	AM_RANGE(0x7d04a000, 0x7d04a003) AM_WRITE(comm_rombank_w)
	AM_RANGE(0x7d050000, 0x7d05ffff) AM_ROMBANK("bank1")		/* COMM BOARD 1 */
	AM_RANGE(0x7e000000, 0x7e7fffff) AM_ROM AM_REGION("user2", 0)		/* Data ROM */
	AM_RANGE(0x7f000000, 0x7f3fffff) AM_ROM AM_SHARE("share2")
	AM_RANGE(0x7fc00000, 0x7fffffff) AM_ROM AM_REGION("user1", 0) AM_SHARE("share2")	/* Program ROM */
ADDRESS_MAP_END

/*****************************************************************************/

static ADDRESS_MAP_START( sound_memmap, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM		/* Work RAM */
	AM_RANGE(0x200000, 0x200fff) AM_DEVREADWRITE("rfsnd", rf5c400_r, rf5c400_w)		/* Ricoh RF5C400 */
	AM_RANGE(0x300000, 0x30000f) AM_DEVREADWRITE("k056800", k056800_sound_r, k056800_sound_w)
	AM_RANGE(0x480000, 0x480001) AM_WRITENOP
	AM_RANGE(0x4c0000, 0x4c0001) AM_WRITENOP
	AM_RANGE(0x500000, 0x500001) AM_WRITENOP
	AM_RANGE(0x600000, 0x600001) AM_NOP
ADDRESS_MAP_END

/*****************************************************************************/

static WRITE16_HANDLER(gn680_sysctrl)
{
	// bit 15 = watchdog toggle
	// lower 4 bits = LEDs?
}

static READ16_HANDLER(gn680_latch_r)
{
	cputag_set_input_line(space->machine, "gn680", M68K_IRQ_6, CLEAR_LINE);

	return gn680_latch;
}

static WRITE16_HANDLER(gn680_latch_w)
{
	if (offset)
	{
		gn680_ret1 = data;
	}
	else
	{
		gn680_ret0 = data;
	}
}

// WORD at 30000e: IRQ 4 tests bits 6 and 7, IRQ5 tests bits 4 and 5
// (vsync and hsync status for each of the two screens?)

static ADDRESS_MAP_START( gn680_memmap, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x200000, 0x203fff) AM_RAM
	AM_RANGE(0x300000, 0x300001) AM_WRITE(gn680_sysctrl)
	AM_RANGE(0x314000, 0x317fff) AM_RAM
	AM_RANGE(0x400000, 0x400003) AM_READWRITE(gn680_latch_r, gn680_latch_w)
	AM_RANGE(0x400008, 0x400009) AM_WRITENOP	// writes 0001 00fe each time IRQ 6 triggers
ADDRESS_MAP_END

/*****************************************************************************/

static READ32_HANDLER( dsp_dataram0_r )
{
	return sharc_dataram[0][offset] & 0xffff;
}

static WRITE32_HANDLER( dsp_dataram0_w )
{
	sharc_dataram[0][offset] = data;
}

static READ32_HANDLER( dsp_dataram1_r )
{
	return sharc_dataram[1][offset] & 0xffff;
}

static WRITE32_HANDLER( dsp_dataram1_w )
{
	sharc_dataram[1][offset] = data;
}

static ADDRESS_MAP_START( sharc0_map, ADDRESS_SPACE_DATA, 32 )
	AM_RANGE(0x0400000, 0x041ffff) AM_READWRITE(cgboard_0_shared_sharc_r, cgboard_0_shared_sharc_w)
	AM_RANGE(0x0500000, 0x05fffff) AM_READWRITE(dsp_dataram0_r, dsp_dataram0_w) AM_BASE(&sharc_dataram[0])
	AM_RANGE(0x1400000, 0x14fffff) AM_RAM
	AM_RANGE(0x2400000, 0x27fffff) AM_DEVREADWRITE("voodoo0", voodoo_r, voodoo_w)
	AM_RANGE(0x3400000, 0x34000ff) AM_READWRITE(cgboard_0_comm_sharc_r, cgboard_0_comm_sharc_w)
	AM_RANGE(0x3500000, 0x35000ff) AM_READWRITE(K033906_0_r, K033906_0_w)
	AM_RANGE(0x3600000, 0x37fffff) AM_ROMBANK("bank5")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sharc1_map, ADDRESS_SPACE_DATA, 32 )
	AM_RANGE(0x0400000, 0x041ffff) AM_READWRITE(cgboard_1_shared_sharc_r, cgboard_1_shared_sharc_w)
	AM_RANGE(0x0500000, 0x05fffff) AM_READWRITE(dsp_dataram1_r, dsp_dataram1_w) AM_BASE(&sharc_dataram[1])
	AM_RANGE(0x1400000, 0x14fffff) AM_RAM
	AM_RANGE(0x2400000, 0x27fffff) AM_DEVREADWRITE("voodoo1", voodoo_r, voodoo_w)
	AM_RANGE(0x3400000, 0x34000ff) AM_READWRITE(cgboard_1_comm_sharc_r, cgboard_1_comm_sharc_w)
	AM_RANGE(0x3500000, 0x35000ff) AM_READWRITE(K033906_1_r, K033906_1_w)
	AM_RANGE(0x3600000, 0x37fffff) AM_ROMBANK("bank6")
ADDRESS_MAP_END

/*****************************************************************************/

static INPUT_PORTS_START( hornet )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_7)
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x80, 0x00, "Test Mode" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Screen Flip (H)" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Screen Flip (V)" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP5" ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Harness" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING( 0x04, "JVS" )
	PORT_DIPSETTING( 0x00, "JAMMA" )
	PORT_DIPNAME( 0x02, 0x02, "DIP7" ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "Monitor Type" ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING( 0x01, "24KHz" )
	PORT_DIPSETTING( 0x00, "15KHz" )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
INPUT_PORTS_END

static INPUT_PORTS_START( sscope )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)		// Gun trigger
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_7)
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x80, 0x00, "Test Mode" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Screen Flip (H)" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Screen Flip (V)" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP5" ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Harness" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING( 0x04, "JVS" )
	PORT_DIPSETTING( 0x00, "JAMMA" )
	PORT_DIPNAME( 0x02, 0x02, "DIP7" ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "Monitor Type" ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING( 0x01, "24KHz" )
	PORT_DIPSETTING( 0x00, "15KHz" )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
INPUT_PORTS_END

static const sharc_config sharc_cfg =
{
	BOOT_MODE_EPROM
};

/* PowerPC interrupts

    IRQ0:   Vblank CG Board 0
    IRQ1:   Vblank CG Board 1
    IRQ2:   LANC
    DMA0
    NMI:    SCI

*/

static MACHINE_START( hornet )
{
	jvs_sdata_ptr = 0;
	jvs_sdata = auto_alloc_array_clear(machine, UINT8, 1024);

	/* set conservative DRC options */
	ppcdrc_set_options(machine->device("maincpu"), PPCDRC_COMPATIBLE_OPTIONS);

	/* configure fast RAM regions for DRC */
	ppcdrc_add_fastram(machine->device("maincpu"), 0x00000000, 0x003fffff, FALSE, workram);

	state_save_register_global(machine, led_reg0);
	state_save_register_global(machine, led_reg1);
	state_save_register_global_pointer(machine, jvs_sdata, 1024);
	state_save_register_global(machine, jvs_sdata_ptr);
}

static MACHINE_RESET( hornet )
{
	UINT8 *usr3 = memory_region(machine, "user3");
	UINT8 *usr5 = memory_region(machine, "user5");
	if (usr3 != NULL)
	{
		memory_configure_bank(machine, "bank1", 0, memory_region_length(machine, "user3") / 0x40000, usr3, 0x40000);
		memory_set_bank(machine, "bank1", 0);
	}

	cputag_set_input_line(machine, "dsp", INPUT_LINE_RESET, ASSERT_LINE);

	if (usr5)
		memory_set_bankptr(machine, "bank5", usr5);
}

static double adc12138_input_callback( running_device *device, UINT8 input )
{
	return (double)0.0;
}

static const adc12138_interface hornet_adc_interface = {
	adc12138_input_callback
};

static TIMER_CALLBACK( irq_off )
{
	cputag_set_input_line(machine, "audiocpu", param, CLEAR_LINE);
}

static void sound_irq_callback( running_machine *machine, int irq )
{
	int line = (irq == 0) ? INPUT_LINE_IRQ1 : INPUT_LINE_IRQ2;

	cputag_set_input_line(machine, "audiocpu", line, ASSERT_LINE);
	timer_set(machine, ATTOTIME_IN_USEC(1), NULL, line, irq_off);
}

static const k056800_interface hornet_k056800_interface =
{
	sound_irq_callback
};

static const k033906_interface hornet_k033906_intf_0 =
{
	"voodoo0"
};

static const k033906_interface hornet_k033906_intf_1 =
{
	"voodoo1"
};

static const k037122_interface hornet_k037122_intf =
{
	"screen", 0
};

static const k037122_interface hornet_k037122_intf_l =
{
	"lscreen", 0
};

static const k037122_interface hornet_k037122_intf_r =
{
	"rscreen", 1
};

static MACHINE_DRIVER_START( hornet )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", PPC403GA, 64000000/2)	/* PowerPC 403GA 32MHz */
	MDRV_CPU_PROGRAM_MAP(hornet_map)

	MDRV_CPU_ADD("audiocpu", M68000, 64000000/4)	/* 16MHz */
	MDRV_CPU_PROGRAM_MAP(sound_memmap)

	MDRV_CPU_ADD("dsp", ADSP21062, 36000000)
	MDRV_CPU_CONFIG(sharc_cfg)
	MDRV_CPU_DATA_MAP(sharc0_map)

	MDRV_QUANTUM_TIME(HZ(6000))

	MDRV_MACHINE_START( hornet )
	MDRV_MACHINE_RESET( hornet )

	MDRV_EEPROM_93C46_ADD("eeprom")

	MDRV_3DFX_VOODOO_1_ADD("voodoo0", STD_VOODOO_1_CLOCK, 2, "screen")
	MDRV_3DFX_VOODOO_CPU("dsp")
	MDRV_3DFX_VOODOO_TMU_MEMORY(0, 4)
	MDRV_3DFX_VOODOO_VBLANK(voodoo_vblank_0)

	MDRV_K033906_ADD("k033906_1", hornet_k033906_intf_0)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 48*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 48*8-1)

	MDRV_PALETTE_LENGTH(65536)

	MDRV_VIDEO_UPDATE(hornet)

	MDRV_K037122_ADD("k037122_1", hornet_k037122_intf)

	MDRV_K056800_ADD("k056800", hornet_k056800_interface)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("rfsnd", RF5C400, 16934400)	// value from Guru readme, gives 44100 Hz sample rate
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_M48T58_ADD( "m48t58" )

	MDRV_ADC12138_ADD( "adc12138", hornet_adc_interface )
MACHINE_DRIVER_END


static MACHINE_RESET( hornet_2board )
{
	UINT8 *usr3 = memory_region(machine, "user3");
	UINT8 *usr5 = memory_region(machine, "user5");

	if (usr3 != NULL)
	{
		memory_configure_bank(machine, "bank1", 0, memory_region_length(machine, "user3") / 0x40000, usr3, 0x40000);
		memory_set_bank(machine, "bank1", 0);
	}
	cputag_set_input_line(machine, "dsp", INPUT_LINE_RESET, ASSERT_LINE);
	cputag_set_input_line(machine, "dsp2", INPUT_LINE_RESET, ASSERT_LINE);

	if (usr5)
		memory_set_bankptr(machine, "bank5", usr5);
}

static MACHINE_DRIVER_START( hornet_2board )

	MDRV_IMPORT_FROM(hornet)

	MDRV_CPU_ADD("dsp2", ADSP21062, 36000000)
	MDRV_CPU_CONFIG(sharc_cfg)
	MDRV_CPU_DATA_MAP(sharc1_map)

	MDRV_MACHINE_RESET(hornet_2board)

	MDRV_VIDEO_UPDATE(hornet_2board)

	MDRV_DEVICE_REMOVE("k037122_1")
	MDRV_K037122_ADD("k037122_1", hornet_k037122_intf_l)
	MDRV_K037122_ADD("k037122_2", hornet_k037122_intf_r)

	MDRV_DEVICE_REMOVE("voodoo0")
	MDRV_3DFX_VOODOO_1_ADD("voodoo0", STD_VOODOO_1_CLOCK, 2, "lscreen")
	MDRV_3DFX_VOODOO_CPU("dsp")
	MDRV_3DFX_VOODOO_TMU_MEMORY(0, 4)
	MDRV_3DFX_VOODOO_VBLANK(voodoo_vblank_0)

	MDRV_3DFX_VOODOO_1_ADD("voodoo1", STD_VOODOO_1_CLOCK, 2, "rscreen")
	MDRV_3DFX_VOODOO_CPU("dsp2")
	MDRV_3DFX_VOODOO_TMU_MEMORY(0, 4)
	MDRV_3DFX_VOODOO_VBLANK(voodoo_vblank_1)

	MDRV_K033906_ADD("k033906_2", hornet_k033906_intf_1)

	/* video hardware */
	MDRV_PALETTE_LENGTH(65536)

	MDRV_DEVICE_REMOVE("screen")

	MDRV_SCREEN_ADD("lscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_SIZE(512, 384)
	MDRV_SCREEN_VISIBLE_AREA(0, 511, 0, 383)

	MDRV_SCREEN_ADD("rscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_SIZE(512, 384)
	MDRV_SCREEN_VISIBLE_AREA(0, 511, 0, 383)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( terabrst )
	MDRV_IMPORT_FROM(hornet_2board)

	MDRV_CPU_ADD("gn680", M68000, 32000000/2)	/* 16MHz */
	MDRV_CPU_PROGRAM_MAP(gn680_memmap)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( hornet_2board_v2 )
	MDRV_IMPORT_FROM(hornet_2board)

	MDRV_DEVICE_REMOVE("voodoo0")
	MDRV_3DFX_VOODOO_2_ADD("voodoo0", STD_VOODOO_2_CLOCK, 2, "lscreen")
	MDRV_3DFX_VOODOO_CPU("dsp")
	MDRV_3DFX_VOODOO_TMU_MEMORY(0, 4)
	MDRV_3DFX_VOODOO_VBLANK(voodoo_vblank_0)

	MDRV_DEVICE_REMOVE("voodoo1")
	MDRV_3DFX_VOODOO_2_ADD("voodoo1", STD_VOODOO_2_CLOCK, 2, "rscreen")
	MDRV_3DFX_VOODOO_CPU("dsp2")
	MDRV_3DFX_VOODOO_TMU_MEMORY(0, 4)
	MDRV_3DFX_VOODOO_VBLANK(voodoo_vblank_1)
MACHINE_DRIVER_END


/*****************************************************************************/

static void jamma_jvs_cmd_exec(running_machine *machine);

static void jamma_jvs_w(running_device *device, UINT8 data)
{
	if (jvs_sdata_ptr == 0 && data != 0xe0)
		return;
	jvs_sdata[jvs_sdata_ptr] = data;
	jvs_sdata_ptr++;

	if (jvs_sdata_ptr >= 3 && jvs_sdata_ptr >= 3 + jvs_sdata[2])
		jamma_jvs_cmd_exec(device->machine);
}

static int jvs_encode_data(running_machine *machine, UINT8 *in, int length)
{
	int inptr = 0;
	int sum = 0;

	while (inptr < length)
	{
		UINT8 b = in[inptr++];
		if (b == 0xe0)
		{
			sum += 0xd0 + 0xdf;
			ppc4xx_spu_receive_byte(machine->device("maincpu"), 0xd0);
			ppc4xx_spu_receive_byte(machine->device("maincpu"), 0xdf);
		}
		else if (b == 0xd0)
		{
			sum += 0xd0 + 0xcf;
			ppc4xx_spu_receive_byte(machine->device("maincpu"), 0xd0);
			ppc4xx_spu_receive_byte(machine->device("maincpu"), 0xcf);
		}
		else
		{
			sum += b;
			ppc4xx_spu_receive_byte(machine->device("maincpu"), b);
		}
	}
	return sum;
}

static int jvs_decode_data(UINT8 *in, UINT8 *out, int length)
{
	int outptr = 0;
	int inptr = 0;

	while (inptr < length)
	{
		UINT8 b = in[inptr++];
		if (b == 0xd0)
		{
			UINT8 b2 = in[inptr++];
			out[outptr++] = b2 + 1;
		}
		else
		{
			out[outptr++] = b;
		}
	};

	return outptr;
}

static void jamma_jvs_cmd_exec(running_machine *machine)
{
	UINT8 sync, node, byte_num;
	UINT8 data[1024], rdata[1024];
#if 0
	int length;
#endif
	int rdata_ptr;
	int sum;

	sync = jvs_sdata[0];
	node = jvs_sdata[1];
	byte_num = jvs_sdata[2];

#if 0
	length =
#endif
		jvs_decode_data(&jvs_sdata[3], data, byte_num-1);
#if 0
    printf("jvs input data:\n");
    for (i=0; i < byte_num; i++)
    {
        printf("%02X ", jvs_sdata[3+i]);
    }
    printf("\n");

    printf("jvs data decoded to:\n");
    for (i=0; i < length; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\n\n");
#endif

	// clear return data
	memset(rdata, 0, sizeof(rdata));
	rdata_ptr = 0;

	// status
	rdata[rdata_ptr++] = 0x01;		// normal

	// handle the command
	switch (data[0])
	{
		case 0xf0:		// Reset
		{
			break;
		}
		case 0xf1:		// Address setting
		{
			rdata[rdata_ptr++] = 0x01;		// report data (normal)
			break;
		}
		case 0xfa:
		{
			break;
		}
		default:
		{
			fatalerror("jamma_jvs_cmd_exec: unknown command %02X\n", data[0]);
		}
	}

	// write jvs return data
	sum = 0x00 + (rdata_ptr+1);
	ppc4xx_spu_receive_byte(machine->device("maincpu"), 0xe0);			// sync
	ppc4xx_spu_receive_byte(machine->device("maincpu"), 0x00);			// node
	ppc4xx_spu_receive_byte(machine->device("maincpu"), rdata_ptr + 1);	// num of bytes
	sum += jvs_encode_data(machine, rdata, rdata_ptr);
	ppc4xx_spu_receive_byte(machine->device("maincpu"), sum - 1);		// checksum

	jvs_sdata_ptr = 0;
}

/*****************************************************************************/


static DRIVER_INIT(hornet)
{
	init_konami_cgboard(machine, 1, CGBOARD_TYPE_HORNET);
	set_cgboard_texture_bank(machine, 0, "bank5", memory_region(machine, "user5"));

	led_reg0 = led_reg1 = 0x7f;

	ppc4xx_spu_set_tx_handler(machine->device("maincpu"), jamma_jvs_w);
}

static DRIVER_INIT(hornet_2board)
{
	init_konami_cgboard(machine, 2, CGBOARD_TYPE_HORNET);
	set_cgboard_texture_bank(machine, 0, "bank5", memory_region(machine, "user5"));
	set_cgboard_texture_bank(machine, 1, "bank6", memory_region(machine, "user5"));

	led_reg0 = led_reg1 = 0x7f;

	ppc4xx_spu_set_tx_handler(machine->device("maincpu"), jamma_jvs_w);
}

/*****************************************************************************/

ROM_START(sscope)
	ROM_REGION32_BE(0x400000, "user1", 0)	/* PowerPC program */
	ROM_LOAD16_WORD_SWAP("830d01.27p", 0x200000, 0x200000, CRC(de9b3dfa) SHA1(660652a5f745cb04687481c3626d8a43cd169193) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "user2", ROMREGION_ERASE00)	/* Data roms */

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68K Program */
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "user5", 0)		/* CG Board texture roms */
	ROM_LOAD32_WORD( "830a14.u32", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.u24", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION(0x1000000, "rfsnd", 0)		/* PCM sample roms */
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y.35d", 0x000000, 0x002000, CRC(b077e262) SHA1(5cdcc1b742bf23562f4558216063fea903f045ab) )
ROM_END

ROM_START(sscopea)
	ROM_REGION32_BE(0x400000, "user1", 0)	/* PowerPC program */
	ROM_LOAD16_WORD_SWAP("830b01.27p", 0x200000, 0x200000, CRC(3b6bb075) SHA1(babc134c3a20c7cdcaa735d5f1fd5cab38667a14) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "user2", ROMREGION_ERASE00)	/* Data roms */

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68K Program */
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "user5", 0)		/* CG Board texture roms */
	ROM_LOAD32_WORD( "830a14.u32", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.u24", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION(0x1000000, "rfsnd", 0)		/* PCM sample roms */
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(ee815325) SHA1(91b10802791b68a8360c0cd6c376c0c4bbbc6fa0) )
ROM_END

ROM_START(sscopeb)
	ROM_REGION32_BE(0x400000, "user1", 0)	/* PowerPC program */
	ROM_LOAD16_WORD_SWAP("830a01.27p", 0x200000, 0x200000, CRC(39e353f1) SHA1(569b06969ae7a690f6d6e63cc3b5336061663a37) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "user2", ROMREGION_ERASE00)	/* Data roms */

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68K Program */
	ROM_LOAD16_WORD_SWAP("830a08.7s", 0x000000, 0x80000, CRC(2805ea1d) SHA1(2556a51ee98cb8f59bf081e916c69a24532196f1) )

	ROM_REGION(0x1000000, "user5", 0)		/* CG Board texture roms */
	ROM_LOAD32_WORD( "830a14.u32", 0x000000, 0x400000, CRC(335793e1) SHA1(d582b53c3853abd59bc728f619a30c27cfc9497c) )
	ROM_LOAD32_WORD( "830a13.u24", 0x000002, 0x400000, CRC(d6e7877e) SHA1(b4d0e17ada7dd126ec564a20e7140775b4b3fdb7) )

	ROM_REGION(0x1000000, "rfsnd", 0)		/* PCM sample roms */
	ROM_LOAD( "830a09.16p", 0x000000, 0x400000, CRC(e4b9f305) SHA1(ce2c6f63bdc9374dde48d8359102b57e48b4fdeb) )
	ROM_LOAD( "830a10.14p", 0x400000, 0x400000, CRC(8b8aaf7e) SHA1(49b694dc171c149056b87c15410a6bf37ff2987f) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(ee815325) SHA1(91b10802791b68a8360c0cd6c376c0c4bbbc6fa0) )
ROM_END

ROM_START(sscope2)
	ROM_REGION32_BE(0x400000, "user1", 0)	/* PowerPC program */
	ROM_LOAD16_WORD_SWAP("931d01.bin", 0x200000, 0x200000, CRC(4065fde6) SHA1(84f2dedc3e8f61651b22c0a21433a64993e1b9e2) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "user2", 0)	/* Data roms */
	ROM_LOAD32_WORD_SWAP("931a04.bin", 0x000000, 0x200000, CRC(4f5917e6) SHA1(a63a107f1d6d9756e4ab0965d72ea446f0692814) )

	ROM_REGION32_BE(0x800000, "user3", 0)	/* Comm board roms */
	ROM_LOAD("931a19.bin", 0x000000, 0x400000, CRC(8b25a6f1) SHA1(41f9c2046a6aae1e9f5f3ffa3e0ffb15eba46211) )
	ROM_LOAD("931a20.bin", 0x400000, 0x400000, CRC(ecf665f6) SHA1(5a73e87435560a7bb2d0f9be7fba12254b18708d) )

	ROM_REGION(0x800000, "user5", ROMREGION_ERASE00)	/* CG Board texture roms */

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68K Program */
	ROM_LOAD16_WORD_SWAP("931a08.bin", 0x000000, 0x80000, CRC(1597d604) SHA1(a1eab4d25907930b59ea558b484c3b6ddcb9303c) )

	ROM_REGION(0xc00000, "rfsnd", 0)		/* PCM sample roms */
	ROM_LOAD( "931a09.bin",   0x000000, 0x400000, CRC(694c354c) SHA1(42f54254a5959e1b341f2801f1ad032c4ed6f329) )
	ROM_LOAD( "931a10.bin",   0x400000, 0x400000, CRC(78ceb519) SHA1(e61c0d21b6dc37a9293e72814474f5aee59115ad) )
	ROM_LOAD( "931a11.bin",   0x800000, 0x400000, CRC(9c8362b2) SHA1(a8158c4db386e2bbd61dc9a600720f07a1eba294) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(d4e69d7a) SHA1(1e29eecf4886e5e098a388dedd5f3901c2bb65e5) )
ROM_END

ROM_START(gradius4)
	ROM_REGION32_BE(0x400000, "user1", 0)	/* PowerPC program */
	ROM_LOAD16_WORD_SWAP( "837c01.27p",   0x200000, 0x200000, CRC(ce003123) SHA1(15e33997be2c1b3f71998627c540db378680a7a1) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "user2", 0)	/* Data roms */
	ROM_LOAD32_WORD_SWAP( "837a04.16t",   0x000000, 0x200000, CRC(18453b59) SHA1(3c75a54d8c09c0796223b42d30fb3867a911a074) )
	ROM_LOAD32_WORD_SWAP( "837a05.14t",   0x000002, 0x200000, CRC(77178633) SHA1(ececdd501d0692390325c8dad6dbb068808a8b26) )

	ROM_REGION32_BE(0x1000000, "user5", 0)	/* CG Board texture roms */
	ROM_LOAD32_WORD_SWAP( "837a14.32u",   0x000002, 0x400000, CRC(ff1b5d18) SHA1(7a38362170133dcc6ea01eb62981845917b85c36) )
	ROM_LOAD32_WORD_SWAP( "837a13.24u",   0x000000, 0x400000, CRC(d86e10ff) SHA1(6de1179d7081d9a93ab6df47692d3efc190c38ba) )
	ROM_LOAD32_WORD_SWAP( "837a16.32v",   0x800002, 0x400000, CRC(bb7a7558) SHA1(8c8cc062793c2dcfa72657b6ea0813d7223a0b87) )
	ROM_LOAD32_WORD_SWAP( "837a15.24v",   0x800000, 0x400000, CRC(e0620737) SHA1(c14078cdb44f75c7c956b3627045d8494941d6b4) )

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68K Program */
	ROM_LOAD16_WORD_SWAP( "837a08.7s",    0x000000, 0x080000, CRC(c3a7ff56) SHA1(9d8d033277d560b58da151338d14b4758a9235ea) )

	ROM_REGION(0x800000, "rfsnd", 0)		/* PCM sample roms */
	ROM_LOAD( "837a09.16p",   0x000000, 0x400000, CRC(fb8f3dc2) SHA1(69e314ac06308c5a24309abc3d7b05af6c0302a8) )
	ROM_LOAD( "837a10.14p",   0x400000, 0x400000, CRC(1419cad2) SHA1(a6369a5c29813fa51e8246d0c091736f32994f3d) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(935f9d05) SHA1(c3a787dff1b2ac4942858ffa1574405db01292b6) )
ROM_END

ROM_START(nbapbp)
	ROM_REGION32_BE(0x400000, "user1", 0)	/* PowerPC program */
	ROM_LOAD16_WORD_SWAP( "778a01.27p",   0x200000, 0x200000, CRC(e70019ce) SHA1(8b187b6e670fdc88771da08a56685cd621b139dc) )
	ROM_RELOAD(0x000000, 0x200000)

	ROM_REGION32_BE(0x800000, "user2", 0)	/* Data roms */
	ROM_LOAD32_WORD_SWAP( "778a04.16t",   0x000000, 0x400000, CRC(62c70132) SHA1(405aed149fc51e0adfa3ace3c644e47d53cf1ee3) )
	ROM_LOAD32_WORD_SWAP( "778a05.14t",   0x000002, 0x400000, CRC(03249803) SHA1(f632a5f1dfa0a8500407214df0ec8d98ce09bc2b) )

	ROM_REGION32_BE(0x1000000, "user5", 0)	/* CG Board texture roms */
	ROM_LOAD32_WORD_SWAP( "778a14.32u",   0x000002, 0x400000, CRC(db0c278d) SHA1(bb9884b6cdcdb707fff7e56e92e2ede062abcfd3) )
	ROM_LOAD32_WORD_SWAP( "778a13.24u",   0x000000, 0x400000, CRC(47fda9cc) SHA1(4aae01c1f1861b4b12a3f9de6b39eb4d11a9736b) )
	ROM_LOAD32_WORD_SWAP( "778a16.32v",   0x800002, 0x400000, CRC(6c0f46ea) SHA1(c6b9fbe14e13114a91a5925a0b46496260539687) )
	ROM_LOAD32_WORD_SWAP( "778a15.24v",   0x800000, 0x400000, CRC(d176ad0d) SHA1(2be755dfa3f60379d396734809bbaaaad49e0db5) )

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68K Program */
	ROM_LOAD16_WORD_SWAP( "778a08.7s",    0x000000, 0x080000, CRC(6259b4bf) SHA1(d0c38870495c9a07984b4b85e736d6477dd44832) )

	ROM_REGION(0x1000000, "rfsnd", 0)		/* PCM sample roms */
	ROM_LOAD( "778a09.16p",   0x000000, 0x400000, CRC(e8c6fd93) SHA1(dd378b67b3b7dd932e4b39fbf4321e706522247f) )
	ROM_LOAD( "778a10.14p",   0x400000, 0x400000, CRC(c6a0857b) SHA1(976734ba56460fcc090619fbba043a3d888c4f4e) )
	ROM_LOAD( "778a11.12p",   0x800000, 0x400000, CRC(40199382) SHA1(bee268adf9b6634a4f6bb39278ecd02f2bdcb1f4) )
	ROM_LOAD( "778a12.9p",    0xc00000, 0x400000, CRC(27d0c724) SHA1(48e48cbaea6db0de8c3471a2eda6faaa16eed46e) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(3cff1b1d) SHA1(bed0fc657a785be0c69bb21ad52365635c83d751) )
ROM_END

ROM_START(terabrst)
	ROM_REGION32_BE(0x400000, "user1", 0)	/* PowerPC program */
        ROM_LOAD32_WORD_SWAP( "715l02.25p",   0x000000, 0x200000, CRC(79586f19) SHA1(8dcfed5d101ebe49d958a7a38d5472323f75dd1d) )
	ROM_LOAD32_WORD_SWAP( "715l03.22p",   0x000002, 0x200000, CRC(c193021e) SHA1(c934b7c4bdab0ceff0f1699fcf2fb7d90e2e8962) )

	ROM_REGION32_BE(0x800000, "user2", 0)	/* Data roms */
	ROM_LOAD32_WORD_SWAP( "715a04.16t",   0x000000, 0x200000, CRC(00d9567e) SHA1(fe372399ad0ae89d557c93c3145b38e3ed0f714d) )
	ROM_LOAD32_WORD_SWAP( "715a05.14t",   0x000002, 0x200000, CRC(462d53bf) SHA1(0216a84358571de6791365c69a1fa8fe2784148d) )

	ROM_REGION32_BE(0x1000000, "user5", 0)	/* CG Board texture roms */
	ROM_LOAD32_WORD_SWAP( "715a14.32u",   0x000002, 0x400000, CRC(bbb36be3) SHA1(c828d0af0546db02e87afe68423b9447db7c7e51) )
	ROM_LOAD32_WORD_SWAP( "715a13.24u",   0x000000, 0x400000, CRC(dbff58a1) SHA1(f0c60bb2cbf268cfcbdd65606ebb18f1b4839c0e) )

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68K Program */
	ROM_LOAD16_WORD_SWAP( "715a08.7s",    0x000000, 0x080000, CRC(3aa2f4a5) SHA1(bb43e5f5ef4ac51f228d4d825be66d3c720d51ea) )

	ROM_REGION(0x1000000, "rfsnd", 0)		/* PCM sample roms */
	ROM_LOAD( "715a09.16p",   0x000000, 0x400000, CRC(65845866) SHA1(d2a63d0deef1901e6fa21b55c5f96e1f781dceda) )
	ROM_LOAD( "715a10.14p",   0x400000, 0x400000, CRC(294fe71b) SHA1(ac5fff5627df1cee4f1e1867377f208b34334899) )

	ROM_REGION(0x20000, "gn680", 0)		/* 68K Program */
	ROM_LOAD16_WORD_SWAP( "715a17.20k",    0x000000, 0x020000, CRC(f0b7ba0c) SHA1(863b260824b0ae2f890ba84d1c9a8f436891b1ff) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "715uel_m48t58y.35d", 0x000000, 0x002000, CRC(57322db4) SHA1(59cb8cd6ab446bf8781e3dddf902a4ff2484068e) )
ROM_END

ROM_START(terabrsta)
	ROM_REGION32_BE(0x400000, "user1", 0)	/* PowerPC program */
	ROM_LOAD32_WORD_SWAP( "715a02.25p",   0x000000, 0x200000, CRC(070c48b3) SHA1(066cefbd34d8f6476083417471114f782bef97fb) )
	ROM_LOAD32_WORD_SWAP( "715a03.22p",   0x000002, 0x200000, CRC(f77d242f) SHA1(7680e4abcccd549b3f6d1d245f64631fab57e80d) )

	ROM_REGION32_BE(0x800000, "user2", 0)	/* Data roms */
	ROM_LOAD32_WORD_SWAP( "715a04.16t",   0x000000, 0x200000, CRC(00d9567e) SHA1(fe372399ad0ae89d557c93c3145b38e3ed0f714d) )
	ROM_LOAD32_WORD_SWAP( "715a05.14t",   0x000002, 0x200000, CRC(462d53bf) SHA1(0216a84358571de6791365c69a1fa8fe2784148d) )

	ROM_REGION32_BE(0x1000000, "user5", 0)	/* CG Board texture roms */
	ROM_LOAD32_WORD_SWAP( "715a14.32u",   0x000002, 0x400000, CRC(bbb36be3) SHA1(c828d0af0546db02e87afe68423b9447db7c7e51) )
	ROM_LOAD32_WORD_SWAP( "715a13.24u",   0x000000, 0x400000, CRC(dbff58a1) SHA1(f0c60bb2cbf268cfcbdd65606ebb18f1b4839c0e) )

	ROM_REGION(0x80000, "audiocpu", 0)		/* 68K Program */
	ROM_LOAD16_WORD_SWAP( "715a08.7s",    0x000000, 0x080000, CRC(3aa2f4a5) SHA1(bb43e5f5ef4ac51f228d4d825be66d3c720d51ea) )

	ROM_REGION(0x1000000, "rfsnd", 0)		/* PCM sample roms */
	ROM_LOAD( "715a09.16p",   0x000000, 0x400000, CRC(65845866) SHA1(d2a63d0deef1901e6fa21b55c5f96e1f781dceda) )
	ROM_LOAD( "715a10.14p",   0x400000, 0x400000, CRC(294fe71b) SHA1(ac5fff5627df1cee4f1e1867377f208b34334899) )

	ROM_REGION(0x20000, "gn680", 0)		/* 68K Program */
	ROM_LOAD16_WORD_SWAP( "715a17.20k",    0x000000, 0x020000, CRC(f0b7ba0c) SHA1(863b260824b0ae2f890ba84d1c9a8f436891b1ff) )

	ROM_REGION(0x2000, "m48t58",0)
	ROM_LOAD( "m48t58y-70pc1", 0x000000, 0x002000, CRC(62fecb78) SHA1(09509be8a947cf2d38e12a6ea755ec0de4aa9bd4) )
ROM_END

/*************************************************************************/

GAME(  1998, gradius4,  0,        hornet,           hornet, hornet,        ROT0, "Konami", "Gradius 4: Fukkatsu", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME(  1998, nbapbp,    0,        hornet,           hornet, hornet,        ROT0, "Konami", "NBA Play By Play", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAMEL( 1998, terabrst,  0,        terabrst,         hornet, hornet_2board, ROT0, "Konami", "Teraburst (1998/07/17 ver UEL)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 1998, terabrsta, terabrst, terabrst,         hornet, hornet_2board, ROT0, "Konami", "Teraburst (1998/02/25 ver AAA)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 2000, sscope,    0,        hornet_2board,    sscope, hornet_2board, ROT0, "Konami", "Silent Scope (ver JZD)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 2000, sscopea,   sscope,   hornet_2board,    sscope, hornet_2board, ROT0, "Konami", "Silent Scope (ver UAB)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 2000, sscopeb,   sscope,   hornet_2board,    sscope, hornet_2board, ROT0, "Konami", "Silent Scope (ver UAA)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE, layout_dualhsxs )
GAMEL( 2000, sscope2,   0,        hornet_2board_v2, sscope, hornet_2board, ROT0, "Konami", "Silent Scope 2", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE, layout_dualhsxs )
