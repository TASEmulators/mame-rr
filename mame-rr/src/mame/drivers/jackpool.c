/*******************************************************************************************

Jackpot Cards / Jackpot Pool (c) 1997 Electronic Projects

driver by David Haywood & Angelo Salese

Notes:
-There's a "(c) 1992 HI-TECH Software..Brisbane, QLD Australia" string in the program roms,
 this is actually the m68k C compiler used for doing this game.

TODO:
-Correct NVRAM emulation (and default eeprom too?), you cannot save settings to the EEPROM
 right now, also remove the patch (it doesn't boot otherwise);
-UART;

*******************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "machine/eeprom.h"


static UINT16 *jackpool_vram;
static UINT8 map_vreg;
static UINT16 *jackpool_io;


static VIDEO_START(jackpool)
{
}

static VIDEO_UPDATE(jackpool)
{
	const gfx_element *gfx = screen->machine->gfx[0];
	int count;// = 0x00000/2;

	int y,x;

	{
		count = map_vreg*(0x4000/2);
		for (y=0;y<32;y++)
		{
			for (x=0;x<64;x++)
			{
				int tile = (jackpool_vram[count+(0x2000/2)] & 0x7fff);
				int attr = (jackpool_vram[count+(0x2000/2)+0x800] & 0x1f00)>>8;

				drawgfx_opaque(bitmap,cliprect,gfx,tile,attr,0,0,x*8,y*8);
				count++;
			}
		}

		count = map_vreg*(0x4000/2);
		for (y=0;y<32;y++)
		{
			for (x=0;x<64;x++)
			{
				int tile = (jackpool_vram[count] & 0x7fff);

				if(tile != 0)
				{
					int attr = (jackpool_vram[count+0x800] & 0x1f00)>>8;
					int t_pen = (jackpool_vram[count+0x800] & 0x1000);

					drawgfx_transpen(bitmap,cliprect,gfx,tile,attr,0,0,x*8,y*8,(t_pen) ? 0 : -1);
				}

				count++;
			}
		}
	}

	return 0;
}

/*Communication ram*/
static READ16_HANDLER( jackpool_ff_r )
{
	return 0xffff;
}

static READ16_HANDLER( jackpool_io_r )
{
	switch(offset*2)
	{
		case 0x00: return input_port_read(space->machine,"COIN1");
		case 0x04: return input_port_read(space->machine,"UNK1");
		case 0x06: return input_port_read(space->machine,"UNK2");
		case 0x08: return input_port_read(space->machine,"SERVICE1");
		case 0x0a: return input_port_read(space->machine,"SERVICE2");//probably not a button, remote?
		case 0x0c: return input_port_read(space->machine,"PAYOUT");
		case 0x0e: return input_port_read(space->machine,"START2");
		case 0x10: return input_port_read(space->machine,"HOLD3");
		case 0x12: return input_port_read(space->machine,"HOLD4");
		case 0x14: return input_port_read(space->machine,"HOLD2");
		case 0x16: return input_port_read(space->machine,"HOLD1");
		case 0x18: return input_port_read(space->machine,"HOLD5");
		case 0x1a: return input_port_read(space->machine,"START1");
		case 0x1c: return input_port_read(space->machine,"BET");
		case 0x1e: return 0xff; //ticket motor
		case 0x20: return 0xff; //hopper motor
    	case 0x2c: return eeprom_read_bit(space->machine->device("eeprom"));
    	case 0x2e: return eeprom_read_bit(space->machine->device("eeprom"));
//      default: printf("R %02x\n",offset*2); break;
	}

//  printf("R %02x\n",offset*2);
	return jackpool_io[offset];
}

static WRITE16_HANDLER( jackpool_io_w )
{
	COMBINE_DATA(&jackpool_io[offset]);

	switch(offset*2)
	{
		case 0x30: /* ---- ---x HOLD3 lamp */  break;
		case 0x32: /* ---- ---x HOLD4 lamp */  break;
		case 0x34: /* ---- ---x HOLD2 lamp */  break;
		case 0x36: /* ---- ---x HOLD1 lamp */  break;
		case 0x38: /* ---- ---x HOLD5 lamp */  break;
		case 0x3a: /* ---- ---x START1 lamp */ break;
		case 0x3c: /* ---- ---x BET lamp */    break;
		case 0x3e: break;
		case 0x40: /* ---- ---x PAYOUT lamp */ break;
		case 0x46: /* ---- ---x coin counter */break;
		case 0x4a: /* ---- ---x Ticket motor */break;
		case 0x4c: /* ---- ---x Hopper motor */break;
		case 0x4e: map_vreg = data & 1;        break;
		case 0x50: eeprom_set_cs_line(space->machine->device("eeprom"), (data & 1) ? CLEAR_LINE : ASSERT_LINE ); break;
		case 0x52: eeprom_set_clock_line(space->machine->device("eeprom"), (data & 1) ? ASSERT_LINE : CLEAR_LINE ); break;
		case 0x54: eeprom_write_bit(space->machine->device("eeprom"), data & 1); break;
//      case 0x5a: eeprom_set_cs_line(space->machine->device("eeprom"), (data & 1) ? CLEAR_LINE : ASSERT_LINE ); break;
//      case 0x5c: eeprom_set_cs_line(space->machine->device("eeprom"), (data & 1) ? CLEAR_LINE : ASSERT_LINE ); break;
		case 0x60: break;
//      default: printf("[%02x] <- %02x W\n",offset*2,data);      break;
	}

	#if 0
	if(offset*2 == 0x54)
	{
		printf("Write bit %02x\n",data);
		eeprom_write_bit(space->machine->device("eeprom"), data & 1);
	}
	if(offset*2 == 0x52)
	{
		printf("Clock bit %02x\n",data);
		eeprom_set_clock_line(space->machine->device("eeprom"), (data & 1) ? ASSERT_LINE : CLEAR_LINE );
	}
	if(offset*2 == 0x50)
	{
		printf("chip select bit %02x\n",data);
		eeprom_set_cs_line(space->machine->device("eeprom"), (data & 1) ? CLEAR_LINE : ASSERT_LINE );
	}
	#endif
}

static ADDRESS_MAP_START( jackpool_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x120000, 0x1200ff) AM_RAM
	AM_RANGE(0x340000, 0x347fff) AM_RAM AM_BASE(&jackpool_vram)
	AM_RANGE(0x348000, 0x34ffff) AM_RAM //<- vram banks 2 & 3?

	AM_RANGE(0x360000, 0x3603ff) AM_RAM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x380000, 0x380061) AM_READWRITE(jackpool_io_r,jackpool_io_w) AM_BASE(&jackpool_io)//AM_READ(jackpool_io_r)

	AM_RANGE(0x800000, 0x80000f) AM_READ(jackpool_ff_r) AM_WRITENOP //UART
	AM_RANGE(0xa00000, 0xa00001) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0x00ff)
ADDRESS_MAP_END


static INPUT_PORTS_START( jackpool )
	PORT_START("COIN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("SERVICE1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("SERVICE2") //toggle this to change game to Jackpot Pool,with different gfxs for cards.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PAYOUT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("START1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_CODE(KEYCODE_1) PORT_NAME("Deal / W-Up")
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("START2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("BET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Cancel / Take")
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("HOLD1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("HOLD2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Low")
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("HOLD3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("HOLD4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / High")
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("HOLD5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
	/* these two both crashes the CPU*/
	PORT_START("UNK1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("UNK2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( jackpool )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout,   0x000, 0x20  ) /* sprites */
GFXDECODE_END


/*irq 2 used for communication stuff.3 is just a rte*/
static INTERRUPT_GEN( jackpool_interrupt )
{
	cpu_set_input_line(device, 1, HOLD_LINE);
}


static MACHINE_DRIVER_START( jackpool )
	MDRV_CPU_ADD("maincpu", M68000, 12000000) // ?
	MDRV_CPU_PROGRAM_MAP(jackpool_mem)
	MDRV_CPU_VBLANK_INT("screen",jackpool_interrupt)  // ?

	MDRV_GFXDECODE(jackpool)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_EEPROM_93C46_ADD("eeprom")

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(jackpool)
	MDRV_VIDEO_UPDATE(jackpool)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


ROM_START( jackpool )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "jpc2", 0x00001, 0x20000,CRC(5aad51ff) SHA1(af504d15c356c241efb6410a5dad09494d693eca) )
	ROM_LOAD16_BYTE( "jpc3", 0x00000, 0x20000,CRC(249c7073) SHA1(e654232d5f454932a108591deacadc9da9fd8055) )

	ROM_REGION( 0x080000, "oki", 0 ) /* Samples */
	ROM_LOAD( "jpc1", 0x00000, 0x40000, CRC(0f1372a1) SHA1(cec8a9bfb03945af4e1e2d2b916b9ded52a8d0bd) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "jpc4", 0x00000, 0x40000,  CRC(b719f138) SHA1(82799cbccab4e39627e48855f6003917602b42c7) )
	ROM_LOAD( "jpc5", 0x40000, 0x40000,  CRC(09661ed9) SHA1(fb298252c95a9040441c12c9d0e9280843d56a0d) )
	ROM_LOAD( "jpc6", 0x80000, 0x40000,  CRC(c3117411) SHA1(8ed044beb1d6ab7ac48595f7d6bf879f1264454a) )
	ROM_LOAD( "jpc7", 0xc0000, 0x40000,  CRC(b1d40623) SHA1(fb76ae6b53474bd4bee19dbce9537da0f2b63ff4) )
ROM_END

static DRIVER_INIT( jackpool )
{
	UINT16 *rom = (UINT16 *)memory_region(machine, "maincpu");

	/* patch NVRAM routine */
	rom[0x9040/2] = 0x6602;
}

GAME( 1997, jackpool, 0, jackpool, jackpool, jackpool, ROT0, "Electronic Projects", "Jackpot Cards / Jackpot Pool (Italy)",GAME_NOT_WORKING )
