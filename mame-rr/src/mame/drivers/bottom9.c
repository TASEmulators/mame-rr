/***************************************************************************

    Bottom of the Ninth (c) 1989 Konami

    Similar to S.P.Y.

    driver by Nicola Salmoria

    2008-07
    Dip locations and factory settings verified with US manual.
    To be verified for Main Stadium

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "cpu/m6809/m6809.h"
#include "video/konicdev.h"
#include "sound/k007232.h"
#include "includes/konamipt.h"
#include "includes/bottom9.h"

static INTERRUPT_GEN( bottom9_interrupt )
{
	bottom9_state *state = (bottom9_state *)device->machine->driver_data;

	if (k052109_is_irq_enabled(state->k052109))
		cpu_set_input_line(device, 0, HOLD_LINE);
}

static READ8_HANDLER( k052109_051960_r )
{
	bottom9_state *state = (bottom9_state *)space->machine->driver_data;

	if (k052109_get_rmrd_line(state->k052109) == CLEAR_LINE)
	{
		if (offset >= 0x3800 && offset < 0x3808)
			return k051937_r(state->k051960, offset - 0x3800);
		else if (offset < 0x3c00)
			return k052109_r(state->k052109, offset);
		else
			return k051960_r(state->k051960, offset - 0x3c00);
	}
	else
		return k052109_r(state->k052109, offset);
}

static WRITE8_HANDLER( k052109_051960_w )
{
	bottom9_state *state = (bottom9_state *)space->machine->driver_data;

	if (offset >= 0x3800 && offset < 0x3808)
		k051937_w(state->k051960, offset - 0x3800, data);
	else if (offset < 0x3c00)
		k052109_w(state->k052109, offset, data);
	else
		k051960_w(state->k051960, offset - 0x3c00, data);
}

static READ8_HANDLER( bottom9_bankedram1_r )
{
	bottom9_state *state = (bottom9_state *)space->machine->driver_data;

	if (state->k052109_selected)
		return k052109_051960_r(space, offset);
	else
	{
		if (state->zoomreadroms)
			return k051316_rom_r(state->k051316, offset);
		else
			return k051316_r(state->k051316, offset);
	}
}

static WRITE8_HANDLER( bottom9_bankedram1_w )
{
	bottom9_state *state = (bottom9_state *)space->machine->driver_data;

	if (state->k052109_selected)
		k052109_051960_w(space, offset, data);
	else
		k051316_w(state->k051316, offset, data);
}

static READ8_HANDLER( bottom9_bankedram2_r )
{
	bottom9_state *state = (bottom9_state *)space->machine->driver_data;

	if (state->k052109_selected)
		return k052109_051960_r(space, offset + 0x2000);
	else
		return space->machine->generic.paletteram.u8[offset];
}

static WRITE8_HANDLER( bottom9_bankedram2_w )
{
	bottom9_state *state = (bottom9_state *)space->machine->driver_data;

	if (state->k052109_selected)
		k052109_051960_w(space, offset + 0x2000, data);
	else
		paletteram_xBBBBBGGGGGRRRRR_be_w(space, offset, data);
}

static WRITE8_HANDLER( bankswitch_w )
{
	int bank;

	/* bit 0 = RAM bank */
	if ((data & 1) == 0)
		popmessage("bankswitch RAM bank 0");

	/* bit 1-4 = ROM bank */
	if (data & 0x10)
		bank = 8 + ((data & 0x06) >> 1);
	else
		bank = ((data & 0x0e) >> 1);

	memory_set_bank(space->machine, "bank1", bank);
}

static WRITE8_HANDLER( bottom9_1f90_w )
{
	bottom9_state *state = (bottom9_state *)space->machine->driver_data;

	/* bits 0/1 = coin counters */
	coin_counter_w(space->machine, 0, data & 0x01);
	coin_counter_w(space->machine, 1, data & 0x02);

	/* bit 2 = enable char ROM reading through the video RAM */
	k052109_set_rmrd_line(state->k052109, (data & 0x04) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 3 = disable video */
	state->video_enable = ~data & 0x08;

	/* bit 4 = enable 051316 ROM reading */
	state->zoomreadroms = data & 0x10;

	/* bit 5 = RAM bank */
	state->k052109_selected = data & 0x20;
}

static WRITE8_HANDLER( bottom9_sh_irqtrigger_w )
{
	bottom9_state *state = (bottom9_state *)space->machine->driver_data;
	cpu_set_input_line_and_vector(state->audiocpu, 0, HOLD_LINE, 0xff);
}

static INTERRUPT_GEN( bottom9_sound_interrupt )
{
	bottom9_state *state = (bottom9_state *)device->machine->driver_data;
	if (state->nmienable)
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( nmi_enable_w )
{
	bottom9_state *state = (bottom9_state *)space->machine->driver_data;
	state->nmienable = data;
}

static WRITE8_HANDLER( sound_bank_w )
{
	bottom9_state *state = (bottom9_state *)space->machine->driver_data;
	int bank_A, bank_B;

	bank_A = ((data >> 0) & 0x03);
	bank_B = ((data >> 2) & 0x03);
	k007232_set_bank(state->k007232_1, bank_A, bank_B);

	bank_A = ((data >> 4) & 0x03);
	bank_B = ((data >> 6) & 0x03);
	k007232_set_bank(state->k007232_2, bank_A, bank_B);
}


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_READWRITE(bottom9_bankedram1_r, bottom9_bankedram1_w)
	AM_RANGE(0x1f80, 0x1f80) AM_WRITE(bankswitch_w)
	AM_RANGE(0x1f90, 0x1f90) AM_WRITE(bottom9_1f90_w)
	AM_RANGE(0x1fa0, 0x1fa0) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x1fb0, 0x1fb0) AM_WRITE(soundlatch_w)
	AM_RANGE(0x1fc0, 0x1fc0) AM_WRITE(bottom9_sh_irqtrigger_w)
	AM_RANGE(0x1fd0, 0x1fd0) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1fd1, 0x1fd1) AM_READ_PORT("P1")
	AM_RANGE(0x1fd2, 0x1fd2) AM_READ_PORT("P2")
	AM_RANGE(0x1fd3, 0x1fd3) AM_READ_PORT("DSW1")
	AM_RANGE(0x1fe0, 0x1fe0) AM_READ_PORT("DSW2")
	AM_RANGE(0x1ff0, 0x1fff) AM_DEVWRITE("k051316", k051316_ctrl_w)
	AM_RANGE(0x2000, 0x27ff) AM_READWRITE(bottom9_bankedram2_r, bottom9_bankedram2_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x0000, 0x3fff) AM_READWRITE(k052109_051960_r, k052109_051960_w)
	AM_RANGE(0x4000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_WRITE(sound_bank_w)
	AM_RANGE(0xa000, 0xa00d) AM_DEVREADWRITE("k007232_1", k007232_r, k007232_w)
	AM_RANGE(0xb000, 0xb00d) AM_DEVREADWRITE("k007232_2", k007232_r, k007232_w)
	AM_RANGE(0xd000, 0xd000) AM_READ(soundlatch_r)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(nmi_enable_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( bottom9 )
	PORT_START("P1")
	KONAMI8_ALT_B12(1)

	PORT_START("P2")
	KONAMI8_ALT_B12(2)

	PORT_START("DSW1")
	KONAMI_COINAGE_ALT_LOC(SW1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x04, "Play Time" ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, "1'00" )
	PORT_DIPSETTING(    0x06, "1'10" )
	PORT_DIPSETTING(    0x05, "1'20" )
	PORT_DIPSETTING(    0x04, "1'30" )
	PORT_DIPSETTING(    0x03, "1'40" )
	PORT_DIPSETTING(    0x02, "1'50" )
	PORT_DIPSETTING(    0x01, "2'00" )
	PORT_DIPSETTING(    0x00, "2'10" )
	PORT_DIPNAME( 0x18, 0x08, "Bonus Time" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "00" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(	0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:2" )	/* According to manual: N/U */
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x80, 0x80, "Fielder Control" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Auto" )
INPUT_PORTS_END

static INPUT_PORTS_START( mstadium )
	PORT_INCLUDE( bottom9 )

	PORT_MODIFY("P1")
	KONAMI8_ALT_B123(1)

	PORT_MODIFY("P2")
	KONAMI8_ALT_B123(2)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Play Inning" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x08, "Play Inning Time" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "6 Min" )
	PORT_DIPSETTING(    0x08, "8 Min" )
	PORT_DIPSETTING(    0x04, "10 Min" )
	PORT_DIPSETTING(    0x00, "12 Min" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static void volume_callback0( running_device *device, int v )
{
	k007232_set_volume(device, 0, (v >> 4) * 0x11, 0);
	k007232_set_volume(device, 1, 0, (v & 0x0f) * 0x11);
}

static void volume_callback1( running_device *device, int v )
{
	k007232_set_volume(device, 0, (v >> 4) * 0x11, 0);
	k007232_set_volume(device, 1, 0, (v & 0x0f) * 0x11);
}

static const k007232_interface k007232_interface_1 =
{
	volume_callback0
};

static const k007232_interface k007232_interface_2 =
{
	volume_callback1
};



static const k052109_interface bottom9_k052109_intf =
{
	"gfx1", 0,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	bottom9_tile_callback
};

static const k051960_interface bottom9_k051960_intf =
{
	"gfx2", 1,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	bottom9_sprite_callback
};

static const k051316_interface bottom9_k051316_intf =
{
	"gfx3", 2,
	4, FALSE, 0,
	0, 0, 0,
	bottom9_zoom_callback
};

static MACHINE_START( bottom9 )
{
	bottom9_state *state = (bottom9_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "maincpu");

	memory_configure_bank(machine, "bank1", 0, 12, &ROM[0x10000], 0x2000);

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->k052109 = machine->device("k052109");
	state->k051960 = machine->device("k051960");
	state->k051316 = machine->device("k051316");
	state->k007232_1 = machine->device("k007232_1");
	state->k007232_2 = machine->device("k007232_2");

	state_save_register_global(machine, state->video_enable);
	state_save_register_global(machine, state->zoomreadroms);
	state_save_register_global(machine, state->k052109_selected);
	state_save_register_global(machine, state->nmienable);
}

static MACHINE_RESET( bottom9 )
{
	bottom9_state *state = (bottom9_state *)machine->driver_data;

	state->video_enable = 0;
	state->zoomreadroms = 0;
	state->k052109_selected = 0;
	state->nmienable = 0;
}

static MACHINE_DRIVER_START( bottom9 )

	/* driver data */
	MDRV_DRIVER_DATA(bottom9_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, 2000000) /* ? */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", bottom9_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, 3579545)
	MDRV_CPU_PROGRAM_MAP(audio_map)
	MDRV_CPU_VBLANK_INT_HACK(bottom9_sound_interrupt,8)	/* irq is triggered by the main CPU */

	MDRV_MACHINE_START(bottom9)
	MDRV_MACHINE_RESET(bottom9)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )

	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(bottom9)
	MDRV_VIDEO_UPDATE(bottom9)

	MDRV_K052109_ADD("k052109", bottom9_k052109_intf)
	MDRV_K051960_ADD("k051960", bottom9_k051960_intf)
	MDRV_K051316_ADD("k051316", bottom9_k051316_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("k007232_1", K007232, 3579545)
	MDRV_SOUND_CONFIG(k007232_interface_1)
	MDRV_SOUND_ROUTE(0, "mono", 0.40)
	MDRV_SOUND_ROUTE(1, "mono", 0.40)

	MDRV_SOUND_ADD("k007232_2", K007232, 3579545)
	MDRV_SOUND_CONFIG(k007232_interface_2)
	MDRV_SOUND_ROUTE(0, "mono", 0.40)
	MDRV_SOUND_ROUTE(1, "mono", 0.40)
MACHINE_DRIVER_END


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( bottom9 )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "891n03.k17",   0x10000, 0x10000, CRC(8b083ff3) SHA1(045fef944b192e4bb147fa0f28680c0602af7377) )
	ROM_LOAD( "891-t02.k15",  0x20000, 0x08000, CRC(2c10ced2) SHA1(ecd43825a67b495cade94a454c96a19143d87760) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code */
	ROM_LOAD( "891j01.g8",    0x0000, 0x8000, CRC(31b0a0a8) SHA1(8e047f81c19f25de97fa22e70dcfe9e06bfae699) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD16_BYTE( "891e10c", 0x00000, 0x10000, CRC(209b0431) SHA1(07f05f63267d5ed5c99b5f786bb66a87045db9e1) )	/* characters */
	ROM_LOAD16_BYTE( "891e10a", 0x00001, 0x10000, CRC(8020a9e8) SHA1(3792794a1b875506089da63cae955668cc61f54b) )
	ROM_LOAD16_BYTE( "891e10d", 0x20000, 0x10000, CRC(16d5fd7a) SHA1(895a53e41173a70c48337d812466857676908a23) )
	ROM_LOAD16_BYTE( "891e10b", 0x20001, 0x10000, CRC(30121cc0) SHA1(79174d00b79855c00c9c872b8f32946be1bf1d8a) )
	ROM_LOAD16_BYTE( "891e09c", 0x40000, 0x10000, CRC(9dcaefbf) SHA1(8b61b1627737b959158aa6c7ea5db63f6aec7436) )
	ROM_LOAD16_BYTE( "891e09a", 0x40001, 0x10000, CRC(56b0ead9) SHA1(ef4b00ed0de93f61f4c8661ec0e6049c51a25cf6) )
	ROM_LOAD16_BYTE( "891e09d", 0x60000, 0x10000, CRC(4e1335e6) SHA1(b892ab40a41978a89658ea2e7aabe9b073430b5d) )
	ROM_LOAD16_BYTE( "891e09b", 0x60001, 0x10000, CRC(b6f914fb) SHA1(e95f3e899c2ead15ef8a529dbc67e8f4a0f88bdd) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD16_BYTE( "891e06e", 0x00000, 0x10000, CRC(0b04db1c) SHA1(0beae7bb8da49379915c0253ce03091eb71a58b5) )	/* sprites */
	ROM_LOAD16_BYTE( "891e06a", 0x00001, 0x10000, CRC(5ee37327) SHA1(f63ddaf63af06ea5421b0361315940582ef57922) )
	ROM_LOAD16_BYTE( "891e06f", 0x20000, 0x10000, CRC(f9ada524) SHA1(2df1fe91f43b95bb4e4a24a0931ab6f540496f65) )
	ROM_LOAD16_BYTE( "891e06b", 0x20001, 0x10000, CRC(2295dfaa) SHA1(96070e1bd07b33b6701e45ee1e200f24532e8630) )
	ROM_LOAD16_BYTE( "891e06g", 0x40000, 0x10000, CRC(04abf78f) SHA1(9a21cc71993c3074a8a61c654b998466503b31ef) )
	ROM_LOAD16_BYTE( "891e06c", 0x40001, 0x10000, CRC(dbdb0d55) SHA1(8269b9be8f36116eb6d10efbb6b7050846a9290c) )
	ROM_LOAD16_BYTE( "891e06h", 0x60000, 0x10000, CRC(5d5ded8c) SHA1(2581aa387c1ba1f2b7c59bae2c59fbf127aa4e86) )
	ROM_LOAD16_BYTE( "891e06d", 0x60001, 0x10000, CRC(f9ecbd71) SHA1(45e28a8b40159fd0cdcc8ad253ffc7eba6cf3535) )
	ROM_LOAD16_BYTE( "891e05e", 0x80000, 0x10000, CRC(b356e729) SHA1(2cda591415b0f139fdb1f80c349d432bb0579d8e) )
	ROM_LOAD16_BYTE( "891e05a", 0x80001, 0x10000, CRC(bfd5487e) SHA1(24e0de9f12f6df6bde6268d090fe9e1ea827c0dc) )
	ROM_LOAD16_BYTE( "891e05f", 0xa0000, 0x10000, CRC(ecdd11c5) SHA1(8eac76b3b0f2ab4d59491e10070a62fd9f1eba81) )
	ROM_LOAD16_BYTE( "891e05b", 0xa0001, 0x10000, CRC(aba18d24) SHA1(ba8e1fab9537199ece2af26bb3f5c8d85d5213d4) )
	ROM_LOAD16_BYTE( "891e05g", 0xc0000, 0x10000, CRC(c315f9ae) SHA1(8e2c8ca1c6dcfe5b7302ea89275b231ffb2e0e84) )
	ROM_LOAD16_BYTE( "891e05c", 0xc0001, 0x10000, CRC(21fcbc6f) SHA1(efc65973ea7702a1b5c26a966f452804ad97dbd4) )
	ROM_LOAD16_BYTE( "891e05h", 0xe0000, 0x10000, CRC(b0aba53b) SHA1(e76b345ae354533959ed06217b91ce3c93b22a23) )
	ROM_LOAD16_BYTE( "891e05d", 0xe0001, 0x10000, CRC(f6d3f886) SHA1(b8bdcc9470aa93849b8c8a1f03971281cacc6d44) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "891e07a",      0x00000, 0x10000, CRC(b8d8b939) SHA1(ee91fb46d70db2d17f5909c4ea7ee1cf2d317d10) )	/* zoom/rotate */
	ROM_LOAD( "891e07b",      0x10000, 0x10000, CRC(83b2f92d) SHA1(c4972018e1f8109656784fae3e023a5522622c4b) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "891b11.f23",   0x0000, 0x0100, CRC(ecb854aa) SHA1(3bd321ca3076d4e0042e0af656d51909fa6a5b3b) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "k007232_1", 0 ) /* samples for 007232 #0 */
	ROM_LOAD( "891e08a",      0x00000, 0x10000, CRC(cef667bf) SHA1(e773fc0ced45e01e13cdee18c404d609356d2d0e) )
	ROM_LOAD( "891e08b",      0x10000, 0x10000, CRC(f7c14a7a) SHA1(05261a065de33e158e8d72d74eb657035abb5d03) )
	ROM_LOAD( "891e08c",      0x20000, 0x10000, CRC(756b7f3c) SHA1(6f36f0b4e08db27a8b6e180d12be6427677ad62d) )
	ROM_LOAD( "891e08d",      0x30000, 0x10000, CRC(cd0d7305) SHA1(82403ce1f38014ebf94008a66c98697a572303f9) )

	ROM_REGION( 0x40000, "k007232_2", 0 ) /* samples for 007232 #1 */
	ROM_LOAD( "891e04a",      0x00000, 0x10000, CRC(daebbc74) SHA1(f61daebf80e5e4640c4cea4ea5767e64a49d928d) )
	ROM_LOAD( "891e04b",      0x10000, 0x10000, CRC(5ffb9ad1) SHA1(e8f00c63dc3091aa344e82dc29f41aedd5a764b4) )
	ROM_LOAD( "891e04c",      0x20000, 0x10000, CRC(2dbbf16b) SHA1(84b2005a1fe61a6a0cf1aa6e0fdf7ff8b1f8f82a) )
	ROM_LOAD( "891e04d",      0x30000, 0x10000, CRC(8b0cd2cc) SHA1(e14109c69fa24d309aed4ff3589cc6619e29f97f) )
ROM_END

ROM_START( bottom9n )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "891n03.k17",   0x10000, 0x10000, CRC(8b083ff3) SHA1(045fef944b192e4bb147fa0f28680c0602af7377) )
	ROM_LOAD( "891n02.k15",   0x20000, 0x08000, CRC(d44d9ed4) SHA1(2a12bcfba81ab7e074569e2ad2da6a237a1c0ce5) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code */
	ROM_LOAD( "891j01.g8",    0x0000, 0x8000, CRC(31b0a0a8) SHA1(8e047f81c19f25de97fa22e70dcfe9e06bfae699) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD16_BYTE( "891e10c", 0x00000, 0x10000, CRC(209b0431) SHA1(07f05f63267d5ed5c99b5f786bb66a87045db9e1) )	/* characters */
	ROM_LOAD16_BYTE( "891e10a", 0x00001, 0x10000, CRC(8020a9e8) SHA1(3792794a1b875506089da63cae955668cc61f54b) )
	ROM_LOAD16_BYTE( "891e10d", 0x20000, 0x10000, CRC(16d5fd7a) SHA1(895a53e41173a70c48337d812466857676908a23) )
	ROM_LOAD16_BYTE( "891e10b", 0x20001, 0x10000, CRC(30121cc0) SHA1(79174d00b79855c00c9c872b8f32946be1bf1d8a) )
	ROM_LOAD16_BYTE( "891e09c", 0x40000, 0x10000, CRC(9dcaefbf) SHA1(8b61b1627737b959158aa6c7ea5db63f6aec7436) )
	ROM_LOAD16_BYTE( "891e09a", 0x40001, 0x10000, CRC(56b0ead9) SHA1(ef4b00ed0de93f61f4c8661ec0e6049c51a25cf6) )
	ROM_LOAD16_BYTE( "891e09d", 0x60000, 0x10000, CRC(4e1335e6) SHA1(b892ab40a41978a89658ea2e7aabe9b073430b5d) )
	ROM_LOAD16_BYTE( "891e09b", 0x60001, 0x10000, CRC(b6f914fb) SHA1(e95f3e899c2ead15ef8a529dbc67e8f4a0f88bdd) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD16_BYTE( "891e06e", 0x00000, 0x10000, CRC(0b04db1c) SHA1(0beae7bb8da49379915c0253ce03091eb71a58b5) )	/* sprites */
	ROM_LOAD16_BYTE( "891e06a", 0x00001, 0x10000, CRC(5ee37327) SHA1(f63ddaf63af06ea5421b0361315940582ef57922) )
	ROM_LOAD16_BYTE( "891e06f", 0x20000, 0x10000, CRC(f9ada524) SHA1(2df1fe91f43b95bb4e4a24a0931ab6f540496f65) )
	ROM_LOAD16_BYTE( "891e06b", 0x20001, 0x10000, CRC(2295dfaa) SHA1(96070e1bd07b33b6701e45ee1e200f24532e8630) )
	ROM_LOAD16_BYTE( "891e06g", 0x40000, 0x10000, CRC(04abf78f) SHA1(9a21cc71993c3074a8a61c654b998466503b31ef) )
	ROM_LOAD16_BYTE( "891e06c", 0x40001, 0x10000, CRC(dbdb0d55) SHA1(8269b9be8f36116eb6d10efbb6b7050846a9290c) )
	ROM_LOAD16_BYTE( "891e06h", 0x60000, 0x10000, CRC(5d5ded8c) SHA1(2581aa387c1ba1f2b7c59bae2c59fbf127aa4e86) )
	ROM_LOAD16_BYTE( "891e06d", 0x60001, 0x10000, CRC(f9ecbd71) SHA1(45e28a8b40159fd0cdcc8ad253ffc7eba6cf3535) )
	ROM_LOAD16_BYTE( "891e05e", 0x80000, 0x10000, CRC(b356e729) SHA1(2cda591415b0f139fdb1f80c349d432bb0579d8e) )
	ROM_LOAD16_BYTE( "891e05a", 0x80001, 0x10000, CRC(bfd5487e) SHA1(24e0de9f12f6df6bde6268d090fe9e1ea827c0dc) )
	ROM_LOAD16_BYTE( "891e05f", 0xa0000, 0x10000, CRC(ecdd11c5) SHA1(8eac76b3b0f2ab4d59491e10070a62fd9f1eba81) )
	ROM_LOAD16_BYTE( "891e05b", 0xa0001, 0x10000, CRC(aba18d24) SHA1(ba8e1fab9537199ece2af26bb3f5c8d85d5213d4) )
	ROM_LOAD16_BYTE( "891e05g", 0xc0000, 0x10000, CRC(c315f9ae) SHA1(8e2c8ca1c6dcfe5b7302ea89275b231ffb2e0e84) )
	ROM_LOAD16_BYTE( "891e05c", 0xc0001, 0x10000, CRC(21fcbc6f) SHA1(efc65973ea7702a1b5c26a966f452804ad97dbd4) )
	ROM_LOAD16_BYTE( "891e05h", 0xe0000, 0x10000, CRC(b0aba53b) SHA1(e76b345ae354533959ed06217b91ce3c93b22a23) )
	ROM_LOAD16_BYTE( "891e05d", 0xe0001, 0x10000, CRC(f6d3f886) SHA1(b8bdcc9470aa93849b8c8a1f03971281cacc6d44) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "891e07a",      0x00000, 0x10000, CRC(b8d8b939) SHA1(ee91fb46d70db2d17f5909c4ea7ee1cf2d317d10) )	/* zoom/rotate */
	ROM_LOAD( "891e07b",      0x10000, 0x10000, CRC(83b2f92d) SHA1(c4972018e1f8109656784fae3e023a5522622c4b) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "891b11.f23",   0x0000, 0x0100, CRC(ecb854aa) SHA1(3bd321ca3076d4e0042e0af656d51909fa6a5b3b) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "k007232_1", 0 ) /* samples for 007232 #0 */
	ROM_LOAD( "891e08a",      0x00000, 0x10000, CRC(cef667bf) SHA1(e773fc0ced45e01e13cdee18c404d609356d2d0e) )
	ROM_LOAD( "891e08b",      0x10000, 0x10000, CRC(f7c14a7a) SHA1(05261a065de33e158e8d72d74eb657035abb5d03) )
	ROM_LOAD( "891e08c",      0x20000, 0x10000, CRC(756b7f3c) SHA1(6f36f0b4e08db27a8b6e180d12be6427677ad62d) )
	ROM_LOAD( "891e08d",      0x30000, 0x10000, CRC(cd0d7305) SHA1(82403ce1f38014ebf94008a66c98697a572303f9) )

	ROM_REGION( 0x40000, "k007232_2", 0 ) /* samples for 007232 #1 */
	ROM_LOAD( "891e04a",      0x00000, 0x10000, CRC(daebbc74) SHA1(f61daebf80e5e4640c4cea4ea5767e64a49d928d) )
	ROM_LOAD( "891e04b",      0x10000, 0x10000, CRC(5ffb9ad1) SHA1(e8f00c63dc3091aa344e82dc29f41aedd5a764b4) )
	ROM_LOAD( "891e04c",      0x20000, 0x10000, CRC(2dbbf16b) SHA1(84b2005a1fe61a6a0cf1aa6e0fdf7ff8b1f8f82a) )
	ROM_LOAD( "891e04d",      0x30000, 0x10000, CRC(8b0cd2cc) SHA1(e14109c69fa24d309aed4ff3589cc6619e29f97f) )
ROM_END

ROM_START( mstadium )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "891-403.k17",   0x10000, 0x10000, CRC(1c00c4e8) SHA1(8a3400a8df44f21616422e5af3bca84d0f390f63) )
	ROM_LOAD( "891-402.k15",   0x20000, 0x08000, CRC(b850bbce) SHA1(a64300d1b1068e59eb59c427946c9bff164e2da8) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code */
	ROM_LOAD( "891w01.g8",    0x0000, 0x8000, CRC(edec565a) SHA1(69cba0d00c6ef76c4ce2b553e3fd15de8abbbf31) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD16_BYTE( "891e10c", 0x00000, 0x10000, CRC(209b0431) SHA1(07f05f63267d5ed5c99b5f786bb66a87045db9e1) )	/* characters */
	ROM_LOAD16_BYTE( "891e10a", 0x00001, 0x10000, CRC(8020a9e8) SHA1(3792794a1b875506089da63cae955668cc61f54b) )
	ROM_LOAD16_BYTE( "891e10d", 0x20000, 0x10000, CRC(16d5fd7a) SHA1(895a53e41173a70c48337d812466857676908a23) )
	ROM_LOAD16_BYTE( "891e10b", 0x20001, 0x10000, CRC(30121cc0) SHA1(79174d00b79855c00c9c872b8f32946be1bf1d8a) )
	ROM_LOAD16_BYTE( "891e09c", 0x40000, 0x10000, CRC(9dcaefbf) SHA1(8b61b1627737b959158aa6c7ea5db63f6aec7436) )
	ROM_LOAD16_BYTE( "891e09a", 0x40001, 0x10000, CRC(56b0ead9) SHA1(ef4b00ed0de93f61f4c8661ec0e6049c51a25cf6) )
	ROM_LOAD16_BYTE( "891e09d", 0x60000, 0x10000, CRC(4e1335e6) SHA1(b892ab40a41978a89658ea2e7aabe9b073430b5d) )
	ROM_LOAD16_BYTE( "891e09b", 0x60001, 0x10000, CRC(b6f914fb) SHA1(e95f3e899c2ead15ef8a529dbc67e8f4a0f88bdd) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD16_BYTE( "891e06e", 0x00000, 0x10000, CRC(0b04db1c) SHA1(0beae7bb8da49379915c0253ce03091eb71a58b5) )	/* sprites */
	ROM_LOAD16_BYTE( "891e06a", 0x00001, 0x10000, CRC(5ee37327) SHA1(f63ddaf63af06ea5421b0361315940582ef57922) )
	ROM_LOAD16_BYTE( "891e06f", 0x20000, 0x10000, CRC(f9ada524) SHA1(2df1fe91f43b95bb4e4a24a0931ab6f540496f65) )
	ROM_LOAD16_BYTE( "891e06b", 0x20001, 0x10000, CRC(2295dfaa) SHA1(96070e1bd07b33b6701e45ee1e200f24532e8630) )
	ROM_LOAD16_BYTE( "891e06g", 0x40000, 0x10000, CRC(04abf78f) SHA1(9a21cc71993c3074a8a61c654b998466503b31ef) )
	ROM_LOAD16_BYTE( "891e06c", 0x40001, 0x10000, CRC(dbdb0d55) SHA1(8269b9be8f36116eb6d10efbb6b7050846a9290c) )
	ROM_LOAD16_BYTE( "891e06h", 0x60000, 0x10000, CRC(5d5ded8c) SHA1(2581aa387c1ba1f2b7c59bae2c59fbf127aa4e86) )
	ROM_LOAD16_BYTE( "891e06d", 0x60001, 0x10000, CRC(f9ecbd71) SHA1(45e28a8b40159fd0cdcc8ad253ffc7eba6cf3535) )
	ROM_LOAD16_BYTE( "891e05e", 0x80000, 0x10000, CRC(b356e729) SHA1(2cda591415b0f139fdb1f80c349d432bb0579d8e) )
	ROM_LOAD16_BYTE( "891e05a", 0x80001, 0x10000, CRC(bfd5487e) SHA1(24e0de9f12f6df6bde6268d090fe9e1ea827c0dc) )
	ROM_LOAD16_BYTE( "891e05f", 0xa0000, 0x10000, CRC(ecdd11c5) SHA1(8eac76b3b0f2ab4d59491e10070a62fd9f1eba81) )
	ROM_LOAD16_BYTE( "891e05b", 0xa0001, 0x10000, CRC(aba18d24) SHA1(ba8e1fab9537199ece2af26bb3f5c8d85d5213d4) )
	ROM_LOAD16_BYTE( "891e05g", 0xc0000, 0x10000, CRC(c315f9ae) SHA1(8e2c8ca1c6dcfe5b7302ea89275b231ffb2e0e84) )
	ROM_LOAD16_BYTE( "891e05c", 0xc0001, 0x10000, CRC(21fcbc6f) SHA1(efc65973ea7702a1b5c26a966f452804ad97dbd4) )
	ROM_LOAD16_BYTE( "891e05h", 0xe0000, 0x10000, CRC(b0aba53b) SHA1(e76b345ae354533959ed06217b91ce3c93b22a23) )
	ROM_LOAD16_BYTE( "891e05d", 0xe0001, 0x10000, CRC(f6d3f886) SHA1(b8bdcc9470aa93849b8c8a1f03971281cacc6d44) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "891e07a",      0x00000, 0x10000, CRC(b8d8b939) SHA1(ee91fb46d70db2d17f5909c4ea7ee1cf2d317d10) )	/* zoom/rotate */
	ROM_LOAD( "891e07b",      0x10000, 0x10000, CRC(83b2f92d) SHA1(c4972018e1f8109656784fae3e023a5522622c4b) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "891b11.f23",   0x0000, 0x0100, CRC(ecb854aa) SHA1(3bd321ca3076d4e0042e0af656d51909fa6a5b3b) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "k007232_1", 0 ) /* samples for 007232 #0 */
	ROM_LOAD( "891e08a",      0x00000, 0x10000, CRC(cef667bf) SHA1(e773fc0ced45e01e13cdee18c404d609356d2d0e) )
	ROM_LOAD( "891e08b",      0x10000, 0x10000, CRC(f7c14a7a) SHA1(05261a065de33e158e8d72d74eb657035abb5d03) )
	ROM_LOAD( "891e08c",      0x20000, 0x10000, CRC(756b7f3c) SHA1(6f36f0b4e08db27a8b6e180d12be6427677ad62d) )
	ROM_LOAD( "891e08d",      0x30000, 0x10000, CRC(cd0d7305) SHA1(82403ce1f38014ebf94008a66c98697a572303f9) )

	ROM_REGION( 0x40000, "k007232_2", 0 ) /* samples for 007232 #1 */
	ROM_LOAD( "891e04a",      0x00000, 0x10000, CRC(daebbc74) SHA1(f61daebf80e5e4640c4cea4ea5767e64a49d928d) )
	ROM_LOAD( "891e04b",      0x10000, 0x10000, CRC(5ffb9ad1) SHA1(e8f00c63dc3091aa344e82dc29f41aedd5a764b4) )
	ROM_LOAD( "891e04c",      0x20000, 0x10000, CRC(2dbbf16b) SHA1(84b2005a1fe61a6a0cf1aa6e0fdf7ff8b1f8f82a) )
	ROM_LOAD( "891e04d",      0x30000, 0x10000, CRC(8b0cd2cc) SHA1(e14109c69fa24d309aed4ff3589cc6619e29f97f) )
ROM_END



GAME( 1989, bottom9,  0,       bottom9, bottom9,  0, ROT0, "Konami", "Bottom of the Ninth (version T)", GAME_SUPPORTS_SAVE )
GAME( 1989, bottom9n, bottom9, bottom9, bottom9,  0, ROT0, "Konami", "Bottom of the Ninth (version N)", GAME_SUPPORTS_SAVE )
GAME( 1989, mstadium, bottom9, bottom9, mstadium, 0, ROT0, "Konami", "Main Stadium (Japan)", GAME_SUPPORTS_SAVE )
