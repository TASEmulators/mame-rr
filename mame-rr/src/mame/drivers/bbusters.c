/***************************************************************************

    Beast Busters           A9003   (c) 1989 SNK Corporation
    Mechanized Attack       A8002   (c) 1989 SNK Corporation

    Beast Busters is a large dedicated (non-jamma) triple machine gun game,
    the gun positions values are read in an interrupt routine that must be
    called for each position (X and Y for 3 guns, so at least 6 times a
    frame).  However I can't make it work reliably..  So for the moment
    I'm writing the gun positions directly to memory and bypassing
    the IRQ routine.

    Mechanized Attack (A8002) is an earlier design, it only has one sprite
    chip, no eeprom, and only 2 machine guns, but the tilemaps are twice
    the size.

    Emulation by Bryan McPhail, mish@tendril.co.uk


Stephh's notes (based on the games M68000 code and some tests) :

1) 'bbusters'

  - Country/version is stored at 0x003954.w and the possible values are
    0x0000, 0x0004 and 0x0008 (0x000c being the same as 0x0008), 0x0008
    being the value stored in ROM in the current set.
    I haven't noticed any major effect (copyright/logo, language, coinage),
    the only thing I found is that the text relative to number of coins
    needed is different (but it's a lie as coinage is NOT modified).
    So my guess is that if other versions exist, part of the code (or at
    least data in it) will have to be different.
    Anyway, here is my guess to determine the versions (by using some infos
    from 'mechatt' :

       Value   Country
      0x0000    Japan
      0x0004    US?
      0x0008    World?   (value stored in the current set)
      0x000c    World?   (same as 0x0008 - impossible choice ?)

  - Coin buttons act differently depending on the "Coin Slots" Dip Switch :

      * "Coin Slots" Dip Switch set to "Common" :

          . COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
          . COIN3    : NO EFFECT !
          . COIN4    : NO EFFECT !
          . COIN5    : NO EFFECT !
          . COIN6    : NO EFFECT !
          . SERVICE1 : adds coin(s)/credit(s) depending on "Coin A" Dip Switch

      * "Coin Slots" Dip Switch set to "Individual" :

          . COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) for player 1 depending on "Coin B" Dip Switch
          . COIN3    : adds coin(s)/credit(s) for player 2 depending on "Coin A" Dip Switch
          . COIN4    : adds coin(s)/credit(s) for player 2 depending on "Coin B" Dip Switch
          . COIN5    : adds coin(s)/credit(s) for player 3 depending on "Coin A" Dip Switch
          . COIN6    : adds coin(s)/credit(s) for player 3 depending on "Coin B" Dip Switch
          . SERVICE1 : adds coin(s)/credit(s) for all players depending on "Coin A" Dip Switch

    Note that I had to map COIN5 and COIN6 to SERVICE2 and SERVICE3 to be
    able to use the default parametrable keys. Let me know if there is a
    another (better ?) way to do so.


2) 'mechatt'

  - Country/version is stored at 0x06a000.w and the possible values are :

       Value   Country
      0x0000    Japan
      0x1111    World    (value stored in the current set)
      0x2222    US
      0x3333    Asia?    (it looks like Japanese text but some "symbols" are missing)

2a) Japan version

  - All texts are in Japanese.
  - "(c) 1989 (Corp) S-N-K".
  - "Coin Slots" Dip Switch has no effect.
  - "Coin A" and "Coin B" Dip Switches are the same as in the World version.
  - Coin buttons effect :

      * "Coin Slots" are ALWAYS considered as "Common" :

          . COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
          . COIN3    : NO EFFECT !
          . COIN4    : NO EFFECT !
          . SERVICE1 : adds coin(s)/credit(s) depending on "Coin A" Dip Switch

2b) World version

  - All texts are in English.
  - "(c) 1989 SNK Corporation".
  - Coin buttons effect :

      * "Coin Slots" Dip Switch set to "Common" :

          . COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
          . COIN3    : NO EFFECT !
          . COIN4    : NO EFFECT !
          . SERVICE1 : adds coin(s)/credit(s) depending on "Coin A" Dip Switch

      * "Coin Slots" Dip Switch set to "Individual" :

          . COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) for player 1 depending on "Coin B" Dip Switch
          . COIN3    : adds coin(s)/credit(s) for player 2 depending on "Coin A" Dip Switch
          . COIN4    : adds coin(s)/credit(s) for player 2 depending on "Coin B" Dip Switch
          . SERVICE1 : adds coin(s)/credit(s) for all players depending on "Coin A" Dip Switch

2c) US version

  - All texts are in English.
  - "(c) 1989 SNK Corp. of America".
  - Additional FBI logo as first screen as well as small FBI notice at the bottom left
    of the screens until a coin is inserted.
  - "Coin Slots" Dip Switch has no effect.
  - "Coin A" Dip Switch is different from the World version :

      World      US
      4C_1C    "Free Play"
      3C_1C    special (see below)
      2C_1C    "2 Coins to Start, 1 to Continue"
      1C_1C    1C_1C

    It's a bit hard to explain the "special" coinage, so here are some infos :

      * when you insert a coin before starting a game, you are awarded 2 credits
        if credits == 0, else you are awarded 1 credit
      * when you insert a coin to continue, you are ALWAYS awarded 1 credit

  - "Coin B" Dip Switch has no effect.

  - Coin buttons effect :

      * "Coin Slots" are ALWAYS considered as "Individual" :

          . COIN1    : adds coin(s)/credit(s) for player 1 depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) for player 2 depending on "Coin A" Dip Switch
          . COIN3    : NO EFFECT !
          . COIN4    : NO EFFECT !
          . SERVICE1 : adds coin(s)/credit(s) for all players depending on "Coin A" Dip Switch

2d) Asia? version

  - All texts are in Japanese ? (to be confirmed)
  - "(c) 1989 SNK Corporation".
  - "Coin Slots" Dip Switch has no effect.
  - "Coin A" and "Coin B" Dip Switches are the same as in the World version.
  - Coin buttons effect :

      * "Coin Slots" are ALWAYS considered as "Common" :

          . COIN1    : adds coin(s)/credit(s) depending on "Coin A" Dip Switch
          . COIN2    : adds coin(s)/credit(s) depending on "Coin B" Dip Switch
          . COIN3    : NO EFFECT !
          . COIN4    : NO EFFECT !
          . SERVICE1 : adds coin(s)/credit(s) depending on "Coin A" Dip Switch



HIGHWAYMAN's notes:

after adding the mechanized attack u.s. roms i suspect that there is more than just a few bytes changed ;-)


RansAckeR's notes:

bbusters:

If you only calibrate the P1 gun or do not hit the correct spots for all guns
you will get either garbage or a black screen when rebooting.
According to the manual this happens when the eprom contains invalid gun aim data.

If you calibrate the guns correctly the game runs as expected:
1) Using P1 controls fire at the indicated spots.
2) Using P2 controls fire at the indicated spots.
3) Using P3 controls fire at the indicated spots.

---

Beast Busters notes (from Brian Hargrove)

1. Stage 2 for example, has background sprites and enemies that float on the
foreground and not behind the moving elevator layer.

2. Oddly enough, the emulation completely misses the huge zombie that jumps
out during the attract demo right before the text story text comes in.
When you hear the the high pitch "zing" sound, there should be a zombie nearly
the entire size of the screen.


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2608intf.h"
#include "sound/2610intf.h"


VIDEO_START( bbuster );
VIDEO_START( mechatt );
VIDEO_UPDATE( bbuster );
VIDEO_UPDATE( mechatt );

static UINT16 *bbusters_ram, *eprom_data;
extern UINT16 *bbusters_pf1_data,*bbusters_pf2_data,*bbusters_pf1_scroll_data,*bbusters_pf2_scroll_data;

WRITE16_HANDLER( bbusters_pf1_w );
WRITE16_HANDLER( bbusters_pf2_w );
WRITE16_HANDLER( bbusters_video_w );

/******************************************************************************/


/* Beast Busters Region code works as follows

ROM[0x003954/2] = data * 4;

Country/Version :

 - 0x0000 : Japan?
 - 0x0004 : US?
 - 0x0008 : World?    (default)
 - 0x000c : World?    (same as 0x0008)

*/

/* Mech Attack Region code works as follows

ROM[0x06a000/2] = (data << 12) | (data << 8) | (data << 4) | (data << 0);

Country :
- 0x0000 : Japan
- 0x1111 : World (default)
- 0x2222 : US
- 0x3333 : Asia?

*/


/******************************************************************************/

static int sound_status;

static READ16_HANDLER( sound_status_r )
{
	return sound_status;
}

static WRITE8_HANDLER( sound_status_w )
{
	sound_status = data;

}

static WRITE16_HANDLER( sound_cpu_w )
{

	if (ACCESSING_BITS_0_7)
	{


		soundlatch_w(space, 0, data&0xff);
		cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
	}

}

/* Eprom is byte wide, top half of word _must_ be 0xff */
static READ16_HANDLER( eprom_r )
{
	return (eprom_data[offset]&0xff) | 0xff00;
}

static int gun_select;

static READ16_HANDLER( control_3_r )
{
	static const char *const port[] = { "GUNX1", "GUNY1", "GUNX2", "GUNY2", "GUNX3", "GUNY3" };

	UINT16 retdata =  input_port_read(space->machine, port[gun_select]);

	retdata >>=1; // by lowering the precision of the gun reading hardware the game seems to work better

	return retdata;
}

static WRITE16_HANDLER( gun_select_w )
{
	logerror("%08x: gun r\n",cpu_get_pc(space->cpu));

	cpu_set_input_line(space->cpu, 2, HOLD_LINE);

	gun_select = data & 0xff;

}

static WRITE16_HANDLER( two_gun_output_w )
{
	output_set_value("Player1_Gun_Recoil",(data & 0x01));
	output_set_value("Player2_Gun_Recoil",(data & 0x02)>>1);


}

static WRITE16_HANDLER( three_gun_output_w )
{
	output_set_value("Player1_Gun_Recoil",(data & 0x01));
	output_set_value("Player2_Gun_Recoil",(data & 0x02)>>1);
	output_set_value("Player3_Gun_Recoil",(data & 0x04)>>2);


}

static READ16_HANDLER( kludge_r )
{
	// might latch the gun value?
	return 0x0000;

}

static READ16_HANDLER( mechatt_gun_r )
{
	int x, y;

	x = input_port_read(space->machine, offset ? "GUNX2" : "GUNX1");
	y = input_port_read(space->machine, offset ? "GUNY2" : "GUNY1");

	/* Todo - does the hardware really clamp like this? */
	x += 0x18;
	if (x > 0xff) x = 0xff;
	if (y > 0xef) y = 0xef;

	return x | (y<<8);
}

/*******************************************************************************/

static ADDRESS_MAP_START( bbusters_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_RAM AM_BASE(&bbusters_ram)
	AM_RANGE(0x090000, 0x090fff) AM_RAM_WRITE(bbusters_video_w) AM_BASE_GENERIC(videoram)
	AM_RANGE(0x0a0000, 0x0a0fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x0a1000, 0x0a7fff) AM_RAM		/* service mode */
	AM_RANGE(0x0a8000, 0x0a8fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram2)
	AM_RANGE(0x0a9000, 0x0affff) AM_RAM		/* service mode */
	AM_RANGE(0x0b0000, 0x0b1fff) AM_RAM_WRITE(bbusters_pf1_w) AM_BASE(&bbusters_pf1_data)
	AM_RANGE(0x0b2000, 0x0b3fff) AM_RAM_WRITE(bbusters_pf2_w) AM_BASE(&bbusters_pf2_data)
	AM_RANGE(0x0b4000, 0x0b5fff) AM_RAM		/* service mode */
	AM_RANGE(0x0b8000, 0x0b8003) AM_WRITEONLY AM_BASE(&bbusters_pf1_scroll_data)
	AM_RANGE(0x0b8008, 0x0b800b) AM_WRITEONLY AM_BASE(&bbusters_pf2_scroll_data)
	AM_RANGE(0x0d0000, 0x0d0fff) AM_RAM_WRITE(paletteram16_RRRRGGGGBBBBxxxx_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x0e0000, 0x0e0001) AM_READ_PORT("COINS")	/* Coins */
	AM_RANGE(0x0e0002, 0x0e0003) AM_READ_PORT("IN0")	/* Player 1 & 2 */
	AM_RANGE(0x0e0004, 0x0e0005) AM_READ_PORT("IN1")	/* Player 3 */
	AM_RANGE(0x0e0008, 0x0e0009) AM_READ_PORT("DSW1")	/* Dip 1 */
	AM_RANGE(0x0e000a, 0x0e000b) AM_READ_PORT("DSW2")	/* Dip 2 */
	AM_RANGE(0x0e0018, 0x0e0019) AM_READ(sound_status_r)
	AM_RANGE(0x0e8000, 0x0e8001) AM_READWRITE(kludge_r, gun_select_w)
	AM_RANGE(0x0e8002, 0x0e8003) AM_READ(control_3_r)
	/* AM_RANGE(0x0f0008, 0x0f0009) AM_WRITENOP */
	AM_RANGE(0x0f0008, 0x0f0009) AM_WRITE(three_gun_output_w)
	AM_RANGE(0x0f0018, 0x0f0019) AM_WRITE(sound_cpu_w)
	AM_RANGE(0x0f8000, 0x0f80ff) AM_READ(eprom_r) AM_WRITEONLY AM_BASE(&eprom_data) /* Eeprom */
ADDRESS_MAP_END

/*******************************************************************************/

static ADDRESS_MAP_START( mechatt_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x06ffff) AM_ROM
	AM_RANGE(0x070000, 0x07ffff) AM_RAM AM_BASE(&bbusters_ram)
	AM_RANGE(0x090000, 0x090fff) AM_RAM_WRITE(bbusters_video_w) AM_BASE_GENERIC(videoram)
	AM_RANGE(0x0a0000, 0x0a0fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x0a1000, 0x0a7fff) AM_WRITENOP
	AM_RANGE(0x0b0000, 0x0b3fff) AM_RAM_WRITE(bbusters_pf1_w) AM_BASE(&bbusters_pf1_data)
	AM_RANGE(0x0b8000, 0x0b8003) AM_WRITEONLY AM_BASE(&bbusters_pf1_scroll_data)
	AM_RANGE(0x0c0000, 0x0c3fff) AM_RAM_WRITE(bbusters_pf2_w) AM_BASE(&bbusters_pf2_data)
	AM_RANGE(0x0c8000, 0x0c8003) AM_WRITEONLY AM_BASE(&bbusters_pf2_scroll_data)
	AM_RANGE(0x0d0000, 0x0d07ff) AM_RAM_WRITE(paletteram16_RRRRGGGGBBBBxxxx_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x0e0000, 0x0e0001) AM_READ_PORT("IN0")
	AM_RANGE(0x0e0002, 0x0e0003) AM_READ_PORT("DSW1")
	AM_RANGE(0x0e0004, 0x0e0007) AM_READ(mechatt_gun_r)
	/* AM_RANGE(0x0e4002, 0x0e4003) AM_WRITENOP  Gun force feedback? */
	AM_RANGE(0x0e4002, 0x0e4003) AM_WRITE(two_gun_output_w)
	AM_RANGE(0x0e8000, 0x0e8001) AM_READWRITE(sound_status_r, sound_cpu_w)
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf800) AM_READWRITE(soundlatch_r, sound_status_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ymsnd", ym2610_r, ym2610_w)
	AM_RANGE(0xc0, 0xc1) AM_WRITENOP /* -> Main CPU */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sounda_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ymsnd", ym2608_r, ym2608_w)
	AM_RANGE(0xc0, 0xc1) AM_WRITENOP /* -> Main CPU */
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( bbusters )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)	PORT_NAME("P1 Fire")	// "Fire"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)	PORT_NAME("P1 Grenade")	// "Grenade"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	PORT_NAME("P2 Fire")	// "Fire"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	PORT_NAME("P2 Grenade")	// "Grenade"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)	PORT_NAME("P3 Fire")	// "Fire"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)	PORT_NAME("P3 Grenade")	// "Grenade"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )		// See notes
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x06, 0x06, "Magazine / Grenade" )		PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x04, "5 / 2" )
	PORT_DIPSETTING(    0x06, "7 / 3" )
	PORT_DIPSETTING(    0x02, "9 / 4" )
	PORT_DIPSETTING(    0x00, "12 / 5" )
	/* Manual (from a different revision/region?) says:
                        SW1:4   SW1:5   SW1:6
    1C_1C 1 To continue OFF     OFF     OFF
    2C_1C 1 To continue ON      OFF     OFF
    1C_2C 1 To continue OFF     ON      OFF
    2C_1C 2 To continue ON      ON      OFF
    3C_1C 1 To continue OFF     OFF     ON
    3C_1C 2 To continue ON      OFF     ON
    4C_3C 1 To continue OFF     ON      ON
    Free Play Mode      OFF     OFF     OFF

    SW1:7 Unused
    SW1:8 Blood color: ON=green OFF=red */
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_A ) )			PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coin_B ) )			PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )				PORT_DIPLOCATION("SW1:8") // See notes
	PORT_DIPSETTING(    0x80, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Game Mode" )					PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "Demo Sounds Off" )
	PORT_DIPSETTING(    0x0c, "Demo Sounds On" )
	PORT_DIPSETTING(    0x04, "Infinite Energy (Cheat)")
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )			/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )			/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )			/* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("GUNX1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("GUNY1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNX2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("GUNY2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNX3")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(3)
	PORT_START("GUNY3")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(3)
INPUT_PORTS_END

static INPUT_PORTS_START( mechatt )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )		// See notes
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Fire")	// "Fire"
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Grenade")	// "Grenade"
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fire")	// "Fire"
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Grenade")	// "Grenade"
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Slots" )				PORT_DIPLOCATION("SW1:1") // Listed as "Unused" (manual from different revision/region?), See notes
	PORT_DIPSETTING(      0x0001, "Common" )
	PORT_DIPSETTING(      0x0000, "Individual" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Magazine / Grenade" )		PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, "5 / 2" )
	PORT_DIPSETTING(      0x000c, "6 / 3" )
	PORT_DIPSETTING(      0x0004, "7 / 4" )
	PORT_DIPSETTING(      0x0000, "8 / 5" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )			PORT_DIPLOCATION("SW1:5,6") // See notes
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )			PORT_DIPLOCATION("SW1:7,8") // Listed as "Unused" (manual from different revision/region?), See notes
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Game Mode" )					PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "Demo Sounds Off" )
	PORT_DIPSETTING(      0x0c00, "Demo Sounds On" )
	PORT_DIPSETTING(      0x0400, "Infinite Energy (Cheat)")
	PORT_DIPSETTING(      0x0000, "Freeze" )
	PORT_DIPUNUSED_DIPLOC(0x1000, 0x1000, "SW2:5" )			/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC(0x2000, 0x2000, "SW2:6" )			/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC(0x4000, 0x4000, "SW2:7" )			/* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("GUNX1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_START("GUNY1")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNX2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_START("GUNY2")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( mechattu )
	PORT_INCLUDE( mechatt )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW1:1" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coinage ) )			PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0010, "1 Coin/2 Credits first, then 1 Coin/1 Credit" )
	PORT_DIPSETTING(      0x0020, "2 Coins/1 Credit first, then 1 Coin/1 Credit" )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x00c0, 0x00c0, "SW1:7,8" )
INPUT_PORTS_END


/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0,4,8,12,16,20,24,28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{
		0, 1, 2, 3,
		16,17,18,19,

		0+32*8, 1+32*8, 2+32*8, 3+32*8,
		16+32*8,17+32*8,18+32*8,19+32*8,
	},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	128*8
};

static const gfx_layout tilelayout =
{
	16,16,	/* 16*16 sprites */
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28,
		0+64*8, 4+64*8, 8+64*8, 12+64*8,
		16+64*8,20+64*8,24+64*8,28+64*8,
	},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

static GFXDECODE_START( bbusters )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 512, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout,   768, 16 )
	GFXDECODE_ENTRY( "gfx5", 0, tilelayout,  1024+256, 16 )
GFXDECODE_END

static GFXDECODE_START( mechatt )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 256, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 512, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout,   512, 16 )
	GFXDECODE_ENTRY( "gfx5", 0, tilelayout,   768, 16 )
GFXDECODE_END

/******************************************************************************/

static void sound_irq( running_device *device, int irq )
{
	cputag_set_input_line(device->machine, "audiocpu", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2608_interface ym2608_config =
{
	{
		AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
	},
	sound_irq
};

static const ym2610_interface ym2610_config =
{
	sound_irq
};

/******************************************************************************/

// default eeprom with reasonable calibration for MAME
static const unsigned char bbusters_default_eeprom[128] = {
	                                    /*y*/                   /*y*/
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0xEE, 0x00,
	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0xFE, 0x00,
	                                    /*y*/                   /*y*/
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x74, 0x00,	0x00, 0x00, 0xEE, 0x00,
	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0xFE, 0x00,
	                                    /*y*/                   /*y*/
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x74, 0x00,	0x00, 0x00, 0xEE, 0x00,
	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0xFE, 0x00,


	0x42, 0x00, 0x45, 0x00, 0x41, 0x00, 0x53, 0x00,	0x54, 0x00, 0x20, 0x00,
	0x42, 0x00, 0x55, 0x00, 0x53, 0x00, 0x54, 0x00, 0x45, 0x00, 0x52, 0x00,
	0x53, 0x00, 0x20, 0x00, 0x54, 0x00, 0x4D, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};


static NVRAM_HANDLER( bbusters )
{
	if( read_or_write ) {
		mame_fwrite (file, eprom_data, 0x80);
	}
	else {
		if (file)
			mame_fread (file, eprom_data, 0x80);
		else
			memcpy(eprom_data, bbusters_default_eeprom, 0x80);
	}
}

static VIDEO_EOF( bbuster )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram16_w(space,0,0,0xffff);
	buffer_spriteram16_2_w(space,0,0,0xffff);
}

static VIDEO_EOF( mechatt )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	buffer_spriteram16_w(space,0,0,0xffff);
}

static MACHINE_DRIVER_START( bbusters )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(bbusters_map)
	MDRV_CPU_VBLANK_INT("screen", irq6_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,4000000) /* Accurate */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(sound_portmap)

	MDRV_NVRAM_HANDLER(bbusters)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(bbusters)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(bbuster)
	MDRV_VIDEO_UPDATE(bbuster)
	MDRV_VIDEO_EOF(bbuster)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2610, 8000000)
	MDRV_SOUND_CONFIG(ym2610_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  1.0)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "lspeaker",  1.0)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( mechatt )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(mechatt_map)
	MDRV_CPU_VBLANK_INT("screen", irq4_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,4000000) /* Accurate */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(sounda_portmap)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(mechatt)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(mechatt)
	MDRV_VIDEO_UPDATE(mechatt)
	MDRV_VIDEO_EOF(mechatt)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2608, 8000000)
	MDRV_SOUND_CONFIG(ym2608_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.50)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.50)
	MDRV_SOUND_ROUTE(1, "lspeaker",  1.0)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_DRIVER_END

/******************************************************************************/

ROM_START( bbusters )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bb-3.k10",   0x000000, 0x20000, CRC(04da1820) SHA1(0b6e06adf9c181d7aef28f781efbdd2c225fe81e) )
	ROM_LOAD16_BYTE( "bb-5.k12",   0x000001, 0x20000, CRC(777e0611) SHA1(b7ac0c6ea3738d560a5be75aed286821de918808) )
	ROM_LOAD16_BYTE( "bb-2.k8",    0x040000, 0x20000, CRC(20141805) SHA1(0958579681bda81bcf48d020a14bc147c1e575f1) )
	ROM_LOAD16_BYTE( "bb-4.k11",   0x040001, 0x20000, CRC(d482e0e9) SHA1(e56ca92965e8954b613ba4b0e3975e3a12840c30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bb-1.e6",     0x000000, 0x10000, CRC(4360f2ee) SHA1(4c6b212f59389bdf4388893d2030493b110ac087) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "bb-10.l9",    0x000000, 0x20000, CRC(490c0d9b) SHA1(567c25a6d96407259c64061d674305e4117d9fa4) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "bb-f11.m16",  0x000000, 0x80000, CRC(39fdf9c0) SHA1(80392947e3a1831c3ee80139f6f3bdc3bafa4f0d) )
	ROM_LOAD( "bb-f12.m13",  0x080000, 0x80000, CRC(69ee046b) SHA1(5c0435f1ce76b584fa8d154d7617d73c7ab5f62f) )
	ROM_LOAD( "bb-f13.m12",  0x100000, 0x80000, CRC(f5ef840e) SHA1(dd0f630c52076e0d330f47931e68a3ae9a401078) )
	ROM_LOAD( "bb-f14.m11",  0x180000, 0x80000, CRC(1a7df3bb) SHA1(1f27a528e6f89fe56a7342c4f1ff733da0a09327) )

	ROM_REGION( 0x200000, "gfx3", 0 )
	ROM_LOAD( "bb-f21.l10",  0x000000, 0x80000, CRC(530f595b) SHA1(820898693b878c4423de9c244f943d39ea69515e) )
	ROM_LOAD( "bb-f22.l12",  0x080000, 0x80000, CRC(889c562e) SHA1(d19172d6515ab9793c98de75d6e41687e61a408d) )
	ROM_LOAD( "bb-f23.l13",  0x100000, 0x80000, CRC(c89fe0da) SHA1(92be860a7191e7473c42aa2da981eda873219d3d) )
	ROM_LOAD( "bb-f24.l15",  0x180000, 0x80000, CRC(e0d81359) SHA1(2213c17651b6c023a456447f352b0739439f913a) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "bb-back1.m4", 0x000000, 0x80000, CRC(b5445313) SHA1(3c99b557b2af30ff0fbc8a7dc6c40448c4f327db) )

	ROM_REGION( 0x80000, "gfx5", 0 )
	ROM_LOAD( "bb-back2.m6", 0x000000, 0x80000, CRC(8be996f6) SHA1(1e2c56f4c24793f806d7b366b92edc03145ae94c) )

	ROM_REGION( 0x10000, "user1", 0 ) /* Zoom table */
	/* same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-6.e7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-7.h7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-8.a14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-9.c14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )
	ROM_LOAD( "bb-pcmb.l3",  0x000000, 0x80000, CRC(c8d5dd53) SHA1(0f7e94532cc14852ca12c1b792e5479667af899e) )
ROM_END

ROM_START( bbustersu )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bbv2-3.k10",   0x000000, 0x20000, CRC(6930088b) SHA1(265f0b584d81b6fdcda5c3a2e0bd15d56443bb35) )
	ROM_LOAD16_BYTE( "bbv2-5.k12",   0x000001, 0x20000, CRC(cfdb2c6c) SHA1(54a837dc84b74d12e931f607f3dc9ee06a7e4d31) )
	ROM_LOAD16_BYTE( "bb-2.k8",    0x040000, 0x20000, CRC(20141805) SHA1(0958579681bda81bcf48d020a14bc147c1e575f1) )
	ROM_LOAD16_BYTE( "bb-4.k11",   0x040001, 0x20000, CRC(d482e0e9) SHA1(e56ca92965e8954b613ba4b0e3975e3a12840c30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bb-1.e6",     0x000000, 0x10000, CRC(4360f2ee) SHA1(4c6b212f59389bdf4388893d2030493b110ac087) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "bb-10.l9",    0x000000, 0x20000, CRC(490c0d9b) SHA1(567c25a6d96407259c64061d674305e4117d9fa4) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "bb-f11.m16",  0x000000, 0x80000, CRC(39fdf9c0) SHA1(80392947e3a1831c3ee80139f6f3bdc3bafa4f0d) )
	ROM_LOAD( "bb-f12.m13",  0x080000, 0x80000, CRC(69ee046b) SHA1(5c0435f1ce76b584fa8d154d7617d73c7ab5f62f) )
	ROM_LOAD( "bb-f13.m12",  0x100000, 0x80000, CRC(f5ef840e) SHA1(dd0f630c52076e0d330f47931e68a3ae9a401078) )
	ROM_LOAD( "bb-f14.m11",  0x180000, 0x80000, CRC(1a7df3bb) SHA1(1f27a528e6f89fe56a7342c4f1ff733da0a09327) )

	ROM_REGION( 0x200000, "gfx3", 0 )
	ROM_LOAD( "bb-f21.l10",  0x000000, 0x80000, CRC(530f595b) SHA1(820898693b878c4423de9c244f943d39ea69515e) )
	ROM_LOAD( "bb-f22.l12",  0x080000, 0x80000, CRC(889c562e) SHA1(d19172d6515ab9793c98de75d6e41687e61a408d) )
	ROM_LOAD( "bb-f23.l13",  0x100000, 0x80000, CRC(c89fe0da) SHA1(92be860a7191e7473c42aa2da981eda873219d3d) )
	ROM_LOAD( "bb-f24.l15",  0x180000, 0x80000, CRC(e0d81359) SHA1(2213c17651b6c023a456447f352b0739439f913a) )

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "bb-back1.m4", 0x000000, 0x80000, CRC(b5445313) SHA1(3c99b557b2af30ff0fbc8a7dc6c40448c4f327db) )

	ROM_REGION( 0x80000, "gfx5", 0 )
	ROM_LOAD( "bb-back2.m6", 0x000000, 0x80000, CRC(8be996f6) SHA1(1e2c56f4c24793f806d7b366b92edc03145ae94c) )

	ROM_REGION( 0x10000, "user1", 0 ) /* Zoom table */
	/* same rom exists in 4 different locations on the board */
	ROM_LOAD( "bb-6.e7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-7.h7",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-8.a14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "bb-9.c14",      0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )
	ROM_LOAD( "bb-pcma.l5",  0x000000, 0x80000, CRC(44cd5bfe) SHA1(26a612191a0aa614c090203485aba17c99c763ee) )
ROM_END


ROM_START( mechatt )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ma5-e.bin", 0x000000, 0x20000, CRC(9bbb852a) SHA1(34b696bf79cf53cac1c384a3143c0f3f243a71f3) )
	ROM_LOAD16_BYTE( "ma4.bin",   0x000001, 0x20000, CRC(0d414918) SHA1(0d51b893d37ba124b983beebb691e65bdc52d300) )
	ROM_LOAD16_BYTE( "ma7.bin",   0x040000, 0x20000, CRC(61d85e1b) SHA1(46234d48ac21c481a5e70c6a654a341ebdd4cd3a) )
	ROM_LOAD16_BYTE( "ma6-f.bin", 0x040001, 0x20000, CRC(4055fe8d) SHA1(b4d8bd5f73805ce1c332eff657dddbb88ff45b37) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ma3.bin",       0x000000, 0x10000, CRC(c06cc8e1) SHA1(65f5f1901120d633f7c3ba07432a188fd7fd7272) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "ma1.bin",       0x000000, 0x10000, CRC(24766917) SHA1(9082a8ae849605ce65b5a0493ae69cfe282f7e7b) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "mao89p13.bin",  0x000000, 0x80000, CRC(8bcb16cf) SHA1(409ee1944188d9ce39adce29b1df029b560dd5b0) )
	ROM_LOAD( "ma189p15.bin",  0x080000, 0x80000, CRC(b84d9658) SHA1(448adecb0067d8f5b219ec2f94a8dec84187a554) )
	ROM_LOAD( "ma289p17.bin",  0x100000, 0x80000, CRC(6cbe08ac) SHA1(8f81f6e92b84ab6867452011d52f3e7689c62a1a) )
	ROM_LOAD( "ma389m15.bin",  0x180000, 0x80000, CRC(34d4585e) SHA1(38d9fd5d775e4b3c8b8b487a6ba9b8bdcb3274b0) )

	ROM_REGION( 0x10000, "gfx3", ROMREGION_ERASEFF )
	/* Unused */

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "mab189a2.bin",  0x000000, 0x80000, CRC(e1c8b4d0) SHA1(2f8a1839cca892f8380c7cffe7a12e615d38fd55) )

	ROM_REGION( 0x80000, "gfx5", 0 )
	ROM_LOAD( "mab289c2.bin",  0x000000, 0x80000, CRC(14f97ceb) SHA1(a22033532ea616dc3a3db8b66ad6ccc6172ed7cc) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "ma2.bin",       0x000000, 0x20000, CRC(ea4cc30d) SHA1(d8f089fc0ce76309411706a8110ad907f93dc97e) )

	ROM_REGION( 0x20000, "user1", 0 ) /* Zoom table */
	ROM_LOAD( "ma8.bin",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "ma9.bin",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) ) /* identical to ma8 */
ROM_END


ROM_START( mechattu )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ma5u.bin",   0x000000, 0x20000, CRC(485ea606) SHA1(0c499f08d7c6d861ba7c50a8f577823613a7923c) )
	ROM_LOAD16_BYTE( "ma4u.bin",   0x000001, 0x20000, CRC(09fa31ec) SHA1(008abb2e09f83614c277471e534f20cba3e354d7) )
	ROM_LOAD16_BYTE( "ma7u.bin",   0x040000, 0x20000, CRC(f45b2c70) SHA1(65523d202d378bab890f1f7bffdde152dd246d4a) )
	ROM_LOAD16_BYTE( "ma6u.bin",   0x040001, 0x20000, CRC(d5d68ce6) SHA1(16057d882781015f6d1c7bb659e0812a8459c3f0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ma3.bin",       0x000000, 0x10000, CRC(c06cc8e1) SHA1(65f5f1901120d633f7c3ba07432a188fd7fd7272) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "ma1.bin",       0x000000, 0x10000, CRC(24766917) SHA1(9082a8ae849605ce65b5a0493ae69cfe282f7e7b) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "mao89p13.bin",  0x000000, 0x80000, CRC(8bcb16cf) SHA1(409ee1944188d9ce39adce29b1df029b560dd5b0) )
	ROM_LOAD( "ma189p15.bin",  0x080000, 0x80000, CRC(b84d9658) SHA1(448adecb0067d8f5b219ec2f94a8dec84187a554) )
	ROM_LOAD( "ma289p17.bin",  0x100000, 0x80000, CRC(6cbe08ac) SHA1(8f81f6e92b84ab6867452011d52f3e7689c62a1a) )
	ROM_LOAD( "ma389m15.bin",  0x180000, 0x80000, CRC(34d4585e) SHA1(38d9fd5d775e4b3c8b8b487a6ba9b8bdcb3274b0) )

	ROM_REGION( 0x10000, "gfx3", ROMREGION_ERASEFF )
	/* Unused */

	ROM_REGION( 0x80000, "gfx4", 0 )
	ROM_LOAD( "mab189a2.bin",  0x000000, 0x80000, CRC(e1c8b4d0) SHA1(2f8a1839cca892f8380c7cffe7a12e615d38fd55) )

	ROM_REGION( 0x80000, "gfx5", 0 )
	ROM_LOAD( "mab289c2.bin",  0x000000, 0x80000, CRC(14f97ceb) SHA1(a22033532ea616dc3a3db8b66ad6ccc6172ed7cc) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "ma2.bin",       0x000000, 0x20000, CRC(ea4cc30d) SHA1(d8f089fc0ce76309411706a8110ad907f93dc97e) )

	ROM_REGION( 0x20000, "user1", 0 ) /* Zoom table */
	ROM_LOAD( "ma8.bin",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) )
	ROM_LOAD( "ma9.bin",       0x000000, 0x10000, CRC(61f3de03) SHA1(736f9634fe054ea68a2aa90a743bd0dc320f23c9) ) /* identical to ma8 */
ROM_END


/******************************************************************************/

// as soon as you calibrate the guns in test mode the game refuses to boot
GAME( 1989, bbusters, 0,        bbusters, bbusters, 0, ROT0,  "SNK", "Beast Busters (World)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1989, bbustersu,bbusters, bbusters, bbusters, 0, ROT0,  "SNK", "Beast Busters (US, Version 2)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1989, mechatt,  0,        mechatt,  mechatt,  0, ROT0,  "SNK", "Mechanized Attack (World)", 0 )
GAME( 1989, mechattu, mechatt,  mechatt,  mechattu, 0, ROT0,  "SNK", "Mechanized Attack (US)", 0 )
