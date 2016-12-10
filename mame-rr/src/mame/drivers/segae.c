/* Sega System E */

/*


 Sega System 'E' is a piece of hardware used for a couple of Arcade Games
 produced by Sega in the Mid 80's, its roughly based on their Sega Master System
 home console unit, using the same '315-5124' VDP (actually in this case 2 of
 them)

 An interesting feature of the system is that the CPU is contained on the ROM
 board, the MAIN System E board contains the Graphics processor, this opens the
 possibility for using processors other than the Standard Z80 to run the main
 game code on; several games have an encrypted Z80 module instead.

 Also interesting is each VDP has double the Video RAM found on the SMS console
 this is banked through Port Writes, the System also allows for the Video RAM
 to be written directly, bypassing the usual procedure of writing to it via the
 '315-5124' data port, it can not however be read directly, the same area used
 for writing is used to access banked ROMs when reading

 Pretty much everything on this hardware is done through port accesses, the
 main memory map consists of simply ROM, BANKED ROM / BANKED RAM, RAM

********************************************************************************

    ROMs + CPU Board (32kb ROMs)

    IC 07 (Good)    IC 05 (Good)
    IC 04 (Good)    IC 03 (Good)
    IC 02 (Good)

    (834-5803) MAIN Board (8kb RAMs)

    IC 49 (Good)    IC 55 (Good)    System RAM (0xc000 - 0xffff)
    IC 03 (Good)    IC 07 (Good)    Front Layer VRAM (Bank 1)   Port F7 -0------
    IC 04 (Good)    IC 08 (Good)    Front Layer VRAM (Bank 2)   Port F7 -1------
    IC 01 (Good)    IC 05 (Good)    Back Layer VRAM (Bank 1)    Port F7 0-------
    IC 02 (Good)    IC 06 (Good)    Back Layer VRAM (Bank 2)    Port F7 1-------
    (or at least this is how it appears from HangOnJr's RAMs Test)

    2x (315-5124)'s here too, these are the VDP chips

    PORTS (to be completed)

    0xba - 0xbb r/w     Back Layer VDP
    0xbe - 0xbf r/w     Front Layer VDP

    0xf7 w/o            Banking Controls

    0xe0 r/o            Inputs (Coins, Start Btns)
    0xe1 r/o            Controls (Transformer)

    0xf2 - 0xf3 r/o     Dipswitches

    0xf8 r/o            Analog Input (Hang On Jr)

    0x7e r/o            V Counter (vertical beam pos in scanlines)
    0x7f r/o            H Counter (horizontal beam pos in 'pixel clock cycles')

********************************************************************************

Sega System E Hardware Overview
Sega, 1985-1988

This PCB is essentially a Sega Master System home console unit, but using
two '315-5124' VDPs and extra RAM.
The CPU is located on a plug-in board that also holds all of the EPROMs.

The games that run on this hardware include....
Hang-On Jr.             1985
Transformer/Astro Flash 1986
Riddle of Pythagoras    1986
Opa Opa                 1987
Fantasy Zone 2          1988
Tetris                  1988

PCB Layout
----------
834-5803 (C)Sega 1985
|-----------------------------------------------------|
|         D4168      D4168      D4168       D4168     |
|                                                     |
|         D4168      D4168      D4168       D4168     |
|                                                     |
|                                                     |
|    SW1                                              |
|CN1                                                  |
|    SW2                                              |
|                                                     |
|   LED            |---|             |---|            |
|                  | 3 |             | 3 |            |
|                  | 1 |             | 1 |            |
|                  | 5 |             | 5 |            |
|                  | | |             | | |         CN3|
|                  | 5 |             | 5 |     8255   |
|CN4               | 1 |             | 1 |            |
|                  | 2 |             | 2 |            |
|                  | 4 |             | 4 |            |
|                  |---|             |---|            |
|               |--------ROM-BOARD-(above)---------|  |
|               |                                  |  |
|               |CN2                   10.7386MHz  |  |
|               |         D4168                    |  |
|  VOL          |         D4168                    |  |
| LA4460        |----------------------------------|  |
|-----------------------------------------------------|
Notes:
      315-5124 VDP clock - 10.7386MHz
      SN76496 clock      - 3.579533MHz [10.7386/3]
      D4168              - 8k x8 SRAM
      VSync              - 60Hz
      HSync              - 15.58kHz
      CN1                - Connector used for standard controls
      CN2                - connector for CPU/ROM PCB
      CN3                - Connector used for special controls (via a small plug-in interface PCB)
      CN4                - Connector for power

ROM Daughterboard
-----------------
834-6592-01
|--------------------------------------------|
|                                            |
|    |---|                                   |
|C   |   |                           IC6     |
|N   |Z80|                                   |
|2   |   |                                   |
|    |   |   IC2   IC3   IC4   IC5        IC7|
|    |---|                                   |
|     IC1             PAD1 PAD2     PAD3 PAD4|
|--------------------------------------------|
Notes:
       IC1: Z80 clock - 5.3693MHz [10.7386/2]
            On some games this is replaced with a NEC MC-8123 Encrypted CPU Module.
            The clock speed is the same. The MC-8123 contains a Z80 core, plus a RAM chip
            and battery. When the battery dies, the program can no longer be decrypted
            and the PCB does not boot up at all. The battery can not be changed because the
            MC-8123 is sealed, so there is no way to access it.

 IC2 - IC5: 27C256 EPROM (DIP28)

       IC6: 74LS139

       IC7: 27C256 EPROM (DIP28)

    PAD1-4: These are jumper pads used to configure the ROM board for use with the
            Z80 or with the MC8123 CPU.
            PAD1 - Ties Z80 pin 24 (WAIT) to pin1 of the EPROMs at IC2, 3, 4 & 5
            PAD2 - Ties CN2 pin B21 to pin1 of the EPROMs at IC2, 3, 4 & 5
            PAD3 - Ties CN2 pin B21 to pin 2 of the 74LS139 @ IC6
            PAD4 - Ties Z80 pin 24 (WAIT) to pin 2 of the 74LS139 @ IC6

            The pads are configured like this..... (U=Upper, L=Lower)

                                                 |----|      |----|
                                                 |IC6 |      |IC7 |
                                                 |  12|------|22  |
                                                 |    |      |    |
                       IC2   IC3    IC4   IC5    |   1|------|27  |
                       PIN1  PIN1   PIN1  PIN1   |   2|--|   |    |
                        O-----O--+---O------O    |----|  |   |----|
                                 |                       |
                                 |         |----|        |
                              O--+----O    |    O    |---O
            CN2    Z80      PAD1U   PAD2U  |  PAD3U  | PAD4U
            B21    PIN24    PAD1L   PAD2L  |  PAD3L  | PAD4L
             O       O--4.7k--O       O----|    O----|   O
             |                |       |                  |
             |                |-------|------------------|
             |                        |
             |------------------------|

            When using a regular Z80B (and thus, unencrypted code):
            PAD1 - Open
            PAD2 - Open
            PAD3 - Shorted
            PAD4 - Open

            When using an encrypted CPU module (MC-8123):
            PAD1 - Open
            PAD2 - Shorted
            PAD3 - Open
            PAD4 - Open
            Additionally, a wire must be tied from CN2 pin B22 to the side
            of PAD3 nearest IC6 (i.e. PAD3U).

ROMs:
-----

Game                     IC2         IC3         IC4         IC5         IC7
---------------------------------------------------------------------------------
Hang-On Jr.              EPR-?       EPR-?       EPR-?       EPR-?       EPR-?     Hello, Sega Part Numbers....!?
Transformer              EPR-7350    EPR-?       EPR-7348    EPR-7347    EPR-?     Ditto
           /Astro Flash  EPR-7350    EPR-7349    EPR-7348    EPR-7347    EPR-7723
Riddle of Pythagoras     EPR-10422   EPR-10423   EPR-10424   EPR-10425   EPR-10426
Opa Opa                  EPR-11220   EPR-11221   EPR-11222   EPR-11223   EPR-11224
Fantasy Zone 2           EPR-11412   EPR-11413   EPR-11414   EPR-11415   EPR-11416
Tetris                   -           -           EPR-12211   EPR-12212   EPR-12213

A System E PCB can run all of the games simply by swapping the EPROMs plus CPU.
Well, in theory anyway. To run the not-encrypted games, just swap EPROMs and they will work.

To run the encrypted games, use a double sized EPROM in IC7 (i.e. a 27C512)
and program the decrypted opcodes to the lower half and the decrypted data to the upper half,
then connect the highest address pin of the EPROM (A15 pin 1) to the M1 pin on the Z80.
This method has been tested and does not actually work. An update on this may follow....


System E PCB Pinout
-------------------

CN1
---

+12V             1A  1B  Coin switch 1
Coin switch 2    2A  2B  Test switch
Service switch   3A  3B
                 4A  4B  1P start
2P start         5A  5B  1P up
1P down          6A  6B  1P left
1P right         7A  7B  1P button 1
1P button 2      8A  8B
                 9A  9B  2P up
2P down          10A 10B 2P left
2P RIGHT         11A 11B 2P button 1
2P button 2      12A 12B
                 13A 13B Video RED
                 14A 14B Video GREEN
                 15A 15B Video BLUE
                 16A 16B Video SYNC
                 17A 17B
                 18A 18B
Speaker [+]      19A 19B
Speaker [-]      20A 20B
Coin counter GND 21A 21B
GND              22A 22B Coin counter 1
                 23A 23B Coin counter 2
                 24A 24B
                 25A 25B
CN4
---

+5V  1A 1B +5V
+5V  2A 2B +5V
     3A 3B
GND  4A 4B GND
GND  5A 5B GND
+12V 6A 6B +12V
+12V 7A 7B +12V
GND  8A 8B GND


 Game Notes:
 Riddle of Pythagoras is interesting, it looks like Sega might have planned it
 as a two player game, there is prelimiary code for 2 player support which
 never gets executed, see code around 0x0E95.  Theres also quite a bit of
 pointless code here and there.  Some Interesting Memory Locations

 C000 : level - value (00-0x32)
 C001 : level - display (00-0x50, BCD coded)
 C003 : credits (00-0x99, BCD coded)
 C005 : DSWA put here (coinage, left and right nibbles for left and right slot
        - freeplay when 0x0f or 0xf0)
 C006 : DSWB put here
  bits 0 and 1 : lives ("02", "03", "04", "98")
  bit 3 : difficulty
  bits 5 and 6 : bonus lives ("50K, 100K, 200K, 500K", "100K, 200K, 500K", "100K,
                               200K, 500K, 99999999", "none")
 C009 : lives (for player 1)
 C00A : lives (for player 2)
 C00B : bonus lives counter

 E20B-E20E : score (00000000-0x99999999, BCD coded)
 E215-E218 : hi-score (00000000-0x99999999, BCD coded)

 E543 : bit 0 : ON = player 1 one still has lives
        bit 1 : ON = player 2 one still has lives
        bit 2 : ON = player 1 is the current player - OFF = player 2 is the
         current player

 E572 : table with L. slot infos (5 bytes wide)
 E577 : table with R. slot infos (5 bytes wide)

Known issues:

sometimes hangonjr has corrupt gfx when you start a game, I don't know why (timing?)

todo:

tidy up, add save states, clean up so we can use it with hazemd again
add emulation of vdp bugs, use mame's new screen timing system instead
covert megatech / megaplay drivers to use new code etc. etc.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "machine/mc8123.h"
#include "machine/segacrp2.h"
#include "includes/segamsys.h"

/****************************************************************************************
 Memory Maps

 most of the memory map / IO maps are filled in at run time - this is due to the SMS
 code that this is based on being designed that way due to weird features of the MD.

****************************************************************************************/

static UINT8 f7_bank_value;

/* we have to fill in the ROM addresses for systeme due to the encrypted games */
static ADDRESS_MAP_START( systeme_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM								/* Fixed ROM */
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")						/* Banked ROM */
ADDRESS_MAP_END


static WRITE8_HANDLER( segasyse_videoram_w )
{
	if (f7_bank_value & 0x20)
	{ // to vdp1 vram
		if (f7_bank_value & 0x80)
		{
			vdp1_vram_bank0[offset] = data;
		}
		else
		{
			vdp1_vram_bank1[offset] = data;
		}
	}
	else
	{ // to vdp2 vram
		if (f7_bank_value & 0x40)
		{
			vdp2_vram_bank0[offset] = data;
		}
		else
		{
			vdp2_vram_bank1[offset] = data;
		}
	}

}

static WRITE8_HANDLER( systeme_bank_w )
{
	int rombank;
	f7_bank_value = data;

	rombank = data & 0x0f;

	segae_set_vram_banks(data);

	//memcpy(sms_rom+0x8000, memory_region(space->machine, "user1")+0x10000+rombank*0x4000, 0x4000);
	memory_set_bank(space->machine, "bank1", rombank);

}


static void init_ports_systeme(running_machine *machine)
{
	/* INIT THE PORTS *********************************************************************************************/

	const address_space *io = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO);
	running_device *sn1 = machine->device("sn1");
	running_device *sn2 = machine->device("sn2");

	memory_install_write8_device_handler(io, sn2, 0x7b, 0x7b, 0, 0, sn76496_w);
	memory_install_write8_device_handler(io, sn1, 0x7e, 0x7f, 0, 0, sn76496_w);
	memory_install_read8_handler        (io, 0x7e, 0x7e, 0, 0, sms_vcounter_r);

	memory_install_readwrite8_handler(io, 0xba, 0xba, 0, 0, sms_vdp_data_r, sms_vdp_data_w);
	memory_install_readwrite8_handler(io, 0xbb, 0xbb, 0, 0, sms_vdp_ctrl_r, sms_vdp_ctrl_w);

	memory_install_readwrite8_handler(io, 0xbe, 0xbe, 0, 0, sms_vdp_2_data_r, sms_vdp_2_data_w);
	memory_install_readwrite8_handler(io, 0xbf, 0xbf, 0, 0, sms_vdp_2_ctrl_r, sms_vdp_2_ctrl_w);

	memory_install_read_port     (io, 0xe0, 0xe0, 0, 0, "e0");
	memory_install_read_port     (io, 0xe1, 0xe1, 0, 0, "e1");
	memory_install_read_port     (io, 0xe2, 0xe2, 0, 0, "e2");
	memory_install_read_port     (io, 0xf2, 0xf2, 0, 0, "f2");
	memory_install_read_port     (io, 0xf3, 0xf3, 0, 0, "f3");

	memory_install_write8_handler    (io, 0xf7, 0xf7, 0, 0, systeme_bank_w );
}



static void init_systeme_map(running_machine *machine)
{
	memory_configure_bank(machine, "bank1", 0, 16, memory_region(machine, "maincpu") + 0x10000, 0x4000);

	/* alternate way of accessing video ram */
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x8000, 0xbfff, 0, 0, segasyse_videoram_w);

	/* main ram area */
	sms_mainram = (UINT8 *)memory_install_ram(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc000, 0xffff, 0, 0, NULL);
	memset(sms_mainram,0x00,0x4000);

	init_ports_systeme(machine);
}

static DRIVER_INIT( segasyse )
{
	init_systeme_map(machine);
	DRIVER_INIT_CALL(hazemd_segasyse);
}


/*******************************************************************************
 Input Ports
********************************************************************************
 mostly unknown for the time being
*******************************************************************************/

	/* The Coinage is similar to Sega System 1 and C2, but
    it seems that Free Play is not used in all games
    (in fact, the only playable game that use it is
    Riddle of Pythagoras) */

#define SEGA_COIN_A \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4") \
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" ) \
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit, 4/3" ) \
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit, 5/6" ) \
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit, 4/5" ) \
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit, 2/3" ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )

#define SEGA_COIN_B \
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8") \
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" ) \
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit, 4/3" ) \
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit, 5/6" ) \
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit, 4/5" ) \
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit, 2/3" ) \
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )


static INPUT_PORTS_START( transfrm ) /* Used By Transformer */
	PORT_START("f2")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	SEGA_COIN_B

	PORT_START("f3")	/* Read from Port 0xf3 */
	PORT_DIPNAME( 0x01, 0x00, "1 Player Only" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "10k, 30k, 50k and 70k" )
	PORT_DIPSETTING(    0x30, "20k, 60k, 100k and 140k"  )
	PORT_DIPSETTING(    0x10, "30k, 80k, 130k and 180k" )
	PORT_DIPSETTING(    0x00, "50k, 150k and 250k" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("e0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START("e1")	/* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("e2")	/* Read from Port 0xe2 */
INPUT_PORTS_END

static INPUT_PORTS_START( fantzn2 ) /* Used By Fantasy Zone 2 */
	PORT_START("f2")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	SEGA_COIN_B

	PORT_START("f3")	/* Read from Port 0xf3 */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Timer" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "90" )	/* 210 seconds */
	PORT_DIPSETTING(    0x30, "80" )	/* 180 seconds */
	PORT_DIPSETTING(    0x10, "70" )	/* 150 seconds */
	PORT_DIPSETTING(    0x00, "60" )	/* 120 seconds */
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("e0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START("e1")	/* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("e2")	/* Read from Port 0xe2 */
INPUT_PORTS_END

static INPUT_PORTS_START( opaopa ) /* Used By Opa Opa */
	PORT_START("f2")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	SEGA_COIN_B

	PORT_START("f3")	/* Read from Port 0xf3 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "0x30 Unknown" )
	PORT_DIPSETTING(    0x20, "0x20 Unknown" )
	PORT_DIPSETTING(    0x10, "0x10 Unknown" )
	PORT_DIPSETTING(    0x00, "0x00 Unknown" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("e0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START("e1")	/* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("e2")	/* Read from Port 0xe2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( tetrisse ) /* Used By Tetris */
	PORT_START("f2")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	SEGA_COIN_B

	PORT_START("f3")	/* Read from Port 0xf3 */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("e0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START("e1")	/* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("e2")	/* Read from Port 0xe2 */
INPUT_PORTS_END


static INPUT_PORTS_START( hangonjr ) /* Used By Hang On Jr */
	PORT_START("f2")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	SEGA_COIN_B

	PORT_START("f3")	/* Read from Port 0xf3 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1") /* Supose to be demo sound but has no effect */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Enemies" ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )	/*  These three dips seems to be unused */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("e0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("e1")	/* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("e2")	/* Read from Port 0xe2 */

	PORT_START("IN2")	/* Read from Port 0xf8 */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START("IN3")  /* Read from Port 0xf8 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)
INPUT_PORTS_END

static INPUT_PORTS_START( ridleofp ) /* Used By Riddle Of Pythagoras */
	PORT_START("f2")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	SEGA_COIN_B
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START("f3")	/* Read from Port 0xf3 */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "100 (Cheat)")
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPNAME( 0x08, 0x08, "Ball Speed" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )  // difficult (on datasheet)
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:6,7") /* Values came from an original dipsheet */
	PORT_DIPSETTING(    0x60, "50K 100K 200K 1M 2M 10M 20M 50M" )
	PORT_DIPSETTING(    0x40, "100K 200K 1M 2M 10M 20M 50M" )
	PORT_DIPSETTING(    0x20, "200K 1M 2M 10M 20M 50M" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("e0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // Would Be IPT_START2 but the code doesn't use it

	PORT_START("e1")	/* Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("e2")	/* Read from Port 0xe2 */

	PORT_START("IN2")	/* Read from Port 0xf8 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(125)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON2 )	/* is this used in the game? */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN3")	/* Read from Port 0xf8 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(125) PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END



ROM_START( hangonjr )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "rom5.ic7",	0x00000, 0x08000, CRC(d63925a7) SHA1(699f222d9712fa42651c753fe75d7b60e016d3ad) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "rom4.ic5",	0x10000, 0x08000, CRC(ee3caab3) SHA1(f583cf92c579d1ca235e8b300e256ba58a04dc90) )
	ROM_LOAD( "rom3.ic4",	0x18000, 0x08000, CRC(d2ba9bc9) SHA1(85cf2a801883bf69f78134fc4d5075134f47dc03) )
	ROM_LOAD( "rom2.ic3",	0x20000, 0x08000, CRC(e14da070) SHA1(f8781f65be5246a23c1f492905409775bbf82ea8) )
	ROM_LOAD( "rom1.ic2",	0x28000, 0x08000, CRC(3810cbf5) SHA1(c8d5032522c0c903ab3d138f62406a66e14a5c69) )
ROM_END

ROM_START( ridleofp )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "epr10426.bin",	0x00000, 0x08000, CRC(4404c7e7) SHA1(555f44786976a009d96a6395c9173929ad6138a7) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr10425.bin",	0x10000, 0x08000, CRC(35964109) SHA1(a7bc64a87b23139b0edb9c3512f47dcf73feb854) )
	ROM_LOAD( "epr10424.bin",	0x18000, 0x08000, CRC(fcda1dfa) SHA1(b8497b04de28fc0d6b7cb0206ad50948cff07840) )
	ROM_LOAD( "epr10423.bin",	0x20000, 0x08000, CRC(0b87244f) SHA1(c88041614735a9b6cba1edde0a11ed413e115361) )
	ROM_LOAD( "epr10422.bin",	0x28000, 0x08000, CRC(14781e56) SHA1(f15d9d89e1ebff36c3867cfc8f0bdf7f6b3c96bc) )
ROM_END

ROM_START( transfrm )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "ic7.top",	0x00000, 0x08000, CRC(ccf1d123) SHA1(5ade9b00e2a36d034fafdf1902d47a9a00e96fc4) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr-7347.ic5",	0x10000, 0x08000, CRC(df0f639f) SHA1(a09a9841b66de246a585be63d911b9a42a323503) )
	ROM_LOAD( "epr-7348.ic4",	0x18000, 0x08000, CRC(0f38ea96) SHA1(d4d421c5d93832e2bc1f22f39dffb6b80f2750bd) )
	ROM_LOAD( "ic3.top",		0x20000, 0x08000, CRC(9d485df6) SHA1(b25f04803c8f7188021f3039aa13aac80d480823) )
	ROM_LOAD( "epr-7350.ic2",	0x28000, 0x08000, CRC(0052165d) SHA1(cf4b5dffa54238e513515b3fc90faa7ce0b65d34) )
ROM_END

ROM_START( astrofl )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "epr-7723.ic7",	0x00000, 0x08000, CRC(66061137) SHA1(cb6a2c7864f9f87bbedfd4b1448ad6c2de65d6ca) ) /* encrypted */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr-7347.ic5",	0x10000, 0x08000, CRC(df0f639f) SHA1(a09a9841b66de246a585be63d911b9a42a323503) )
	ROM_LOAD( "epr-7348.ic4",	0x18000, 0x08000, CRC(0f38ea96) SHA1(d4d421c5d93832e2bc1f22f39dffb6b80f2750bd) )
	ROM_LOAD( "epr-7349.ic3",	0x20000, 0x08000, CRC(f8c352d5) SHA1(e59565ab6928c67706c6f82f6ea9a64cdfc65a21) )
	ROM_LOAD( "epr-7350.ic2",	0x28000, 0x08000, CRC(0052165d) SHA1(cf4b5dffa54238e513515b3fc90faa7ce0b65d34) )
ROM_END


ROM_START( tetrisse )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "epr12213.7",	0x00000, 0x08000, CRC(ef3c7a38) SHA1(cbb91aef330ab1a37d3e21ecf1d008143d0dd7ec) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr12212.5",	0x10000, 0x08000, CRC(28b550bf) SHA1(445922a62e8a7360335c754ad70dabbe0208207b) )
	ROM_LOAD( "epr12211.4",	0x18000, 0x08000, CRC(5aa114e9) SHA1(f9fc7fe4d0444a264185e74d2abc8475f0976534) )
	/* ic3 unpopulated */
	/* ic2 unpopulated */
ROM_END


ROM_START( fantzn2 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "epr-11416.ic7",	0x00000, 0x08000, CRC(76db7b7b) SHA1(d60e2961fc893dcb4445aed5f67515cbd25b610f) )	/* encrypted */

	ROM_LOAD( "epr-11415.ic5",	0x10000, 0x10000, CRC(57b45681) SHA1(1ae6d0d58352e246a4ec4e1ce02b0417257d5d20) )
	ROM_LOAD( "epr-11413.ic3",	0x20000, 0x10000, CRC(a231dc85) SHA1(45b94fdbde28c02e88546178ef3e8f9f3a96ab86) )
	ROM_LOAD( "epr-11414.ic4",	0x30000, 0x10000, CRC(6f7a9f5f) SHA1(b53aa2eded781c80466a79b7d81383b9a875d0be) )
	ROM_LOAD( "epr-11412.ic2",	0x40000, 0x10000, CRC(b14db5af) SHA1(04c7fb659385438b3d8f9fb66800eb7b6373bda9) )

	ROM_REGION( 0x2000, "user1", 0 ) /* MC8123 key */
	ROM_LOAD( "317-0057.key",  0x0000, 0x2000, CRC(ee43d0f0) SHA1(72cb75a4d8352fe372db12046a59ea044360d5c3) )
ROM_END

ROM_START( opaopa )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "epr11224.ic7",	0x00000, 0x08000, CRC(024b1244) SHA1(59a522ac3d98982cc4ddb1c81f9584d3da453649) ) /* encrypted */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr11223.ic5",	0x10000, 0x08000, CRC(6bc41d6e) SHA1(8997a4ac2a9704f1400d0ec16b259ee496a7efef) ) /* encrypted */
	ROM_LOAD( "epr11222.ic4",	0x18000, 0x08000, CRC(395c1d0a) SHA1(1594bad13e78c5fad4db644cd85a6bac1eaddbad) ) /* encrypted */
	ROM_LOAD( "epr11221.ic3",	0x20000, 0x08000, CRC(4ca132a2) SHA1(cb4e4c01b6ab070eef37c0603190caafe6236ccd) ) /* encrypted */
	ROM_LOAD( "epr11220.ic2",	0x28000, 0x08000, CRC(a165e2ef) SHA1(498ff4c5d3a2658567393378c56be6ed86ac0384) ) /* encrypted */

	ROM_REGION( 0x2000, "user1", 0 ) /* MC8123 key */
	ROM_LOAD( "317-0042.key",  0x0000, 0x2000, CRC(d6312538) SHA1(494ac7f080775c21dc7d369e6ea78f3299e6975a) )
ROM_END

static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static MACHINE_DRIVER_START( systeme )
	MDRV_CPU_ADD("maincpu", Z80, 10738600/2) /* correct?  */
	MDRV_CPU_PROGRAM_MAP(systeme_map)
	MDRV_CPU_IO_MAP(io_map)

	/* IRQ handled via the timers */
	MDRV_MACHINE_RESET(systeme)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)) // Vblank handled manually.
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB15)
	MDRV_SCREEN_SIZE(256, 256)
//  MDRV_SCREEN_VISIBLE_AREA(0, 255, 0, 223)
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 0, 191)


	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(sms)
	MDRV_VIDEO_UPDATE(systeme) /* Copies a bitmap */
	MDRV_VIDEO_EOF(systeme) /* Used to Sync the timing */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn1", SN76496, 3579540)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("sn2", SN76496, 3579540)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


/*- Hang On Jr. Specific -*/
static UINT8 port_fa_last;		/* Last thing written to port 0xfa (control related) */

static READ8_HANDLER (segae_hangonjr_port_f8_r)
{
	UINT8 temp;

	temp = 0;

	if (port_fa_last == 0x08)  /* 0000 1000 */ /* Angle */
		temp = input_port_read(space->machine, "IN2");

	if (port_fa_last == 0x09)  /* 0000 1001 */ /* Accel */
		temp = input_port_read(space->machine, "IN3");

	return temp;
}

static WRITE8_HANDLER (segae_hangonjr_port_fa_w)
{
	/* Seems to write the same pattern again and again bits ---- xx-x used */
	port_fa_last = data;
}

/*- Riddle of Pythagoras Specific -*/

static int port_to_read,last1,last2,diff1,diff2;

static READ8_HANDLER (segae_ridleofp_port_f8_r)
{
	switch (port_to_read)
	{
		default:
		case 0:	return diff1 & 0xff;
		case 1:	return diff1 >> 8;
		case 2:	return diff2 & 0xff;
		case 3:	return diff2 >> 8;
	}
}

static WRITE8_HANDLER (segae_ridleofp_port_fa_w)
{
	/* 0x10 is written before reading the dial (hold counters?) */
	/* 0x03 is written after reading the dial (reset counters?) */

	port_to_read = (data & 0x0c) >> 2;

	if (data & 1)
	{
		int curr = input_port_read(space->machine, "IN2");
		diff1 = ((curr - last1) & 0x0fff) | (curr & 0xf000);
		last1 = curr;
	}
	if (data & 2)
	{
		int curr = input_port_read(space->machine, "IN3") & 0x0fff;
		diff2 = ((curr - last2) & 0x0fff) | (curr & 0xf000);
		last2 = curr;
	}
}

static DRIVER_INIT( ridleofp )
{
	DRIVER_INIT_CALL(segasyse);

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0xf8, 0xf8, 0, 0, segae_ridleofp_port_f8_r);
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0xfa, 0xfa, 0, 0, segae_ridleofp_port_fa_w);
}


static DRIVER_INIT( hangonjr )
{
	DRIVER_INIT_CALL(segasyse);

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0xf8, 0xf8, 0, 0, segae_hangonjr_port_f8_r);
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0xfa, 0xfa, 0, 0, segae_hangonjr_port_fa_w);
}

static DRIVER_INIT( opaopa )
{
	DRIVER_INIT_CALL(segasyse);

	mc8123_decrypt_rom(machine, "maincpu", "user1", "bank1", 8);
}

static DRIVER_INIT( fantzn2 )
{
	DRIVER_INIT_CALL(segasyse);

	mc8123_decrypt_rom(machine, "maincpu", "user1", NULL, 0);
}

static DRIVER_INIT( astrofl )
{
	DRIVER_INIT_CALL(segasyse);

	sega_315_5177_decode(machine, "maincpu");
}

GAME( 1985, hangonjr, 0,        systeme, hangonjr, hangonjr, ROT0,  "Sega", "Hang-On Jr.", 0 )
GAME( 1986, transfrm, 0,        systeme, transfrm, segasyse, ROT0,  "Sega", "Transformer", 0 )
GAME( 1986, astrofl,  transfrm, systeme, transfrm, astrofl,  ROT0,  "Sega", "Astro Flash (Japan)", 0 )
GAME( 1986, ridleofp, 0,        systeme, ridleofp, ridleofp, ROT90, "Sega / Nasco", "Riddle of Pythagoras (Japan)", 0 )
GAME( 1987, opaopa,   0,        systeme, opaopa,   opaopa,   ROT0,  "Sega", "Opa Opa (MC-8123, 317-0042)", 0 )
GAME( 1988, fantzn2,  0,        systeme, fantzn2,  fantzn2,  ROT0,  "Sega", "Fantasy Zone 2 (MC-8123, 317-0057)", 0 )
GAME( 1988, tetrisse, 0,        systeme, tetrisse, segasyse, ROT0,  "Sega", "Tetris (Japan, System E)", 0 )

