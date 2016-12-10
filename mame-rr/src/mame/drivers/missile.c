/***************************************************************************

    Atari Missile Command hardware

    Games supported:
        * Missile Command

    Known issues:
        * none at this time

******************************************************************************************

Missile Command
Atari, 1979

PCB Layout
----------

MISSILE COMMAND
A035467-04
ATARI (C)79
|---------------------------------------------------------------------|
|                           T_5MHz   T_10MHz                          |
|    MC14584     MC14584         10MHz               RESET_SW         |
|J19                    T_HSYNC              T_R/W02         UM6502A  |
|                 T_WDOG_DIS                           T_-5V          |
|   T_BW_VID                                                       X  |
|-|                                                                X  |
  |        T_START2                        MM5290J-3               X  |
|-|J20     T_START1   T_GND     T_GND      MM5290J-3     035820-01.H1 |
|2         T_SLAM                          MM5290J-3     035821-01.JK1|
|2         T_COINR  T_COINC                MM5290J-3     035822-01.KL1|
|W  T_TEST T_COINL                                     T_GND          |
|A         T_AUD2           N82S25         MM5290J-3     035823-01.MN1|
|Y         T_AUD1             035826-01.L6 MM5290J-3               X  |
|-|           LM324   T_VSYNC              MM5290J-3   T_+12V         |
  |                   C012294B-01          MM5290J-3     035824-01.NP1|
|-|T_+5V    DSW(8)     DSW(8)                       X   035825-01.R1  |
|---------------------------------------------------------------------|
Notes:
       UM6502A     - 6502 CPU, clock input is on pin 37. This is a little strange because it
                     measures 1.17240MHz. It was assumed to be 1.25MHz [10/8]. This might be
                     caused by old components that are out of spec now, but the PCB does run
                     flawlessly, and the other clocks measure correctly so I'm not sure what's going on.
       C012294B-01 - 'Pokey' sound chip, clock 1.25MHz on pin 7 [10/8]
       035826-01   - MMI 6331 bipolar PROM
       MM5290J-3   - National Semiconductor MM5290J-3 16kx1 DRAM (=TMM416, uPD416, MK4116, TMS4116 etc)
       MC14584     - Hex Schmitt Trigger
       82S25       - Signetics 64-bit (16x4) bipolar scratch pad memory (=3101, 74S189, 7489, 9410 etc)
       LM324       - Low Power Quad Operational Amplifier
       J20         - 22-Way edge connector (for upright)
       J19         - 12-Way edge connector (additional controls for cocktail)
       T_*         - Test points
       X           - Empty location for DIP24 device (no socket)
       HSYNC       - 15.618kHz
       VSYNC       - 61.0076Hz


       Edge Connector J20 Pinout
       -------------------------
       GND                  A | 1   GND
       + 5V                 B | 2   + 5V
       + 12V                C | 3
       - 5V                 D | 4
       Audio 1 Out          E | 5   Audio 2 Out
       HSync                F | 6   VSync
       Start2 LED           H | 7   Left Fire
       Center Fire          J | 8   Right Fire
       Right Coin           K | 9   Left Coin
       Video Blue           L | 10  Video Red
       Video Green          M | 11  Left Coin Counter
       Center Coin Counter  N | 12  Right Coin Counter
       Start 1 LED          P | 13  Start Button 1
       Test Switch          R | 14  Slam Switch
       Center Coin Slot*    S | 15  Start Button 2
       Vert. Trackball Dir. T | 16  Vert. Trackball Clock
       Horiz. Trackball Clk U | 17  Horiz. Trackball Dir.
                            V | 18  Comp Sync
       NC                   W | 19  - 5V
       NC                   X | 20  + 12V
       + 5V                 Y | 21  + 5V
       GND                  Z | 22  GND


       Edge Connector J19 Pinout (Only used in cocktail version)
       -------------------------
                              A | 1
                              B | 2
                              C | 3
       Right Fire Button 2    D | 4
       Center Fire Button 2   E | 5   Left Fire Button 2
                              F | 6
       Cocktail               H | 7
                              J | 8
       Horiz. Trkball Dir. 2  K | 9   Vert. Trackball Dir. 2
       Horiz. Trkball Clock 2 L | 10  Vert. Trackball Clock 2
                              M | 11
                              N | 12

****************************************************************************

    Horizontal sync chain:

        A J/K flip flop @ D6 counts the 1H line, and cascades into a
        4-bit binary counter @ D5, which counts the 2H,4H,8H,16H lines.
        This counter cascades into a 4-bit BCD decade counter @ E5
        which counts the 32H,64H,128H,HBLANK lines. The counter system
        rolls over after counting to 320.

        Pixel clock = 5MHz
        HBLANK ends at H = 0
        HBLANK begins at H = 256
        HSYNC begins at H = 260
        HSYNC ends at H = 288
        HTOTAL = 320

    Vertical sync chain:

        The HSYNC signal clocks a 4-bit binary counter @ A4, which counts
        the 1V,2V,4V,8V lines. This counter cascades into a second 4-bit
        binary counter @ B4 which counts the 16V,32V,64V,128V lines. The
        counter system rolls over after counting to 256.

        if not flipped (V counts up):
            VBLANK ends at V = 24
            VBLANK begins at V = 0
            VSYNC begins at V = 4
            VSYNC ends at V = 8
            VTOTAL = 256

        if flipped (V counts down):
            VBLANK ends at V = 0
            VBLANK begins at V = 24
            VSYNC begins at V = 20
            VSYNC ends at V = 16
            VTOTAL = 256

    Interrupts:

        /IRQ connected to Q on flip-flop @ F7, clocked by SYNC which
        indicates an instruction opcode fetch. Input to the flip-flop
        (D) comes from a second flip-flop @ F7, which is clocked by
        /16V or 16V depending on whether or not we are flipped. Input
        to this second flip-flop is 32V.

        if not flipped (V counts up):
            clock @   0 -> 32V = 0 -> /IRQ = 0
            clock @  32 -> 32V = 1 -> /IRQ = 1
            clock @  64 -> 32V = 0 -> /IRQ = 0
            clock @  96 -> 32V = 1 -> /IRQ = 1
            clock @ 128 -> 32V = 0 -> /IRQ = 0
            clock @ 160 -> 32V = 1 -> /IRQ = 1
            clock @ 192 -> 32V = 0 -> /IRQ = 0
            clock @ 224 -> 32V = 1 -> /IRQ = 1

        if flipped (V counts down):
            clock @ 208 -> 32V = 0 -> /IRQ = 0
            clock @ 176 -> 32V = 1 -> /IRQ = 1
            clock @ 144 -> 32V = 0 -> /IRQ = 0
            clock @ 112 -> 32V = 1 -> /IRQ = 1
            clock @  80 -> 32V = 0 -> /IRQ = 0
            clock @  48 -> 32V = 1 -> /IRQ = 1
            clock @  16 -> 32V = 0 -> /IRQ = 0
            clock @ 240 -> 32V = 1 -> /IRQ = 1

****************************************************************************

    CPU Clock:
      _   _   _   _   _   _   _   _   _   _   _   _   _   _
    _| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |  1H
        ___     ___     ___     ___     ___     ___     ___
    ___|   |___|   |___|   |___|   |___|   |___|   |___|   |  2H
            _______         _______         _______
    _______|       |_______|       |_______|       |_______|  4H
    _______     ___________     ___________     ___________
           |___|           |___|           |___|              /(4H & /2H)
    _         ___             ___             ___
     |_______|   |___________|   |___________|   |_________   /Q on FF @ A7
                ___             ___             ___
    ___________|   |___________|   |___________|   |_______   Q on FF @ B8

    When V < 224,
        ___     ___     ___     ___     ___     ___     ___
    ___|   |___|   |___|   |___|   |___|   |___|   |___|   |  Sigma-X = 2H

    When V >= 224,
                ___             ___             ___
    ___________|   |___________|   |___________|   |_______   Sigma-X = Q on FF @ B8

****************************************************************************

    Modified from original schematics...

    MISSILE COMMAND
    ---------------
    HEX      R/W   D7 D6 D5 D4 D3 D2 D2 D0  function
    ---------+-----+------------------------+------------------------
    0000-01FF  R/W   D  D  D    D  D  D  D  D   512 bytes working ram

    0200-05FF  R/W   D  D  D    D  D  D  D  D   3rd color bit region
                                                of screen ram.
                                                Each bit of every odd byte is the low color
                                                bit for the bottom scanlines
                                                The schematics say that its for the bottom
                                                32 scanlines, although the code only accesses
                                                $401-$5FF for the bottom 8 scanlines...
                                                Pretty wild, huh?

    0600-063F  R/W   D  D  D    D  D  D  D  D   More working ram.

    0640-3FFF  R/W   D  D  D    D  D  D  D  D   2-color bit region of
                                                screen ram.
                                                Writes to 4 bytes each to effectively
                                                address $1900-$ffff.

    1900-FFFF  R/W   D  D                       2-color bit region of
                                                screen ram
                                                  Only accessed with
                                                   LDA ($ZZ,X) and
                                                   STA ($ZZ,X)
                                                  Those instructions take longer
                                                  than 5 cycles.

    ---------+-----+------------------------+------------------------
    4000-400F  R/W   D  D  D    D  D  D  D  D   POKEY ports.
    -----------------------------------------------------------------
    4008         R     D  D  D  D  D  D  D  D   Game Option switches
    -----------------------------------------------------------------
    4800         R     D                        Right coin
    4800         R        D                     Center coin
    4800         R           D                  Left coin
    4800         R              D               1 player start
    4800         R                 D            2 player start
    4800         R                    D         2nd player left fire(cocktail)
    4800         R                       D      2nd player center fire  "
    4800         R                          D   2nd player right fire   "
    ---------+-----+------------------------+------------------------
    4800         R                 D  D  D  D   Horiz trackball displacement
                                                        if ctrld=high.
    4800         R     D  D  D  D               Vert trackball displacement
                                                        if ctrld=high.
    ---------+-----+------------------------+------------------------
    4800         W     D                        Unused ??
    4800         W        D                     screen flip
    4800         W           D                  left coin counter
    4800         W              D               center coin counter
    4800         W                 D            right coin counter
    4800         W                    D         2 player start LED.
    4800         W                       D      1 player start LED.
    4800         W                          D   CTRLD, 0=read switches,
                                                        1= read trackball.
    ---------+-----+------------------------+------------------------
    4900         R     D                        VBLANK read
    4900         R        D                     Self test switch input.
    4900         R           D                  SLAM switch input.
    4900         R              D               Horiz trackball direction input.
    4900         R                 D            Vert trackball direction input.
    4900         R                    D         1st player left fire.
    4900         R                       D      1st player center fire.
    4900         R                          D   1st player right fire.
    ---------+-----+------------------------+------------------------
    4A00         R     D  D  D  D  D  D  D  D   Pricing Option switches.
    4B00-4B07  W                   D  D  D  D   Color RAM.
    4C00         W                              Watchdog.
    4D00         W                              Interrupt acknowledge.
    ---------+-----+------------------------+------------------------
    5000-7FFF  R       D  D  D  D  D  D  D  D   Program.
    ---------+-----+------------------------+------------------------


    MISSILE COMMAND SWITCH SETTINGS (Atari, 1980)
    ---------------------------------------------


    GAME OPTIONS:
    (8-position switch at R8)

    1   2   3   4   5   6   7   8   Meaning
    -------------------------------------------------------------------------
    Off Off                         Game starts with 7 cities
    On  On                          Game starts with 6 cities
    On  Off                         Game starts with 5 cities
    Off On                          Game starts with 4 cities
            On                      No bonus credit
            Off                     1 bonus credit for 4 successive coins
                On                  Large trak-ball input
                Off                 Mini Trak-ball input
                    On  Off Off     Bonus city every  8000 pts
                    On  On  On      Bonus city every 10000 pts
                    Off On  On      Bonus city every 12000 pts
                    On  Off On      Bonus city every 14000 pts
                    Off Off On      Bonus city every 15000 pts
                    On  On  Off     Bonus city every 18000 pts
                    Off On  Off     Bonus city every 20000 pts
                    Off Off Off     No bonus cities
                                On  Upright
                                Off Cocktail



    PRICING OPTIONS:
    (8-position switch at R10)

    1   2   3   4   5   6   7   8   Meaning
    -------------------------------------------------------------------------
    On  On                          1 coin 1 play
    Off On                          Free play
    On Off                          2 coins 1 play
    Off Off                         1 coin 2 plays
            On  On                  Right coin mech * 1
            Off On                  Right coin mech * 4
            On  Off                 Right coin mech * 5
            Off Off                 Right coin mech * 6
                    On              Center coin mech * 1
                    Off             Center coin mech * 2
                        On  On      English
                        Off On      French
                        On  Off     German
                        Off Off     Spanish
                                On  ( Unused )
                                Off ( Unused )

    -there are 2 different versions of the Super Missile Attack board.  It's not known if
    the roms are different.  The SMA manual mentions a set 3(035822-03E) that will work
    as well as set 2. Missile Command set 1 will not work with the SMA board. It would
        appear set 1 and set 2 as labeled by mame are reversed.

*****************************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/pokey.h"


#define MASTER_CLOCK	XTAL_10MHz

#define PIXEL_CLOCK		(MASTER_CLOCK/2)
#define HTOTAL			(320)
#define HBSTART			(256)
#define HBEND			(0)
#define VTOTAL			(256)
#define VBSTART			(256)
#define VBEND			(25)	/* 24 causes a garbage line at the top of the screen */


/*************************************
 *
 *  Globals
 *
 *************************************/

static const UINT8 *writeprom;

static emu_timer *irq_timer;
static emu_timer *cpu_timer;

static UINT8 irq_state;
static UINT8 ctrld;
static UINT8 flipscreen;
static UINT8 madsel_delay;
static UINT16 madsel_lastpc;



/*************************************
 *
 *  VBLANK and IRQ generation
 *
 *************************************/

INLINE int scanline_to_v(int scanline)
{
	/* since the vertical sync counter counts backwards when flipped,
        this function returns the current effective V value, given
        that vpos() only counts forward */
	return flipscreen ? (256 - scanline) : scanline;
}


INLINE int v_to_scanline(int v)
{
	/* same as a above, but the opposite transformation */
	return flipscreen ? (256 - v) : v;
}


INLINE void schedule_next_irq(running_machine *machine, int curv)
{
	/* IRQ = /32V, clocked by /16V ^ flip */
	/* When not flipped, clocks on 0, 64, 128, 192 */
	/* When flipped, clocks on 16, 80, 144, 208 */
	if (flipscreen)
		curv = ((curv - 32) & 0xff) | 0x10;
	else
		curv = ((curv + 32) & 0xff) & ~0x10;

	/* next one at the start of this scanline */
	timer_adjust_oneshot(irq_timer, machine->primary_screen->time_until_pos(v_to_scanline(curv), 0), curv);
}


static TIMER_CALLBACK( clock_irq )
{
	int curv = param;

	/* assert the IRQ if not already asserted */
	irq_state = (~curv >> 5) & 1;
	cputag_set_input_line(machine, "maincpu", 0, irq_state ? ASSERT_LINE : CLEAR_LINE);

	/* force an update while we're here */
	machine->primary_screen->update_partial(v_to_scanline(curv));

	/* find the next edge */
	schedule_next_irq(machine, curv);
}


static CUSTOM_INPUT( get_vblank )
{
	int v = scanline_to_v(field->port->machine->primary_screen->vpos());
	return v < 24;
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

static TIMER_CALLBACK( adjust_cpu_speed )
{
	int curv = param;

	/* starting at scanline 224, the CPU runs at half speed */
	if (curv == 224)
		cputag_set_clock(machine, "maincpu", MASTER_CLOCK/16);
	else
		cputag_set_clock(machine, "maincpu", MASTER_CLOCK/8);

	/* scanline for the next run */
	curv ^= 224;
	timer_adjust_oneshot(cpu_timer, machine->primary_screen->time_until_pos(v_to_scanline(curv), 0), curv);
}


static DIRECT_UPDATE_HANDLER( missile_direct_handler )
{
	/* offset accounts for lack of A15 decoding */
	int offset = address & 0x8000;
	address &= 0x7fff;

	/* RAM? */
	if (address < 0x4000)
	{
		direct->raw = direct->decrypted = space->machine->generic.videoram.u8 - offset;
		return ~0;
	}

	/* ROM? */
	else if (address >= 0x5000)
	{
		direct->raw = direct->decrypted = memory_region(space->machine, "maincpu") - offset;
		return ~0;
	}

	/* anything else falls through */
	return address;
}


static MACHINE_START( missile )
{
	/* initialize globals */
	writeprom = memory_region(machine, "proms");
	flipscreen = 0;

	/* set up an opcode base handler since we use mapped handlers for RAM */
	memory_set_direct_update_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), missile_direct_handler);

	/* create a timer to speed/slow the CPU */
	cpu_timer = timer_alloc(machine, adjust_cpu_speed, NULL);
	timer_adjust_oneshot(cpu_timer, machine->primary_screen->time_until_pos(v_to_scanline(0), 0), 0);

	/* create a timer for IRQs and set up the first callback */
	irq_timer = timer_alloc(machine, clock_irq, NULL);
	irq_state = 0;
	schedule_next_irq(machine, -32);

	/* setup for save states */
	state_save_register_global(machine, irq_state);
	state_save_register_global(machine, ctrld);
	state_save_register_global(machine, flipscreen);
	state_save_register_global(machine, madsel_delay);
	state_save_register_global(machine, madsel_lastpc);
}


static MACHINE_RESET( missile )
{
	cputag_set_input_line(machine, "maincpu", 0, CLEAR_LINE);
	irq_state = 0;
}



/*************************************
 *
 *  VRAM access override
 *
 *************************************/

INLINE int get_madsel(const address_space *space)
{
	UINT16 pc = cpu_get_previouspc(space->cpu);

	/* if we're at a different instruction than last time, reset our delay counter */
	if (pc != madsel_lastpc)
		madsel_delay = 0;

	/* MADSEL signal disables standard address decoding and routes
        writes to video RAM; it is enabled if the IRQ signal is clear
        and the low 5 bits of the fetched opcode are 0x01 */
	if (!irq_state && (memory_decrypted_read_byte(space, pc) & 0x1f) == 0x01)
	{
		/* the MADSEL signal goes high 5 cycles after the opcode is identified;
            this effectively skips the indirect memory read. Since this is difficult
            to do in MAME, we just ignore the first two positive hits on MADSEL
            and only return TRUE on the third or later */
		madsel_lastpc = pc;
		return (++madsel_delay >= 4);
	}
	madsel_delay = 0;
	return 0;
}


INLINE offs_t get_bit3_addr(offs_t pixaddr)
{
	/* the 3rd bit of video RAM is scattered about various areas
        we take a 16-bit pixel address here and convert it into
        a video RAM address based on logic in the schematics */
	return	(( pixaddr & 0x0800) >> 1) |
			((~pixaddr & 0x0800) >> 2) |
			(( pixaddr & 0x07f8) >> 2) |
			(( pixaddr & 0x1000) >> 12);
}


static void write_vram(const address_space *space, offs_t address, UINT8 data)
{
	static const UINT8 data_lookup[4] = { 0x00, 0x0f, 0xf0, 0xff };
	offs_t vramaddr;
	UINT8 vramdata;
	UINT8 vrammask;

	/* basic 2 bit VRAM writes go to addr >> 2 */
	/* data comes from bits 6 and 7 */
	/* this should only be called if MADSEL == 1 */
	vramaddr = address >> 2;
	vramdata = data_lookup[data >> 6];
	vrammask = writeprom[(address & 7) | 0x10];
	space->machine->generic.videoram.u8[vramaddr] = (space->machine->generic.videoram.u8[vramaddr] & vrammask) | (vramdata & ~vrammask);

	/* 3-bit VRAM writes use an extra clock to write the 3rd bit elsewhere */
	/* on the schematics, this is the MUSHROOM == 1 case */
	if ((address & 0xe000) == 0xe000)
	{
		vramaddr = get_bit3_addr(address);
		vramdata = -((data >> 5) & 1);
		vrammask = writeprom[(address & 7) | 0x18];
		space->machine->generic.videoram.u8[vramaddr] = (space->machine->generic.videoram.u8[vramaddr] & vrammask) | (vramdata & ~vrammask);

		/* account for the extra clock cycle */
		cpu_adjust_icount(space->cpu, -1);
	}
}


static UINT8 read_vram(const address_space *space, offs_t address)
{
	offs_t vramaddr;
	UINT8 vramdata;
	UINT8 vrammask;
	UINT8 result = 0xff;

	/* basic 2 bit VRAM reads go to addr >> 2 */
	/* data goes to bits 6 and 7 */
	/* this should only be called if MADSEL == 1 */
	vramaddr = address >> 2;
	vrammask = 0x11 << (address & 3);
	vramdata = space->machine->generic.videoram.u8[vramaddr] & vrammask;
	if ((vramdata & 0xf0) == 0)
		result &= ~0x80;
	if ((vramdata & 0x0f) == 0)
		result &= ~0x40;

	/* 3-bit VRAM reads use an extra clock to read the 3rd bit elsewhere */
	/* on the schematics, this is the MUSHROOM == 1 case */
	if ((address & 0xe000) == 0xe000)
	{
		vramaddr = get_bit3_addr(address);
		vrammask = 1 << (address & 7);
		vramdata = space->machine->generic.videoram.u8[vramaddr] & vrammask;
		if (vramdata == 0)
			result &= ~0x20;

		/* account for the extra clock cycle */
		cpu_adjust_icount(space->cpu, -1);
	}
	return result;
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static VIDEO_UPDATE( missile )
{
	int x, y;

	/* draw the bitmap to the screen, looping over Y */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *dst = (UINT16 *)bitmap->base + y * bitmap->rowpixels;

		int effy = flipscreen ? ((256+24 - y) & 0xff) : y;
		UINT8 *src = &screen->machine->generic.videoram.u8[effy * 64];
		UINT8 *src3 = NULL;

		/* compute the base of the 3rd pixel row */
		if (effy >= 224)
			src3 = &screen->machine->generic.videoram.u8[get_bit3_addr(effy << 8)];

		/* loop over X */
		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			UINT8 pix = src[x / 4] >> (x & 3);
			pix = ((pix >> 2) & 4) | ((pix << 1) & 2);

			/* if we're in the lower region, get the 3rd bit */
			if (src3 != NULL)
				pix |= (src3[(x / 8) * 2] >> (x & 7)) & 1;

			dst[x] = pix;
		}
	}
	return 0;
}



/*************************************
 *
 *  Global read/write handlers
 *
 *************************************/

static WRITE8_HANDLER( missile_w )
{
	/* if we're in MADSEL mode, write to video RAM */
	if (get_madsel(space))
	{
		write_vram(space, offset, data);
		return;
	}

	/* otherwise, strip A15 and handle manually */
	offset &= 0x7fff;

	/* RAM */
	if (offset < 0x4000)
		space->machine->generic.videoram.u8[offset] = data;

	/* POKEY */
	else if (offset < 0x4800)
		pokey_w(space->machine->device("pokey"), offset & 0x0f, data);

	/* OUT0 */
	else if (offset < 0x4900)
	{
		flipscreen = ~data & 0x40;
		coin_counter_w(space->machine, 0, data & 0x20);
		coin_counter_w(space->machine, 1, data & 0x10);
		coin_counter_w(space->machine, 2, data & 0x08);
		set_led_status(space->machine, 1, ~data & 0x04);
		set_led_status(space->machine, 0, ~data & 0x02);
		ctrld = data & 1;
	}

	/* color RAM */
	else if (offset >= 0x4b00 && offset < 0x4c00)
		palette_set_color_rgb(space->machine, offset & 7, pal1bit(~data >> 3), pal1bit(~data >> 2), pal1bit(~data >> 1));

	/* watchdog */
	else if (offset >= 0x4c00 && offset < 0x4d00)
		watchdog_reset(space->machine);

	/* interrupt ack */
	else if (offset >= 0x4d00 && offset < 0x4e00)
	{
		if (irq_state)
		{
			cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);
			irq_state = 0;
		}
	}

	/* anything else */
	else
		logerror("%04X:Unknown write to %04X = %02X\n", cpu_get_pc(space->cpu), offset, data);
}


static READ8_HANDLER( missile_r )
{
	UINT8 result = 0xff;

	/* if we're in MADSEL mode, read from video RAM */
	if (get_madsel(space))
		return read_vram(space, offset);

	/* otherwise, strip A15 and handle manually */
	offset &= 0x7fff;

	/* RAM */
	if (offset < 0x4000)
		result = space->machine->generic.videoram.u8[offset];

	/* ROM */
	else if (offset >= 0x5000)
		result = memory_region(space->machine, "maincpu")[offset];

	/* POKEY */
	else if (offset < 0x4800)
		result = pokey_r(space->machine->device("pokey"), offset & 0x0f);

	/* IN0 */
	else if (offset < 0x4900)
	{
		if (ctrld)	/* trackball */
		{
			if (!flipscreen)
			    result = ((input_port_read(space->machine, "TRACK0_Y") << 4) & 0xf0) | (input_port_read(space->machine, "TRACK0_X") & 0x0f);
			else
			    result = ((input_port_read(space->machine, "TRACK1_Y") << 4) & 0xf0) | (input_port_read(space->machine, "TRACK1_X") & 0x0f);
		}
		else	/* buttons */
			result = input_port_read(space->machine, "IN0");
	}

	/* IN1 */
	else if (offset < 0x4a00)
		result = input_port_read(space->machine, "IN1");

	/* IN2 */
	else if (offset < 0x4b00)
		result = input_port_read(space->machine, "R10");

	/* anything else */
	else
		logerror("%04X:Unknown read from %04X\n", cpu_get_pc(space->cpu), offset);
	return result;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

/* complete memory map derived from schematics (implemented above) */
static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(missile_r, missile_w) AM_BASE_GENERIC(videoram)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( missile )
	PORT_START("IN0")	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("IN1")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x18, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_vblank, NULL)

	PORT_START("R10")	/* IN2 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("R10:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, "Right Coin" ) PORT_DIPLOCATION("R10:3,4")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x04, "*4" )
	PORT_DIPSETTING(    0x08, "*5" )
	PORT_DIPSETTING(    0x0c, "*6" )
	PORT_DIPNAME( 0x10, 0x00, "Center Coin" ) PORT_DIPLOCATION("R10:5")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x10, "*2" )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("R10:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x20, DEF_STR( French ) )
	PORT_DIPSETTING(    0x40, DEF_STR( German ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Spanish ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("R10:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("R8")	/* IN3 */
	PORT_DIPNAME( 0x03, 0x00, "Cities" ) PORT_DIPLOCATION("R8:1,2")
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x04, "Bonus Credit for 4 Coins" ) PORT_DIPLOCATION("R8:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Trackball Size" ) PORT_DIPLOCATION("R8:4")
	PORT_DIPSETTING(    0x00, "Large" )
	PORT_DIPSETTING(    0x08, "Mini" )
	PORT_DIPNAME( 0x70, 0x70, "Bonus City" ) PORT_DIPLOCATION("R8:5,6,7")
	PORT_DIPSETTING(    0x10, "8000" )
	PORT_DIPSETTING(    0x70, "10000" )
	PORT_DIPSETTING(    0x60, "12000" )
	PORT_DIPSETTING(    0x50, "14000" )
	PORT_DIPSETTING(    0x40, "15000" )
	PORT_DIPSETTING(    0x30, "18000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("R8:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("TRACK0_X")	/* FAKE */
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10)

	PORT_START("TRACK0_Y")	/* FAKE */
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("TRACK1_X")	/* FAKE */
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL

	PORT_START("TRACK1_Y")	/* FAKE */
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START( suprmatk )
	PORT_START("IN0")	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("IN1")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x18, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_vblank, NULL)

	PORT_START("R10")	/* IN2 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("R10:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, "Right Coin" ) PORT_DIPLOCATION("R10:3,4")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x04, "*4" )
	PORT_DIPSETTING(    0x08, "*5" )
	PORT_DIPSETTING(    0x0c, "*6" )
	PORT_DIPNAME( 0x10, 0x00, "Center Coin" ) PORT_DIPLOCATION("R10:5")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x10, "*2" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("R10:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, "Game" ) PORT_DIPLOCATION("R10:7,8")
	PORT_DIPSETTING(    0x00, "Missile Command" )
	PORT_DIPSETTING(    0x40, "Easy Super Missile Attack" )
	PORT_DIPSETTING(    0x80, "Reg. Super Missile Attack" )
	PORT_DIPSETTING(    0xc0, "Hard Super Missile Attack" )

	PORT_START("R8")	/* IN3 */
	PORT_DIPNAME( 0x03, 0x00, "Cities" ) PORT_DIPLOCATION("R8:1,2")
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x04, "Bonus Credit for 4 Coins" ) PORT_DIPLOCATION("R8:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Trackball Size" ) PORT_DIPLOCATION("R8:4")
	PORT_DIPSETTING(    0x00, "Large" )
	PORT_DIPSETTING(    0x08, "Mini" )
	PORT_DIPNAME( 0x70, 0x70, "Bonus City" ) PORT_DIPLOCATION("R8:5,6,7")
	PORT_DIPSETTING(    0x10, "8000" )
	PORT_DIPSETTING(    0x70, "10000" )
	PORT_DIPSETTING(    0x60, "12000" )
	PORT_DIPSETTING(    0x50, "14000" )
	PORT_DIPSETTING(    0x40, "15000" )
	PORT_DIPSETTING(    0x30, "18000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("R8:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("TRACK0_X")	/* FAKE */
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10)

	PORT_START("TRACK0_Y")	/* FAKE */
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("TRACK1_X")	/* FAKE */
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL

	PORT_START("TRACK1_Y")	/* FAKE */
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const pokey_interface pokey_config =
{
	{ DEVCB_NULL },
	DEVCB_INPUT_PORT("R8")
};



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( missile )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6502, MASTER_CLOCK/8)
	MDRV_CPU_PROGRAM_MAP(main_map)

	MDRV_MACHINE_START(missile)
	MDRV_MACHINE_RESET(missile)
	MDRV_WATCHDOG_VBLANK_INIT(8)

	/* video hardware */
	MDRV_PALETTE_LENGTH(8)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)

	MDRV_VIDEO_UPDATE(missile)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("pokey", POKEY, MASTER_CLOCK/8)
	MDRV_SOUND_CONFIG(pokey_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( missile )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "035820.02",    0x5000, 0x0800, CRC(7a62ce6a) SHA1(9a39978138dc28fdefe193bfae1b226391e471db) )
	ROM_LOAD( "035821.02",    0x5800, 0x0800, CRC(df3bd57f) SHA1(0916925d3c94d766d33f0e4badf6b0add835d748) )
	ROM_LOAD( "035822.02",    0x6000, 0x0800, CRC(a1cd384a) SHA1(a1dd0953423750a0fbc6e3dccbf2ca64ef5a1f54) )
	ROM_LOAD( "035823.02",    0x6800, 0x0800, CRC(82e552bb) SHA1(d0f22894f779c74ceef644c9f03d840d9545efea) )
	ROM_LOAD( "035824.02",    0x7000, 0x0800, CRC(606e42e0) SHA1(9718f84a73c66b4e8ef7805a7ab638a7380624e1) )
	ROM_LOAD( "035825.02",    0x7800, 0x0800, CRC(f752eaeb) SHA1(0339a6ce6744d2091cc7e07675e509b202b0f380) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "035826.01",   0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END


ROM_START( missile2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "35820-01.h1",  0x5000, 0x0800, CRC(41cbb8f2) SHA1(5dcb58276c08d75d36baadb6cefe30d4916de9b0) )
	ROM_LOAD( "35821-01.jk1", 0x5800, 0x0800, CRC(728702c8) SHA1(6f25af7133d3ec79029117162649f94e93f36e0e) )
	ROM_LOAD( "35822-01.kl1", 0x6000, 0x0800, CRC(28f0999f) SHA1(eb52b11c6757c8dc3be88b276ea4dc7dfebf7cf7) )
	ROM_LOAD( "35823-01.mn1", 0x6800, 0x0800, CRC(bcc93c94) SHA1(f0daa5d2835a856e2038612e755dc7ded28fc923) )
	ROM_LOAD( "35824-01.np1", 0x7000, 0x0800, CRC(0ca089c8) SHA1(7f69ee990fd4fa1f2fceca7fc66fcaa02e4d2314) )
	ROM_LOAD( "35825-01.r1",  0x7800, 0x0800, CRC(428cf0d5) SHA1(03cabbef50c33852fbbf38dd3eecaf70a82df82f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "035826.01",   0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END


ROM_START( suprmatk )
	ROM_REGION( 0x9000, "maincpu", 0 )
	ROM_LOAD( "035820.02",    0x5000, 0x0800, CRC(7a62ce6a) SHA1(9a39978138dc28fdefe193bfae1b226391e471db) )
	ROM_LOAD( "035821.02",    0x5800, 0x0800, CRC(df3bd57f) SHA1(0916925d3c94d766d33f0e4badf6b0add835d748) )
	ROM_LOAD( "035822.02",    0x6000, 0x0800, CRC(a1cd384a) SHA1(a1dd0953423750a0fbc6e3dccbf2ca64ef5a1f54) )
	ROM_LOAD( "035823.02",    0x6800, 0x0800, CRC(82e552bb) SHA1(d0f22894f779c74ceef644c9f03d840d9545efea) )
	ROM_LOAD( "035824.02",    0x7000, 0x0800, CRC(606e42e0) SHA1(9718f84a73c66b4e8ef7805a7ab638a7380624e1) )
	ROM_LOAD( "035825.02",    0x7800, 0x0800, CRC(f752eaeb) SHA1(0339a6ce6744d2091cc7e07675e509b202b0f380) )
	ROM_LOAD( "e0.rom",       0x8000, 0x0800, CRC(d0b20179) SHA1(e2a9855899b6ff96b8dba169e0ab83f00a95919f) )
	ROM_LOAD( "e1.rom",       0x8800, 0x0800, CRC(c6c818a3) SHA1(b9c92a85c07dd343d990e196d37b92d92a85a5e0) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "035826.01",   0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END


ROM_START( suprmatkd )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "035820.sma",   0x5000, 0x0800, CRC(75f01b87) SHA1(32ed71b6a869d7b361f244c384bbe6f407f6c6d7) )
	ROM_LOAD( "035821.sma",   0x5800, 0x0800, CRC(3320d67e) SHA1(5bb04b985421af6309818b94676298f4b90495cf) )
	ROM_LOAD( "035822.sma",   0x6000, 0x0800, CRC(e6be5055) SHA1(43912cc565cb43256a9193594cf36abab1c85d6f) )
	ROM_LOAD( "035823.sma",   0x6800, 0x0800, CRC(a6069185) SHA1(899cd8b378802eb6253d4bca7432797168595d53) )
	ROM_LOAD( "035824.sma",   0x7000, 0x0800, CRC(90a06be8) SHA1(f46fd6847bc9836d11ea0042df19fbf33ddab0db) )
	ROM_LOAD( "035825.sma",   0x7800, 0x0800, CRC(1298213d) SHA1(c8e4301704e3700c339557f2a833e70f6a068d5e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "035826.01",   0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END


/*

Missile Combat bootlegs by 'Videotron'

1x 6502A (main)
1x AY-3-8912 (sound)
1x oscillator 10000

PCB is marked: "VIDEOTRON BOLOGNA 002"

*/

ROM_START( mcombat )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "002-0-0.10a",  0x5000, 0x0800, CRC(589b81de) SHA1(06f18a837cedb0da5464dfaa04f92bd035db3752) )
	ROM_LOAD( "002-1-1.9a",   0x5800, 0x0800, CRC(08796a78) SHA1(e5aabe775889752ad1581098fcbf52ff1fa03b3b) )
	ROM_LOAD( "002-2-2.8a",   0x6000, 0x0800, CRC(59ab750c) SHA1(4555c27ddeb22ba895610a9c516fe574664a6f4b) )
	ROM_LOAD( "002-3-3.7a",   0x6800, 0x0800, CRC(3295cc3f) SHA1(2be0d492bd791df19d138d5bfe956361ee461989) )
	ROM_LOAD( "002-4-4.6a",   0x7000, 0x0800, CRC(aac71e95) SHA1(7daf115eb2cdde69b7c4de1e1a6ee68cd2fd0f2c) )
	ROM_LOAD( "002-5-5.5a",   0x7800, 0x0800, CRC(1b9a16e2) SHA1(03fb292bb6f815724b2fc4b2f561398000367373) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6f",   0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END


ROM_START( mcombata )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "002-0-0.10a",  0x5000, 0x0800, CRC(589b81de) SHA1(06f18a837cedb0da5464dfaa04f92bd035db3752) )
	ROM_LOAD( "002-1-1.9a",   0x5800, 0x0800, CRC(08796a78) SHA1(e5aabe775889752ad1581098fcbf52ff1fa03b3b) )
	ROM_LOAD( "002-2-2.8a",   0x6000, 0x0800, CRC(59ab750c) SHA1(4555c27ddeb22ba895610a9c516fe574664a6f4b) )
	ROM_LOAD( "3.bin",        0x6800, 0x0800, CRC(ddbfda20) SHA1(444daaa76751853f67a7c0e5bf620ae5623d2105) )
	ROM_LOAD( "4.bin",        0x7000, 0x0800, CRC(e3b5428d) SHA1(ac9eb459df68a117a49e92fbc5ed88faaf46a395) )
	ROM_LOAD( "002-5-5.5a",   0x7800, 0x0800, CRC(1b9a16e2) SHA1(03fb292bb6f815724b2fc4b2f561398000367373) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6f",   0x0000, 0x0020, CRC(86a22140) SHA1(2beebf7855e29849ada1823eae031fc98220bc43) )
ROM_END

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( suprmatk )
{
	int i;
	UINT8 *rom = memory_region(machine, "maincpu");

	for (i = 0; i < 0x40; i++)
	{
        rom[0x7CC0+i] = rom[0x8000+i];
        rom[0x5440+i] = rom[0x8040+i];
        rom[0x5B00+i] = rom[0x8080+i];
        rom[0x5740+i] = rom[0x80C0+i];
        rom[0x6000+i] = rom[0x8100+i];
        rom[0x6540+i] = rom[0x8140+i];
        rom[0x7500+i] = rom[0x8180+i];
        rom[0x7100+i] = rom[0x81C0+i];
        rom[0x7800+i] = rom[0x8200+i];
        rom[0x5580+i] = rom[0x8240+i];
        rom[0x5380+i] = rom[0x8280+i];
        rom[0x6900+i] = rom[0x82C0+i];
        rom[0x6E00+i] = rom[0x8300+i];
        rom[0x6CC0+i] = rom[0x8340+i];
        rom[0x7DC0+i] = rom[0x8380+i];
        rom[0x5B80+i] = rom[0x83C0+i];
        rom[0x5000+i] = rom[0x8400+i];
        rom[0x7240+i] = rom[0x8440+i];
        rom[0x7040+i] = rom[0x8480+i];
        rom[0x62C0+i] = rom[0x84C0+i];
        rom[0x6840+i] = rom[0x8500+i];
        rom[0x7EC0+i] = rom[0x8540+i];
        rom[0x7D40+i] = rom[0x8580+i];
        rom[0x66C0+i] = rom[0x85C0+i];
        rom[0x72C0+i] = rom[0x8600+i];
        rom[0x7080+i] = rom[0x8640+i];
        rom[0x7D00+i] = rom[0x8680+i];
        rom[0x5F00+i] = rom[0x86C0+i];
        rom[0x55C0+i] = rom[0x8700+i];
        rom[0x5A80+i] = rom[0x8740+i];
        rom[0x6080+i] = rom[0x8780+i];
        rom[0x7140+i] = rom[0x87C0+i];
        rom[0x7000+i] = rom[0x8800+i];
        rom[0x6100+i] = rom[0x8840+i];
        rom[0x5400+i] = rom[0x8880+i];
        rom[0x5BC0+i] = rom[0x88C0+i];
        rom[0x7E00+i] = rom[0x8900+i];
        rom[0x71C0+i] = rom[0x8940+i];
        rom[0x6040+i] = rom[0x8980+i];
        rom[0x6E40+i] = rom[0x89C0+i];
        rom[0x5800+i] = rom[0x8A00+i];
        rom[0x7D80+i] = rom[0x8A40+i];
        rom[0x7A80+i] = rom[0x8A80+i];
        rom[0x53C0+i] = rom[0x8AC0+i];
        rom[0x6140+i] = rom[0x8B00+i];
        rom[0x6700+i] = rom[0x8B40+i];
        rom[0x7280+i] = rom[0x8B80+i];
        rom[0x7F00+i] = rom[0x8BC0+i];
        rom[0x5480+i] = rom[0x8C00+i];
        rom[0x70C0+i] = rom[0x8C40+i];
        rom[0x7F80+i] = rom[0x8C80+i];
        rom[0x5780+i] = rom[0x8CC0+i];
        rom[0x6680+i] = rom[0x8D00+i];
        rom[0x7200+i] = rom[0x8D40+i];
        rom[0x7E40+i] = rom[0x8D80+i];
        rom[0x7AC0+i] = rom[0x8DC0+i];
        rom[0x6300+i] = rom[0x8E00+i];
        rom[0x7180+i] = rom[0x8E40+i];
        rom[0x7E80+i] = rom[0x8E80+i];
        rom[0x6280+i] = rom[0x8EC0+i];
        rom[0x7F40+i] = rom[0x8F00+i];
        rom[0x6740+i] = rom[0x8F40+i];
        rom[0x74C0+i] = rom[0x8F80+i];
        rom[0x7FC0+i] = rom[0x8FC0+i];
	}
}


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1980, missile,  0,       missile, missile,         0, ROT0, "Atari", "Missile Command (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1980, missile2, missile, missile, missile,         0, ROT0, "Atari", "Missile Command (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1981, suprmatk, missile, missile, suprmatk, suprmatk, ROT0, "Atari / Gencomp", "Super Missile Attack (for set 2)", GAME_SUPPORTS_SAVE )
GAME( 1981, suprmatkd,missile, missile, suprmatk,        0, ROT0, "Atari / Gencomp", "Super Missile Attack (not encrypted)", GAME_SUPPORTS_SAVE )

/* the bootlegs are on different hardware and don't work */
GAME( 198?, mcombat,  missile, missile, missile,         0, ROT0, "bootleg (Videotron)", "Missile Combat (Videotron bootleg, set 1)", GAME_NOT_WORKING )
GAME( 198?, mcombata, missile, missile, missile,         0, ROT0, "bootleg (Videotron)", "Missile Combat (Videotron bootleg, set 2)", GAME_NOT_WORKING )
