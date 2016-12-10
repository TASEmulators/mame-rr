/*

  Greyhound Electronics Inc. hardware

  driver by Nicola Salmoria
  driver by Pierpaolo Prazzoli and graphic fixes by Tomasz Slanina
  additional work by Mariusz Wojcieszek


ROM board has a part # UVM10B  1984
Main board has a part # UV-1B Rev 5 1982-83-84

Processor: Z80
Support Chips:(2) 8255s
RAM: 6117on ROM board and (24) MCM4517s on main board

When games are first run a RAM error will occur because the nvram needs initialising.
When ERROR appears, press F2, then F3 to reset, then F2 again and the game will start

Trivia games "No Coins" mode: if DSW "No Coins" is on, coin inputs are
replaced by a 6th button to start games. This is a feature of the PCB for private use.

Selection/Poker payout button: if pressed, all coins/credits are gone and added to the
payout bookkeeping, shown in the service mode under the coin in total. Last Winner shows
the last payout. Payout hardware is unknown.

Video Trivia sets (as stated from Greyhound Electronics, Inc. manual):

Series 1: (128K)           Series 2: (128K)           Series 3: (128K)
----------------           ----------------           ----------------
Science                    T.V. Mash                  Beatlemania
General I                  General II                 General III
Sports I                   Sports II                  Sports III
Music                      Comics                     Country-Western
Movies-T.V.                Entertainment              T.V. Soaps


Series 4: (128K)           Series 5: (128K)           Series 6: (128K)
----------------           ----------------           ----------------
History-Geography          The States                 Science II
Star Trek                  James Bond                 General IV
Baseball                   Hockey                     Commercials-Ads
Hollywood                  Elvismania                 Honeymooners
Television I               The Wild West              Television II


Series 7: (128K)           Series 8: (256K)           * Starting with Series 8
----------------           ----------------           "Announcement":
T.V. Dallas                Science                    3 Times as many
General V                  General                    questions in this
Kids Korner                Sports                     series!
Good Guys                  T.V./Entertainment
Biblical                   Adult Sex
                            or alt: Potpourri


Series 9: (256K)           Series 10: (256K)          Series 11: (256K)
----------------           -----------------          -----------------
Facts                      New Science                Rich and Famous
Rock-N-Roll                New General                Fast Women and Cars
Television                 New T.V. Mash              Aerospace
Artists-Athletes           New Entertainment          TV/Music Alternative
U.S.A. Trivia              New Sports                 General Facts
 or alt: Adult Sex 2        or alt: Adult Sex 3        or alt: Gay Times

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "machine/ticket.h"
#include "sound/dac.h"

static UINT8 drawctrl[3];
static UINT8 color[8];

static WRITE8_HANDLER( gei_drawctrl_w )
{
	drawctrl[offset] = data;
	if (offset == 2)
	{
		int i;
		for (i = 0; i < 8; i++)
			if (BIT(drawctrl[1],i)) color[i] = drawctrl[0] & 7;
	}
}

static WRITE8_HANDLER( gei_bitmap_w )
{
	int sx,sy;
	static int prevoffset, yadd;
	int i;

	space->machine->generic.videoram.u8[offset] = data;

	yadd = (offset==prevoffset) ? (yadd+1):0;
	prevoffset = offset;

	sx = 8 * (offset % 64);
	sy = offset / 64;
	sy = (sy + yadd) & 0xff;


	for (i = 0; i < 8; i++)
		*BITMAP_ADDR16(space->machine->generic.tmpbitmap, sy, sx+i) = color[8-i-1];
}

static PALETTE_INIT(gei)
{
	int i;

	for (i = 0; i < 8; i++ )
	{
		palette_set_color(machine, i, MAKE_RGB(pal1bit(i >> 2), pal1bit(i), pal1bit(i >> 1)));
	}
}

static PALETTE_INIT(quizvid)
{
	int i;

	for (i = 0; i < 8; i++ )
	{
		palette_set_color(machine, i, MAKE_RGB(pal1bit(i >> 1), pal1bit(i), pal1bit(i >> 2)));
	}
}

static WRITE8_DEVICE_HANDLER( lamps_w )
{
	/* 5 button lamps */
	set_led_status(device->machine, 0,data & 0x01);
	set_led_status(device->machine, 1,data & 0x02);
	set_led_status(device->machine, 2,data & 0x04);
	set_led_status(device->machine, 3,data & 0x08);
	set_led_status(device->machine, 4,data & 0x10);

	/* 3 button lamps for deal, cancel, stand in poker games;
    lamp order verified in poker and selection self tests */
	set_led_status(device->machine, 7,data & 0x20);
	set_led_status(device->machine, 5,data & 0x40);
	set_led_status(device->machine, 6,data & 0x80);
}

static WRITE8_DEVICE_HANDLER( sound_w )
{
	const address_space *space = cputag_get_address_space(device->machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	/* bit 3 - coin lockout, lamp10 in poker / lamp6 in trivia test modes */
	coin_lockout_global_w(device->machine, ~data & 0x08);
	set_led_status(device->machine, 9,data & 0x08);

	/* bit 5 - ticket out in trivia games */
	ticket_dispenser_w(device->machine->device("ticket"), 0, (data & 0x20)<< 2);

	/* bit 6 enables NMI */
	interrupt_enable_w(space, 0, data & 0x40);

	/* bit 7 goes directly to the sound amplifier */
	dac_data_w(device->machine->device("dac"), ((data & 0x80) >> 7) * 255);
}

static WRITE8_DEVICE_HANDLER( sound2_w )
{
	/* bit 3,6 - coin lockout, lamp10+11 in selection test mode */
	coin_lockout_w(device->machine, 0, ~data & 0x08);
	coin_lockout_w(device->machine, 1, ~data & 0x40);
	set_led_status(device->machine, 9,data & 0x08);
	set_led_status(device->machine, 10,data & 0x40);

	/* bit 4,5 - lamps 12, 13 in selection test mode;
    12 lights up if dsw maximum bet = 30 an bet > 15 or if dsw maximum bet = 10 an bet = 10 */
	set_led_status(device->machine, 11,data & 0x10);
	set_led_status(device->machine, 12,data & 0x20);

	/* bit 7 goes directly to the sound amplifier */
	dac_data_w(device->machine->device("dac"), ((data & 0x80) >> 7) * 255);
}

static WRITE8_DEVICE_HANDLER( lamps2_w )
{
	/* bit 4 - play/raise button lamp, lamp 9 in poker test mode  */
	set_led_status(device->machine, 8,data & 0x10);
}

static WRITE8_DEVICE_HANDLER( nmi_w )
{
	const address_space *space = cputag_get_address_space(device->machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	/* bit 4 - play/raise button lamp, lamp 9 in selection test mode  */
	set_led_status(device->machine, 8,data & 0x10);

	/* bit 6 enables NMI */
	interrupt_enable_w(space, 0, data & 0x40);
}

static READ8_HANDLER( catchall )
{
	int pc = cpu_get_pc(space->cpu);

	if (pc != 0x3c74 && pc != 0x0364 && pc != 0x036d)	/* weed out spurious blit reads */
		logerror("%04x: unmapped memory read from %04x\n",pc,offset);

	return 0xff;
}

static READ8_DEVICE_HANDLER( portC_r )
{
	return 4;
}

static WRITE8_HANDLER( banksel_main_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x8000);
}
static WRITE8_HANDLER( banksel_1_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x10000);
}
static WRITE8_HANDLER( banksel_2_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x18000);
}
static WRITE8_HANDLER( banksel_3_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x20000);
}
static WRITE8_HANDLER( banksel_4_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x28000);
}
static WRITE8_HANDLER( banksel_5_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x30000);
}

static WRITE8_HANDLER( banksel_1_1_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x10000);
}
static WRITE8_HANDLER( banksel_2_1_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x14000);
}
static WRITE8_HANDLER( banksel_3_1_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x18000);
}
static WRITE8_HANDLER( banksel_4_1_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x1c000);
}
static WRITE8_HANDLER( banksel_5_1_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x20000);
}
static WRITE8_HANDLER( banksel_1_2_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x12000);
}
static WRITE8_HANDLER( banksel_2_2_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x16000);
}
static WRITE8_HANDLER( banksel_3_2_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x1a000);
}
static WRITE8_HANDLER( banksel_4_2_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x1e000);
}
static WRITE8_HANDLER( banksel_5_2_w )
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x22000);
}

static WRITE8_HANDLER(geimulti_bank_w)
{
	int bank = -1;

	switch(offset + 0x5a00)
	{
		case 0x5a7e: bank = 0; break;
		case 0x5a7d: bank = 1; break;
		case 0x5a7b: bank = 2; break;
		case 0x5a77: bank = 3; break;
		case 0x5a6f: bank = 4; break;
		case 0x5a5f: bank = 5; break;
		case 0x5a3f: bank = 6; break;
		case 0x5c7d: bank = 7; break;
		case 0x5c7b: bank = 8; break;
		case 0x5c77: bank = 9; break;
		case 0x5c6f: bank = 10; break;
		case 0x5c5f: bank = 11; break;
		case 0x5c3f: bank = 12; break;
		case 0x5c7e: bank = 13; break;
		case 0x5aff: case 0x5cff: break;
		default: logerror( "Uknown banking write, offset = %04x, data = %02x\n", offset, data);
	}

	if (bank != -1)
		memory_set_bankptr(space->machine, "bank1", memory_region(space->machine, "bank") + bank*0x8000);
}

static READ8_HANDLER(banksel_1_r)
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x10000);
	return 0x03;
};

static READ8_HANDLER(banksel_2_r)
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x18000);
	return 0x03;
}

static READ8_HANDLER(banksel_3_r)
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x20000);
	return 0x03;
}

static READ8_HANDLER(banksel_4_r)
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x28000);
	return 0x03;
}

static READ8_HANDLER(banksel_5_r)
{
	memory_set_bankptr(space->machine, "bank1",memory_region(space->machine, "maincpu") + 0x30000);
	return 0x03;
}

/* This signature is used to validate the ROMs in sportauth. Simple protection check? */
static int signature_answer,signature_pos;

static READ8_HANDLER( signature_r )
{
	return signature_answer;
}

static WRITE8_HANDLER( signature_w )
{
	if (data == 0) signature_pos = 0;
	else
	{
		static const UINT8 signature[8] = { 0xff, 0x01, 0xfd, 0x05, 0xf5, 0x15, 0xd5, 0x55 };

		signature_answer = signature[signature_pos++];

		signature_pos &= 7;	/* safety; shouldn't happen */
	}
}

static WRITE8_HANDLER( signature2_w )
{
	if (data == 0) signature_pos = 0;
	else
	{
		static const UINT8 signature[8] = { 0xff, 0x01, 0xf7, 0x11, 0xd7, 0x51, 0x57, 0x51 };

		signature_answer = signature[signature_pos++];

		signature_pos &= 7;	/* safety; shouldn't happen */
	}
}

static ADDRESS_MAP_START( getrivia_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0x600f, 0x600f) AM_WRITE(banksel_5_1_w)
	AM_RANGE(0x6017, 0x6017) AM_WRITE(banksel_4_1_w)
	AM_RANGE(0x601b, 0x601b) AM_WRITE(banksel_3_1_w)
	AM_RANGE(0x601d, 0x601d) AM_WRITE(banksel_2_1_w)
	AM_RANGE(0x601e, 0x601e) AM_WRITE(banksel_1_1_w)
	AM_RANGE(0x608f, 0x608f) AM_WRITE(banksel_5_2_w)
	AM_RANGE(0x6097, 0x6097) AM_WRITE(banksel_4_2_w)
	AM_RANGE(0x609b, 0x609b) AM_WRITE(banksel_3_2_w)
	AM_RANGE(0x609d, 0x609d) AM_WRITE(banksel_2_2_w)
	AM_RANGE(0x609e, 0x609e) AM_WRITE(banksel_1_2_w)
	AM_RANGE(0x8000, 0x8002) AM_WRITE(gei_drawctrl_w)
	AM_RANGE(0x8000, 0x9fff) AM_ROM /* space for diagnostic ROM? */
	AM_RANGE(0xa000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAM_WRITE(gei_bitmap_w) AM_BASE_GENERIC(videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gselect_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x40ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x4400, 0x4400) AM_WRITE(banksel_1_1_w)
	AM_RANGE(0x4401, 0x4401) AM_WRITE(banksel_1_2_w)
	AM_RANGE(0x4402, 0x4402) AM_WRITE(banksel_2_1_w)
	AM_RANGE(0x4403, 0x4403) AM_WRITE(banksel_2_2_w)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0x8000, 0x8002) AM_WRITE(gei_drawctrl_w)
	AM_RANGE(0xc000, 0xffff) AM_RAM_WRITE(gei_bitmap_w) AM_BASE_GENERIC(videoram)
ADDRESS_MAP_END

// TODO: where are mapped the lower 0x2000 bytes of the banks?
static ADDRESS_MAP_START( amuse_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0x606f, 0x606f) AM_WRITE(banksel_5_1_w)
	AM_RANGE(0x6077, 0x6077) AM_WRITE(banksel_4_1_w)
	AM_RANGE(0x607b, 0x607b) AM_WRITE(banksel_3_1_w)
	AM_RANGE(0x607d, 0x607d) AM_WRITE(banksel_2_1_w)
	AM_RANGE(0x607e, 0x607e) AM_WRITE(banksel_1_1_w)
	AM_RANGE(0x8000, 0x8002) AM_WRITE(gei_drawctrl_w)
	AM_RANGE(0x8000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAM_WRITE(gei_bitmap_w) AM_BASE_GENERIC(videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gepoker_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0x60ef, 0x60ef) AM_WRITE(banksel_3_1_w)
	AM_RANGE(0x60f7, 0x60f7) AM_WRITE(banksel_2_2_w)
	AM_RANGE(0x60fb, 0x60fb) AM_WRITE(banksel_2_1_w)
	AM_RANGE(0x60fd, 0x60fd) AM_WRITE(banksel_1_2_w)
	AM_RANGE(0x60fe, 0x60fe) AM_WRITE(banksel_1_1_w)
	AM_RANGE(0x8000, 0x8002) AM_WRITE(gei_drawctrl_w)
	AM_RANGE(0x8000, 0xbfff) AM_ROM /* space for diagnostic ROM? */
	AM_RANGE(0xe000, 0xffff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAM_WRITE(gei_bitmap_w) AM_BASE_GENERIC(videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( amuse1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x4400, 0x4400) AM_WRITE(banksel_1_1_w)
	AM_RANGE(0x4401, 0x4401) AM_WRITE(banksel_2_1_w)
	AM_RANGE(0x4402, 0x4402) AM_WRITE(banksel_3_1_w)
	AM_RANGE(0x4403, 0x4403) AM_WRITE(banksel_4_1_w)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5800, 0x5fff) AM_ROM
	AM_RANGE(0x8000, 0x8002) AM_WRITE(gei_drawctrl_w)
	AM_RANGE(0x8000, 0xbfff) AM_ROM /* space for diagnostic ROM? */
	AM_RANGE(0xe000, 0xffff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAM_WRITE(gei_bitmap_w) AM_BASE_GENERIC(videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( findout_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r,ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r,ppi8255_w)
	/* banked ROMs are enabled by low 6 bits of the address */
	AM_RANGE(0x601f, 0x601f) AM_WRITE(banksel_main_w)
	AM_RANGE(0x602f, 0x602f) AM_WRITE(banksel_5_w)
	AM_RANGE(0x6037, 0x6037) AM_WRITE(banksel_4_w)
	AM_RANGE(0x603b, 0x603b) AM_WRITE(banksel_3_w)
	AM_RANGE(0x603d, 0x603d) AM_WRITE(banksel_2_w)
	AM_RANGE(0x603e, 0x603e) AM_WRITE(banksel_1_w)
	AM_RANGE(0x6200, 0x6200) AM_WRITE(signature_w)
	AM_RANGE(0x6400, 0x6400) AM_READ(signature_r)
	AM_RANGE(0x7800, 0x7fff) AM_ROM /*space for diagnostic ROM?*/
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x8002) AM_WRITE(gei_drawctrl_w)
	AM_RANGE(0xc000, 0xffff) AM_WRITE(gei_bitmap_w)  AM_BASE_GENERIC(videoram)
	AM_RANGE(0x0000, 0xffff) AM_READ(catchall)
ADDRESS_MAP_END

static ADDRESS_MAP_START( quizvid_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r,ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r,ppi8255_w)
	/* banked ROMs are enabled by low 6 bits of the address */
	AM_RANGE(0x602f, 0x602f) AM_READ(banksel_5_r)
	AM_RANGE(0x6037, 0x6037) AM_READ(banksel_4_r)
	AM_RANGE(0x603b, 0x603b) AM_READ(banksel_3_r)
	AM_RANGE(0x603d, 0x603d) AM_READ(banksel_2_r)
	AM_RANGE(0x603e, 0x603e) AM_READ(banksel_1_r)
	AM_RANGE(0x7800, 0x7fff) AM_ROM /*space for diagnostic ROM?*/
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x8002) AM_WRITE(gei_drawctrl_w)
	AM_RANGE(0xc000, 0xffff) AM_WRITE(gei_bitmap_w)  AM_BASE_GENERIC(videoram)
	AM_RANGE(0x0000, 0xffff) AM_READ(catchall)
ADDRESS_MAP_END

static ADDRESS_MAP_START( suprpokr_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0x6200, 0x6200) AM_WRITE(signature2_w)
	AM_RANGE(0x6400, 0x6400) AM_READ(signature_r)
	AM_RANGE(0x8000, 0x8002) AM_WRITE(gei_drawctrl_w)
	AM_RANGE(0x8000, 0xbfff) AM_ROM /* space for diagnostic ROM? */
	AM_RANGE(0xe000, 0xffff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAM_WRITE(gei_bitmap_w) AM_BASE_GENERIC(videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( geimulti_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5800, 0x5fff) AM_ROM
	AM_RANGE(0x5a00, 0x5cff) AM_WRITE(geimulti_bank_w)
	AM_RANGE(0x6000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8002) AM_WRITE(gei_drawctrl_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xffff) AM_RAM_WRITE(gei_bitmap_w) AM_BASE_GENERIC(videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sprtauth_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x4800, 0x4803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0x5600, 0x5600) AM_READ(signature_r)
	AM_RANGE(0x5800, 0x5800) AM_WRITE(signature_w)
	AM_RANGE(0x5a00, 0x5cff) AM_WRITE(geimulti_bank_w)
	AM_RANGE(0x6000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8002) AM_WRITE(gei_drawctrl_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xffff) AM_RAM_WRITE(gei_bitmap_w) AM_BASE_GENERIC(videoram)
ADDRESS_MAP_END

static INPUT_PORTS_START(reelfun_standard)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1 Left A-Z")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("2 Right A-Z")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("3 Select Letter")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("4 Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("5 Solve Puzzle")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START(trivia_standard)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("ticket", ticket_dispenser_line_r)		/* ticket status */
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( gselect )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Poker: Discard Cards" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x06, 0x06, "Poker: Pay on" )
	PORT_DIPSETTING(    0x06, "any Pair" )
	PORT_DIPSETTING(    0x04, "Pair of Eights or better" )
	PORT_DIPSETTING(    0x02, "Pair of Jacks or better" )
	PORT_DIPSETTING(    0x00, "Pair of Aces only" )
	PORT_DIPNAME( 0x08, 0x00, "Maximum Bet" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x10, 0x10, "Poker: Credits needed for 2 Jokers" )
	PORT_DIPSETTING(    0x10, "8" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xe0, 0x80, "Payout Percentage" )
	PORT_DIPSETTING(    0xe0, "35" )
	PORT_DIPSETTING(    0xc0, "40" )
	PORT_DIPSETTING(    0xa0, "45" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x60, "55" )
	PORT_DIPSETTING(    0x40, "60" )
	PORT_DIPSETTING(    0x20, "65" )
	PORT_DIPSETTING(    0x00, "70" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_IMPULSE(2) PORT_NAME("Button 12 ?")
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(2) PORT_NAME ("Payout")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_IMPULSE(2) PORT_NAME ("Play / Raise")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_IMPULSE(2) PORT_NAME ("Deal")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_IMPULSE(2) PORT_NAME ("Cancel")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_IMPULSE(2) PORT_NAME ("Stand")
/*  Button 8, 6, 7 order verified in test mode switch test */

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( gepoker )
	PORT_INCLUDE( gselect )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no coin 2 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no button 12 */
INPUT_PORTS_END

static INPUT_PORTS_START( getrivia )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x01, "Questions" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x01, "5" )
/*  PORT_DIPSETTING(    0x02, "5" )*/
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, "Show Answer" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Max Coins" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPNAME( 0x10, 0x00, "Timeout" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Tickets" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "No Coins" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_CONDITION("DSWA", 0x40, PORTCOND_EQUALS, 0x40)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_IMPULSE(2) PORT_CONDITION("DSWA", 0x40, PORTCOND_EQUALS, 0x00) PORT_NAME ("Start in no coins mode")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_CONDITION("DSWA", 0x40, PORTCOND_EQUALS, 0x40)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CONDITION("DSWA", 0x40, PORTCOND_EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("ticket", ticket_dispenser_line_r)		/* ticket status */
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( sextriv1 )
	PORT_INCLUDE( getrivia )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no coin 2 */
INPUT_PORTS_END

static INPUT_PORTS_START(suprpokr)
	PORT_INCLUDE(gepoker)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Screen" )
	PORT_DIPSETTING(    0x10, "Horizontal" )
	PORT_DIPSETTING(    0x00, "Vertical" )
	PORT_DIPNAME( 0x20, 0x20, "Percentage" )
	PORT_DIPSETTING(    0x20, "Dynamic" )
	PORT_DIPSETTING(    0x00, "Actual" )
	PORT_DIPNAME( 0x40, 0x40, "Hopper" )
	PORT_DIPSETTING(    0x40, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )
	PORT_DIPNAME( 0x80, 0x80, "If RAM Error" )
	PORT_DIPSETTING(    0x80, "Play" )
	PORT_DIPSETTING(    0x00, "Freeze" )
INPUT_PORTS_END

static INPUT_PORTS_START( reelfun )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x01, "Coinage Multiplier" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING( 0x08, "Credits per Coin" )
	PORT_DIPSETTING( 0x00, "Coins per Credit" )
	PORT_DIPNAME( 0x10, 0x10, "Screen" )
	PORT_DIPSETTING( 0x10, "Horizontal" )
	PORT_DIPSETTING( 0x00, "Vertical" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_INCLUDE(reelfun_standard)
INPUT_PORTS_END

static INPUT_PORTS_START( findout )
	PORT_START("DSWA")		/* DSW A */
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x00, "Game Repetition" )
	PORT_DIPSETTING( 0x08, DEF_STR( No ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Orientation" )
	PORT_DIPSETTING( 0x10, "Horizontal" )
	PORT_DIPSETTING( 0x00, "Vertical" )
	PORT_DIPNAME( 0x20, 0x20, "Buy Letter" )
	PORT_DIPSETTING( 0x20, DEF_STR( No ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Starting Letter" )
	PORT_DIPSETTING( 0x40, DEF_STR( No ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Bonus Letter" )
	PORT_DIPSETTING( 0x80, DEF_STR( No ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )

	PORT_INCLUDE(reelfun_standard)
INPUT_PORTS_END

static INPUT_PORTS_START( gt103 )
	PORT_START("DSWA")		/* DSW A */
	PORT_DIPNAME( 0x07, 0x01, "Coinage Multiplier" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING( 0x08, "Credits per Coin" )
	PORT_DIPSETTING( 0x00, "Coins per Credit" )
	PORT_DIPNAME( 0x10, 0x10, "Screen" )
	PORT_DIPSETTING( 0x10, "Horizontal" )
	PORT_DIPSETTING( 0x00, "Vertical" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_INCLUDE(trivia_standard)
INPUT_PORTS_END

static INPUT_PORTS_START( gt103a )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x01, "Questions" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x01, "5" )
//  PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, "Show Answer" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Max Coins" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x10, 0x00, "Timeout" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Tickets" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "No Coins" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_CONDITION("DSWA", 0x40, PORTCOND_EQUALS, 0x40)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_IMPULSE(2) PORT_CONDITION("DSWA", 0x40, PORTCOND_EQUALS, 0x00) PORT_NAME ("Start in no coins mode")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_CONDITION("DSWA", 0x40, PORTCOND_EQUALS, 0x40)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CONDITION("DSWA", 0x40, PORTCOND_EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("ticket", ticket_dispenser_line_r)		/* ticket status */
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( quiz )
	PORT_INCLUDE( gt103a )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	/* no tickets */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no coin 2 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no tickets */
INPUT_PORTS_END

static INPUT_PORTS_START( gt507uk )
	PORT_START("DSWA")		/* DSW A */
	PORT_DIPNAME( 0x01, 0x00, "If Ram Error" )
	PORT_DIPSETTING( 0x01, "Freeze" )
	PORT_DIPSETTING( 0x00, "Play" )
	PORT_DIPNAME( 0x0a, 0x08, "Payout" )
	PORT_DIPSETTING( 0x0a, "Bank" )
	PORT_DIPSETTING( 0x08, "N/A" )
	PORT_DIPSETTING( 0x02, "Credit" )
	PORT_DIPSETTING( 0x00, "Direct" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Hardware" )
	PORT_DIPSETTING( 0x04, "Hopper" )
	PORT_DIPSETTING( 0x00, "Solenoid" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_INCLUDE(trivia_standard)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* coin 3, 2, 4 order verified in test mode */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START(geimulti)
	PORT_INCLUDE(gselect)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x0a, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_7C ) )
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

static INPUT_PORTS_START(sprtauth)
	PORT_INCLUDE(getrivia)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("ticket", ticket_dispenser_line_r)		/* ticket status */
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x0a, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_7C ) )
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

static const ppi8255_interface getrivia_ppi8255_intf[2] =
{
	{
		DEVCB_INPUT_PORT("DSWA"),	/* Port A read */
		DEVCB_INPUT_PORT("IN0"),	/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_HANDLER(sound_w)		/* Port C write */
	},
	{
		DEVCB_INPUT_PORT("IN1"),	/* Port A read */
		DEVCB_NULL,					/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_HANDLER(lamps_w),		/* Port B write */
		DEVCB_HANDLER(lamps2_w)		/* Port C write */
	}
};

static const ppi8255_interface gselect_ppi8255_intf[2] =
{
	{
		DEVCB_INPUT_PORT("DSWA"),	/* Port A read */
		DEVCB_INPUT_PORT("IN0"),	/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_HANDLER(sound2_w)				/* Port C write */
	},
	{
		DEVCB_INPUT_PORT("IN1"),	/* Port A read */
		DEVCB_NULL,					/* Port B read */
		DEVCB_INPUT_PORT("IN2"),	/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_HANDLER(lamps_w),		/* Port B write */
		DEVCB_HANDLER(nmi_w)		/* Port C write */
	}
};

static const ppi8255_interface findout_ppi8255_intf[2] =
{
	{
		DEVCB_INPUT_PORT("DSWA"),		/* Port A read */
		DEVCB_INPUT_PORT("IN0"),		/* Port B read */
		DEVCB_NULL,						/* Port C read */
		DEVCB_NULL,						/* Port A write */
		DEVCB_NULL,						/* Port B write */
		DEVCB_HANDLER(sound_w),			/* Port C write */
	},
	{
		DEVCB_INPUT_PORT("IN1"),		/* Port A read */
		DEVCB_NULL,						/* Port B read */
		DEVCB_HANDLER(portC_r),			/* Port C read */
		DEVCB_NULL,						/* Port A write */
		DEVCB_HANDLER(lamps_w),			/* Port B write */
		DEVCB_NULL						/* Port C write */
	}
};

static MACHINE_DRIVER_START( getrivia )
	MDRV_CPU_ADD("maincpu",Z80,4000000) /* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(getrivia_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(48, 511-48, 16, 255-16)

	MDRV_PALETTE_LENGTH(8)
	MDRV_PALETTE_INIT(gei)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_VIDEO_START(generic_bitmapped)
	MDRV_VIDEO_UPDATE(generic_bitmapped)

	MDRV_PPI8255_ADD( "ppi8255_0", getrivia_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", getrivia_ppi8255_intf[1] )
	MDRV_TICKET_DISPENSER_ADD("ticket", 100, TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( findout )
	MDRV_IMPORT_FROM(getrivia)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(findout_map)

	MDRV_PPI8255_RECONFIG( "ppi8255_0", findout_ppi8255_intf[0] )
	MDRV_PPI8255_RECONFIG( "ppi8255_1", findout_ppi8255_intf[1] )
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( quizvid )
	MDRV_IMPORT_FROM(findout)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(quizvid_map)

	MDRV_PALETTE_INIT(quizvid)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( gselect )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(getrivia)

	MDRV_DEVICE_REMOVE("ticket")

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(gselect_map)

	MDRV_PPI8255_RECONFIG( "ppi8255_0", gselect_ppi8255_intf[0] )
	MDRV_PPI8255_RECONFIG( "ppi8255_1", gselect_ppi8255_intf[1] )
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( jokpokera )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(getrivia)

	MDRV_DEVICE_REMOVE("ticket")

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(gselect_map)

	MDRV_PPI8255_RECONFIG( "ppi8255_0", gselect_ppi8255_intf[0] )
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( amuse )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(getrivia)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(amuse_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( gepoker )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(getrivia)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(gepoker_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( amuse1 )

	MDRV_IMPORT_FROM(getrivia)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(amuse1_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( suprpokr )

	MDRV_IMPORT_FROM(getrivia)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(suprpokr_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( geimulti )

	MDRV_IMPORT_FROM(getrivia)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(geimulti_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( sprtauth )

	MDRV_IMPORT_FROM(getrivia)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(sprtauth_map)
MACHINE_DRIVER_END

/***************************************************
Rom board is UVM-1A

Contains:
 3 2732  eproms (Program Code, 1 empty socket)
 2 X2212P (Ram chips, no battery backup)
 DM7408N

PCB labeled M075

****************************************************/

ROM_START( jokpoker )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "m075.1", 0x00000, 0x1000, CRC(ad42465b) SHA1(3f06847a9aecb0592f99419dba9be5f18005d57b) ) /* rom board UMV-1A */
	ROM_LOAD( "m075.2", 0x01000, 0x1000, CRC(bd129fc2) SHA1(2e05ba34922c16d127be32941447013efea05bcd) )
	ROM_LOAD( "m075.3", 0x02000, 0x1000, CRC(45725bc9) SHA1(9e6dcbec955ef8190f2307ddb367b24b7f34338d) )
ROM_END

ROM_START( jokpokera )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "jpbiwr930-1.bin", 0x00000, 0x2000, CRC(d0f4fec5) SHA1(5fcc72522df66464759d5ba3d5209bc7a3a80002) )  /* rom board UMV-7C */
	ROM_LOAD( "jpbiwr930-2.bin", 0x02000, 0x2000, CRC(824d1aee) SHA1(6eebde351c3b5bbed3796846d8d651b41ed6f84a) )
ROM_END

/*
Joker Poker BI Version 16.04BI 10-19-88

Standard GEI mainboard

UVM 7S REV A romboard
*/

ROM_START( jokpokerb )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "jp_bi_10-19-88.e1", 0x00000, 0x2000, CRC(d59a78e2) SHA1(d8463675f30a52e0f93c5ea5c2ee663095d3d5ea) )
	ROM_LOAD( "jp_bi_10-19-88.e2", 0x02000, 0x2000, CRC(1a34dc80) SHA1(27dff743e661ae7421fef0b046e3ae205b842603) )
ROM_END

/*
Joker Poker BI Version 16.03BI 5-10-85

Standard GEI mainboard

UVM7-C romboard
*/

ROM_START( jokpokerc )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "jp_bi_5-10-85.1", 0x00000, 0x2000, CRC(d0f4fec5) SHA1(5fcc72522df66464759d5ba3d5209bc7a3a80002) )
	ROM_LOAD( "jp_bi_5-10-85.2", 0x02000, 0x2000, CRC(9f8bee22) SHA1(8d894d2a07bd18d731b7a54a16bb9b9230c79306) )
ROM_END

/***************************************************
Rom board is UVM-1B

Contains:
 4 2732  eproms (Program Code)
 Battery (3.5V litium battery) backed up NEC 8444XF301
 DM7408N

****************************************************/

ROM_START( superbwl )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "super_bowl.1", 0x00000, 0x1000, CRC(82edf064) SHA1(8a26377590282f51fb39d013452ba11252e7dd05) ) /* rom board UMV-1B */
	ROM_LOAD( "super_bowl.2", 0x01000, 0x1000, CRC(2438dd1f) SHA1(26bbd1cb3d0d5b93f61b92ff95948ac9de060715) )
	ROM_LOAD( "super_bowl.3", 0x02000, 0x1000, CRC(9b111430) SHA1(9aaa755f3e4b369477c1a0525c994a19fe0f6107) )
	ROM_LOAD( "super_bowl.4", 0x03000, 0x1000, CRC(037cad42) SHA1(d4037a28bb49b31358b5d560e5e028d958ae2bc9) )
ROM_END

/***************************************************
Rom board is UVM 10 B

Contains:
 2 2764  eproms (Program Code)
 5 27128 eproms (Question roms)
 Battery (3V litium battery) backed up HM6117P-4
 SN74LS374
 MMI PAL10L8


Sets will be listed by "series" - the program code version
 is not as important as maintaining the correct questions
 sets as per known series
Missing sets will be filled as dumped, as question roms
 are interchangable, operators did thier own swaps

Note: Question roms that contain "#1" (or 2 ect)
      are corrected roms (spelling and / or answers)

****************************************************/

ROM_START( gtsers1 ) /* Series 1 (Complete) */
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog101c_right", 0x00000, 0x2000, CRC(767f0e46) SHA1(5de7b54876fcbfb2328174ffe6b656ffea886fcb) ) /* rom board UMV 10 B */
	ROM_LOAD( "prog101c_left",  0x0a000, 0x2000, CRC(24c0a097) SHA1(b8de58baecb92775e0882cd6eca3b9e07cf7c5a5) )
	/* Question roms */
	ROM_LOAD( "science_#1",    0x10000, 0x4000, CRC(68259e09) SHA1(29e848b4744b767c51ff81a756fba7bf96daefec) )
	ROM_LOAD( "general_#1",    0x14000, 0x4000, CRC(25a0ef9d) SHA1(793abd779cc237e14933933747bbf27bbcbfcd32) )
	ROM_LOAD( "sports_#1",     0x18000, 0x4000, CRC(cb1744f5) SHA1(ea3f7bfcecf5c58c26aa0f34908ba5d54f7279ec) )
	ROM_LOAD( "music_#1",      0x1c000, 0x4000, CRC(1b546857) SHA1(31e04bb5016e8ef6dc48f9b3ddaeab5fe04f91c2) )
	ROM_LOAD( "movies-tv_#1",  0x20000, 0x4000, CRC(e9a55dad) SHA1(c87682e72bad3507b24eb6a52b4e430e0bfcdab6) )
ROM_END

ROM_START( gtsers2 ) /* Series 2 (Complete - question roms dated 2/9) */
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog101c_right", 0x00000, 0x2000, CRC(767f0e46) SHA1(5de7b54876fcbfb2328174ffe6b656ffea886fcb) ) /* rom board UMV 10 B */
	ROM_LOAD( "prog101c_left",  0x0a000, 0x2000, CRC(24c0a097) SHA1(b8de58baecb92775e0882cd6eca3b9e07cf7c5a5) )
	/* Question roms */
	ROM_LOAD( "tv_mash",          0x10000, 0x4000, CRC(a86990fc) SHA1(6a11b038d48bb97feb4857546349ed93ea1f9273) )
	ROM_LOAD( "general_2",        0x14000, 0x4000, CRC(5798f2b3) SHA1(0636017969d9b1eac5d33cfb18cb36f7cf4cba88) )
	ROM_LOAD( "sports_2_#2",      0x18000, 0x4000, CRC(fb632622) SHA1(c14d8178f5cfc5994e2ab4f829e353fa75b57304) )
	ROM_LOAD( "comics_#1",        0x1c000, 0x4000, CRC(8c5cd561) SHA1(1ca566acf72ce636b1b34ee6b7cafb9584340bcc) )
	ROM_LOAD( "entertainment_#1", 0x20000, 0x4000, CRC(cd3ce4c7) SHA1(4bd121fa5899a96b015605f84179ed82be0a25f3) ) /* Correct spelling of "Acapella" */
ROM_END

ROM_START( gtsers3 ) /* Series 3 (Complete - question roms dated 3/9) */
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog102b_right",   0x00000, 0x2000, CRC(e8391f71) SHA1(a955eff87d622d4fcfd25f6d888c48ff82556879) )
	ROM_LOAD( "prog102b_left",    0x0a000, 0x2000, CRC(cc7b45a7) SHA1(c708f56feb36c1241358a42bb7dce25b799f1f0b) )
	/* Question roms */
	ROM_LOAD( "beatlemania_#1", 0x10000, 0x4000, CRC(c35ab539) SHA1(aa7c9b532aeb289b71c179e6ff1cc5b63dbe240c) )
	ROM_LOAD( "general_3",      0x14000, 0x4000, CRC(a60f17a4) SHA1(0d79be9e2e49b9817e94d410e25bb6dcda10aa9e) )
	ROM_LOAD( "sports_3_#3",    0x18000, 0x4000, CRC(b22cec38) SHA1(a416c3de9749fda3ab5ae5841304da0cef900cbf) )
	ROM_LOAD( "country-west",   0x1c000, 0x4000, CRC(3227c475) SHA1(d07ad4876122223fe7ab3f21781e0d847332ea5c) )
	ROM_LOAD( "tv_soaps",       0x20000, 0x4000, CRC(26914f3a) SHA1(aec380cea14d6acb71986f3d65c7620b16c174ae) )
ROM_END

ROM_START( gtsers4 ) /* Series 4 (Incomplete - question roms dated 4/9) */
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	/* Question roms */
	ROM_LOAD( "history-geog",   0x10000, 0x4000, CRC(76d6b026) SHA1(613809b247cb27773631a1bb34af485c2b1bd486) )
	ROM_LOAD( "star_trek",      0x14000, 0x4000, CRC(19764e00) SHA1(d7ed577dba02776ac58e8f34b833ed07679c0af1) )
	ROM_LOAD( "television_#1",  0x18000, 0x4000, CRC(0f646389) SHA1(23fefe2e6cc26767d52604e7ab15bb4db99a6e94) )
	/* Missing "baseball" */
	/* Missing "hollywood" */
ROM_END

ROM_START( gtsers5 ) /* Series 5 (Incomplete - question roms dated 5/9) */
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	/* Question roms */
	ROM_LOAD( "james_bond",    0x10000, 0x4000, CRC(fe9fadfd) SHA1(44b3fee1f14148f47b0b40600aabd5bff9b65e85) )
	ROM_LOAD( "hockey",        0x14000, 0x4000, CRC(4874a431) SHA1(f3c11dfbf71d101aa1a6cd3622b282a4ebe4664b) )
	/* Missing "the_states" */
	/* Missing "wild_west" */
	/* Missing "elvismania" */
ROM_END

ROM_START( gtsers7 ) /* Series 7 (Incomplete - question roms dated 7/9?) */
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	/* Question roms */
	ROM_LOAD( "general_5",     0x10000, 0x4000, CRC(81bf07c7) SHA1(a53f050b4ef8ffc0499b50224d4bbed4af0ca09c) )
	/* Missing "tv_dallas" */
	/* Missing "kids_korner" */
	/* Missing "good_guys" */
	/* Missing "biblical" */
ROM_END

ROM_START( gtsersa ) /* alt or older version questions */
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	/* Question roms */
	ROM_LOAD( "sports",               0x10000, 0x4000, CRC(9b4a17b6) SHA1(1b5358b5bc83c2817ecfa4e277fa351a679d5023) ) /* Series 1 question */
	ROM_LOAD( "entertainment_#1_old", 0x14000, 0x4000, CRC(2bffb3b4) SHA1(5947ebd708df35cefa86608392909c16b25d0710) ) /* Dated 2/9 - Spells "Acapella" as "Cappella" */
	ROM_LOAD( "sports_2",             0x18000, 0x4000, CRC(e8f8e168) SHA1(d2bc57dc0799dd8817b15857f17c4d7ee4d9f932) ) /* Dated 2/9 */
	ROM_LOAD( "comics",               0x1c000, 0x4000, CRC(7efdfe8f) SHA1(ec255777c61677ca32c49b9da5e85e07c0647e5f) ) /* Dated 2/9 */
	ROM_LOAD( "entertainment",        0x20000, 0x4000, CRC(b670b9e8) SHA1(0d2246fcc6c753694bc9bd1fc05ac439f24059ef) ) /* Dated 2/9 */
ROM_END

ROM_START( gtsersb ) /* alt or older version questions */
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog102c_right", 0x00000, 0x2000, CRC(76fdc3a3) SHA1(212e09644b9cab334aad22ec5860e8638c6ba3fa) )
	ROM_LOAD( "prog102c_left",  0x0a000, 0x2000, CRC(901fb2f9) SHA1(98e49c74d89c4911a1f4d5ccf3e6cf3226c6a178) )
	/* Question roms */
	ROM_LOAD( "beatlemania",    0x10000, 0x4000, CRC(cb241960) SHA1(e560b776b2cb5fd29d1663fffdf68f4427d674a9) ) /* Dated 3/9 */
	ROM_LOAD( "sports_3",       0x14000, 0x4000, CRC(5986996c) SHA1(56432c15a3b0204ed527c18e24716f17bb52dc4e) ) /* Dated 3/9 */
	ROM_LOAD( "television",     0x18000, 0x4000, CRC(413f34c8) SHA1(318f6b464449bf3f0c43c4210a667190c774eb67) ) /* Dated 4/9 */
	ROM_LOAD( "sex_triv",       0x1c000, 0x4000, CRC(cd0ce4e2) SHA1(2046ee3da94f00bf4a8b3fc62b1190d58e83cc89) ) /* Dated 7/9 - likely an alt series 7 question set */
	ROM_LOAD( "facts_of_life",  0x20000, 0x4000, CRC(1668c7bf) SHA1(6bf43de26f8a626560579ab75fd0890fe00f99dd) ) /* Uknown series question set */
ROM_END

ROM_START( sextriv1 )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog1_right",   0x00000, 0x2000, CRC(73abcd12) SHA1(3b985f25a11507878cef6d11420e215065fb0906) )
	ROM_LOAD( "prog1_left",    0x0a000, 0x2000, CRC(04ee6ecd) SHA1(28342fcdcf36b34fa93f1a985163ca5aab03defe) )
	/* Question roms */
	ROM_LOAD( "adult_sex",    0x10000, 0x4000, CRC(509a8183) SHA1(635c784860e423b22aaea94abc53c1d9477cb1df) )
	ROM_LOAD( "arousing_sex", 0x14000, 0x4000, CRC(1dbf4578) SHA1(51a548d5fe59739e62b5f0e9e6ebc7deb8656210) )
	ROM_LOAD( "intimate_sex", 0x18000, 0x4000, CRC(1f46b626) SHA1(04aa5306c69d130e0f84fa390a773e73c06e5e9b) )
	ROM_LOAD( "sizzling_sex", 0x1c000, 0x4000, CRC(c718833d) SHA1(02ea341e56554dd9302fe95f45dcf446a2978917) )
ROM_END

ROM_START( sextriv2 )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "prog1_right",     0x00000, 0x2000, CRC(73abcd12) SHA1(3b985f25a11507878cef6d11420e215065fb0906) )
	ROM_LOAD( "prog1_left",      0x0a000, 0x2000, CRC(04ee6ecd) SHA1(28342fcdcf36b34fa93f1a985163ca5aab03defe) )
	/* Question roms */
	ROM_LOAD( "general_sex",     0x10000, 0x4000, CRC(36fed946) SHA1(25d445ab6cb4b6f41a1dd7104ee1141e597b2e9e) )
	ROM_LOAD( "educational_sex", 0x14000, 0x4000, CRC(281cbe03) SHA1(9c3900cd2535f942a5cbae7edd46ac0170e04c52) )
	ROM_LOAD( "novelty_sex",     0x18000, 0x4000, CRC(26603979) SHA1(78061741e5224b3162be51e637a2fbb9a48962a3) )
ROM_END

/***************************************************
Rom board is UVM-4B

Contains 5 2764 eproms, MMI PAL16R4CN

Battery (3V litium battery) backed up HM6117P-4

Roms labeled as:

4/1  at spot 1
BLJK at spot 2
POKR at spot 3
SPRD at spot 4
SLOT at spot 3

Alt set included BONE in place of SPRD & a newer SLOT

These board sets may also be known as the V116 (or V16)
sets as the alt set also included that name on the labels

****************************************************/

ROM_START( gs4002 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "4-1.1",          0x00000, 0x2000, CRC(a456e456) SHA1(f36b96ac31ce0f128ecb94f94d1dbdd88ee03c77) ) /* V16M 5-4-84 */
	/* Banked roms */
	ROM_LOAD( "bljk_3-16-84.2", 0x10000, 0x2000, CRC(c3785523) SHA1(090f324fc7adb0a36b189cf04086f0e050895ee4) )
	ROM_LOAD( "pokr_5-16-84.3", 0x12000, 0x2000, CRC(f0e99cc5) SHA1(02fdc95974e503b6627930918fcc3c029a7a4612) )
	ROM_LOAD( "sprd_1-24-84.4", 0x14000, 0x2000, CRC(5fe90ed4) SHA1(38db69567d9c38f78127e581fdf924aca4926378) )
	ROM_LOAD( "slot_1-24-84.5", 0x16000, 0x2000, CRC(cd7cfa4c) SHA1(aa3de086e5a1018b9e5a18403a6144a6b0ed1036) )
ROM_END

ROM_START( gs4002a )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "4-1.1",          0x00000, 0x2000, CRC(a456e456) SHA1(f36b96ac31ce0f128ecb94f94d1dbdd88ee03c77) ) /* V16M 5-4-84 */
	/* Banked roms */
	ROM_LOAD( "bljk_3-16-84.2", 0x10000, 0x2000, CRC(c3785523) SHA1(090f324fc7adb0a36b189cf04086f0e050895ee4) )
	ROM_LOAD( "pokr_5-16-84.3", 0x12000, 0x2000, CRC(f0e99cc5) SHA1(02fdc95974e503b6627930918fcc3c029a7a4612) )
	ROM_LOAD( "bone_5-16-84.4", 0x14000, 0x2000, CRC(eccd2fb0) SHA1(2683e432ffcca4280c31f57b2596e4389bc59b7b) )
	ROM_LOAD( "slot_9-24-84.5", 0x16000, 0x2000, CRC(25d8c504) SHA1(2d52b66e8a1f06f486015440668bd924a123dad0) )
ROM_END

/*
Greyhound Poker board...

Standard Greyhound Electronics Inc UV-1B mainboard.

Rom board UVM 10 B or UMV 10 C

Battery backed up NEC D449C ram
PAL16R4
74L374

Roms in this order on the UMV 10 C board:

Label        Normaly in the slot
--------------------------------
High
Control
rom1         Joker Poker
rom2         Black jack
rom3         Rolling Bones
rom4         Casino Slots
rom5         Horse Race

For UMV 10 B: 1C, 2C, 1, 2, 3, 4, & 5

There looks to be several types and combos for these, some are "ICB" or "IAM"
It also looks like operators mixed & matched to upgrade (some times incorrectly)
their boards.  Sets will be filled in as found and dumped.

There are some versions, like, the ICB sets that use 2764 roms for all roms. While the IAM set uses
27128 roms for all roms.  These roms are the correct size, but it's currently not known if the rom
board (UVM 10 B/C) "sees" them as 27128 or the standard size of 2764.

Dumped, but not known to be supported by any High/Control combo:
ROM_LOAD( "rollingbones_am_3-16-84",  0x16000, 0x4000, CRC(41879e9b) SHA1(5106d5772bf43b28817e27efd16c785359cd929e) ) // Might work with IAM control, once it gets figured out

The ICB set is known as the M105 set as some label sets included that name.

*/

ROM_START( gepoker ) /* v50.02 with most roms for ICB dated 8-16-84 */
	ROM_REGION( 0x1b000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "control_icb_8-16",    0x00000, 0x2000, CRC(0103963d) SHA1(9bc646e721048b84111e0686eaca23bc24eee3e2) )
	ROM_LOAD( "high_icb_6-25-85-5",  0x0e000, 0x2000, CRC(dfb6592e) SHA1(d68de9f537d3c14279dc576424d195bb266e3897) )
	/* Banked roms */
	ROM_LOAD( "jokerpoker_icb_8-16-84",    0x10000, 0x2000, CRC(0834a1e6) SHA1(663e6f4e0586eb9b84d3098aef8c596585c27304) )
	ROM_LOAD( "blackjack_icb_8-16-84",     0x12000, 0x2000, CRC(cff27ffd) SHA1(fd85b54400b2f22ae92042b01a2c162e64d2d066) )
	ROM_LOAD( "rollingbones_icb_8-16-84",  0x14000, 0x2000, CRC(52d66cb6) SHA1(57db34906fcafd37f3a361df209dafe080aeac16) )
	ROM_LOAD( "casinoslots_icb_8-16-84",   0x16000, 0x2000, CRC(3db002a3) SHA1(7dff4efceee37b25328303cf0606bf4baa4df5f3) )
	ROM_LOAD( "horserace_icb_3-19-85",     0x18000, 0x2000, CRC(f1e6e61e) SHA1(944b1ab4af911e5ed136f1fca3c44219726eeebb) )
ROM_END

ROM_START( gepoker1 ) /* v50.02 with roms for ICB dated 9-30-86 */
	ROM_REGION( 0x1b000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "control_icb_9-30",    0x00000, 0x2000, CRC(08b996f2) SHA1(5f5efb5015ec9571cc94734c18debfadaa28f585) )
	ROM_LOAD( "high_icb_6-25-85-5a", 0x0e000, 0x2000, CRC(6ddc1750) SHA1(ee19206b7f4a98e3e7647414127f4e09b3e9134f) )
	/* Banked roms */
	ROM_LOAD( "jokerpoker_icb_9-30-86",    0x10000, 0x2000, CRC(a1473367) SHA1(9b37ccafc02704e8f1d61150326494e86148d84e) )
	ROM_LOAD( "blackjack_icb_9-30-86",     0x12000, 0x2000, CRC(82804184) SHA1(2e2e6a80c99c8eb226dc54c1d32d0bf24de300a4) )
	ROM_LOAD( "casinoslots_icb_9-30-86",   0x14000, 0x2000, CRC(713c3963) SHA1(a9297c04fc44522ca6891516a2c744712132896a) )
	ROM_LOAD( "beatthespread_icb_9-30-86", 0x16000, 0x2000, CRC(93654d2a) SHA1(3aa5a54b91867c03182e93a7f1607545503a33f7) )
	ROM_LOAD( "instantbingo_t24_10-07-86", 0x18000, 0x2000, CRC(de87ed0a) SHA1(4a26d93368c1a39dd38aabe450c34203101f0ef7) ) /* Found with this set, is it compatible or an operater swap? */
ROM_END

ROM_START( gepoker2 ) /* v50.02 with control dated 9-30-84 */
	ROM_REGION( 0x1b000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "control_icb_9-30",  0x00000, 0x2000, CRC(08b996f2) SHA1(5f5efb5015ec9571cc94734c18debfadaa28f585) )
	ROM_LOAD( "high_icb_6-25a",    0x0e000, 0x2000, CRC(6ddc1750) SHA1(ee19206b7f4a98e3e7647414127f4e09b3e9134f) )
	/* Banked roms */
	ROM_LOAD( "jokerpoker_cb_10-19-88",    0x10000, 0x2000, CRC(a590af75) SHA1(63bc64fbc9ac0c489b1f4894d77a4be13d7251e7) )
	ROM_LOAD( "horserace_icb_1-1-87",      0x12000, 0x2000, CRC(6d5092e3) SHA1(ef99d1b858aef3c438c61c2b17e371dc6aca6623) )
ROM_END

ROM_START( amuse ) /* v50.08 with most roms for IAM dated 8-16-84 */
	ROM_REGION( 0x24000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "control_iam_8-16",  0x00000, 0x4000, CRC(345434b9) SHA1(ec880f6f5e90aa971937e0270701e323f6a83671) ) /* all roms were 27128, twice the size of other sets */
	ROM_LOAD( "high_iam_8-16",     0x08000, 0x4000, CRC(57000fb7) SHA1(144723eb528c4816b9aa4b0ba77d2723c6442546) ) /* Is only the 1st half used by the board / program? */
	/* Banked roms */
	ROM_LOAD( "jokerpoker_iam_8-16-84",    0x10000, 0x4000, CRC(33794a87) SHA1(2b46809623713582746d9f817db33077f15a3684) ) /* This set is verified correct by 3 different sets checked */
	ROM_LOAD( "blackjack_iam_8-16-84",     0x14000, 0x4000, CRC(6e10b5b8) SHA1(5dc294b4a562193a99b0d307323fcc084a053426) )
	ROM_LOAD( "rollingbones_iam_8-16-84",  0x18000, 0x4000, CRC(26949774) SHA1(20571b955521ec3929430249aa651cee8a97043d) )
	ROM_LOAD( "casinoslots_iam_8-16-84",   0x1c000, 0x4000, CRC(c5a1eec6) SHA1(43d31bfe4cbbb6b86f52f675f513050866443176) )
	ROM_LOAD( "horserace_iam_3-19-84",     0x20000, 0x4000, CRC(7b9e75cb) SHA1(0db8da6f5f59f57886766bec96102d43796567ef) )
ROM_END

/*

UVM-8B romboard

*/

ROM_START( amuse1 ) /* v30.08 */
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "m108_control_5-16-84",  0x00000, 0x4000, CRC(9dda922f) SHA1(ded42adda8376452e0ac4f771ebb42fd86811dc5) )
	ROM_LOAD( "m108_hcon_5-16-84",     0x05800, 0x1000, CRC(ade5c42d) SHA1(d3e32ecaeb21a4e5eedd243c42d5914b03f572bd) )
	/* Banked roms */
	ROM_LOAD( "jokerpoker_amba_5-16-84",   0x10000, 0x4000, CRC(530b22d7) SHA1(418d41820429d0f8e054d383c502f4d75505ddca) )
	ROM_LOAD( "rollingbones_am_5-16-84",   0x14000, 0x4000, CRC(60496f5a) SHA1(9f9fd4191eaaa12350dbcc9d414306454b2adfba) )
	ROM_LOAD( "beatthespread_am2p_3-16-84",0x18000, 0x4000, CRC(40997230) SHA1(49e92a9f371a9839c94aa923aa5883284dae9dc2) )
ROM_END


ROM_START( suprpokr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "10-19s-1.e1",  0x00000, 0x4000, CRC(50662b4d) SHA1(967161a755db43d2cfd5ce92e14c5284f1f1f8ad) )
	ROM_LOAD( "10-19s-1.e2",  0x08000, 0x4000, BAD_DUMP CRC(22b45aeb) SHA1(006c3072cc44c6fde9b4d15163dc70707bbd5a9c) ) /* Self test reports this ROM bad */
ROM_END

ROM_START( reelfun ) /* v7.03 */
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "reelfun.cnt",          0x00000, 0x4000, CRC(ce42e0ea) SHA1(87f703b14aa819c54e54b42e639448521c01f76b) )
	ROM_LOAD( "reelfun.prg",          0x08000, 0x2000, CRC(615d846a) SHA1(ffa1c47393f4f364aa34d14cf3ac2f56d9eaecb0) )	/* banked */
	ROM_LOAD( "reelfun-1-title",      0x10000, 0x8000, CRC(0e165fbc) SHA1(a3a5b7db72ab86efe973f649f5dfe5133830e3fc) )	/* banked ROMs for solution data */
	ROM_LOAD( "reelfun-2-place",      0x18000, 0x8000, CRC(a0066bfd) SHA1(b6f031ab50eb396be79e79e06f2101400683ec3e) )
	ROM_LOAD( "reelfun-3-phrase",     0x20000, 0x8000, CRC(199e36b0) SHA1(d9dfe39c9a4fca1169150f8941f8ebc499dfbaf5) )
	ROM_LOAD( "reelfun-4-person",     0x28000, 0x8000, CRC(49b0710b) SHA1(a38b3251bcb8683d43bdb903036970140a9735e6) )
	ROM_LOAD( "reelfun-5-song_title", 0x30000, 0x8000, CRC(cce01c45) SHA1(c484f5828928edf39335cedf21acab0b9e2a6881) )
ROM_END

ROM_START( reelfun1 ) /* v7.01 */
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "reelfun701.cnt",       0x00000, 0x4000, CRC(d9d1e92b) SHA1(337f66a37b3734d565b3ff3d912e0f690fd7c445) )
	ROM_LOAD( "reelfun.prg",          0x08000, 0x2000, CRC(615d846a) SHA1(ffa1c47393f4f364aa34d14cf3ac2f56d9eaecb0) )	/* banked */
	ROM_LOAD( "reelfun-1-title",      0x10000, 0x8000, CRC(0e165fbc) SHA1(a3a5b7db72ab86efe973f649f5dfe5133830e3fc) )	/* banked ROMs for solution data */
	ROM_LOAD( "reelfun-2-place",      0x18000, 0x8000, CRC(a0066bfd) SHA1(b6f031ab50eb396be79e79e06f2101400683ec3e) )
	ROM_LOAD( "reelfun-3-phrase",     0x20000, 0x8000, CRC(199e36b0) SHA1(d9dfe39c9a4fca1169150f8941f8ebc499dfbaf5) )
	ROM_LOAD( "reelfun-4-person",     0x28000, 0x8000, CRC(49b0710b) SHA1(a38b3251bcb8683d43bdb903036970140a9735e6) )
	ROM_LOAD( "reelfun-5-song_title", 0x30000, 0x8000, CRC(cce01c45) SHA1(c484f5828928edf39335cedf21acab0b9e2a6881) )
ROM_END

ROM_START( findout )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "12.bin",       0x00000, 0x4000, CRC(21132d4c) SHA1(e3562ee2f46b3f022a852a0e0b1c8fb8164f64a3) )
	ROM_LOAD( "11.bin",       0x08000, 0x2000, CRC(0014282c) SHA1(c6792f2ff712ba3759ff009950d78750df844d01) )	/* banked */
	ROM_LOAD( "13.bin",       0x10000, 0x8000, CRC(cea91a13) SHA1(ad3b395ab0362f3decf178824b1feb10b6335bb3) )	/* banked ROMs for solution data */
	ROM_LOAD( "14.bin",       0x18000, 0x8000, CRC(2a433a40) SHA1(4132d81256db940789a40aa1162bf1b3997cb23f) )
	ROM_LOAD( "15.bin",       0x20000, 0x8000, CRC(d817b31e) SHA1(11e6e1042ee548ce2080127611ce3516a0528ae0) )
	ROM_LOAD( "16.bin",       0x28000, 0x8000, CRC(143f9ac8) SHA1(4411e8ba853d7d5c032115ce23453362ab82e9bb) )
	ROM_LOAD( "17.bin",       0x30000, 0x8000, CRC(dd743bc7) SHA1(63f7e01ac5cda76a1d3390b6b83f4429b7d3b781) )

	ROM_REGION( 0x0200, "gfx2", 0 )
	ROM_LOAD( "82s147.bin",   0x0000, 0x0200, CRC(f3b663bb) SHA1(5a683951c8d3a2baac4b49e379d6e10e35465c8a) )	/* unknown */
ROM_END

ROM_START( gt507uk )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "triv_3_2.bin",    0x00000, 0x4000, CRC(2d72a081) SHA1(8aa32acf335d027466799b097e0de66bcf13247f) )
	ROM_LOAD( "rom_ad.bin",      0x08000, 0x2000, CRC(c81cc847) SHA1(057b7b75a2fe1abf88b23e7b2de230d9f96139f5) )
	ROM_LOAD( "aerospace",       0x10000, 0x8000, CRC(cb555d46) SHA1(559ae05160d7893ff96311a2177eba039a4cf186) )
	ROM_LOAD( "english_sport_4", 0x18000, 0x8000, CRC(6ae8a63d) SHA1(c6018141d8bbe0ed7619980bf7da89dd91d7fcc2) )
	ROM_LOAD( "general_facts",   0x20000, 0x8000, CRC(f921f108) SHA1(fd72282df5cee0e6ab55268b40785b3dc8e3d65b) )
	ROM_LOAD( "horrors",         0x28000, 0x8000, CRC(5f7b262a) SHA1(047480d6bf5c6d0603d538b84c996bd226f07f77) )
	ROM_LOAD( "pop_music",       0x30000, 0x8000, CRC(884fec7c) SHA1(b389216c17f516df4e15eee46246719dd4acb587) )
ROM_END

ROM_START( gt5 ) /* v5.06, From a TRIV3D romboard */
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "program",         0x00000, 0x4000, CRC(e9d6226c) SHA1(42e62c5cafa3f051bf48c18c8c549ffcd4c766c5) )
	ROM_LOAD( "entertainment_2", 0x10000, 0x8000, CRC(c75c2331) SHA1(9c5947616a4cba2623c599def6cf3b2b1981b681) ) /* rom / question set #15 */
	ROM_LOAD( "facts_2",         0x18000, 0x8000, CRC(7836ef31) SHA1(6a84cfa39de392eed46a4b37752e00b6d094bbd6) )
	ROM_LOAD( "new_science_3",   0x20000, 0x8000, CRC(fcbc3bc3) SHA1(2dbdd39dce9dbf53c0954dec44a4f5109243dc60) )
	ROM_LOAD( "nfl_football",    0x28000, 0x8000, CRC(42eb2849) SHA1(c24e681a508ef8350f7e5d50aea2c31cf70ce5c9) )
	ROM_LOAD( "adult_sex_6",     0x30000, 0x8000, CRC(d66f35f7) SHA1(81b56756230b27b0903d0c5df30439726526afe2) )
ROM_END

ROM_START( gtsers8 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc",  0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "science",         0x10000, 0x8000, CRC(2f940ebd) SHA1(bead4988ac0a97d70f2a3c0b40a05968436de2ed) )
	ROM_LOAD( "general",         0x18000, 0x8000, CRC(1efa01c3) SHA1(801ef5ab55184e488b08ef99ebd641ea4f7edb24) )
	ROM_LOAD( "sports",          0x20000, 0x8000, CRC(6bd1ba9a) SHA1(7caac1bd438a9b1d11fb33e11814b5d76951211a) )
	ROM_LOAD( "soccer",          0x28000, 0x8000, CRC(f821f860) SHA1(b0437ef5d31c507c6499c1fb732d2ba3b9beb151) )
	ROM_LOAD( "potpourri",       0x30000, 0x8000, CRC(f2968a28) SHA1(87c08c59dfee71e7bf071f09c3017c750a1c5694) )
	/* Missing Alternate question set: "Adult Sex" */
ROM_END

ROM_START( gtsers9 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc",  0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "facts",           0x10000, 0x8000, CRC(21bd6181) SHA1(609ae1097a4011e90d03d4c4f03140fbe84c084a) )
	ROM_LOAD( "rock-n-roll",     0x18000, 0x8000, CRC(1be036b1) SHA1(0b262906044950319dd911b956ac2e0b433f6c7f) )
	ROM_LOAD( "television",      0x20000, 0x8000, CRC(731d4cc0) SHA1(184b6e48edda24f50e377a473a1a4709a218181b) )
	ROM_LOAD( "usa_trivia",      0x28000, 0x8000, CRC(829543b4) SHA1(deb0a4132852643ad884cf194b0a2e6671aa2b4e) )
	ROM_LOAD( "adult_sex_2",     0x30000, 0x8000, CRC(0d683f21) SHA1(f47ce3c31c4c5ed02247fa280303e6ae760315df) ) /* Listed as an alternate question set */
	/* Missing "Artists-Athletes" */
ROM_END

ROM_START( gtsers10 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc", 0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "new_general",    0x10000, 0x8000, CRC(ba1f5b92) SHA1(7e94be0ef6904331d3a6b266e5887e9a15c5e7f9) )
	ROM_LOAD( "new_tv_mash",    0x18000, 0x8000, CRC(f73240c6) SHA1(78020644074da719414133a86a91c1328e5d8929) )
	ROM_LOAD( "new_entrtnmnt",  0x20000, 0x8000, CRC(0f54340c) SHA1(1ca4c23b542339791a2d8f4a9a857f755feca8a1) )
	ROM_LOAD( "new_sports",     0x28000, 0x8000, CRC(19eff1a3) SHA1(8e024ae6cc572176c90d819a438ace7b2512dbf2) )
	ROM_LOAD( "adult_sex_3",    0x30000, 0x8000, CRC(2c46e355) SHA1(387ab389abaaea8e870b00039dd884237f7dd9c6) ) /* Listed as an alternate question set */
	/* Missing "New Science" */
ROM_END

ROM_START( gtsers11 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc", 0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "rich-famous",    0x10000, 0x8000, CRC(39e07e4a) SHA1(6e5a0bcefaa1169f313e8818cf50919108b3e121) )
	ROM_LOAD( "cars-women",     0x18000, 0x8000, CRC(4c5dd1df) SHA1(f3e2146eeab07ec71617c7614c6e8f6bc844e6e3) )
	ROM_LOAD( "aerospace",      0x20000, 0x8000, CRC(cb555d46) SHA1(559ae05160d7893ff96311a2177eba039a4cf186) )
	ROM_LOAD( "tv_music",       0x28000, 0x8000, CRC(5138e0fb) SHA1(102146d63752258c2fda95df49289c42b392c838) )
	ROM_LOAD( "gay_times",      0x30000, 0x8000, CRC(c4f9a8cf) SHA1(9247ecc5708aba263e0365fc43a1a7d0c2b7c391) ) /* Listed as an alternate question set */
	/* Missing "General Facts" */
ROM_END

ROM_START( gt103a1 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versiona",  0x00000, 0x4000, CRC(537d6566) SHA1(282a33e4a9fc54d34094393c00026bf31ccd6ab5) )
	ROM_LOAD( "new_science_2",   0x10000, 0x8000, CRC(3bd80fb8) SHA1(9a196595bc5dc6ed5ee5853786839ed4847fa436) ) /* Questions from an unknown set, will be corrected when verified */
	ROM_LOAD( "nfl_football",    0x18000, 0x8000, CRC(d676b7cd) SHA1(d652d2441adb500f7af526d110d0335ea453d75b) ) /* These questions are likely mix-n-match do to opperator swaps   */
	ROM_LOAD( "rock_music",      0x20000, 0x8000, CRC(7f11733a) SHA1(d4d0dee75518edf986cb1241ade45ccb4840f088) ) /* Currently unverified are Series 12, 13 & 14 */
	ROM_LOAD( "war_and_peace",   0x28000, 0x8000, CRC(bc709383) SHA1(2fba4c80773abea7bbd826c39378b821cddaa255) )
	ROM_LOAD( "entertainment",   0x30000, 0x8000, CRC(07068c9f) SHA1(1aedc78d071281ec8b08488cd82655d41a77cf6b) )
ROM_END

ROM_START( gt103a2 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "prog1_versionc",  0x00000, 0x4000, CRC(340246a4) SHA1(d655e1cf2b1e87a05e87ff6af4b794e6d54a2a52) )
	ROM_LOAD( "vices",           0x10000, 0x8000, CRC(e6069955) SHA1(68f7453f21a4ce1be912141bbe947fbd81d918a3) ) /* Questions from an unknown set, will be corrected when verified */
	ROM_LOAD( "cops_&_robbers",  0x18000, 0x8000, CRC(8b367c33) SHA1(013468157bf469c9cf138809fdc45b3ba60a423b) ) /* These questions are likely mix-n-match do to opperator swaps   */
	ROM_LOAD( "famous_couples",  0x20000, 0x8000, CRC(e0618218) SHA1(ff64fcd6dec83a2271b63c3ae64dc932a3954ec5) ) /* Currently unverified are Series 12, 13 & 14 */
	ROM_LOAD( "famous_quotes",   0x28000, 0x8000, CRC(0a27d8ae) SHA1(427e6ae25e47da7f7f7c3e92a37e330d711da90c) )
ROM_END

ROM_START( gt103a3 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "t_3a-8_1.bin",    0x00000, 0x4000, CRC(02aef306) SHA1(1ffc10c79a55d41ea36bcaab13cb3f02cb3f9712) )
	ROM_LOAD( "rock-n-roll_alt", 0x10000, 0x8000, CRC(8eb83052) SHA1(93e3c1ae6c2048fb44ecafe1013b6a96da38fa84) ) /* Questions from an unknown set, will be corrected when verified */
	ROM_LOAD( "history-geog.",   0x18000, 0x8000, CRC(c9a70fc3) SHA1(4021e5d702844416e8c798ed0a57c9ecd20b1d4b) ) /* These questions are likely mix-n-match do to opperator swaps   */
	ROM_LOAD( "the_sixties",     0x20000, 0x8000, CRC(8cfa854e) SHA1(81428c12f99841db1c61b471ac8d00f0c411883b) ) /* Currently unverified are Series 12, 13 & 14 */
	ROM_LOAD( "tv_comedies",     0x28000, 0x8000, CRC(992ae38e) SHA1(312780d651a85a1c433f587ff2ede579456d3fd9) )
ROM_END

ROM_START( gt103aa )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "t_3a-8_1.bin",      0x00000, 0x4000, CRC(02aef306) SHA1(1ffc10c79a55d41ea36bcaab13cb3f02cb3f9712) )
	ROM_LOAD( "entertainment_alt", 0x10000, 0x8000, CRC(9a6628b9) SHA1(c0cb7e974329d4d5b91f107296d21a674e35a51b) ) /* Questions from an unknown set, will be corrected when verified */
	ROM_LOAD( "general_alt",       0x18000, 0x8000, CRC(df34f7f9) SHA1(329d123eea711d5135dc02dd7b89b220ce8ddd28) ) /* These questions are likely mix-n-match do to opperator swaps   */
	ROM_LOAD( "science_alt",       0x20000, 0x8000, CRC(9eaebd18) SHA1(3a4d787cb006dbb23ce346577cb1bb5e543ba52c) ) /* Currently unverified are Series 12, 13 & 14 */
	ROM_LOAD( "science_alt2",      0x28000, 0x8000, CRC(ac93d348) SHA1(55550ba6b5daffdf9653854075ad4f8398a5e621) )
	ROM_LOAD( "sports_alt2",       0x30000, 0x8000, CRC(40207845) SHA1(2dddb9685dcefabfde07057a639aa9d08da2329e) )
ROM_END

ROM_START( gt103asx )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "t_3a-8_1.bin",    0x00000, 0x4000, CRC(02aef306) SHA1(1ffc10c79a55d41ea36bcaab13cb3f02cb3f9712) )
	ROM_LOAD( "adult_sex_2",     0x10000, 0x8000, CRC(0d683f21) SHA1(f47ce3c31c4c5ed02247fa280303e6ae760315df) )
	ROM_LOAD( "adult_sex_2_alt", 0x18000, 0x8000, CRC(8c0eacc8) SHA1(ddaa25548d161394b41c65a2db57a9fcf793062b) )
	ROM_LOAD( "adult_sex_3_alt", 0x20000, 0x8000, CRC(63cbd1d6) SHA1(8dcd5546dc8688d6b8404d5cf63d8a59acc9bf4c) )
	ROM_LOAD( "adult_sex_4",     0x28000, 0x8000, CRC(36a75071) SHA1(f08d31f241e1dc9b94b940cd2872a692f6f8475b) )
	ROM_LOAD( "adult_sex_5",     0x30000, 0x8000, CRC(fdbc3729) SHA1(7cb7cec4439ddc39de2f7f62c25623cfb869f493) )
ROM_END

ROM_START( quiz )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x00000, 0x4000, CRC(4e3204da) SHA1(291f1c9b8c4c07881621c3ecbba7af80f86b9520) )
	ROM_LOAD( "2.bin",        0x10000, 0x8000, CRC(b79f3ae1) SHA1(4b4aa50ec95138bc8ee4bc2a61bcbfa2515ac854) )
	ROM_LOAD( "3.bin",        0x18000, 0x8000, CRC(9c7e9608) SHA1(35ee9aa36d16bca64875640224c7fe9d327a95c3) )
	ROM_LOAD( "4.bin",        0x20000, 0x8000, CRC(30f6b4d0) SHA1(ab2624eb1a3fd9cd8d44433962d09496cd67d569) )
	ROM_LOAD( "5.bin",        0x28000, 0x8000, CRC(e9cdae21) SHA1(4de4a4edf9eccd8f9f7b935f47bee42c10ad606f) )
	ROM_LOAD( "6.bin",        0x30000, 0x8000, CRC(89e2b7e8) SHA1(e85c66f0cf37418f522c2d6384997d52f2f15117) )

	ROM_REGION( 0x0200, "proms", 0 ) /* unknown */
	ROM_LOAD( "prom_am27s29pc.bin", 0x0000, 0x0200, CRC(19e3f161) SHA1(52da3c1e50c2329454de14cb9c46149e573e562b) )
ROM_END

ROM_START( quizvid )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "quiz.rev2a.prg.0",    0x00000, 0x4000, CRC(28e5855a) SHA1(eec16ef7a46451054565c7001478382a0a84725a) )
	ROM_LOAD( "quiz.reva.sesso.1",   0x10000, 0x8000, CRC(72367eee) SHA1(09cd8e8c20e159ab78025e4201065c41dde90ef5) )
	ROM_LOAD( "quiz.rev2a.sport.2",  0x18000, 0x8000, CRC(1f050838) SHA1(2e44098476d05366b05f7071a615f52b588bcac5) )
	ROM_LOAD( "quiz.rev2a.musica.3", 0x20000, 0x8000, CRC(6ae39e6b) SHA1(b5ce2678c07d6c1d3d49f1703e2b9f84105e88fa) )
	ROM_LOAD( "quiz.reva.storia.4",  0x28000, 0x8000, CRC(1c648781) SHA1(e363ae1a3946446c0bc42f9857a9d66702cb3367) )
	ROM_LOAD( "quiz.reva.attuale.5", 0x30000, 0x8000, CRC(a3642478) SHA1(57851aabc6d2f5acff426c09574559e141da5d13) )

	ROM_REGION( 0x0104, "plds", 0 )
	ROM_LOAD( "pal10l8cn.pal1",            0x0000, 0x002c, CRC(7f4499de) SHA1(74838150d0b71171f00f65e03748b262c2bb6e4c) )
	ROM_LOAD( "pal10l8cn.pal4",            0x0000, 0x002c, CRC(f14a34ab) SHA1(78af7f5eafbf2d52ee7b01b497ad59448c986693) )
	ROM_LOAD( "pal16l8a-2.bad.dump",       0x0000, 0x0104, BAD_DUMP CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
	ROM_LOAD( "pal16l8cn.pal3.bad.dump",   0x0000, 0x0104, BAD_DUMP CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
	ROM_LOAD( "pal16l8cn.pal5.bad.dump",   0x0000, 0x0104, BAD_DUMP CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
ROM_END

ROM_START( quiz211 )
	ROM_REGION( 0x38000, "maincpu", 0 )
	ROM_LOAD( "1a.bin",         0x000000, 0x4000, CRC(116de0ea) SHA1(9af97b100aa2c79a58de055abe726d6e2e00aab4) )
	ROM_CONTINUE(               0x000000, 0x4000 ) // halves identical
	ROM_LOAD( "hobby.bin",      0x10000, 0x8000, CRC(c86d0c2b) SHA1(987ef17c7b9cc119511a16cbd98ec44d24665af5) )
	ROM_LOAD( "musica.bin",     0x18000, 0x8000, CRC(6b08990f) SHA1(bbc633dc4e0c395269d3d3fbf1f7617ea7adabf1) )
	ROM_LOAD( "natura.bin",     0x20000, 0x8000, CRC(f17b0d59) SHA1(ebe3d5a0247f3065f0c5d4ee0b846a737700f379) )
	ROM_LOAD( "spettacolo.bin", 0x28000, 0x8000, CRC(38b8e37a) SHA1(e6df575f61ac61e825d98eaef99c128647806a75) )
	ROM_LOAD( "mondiali90.bin", 0x30000, 0x4000, CRC(35622870) SHA1(f2dab64106ca4ef07175a0ad9491470964d8a0d2) )

	ROM_REGION( 0x0e00, "proms", 0 ) /* unknown */
	ROM_LOAD( "prom_27s13-1.bin", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-2.bin", 0x0200, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-3.bin", 0x0400, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-4.bin", 0x0600, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-5.bin", 0x0800, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_27s13-6.bin", 0x0a00, 0x0200, NO_DUMP )
	ROM_LOAD( "prom_6349-1n.bin", 0x0c00, 0x0200, CRC(19e3f161) SHA1(52da3c1e50c2329454de14cb9c46149e573e562b) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pal10l8cn.bin",   0x0000, 0x002c, CRC(86095226) SHA1(e7496efbd5ca240f0df2dfa5627402342c7f5384) )
ROM_END

/*
GEI Multi Game System
(c) 1992

Much newer satellite board for the Greyhound UV-1B mainboard system

Contains:
Power LED
3 Pals
2 74LS374N

ST MK48Z02B-20 Zeropower RAM

Control, graphics & 13 banked roms (as labeled):

Cont: R.5 Cont 92
GRPH: R.5 K.B. 93

Bank  1: pokr 4/3 92
Bank  2: bljk 4/3 92
Bank  3: bone 4/3 92
Bank  4: slot 4/3 92
Bank  5: sprd 4/3 92
Bank  6: hrse 7/8 92
Bank  7: dogs 7/8 92
Bank  8: memo test
Bank  9: reel fun
Bank 10: fact #1
Bank 11: adlt sex #1
Bank 12: nfl #1
Bank 13: entr #1

All roms are 27C256 type eproms
*/

ROM_START( geimulti )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "cont", 0x00000, 0x8000, CRC(920020df) SHA1(01cb0a58bf863d63bcafe1e198ce5ea10bb8f59e) )

	ROM_REGION( 0x8000*(13 + 1), "bank", ROMREGION_ERASEFF )
	ROM_LOAD( "pokr",      0x00000, 0x8000, CRC(1e9651a0) SHA1(23f98633701a9ac01784b29d1283ec9236810baf) )
	ROM_LOAD( "bljk",      0x08000, 0x8000, CRC(afde21e6) SHA1(4db37ee4ffc72e3513cbfe31d018caa062683adf) )
	ROM_LOAD( "bone",      0x10000, 0x8000, CRC(6a84fc5d) SHA1(09d5d052a90744e5c6e0b06cabfdbbaa2458bbb5) )
	ROM_LOAD( "slot",      0x18000, 0x8000, CRC(11b85a71) SHA1(5672f6f796b2743c252d159778bd8746cf1f5f86) )
	ROM_LOAD( "sprd",      0x20000, 0x8000, CRC(2f067b3d) SHA1(cd7cae3d2def74369b8d015b9f36d5a174bea92c) )
	ROM_LOAD( "hrse",      0x28000, 0x8000, CRC(ab1d014a) SHA1(aa7a372266decd7b65234de4d866fe7baf71c9bb) )
	ROM_LOAD( "dogs",      0x30000, 0x8000, CRC(83cd070a) SHA1(ed7ce3ffe030802e99dd27f2310524615fcfc8bf) )
	ROM_LOAD( "memotest",  0x38000, 0x8000, CRC(8942b98c) SHA1(3f182eb6d00618d9859cd8acdf13c829d4469075) )
	ROM_LOAD( "reelfun",   0x40000, 0x8000, CRC(b3ede904) SHA1(e77c15e893583572650bb60432d68a1f7bf67d09) )
	ROM_LOAD( "fact#1",    0x48000, 0x8000, CRC(949b5519) SHA1(d4ac35ccbbb50f4e0d1e21e8427e6ad535e6da53) )
	ROM_LOAD( "adltsex#1", 0x50000, 0x8000, CRC(9e4d320b) SHA1(76a6280bce884acb3faa9ef3882de016612c34f6) )
	ROM_LOAD( "nfl#1",     0x58000, 0x8000, CRC(74fbbf17) SHA1(9d559034d3ec0b293d4b720150197daea3d450ff) )
	ROM_LOAD( "entr#1",    0x60000, 0x8000, CRC(caceaa7b) SHA1(c51f10f5acd3d3fedce43103b9f11d006139043c) )
	ROM_LOAD( "grph",      0x68000, 0x8000, CRC(25e265db) SHA1(6e184309ee67dbe7930570b135ace09eeb1eb333) )

ROM_END

ROM_START( sprtauth )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sprt-auth-rev1-cont.cont", 0x00000, 0x8000, CRC(19dd0aa6) SHA1(7805d8139ac20061fc782bcaeff2202ed451fa91) )

	ROM_REGION( 0x8000*(13 + 1), "bank", ROMREGION_ERASEFF )
	ROM_LOAD( "sprt-auth-prem-bsbl.bank1",    0x00000, 0x8000, CRC(ef296636) SHA1(e727fe58bc961f56503788b2b670f5b3b04743f2) )
	ROM_LOAD( "sprt-auth-prem-bskt.bank2",    0x08000, 0x8000, CRC(4649d523) SHA1(fdfa1263cc6b3c45e62af0e3baf79311f243ac5f) )
	ROM_LOAD( "sprt-auth-prem-foot.bank3",    0x10000, 0x8000, CRC(76076022) SHA1(5014133e7143c4f4ccf4c7d1f05effe286b5a30a) )
	ROM_LOAD( "sprt-auth-prem-hcky.bank4",    0x18000, 0x8000, CRC(135beec3) SHA1(66fce1c0c0abbbf4971ab0f764d27f1d0849ccdc) )
	ROM_LOAD( "sprt-auth-asrt-sprt.bank5",    0x20000, 0x8000, CRC(fe8fc879) SHA1(efe38e8d3a314062b8005636df683349cd54f857) )
	ROM_LOAD( "sprt-auth-auto-racg.bank6",    0x28000, 0x8000, CRC(7ac1bbd6) SHA1(8635791bf2707b4d028ee8b020199770984b9ef4) )
	ROM_LOAD( "sprt-auth-ii-bsbl-ball.bank7", 0x30000, 0x8000, CRC(bad3e6bd) SHA1(e3df34a4d5db7cfbd014a841d224eeefc8386d40) )
	ROM_LOAD( "sprt-auth-ii-foot-ball.bank8", 0x38000, 0x8000, CRC(c305dec2) SHA1(924e608cd327bf6ffe831225affa270181599cea) )
	ROM_LOAD( "sprt-auth-supr-bowl.bank9",    0x40000, 0x8000, CRC(3a548fe5) SHA1(6ad35516651a8a878b512cb3eff697952e194dd0) )
	ROM_LOAD( "sprt-auth-rev1-kb.grph",       0x68000, 0x8000, CRC(c4f734ac) SHA1(028217fe6d7be75f75e9f67b665d465c729d2995) )

ROM_END

static DRIVER_INIT( setbank )
{
	memory_set_bankptr(machine, "bank1",memory_region(machine, "maincpu") + 0x2000);
}

static DRIVER_INIT( geimulti )
{
	memory_set_bankptr(machine, "bank1",memory_region(machine, "bank") + 0x0000);
}

GAME( 1982, jokpoker, 0,        gselect,  gselect,  setbank, ROT0, "Greyhound Electronics", "Joker Poker (Version 16.03B)",            GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1983, jokpokera,jokpoker, jokpokera,gselect,  setbank, ROT0, "Greyhound Electronics", "Joker Poker (Version 16.03BI)",           GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1983, jokpokerb,jokpoker, jokpokera,gselect,  setbank, ROT0, "Greyhound Electronics", "Joker Poker (Version 16.04BI 10-19-88, Joker Poker ICB 9-30-86)",    GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1983, jokpokerc,jokpoker, jokpokera,gselect,  setbank, ROT0, "Greyhound Electronics", "Joker Poker (Version 16.03BI 5-10-85, Poker No Raise ICB 9-30-86)",  GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1982, superbwl, 0,        gselect,  gselect,  setbank, ROT0, "Greyhound Electronics", "Super Bowl (Version 16.03B)",             GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1982, gs4002,   0,        gselect,  gselect,  0,       ROT0, "Greyhound Electronics", "Selection (Version 40.02TMB, set 1)",     GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1982, gs4002a,  gs4002,   gselect,  gselect,  0,       ROT0, "Greyhound Electronics", "Selection (Version 40.02TMB, set 2)",     GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1982, amuse,    0,        amuse,    gepoker,  0,       ROT0, "Greyhound Electronics", "Amuse (Version 50.08 IBA)",               GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1982, amuse1,   amuse,    amuse1,   gepoker,  0,       ROT0, "Greyhound Electronics", "Amuse (Version 30.08 IBA)",               GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1984, gepoker,  0,        gepoker,  gepoker,  0,       ROT0, "Greyhound Electronics", "Poker (Version 50.02 ICB, set 1)",        GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gepoker1, gepoker,  gepoker,  gepoker,  0,       ROT0, "Greyhound Electronics", "Poker (Version 50.02 ICB, set 2)",        GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gepoker2, gepoker,  gepoker,  gepoker,  0,       ROT0, "Greyhound Electronics", "Poker (Version 50.02 ICB, set 3)",        GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1984, gtsers1,  0,        getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 1)",             GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers2,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 2)",             GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers3,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 3)",             GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers4,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 4)",             GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers5,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 5)",             GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers7,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 7)",             GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsersa,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Alt revision questions set 1)",   GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsersb,  gtsers1,  getrivia, getrivia, 0,       ROT0, "Greyhound Electronics", "Trivia (Alt revision questions set 2)",   GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers8,  0,        findout,  gt103a,   0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 8)",             GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers9,  gtsers8,  findout,  gt103a,   0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 9)",             GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers10, gtsers8,  findout,  gt103a,   0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 10)",            GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gtsers11, gtsers8,  findout,  gt103a,   0,       ROT0, "Greyhound Electronics", "Trivia (Questions Series 11)",            GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gt103a1,  gtsers8,  findout,  gt103a,   0,       ROT0, "Greyhound Electronics", "Trivia (Version 1.03a) (alt 1)",          GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gt103a2,  gtsers8,  findout,  gt103a,   0,       ROT0, "Greyhound Electronics", "Trivia (Version 1.03a) (alt 2)",          GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gt103a3,  gtsers8,  findout,  gt103a,   0,       ROT0, "Greyhound Electronics", "Trivia (Version 1.03a) (alt 3)",          GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gt103aa,  gtsers8,  findout,  gt103a,   0,       ROT0, "Greyhound Electronics", "Trivia (Version 1.03a Alt questions)",    GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1984, gt103asx, gtsers8,  findout,  gt103a,   0,       ROT0, "Greyhound Electronics", "Trivia (Version 1.03a Sex questions)",    GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1985, sextriv1, 0,        getrivia, sextriv1, 0,       ROT0, "Kinky Kit and Game Co.", "Sexual Trivia (Version 1.02SB, set 1)",  GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1985, sextriv2, sextriv1, getrivia, sextriv1, 0,       ROT0, "Kinky Kit and Game Co.", "Sexual Trivia (Version 1.02SB, set 2)",  GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1986, gt507uk,  0,        findout,  gt507uk,  0,       ROT0, "Greyhound Electronics", "Trivia (UK Version 5.07)",                GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1986, gt5,      0,        findout,  gt103,    0,       ROT0, "Greyhound Electronics", "Trivia (Version 5.06)",                   GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1986, quiz,     0,        findout,  quiz,     0,       ROT0, "bootleg",               "Quiz (Revision 2)",                       GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1986, quizvid,  0,        quizvid,  quiz,     0,       ROT0, "bootleg",               "Video Quiz",                              GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1986, reelfun,  0,        findout,  reelfun,  0,       ROT0, "Greyhound Electronics", "Reel Fun (Version 7.03)",                 GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1986, reelfun1, reelfun,  findout,  reelfun,  0,       ROT0, "Greyhound Electronics", "Reel Fun (Version 7.01)",                 GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1987, findout,  0,        findout,  findout,  0,       ROT0, "Elettronolo",           "Find Out (Version 4.04)",                 GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1987, suprpokr, 0,        suprpokr, suprpokr, 0,       ROT0, "Greyhound Electronics", "Super Poker",                             GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1991, quiz211,  0,        findout,  quiz,     0,       ROT0, "Elettronolo",           "Quiz (Revision 2.11)",                    GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAME( 1992, geimulti, 0,        geimulti, geimulti, geimulti,ROT0, "Greyhound Electronics",  "GEI Multi Game",                         GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1992, sprtauth, 0,        sprtauth, sprtauth, geimulti,ROT0, "Classic Games",          "Sports Authority",                       GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
