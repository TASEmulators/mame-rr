/********************************************************************

Pasha Pasha 2
Dong Sung, 1998

3PLAY
|--------------------------------------------------|
|      DA1311    UM53   AD-65    DREAM  9.6MHz     |
|KA22065  TL062  UM51   AD-65          RESET_SW    |
|   VOL1   VOL2                  1MHz   TL7705     |
|                                                  |
|    DSW2(8)            20MHz    GM71C18163        |
|     93C46                                   UM2  |
|           PAL                                    |
|J                                                 |
|A                                                 |
|M                      E1-16XT             AT89C52|
|M                                                 |
|A                                  U102           |
|           6116                                   |
|           6116       U3           U101           |
|                                                  |
|                                                  |
|                                             12MHz|
|                    A42MX16                       |
|    DSW1(8)                                       |
| UCN5801                                          |
| UCN5801                                          |
|         16MHz                                    |
|--------------------------------------------------|
Notes:
      U3         - 27C040 EPROM (DIP32)
      UM2/UM51/53- 29F040 EPROM (PLCC32)
      U101/102   - Each location contains a small adapter board plugged into a DIP42 socket. Each
                   adapter board holds 2x Intel E28F016S5 TSOP40 16M FlashROMs. On the PCB under the ROMs
                   it's marked '32MASK'. However, the adapter boards are not standard. If you try to read
                   the ROMs while they are _ON-THE-ADAPTER_ as a 32M DIP42 EPROM (such as 27C322), the
                   FlashROMs are damaged and the PCB no longer works :(
                   Thus, the FlashROMs must be removed and read separately!
                   The small adapter boards with their respective FlashROMs are laid out like this........

                   |------------------------------|
                   |                              |
                   |       U2           U1        |  U102
                   |                              |
                   |------------------------------|

                   |------------------------------|
                   |                              |
                   |       U2           U1        |  U101
                   |                              |
                   |------------------------------|

      A42MX16    - Actel A42MX16 FPGA (QFP160)
      AT89C52    - Atmel AT89C52 Microcontroller w/8k internal FlashROM, clock 12MHz (DIP40)
      E1-16XT    - Hyperstone E1-16XT CPU, clock 20MHz
      DREAM      - ATMEL DREAM SAM9773 Single Chip Synthesizer/MIDI with Effects and Serial Interface, clock 9.6MHz (TQFP80)
      AD-65      - Oki compatible M6295 sound chip, clock 1MHz
      5493R45    - ISSI 5493R45-001 128k x8 SRAM (SOJ32)
      GM71C18163 - Hynix 1M x16 DRAM (SOJ42)
      VSync      - 60Hz
      HSync      - 15.15kHz

 driver by Pierpaolo Prazzoli

 TODO:
 - eeprom - is it used?
 - irq2 - sound related? reads the 2 unmapped input registers.
 - irq3 - it only writes a 0 into memory and changes a registe
 - simulate music (DREAM chip)

*********************************************************************/

#include "emu.h"
#include "cpu/e132xs/e132xs.h"
#include "machine/eeprom.h"
#include "sound/okim6295.h"

class pasha2_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, pasha2_state(machine)); }

	pasha2_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *     bitmap0;
	UINT16 *     bitmap1;
	UINT16 *     paletteram;
	UINT16 *     wram;

	/* video-related */
	int vbuffer;
	int old_bank;
};


static WRITE16_HANDLER( pasha2_misc_w )
{
	pasha2_state *state = (pasha2_state *)space->machine->driver_data;

	if (offset)
	{
		if (data & 0x0800)
		{
			int bank = data & 0xf000;

			if (bank != state->old_bank)
			{
				state->old_bank = bank;

				switch (bank)
				{
					case 0x8000:
					case 0x9000:
					case 0xa000:
					case 0xb000:
					case 0xc000:
					case 0xd000:
						memory_set_bankptr(space->machine, "bank1", memory_region(space->machine, "user2") + 0x400 * (bank - 0x8000)); break;
				}
			}
		}
	}
}

static WRITE16_HANDLER( pasha2_palette_w )
{
	pasha2_state *state = (pasha2_state *)space->machine->driver_data;
	int color;

	COMBINE_DATA(&state->paletteram[offset]);

	offset &= 0xff;

	color = (state->paletteram[offset] >> 8) | (state->paletteram[offset + 0x100] & 0xff00);
	palette_set_color_rgb(space->machine, offset * 2 + 0, pal5bit(color), pal5bit(color >> 5), pal5bit(color >> 10));

	color = (state->paletteram[offset] & 0xff) | ((state->paletteram[offset + 0x100] & 0xff) << 8);
	palette_set_color_rgb(space->machine, offset * 2 + 1, pal5bit(color), pal5bit(color >> 5), pal5bit(color >> 10));
}

static WRITE16_HANDLER( vbuffer_set_w )
{
	pasha2_state *state = (pasha2_state *)space->machine->driver_data;
	state->vbuffer = 1;
}

static WRITE16_HANDLER( vbuffer_clear_w )
{
	pasha2_state *state = (pasha2_state *)space->machine->driver_data;
	state->vbuffer = 0;
}

static WRITE16_HANDLER( bitmap_0_w )
{
	pasha2_state *state = (pasha2_state *)space->machine->driver_data;
	COMBINE_DATA(&state->bitmap0[offset + state->vbuffer * 0x20000 / 2]);
}

static WRITE16_HANDLER( bitmap_1_w )
{
	pasha2_state *state = (pasha2_state *)space->machine->driver_data;

	// handle overlapping pixels without writing them
	switch (mem_mask)
	{
		case 0xffff:
			bitmap_1_w(space, offset, data, 0xff00);
			bitmap_1_w(space, offset, data, 0x00ff);
			return;

		case 0xff00:
			if ((data & 0xff00) == 0xff00)
				return;
		break;

		case 0x00ff:
			if ((data & 0x00ff) == 0x00ff)
				return;
		break;
	}

	COMBINE_DATA(&state->bitmap1[offset + state->vbuffer * 0x20000 / 2]);
}

static READ16_DEVICE_HANDLER( oki_r )
{
	if (offset)
		return okim6295_r(device, 0);
	else
		return 0;
}

static WRITE16_DEVICE_HANDLER( oki_w )
{
	if (offset)
		okim6295_w(device, 0, data);
}

static WRITE16_DEVICE_HANDLER( oki_bank_w )
{
	if (offset)
		downcast<okim6295_device *>(device)->set_bank_base((data & 1) * 0x40000);
}

static WRITE16_HANDLER( pasha2_lamps_w )
{
	if (data)
		popmessage("1P: %c%c%c 2P: %c%c%c 3P: %c%c%c",
				(data & 0x001) ? 'R' : '-',
				(data & 0x002) ? 'G' : '-',
				(data & 0x004) ? 'B' : '-',
				(data & 0x010) ? 'R' : '-',
				(data & 0x020) ? 'G' : '-',
				(data & 0x040) ? 'B' : '-',
				(data & 0x100) ? 'R' : '-',
				(data & 0x200) ? 'G' : '-',
				(data & 0x400) ? 'B' : '-');
}

static ADDRESS_MAP_START( pasha2_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_BASE_MEMBER(pasha2_state, wram)
	AM_RANGE(0x40000000, 0x4001ffff) AM_RAM_WRITE(bitmap_0_w)
	AM_RANGE(0x40020000, 0x4003ffff) AM_RAM_WRITE(bitmap_1_w)
	AM_RANGE(0x40060000, 0x40060001) AM_WRITENOP
	AM_RANGE(0x40064000, 0x40064001) AM_WRITENOP
	AM_RANGE(0x40068000, 0x40068001) AM_WRITENOP
	AM_RANGE(0x4006c000, 0x4006c001) AM_WRITENOP
	AM_RANGE(0x40070000, 0x40070001) AM_WRITE(vbuffer_clear_w)
	AM_RANGE(0x40074000, 0x40074001) AM_WRITE(vbuffer_set_w)
	AM_RANGE(0x40078000, 0x40078001) AM_WRITENOP //once at startup -> to disable the eeprom?
	AM_RANGE(0x80000000, 0x803fffff) AM_ROMBANK("bank1")
	AM_RANGE(0xe0000000, 0xe00003ff) AM_RAM_WRITE(pasha2_palette_w) AM_BASE_MEMBER(pasha2_state, paletteram) //tilemap? palette?
	AM_RANGE(0xfff80000, 0xffffffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pasha2_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x08, 0x0b) AM_READNOP //sound status?
	AM_RANGE(0x18, 0x1b) AM_READNOP //sound status?
	AM_RANGE(0x20, 0x23) AM_WRITE(pasha2_lamps_w)
	AM_RANGE(0x40, 0x43) AM_READ_PORT("COINS")
	AM_RANGE(0x60, 0x63) AM_READ_PORT("DSW")
	AM_RANGE(0x80, 0x83) AM_READ_PORT("INPUTS")
	AM_RANGE(0xa0, 0xa3) AM_WRITENOP //soundlatch?
	AM_RANGE(0xc0, 0xc3) AM_WRITE(pasha2_misc_w)
	AM_RANGE(0xe0, 0xe3) AM_DEVREADWRITE("oki1", oki_r, oki_w)
	AM_RANGE(0xe4, 0xe7) AM_DEVREADWRITE("oki2", oki_r, oki_w)
	AM_RANGE(0xe8, 0xeb) AM_DEVWRITE("oki1", oki_bank_w)
	AM_RANGE(0xec, 0xef) AM_DEVWRITE("oki2", oki_bank_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( pasha2 )
	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// 2 physical dip-switches
	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0018, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

static VIDEO_START( pasha2 )
{
	pasha2_state *state = (pasha2_state *)machine->driver_data;
	state->bitmap0 = auto_alloc_array(machine, UINT16, 0x40000/2);
	state->bitmap1 = auto_alloc_array(machine, UINT16, 0x40000/2);

	state_save_register_global_pointer(machine, state->bitmap0, 0x40000/2);
	state_save_register_global_pointer(machine, state->bitmap1, 0x40000/2);
}

static VIDEO_UPDATE( pasha2 )
{
	pasha2_state *state = (pasha2_state *)screen->machine->driver_data;
	int x, y, count;
	int color;

	/* 2 512x256 bitmaps */

	count = 0;
	for (y = 0; y <= cliprect->max_y; y++)
	{
		for (x = 0; x < 512 / 2; x++)
		{
			if (x * 2 < cliprect->max_x)
			{
				color = (state->bitmap0[count + (state->vbuffer ^ 1) * 0x20000 / 2] & 0xff00) >> 8;
				*BITMAP_ADDR16(bitmap, y, x * 2 + 0) = color + 0x100;

				color = state->bitmap0[count + (state->vbuffer ^ 1) * 0x20000 / 2] & 0xff;
				*BITMAP_ADDR16(bitmap, y, x * 2 + 1) = color + 0x100;
			}

			count++;
		}
	}

	count = 0;
	for (y = 0; y <= cliprect->max_y; y++)
	{
		for (x = 0; x < 512 / 2; x++)
		{
			if (x * 2 < cliprect->max_x)
			{
				color = state->bitmap1[count + (state->vbuffer ^ 1) * 0x20000 / 2] & 0xff;
				if (color != 0)
					*BITMAP_ADDR16(bitmap, y, x * 2 + 1) = color;

				color = (state->bitmap1[count + (state->vbuffer ^ 1) * 0x20000 / 2] & 0xff00) >> 8;
				if (color != 0)
					*BITMAP_ADDR16(bitmap, y, x * 2 + 0) = color;
			}

			count++;
		}
	}

	return 0;
}

static MACHINE_START( pasha2 )
{
	pasha2_state *state = (pasha2_state *)machine->driver_data;

	state_save_register_global(machine, state->old_bank);
	state_save_register_global(machine, state->vbuffer);
}

static MACHINE_RESET( pasha2 )
{
	pasha2_state *state = (pasha2_state *)machine->driver_data;

	state->old_bank = -1;
	state->vbuffer = 0;
}

static MACHINE_DRIVER_START( pasha2 )

	/* driver data */
	MDRV_DRIVER_DATA(pasha2_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", E116XT, 20000000*4)		/* 4x internal multiplier */
	MDRV_CPU_PROGRAM_MAP(pasha2_map)
	MDRV_CPU_IO_MAP(pasha2_io)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_START(pasha2)
	MDRV_MACHINE_RESET(pasha2)
	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 383, 0, 239)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(pasha2)
	MDRV_VIDEO_UPDATE(pasha2)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki1", 1000000, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_OKIM6295_ADD("oki2", 1000000, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	//and ATMEL DREAM SAM9773
MACHINE_DRIVER_END

ROM_START( pasha2 )
	ROM_REGION16_BE( 0x80000, "user1", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "pp2.u3",       0x00000, 0x80000, CRC(1c701273) SHA1(f465323a1d3f2fd752c51c178fafe4cc866e28d6) )

	ROM_REGION16_BE( 0x400000*6, "user2", ROMREGION_ERASEFF ) /* data roms */
	ROM_LOAD16_BYTE( "pp2-u2.u101",  0x000000, 0x200000, CRC(85c4a2d0) SHA1(452b24b74bd0b65d2d6852486e2917f94e21ecc8) )
	ROM_LOAD16_BYTE( "pp2-u1.u101",  0x000001, 0x200000, CRC(96cbd04e) SHA1(a4e7dd61194584b3c4217674d78ab2fd96b7b2e0) )
	ROM_LOAD16_BYTE( "pp2-u2.u102",  0x400000, 0x200000, CRC(2097d88c) SHA1(7597578e6ddca00909feac35d9d7331f783b2bd6) )
	ROM_LOAD16_BYTE( "pp2-u1.u102",  0x400001, 0x200000, CRC(7a3492fb) SHA1(de72c4d10e17eaf2b7531f637b42cbb3d07819b5) )
	// empty space, but no empty sockets on the pcb

	// not hooked up yet
	ROM_REGION( 0x2000, "cpu1", 0 ) /* AT89C52 */
	ROM_LOAD( "89c52.bin",  0x0000, 0x2000, CRC(9ce43ce4) SHA1(8027a3549b38e9a2e7bb8f518a0defcaf9743371) )

	ROM_REGION( 0x80000, "user3", 0 ) /* SAM9773 sound data */
	ROM_LOAD( "pp2.um2",      0x00000, 0x80000, CRC(86814b37) SHA1(70f8a94410e362669570c39e00492c0d69de6b17) )

	ROM_REGION( 0x80000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "pp2.um51",     0x00000, 0x80000, CRC(3b1b1a30) SHA1(1ea1266d280a2b96ac4ef9fe8ee7b1a5f7861672) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Oki Samples */
	ROM_LOAD( "pp2.um53",     0x00000, 0x80000, CRC(8a29ad03) SHA1(3e9b0c86d8e3bb0b7691f68ad45431f6f9e8edbd) )
ROM_END

static READ16_HANDLER( pasha2_speedup_r )
{
	pasha2_state *state = (pasha2_state *)space->machine->driver_data;

	if(cpu_get_pc(space->cpu) == 0x8302)
		cpu_spinuntil_int(space->cpu);

	return state->wram[(0x95744 / 2) + offset];
}

static DRIVER_INIT( pasha2 )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x95744, 0x95747, 0, 0, pasha2_speedup_r );

	memory_set_bankptr(machine, "bank1", memory_region(machine, "user2"));
}

GAME( 1998, pasha2, 0, pasha2, pasha2, pasha2, ROT0, "Dong Sung", "Pasha Pasha 2", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
