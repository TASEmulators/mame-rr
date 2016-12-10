/*

  Merit trivia games

  driver by Pierpaolo Prazzoli

  Games known to be needed:
  - Some other missing categories romsets and new / old revision

  TODO:
  - add flipscreen according to schematics
  - pitboss: dip switches
  - general - add named output notifiers

Notes: it's important that "user1" is 0xa0000 bytes with empty space filled
       with 0xff, because the built-in roms test checks how many question roms
       the games has and the type of each one.
       The  type is stored in one byte in an offset which change for every game,
       using it in a form of protection.

       Rom type byte legend:
       0 -> 0x02000 bytes rom
       1 -> 0x04000 bytes rom
       2 -> 0x08000 bytes rom
       3 -> 0x10000 bytes rom

       ---------------------------------------------------------------------------

       Trivia Whiz ? Horizontal and Vertical can accept 10 question roms, with
       4 categories, but they only use 3 categories.

       ---------------------------------------------------------------------------

       The Couples: Year is uncertain,title screen says "1988"
       PCB marking (set 2) and (set 3) says "230188"
       service mode says "6221-51 U5-0 12/02/86"

       ---------------------------------------------------------------------------
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#define MASTER_CLOCK			(XTAL_10MHz)
#define CPU_CLOCK				(MASTER_CLOCK / 4)
#define PIXEL_CLOCK				(MASTER_CLOCK / 1)
#define CRTC_CLOCK				(MASTER_CLOCK / 8)

#define NUM_PENS				(16)
#define RAM_PALETTE_SIZE		(1024)

static pen_t pens[NUM_PENS];

static UINT8 *ram_attr;
static UINT8 *ram_video;

static UINT8 *ram_palette;

static UINT8 lscnblk;
static int extra_video_bank_bit;

static int question_address;
static int decryption_key;

static UINT8 *backup_ram;

static MACHINE_START(merit)
{
	question_address = 0;
	ram_palette = auto_alloc_array(machine, UINT8, RAM_PALETTE_SIZE);

	state_save_register_global_pointer(machine, ram_palette, RAM_PALETTE_SIZE);
	state_save_register_global(machine, lscnblk);
	state_save_register_global(machine, extra_video_bank_bit);
	state_save_register_global(machine, question_address);
	state_save_register_global(machine, decryption_key);
}


static READ8_HANDLER( questions_r )
{
	UINT8 *questions = memory_region(space->machine, "user1");
	int address;

	switch(question_address >> 16)
	{
		case 0x30: address = 0x00000;
			break;

		case 0x31: address = 0x10000;
			break;

		case 0x32: address = 0x20000;
			break;

		case 0x33: address = 0x30000;
			break;

		case 0x34: address = 0x40000;
			break;

		case 0x35: address = 0x50000;
			break;

		case 0x36: address = 0x60000;
			break;

		case 0x37: address = 0x70000;
			break;

		case 0x28: address = 0x80000;
			break;

		case 0x18: address = 0x90000;
			break;

/* not used? only 10 roms are tested in service mode
 * b0 b1 b2 b3 b4 b5 b6 b7 a8 98
 *      case 0xb0: address = 0xa0000;
 *          break;
 */

		default: logerror("read unknown question rom: %02X\n",question_address >> 16);
			return 0xff;
	}

	address |= question_address & 0xffff;

	return questions[address];
}

static WRITE8_HANDLER( low_offset_w )
{
	offset = (offset & 0xf0) | ((offset - decryption_key) & 0x0f);
	offset = BITSWAP8(offset,7,6,5,4,0,1,2,3);
	question_address = (question_address & 0xffff00) | offset;
}

static WRITE8_HANDLER( med_offset_w )
{
	offset = (offset & 0xf0) | ((offset - decryption_key) & 0x0f);
	offset = BITSWAP8(offset,7,6,5,4,0,1,2,3);
	question_address = (question_address & 0xff00ff) | (offset << 8);
}

static WRITE8_HANDLER( high_offset_w )
{
	offset = BITSWAP8(offset,7,6,5,4,0,1,2,3);
	question_address = (question_address & 0x00ffff) | (offset << 16);
}

static READ8_HANDLER( palette_r )
{
	int co;

	co = ((ram_attr[offset] & 0x7F) << 3) | (offset & 0x07);
	return ram_palette[co];
}

static WRITE8_HANDLER( palette_w )
{
	int co;

	space->machine->primary_screen->update_now();
	data &= 0x0f;

	co = ((ram_attr[offset] & 0x7F) << 3) | (offset & 0x07);
	ram_palette[co] = data;

}


static MC6845_BEGIN_UPDATE( begin_update )
{
	int i;
	int dim, bit0, bit1, bit2;

	for (i=0; i < NUM_PENS; i++)
	{
		dim = BIT(i,3) ? 255 : 127;
		bit0 = BIT(i,0);
		bit1 = BIT(i,1);
		bit2 = BIT(i,2);
		pens[i] = MAKE_RGB(dim*bit0, dim*bit1, dim*bit2);
	}

	return pens;
}


static MC6845_UPDATE_ROW( update_row )
{
	UINT8 cx;

	pen_t *pens = (pen_t *)param;
	UINT8 *gfx[2];
	UINT16 x = 0;
	int rlen;

	gfx[0] = memory_region(device->machine, "gfx1");
	gfx[1] = memory_region(device->machine, "gfx2");
	rlen = memory_region_length(device->machine, "gfx2");

	//ma = ma ^ 0x7ff;
	for (cx = 0; cx < x_count; cx++)
	{
		int i;
		int attr = ram_attr[ma & 0x7ff];
		int region = (attr & 0x40) >> 6;
		int addr = ((ram_video[ma & 0x7ff] | ((attr & 0x80) << 1) | (extra_video_bank_bit)) << 4) | (ra & 0x0f);
		int colour = (attr & 0x7f) << 3;
		UINT8	*data;

		addr &= (rlen-1);
		data = gfx[region];

		for (i = 7; i>=0; i--)
		{
			int col = colour;

			col |= (BIT(data[0x0000 | addr],i)<<2);
			if (region==0)
			{
				col |= (BIT(data[rlen | addr],i)<<1);
				col |= (BIT(data[rlen<<1 | addr],i)<<0);
			}
			else
				col |= 0x03;

			col = ram_palette[col & 0x3ff];
			*BITMAP_ADDR32(bitmap, y, x) = pens[col ? col : (lscnblk ? 8 : 0)];

			x++;
		}
		ma++;
	}
}


static WRITE_LINE_DEVICE_HANDLER(hsync_changed)
{
	/* update any video up to the current scanline */
	device->machine->primary_screen->update_now();
}

static WRITE_LINE_DEVICE_HANDLER(vsync_changed)
{
	cputag_set_input_line(device->machine, "maincpu", 0, state ? ASSERT_LINE : CLEAR_LINE);
}

static const mc6845_interface mc6845_intf =
{
	"screen",					/* screen we are acting on */
	8,							/* number of pixels per video memory address */
	begin_update,				/* before pixel update callback */
	update_row,					/* row update callback */
	NULL,						/* after pixel update callback */
	DEVCB_NULL,					/* callback for display state changes */
	DEVCB_NULL,					/* callback for cursor state changes */
	DEVCB_LINE(hsync_changed),	/* HSYNC callback */
	DEVCB_LINE(vsync_changed),	/* VSYNC callback */
	NULL						/* update address callback */
};

static WRITE8_DEVICE_HANDLER( led1_w )
{
	/* 5 button lamps player 1 */
	set_led_status(device->machine, 0,~data & 0x01);
	set_led_status(device->machine, 1,~data & 0x02);
	set_led_status(device->machine, 2,~data & 0x04);
	set_led_status(device->machine, 3,~data & 0x08);
	set_led_status(device->machine, 4,~data & 0x10);
}

static WRITE8_DEVICE_HANDLER( led2_w )
{
	/* 5 button lamps player 2 */
	set_led_status(device->machine, 5,~data & 0x01);
	set_led_status(device->machine, 6,~data & 0x02);
	set_led_status(device->machine, 7,~data & 0x04);
	set_led_status(device->machine, 8,~data & 0x08);
	set_led_status(device->machine, 9,~data & 0x10);

	/* coin counter */
	coin_counter_w(device->machine,0,0x80-(data & 0x80));
}

static WRITE8_DEVICE_HANDLER( misc_w )
{
	flip_screen_set(device->machine, ~data & 0x10);
	extra_video_bank_bit = (data & 2) << 8;
	lscnblk = (data >> 3) & 1;

	/* other bits unknown */
}

static WRITE8_DEVICE_HANDLER( misc_couple_w )
{
	flip_screen_set(device->machine, ~data & 0x10);
	extra_video_bank_bit = (data & 2) << 8;
	lscnblk = (data >> 3) & 1;

	/* other bits unknown */

	/*kludge to avoid jumps on ram area in The Couples*/
	backup_ram[0x1011] = 0xc9; //ret
}

static WRITE8_HANDLER(casino5_bank_w)
{
	if ( data == 0 )
	{
		memory_set_bank(space->machine, "bank1", 1);
		memory_set_bank(space->machine, "bank2", 1);
	}
	else if ( data == 0xff )
	{
		memory_set_bank(space->machine, "bank1", 0);
		memory_set_bank(space->machine, "bank2", 0);
	}
	else
	{
		logerror( "Uknown banking write %02x\n", data );
	}
}

static CUSTOM_INPUT(rndbit_r)
{
	return mame_rand(field->port->machine);
}

static ADDRESS_MAP_START( pitboss_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0xa000, 0xa003) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xc000, 0xc003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0xe001, 0xe001) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_BASE(&ram_attr)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_BASE(&ram_video)
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( casino5_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank2")
	AM_RANGE(0x6000, 0x6fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x7000, 0x7000) AM_WRITE(casino5_bank_w)
	AM_RANGE(0x7001, 0x7fff) AM_RAM
	AM_RANGE(0xa000, 0xa003) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xc000, 0xc003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0xe001, 0xe001) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_BASE(&ram_attr)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_BASE(&ram_video)
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bigappg_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0xc004, 0xc007) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xc008, 0xc00b) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0xe001, 0xe001) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_BASE(&ram_attr)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_BASE(&ram_video)
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dodge_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0xc004, 0xc007) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xc008, 0xc00b) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0xe001, 0xe001) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_BASE(&ram_attr)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_BASE(&ram_video)
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

/* Address decoding is done by prom u13 on crt200a hardware. It decodes
 * the following addr lines: 2,3,9,13,14,15 ==> E20C
 * ==> mirror 1DF3 & ~effective_addr_lines
 * */

static ADDRESS_MAP_START( trvwhiz_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4c00, 0x4cff) AM_READWRITE(questions_r, high_offset_w)
	AM_RANGE(0x5400, 0x54ff) AM_WRITE(low_offset_w)
	AM_RANGE(0x5800, 0x58ff) AM_WRITE(med_offset_w)
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0xa000, 0xa003) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xc000, 0xc003) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0xe001, 0xe001) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_BASE(&ram_attr)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_BASE(&ram_video)
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( trvwhiz_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x8000, 0x8000) AM_DEVWRITE("aysnd", ay8910_address_w)
	AM_RANGE(0x8100, 0x8100) AM_DEVWRITE("aysnd", ay8910_data_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( phrcraze_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_RAM
	AM_RANGE(0xc008, 0xc00b) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xc00c, 0xc00f) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xce00, 0xceff) AM_READWRITE(questions_r, high_offset_w)
	AM_RANGE(0xd600, 0xd6ff) AM_WRITE(low_offset_w)
	AM_RANGE(0xda00, 0xdaff) AM_WRITE(med_offset_w)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0xe001, 0xe001) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_BASE(&ram_attr)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_BASE(&ram_video)
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( phrcraze_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0xc004, 0xc004) AM_MIRROR(0x1cf3) AM_DEVWRITE("aysnd", ay8910_address_w)
	AM_RANGE(0xc104, 0xc104) AM_MIRROR(0x1cf3) AM_DEVWRITE("aysnd", ay8910_data_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( tictac_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xc004, 0xc007) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xc008, 0xc00b) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xce00, 0xceff) AM_READWRITE(questions_r, high_offset_w)
	AM_RANGE(0xd600, 0xd6ff) AM_WRITE(low_offset_w)
	AM_RANGE(0xda00, 0xdaff) AM_WRITE(med_offset_w)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0xe001, 0xe001) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_BASE(&ram_attr)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_BASE(&ram_video)
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tictac_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0xc00c, 0xc00c) AM_MIRROR(0x1cf3) AM_DEVWRITE("aysnd", ay8910_address_w)
	AM_RANGE(0xc10c, 0xc10c) AM_MIRROR(0x1cf3) AM_DEVWRITE("aysnd", ay8910_data_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( trvwhziv_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_RAM
	AM_RANGE(0xc004, 0xc007) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xc008, 0xc00b) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xce00, 0xceff) AM_READWRITE(questions_r, high_offset_w)
	AM_RANGE(0xd600, 0xd6ff) AM_WRITE(low_offset_w)
	AM_RANGE(0xda00, 0xdaff) AM_WRITE(med_offset_w)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0xe001, 0xe001) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_BASE(&ram_attr)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_BASE(&ram_video)
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( couple_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xbfff) AM_RAM AM_BASE(&backup_ram)
	AM_RANGE(0xc004, 0xc007) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xc008, 0xc00b) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0xe001, 0xe001) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_BASE(&ram_attr)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_BASE(&ram_video)
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END


/* in service mode:
 * keep service test button pressed to clear the coin counter.
 * keep it pressed for 10 seconds to clear all the memory.
 * to enter hidden test mode enable "Enable Test Mode", enable "Reset High Scores"
 */
static INPUT_PORTS_START( phrcraze )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x80, 0x80, "Enable Test Mode" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, "Reset High Scores" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPNAME( 0x04, 0x04, "Topic \"8\"" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x00, "Upright 2 Players" )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
	/*  PORT_DIPSETTING(    0x40, "Upright 1 Player" ) */
INPUT_PORTS_END

/* in service mode:
 * keep service test button pressed to clear the coin counter.
 * keep it pressed for 10 seconds to clear all the memory.
 * to enter hidden test mode enable "Enable Test Mode", enable "Reset High Scores"
 */
static INPUT_PORTS_START( phrcrazs )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x80, 0x80, "Enable Test Mode" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, "Reset High Scores" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x04, "XXX-Rated Sex Topic" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, "Bonus Phraze" )
	PORT_DIPSETTING(    0x18, DEF_STR( None ) )
	PORT_DIPSETTING(    0x10, "800K" )
	PORT_DIPSETTING(    0x08, "1M" )
	PORT_DIPSETTING(    0x00, "1.5M" )
	PORT_DIPNAME( 0x20, 0x20, "Random Sex Category" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x00, "Upright 2 Players" )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
/* PORT_DIPSETTING(    0x40, "Upright 1 Player" ) */
INPUT_PORTS_END

/* keep service test button pressed to clear the coin counter.
 * To enter hidden test-mode in service mode:
 * enable "Reset High Scores" then press "Service Mode"
 */
static INPUT_PORTS_START( tictac )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, "Reset High Scores" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Lightning Round 1 Credit" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x00, "Upright 2 Players" )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
/*  PORT_DIPSETTING(    0x40, "Upright 1 Player" ) */
INPUT_PORTS_END

static INPUT_PORTS_START( trivia )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, "Reset High Scores" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "On 0 Points" )
	PORT_DIPSETTING(    0x04, "Continue" )
	PORT_DIPSETTING(    0x00, "Game Over" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* in service mode:
 * keep service test button pressed to clear the coin counter.
 * keep it pressed for 10 seconds to clear all the memory.
 * to enter hidden test mode enable "Enable Test Mode", enable "Reset High Scores"
*/
static INPUT_PORTS_START( trvwhziv )
	PORT_INCLUDE( trivia )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	/* no coinage DSW */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pitboss )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1/P2 Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1/P2 Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1/P2 Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1/P2 Button 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1/P2 Button 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1/P2 Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_A) PORT_NAME("P1/P2 Cancel")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) //causes "unauthorized conversion" msg.

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, "0-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "0-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "0-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "0-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "0-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/*probably unused*/
	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "1-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "1-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "DSW" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( casino5 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Play")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) // putting this on makes the horse racing game to not work
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, "Reset High Scores" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* 1 displays additional screens in attract mode - custom ads? */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )


	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* 0 causes "Unathorized conversion" */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Draw Poker enabled" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Black Jack enabled" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Dice Game enabled" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Foto Finish enabled" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Acey Deucey enabled" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( bigappg )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Play")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, "Reset High Scores" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )


	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(rndbit_r, NULL)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Draw Poker enabled" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Black Jack enabled" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Dice Game enabled" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Foto Finish enabled" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Acey Deucey enabled" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( couple )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Number of Attempts" )
	PORT_DIPSETTING(    0x01, "99" )
	PORT_DIPSETTING(    0x00, "9" )
	PORT_DIPNAME( 0x02, 0x02, "Tries Per Coin" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
/*2 Coins for 2 Credits?I think this is an invalid setting,it doesn't even work correctly*/
/*  PORT_DIPSETTING(    0x00, DEF_STR( 2C_2C ) ) */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x00, "Sound" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Clear RAM" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/*Different DSWs*/
static INPUT_PORTS_START( couplep )
	PORT_INCLUDE( couple )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x40, 0x40, "Bonus Play" )
	PORT_DIPSETTING(    0x40, "at 150.000" )
	PORT_DIPSETTING(    0x00, "at 200.000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static VIDEO_UPDATE( merit )
{
	running_device *mc6845 = screen->machine->device("crtc");
	mc6845_update(mc6845, bitmap, cliprect);

	return 0;
}

static const ppi8255_interface ppi8255_intf[2] =
{
	{
		DEVCB_INPUT_PORT("IN0"),		/* Port A read */
		DEVCB_INPUT_PORT("IN1"),		/* Port B read */
		DEVCB_INPUT_PORT("IN2"),		/* Port C read */
		DEVCB_NULL,						/* Port A write */
		DEVCB_NULL,						/* Port B write */
		DEVCB_NULL						/* Port C write */
	},
	{
		DEVCB_INPUT_PORT("DSW"),		/* Port A read */
		DEVCB_NULL,						/* Port B read */
		DEVCB_NULL,						/* Port C read */
		DEVCB_NULL,						/* Port A write */
		DEVCB_HANDLER(led1_w),			/* Port B write */
		DEVCB_HANDLER(misc_w)			/* Port C write */
	}
};

static const ppi8255_interface ppi8255_couple_intf[2] =
{
	{
		DEVCB_INPUT_PORT("IN0"),		/* Port A read */
		DEVCB_INPUT_PORT("IN1"),		/* Port B read */
		DEVCB_INPUT_PORT("IN2"),		/* Port C read */
		DEVCB_NULL,						/* Port A write */
		DEVCB_NULL,						/* Port B write */
		DEVCB_NULL						/* Port C write */
	},
	{
		DEVCB_INPUT_PORT("DSW"),		/* Port A read */
		DEVCB_NULL,						/* Port B read */
		DEVCB_NULL,						/* Port C read */
		DEVCB_NULL,						/* Port A write */
		DEVCB_HANDLER(led1_w),			/* Port B write */
		DEVCB_HANDLER(misc_couple_w)	/* Port C write */
	}
};

static const ay8910_interface merit_ay8912_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL, DEVCB_NULL,
	DEVCB_HANDLER(led2_w), DEVCB_NULL
};

static NVRAM_HANDLER(dodge)
{
	if (read_or_write)
	{
		mame_fwrite(file, machine->generic.nvram.v, machine->generic.nvram_size);
	}
	else if (file)
	{
		mame_fread(file, machine->generic.nvram.v, machine->generic.nvram_size);
	}
	else
	{
		memset(machine->generic.nvram.v, 0x00, machine->generic.nvram_size);
		machine->generic.nvram.u8[0x1040] = 0xc9; /* ret */
	}
}

static MACHINE_START(casino5)
{
	MACHINE_START_CALL(merit);
	memory_configure_bank(machine, "bank1", 0, 2, memory_region(machine, "maincpu") + 0x2000, 0x2000);
	memory_configure_bank(machine, "bank2", 0, 2, memory_region(machine, "maincpu") + 0x6000, 0x2000);
	memory_set_bank(machine, "bank1", 0);
	memory_set_bank(machine, "bank2", 0);
}

static MACHINE_DRIVER_START( pitboss )
	MDRV_CPU_ADD("maincpu",Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(pitboss_map)
	MDRV_CPU_IO_MAP(trvwhiz_io_map)

	MDRV_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ppi8255_intf[1] )

	MDRV_MACHINE_START(merit)
	/* video hardware */

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 512, 0, 512, 256, 0, 256)	/* temporary, CRTC will configure screen */

	MDRV_MC6845_ADD("crtc", MC6845, CRTC_CLOCK, mc6845_intf)

	MDRV_VIDEO_UPDATE(merit)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, CRTC_CLOCK)
	MDRV_SOUND_CONFIG(merit_ay8912_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( casino5 )
	MDRV_IMPORT_FROM(pitboss)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(casino5_map)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_MACHINE_START(casino5)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( bigappg )
	MDRV_IMPORT_FROM(pitboss)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(bigappg_map)
	MDRV_CPU_IO_MAP(tictac_io_map)

	MDRV_NVRAM_HANDLER(generic_0fill)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( dodge )
	MDRV_IMPORT_FROM(pitboss)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(dodge_map)

	MDRV_NVRAM_HANDLER(dodge)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( tictac )
	MDRV_IMPORT_FROM(pitboss)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(tictac_map)
	MDRV_CPU_IO_MAP(tictac_io_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( trvwhiz )
	MDRV_IMPORT_FROM(pitboss)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(trvwhiz_map)
	MDRV_CPU_IO_MAP(trvwhiz_io_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( phrcraze )
	MDRV_IMPORT_FROM(pitboss)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(phrcraze_map)
	MDRV_CPU_IO_MAP(phrcraze_io_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( trvwhziv )
	MDRV_IMPORT_FROM(pitboss)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(trvwhziv_map)
	MDRV_CPU_IO_MAP(tictac_io_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( couple )
	MDRV_IMPORT_FROM(pitboss)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(couple_map)
	MDRV_CPU_IO_MAP(tictac_io_map)

	MDRV_DEVICE_MODIFY("ppi8255_0")
	MDRV_DEVICE_CONFIG( ppi8255_couple_intf[0])
	MDRV_DEVICE_MODIFY("ppi8255_1")
	MDRV_DEVICE_CONFIG( ppi8255_couple_intf[1])
MACHINE_DRIVER_END


ROM_START( pitboss )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5-0c.rom", 0x0000, 0x2000, CRC(d8902656) SHA1(06da829201f6141a6b23afa0e277a3c7a122c26e) ) /* Games included in this set are: */
	ROM_LOAD( "u6-0.rom",  0x2000, 0x2000, CRC(bf903b01) SHA1(1f5f69cfd3eb105bd9bad071016931a79defa16b) ) /* Joker Poker, Blackjack, Super Slots & The Dice Game */
	ROM_LOAD( "u7-0.rom",  0x4000, 0x2000, CRC(306351b9) SHA1(32cd243aa65571ee7fc72971b6a16beeb4ed9d85) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) /* Shows: */
	ROM_LOAD( "chr7.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) /* (c) 1983 Merit industries */
	ROM_LOAD( "chr7.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) ) /* Cheltenham PA. 19012      */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7.u40",   0x0000, 0x2000, CRC(52298162) SHA1(79aa6c4ab6bec6450d882615e64f61cfef934153) )
ROM_END

ROM_START( pitbossa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2214-03.u5-0c", 0x0000, 0x2000, CRC(97f870bd) SHA1(b1b01abff0385e3b0585e49f78b93bcf56e434ef) ) /* Games included in this set are: */
	ROM_LOAD( "2214-03.u6-0",  0x2000, 0x2000, CRC(086e699b) SHA1(a1d1eafaac9262f924f175961aa52c6d8e779bf0) ) /* Joker Poker, Blackjack, Foto Finish & The Dice Game */
	ROM_LOAD( "2214-03.u7-0",  0x4000, 0x2000, CRC(023e8cb8) SHA1(cdb180a94d801137466c13ddfaf65918cb608c5a) ) /* Roms also found labeled simply as "PBHD" U5 through U7 */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) /* Shows: */
	ROM_LOAD( "chr7.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) /* (c) 1983 Merit industries */
	ROM_LOAD( "chr7.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) ) /* Cheltenham PA. 19012      */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7.u40",   0x0000, 0x2000, CRC(52298162) SHA1(79aa6c4ab6bec6450d882615e64f61cfef934153) )
ROM_END

ROM_START( pitbossb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m4a1.u5",   0x0000, 0x2000, CRC(f5284472) SHA1(9170b90d06caa382be29feb2f6e80993bba1e07e) ) /* Games included in this set are: */
	ROM_LOAD( "m4a1.u6",   0x2000, 0x2000, CRC(dd8df5fe) SHA1(dab8c1077058263729b2589dd9bf9989ad53be1c) ) /* Draw Poker, Blackjack, Acey Deucey & The Dice Game */
	ROM_LOAD( "m4a1.u7",   0x4000, 0x2000, CRC(5fa5d436) SHA1(9f3fd81eae7f378268f3b4af8fd299ffb97d7fb6) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr2.u39",  0x0000, 0x2000, CRC(f9613e7b) SHA1(1e8cafe142a235d65b43c7e46a79ed4f6272b61c) ) /* Shows: */
	ROM_LOAD( "chr2.u38",  0x2000, 0x2000, CRC(7af28902) SHA1(04f685389958d581aaf2c86940d1b8b8cec05d7a) ) /* (c) 1983 Merit industries   Phila. PA. */
	ROM_LOAD( "chr2.u37",  0x4000, 0x2000, CRC(ea6f0c59) SHA1(f2c0ff99518c2cec3eb1b4042fa3754a702c0e34) ) /* All Rights Reserverd                   */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr2.u40",  0x0000, 0x2000, CRC(40c94dce) SHA1(86611e3a1048b2a3fffcc0110811656a2d0fc4a5) )
ROM_END

ROM_START( pitbossc ) /* Program roms on a CTR-202 daughter card */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2214-04.u5-0",   0x0000, 0x2000, CRC(10b782e7) SHA1(158819898ad81506c47b76ffe2a949ee7208740f) ) /* Games included in this set are: */
	ROM_LOAD( "2214-04.u6-0",   0x2000, 0x2000, CRC(c3fd6510) SHA1(8c89fd2cbcb6f12fa6427883700971f7c39f6ccf) ) /* Joker Poker, Blackjack, Foto Finish & The Dice Game */
	ROM_RELOAD( 0x4000, 0x2000 )
	ROM_LOAD( "2214-04.u7-0",   0x6000, 0x4000, CRC(c5cf7060) SHA1(4a3209ad24ae649348b0e0470fc446d37b667975) ) /* 27128 eprom */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) /* Shows: */
	ROM_LOAD( "chr7.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) /* (c) 1983 Merit industries */
	ROM_LOAD( "chr7.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7.rom.u40",  0x0000, 0x2000, CRC(db62c5ec) SHA1(a9967eb51436f342902fa3ce9c43d4d1ec5e0f3c) )
ROM_END

ROM_START( casino5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5", 0x0000, 0x2000, CRC(abe240d8) SHA1(296eb3251dd51147d6984a8c08c3be22e5ed8e86) )
	ROM_LOAD( "u6", 0x2000, 0x4000, CRC(4d9f0c57) SHA1(d19b4b4f42d329ea35907d17c15a55b954b07295) )
	ROM_LOAD( "u7", 0x6000, 0x4000, CRC(d3bc510d) SHA1(6222badabf629dd6334591867596f811883aed52) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) )
	ROM_LOAD( "chr7.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) )
	ROM_LOAD( "chr7.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "u40", 0x0000, 0x2000, CRC(b13a3fb1) SHA1(25760aa27c88b8be248a87df724bf8797d179e7a) )
ROM_END

ROM_START( bigappg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-13_u5-0.u5", 0x0000, 0x8000, CRC(47bad6fd) SHA1(87f6c603b52e184f82179869d7b58453cbd34814) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "haip_u39.u39", 0x0000, 0x2000, CRC(506e20b4) SHA1(cd28d4f56f83ed3f7cfa44280975c0a6af474707) )
	ROM_LOAD( "haip_u38.u38", 0x2000, 0x2000, CRC(1924feeb) SHA1(c4656a04e2284a265554ea9774d0201c44864809) )
	ROM_LOAD( "haip_u37.u37", 0x4000, 0x2000, CRC(12a04867) SHA1(8e3a6050d854ccc906ae473a104a16d610e4f1da) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "haip_u40.u40", 0x0000, 0x2000, CRC(ac4983b8) SHA1(a552a15f813c331de67eaae2ed42cc037b26c5bd) )
ROM_END

ROM_START( trvwzh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trivia.u5",    0x0000, 0x2000, CRC(731fd5b1) SHA1(1074780321029446da0e6765b9e036b06b067a48) ) /* Program doesn't seem to be able to use all the question roms? */
	ROM_LOAD( "trivia.u6",    0x2000, 0x2000, CRC(af6886c0) SHA1(48005b921d7ce33ffc0ba160be82053a26382a9d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40.u40", 0x0000, 0x2000, CRC(cea7319f) SHA1(663cd18a4699dfd5ad1d3357094eff247e9b4a47) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "ent-001_01a",  0x08000, 0x8000, CRC(ff45d92b) SHA1(10356bc6a04b2c53ecaf76cb0cba3ec70b4ba612) ) /* This set verified as all found on the same question board */
	ROM_LOAD( "ent-001_02a",  0x18000, 0x8000, CRC(902e26f7) SHA1(f13b816bfc507fb429fb3f44531de346a82c780d) )
	ROM_LOAD( "gen-001_01a",  0x28000, 0x8000, CRC(1d8d353f) SHA1(6bd0cc5c67da81a48737f32bc49cbf235648c4c6) )
	ROM_LOAD( "gen-001_02a",  0x3c000, 0x4000, CRC(2000e3c3) SHA1(21737fde3d1a1b22da4590476e4e52ee1bab026f) ) /* 27128 eprom, others are 27256 */
	ROM_LOAD( "spo-001_01a",  0x48000, 0x8000, CRC(ae111429) SHA1(ff551d7ac7ad367338e908805aeb78c59a747919) )
	ROM_LOAD( "spo-001_02a",  0x58000, 0x8000, CRC(ee9263b3) SHA1(1644ab01f17e3af1e193e509d64dcbb243d3eb80) )
	ROM_LOAD( "spo-001_03a",  0x68000, 0x8000, CRC(64181d34) SHA1(f84e28fc589b86ca6a596815871ed26602bcc095) )
	ROM_LOAD( "sex-001_01a",  0x78000, 0x8000, CRC(32519098) SHA1(d070e02bb10e04964893903599a69a8943f9ac8a) )
	ROM_LOAD( "sex-001_02a",  0x88000, 0x8000, CRC(0be4ef9a) SHA1(c80080f1c853e1043bf7e47bea322540a8ac9195) )
ROM_END

ROM_START( trvwzha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trivia.u5",    0x0000, 0x2000, CRC(731fd5b1) SHA1(1074780321029446da0e6765b9e036b06b067a48) )
	ROM_LOAD( "trivia.u6",    0x2000, 0x2000, CRC(af6886c0) SHA1(48005b921d7ce33ffc0ba160be82053a26382a9d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40.u40", 0x0000, 0x2000, CRC(cea7319f) SHA1(663cd18a4699dfd5ad1d3357094eff247e9b4a47) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "spo-001_01a",  0x08000, 0x8000, CRC(ae111429) SHA1(ff551d7ac7ad367338e908805aeb78c59a747919) )
	ROM_LOAD( "spo-001_02a",  0x18000, 0x8000, CRC(ee9263b3) SHA1(1644ab01f17e3af1e193e509d64dcbb243d3eb80) )
	ROM_LOAD( "spo-001_03a",  0x28000, 0x8000, CRC(64181d34) SHA1(f84e28fc589b86ca6a596815871ed26602bcc095) )
ROM_END

/* question board only - this contained a variety of roms from the 'trvwzh' and 'trvwzha' sets as well as 2 unique general knowledge ones */
ROM_START( trvwzhb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trivia.u5",    0x0000, 0x2000, CRC(731fd5b1) SHA1(1074780321029446da0e6765b9e036b06b067a48) )
	ROM_LOAD( "trivia.u6",    0x2000, 0x2000, CRC(af6886c0) SHA1(48005b921d7ce33ffc0ba160be82053a26382a9d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40.u40",   0x0000, 0x2000, CRC(cea7319f) SHA1(663cd18a4699dfd5ad1d3357094eff247e9b4a47) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "merit2_6.1",   0x08000, 0x8000, CRC(8a4bcde3) SHA1(528ae9d3ff0b98201f89fd6b93a712cd7f0e9ab4) ) // 0xxxxxxxxxxxxxx = 0x00
	ROM_LOAD( "merit2_6.2",   0x18000, 0x8000, CRC(ded7e124) SHA1(7e6e04ae79dceba70d83ccfde4f9d0ccc0737c78) )
ROM_END

ROM_START( trvwzv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5.bin", 0x0000, 0x2000, CRC(fd3531ac) SHA1(d11573df65676e704b28cc2d99fb004b48a358a4) )
	ROM_LOAD( "u6.bin", 0x2000, 0x2000, CRC(29e43d0e) SHA1(ad610748fe37436880648078f5d1a305cb147c5d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40.u40", 0x0000, 0x2000, CRC(1f0ff6e0) SHA1(5a31afde34aeb6f851389d093bb426e5cfdedbf2) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "ent-001_01",  0x08000, 0x8000, CRC(c68ce954) SHA1(bf70fe64f095d5cfcf5347d83651b78c6c8bf05f) )
	ROM_LOAD( "ent-001_02",  0x18000, 0x8000, CRC(aac4ff63) SHA1(d68c4408b4dad976e317a33f2a4eaee39d90dbed) )
	ROM_LOAD( "gen-001_01",  0x28000, 0x8000, CRC(5deb1900) SHA1(b7e9407c37481ef8953e8283d45949d951302e92) )
	ROM_LOAD( "gen-001_02",  0x3c000, 0x4000, CRC(d2b53b6a) SHA1(f75334e47885086e277682daf018818a02ce1026) ) /* 27128 eprom, others are 27256 */
	ROM_LOAD( "sex-001_01a", 0x48000, 0x8000, CRC(32519098) SHA1(d070e02bb10e04964893903599a69a8943f9ac8a) )
	ROM_LOAD( "sex-001_02a", 0x58000, 0x8000, CRC(0be4ef9a) SHA1(c80080f1c853e1043bf7e47bea322540a8ac9195) )
ROM_END

ROM_START( trvwzva )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5.bin", 0x0000, 0x2000, CRC(fd3531ac) SHA1(d11573df65676e704b28cc2d99fb004b48a358a4) )
	ROM_LOAD( "u6.bin", 0x2000, 0x2000, CRC(29e43d0e) SHA1(ad610748fe37436880648078f5d1a305cb147c5d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40.u40", 0x0000, 0x2000, CRC(1f0ff6e0) SHA1(5a31afde34aeb6f851389d093bb426e5cfdedbf2) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "spo-001_01", 0x08000, 0x8000, CRC(7b56315d) SHA1(4c8c63b80176bfac9594958a7043627012baada3) )
	ROM_LOAD( "spo-001_02", 0x18000, 0x8000, CRC(148b63ee) SHA1(9f3b222d979f23b313f379cbc06cc00d88d08c56) )
	ROM_LOAD( "spo-001_03", 0x28000, 0x8000, CRC(a6af8e41) SHA1(64f672bfa5fb2c0575103614986e53e238c5984f) )
ROM_END

ROM_START( trvwz2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-05_u5-0e.u5", 0x0000, 0x2000, CRC(97b8d320) SHA1(573945531113d8aae9418ba1e9a2063052227029) )
	ROM_LOAD( "6221-05_u6-0e.u6", 0x2000, 0x2000, CRC(2e86288d) SHA1(62c7024d8dfebed9bb05ea91302efe5d18cb7d2a) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "ent-101.01a",  0x08000, 0x8000, CRC(3825ac47) SHA1(d0da047c4d30a26f496b3663cfda77c229279be8) ) /* This set verified as all found on the same question board */
	ROM_LOAD( "ent-101.02a",  0x18000, 0x8000, CRC(a0153407) SHA1(e669957a5d4775bfa2c16960a2a909a3505c078b) )
	ROM_LOAD( "ent-101.03a",  0x28000, 0x8000, CRC(755b16ab) SHA1(277ea4110479ecdb2c772299ea04f4918cf7f561) )
	ROM_LOAD( "gen-101.01a",  0x38000, 0x8000, CRC(74d14039) SHA1(54b85581d60fb535d37a051f375e687a933600ea) )
	ROM_LOAD( "gen-101.02a",  0x48000, 0x8000, CRC(b1b930d8) SHA1(57be3ee1c0adcb549088818dc7efda64508b5647) ) /* These question roms have been found with the "trvwz3ha"    */
	ROM_LOAD( "spo-101.01a",  0x58000, 0x8000, CRC(9dc4ba98) SHA1(4ce2bbbd7436a0ba8140879d5d8614bddbd5a8ec) ) /* program & CHAR roms and might actually be an Edition 3 set */
	ROM_LOAD( "spo-101.02a",  0x68000, 0x8000, CRC(9c106ad9) SHA1(1d1a5c91152283e3937a2df17cd57b8fe04072b7) )
	ROM_LOAD( "spo-101.03a",  0x78000, 0x8000, CRC(3d69c3a3) SHA1(9f16d45660f3cb15e44e9fc0d940a7b2b12819e8) )
	ROM_LOAD( "sex-101.01a",  0x88000, 0x8000, CRC(301d65c2) SHA1(48d260077e9c9ed82f6dfa176b1103723dc9e19a) )
	ROM_LOAD( "sex-101.02a",  0x98000, 0x8000, CRC(2596091b) SHA1(7fbb9c2c3f74e12714513928c8cf3769bf29fc8b) )

	ROM_REGION( 0x0100, "prom", 0 )
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) /* N82S129N BPROM found on question board, unknown use */
ROM_END

/* questions only - this dump contained a variety of questions from the 'trvwz2' ones above, and 2 unique roms, which are Sex Trivia */
ROM_START( trvwz2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-05_u5-0e.u5", 0x0000, 0x2000, CRC(97b8d320) SHA1(573945531113d8aae9418ba1e9a2063052227029) )
	ROM_LOAD( "6221-05_u6-0e.u6", 0x2000, 0x2000, CRC(2e86288d) SHA1(62c7024d8dfebed9bb05ea91302efe5d18cb7d2a) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "merit2_4.0",  0x08000, 0x8000, CRC(069b59a3) SHA1(e3d75edd3a9271df73bf51f409f066547025abbe) ) // Sex Trivia
	ROM_LOAD( "merit2_4.1",  0x18000, 0x8000, CRC(938d319f) SHA1(5b5841692666c31f2c09cb318f7e106942fffea7) ) // Sex Trivia
ROM_END

ROM_START( trvwz3h )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5", 0x0000, 0x2000, CRC(ad4ab519) SHA1(80e99f4e5542115e34074b41bbc69906e01a408f) )
	ROM_LOAD( "u6", 0x2000, 0x2000, CRC(21a44014) SHA1(331f8b4fa3f837de070b68b959c818122aedc68a) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40b.u40", 0x0000, 0x2000, CRC(e829473f) SHA1(ba754d9377d955b409970494e1a14dbe1d359ee5) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "spo-002_01", 0x08000, 0x8000, CRC(974dca96) SHA1(eb4a745c84307a1bbb220659877f97c28cd515ac) )
	ROM_LOAD( "spo-002_02", 0x18000, 0x8000, CRC(e15ef8d0) SHA1(51c946311ffe507aa9031044bc34e5ae8d3473ab) )
	ROM_LOAD( "spo-002_03", 0x28000, 0x8000, CRC(503115a1) SHA1(5e6630191465b3d2a590fab08b4f47f7408ecc44) )
	ROM_LOAD( "ent-002_01", 0x38000, 0x8000, CRC(0e4fe73d) SHA1(9aee22a5837637ec5e360b72e71555942df1d26f) )
	ROM_LOAD( "ent-002_02", 0x48000, 0x8000, CRC(f56c0935) SHA1(8e16133ad90829bbc0e0f2e9ee9c26e9d0c5057e) )
	ROM_LOAD( "ent-002_03", 0x58000, 0x8000, CRC(057f6676) SHA1(a93a7a76fc8b8263568a50b00a57f3abe76c9aa3) )
	ROM_LOAD( "gen-002_01", 0x68000, 0x8000, CRC(1fa46b86) SHA1(16d54d0932fe342399faf303eafa3c0b7ba2e202) )
	ROM_LOAD( "gen-002_02", 0x78000, 0x8000, CRC(b395cd97) SHA1(a42c7c1687eaba64a725888cd6413568cc90b010) )
	ROM_LOAD( "sex-001_02", 0x88000, 0x8000, CRC(b064876b) SHA1(588300fb6603f334de41a9685b1fcf8c642b5c16) ) /* Does this one actually belong here? */

	ROM_REGION( 0x0100, "prom", 0 )
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) /* N82S129N BPROM found on question board, unknown use */

	ROM_REGION( 0x8000, "misc", 0 )
	ROM_LOAD( "dec002.u13", 0x00000, 0x01f3, CRC(686d2ad0) SHA1(7aad0a1ed09942528eceaf4d7a5e1fd7601aeac7) ) /* PAL10L8CN */
	ROM_LOAD( "sex-001_01", 0x00000, 0x8000, CRC(77a2a734) SHA1(7ba662d275b7914c9dcc9532116086e091e6cf88) ) /* These seem to be misplaced? Verfied good dumps, not */
	ROM_LOAD( "sex-001_02", 0x00000, 0x8000, CRC(b064876b) SHA1(588300fb6603f334de41a9685b1fcf8c642b5c16) ) /* sure which set they actually belong to, maybe trvwzv */
ROM_END

ROM_START( trvwz3v )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-05_u5-0e.u5", 0x0000, 0x2000, CRC(97b8d320) SHA1(573945531113d8aae9418ba1e9a2063052227029) )
	ROM_LOAD( "6221-05_u6-0e.u6", 0x2000, 0x2000, CRC(2e86288d) SHA1(62c7024d8dfebed9bb05ea91302efe5d18cb7d2a) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "spo-002_01", 0x08000, 0x8000, CRC(974dca96) SHA1(eb4a745c84307a1bbb220659877f97c28cd515ac) )
	ROM_LOAD( "spo-002_02", 0x18000, 0x8000, CRC(e15ef8d0) SHA1(51c946311ffe507aa9031044bc34e5ae8d3473ab) )
	ROM_LOAD( "spo-002_03", 0x28000, 0x8000, CRC(503115a1) SHA1(5e6630191465b3d2a590fab08b4f47f7408ecc44) )
	ROM_LOAD( "ent-002_01", 0x38000, 0x8000, CRC(0e4fe73d) SHA1(9aee22a5837637ec5e360b72e71555942df1d26f) )
	ROM_LOAD( "ent-002_02", 0x48000, 0x8000, CRC(f56c0935) SHA1(8e16133ad90829bbc0e0f2e9ee9c26e9d0c5057e) )
	ROM_LOAD( "ent-002_03", 0x58000, 0x8000, CRC(057f6676) SHA1(a93a7a76fc8b8263568a50b00a57f3abe76c9aa3) )
	ROM_LOAD( "gen-002_01", 0x68000, 0x8000, CRC(1fa46b86) SHA1(16d54d0932fe342399faf303eafa3c0b7ba2e202) )
	ROM_LOAD( "gen-002_02", 0x78000, 0x8000, CRC(b395cd97) SHA1(a42c7c1687eaba64a725888cd6413568cc90b010) )
	ROM_LOAD( "sex_triv_a", 0x88000, 0x8000, CRC(15d16703) SHA1(9184f63669e9ec93e88276777e1b7f209543c3e3) ) /* Actual label unknown, maybe sex-002_01? */
	ROM_LOAD( "sex_triv_b", 0x98000, 0x8000, CRC(647f3394) SHA1(636647ae620fd2f985b82e3516451e3bffd44040) ) /* Actual label unknown, maybe sex-002_02? */

	ROM_REGION( 0x0100, "prom", 0 )
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) /* N82S129N BPROM found on question board, unknown use */
ROM_END

ROM_START( trvwz3ha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-04_u5-0c", 0x0000, 0x2000, CRC(e0a07f06) SHA1(02cde0fc4a62d108ecd3e2f7704b9166c31707f2) ) /* Also found labeled as "6221-04 U5-0D" */
	ROM_LOAD( "6221-04_u6-0c", 0x2000, 0x2000, CRC(223482d6) SHA1(4d9dbce7505b98ccd8e2b55f6f86a59b213d72a1) ) /* Also found labeled as "6221-04 U6-0D" */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40b.u40", 0x0000, 0x2000, CRC(e829473f) SHA1(ba754d9377d955b409970494e1a14dbe1d359ee5) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "spo-002_01", 0x08000, 0x8000, CRC(974dca96) SHA1(eb4a745c84307a1bbb220659877f97c28cd515ac) ) /* This set verified as all found on the same question board */
	ROM_LOAD( "spo-002_02", 0x18000, 0x8000, CRC(e15ef8d0) SHA1(51c946311ffe507aa9031044bc34e5ae8d3473ab) )
	ROM_LOAD( "spo-002_03", 0x28000, 0x8000, CRC(503115a1) SHA1(5e6630191465b3d2a590fab08b4f47f7408ecc44) )
	ROM_LOAD( "ent-002_01", 0x38000, 0x8000, CRC(0e4fe73d) SHA1(9aee22a5837637ec5e360b72e71555942df1d26f) )
	ROM_LOAD( "ent-002_02", 0x48000, 0x8000, CRC(f56c0935) SHA1(8e16133ad90829bbc0e0f2e9ee9c26e9d0c5057e) )
	ROM_LOAD( "ent-002_03", 0x58000, 0x8000, CRC(057f6676) SHA1(a93a7a76fc8b8263568a50b00a57f3abe76c9aa3) )
	ROM_LOAD( "gen-002_01", 0x68000, 0x8000, CRC(1fa46b86) SHA1(16d54d0932fe342399faf303eafa3c0b7ba2e202) )
	ROM_LOAD( "gen-002_02", 0x78000, 0x8000, CRC(b395cd97) SHA1(a42c7c1687eaba64a725888cd6413568cc90b010) )
	ROM_LOAD( "sex-002_03", 0x88000, 0x8000, CRC(2f37dcb0) SHA1(e96eeabfa62c0a56c2f888cf1abdfdcb059572c6) ) /* Shows in game as SEX TRIVIA III */
	ROM_LOAD( "sex-002_04", 0x98000, 0x8000, CRC(20bf245e) SHA1(1286fd2eb51c6125a7560da3e2390ec51b64fb43) ) /* Shows in game as SEX TRIVIA III */

	ROM_REGION( 0x0100, "prom", 0 )
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) /* N82S129N BPROM found on question board, unknown use */

	ROM_REGION( 0x200, "misc", 0 )
	ROM_LOAD( "dec002.u13", 0x00000, 0x01f3, CRC(686d2ad0) SHA1(7aad0a1ed09942528eceaf4d7a5e1fd7601aeac7) )
ROM_END

ROM_START( trvwz4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5.bin", 0x0000, 0x8000, CRC(bc23a1ab) SHA1(b9601f316e373c568c5b208de417617094046559) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "tw4-05_ent-1", 0x08000, 0x8000, CRC(1b317149) SHA1(94e882e9cc041ac8f292136c1ce2d21340ac5e7f) )
	ROM_LOAD( "tw4-05_ent-2", 0x18000, 0x8000, CRC(43d51697) SHA1(7af3f16f9519184ae63d8818bbc52a2ba897f275) )
	ROM_LOAD( "tw4-05_rnp-1", 0x28000, 0x8000, CRC(fee2d0b0) SHA1(9c9abec4ce693fc2d3976f3d499213c2ce67c197) )
	ROM_LOAD( "tw4-05_rnp-2", 0x38000, 0x8000, CRC(e54fc4bc) SHA1(4607974ed2bf83c475396fc1cbb1e09ad084ace8) )
	ROM_LOAD( "tw4-05_sbt-1", 0x48000, 0x8000, CRC(f1560804) SHA1(2ef0d587fbedfc342a12e913fa3c94eb8d67e2c5) )
	ROM_LOAD( "tw4-05_sbt-2", 0x58000, 0x8000, CRC(b0d6f6b2) SHA1(b08622d3775d1bb40c3b07ef932f3db4166ee284) )
	ROM_LOAD( "tw4-05_sex-1", 0x68000, 0x8000, CRC(976352b0) SHA1(5f89caca410704ba8a90da3167ba18e45fb21d43) )
	ROM_LOAD( "tw4-05_sex-2", 0x78000, 0x8000, CRC(5f148bc9) SHA1(2fd2cf819c2f395dcffad59857b3533fe3cce60b) )
	ROM_LOAD( "tw4-05_spo-1", 0x88000, 0x8000, CRC(5fe0c6a3) SHA1(17bdb5262ce4edf5f022f075537f6161e1397b46) )
	ROM_LOAD( "tw4-05_spo-2", 0x98000, 0x8000, CRC(3f3390e0) SHA1(50bd7b79268438584bb0f497ab0055b4d4864590) )
ROM_END

/* only the question board was dumped, but it contained a selection
  of roms from the above 'trvwz4' set, and one additional one which is Sex Trivia III
*/
ROM_START( trvwz4a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5.bin", 0x0000, 0x8000, CRC(bc23a1ab) SHA1(b9601f316e373c568c5b208de417617094046559) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "merit2_5.0",  0x08000, 0x8000, CRC(e07d139f) SHA1(e364dcc628719c1bcdc119bdb2f3c98b5538c411) ) // sex trivia III
ROM_END

/*

crt200 rev e-1
1985 merit industries

u39   u37                      z80b
u40   u38
           6845                 u5

      2016                      pb
      2016                      6264

                        ay3-8912
                        8255
                        8255


crt205a

pba pb9 pb8 pb7 pb6 pb5 pb4 pb3 pb2 pb1

*/

ROM_START( tictac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "merit.u5",     0x00000, 0x8000, CRC(f0dd73f5) SHA1(f2988b84255ce5f7ea6d25150cdbae88b98e1be3) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "merit.u39",    0x00000, 0x2000, CRC(dd79e824) SHA1(d65ee1c758293ddf8a5f4913878a2867ba526e68) )
	ROM_LOAD( "merit.u38",    0x02000, 0x2000, CRC(e1bf0fab) SHA1(291261ea817c42d6e8a19c17a2d3706fed7d78c4) )
	ROM_LOAD( "merit.u37",    0x04000, 0x2000, CRC(94f9c7f8) SHA1(494389983fb62fe2d772c276e659b6b20c531933) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "merit.u40",    0x00000, 0x2000, CRC(ab0088eb) SHA1(23a05a4dc11a8497f4fc7e4a76085af15ff89cea) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "merit.pb1",    0x08000, 0x8000, CRC(d1584173) SHA1(7a2190203f478f446cc70c473c345e7cc332e049) ) /* Trivia catagories are: */
	ROM_LOAD( "merit.pb2",    0x18000, 0x8000, CRC(d00ab1fd) SHA1(c94269c8a478e88f71aeca94c6f20fc05a9c62bd) ) /* General Interest, Sports, Entertainment & Sex Trivia III */
	ROM_LOAD( "merit.pb3",    0x28000, 0x8000, CRC(71b398a9) SHA1(5ea07c409afd52c7d08592b30ff0ff3b72c3f8c3) )
	ROM_LOAD( "merit.pb4",    0x38000, 0x8000, CRC(eb34672f) SHA1(c472fc4445fc434029a2740dfc1d9ab9b1ef9f87) )
	ROM_LOAD( "merit.pb5",    0x48000, 0x8000, CRC(8eea30b9) SHA1(fe1d0332106631f56bc6c57a888da9e4e63fa52f) )
	ROM_LOAD( "merit.pb6",    0x58000, 0x8000, CRC(3f45064d) SHA1(de109ac0b19fd1cd7f0020cc174c2da21708108c) )
	ROM_LOAD( "merit.pb7",    0x68000, 0x8000, CRC(f1c446cd) SHA1(9a6f18defbb64e202ae12e1a59502b8f2d6a58a6) )
	ROM_LOAD( "merit.pb8",    0x78000, 0x8000, CRC(206cfc0d) SHA1(78f6b684713459a617096aa3ffe6e9e62583938c) )
	ROM_LOAD( "merit.pb9",    0x88000, 0x8000, CRC(9333dbca) SHA1(dd87e6f69d60580fdb6f979398edbeb1a51be355) )
	ROM_LOAD( "merit.pba",    0x98000, 0x8000, CRC(6eda81f4) SHA1(6d64344691e3e52035a7d30fb3e762f0bd397db7) )
ROM_END

ROM_START( phrcraze )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "phrz.11",      0x00000, 0x8000, CRC(ccd33a0c) SHA1(869b66af4369f3b4bc19336ca2b8104c7f652de7) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "phrz_u37.u37", 0x00000, 0x8000, CRC(237e221a) SHA1(7aa69375c2b9a9e73e0e4ed207bf595368b2deb2) ) /* 1st & 2nd half identical, but correct and verified */
	ROM_LOAD( "phrz_u38.u38", 0x08000, 0x8000, CRC(bfa78b67) SHA1(1b51c0e00240f798fe717624e706cb15700bc2f9) )
	ROM_LOAD( "phrz_u39.u39", 0x10000, 0x8000, CRC(9ce22cb3) SHA1(b653afb8f13decd993e434aaad69a6e09ab65f83) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "phrz_u40.u40", 0x00000, 0x8000, CRC(17dcddd4) SHA1(51682bdbfb67cd0ccf20b97e8fa12d72f0fe82ed) ) /* 1st & 2nd half identical, but correct and verified */

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) // questions
	ROM_LOAD( "phrz1-07_std-1", 0x00000, 0x8000, CRC(0a016c5e) SHA1(1a24ecd7fe59b08c75a1b4575c7fe467cc7f0cf8) )
	ROM_LOAD( "phrz1-07_std-2", 0x10000, 0x8000, CRC(e67dc49e) SHA1(5265af228531dc16db7f7ee78da6e51ef9a1d772) )
	ROM_LOAD( "phrz1-07_std-3", 0x20000, 0x8000, CRC(5c79a653) SHA1(85a904465b347564e937074e2b18159604c83e51) )
	ROM_LOAD( "phrz1-07_std-4", 0x30000, 0x8000, CRC(9837f757) SHA1(01106114b6997fe6432e519101f95c83a1f7cc1e) )
	ROM_LOAD( "phrz1-07_std-5", 0x40000, 0x8000, CRC(dc9d8682) SHA1(46973da4298d0ed149c651498527c91b8ba57e0a) )
	ROM_LOAD( "phrz1-07_std-6", 0x50000, 0x8000, CRC(48e24f17) SHA1(f50c85505f6ab2360f0885494001f174224f8575) )
ROM_END

ROM_START( phrcrazea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pc_u5.bin", 0x00000, 0x8000, CRC(f9642d0a) SHA1(6e9b9929bc28f6c26c70a8b762a2755dc097dbc4) ) /* Horizontal version */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "phrz_u37.u37", 0x00000, 0x8000, CRC(237e221a) SHA1(7aa69375c2b9a9e73e0e4ed207bf595368b2deb2) ) /* 1st & 2nd half identical, but correct and verified */
	ROM_LOAD( "phrz_u38.u38", 0x08000, 0x8000, CRC(bfa78b67) SHA1(1b51c0e00240f798fe717624e706cb15700bc2f9) )
	ROM_LOAD( "phrz_u39.u39", 0x10000, 0x8000, CRC(9ce22cb3) SHA1(b653afb8f13decd993e434aaad69a6e09ab65f83) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "phrz_u40.u40", 0x00000, 0x8000, CRC(17dcddd4) SHA1(51682bdbfb67cd0ccf20b97e8fa12d72f0fe82ed) ) /* 1st & 2nd half identical, but correct and verified */

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "phrz1-07_std-1", 0x00000, 0x8000, CRC(0a016c5e) SHA1(1a24ecd7fe59b08c75a1b4575c7fe467cc7f0cf8) )
	ROM_LOAD( "phrz1-07_std-2", 0x10000, 0x8000, CRC(e67dc49e) SHA1(5265af228531dc16db7f7ee78da6e51ef9a1d772) )
	ROM_LOAD( "phrz1-07_std-3", 0x20000, 0x8000, CRC(5c79a653) SHA1(85a904465b347564e937074e2b18159604c83e51) )
	ROM_LOAD( "phrz1-07_std-4", 0x30000, 0x8000, CRC(9837f757) SHA1(01106114b6997fe6432e519101f95c83a1f7cc1e) )
	ROM_LOAD( "phrz1-07_std-5", 0x40000, 0x8000, CRC(dc9d8682) SHA1(46973da4298d0ed149c651498527c91b8ba57e0a) )
	ROM_LOAD( "phrz1-07_std-6", 0x50000, 0x8000, CRC(48e24f17) SHA1(f50c85505f6ab2360f0885494001f174224f8575) )
ROM_END

ROM_START( phrcrazeb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5281-40_u5-3a.u5",0x00000, 0x8000, CRC(d04c7657) SHA1(0b59fbf553eb5b68544ee2f94cf8106ab30ff1ed) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "phrz_u37.u37", 0x00000, 0x8000, CRC(237e221a) SHA1(7aa69375c2b9a9e73e0e4ed207bf595368b2deb2) ) /* 1st & 2nd half identical, but correct and verified */
	ROM_LOAD( "phrz_u38.u38", 0x08000, 0x8000, CRC(bfa78b67) SHA1(1b51c0e00240f798fe717624e706cb15700bc2f9) )
	ROM_LOAD( "phrz_u39.u39", 0x10000, 0x8000, CRC(9ce22cb3) SHA1(b653afb8f13decd993e434aaad69a6e09ab65f83) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "phrz_u40.u40", 0x00000, 0x8000, CRC(17dcddd4) SHA1(51682bdbfb67cd0ccf20b97e8fa12d72f0fe82ed) ) /* 1st & 2nd half identical, but correct and verified */

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "phrz1-07_sex-2a", 0x00000, 0x8000, CRC(7ef3bca7) SHA1(f25cd01f996882a500e1a800d924759cd1de255d) )
	ROM_LOAD( "phrz1-07_sex-1a", 0x10000, 0x8000, CRC(ed7604b8) SHA1(b1e841b50b8ef6ae95fafac1c34b6d0337a05d18) )
	ROM_LOAD( "phrz1-07_std-8a", 0x20000, 0x8000, CRC(423eecd6) SHA1(ca8d181ccba05acba8ebc57f20e0542eda00c917) )
	ROM_LOAD( "phrz1-07_std-7a", 0x30000, 0x8000, CRC(05dd3900) SHA1(bb13a3c5f84771c450fa88560cc74c5a1be1b876) )
	ROM_LOAD( "phrz1-07_std-6a", 0x40000, 0x8000, CRC(c5980f24) SHA1(a3b665c74aaa704ffa382f95adac70c7c46fb446) )
	ROM_LOAD( "phrz1-07_std-5a", 0x50000, 0x8000, CRC(7cb395a9) SHA1(48b3ac524e6ae23f885b9b767e77930a89a81f5f) )
	ROM_LOAD( "phrz1-07_std-4a", 0x60000, 0x8000, CRC(effc811b) SHA1(2479539965ed541be417bbe48a5e66a58a6294aa) )
	ROM_LOAD( "phrz1-07_std-3a", 0x70000, 0x8000, CRC(c4c7dcee) SHA1(81d879df3da0fbe1cf2247d92b3853104a99689d) )
	ROM_LOAD( "phrz1-07_std-2a", 0x80000, 0x8000, CRC(527b3025) SHA1(36dc129d2276909643e90ae3810c8341076fd88c) )
	ROM_LOAD( "phrz1-07_std-1a", 0x90000, 0x8000, CRC(367f1dfa) SHA1(01d69004c365acefb0e52ac12593a3874c16ab9d) )
ROM_END

ROM_START( phrcrazec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-40_u5-1a.u5",0x00000, 0x8000, CRC(bd8b5612) SHA1(614436da4ed45e0d974b565c5c765bcc1b9d94b5) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "phrz_u37.u37", 0x00000, 0x8000, CRC(237e221a) SHA1(7aa69375c2b9a9e73e0e4ed207bf595368b2deb2) ) /* 1st & 2nd half identical, but correct and verified */
	ROM_LOAD( "phrz_u38.u38", 0x08000, 0x8000, CRC(bfa78b67) SHA1(1b51c0e00240f798fe717624e706cb15700bc2f9) )
	ROM_LOAD( "phrz_u39.u39", 0x10000, 0x8000, CRC(9ce22cb3) SHA1(b653afb8f13decd993e434aaad69a6e09ab65f83) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "phrz_u40.u40", 0x00000, 0x8000, CRC(17dcddd4) SHA1(51682bdbfb67cd0ccf20b97e8fa12d72f0fe82ed) ) /* 1st & 2nd half identical, but correct and verified */

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "phrz1-07_sex-2a", 0x00000, 0x8000, CRC(7ef3bca7) SHA1(f25cd01f996882a500e1a800d924759cd1de255d) )
	ROM_LOAD( "phrz1-07_sex-1a", 0x10000, 0x8000, CRC(ed7604b8) SHA1(b1e841b50b8ef6ae95fafac1c34b6d0337a05d18) )
	ROM_LOAD( "phrz1-07_std-8a", 0x20000, 0x8000, CRC(423eecd6) SHA1(ca8d181ccba05acba8ebc57f20e0542eda00c917) )
	ROM_LOAD( "phrz1-07_std-7a", 0x30000, 0x8000, CRC(05dd3900) SHA1(bb13a3c5f84771c450fa88560cc74c5a1be1b876) )
	ROM_LOAD( "phrz1-07_std-6a", 0x40000, 0x8000, CRC(c5980f24) SHA1(a3b665c74aaa704ffa382f95adac70c7c46fb446) )
	ROM_LOAD( "phrz1-07_std-5a", 0x50000, 0x8000, CRC(7cb395a9) SHA1(48b3ac524e6ae23f885b9b767e77930a89a81f5f) )
	ROM_LOAD( "phrz1-07_std-4a", 0x60000, 0x8000, CRC(effc811b) SHA1(2479539965ed541be417bbe48a5e66a58a6294aa) )
	ROM_LOAD( "phrz1-07_std-3a", 0x70000, 0x8000, CRC(c4c7dcee) SHA1(81d879df3da0fbe1cf2247d92b3853104a99689d) )
	ROM_LOAD( "phrz1-07_std-2a", 0x80000, 0x8000, CRC(527b3025) SHA1(36dc129d2276909643e90ae3810c8341076fd88c) )
	ROM_LOAD( "phrz1-07_std-1a", 0x90000, 0x8000, CRC(367f1dfa) SHA1(01d69004c365acefb0e52ac12593a3874c16ab9d) )
ROM_END

ROM_START( phrcrazev )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5.bin",       0x00000, 0x8000, CRC(6122b5bb) SHA1(9952b14334287a992eefefbdc887b9a9215304ef) ) /* Vertical version */

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "u39.bin",      0x00000, 0x4000, BAD_DUMP CRC(adbd2cdc) SHA1(a1e9481bd6ee0f8915cea43eaad3ebdd54438eed) )
	ROM_LOAD( "u38.bin",      0x04000, 0x4000, BAD_DUMP CRC(3578f00d) SHA1(c6780a6ee1b5eb00258a89bceabbbe380d79d299) )
	ROM_LOAD( "u37.bin",      0x08000, 0x4000, BAD_DUMP CRC(962f18a3) SHA1(ec1c3e470c59905c0f56fce2703f6ff586849512) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "u40.bin",      0x00000, 0x4000, BAD_DUMP CRC(493172c8) SHA1(a76ff5d0d3dd56b0ee4352f03c9ce92f107d34ec) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "phrz1-07_std-1", 0x00000, 0x8000, CRC(0a016c5e) SHA1(1a24ecd7fe59b08c75a1b4575c7fe467cc7f0cf8) )
	ROM_LOAD( "phrz1-07_std-2", 0x10000, 0x8000, CRC(e67dc49e) SHA1(5265af228531dc16db7f7ee78da6e51ef9a1d772) )
	ROM_LOAD( "phrz1-07_std-3", 0x20000, 0x8000, CRC(5c79a653) SHA1(85a904465b347564e937074e2b18159604c83e51) )
	ROM_LOAD( "phrz1-07_std-4", 0x30000, 0x8000, CRC(9837f757) SHA1(01106114b6997fe6432e519101f95c83a1f7cc1e) )
	ROM_LOAD( "phrz1-07_std-5", 0x40000, 0x8000, CRC(dc9d8682) SHA1(46973da4298d0ed149c651498527c91b8ba57e0a) )
	ROM_LOAD( "phrz1-07_std-6", 0x50000, 0x8000, CRC(48e24f17) SHA1(f50c85505f6ab2360f0885494001f174224f8575) )
	/* empty space as per instructions for other "sex" category roms */
	/* "Sex" questions revision A */
	ROM_LOAD( "phrz1-07_sex-2a", 0x80000, 0x8000, CRC(7ef3bca7) SHA1(f25cd01f996882a500e1a800d924759cd1de255d) )
	ROM_LOAD( "phrz1-07_sex-1a", 0x90000, 0x8000, CRC(ed7604b8) SHA1(b1e841b50b8ef6ae95fafac1c34b6d0337a05d18) )
ROM_END

ROM_START( dodge )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-82.u5",     0x0000, 0x8000, CRC(eb82515d) SHA1(d2c15bd633472f50b621ba90598559e345246d01) )

	ROM_REGION( 0x18000, "gfx1", 0 )
    /*
    dodge.u37        1ST AND 2ND HALF IDENTICAL
    dodge.u38        1ST AND 2ND HALF IDENTICAL
    dodge.u39        1ST AND 2ND HALF IDENTICAL */

	ROM_LOAD( "dodge.u39",    0x00000, 0x8000, CRC(3b3376a1) SHA1(6880cdc29686ff7328717c3833ff826c278b023e) )
	ROM_LOAD( "dodge.u38",    0x08000, 0x8000, CRC(654d5b00) SHA1(9e16330b2dc8821fc20a39eb42176fda23085bfc) )
	ROM_LOAD( "dodge.u37",    0x10000, 0x8000, CRC(bc9e63d4) SHA1(2320f5a0545f18e1e42a3a45fedce912c36fbe13) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_ERASEFF )
    /* socket at position u40 is unpopulated */
	ROM_LOAD( "dodge.u40",    0x00000, 0x8000, NO_DUMP )
ROM_END

ROM_START( couple )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.1d",  0x00000, 0x8000, CRC(bc70337a) SHA1(ffc484bc3965f0780d3fa5d8801af27a7164a417) )
	ROM_LOAD( "2.1e",  0x10000, 0x8000, CRC(17372a93) SHA1(e0f0980003473555c2543d98d1494f82afa49f1a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "3.9c",  0x00000, 0x8000, CRC(f017399a) SHA1(baf4c1bea6a12b1d4c8838552503fbdb81378411) )
	ROM_LOAD( "4.9d",  0x08000, 0x8000, CRC(66da76c1) SHA1(8cdcec008d0d51704544069246e9eabb5d5958ea) )
	ROM_LOAD( "5.10c", 0x10000, 0x8000, CRC(fc22bcf4) SHA1(cf3f6872965cb264d56d3a0b5ab998541b9af4ef) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "6.10d", 0x00000, 0x8000, CRC(a6a9a73d) SHA1(f3cb1d434d730f6e00f48079eaf8b88f57779fa0) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "7.7a",  0x00000, 0x0800, CRC(6c36361e) SHA1(7a018eecf3d8b7cf8845dcfcf8067feb292933b2) )	/*video timing?*/
ROM_END

/*f205v's dump,same except for the first z80 rom,first noticeable differences are that
it doesn't jump to the backup ram area and it gives an extra play if you reach a certain
amount of points (there is a dip switch to select the trigger: 150.000 or 200.000*/
ROM_START( couplep )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "p_1.1d", 0x00000, 0x8000, CRC(4601ace6) SHA1(a824ceebf8b9ce77ef2c8e92636e4261f2ae0420) )
	ROM_LOAD( "2.1e",  0x10000, 0x8000, CRC(17372a93) SHA1(e0f0980003473555c2543d98d1494f82afa49f1a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "3.9c",  0x00000, 0x8000, CRC(f017399a) SHA1(baf4c1bea6a12b1d4c8838552503fbdb81378411) )
	ROM_LOAD( "4.9d",  0x08000, 0x8000, CRC(66da76c1) SHA1(8cdcec008d0d51704544069246e9eabb5d5958ea) )
	ROM_LOAD( "5.10c", 0x10000, 0x8000, CRC(fc22bcf4) SHA1(cf3f6872965cb264d56d3a0b5ab998541b9af4ef) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "6.10d", 0x00000, 0x8000, CRC(a6a9a73d) SHA1(f3cb1d434d730f6e00f48079eaf8b88f57779fa0) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "7.7a",  0x00000, 0x0800, CRC(6c36361e) SHA1(7a018eecf3d8b7cf8845dcfcf8067feb292933b2) )	/*video timing?*/
ROM_END

/*f205v's dump,this one looks like an intermediate release between set1 and set2;
it has same dips as set1, but remaining machine code is the same as set2*/
ROM_START( couplei )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "i_1.1d", 0x00000, 0x8000, CRC(760fa29e) SHA1(a37a1562028d9615adff3d2ef88e0156354c720a) )
	ROM_LOAD( "2.1e",  0x10000, 0x8000, CRC(17372a93) SHA1(e0f0980003473555c2543d98d1494f82afa49f1a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "3.9c",  0x00000, 0x8000, CRC(f017399a) SHA1(baf4c1bea6a12b1d4c8838552503fbdb81378411) )
	ROM_LOAD( "4.9d",  0x08000, 0x8000, CRC(66da76c1) SHA1(8cdcec008d0d51704544069246e9eabb5d5958ea) )
	ROM_LOAD( "5.10c", 0x10000, 0x8000, CRC(fc22bcf4) SHA1(cf3f6872965cb264d56d3a0b5ab998541b9af4ef) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "6.10d", 0x00000, 0x8000, CRC(a6a9a73d) SHA1(f3cb1d434d730f6e00f48079eaf8b88f57779fa0) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "7.7a",  0x00000, 0x0800, CRC(6c36361e) SHA1(7a018eecf3d8b7cf8845dcfcf8067feb292933b2) )	/*video timing?*/
ROM_END

static DRIVER_INIT( key_0 )
{
	decryption_key = 0;
}

static DRIVER_INIT( key_2 )
{
	decryption_key = 2;
}

static DRIVER_INIT( key_4 )
{
	decryption_key = 4;
}

static DRIVER_INIT( key_5 )
{
	decryption_key = 5;
}

static DRIVER_INIT( key_7 )
{
	decryption_key = 7;
}

static DRIVER_INIT( couple )
{
	UINT8 *ROM = memory_region(machine, "maincpu");

	#if 0 //quick rom compare test
	{
		static int i,r;
		r = 0;
		for(i=0;i<0x2000;i++)
		{
			if(ROM[0x14000+i] == ROM[0x16000+i])
				r++;
		}
		mame_printf_debug("%02x (in HEX) identical bytes (no offset done)\n",r);
	}
	#endif

	/*The banked rom isn't a *real* banking,it's just a strange rom hook-up,the 2nd
      and the 3rd halves are 100% identical(!),unless it's an error of TWO different
      dumpers it's just the way it is,a.k.a. it's an "hardware" banking.
      update 20060118 by f205v: now we have 3 dumps from 3 different boards and they
      all behave the same...*/
	memory_set_bankptr(machine, "bank1",ROM + 0x10000 + (0x2000 * 2));
}

GAME( 1983, pitboss,  0,       pitboss,  pitboss,  0,      ROT0,  "Merit", "The Pit Boss (Set 1)",                        GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS )
GAME( 1983, pitbossa, pitboss, pitboss,  pitboss,  0,      ROT0,  "Merit", "The Pit Boss (Set 2)",                        GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS )
GAME( 1983, pitbossb, pitboss, pitboss,  pitboss,  0,      ROT0,  "Merit", "The Pit Boss (Set 3)",                        GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS )
GAME( 1983, pitbossc, pitboss, casino5,  pitboss,  0,      ROT0,  "Merit", "The Pit Boss (Set 4)",                        GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS )

GAME( 1984, casino5,  0,       casino5,  casino5,  0,      ROT0,  "Merit", "Casino Five",                                 GAME_SUPPORTS_SAVE )

GAME( 1985, trvwzh,   0,       trvwhiz,  trivia,   key_0,  ROT0,  "Merit", "Trivia ? Whiz (Horizontal) (question set 1)", GAME_SUPPORTS_SAVE )
GAME( 1985, trvwzha,  trvwzh,  trvwhiz,  trivia,   key_0,  ROT0,  "Merit", "Trivia ? Whiz (Horizontal) (question set 2)", GAME_SUPPORTS_SAVE )
GAME( 1985, trvwzhb,  trvwzh,  trvwhiz,  trivia,   key_0,  ROT0,  "Merit", "Trivia ? Whiz (Horizontal) (question set 3)", GAME_SUPPORTS_SAVE )
GAME( 1985, trvwzv,   trvwzh,  trvwhiz,  trivia,   key_0,  ROT90, "Merit", "Trivia ? Whiz (Vertical) (question set 1)",   GAME_SUPPORTS_SAVE )
GAME( 1985, trvwzva,  trvwzh,  trvwhiz,  trivia,   key_0,  ROT90, "Merit", "Trivia ? Whiz (Vertical) (question set 2)",   GAME_SUPPORTS_SAVE )

GAME( 1985, trvwz2,   0,       trvwhiz,  trivia,   key_2,  ROT90, "Merit", "Trivia ? Whiz (Edition 2) (question set 1)",  GAME_SUPPORTS_SAVE )
GAME( 1985, trvwz2a,  trvwz2,  trvwhiz,  trivia,   key_2,  ROT90, "Merit", "Trivia ? Whiz (Edition 2) (question set 2)",  GAME_SUPPORTS_SAVE )

GAME( 1985, trvwz3h,  0,       trvwhiz,  trivia,   key_0,  ROT0,  "Merit", "Trivia ? Whiz (Edition 3 - Horizontal) (set 1)",GAME_SUPPORTS_SAVE )
GAME( 1985, trvwz3ha, trvwz3h, trvwhiz,  trivia,   key_0,  ROT0,  "Merit", "Trivia ? Whiz (Edition 3 - Horizontal) (set 2)",GAME_SUPPORTS_SAVE )
GAME( 1985, trvwz3v,  trvwz3h, trvwhiz,  trivia,   key_0,  ROT90, "Merit", "Trivia ? Whiz (Edition 3 - Vertical)",        GAME_SUPPORTS_SAVE )

GAME( 1985, trvwz4,   0,       trvwhziv, trvwhziv, key_5,  ROT90, "Merit", "Trivia ? Whiz (Edition 4) (question set 1)",  GAME_SUPPORTS_SAVE )
GAME( 1985, trvwz4a,  trvwz4,  trvwhziv, trvwhziv, key_5,  ROT90, "Merit", "Trivia ? Whiz (Edition 4) (question set 2)",  GAME_SUPPORTS_SAVE )

GAME( 1985, tictac,   0,       tictac,   tictac,   key_4,  ROT0,  "Merit", "Tic Tac Trivia",                              GAME_SUPPORTS_SAVE )

GAME( 1986, phrcraze, 0,       phrcraze, phrcraze, key_7,  ROT0,  "Merit", "Phraze Craze (set 1)",                        GAME_SUPPORTS_SAVE )
GAME( 1986, phrcrazea,phrcraze,phrcraze, phrcraze, key_7,  ROT0,  "Merit", "Phraze Craze (set 2)",                        GAME_SUPPORTS_SAVE )
GAME( 1986, phrcrazeb,phrcraze,phrcraze, phrcrazs, key_7,  ROT0,  "Merit", "Phraze Craze (Expanded Questions, set 1)",    GAME_SUPPORTS_SAVE )
GAME( 1986, phrcrazec,phrcraze,phrcraze, phrcrazs, key_7,  ROT0,  "Merit", "Phraze Craze (Expanded Questions, set 2)",    GAME_SUPPORTS_SAVE )
GAME( 1986, phrcrazev,phrcraze,phrcraze, phrcrazs, key_7,  ROT90, "Merit", "Phraze Craze (Sex Kit, Vertical)",            GAME_SUPPORTS_SAVE )

GAME( 1986, bigappg,  0,       bigappg,  bigappg,  0,      ROT0,  "Merit", "Big Apple Games",                             GAME_SUPPORTS_SAVE )

GAME( 1986, dodge,    0,       dodge,    couple,   0,      ROT0,  "Merit", "Dodge City",                                  GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )

GAME( 1988, couple,   0,       couple,   couple,   couple, ROT0,  "Merit", "The Couples (Set 1)",                         GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION )
GAME( 1988, couplep,  couple,  couple,   couplep,  couple, ROT0,  "Merit", "The Couples (Set 2)",                         GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION )
GAME( 1988, couplei,  couple,  couple,   couple,   couple, ROT0,  "Merit", "The Couples (Set 3)",                         GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION )
