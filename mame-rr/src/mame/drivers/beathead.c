/***************************************************************************

    Atari "Stella on Steroids" hardware

    driver by Aaron Giles

    Games supported:
        * BeatHead

    Known bugs:
        * none known

****************************************************************************

    Memory map

    ===================================================================================================
    MAIN CPU
    ===================================================================================================
    00000000-0001FFFFF  R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Main RAM
    01800000-01BFFFFFF  R     xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Main ROM
    40000000-4000007FF  R/W   -------- -------- -------- xxxxxxxx   EEPROM
    41000000            R     -------- -------- -------- xxxxxxxx   Data from sound board
    41000000              W   -------- -------- -------- xxxxxxxx   Data to sound board
    41000100            R     -------- -------- -------- -----xxx   Interrupt enables
                              -------- -------- -------- -----x--      (scanline int enable)
                              -------- -------- -------- ------x-      (unknown int enable)
                              -------- -------- -------- -------x      (unknown int enable)
    41000100              W   -------- -------- -------- --------   Interrupt acknowledge
    41000104              W   -------- -------- -------- --------   Unknown int disable
    41000108              W   -------- -------- -------- --------   Unknown int disable
    4100010c              W   -------- -------- -------- --------   Scanline int disable
    41000114              W   -------- -------- -------- --------   Unknown int enable
    41000118              W   -------- -------- -------- --------   Unknown int enable
    4100011c              W   -------- -------- -------- --------   Scanline int enable
    41000200            R     -------- -------- xxxx--xx xxxx--xx   Player 2/3 inputs
                        R     -------- -------- xxxx---- --------      (player 3 joystick UDLR)
                        R     -------- -------- ------x- --------      (player 3 button 1)
                        R     -------- -------- -------x --------      (player 3 button 2)
                        R     -------- -------- -------- xxxx----      (player 2 joystick UDLR)
                        R     -------- -------- -------- ------x-      (player 2 button 1)
                        R     -------- -------- -------- -------x      (player 2 button 2)
    41000204            R     -------- -------- xxxx--xx xxxx--xx   Player 1/4 inputs
                        R     -------- -------- xxxx---- --------      (player 1 joystick UDLR)
                        R     -------- -------- ------x- --------      (player 1 button 1)
                        R     -------- -------- -------x --------      (player 1 button 2)
                        R     -------- -------- -------- xxxx----      (player 4 joystick UDLR)
                        R     -------- -------- -------- ------x-      (player 4 button 1)
                        R     -------- -------- -------- -------x      (player 4 button 2)
    41000208              W   -------- -------- -------- --------   Sound /RESET assert
    4100020C              W   -------- -------- -------- --------   Sound /RESET deassert
    41000220              W   -------- -------- -------- --------   Coin counter assert
    41000224              W   -------- -------- -------- --------   Coin counter deassert
    41000300            R     -------- -------- xxxxxxxx -xxx----   DIP switches/additional inputs
                        R     -------- -------- xxxxxxxx --------      (debug DIP switches)
                        R     -------- -------- -------- -x------      (service switch)
                        R     -------- -------- -------- --x-----      (sound output buffer full)
                        R     -------- -------- -------- ---x----      (sound input buffer full)
    41000304            R     -------- -------- -------- xxxxxxxx   Coin/service inputs
                        R     -------- -------- -------- xxxx----      (service inputs: R,RC,LC,L)
                        R     -------- -------- -------- ----xxxx      (coin inputs: R,RC,LC,L)
    41000400              W   -------- -------- -------- -xxxxxxx   Palette select
    41000500              W   -------- -------- -------- --------   EEPROM write enable
    41000600              W   -------- -------- -------- ----xxxx   Finescroll, vertical SYNC flags
                          W   -------- -------- -------- ----x---      (VBLANK)
                          W   -------- -------- -------- -----x--      (VSYNC)
                          W   -------- -------- -------- ------xx      (fine scroll value)
    41000700              W   -------- -------- -------- --------   Watchdog reset
    42000000-4201FFFF   R/W   -------- -------- xxxxxxxx xxxxxxxx   Palette RAM
                        R/W   -------- -------- x------- --------      (LSB of all three components)
                        R/W   -------- -------- -xxxxx-- --------      (red component)
                        R/W   -------- -------- ------xx xxx-----      (green component)
                        R/W   -------- -------- -------- ---xxxxx      (blue component)
    43000000              W   -------- -------- ----xxxx xxxxxxxx   HSYNC RAM address latch
                          W   -------- -------- ----x--- --------      (counter enable)
                          W   -------- -------- -----xxx xxxxxxxx      (RAM address)
    43000004            R/W   -------- -------- -------- xxxxx---   HSYNC RAM data latch
                        R/W   -------- -------- -------- x-------      (generate IRQ)
                        R/W   -------- -------- -------- -x------      (VRAM shift enable)
                        R/W   -------- -------- -------- --x-----      (HBLANK)
                        R/W   -------- -------- -------- ---x----      (/HSYNC)
                        R/W   -------- -------- -------- ----x---      (release wait for sync)
    43000008              W   -------- -------- -------- ---x-xx-   HSYNC unknown control
    8DF80000            R     -------- -------- -------- --------   Unknown
    8F380000-8F3FFFFF     W   -------- -------- -------- --------   VRAM latch address
    8F900000-8F97FFFF     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   VRAM transparent write
    8F980000-8F9FFFFF   R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   VRAM standard read/write
    8FB80000-8FBFFFFF     W   ----xxxx ----xxxx ----xxxx ----xxxx   VRAM "bulk" write
                          W   ----xxxx -------- -------- --------      (enable byte lanes for word 3?)
                          W   -------- ----xxxx -------- --------      (enable byte lanes for word 2?)
                          W   -------- -------- ----xxxx --------      (enable byte lanes for word 1?)
                          W   -------- -------- -------- ----xxxx      (enable byte lanes for word 0?)
    8FFF8000              W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   VRAM "bulk" data latch
    9E280000-9E2FFFFF     W   -------- -------- -------- --------   VRAM copy destination address latch
    ===================================================================================================

***************************************************************************/


#include "emu.h"
#include "cpu/asap/asap.h"
#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "includes/beathead.h"



#define MAX_SCANLINES	262



/*************************************
 *
 *  Machine init
 *
 *************************************/

static void update_interrupts(running_machine *machine);

static TIMER_DEVICE_CALLBACK( scanline_callback )
{
	beathead_state *state = (beathead_state *)timer.machine->driver_data;
	int scanline = param;

	/* update the video */
	timer.machine->primary_screen->update_now();

	/* on scanline zero, clear any halt condition */
	if (scanline == 0)
		cputag_set_input_line(timer.machine, "maincpu", INPUT_LINE_HALT, CLEAR_LINE);

	/* wrap around at 262 */
	scanline++;
	if (scanline >= MAX_SCANLINES)
		scanline = 0;

	/* set the scanline IRQ */
	state->irq_state[2] = 1;
	update_interrupts(timer.machine);

	/* set the timer for the next one */
	timer.adjust(double_to_attotime(attotime_to_double(timer.machine->primary_screen->time_until_pos(scanline)) - state->hblank_offset), scanline);
}


static MACHINE_START( beathead )
{
	atarigen_init(machine);
}


static MACHINE_RESET( beathead )
{
	beathead_state *state = (beathead_state *)machine->driver_data;

	/* reset the common subsystems */
	atarigen_eeprom_reset(&state->atarigen);
	atarigen_interrupt_reset(&state->atarigen, update_interrupts);
	atarijsa_reset();

	/* the code is temporarily mapped at 0 at startup */
	/* just copying the first 0x40 bytes is sufficient */
	memcpy(state->ram_base, state->rom_base, 0x40);

	/* compute the timing of the HBLANK interrupt and set the first timer */
	state->hblank_offset = attotime_to_double(machine->primary_screen->scan_period()) * ((455. - 336. - 25.) / 455.);
	timer_device *scanline_timer = machine->device<timer_device>("scan_timer");
	scanline_timer->adjust(double_to_attotime(attotime_to_double(machine->primary_screen->time_until_pos(0)) - state->hblank_offset));

	/* reset IRQs */
	state->irq_line_state = CLEAR_LINE;
	state->irq_state[0] = state->irq_state[1] = state->irq_state[2] = 0;
	state->irq_enable[0] = state->irq_enable[1] = state->irq_enable[2] = 0;
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

static void update_interrupts(running_machine *machine)
{
	beathead_state *state = (beathead_state *)machine->driver_data;
	int gen_int;

	/* compute the combined interrupt signal */
	gen_int  = state->irq_state[0] & state->irq_enable[0];
	gen_int |= state->irq_state[1] & state->irq_enable[1];
	gen_int |= state->irq_state[2] & state->irq_enable[2];
	gen_int  = gen_int ? ASSERT_LINE : CLEAR_LINE;

	/* if it's changed since the last time, call through */
	if (state->irq_line_state != gen_int)
	{
		state->irq_line_state = gen_int;
		//if (state->irq_line_state != CLEAR_LINE)
			cputag_set_input_line(machine, "maincpu", ASAP_IRQ0, state->irq_line_state);
		//else
			//asap_set_irq_line(ASAP_IRQ0, state->irq_line_state);
	}
}


static WRITE32_HANDLER( interrupt_control_w )
{
	beathead_state *state = (beathead_state *)space->machine->driver_data;
	int irq = offset & 3;
	int control = (offset >> 2) & 1;

	/* offsets 1-3 seem to be the enable latches for the IRQs */
	if (irq != 0)
		state->irq_enable[irq - 1] = control;

	/* offset 0 seems to be the interrupt ack */
	else
		state->irq_state[0] = state->irq_state[1] = state->irq_state[2] = 0;

	/* update the current state */
	update_interrupts(space->machine);
}


static READ32_HANDLER( interrupt_control_r )
{
	beathead_state *state = (beathead_state *)space->machine->driver_data;

	/* return the enables as a bitfield */
	return (state->irq_enable[0]) | (state->irq_enable[1] << 1) | (state->irq_enable[2] << 2);
}



/*************************************
 *
 *  EEPROM handling
 *
 *************************************/

static WRITE32_HANDLER( eeprom_data_w )
{
	beathead_state *state = (beathead_state *)space->machine->driver_data;

	if (state->eeprom_enabled)
	{
		mem_mask &= 0x000000ff;
		COMBINE_DATA(space->machine->generic.nvram.u32 + offset);
		state->eeprom_enabled = 0;
	}
}


static WRITE32_HANDLER( eeprom_enable_w )
{
	beathead_state *state = (beathead_state *)space->machine->driver_data;

	state->eeprom_enabled = 1;
}



/*************************************
 *
 *  Input handling
 *
 *************************************/

static READ32_HANDLER( input_2_r )
{
	beathead_state *state = (beathead_state *)space->machine->driver_data;
	int result = input_port_read(space->machine, "IN2");
	if (state->atarigen.sound_to_cpu_ready) result ^= 0x10;
	if (state->atarigen.cpu_to_sound_ready) result ^= 0x20;
	return result;
}



/*************************************
 *
 *  Sound communication
 *
 *************************************/

static READ32_HANDLER( sound_data_r )
{
	return atarigen_sound_r(space,offset,0xffff);
}


static WRITE32_HANDLER( sound_data_w )
{
	if (ACCESSING_BITS_0_7)
		atarigen_sound_w(space,offset, data, mem_mask);
}


static WRITE32_HANDLER( sound_reset_w )
{
	logerror("Sound reset = %d\n", !offset);
	cputag_set_input_line(space->machine, "jsa", INPUT_LINE_RESET, offset ? CLEAR_LINE : ASSERT_LINE);
}



/*************************************
 *
 *  Misc other I/O
 *
 *************************************/

static WRITE32_HANDLER( coin_count_w )
{
	coin_counter_w(space->machine, 0, !offset);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0001ffff) AM_RAM AM_BASE_MEMBER(beathead_state, ram_base)
	AM_RANGE(0x01800000, 0x01bfffff) AM_ROM AM_REGION("user1", 0) AM_BASE_MEMBER(beathead_state, rom_base)
	AM_RANGE(0x40000000, 0x400007ff) AM_RAM_WRITE(eeprom_data_w) AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x41000000, 0x41000003) AM_READWRITE(sound_data_r, sound_data_w)
	AM_RANGE(0x41000100, 0x41000103) AM_READ(interrupt_control_r)
	AM_RANGE(0x41000100, 0x4100011f) AM_WRITE(interrupt_control_w)
	AM_RANGE(0x41000200, 0x41000203) AM_READ_PORT("IN1")
	AM_RANGE(0x41000204, 0x41000207) AM_READ_PORT("IN0")
	AM_RANGE(0x41000208, 0x4100020f) AM_WRITE(sound_reset_w)
	AM_RANGE(0x41000220, 0x41000227) AM_WRITE(coin_count_w)
	AM_RANGE(0x41000300, 0x41000303) AM_READ(input_2_r)
	AM_RANGE(0x41000304, 0x41000307) AM_READ_PORT("IN3")
	AM_RANGE(0x41000400, 0x41000403) AM_WRITEONLY AM_BASE_MEMBER(beathead_state, palette_select)
	AM_RANGE(0x41000500, 0x41000503) AM_WRITE(eeprom_enable_w)
	AM_RANGE(0x41000600, 0x41000603) AM_WRITE(beathead_finescroll_w)
	AM_RANGE(0x41000700, 0x41000703) AM_WRITE(watchdog_reset32_w)
	AM_RANGE(0x42000000, 0x4201ffff) AM_RAM_WRITE(beathead_palette_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x43000000, 0x43000007) AM_READWRITE(beathead_hsync_ram_r, beathead_hsync_ram_w)
	AM_RANGE(0x8df80000, 0x8df80003) AM_READNOP	/* noisy x4 during scanline int */
	AM_RANGE(0x8f380000, 0x8f3fffff) AM_WRITE(beathead_vram_latch_w)
	AM_RANGE(0x8f900000, 0x8f97ffff) AM_WRITE(beathead_vram_transparent_w)
	AM_RANGE(0x8f980000, 0x8f9fffff) AM_RAM AM_BASE_GENERIC(videoram)
	AM_RANGE(0x8fb80000, 0x8fbfffff) AM_WRITE(beathead_vram_bulk_w)
	AM_RANGE(0x8fff8000, 0x8fff8003) AM_WRITEONLY AM_BASE_MEMBER(beathead_state, vram_bulk_latch)
	AM_RANGE(0x9e280000, 0x9e2fffff) AM_WRITE(beathead_vram_copy_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( beathead )
	PORT_START("IN0")		/* Player 1 */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("IN1")		/* Player 2 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0006, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_INCLUDE( atarijsa_iii )		/* audio board port */
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( beathead )
	MDRV_DRIVER_DATA(beathead_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", ASAP, ATARI_CLOCK_14MHz)
	MDRV_CPU_PROGRAM_MAP(main_map)

	MDRV_MACHINE_START(beathead)
	MDRV_MACHINE_RESET(beathead)
	MDRV_NVRAM_HANDLER(generic_1fill)

	MDRV_TIMER_ADD("scan_timer", scanline_callback)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(42*8, 262)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 42*8-1, 0*8, 30*8-1)
	MDRV_PALETTE_LENGTH(32768)

	MDRV_VIDEO_START(beathead)
	MDRV_VIDEO_UPDATE(beathead)

	/* sound hardware */
	MDRV_IMPORT_FROM(jsa_iii_mono)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( beathead )
	ROM_REGION( 0x14000, "jsa", 0 )			/* 64k + 16k for 6502 code */
	ROM_LOAD( "bhsnd.bin",  0x10000, 0x4000, CRC(dfd33f02) SHA1(479a4838c89691d5a4654a4cd84b6433a9e86109) )
	ROM_CONTINUE(           0x04000, 0xc000 )

	ROM_REGION32_LE( 0x400000, "user1", 0 )	/* 4MB for ASAP code */
	ROM_LOAD32_BYTE( "bhprog0.bin", 0x000000, 0x80000, CRC(87975721) SHA1(862cb3a290c829aedea26ee7100c50a12e9517e7) )
	ROM_LOAD32_BYTE( "bhprog1.bin", 0x000001, 0x80000, CRC(25d89743) SHA1(9ff9a41355aa6914efc4a44909026e648a3c40f3) )
	ROM_LOAD32_BYTE( "bhprog2.bin", 0x000002, 0x80000, CRC(87722609) SHA1(dbd766fa57f4528702a98db28ae48fb5d2a7f7df) )
	ROM_LOAD32_BYTE( "bhprog3.bin", 0x000003, 0x80000, CRC(a795d616) SHA1(d3b201be62486f3b12e1b20c4694eeff0b4e3fca) )
	ROM_LOAD32_BYTE( "bhpics0.bin", 0x200000, 0x80000, CRC(926bf65d) SHA1(49f25a2844ca1cd940d17fc56c0d2698e95e0e1d) )
	ROM_LOAD32_BYTE( "bhpics1.bin", 0x200001, 0x80000, CRC(a8f12e41) SHA1(693cb7a2510f34af5442870a6ae4d19445d991f9) )
	ROM_LOAD32_BYTE( "bhpics2.bin", 0x200002, 0x80000, CRC(00b96481) SHA1(39daa46321c1d4f8bce8c25d0450b97f1f19dedb) )
	ROM_LOAD32_BYTE( "bhpics3.bin", 0x200003, 0x80000, CRC(99c4f1db) SHA1(aba4440c5cdf413f970a0c65457e2d1b37caf2d6) )

	ROM_REGION( 0x80000, "adpcm", 0 )		/* 1MB for ADPCM */
	ROM_LOAD( "bhpcm0.bin",  0x00000, 0x20000, CRC(609ca626) SHA1(9bfc913fc4c3453b132595f8553245376bce3a51) )
	ROM_LOAD( "bhpcm1.bin",  0x20000, 0x20000, CRC(35511509) SHA1(41294b81e253db5d2f30f8589dd59729a31bb2bb) )
	ROM_LOAD( "bhpcm2.bin",  0x40000, 0x20000, CRC(f71a840a) SHA1(09d045552704cd1434307f9a36ce03c5c06a8ff6) )
	ROM_LOAD( "bhpcm3.bin",  0x60000, 0x20000, CRC(fedd4936) SHA1(430ed894fa4bfcd56ee5a8a8ef5e161246530e2d) )
ROM_END



/*************************************
 *
 *  Driver speedups
 *
 *************************************/

/*
    In-game hotspot @ 0180F8D8
*/


static UINT32 *speedup_data;
static READ32_HANDLER( speedup_r )
{
	int result = *speedup_data;
	if ((cpu_get_previouspc(space->cpu) & 0xfffff) == 0x006f0 && result == cpu_get_reg(space->cpu, ASAP_R3))
		cpu_spinuntil_int(space->cpu);
	return result;
}


static UINT32 *movie_speedup_data;
static READ32_HANDLER( movie_speedup_r )
{
	int result = *movie_speedup_data;
	if ((cpu_get_previouspc(space->cpu) & 0xfffff) == 0x00a88 && (cpu_get_reg(space->cpu, ASAP_R28) & 0xfffff) == 0x397c0 &&
		movie_speedup_data[4] == cpu_get_reg(space->cpu, ASAP_R1))
	{
		UINT32 temp = (INT16)result + movie_speedup_data[4] * 262;
		if (temp - (UINT32)cpu_get_reg(space->cpu, ASAP_R15) < (UINT32)cpu_get_reg(space->cpu, ASAP_R23))
			cpu_spinuntil_int(space->cpu);
	}
	return result;
}



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( beathead )
{
	/* initialize the common systems */
	atarijsa_init(machine, "IN2", 0x0040);

	/* prepare the speedups */
	speedup_data = memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00000ae8, 0x00000aeb, 0, 0, speedup_r);
	movie_speedup_data = memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00000804, 0x00000807, 0, 0, movie_speedup_r);
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1993, beathead, 0, beathead, beathead, beathead, ROT0, "Atari Games", "BeatHead (prototype)", 0 )
