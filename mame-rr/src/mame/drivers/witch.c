/*
Pinball Champ '95 / Witch

witch   : Witch
      press F1 to initialize NVRAM

pbchmp95: Pinball Champ '95. Seems to be a simple mod with the following differences:
                        -The title screen is changed
                        -The sample saying "witch" is not played (obviously)
                        -Different configuration values (time limit, etc)
                        -Auto-initialization on NVRAM error(?)
                        -Stars keep falling at the title screen

This is so far what could be reverse-engineered from the code.
BEWARE : these are only suppositions, not facts.

Featured hardware

    2xZ80 ; frequency unknown (CPU2 used mainly for sound effects)
    2xYM2203 (or compatible?) ; frequency unknown (music + sound effects + video scrolling access)
    1xES8712 ; frequency unknown (samples)


GFX

    2 gfx layers accessed by cpu1 (& cpu2 for scrolling) + 1 sprite layer

    In (assumed) order of priority :
        - Top layer @0xc000-0xc3ff(vram) + 0xc400-0xc7ff(cram) apparently not scrollable (gfx0)
            Uses tiles from "gfx2"

            tileno =    vram | ((cram & 0xe0) << 3)
            color  =    cram & 0x0f
            priority =  cram & 0x10 (0x10 = under sprites, 0x00 = over sprites)

        - Sprites @0xd000-0xd7ff + 0xd800-0xdfff
                One sprite every 0x20 bytes
                0x40 sprites
                Tiles are from "gfx2"
                Seems to be only 16x16 sprites (2x2 tiles)
                xflip and yflip available

                tileno                = sprite_ram[i*0x20] << 2 | (( sprite_ram[i*0x20+0x800] & 0x07 ) << 10 );
                sx                    = sprite_ram[i*0x20+1];
                sy              = sprite_ram[i*0x20+2];
                flags+colors    = sprite_ram[i*0x20+3];

        - Background layer @0xc800-0xcbff(vram) + 0xcc00-0xcfff(cram) (gfx1)
                Uses tiles from "gfx1"
                    tileno = vram | ((cram & 0xf0) << 4),
                    color  = cram & 0x0f

                The background is scrolled via 2 registers accessed through one of the ym2203, port A&B
                The scrolling is set by CPU2 in its interrupt handler.
                CPU1 doesn't seem to offset vram accesses for the scrolling, so it's assumed to be done
                in hardware.
                This layer looks misaligned with the sprites, but the top layer is not. This is perhaps
                due to the weird handling of the scrolling. For now we just offset it by 7 pixels.


Palette

    3*0x100 palette banks @ 0xe000-0xe300 & 0xe800-0xe8ff (xBBBBBGGGGGRRRRR_split format?)
    Bank 1 is used for gfx0 (top layer) and sprites
    Bank 2 is for gfx1 (background layer)

    Could not find any use of bank 0 ; I'm probably missing a flag somewhere.


Sound

    Mainly handled by CPU2

    2xYM2203

    0x8000-0x8001 : Mainly used for sound effects & to read dipswitches
    0x8008-0x8009 : Music & scrolling

    1xES8712

    Mapped @0x8010-0x8016
    Had to patch es8712.c to start playing on 0x8016 write and to prevent continuous looping.
    There's a test on bit1 at offset 0 (0x8010), so this may be a "read status" kind of port.
    For now reading at 8010 always reports ready.


Ports

    0xA000-0xA00f : Various ports yet to figure out...

      - 0xA000 : unknown ; seems muxed with a002
      - 0xA002 : banking?
                         bank number = bits 7&6 (swapped?)
                 mapped 0x0800-0x7fff?
                 0x0000-0x07ff ignored?
                 see code @ 61d
                 lower bits seems to mux port A000 reads
      - 0xA003 : ?
      - 0xA004 : dipswitches
      - 0xA005 : dipswitches
      - 0xA006 : bit1(out) = release coin?
      - 0xA007 : ?
      - 0xA008 : cpu1 sets it to 0x80 on reset ; cleared in interrupt handler
                             cpu2 sets it to 0x40 on reset ; cleared in interrupt handler
      - 0xA00C : bit0 = payout related?
                         bit3 = reset? (see cpu2 code @14C)
      - 0xA00E : ?


Memory

    RAM:
        Considering that
            -CPU1 busy loops on fd00 and that CPU2 modifies fd00 once it is initialized
            -CPU1 writes to fd01-fd05 and CPU2 reads there and plays sounds accordingly
            -CPU1 writes to f208-f209 and CPU2 forwards this to the scrolling registers
        we can assume that the 0xf2xx and 0fdxx segments are shared.

        From the fact that
            -CPU1's SP is set to 0xf100 on reset
            -CPU2's SP is set to 0xf080 on reset
        we may suppose that this memory range (0xf000-0xf0ff) is shared too.

        Moreover, range 0xf100-0xf17f is checked after reset without prior initialization and
        is being reset ONLY by changing a particular port bit whose modification ends up with
        a soft reboot. This looks like a good candidate for an NVRAM segment.
        Whether CPU2 can access the NVRAM or not is still a mystery considering that it never
        attempts to do so.

        From these we consider that the 0xfxxx segment, except for the NVRAM range, is shared
        between the two CPUs.

  CPU1:
      The ROM segment (0x0000-0x7fff) is banked, but the 0x0000-0x07ff region does not look
      like being affected (the SEGA Master System did something similar IIRC). A particular
      bank is selected by changing the two most significant bits of port 0xa002 (swapped?).

  CPU2:
            No banking
        Doesn't seem to be banking going on. However there's a strange piece of code @0x021a:
        Protection(?) check @ $21a


Interesting memory locations

        +f180-f183 : dipswitches stored here (see code@2746). Beware, all values are "CPL"ed!
            *f180   : kkkbbppp / A005
                             ppp  = PAY OUT | 60 ; 65 ; 70 ; 75 ; 80 ; 85 ; 90 ; 95
                             bb   = MAX BET | 20 ; 30 ; 40 ; 60
                             kkk  = KEY IN  | 1-10 ; 1-20 ; 1-40 ; 1-50 ; 1-100 ; 1-200 ; 1-250 ; 1-500

            *f181   : ccccxxxd / A004
                             d    = DOUBLE UP | ON ; OFF
                             cccc = COIN IN1 | 1-1 ; 1-2 ; 1-3 ; 1-4 ; 1-5 ; 1-6 ; 1-7 ; 1-8 ; 1-9 ; 1-10 ; 1-15 ; 1-20 ; 1-25 ; 1-30 ; 1-40 ; 1-50

            *f182   : sttpcccc / portA
                             cccc = COIN IN2 | 1-1 ; 1-2 ; 1-3 ; 1-4 ; 1-5 ; 1-6 ; 1-7 ; 1-8 ; 1-9 ; 1-10 ; 2-1 ; 3-1 ; 4-1 ; 5-1 ; 6-1 ; 10-1
                             p    = PAYOUT SWITCH | ON ; OFF
                             tt   = TIME | 40 ; 45 ; 50 ; 55
                             s    = DEMO SOUND | ON ; OFF
            *f183 : xxxxhllb / portB
                             b    = AUTO BET | ON ; OFF
                             ll   = GAME LIMIT | 500 ; 1000 ; 5000 ; 990000
                             h    = HOPPER ACTIVE | LOW ; HIGH


        +f15c-f15e : MAX WIN
        +f161      : JACK POT
        +f166-f168 : DOUBLE UP
        +f16b-f16d : MAX D-UP WIN

        +f107-f109 : TOTAL IN
        +f10c-f10e : TOTAL OUT

        +f192-f194 : credits (bcd)

        +fd00 = cpu2 ready
        +f211 = input port cache?

    CPU2 Commands :
        -0xfd01 start music
        -0xfd02 play sound effect
        -0xfd03 play sample on the ES8712
        -0xfd04 ?
        -0xfd05 ?


TODO :
    - Figure out the ports for the "PayOut" stuff (a006/a00c?)
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/es8712.h"
#include "sound/2203intf.h"

#define UNBANKED_SIZE 0x800

static tilemap_t *gfx0a_tilemap;
static tilemap_t *gfx0b_tilemap;
static tilemap_t *gfx1_tilemap;

static UINT8 *gfx0_cram;
static UINT8 *gfx0_vram;

static UINT8 *gfx1_cram;
static UINT8 *gfx1_vram;

static UINT8 *sprite_ram;

static int scrollx=0;
static int scrolly=0;

static UINT8 reg_a002=0;
static int bank;

static TILE_GET_INFO( get_gfx0b_tile_info )
{
	int code  = gfx0_vram[tile_index];
	int color = gfx0_cram[tile_index];

	code=code | ((color & 0xe0) << 3);

	if(color&0x10)
	{
		code=0;
	}

	SET_TILE_INFO(
			1,
			code,//tiles beyond 0x7ff only for sprites?
			color & 0x0f,
			0);
}

static TILE_GET_INFO( get_gfx0a_tile_info )
{
	int code  = gfx0_vram[tile_index];
	int color = gfx0_cram[tile_index];

	code=code | ((color & 0xe0) << 3);

	if((color&0x10)==0)
	{
		code=0;
	}

	SET_TILE_INFO(
			1,
			code,//tiles beyond 0x7ff only for sprites?
			color & 0x0f,
			0);
}

static TILE_GET_INFO( get_gfx1_tile_info )
{
	int code  = gfx1_vram[tile_index];
	int color = gfx1_cram[tile_index];

	SET_TILE_INFO(
			0,
			code | ((color & 0xf0) << 4),
			(color>>0) & 0x0f,
			0);
}

static WRITE8_HANDLER( gfx0_vram_w )
{
	gfx0_vram[offset] = data;
	tilemap_mark_tile_dirty(gfx0a_tilemap,offset);
	tilemap_mark_tile_dirty(gfx0b_tilemap,offset);
}

static WRITE8_HANDLER( gfx0_cram_w )
{
	gfx0_cram[offset] = data;
	tilemap_mark_tile_dirty(gfx0a_tilemap,offset);
	tilemap_mark_tile_dirty(gfx0b_tilemap,offset);
}
static READ8_HANDLER( gfx0_vram_r )
{
	return gfx0_vram[offset];
}

static READ8_HANDLER( gfx0_cram_r )
{
	return gfx0_cram[offset];
}

#define FIX_OFFSET() do { offset=(((offset + ((scrolly & 0xf8) << 2) ) & 0x3e0)+((offset + (scrollx >> 3) ) & 0x1f)+32)&0x3ff; } while(0)

static WRITE8_HANDLER( gfx1_vram_w )
{
	FIX_OFFSET();
	gfx1_vram[offset] = data;
	tilemap_mark_tile_dirty(gfx1_tilemap,offset);
}

static WRITE8_HANDLER( gfx1_cram_w )
{
	FIX_OFFSET();
	gfx1_cram[offset] = data;
	tilemap_mark_tile_dirty(gfx1_tilemap,offset);
}
static READ8_HANDLER( gfx1_vram_r )
{
	FIX_OFFSET();
	return gfx1_vram[offset];
}

static READ8_HANDLER( gfx1_cram_r )
{
	FIX_OFFSET();
	return gfx1_cram[offset];
}

static READ8_HANDLER(read_a00x)
{
	switch(offset)
	{
		case 0x02: return reg_a002;
		case 0x04: return input_port_read(space->machine, "A004");
		case 0x05: return input_port_read(space->machine, "A005");
		case 0x0c: return input_port_read(space->machine, "SERVICE");	// stats / reset
		case 0x0e: return input_port_read(space->machine, "A00E");		// coin/reset
	}

	if(offset == 0x00) //muxed with A002?
	{
		switch(reg_a002 & 0x3f)
		{
		case 0x3b:
			return input_port_read(space->machine, "UNK");	//bet10 / pay out
		case 0x3e:
			return input_port_read(space->machine, "INPUTS");	//TODO : trace f564
		case 0x3d:
			return input_port_read(space->machine, "A005");
		default:
			logerror("A000 read with mux=0x%02x\n", reg_a002 & 0x3f);
		}
	}
	return 0xff;
}

static WRITE8_HANDLER(write_a00x)
{
	switch(offset)
	{
		case 0x02: //A002 bit 7&6 = bank ????
		{
			int newbank;
			reg_a002 = data;
			newbank = (data>>6)&3;

			if(newbank != bank)
			{
				UINT8 *ROM = memory_region(space->machine, "maincpu");
				bank = newbank;
				ROM = &ROM[0x10000+0x8000 * newbank + UNBANKED_SIZE];
				memory_set_bankptr(space->machine, "bank1",ROM);
			}
		}
		break;

		case 0x06: // bit 1 = coin lockout/counter ?
		break;

		case 0x08: //A008
			cpu_set_input_line(space->cpu,0,CLEAR_LINE);
		break;
	}
}

static READ8_HANDLER(prot_read_700x)
{
/*
    Code @$21a looks like simple protection check.

    - write 7,6,0 to $700f
    - read 5 bytes from $7000-$7004 ( bit 1 of $700d is data "READY" status)

    Data @ $7000 must differs from data @$7001-04.
    Otherwise later in game some I/O (controls) reads are skipped.
*/

  switch(cpu_get_pc(space->cpu))
  {
	case 0x23f:
	case 0x246:
	case 0x24c:
	case 0x252:
	case 0x258:
	case 0x25e:
		return offset;//enough to pass...
  }
  return memory_region(space->machine, "sub")[0x7000+offset];
}

/*
 * Status from ES8712?
 * BIT1 is zero when no sample is playing?
 */
static READ8_DEVICE_HANDLER(read_8010) {	return 0x00; }

static WRITE8_DEVICE_HANDLER(xscroll_w)
{
	scrollx=data;
}
static WRITE8_DEVICE_HANDLER(yscroll_w)
{
	scrolly=data;
}

static const ym2203_interface ym2203_interface_0 =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_INPUT_PORT("YM_PortA"),
		DEVCB_INPUT_PORT("YM_PortB"),
		DEVCB_NULL,
		DEVCB_NULL
	},
	NULL
};

static const ym2203_interface ym2203_interface_1 =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_HANDLER(xscroll_w),
		DEVCB_HANDLER(yscroll_w)
	},
	NULL
};

static ADDRESS_MAP_START( map_main, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, UNBANKED_SIZE-1) AM_ROM
	AM_RANGE(UNBANKED_SIZE, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x8001) AM_DEVREADWRITE("ym1", ym2203_r, ym2203_w)
	AM_RANGE(0x8008, 0x8009) AM_DEVREADWRITE("ym2", ym2203_r, ym2203_w)
	AM_RANGE(0xa000, 0xa00f) AM_READWRITE(read_a00x, write_a00x)
	AM_RANGE(0xc000, 0xc3ff) AM_READWRITE(gfx0_vram_r, gfx0_vram_w) AM_BASE(&gfx0_vram)
	AM_RANGE(0xc400, 0xc7ff) AM_READWRITE(gfx0_cram_r, gfx0_cram_w) AM_BASE(&gfx0_cram)
	AM_RANGE(0xc800, 0xcbff) AM_READWRITE(gfx1_vram_r, gfx1_vram_w) AM_BASE(&gfx1_vram)
	AM_RANGE(0xcc00, 0xcfff) AM_READWRITE(gfx1_cram_r, gfx1_cram_w) AM_BASE(&gfx1_cram)
	AM_RANGE(0xd000, 0xdfff) AM_RAM AM_BASE(&sprite_ram)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_split1_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_split2_w) AM_BASE_GENERIC(paletteram2)
	AM_RANGE(0xf000, 0xf0ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xf100, 0xf17f) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0xf180, 0xffff) AM_RAM AM_SHARE("share2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( map_sub, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8001) AM_DEVREADWRITE("ym1", ym2203_r, ym2203_w)
	AM_RANGE(0x8008, 0x8009) AM_DEVREADWRITE("ym2", ym2203_r, ym2203_w)
	AM_RANGE(0x8010, 0x8016) AM_DEVREADWRITE("essnd", read_8010, es8712_w)
	AM_RANGE(0xa000, 0xa00f) AM_READWRITE(read_a00x, write_a00x)
	AM_RANGE(0xf000, 0xf0ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xf180, 0xffff) AM_RAM AM_SHARE("share2")
ADDRESS_MAP_END

static INPUT_PORTS_START( witch )
	PORT_START("SERVICE")	/* DSW */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("NVRAM Init") PORT_CODE(KEYCODE_F1)
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Stats")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("A00E")	/* DSW */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Key In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset ?")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
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

	PORT_START("UNK")	/* DSW ?*/
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START("INPUTS")	/* Inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Flipper")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Big")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Small")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Double Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Bet")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Flipper")

/*
F180 kkkbbppp ; Read onPORT 0xA005
 ppp  = PAY OUT | 60 ; 65 ; 70 ; 75 ; 80 ; 85 ; 90 ; 95
 bb   = MAX BET | 20 ; 30 ; 40 ; 60
 kkk  = KEY IN  | 1-10 ; 1-20 ; 1-40 ; 1-50 ; 1-100 ; 1-200 ; 1-250 ; 1-500
*/
	PORT_START("A005")	/* DSW */
	PORT_DIPNAME( 0x07, 0x07, "PAY OUT" )
	PORT_DIPSETTING(    0x07, "60" )
	PORT_DIPSETTING(    0x06, "65" )
	PORT_DIPSETTING(    0x05, "70" )
	PORT_DIPSETTING(    0x04, "75" )
	PORT_DIPSETTING(    0x03, "80" )
	PORT_DIPSETTING(    0x02, "85" )
	PORT_DIPSETTING(    0x01, "90" )
	PORT_DIPSETTING(    0x00, "95" )
	PORT_DIPNAME( 0x18, 0x00, "MAX BET" )
	PORT_DIPSETTING(    0x18, "20" )
	PORT_DIPSETTING(    0x10, "30" )
	PORT_DIPSETTING(    0x08, "40" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0xe0, 0xe0, "KEY IN" )
	PORT_DIPSETTING(    0xE0, "1-10"  )
	PORT_DIPSETTING(    0xC0, "1-20"  )
	PORT_DIPSETTING(    0xA0, "1-40"  )
	PORT_DIPSETTING(    0x80, "1-50"  )
	PORT_DIPSETTING(    0x60, "1-100" )
	PORT_DIPSETTING(    0x40, "1-200" )
	PORT_DIPSETTING(    0x20, "1-250" )
	PORT_DIPSETTING(    0x00, "1-500" )
/*
*f181   : ccccxxxd ; Read onPORT 0xA004
 d    = DOUBLE UP | ON ; OFF
 cccc = COIN IN1 | 1-1 ; 1-2 ; 1-3 ; 1-4 ; 1-5 ; 1-6 ; 1-7 ; 1-8 ; 1-9 ; 1-10 ; 1-15 ; 1-20 ; 1-25 ; 1-30 ; 1-40 ; 1-50
*/
	PORT_START("A004")	/* DSW */
	PORT_DIPNAME( 0x01, 0x00, "DOUBLE UP" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0, 0xf0, "COIN IN1" )
	PORT_DIPSETTING(    0xf0, "1-1" )
	PORT_DIPSETTING(    0xe0, "1-2" )
	PORT_DIPSETTING(    0xd0, "1-3" )
	PORT_DIPSETTING(    0xc0, "1-4" )
	PORT_DIPSETTING(    0xb0, "1-5" )
	PORT_DIPSETTING(    0xa0, "1-6" )
	PORT_DIPSETTING(    0x90, "1-7" )
	PORT_DIPSETTING(    0x80, "1-8" )
	PORT_DIPSETTING(    0x70, "1-9" )
	PORT_DIPSETTING(    0x60, "1-10" )
	PORT_DIPSETTING(    0x50, "1-15" )
	PORT_DIPSETTING(    0x40, "1-20" )
	PORT_DIPSETTING(    0x30, "1-25" )
	PORT_DIPSETTING(    0x20, "1-30" )
	PORT_DIPSETTING(    0x10, "1-40" )
	PORT_DIPSETTING(    0x00, "1-50" )

/*
*f182   : sttpcccc ; Read onPORT A of YM2203 @ 0x8001
 cccc = COIN IN2 | 1-1 ; 1-2 ; 1-3 ; 1-4 ; 1-5 ; 1-6 ; 1-7 ; 1-8 ; 1-9 ; 1-10 ; 2-1 ; 3-1 ; 4-1 ; 5-1 ; 6-1 ; 10-1
 p    = PAYOUT SWITCH | ON ; OFF
 tt   = TIME | 40 ; 45 ; 50 ; 55
 s    = DEMO SOUND | ON ; OFF
*/
	PORT_START("YM_PortA")	/* DSW */
	PORT_DIPNAME( 0x0f, 0x0f, "COIN IN2" )
	PORT_DIPSETTING(    0x0f, "1-1" )
	PORT_DIPSETTING(    0x0e, "1-2" )
	PORT_DIPSETTING(    0x0d, "1-3" )
	PORT_DIPSETTING(    0x0c, "1-4" )
	PORT_DIPSETTING(    0x0b, "1-5" )
	PORT_DIPSETTING(    0x0a, "1-6" )
	PORT_DIPSETTING(    0x09, "1-7" )
	PORT_DIPSETTING(    0x08, "1-8" )
	PORT_DIPSETTING(    0x07, "1-9" )
	PORT_DIPSETTING(    0x06, "1-10" )
	PORT_DIPSETTING(    0x05, "2-1" )
	PORT_DIPSETTING(    0x04, "3-1" )
	PORT_DIPSETTING(    0x03, "4-1" )
	PORT_DIPSETTING(    0x02, "5-1" )
	PORT_DIPSETTING(    0x01, "6-1" )
	PORT_DIPSETTING(    0x00, "10-1" )
	PORT_DIPNAME( 0x10, 0x00, "PAYOUT SWITCH" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x00, "TIME" )
	PORT_DIPSETTING(    0x60, "40" )
	PORT_DIPSETTING(    0x40, "45" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x00, "55" )
	PORT_DIPNAME( 0x80, 0x00, "DEMO SOUND" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

/*
*f183 : xxxxhllb ; Read onPORT B of YM2203 @ 0x8001
 b    = AUTO BET | ON ; OFF
 ll   = GAME LIMIT | 500 ; 1000 ; 5000 ; 990000
 h    = HOPPER ACTIVE | LOW ; HIGH
*/
	PORT_START("YM_PortB")	/* DSW */
	PORT_DIPNAME( 0x01, 0x01, "AUTO BET" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "GAME LIMIT" )
	PORT_DIPSETTING(    0x06, "500" )
	PORT_DIPSETTING(    0x04, "1000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x00, "990000" )
	PORT_DIPNAME( 0x08, 0x08, "HOPPER" )
	PORT_DIPSETTING(    0x08, DEF_STR(Low) )
	PORT_DIPSETTING(    0x00, DEF_STR(High) )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2,3 },
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 8, 12, RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+12},
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( witch )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

static VIDEO_START(witch)
{
	gfx0a_tilemap = tilemap_create(machine, get_gfx0a_tile_info,tilemap_scan_rows,8,8,32,32);
	gfx0b_tilemap = tilemap_create(machine, get_gfx0b_tile_info,tilemap_scan_rows,8,8,32,32);
	gfx1_tilemap = tilemap_create(machine, get_gfx1_tile_info,tilemap_scan_rows,8,8,32,32);

	tilemap_set_transparent_pen(gfx0a_tilemap,0);
	tilemap_set_transparent_pen(gfx0b_tilemap,0);
  tilemap_set_palette_offset(gfx0a_tilemap,0x100);
  tilemap_set_palette_offset(gfx0b_tilemap,0x100);
  tilemap_set_palette_offset(gfx1_tilemap,0x200);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int i,sx,sy,tileno,flags,color;
	int flipx=0;
	int flipy=0;

	for(i=0;i<0x800;i+=0x20) {


		sx     = sprite_ram[i+1];
		if(sx!=0xF8) {
			tileno = (sprite_ram[i]<<2)  | (( sprite_ram[i+0x800] & 0x07 ) << 10 );

			sy     = sprite_ram[i+2];
			flags  = sprite_ram[i+3];

			flipx  = (flags & 0x10 ) ? 1 : 0;
			flipy  = (flags & 0x20 ) ? 1 : 0;

			color  =  flags & 0x0f;


			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				tileno, color,
				flipx, flipy,
				sx+8*flipx,sy+8*flipy,0);

			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				tileno+1, color,
				flipx, flipy,
				sx+8-8*flipx,sy+8*flipy,0);

			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				tileno+2, color,
				flipx, flipy,
				sx+8*flipx,sy+8-8*flipy,0);

			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				tileno+3, color,
				flipx, flipy,
				sx+8-8*flipx,sy+8-8*flipy,0);

		}
	}

}

static VIDEO_UPDATE(witch)
{
	tilemap_set_scrollx( gfx1_tilemap, 0, scrollx-7 ); //offset to have it aligned with the sprites
	tilemap_set_scrolly( gfx1_tilemap, 0, scrolly+8 );



	tilemap_draw(bitmap,cliprect,gfx1_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,gfx0a_tilemap,0,0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap,cliprect,gfx0b_tilemap,0,0);
	return 0;
}

static INTERRUPT_GEN( witch_main_interrupt )
{
	cpu_set_input_line(device,0,ASSERT_LINE);
}

static INTERRUPT_GEN( witch_sub_interrupt )
{
	cpu_set_input_line(device,0,ASSERT_LINE);
}

static MACHINE_DRIVER_START( witch )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,8000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(map_main)
	MDRV_CPU_VBLANK_INT("screen", witch_main_interrupt)

	/* 2nd z80 */
	MDRV_CPU_ADD("sub", Z80,8000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(map_sub)
	MDRV_CPU_VBLANK_INT("screen", witch_sub_interrupt)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(8, 256-1-8, 8*4, 256-8*4-1)

	MDRV_GFXDECODE(witch)
	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(witch)
	MDRV_VIDEO_UPDATE(witch)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("essnd", ES8712, 8000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("ym1", YM2203, 1500000)
	MDRV_SOUND_CONFIG(ym2203_interface_0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MDRV_SOUND_ADD("ym2", YM2203, 1500000)
	MDRV_SOUND_CONFIG(ym2203_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

MACHINE_DRIVER_END

/* this set has (c)1992 Sega / Vic Tokai in the roms? */
ROM_START( witch )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "rom.u5", 0x10000, 0x20000, CRC(348fccb8) SHA1(947defd86c4a597fbfb9327eec4903aa779b3788)  )
	ROM_COPY( "maincpu" , 0x10000, 0x0000, 0x8000 )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "rom.s6", 0x00000, 0x08000, CRC(82460b82) SHA1(d85a9d77edaa67dfab8ff6ac4cb6273f0904b3c0)  )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "rom.u3", 0x00000, 0x20000,  CRC(7007ced4) SHA1(6a0aac3ff9a4d5360c8ba1142f010add1b430ada)  )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "rom.a1", 0x00000, 0x40000,  CRC(512300a5) SHA1(1e9ba58d1ddbfb8276c68f6d5c3591e6b77abf21)  )

	ROM_REGION( 0x40000, "essnd", 0 )
	ROM_LOAD( "rom.v10", 0x00000, 0x40000, CRC(62e42371) SHA1(5042abc2176d0c35fd6b698eca4145f93b0a3944) )
ROM_END

/* no sega logo? a bootleg? */
ROM_START( pbchmp95 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "3.bin", 0x10000, 0x20000, CRC(e881aa05) SHA1(10d259396cac4b9a1b72c262c11ffa5efbdac433)  )
	ROM_COPY( "maincpu" , 0x10000, 0x0000, 0x8000 )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "4.bin", 0x00000, 0x08000, CRC(82460b82) SHA1(d85a9d77edaa67dfab8ff6ac4cb6273f0904b3c0)  )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x00000, 0x20000,  CRC(7007ced4) SHA1(6a0aac3ff9a4d5360c8ba1142f010add1b430ada)  )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x40000,  CRC(f6cf7ed6) SHA1(327580a17eb2740fad974a01d97dad0a4bef9881)  )

	ROM_REGION( 0x40000, "essnd", 0 )
	ROM_LOAD( "5.bin", 0x00000, 0x40000, CRC(62e42371) SHA1(5042abc2176d0c35fd6b698eca4145f93b0a3944) )
ROM_END

static DRIVER_INIT(witch)
{
	UINT8 *ROM = (UINT8 *)memory_region(machine, "maincpu");
	memory_set_bankptr(machine, "bank1", &ROM[0x10000+UNBANKED_SIZE]);

	memory_install_read8_handler(cputag_get_address_space(machine, "sub", ADDRESS_SPACE_PROGRAM), 0x7000, 0x700f, 0, 0, prot_read_700x);
	bank = -1;
}

GAME( 1992, witch,    0,     witch, witch, witch, ROT0, "Sega / Vic Tokai", "Witch", 0 )
GAME( 1995, pbchmp95, witch, witch, witch, witch, ROT0, "bootleg? (Veltmeijer Automaten)", "Pinball Champ '95 (bootleg?)", 0 )
