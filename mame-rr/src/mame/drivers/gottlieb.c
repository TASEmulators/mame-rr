/***************************************************************************

    Gottlieb hardware
    dedicated to Warren Davis, Jeff Lee, Tim Skelly & David Thiel

    driver by Fabrice Frances & Nicola Salmoria

    Games supported:
        * Reactor
        * Q*Bert [6 sets]
        * Insector (prototype)
        * Tylz (prototype)
        * Argus (prototype)
        * Mad Planets
        * Krull
        * Knightmare (prototype)
        * Faster, Harder, More Challenging Q*bert (prototype)
        * Q*Bert's Qubes
        * M.A.C.H. 3
        * Screw Loose (prototype)
        * Cobra Command
        * Curve Ball
        * Us vs. Them
        * The Three Stooges in Brides Is Brides
        * Video Vince and the Game Factory (prototype)
        * Wiz Warz (prototype)

    Known issues:
        * none at this time

****************************************************************************

    Board Id           Game             Game Id
    ---------------    -------------    --------
                       Reactor          GV-100
                       Mad Planets      GV-102
    C-22248-2          Q*Bert           GV-103
                       Krull            GV-105
    22399-1 (GD-01)    M.A.C.H. 3       GV-109
                       Wiz Warz         GV-110
                       Knightmare       GV-112
                       Three Stooges    GV-113
                       Q*Bert Qubes     GV-118
                       Us vs. Them      GV-126
                       Video Vince      GV-132
                       Curve Ball       GV-134

****************************************************************************

    Thanks to Frans van Egmond for locating and dumping Tylz

    Notes:

    There was a bug in the hardware of the GG1 and GG2 boards, which is not
    emulated. The bug seems to have disappeared with the later revision of
    the board, e.g the board used by 3Stooges and Mach3 don't seem to have
    it). The bug was affecting the first character column (on horizontal
    games): screen memory could be used, but whatever was stored in this
    column, always the same character was displayed.

    This led to two consequences:
    - the image on the monitor had to be stretched so that the column was
      not visible
    - game designers were not using the first column. In fact, when the
      first column was ejected from the screen, the last one was usually
      out too, so it wasn't used either...

****************************************************************************

Reactor: earlier version of the board, with a different memory map

Main processor (8088 minimum mode)
0000-1fff RAM (NOT battery backed unlike all the others)
2000-2fff sprites
3000-37ff video ram
4000-4fff char generator ram
6000-67ff palette ram (palette of 16 colors)
7000-77ff i/o ports
8000-ffff ROM

memory mapped ports:

read:
7000    Dip switch
7001    Inputs 10-17
7002    trackball H
7003    trackball V
7004    Inputs 40-47

write:
7000    watchdog reset
7001    trackball clear
7002    Outputs 20-27
7003    Flipflop outputs:
        b0: F/B priority
        b1: horiz. flip
        b2: vert. flip
        b3: Output 33
        b4: coin counter
        b5: left lamp (1P/3Lives start)
        b6: middle lamps (2P/3Lives start, 1P/7Lives start)
        b7: right lamp (2P/7Lives start)
7004    Outputs 40-47

interrupts:
INTR not connected
NMI connected to vertical blank



Gottlieb games memory map

Main processor (8088 minimum mode)
0000-0fff RAM (battery backed)
1000-1fff RAM or ROM (selected with jumpers on the board)
2000-2fff RAM or ROM (selected with jumpers on the board)
3000-37ff sprites. The manual says there are 63 sprites (NOT 64),
          but the Q*Bert object priority test leaves sprite #63 dangling, so
          they are probably only 62.
3800-3fff video RAM
4000-4fff char generator RAM (can be replaced by a ROM twice as large,
          selection made with jumpers on the board. If it's ROM, the CPU
          cannot fully access it, I think it could read half the data if it
          wanted to but none of the games do that)
5000-57ff palette ram (palette of 16 colors)
5800-5fff i/o ports
6000-ffff ROM (not necessarily fully populated)

memory mapped ports:

read:
5800    Dip switch
5801    Inputs 10-17
5802    trackball H (optional)
5803    trackball V (optional)
5804    Inputs 40-47

write:
5800    watchdog reset
5801    trackball clear (optional)
5802    Outputs 20-27
5803    Flipflop outputs:
        b0: F/B priority
        b1: horiz. flip (sprite bank in Us vs. Them)
        b2: vert. flip (maybe genlock control in the laser disc games)
        b3: Output 33
        b4: coin counter (sprite bank in Q*Bert Qubes)
        b5: Q*Bert: kicker; Q*Bert Qubes: coin counter
        b5/b6: 3 Stooges: joystick input multiplexer
        b7: ?
5804    Outputs 40-47

interrupts:
INTR not connected
NMI connected to vertical blank



Sound processor (6502) memory map (earlier revision, used by games up to Krull):
0000-0fff RIOT (6532)
1000-1fff amplitude DAC
2000-2fff SC01 voice chip
3000-3fff voice clock DAC
4000-4fff expansion socket
5000-5fff expansion socket
6000-6fff expansion socket or ROM (selected with jumpers on the board)
7000-7fff ROM
(repeated in 8000-ffff, A15 only used in expansion socket)

Use of I/Os on the RIOT:
both ports A and B are programmed as inputs, A is connected to the main
motherboard, and B has SW1 (test) connected on bit 6.

interrupts:
INTR is connected to the RIOT, so an INTR can be generated by a variety
of sources, e.g active edge detection on PA7, or timer countdown.
It seems that all gottlieb games program the interrupt conditions so that
a positive active edge on PA7 triggers an interrupt, so the
main board ensures a command is correctly received by sending nul (0)
commands between two commands. Also, the timer interrupt is enabled but
doesn't seem to serve any purpose...(?)


In the later revision of the sound board, used from M.A.C.H. 3 onwards, there
are two 6502, two 8910, a DAC and a GI SP-0250 speech chip.


Video timings:
XTAL = 20 MHz
Horizontal video frequency: HSYNC = XTAL/4/318 = 15.72327 kHz
Video frequency: VSYNC = HSYNC/256 = 61.41903 Hz
VBlank duration: 1/VSYNC * (16/256) = 1017.6 us

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/6532riot.h"
#include "machine/laserdsc.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/samples.h"
#include "sound/sp0250.h"
#include "streams.h"
#include "includes/gottlieb.h"


#define LOG_AUDIO_DECODE	(0)

#define SYSTEM_CLOCK		XTAL_20MHz
#define CPU_CLOCK			XTAL_15MHz
#define NTSC_CLOCK			XTAL_14_31818MHz
#define LASERDISC_CLOCK		PERIOD_OF_555_ASTABLE(16000, 10000, 0.001e-6)

#define AUDIORAM_SIZE		0x400



/*************************************
 *
 *  Globals
 *
 *************************************/

static UINT8 joystick_select;
static UINT8 track[2];

static running_device *laserdisc;
static emu_timer *laserdisc_bit_timer;
static emu_timer *laserdisc_philips_timer;
static UINT8 laserdisc_select;
static UINT8 laserdisc_status;
static UINT16 laserdisc_philips_code;

static UINT8 *laserdisc_audio_buffer;
static UINT16 laserdisc_audio_address;
static INT16 laserdisc_last_samples[2];
static attotime laserdisc_last_time;
static attotime laserdisc_last_clock;
static UINT8 laserdisc_zero_seen;
static UINT8 laserdisc_audio_bits;
static UINT8 laserdisc_audio_bit_count;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static TIMER_CALLBACK( laserdisc_bit_callback );
static TIMER_CALLBACK( laserdisc_philips_callback );
static READ8_HANDLER( laserdisc_status_r );
static WRITE8_HANDLER( laserdisc_select_w );
static WRITE8_HANDLER( laserdisc_command_w );



/*************************************
 *
 *  Initialization
 *
 *************************************/

static MACHINE_START( gottlieb )
{
	/* register for save states */
	state_save_register_global(machine, joystick_select);
	state_save_register_global_array(machine, track);

	/* see if we have a laserdisc */
	laserdisc = machine->m_devicelist.first(LASERDISC);
	if (laserdisc != NULL)
	{
		/* attach to the I/O ports */
		memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x05805, 0x05807, 0, 0x07f8, laserdisc_status_r);
		memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x05805, 0x05805, 0, 0x07f8, laserdisc_command_w);	/* command for the player */
		memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x05806, 0x05806, 0, 0x07f8, laserdisc_select_w);

		/* allocate a timer for serial transmission, and one for philips code processing */
		laserdisc_bit_timer = timer_alloc(machine, laserdisc_bit_callback, NULL);
		laserdisc_philips_timer = timer_alloc(machine, laserdisc_philips_callback, NULL);

		/* create some audio RAM */
		laserdisc_audio_buffer = auto_alloc_array(machine, UINT8, AUDIORAM_SIZE);
		laserdisc_status = 0x38;

		/* more save state registration */
		state_save_register_global(machine, laserdisc_select);
		state_save_register_global(machine, laserdisc_status);
		state_save_register_global(machine, laserdisc_philips_code);

		state_save_register_global_pointer(machine, laserdisc_audio_buffer, AUDIORAM_SIZE);
		state_save_register_global(machine, laserdisc_audio_address);
		state_save_register_global_array(machine, laserdisc_last_samples);
		state_save_register_global(machine, laserdisc_last_time.seconds);
		state_save_register_global(machine, laserdisc_last_time.attoseconds);
		state_save_register_global(machine, laserdisc_last_clock.seconds);
		state_save_register_global(machine, laserdisc_last_clock.attoseconds);
		state_save_register_global(machine, laserdisc_zero_seen);
		state_save_register_global(machine, laserdisc_audio_bits);
		state_save_register_global(machine, laserdisc_audio_bit_count);
	}
}


static MACHINE_RESET( gottlieb )
{
	/* if we have a laserdisc, reset our philips code callback for the next line 17 */
	if (laserdisc != NULL)
		timer_adjust_oneshot(laserdisc_philips_timer, machine->primary_screen->time_until_pos(17), 17);
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static CUSTOM_INPUT( analog_delta_r )
{
	const char *string = (const char *)param;
	int which = string[0] - '0';

	return input_port_read(field->port->machine, &string[1]) - track[which];
}


static WRITE8_HANDLER( gottlieb_analog_reset_w )
{
	/* reset the trackball counters */
	track[0] = input_port_read_safe(space->machine, "TRACKX", 0);
	track[1] = input_port_read_safe(space->machine, "TRACKY", 0);
}


static CUSTOM_INPUT( stooges_joystick_r )
{
	static const char *const joyport[] = { "P2JOY", "P3JOY", "P1JOY", NULL };
	return (joyport[joystick_select & 3] != NULL) ? input_port_read(field->port->machine, joyport[joystick_select & 3]) : 0xff;
}



/*************************************
 *
 *  Output ports
 *
 *************************************/

static WRITE8_HANDLER( general_output_w )
{
	/* bits 0-3 control video features, and are different for laserdisc games */
	if (laserdisc == NULL)
		gottlieb_video_control_w(space, offset, data);
	else
		gottlieb_laserdisc_video_control_w(space, offset, data);

	/* bit 4 controls the coin meter */
	coin_counter_w(space->machine, 0, data & 0x10);

	/* bit 5 controls the knocker */
	output_set_value("knocker0", (data >> 5) & 1);

	/* bit 6 controls "COIN1"; it appears that no games used this */

	/* bit 7 controls the optional coin lockout; it appears that no games used this */
}


static WRITE8_HANDLER( reactor_output_w )
{
	general_output_w(space, offset, data & ~0xe0);
	set_led_status(space->machine, 0, data & 0x20);
	set_led_status(space->machine, 1, data & 0x40);
	set_led_status(space->machine, 2, data & 0x80);
}


static WRITE8_HANDLER( stooges_output_w )
{
	general_output_w(space, offset, data & ~0x60);
	joystick_select = (data >> 5) & 0x03;
}



/*************************************
 *
 *  Laserdisc I/O interface
 *
 *************************************/

static READ8_HANDLER( laserdisc_status_r )
{
	/* IP5 reads low 8 bits of philips code */
	if (offset == 0)
		return laserdisc_philips_code;

	/* IP6 reads middle 8 bits of philips code */
	if (offset == 1)
		return laserdisc_philips_code >> 8;

	/* IP7 reads either status or audio detect, depending on the select */
	if (laserdisc_select)
	{
		/* bits 0-2 frame number MSN */
		/* bit 3 audio buffer ready */
		/* bit 4 ready to send new laserdisc command? */
		/* bit 5 disc ready */
		/* bit 6 break in audio trasmission */
		/* bit 7 missing audio clock */
		return laserdisc_status;
	}
	else
	{
		UINT8 result = laserdisc_audio_buffer[laserdisc_audio_address++];
		laserdisc_audio_address %= AUDIORAM_SIZE;
		return result;
	}
	return 0;
}


static WRITE8_HANDLER( laserdisc_select_w )
{
	/* selects between reading audio data and reading status */
	laserdisc_select = data & 1;
}


static WRITE8_HANDLER( laserdisc_command_w )
{
	/* a write here latches data into a 8-bit register and starts
       a sequence of events that sends serial data to the player */

	/* set a timer to clock the bits through; a total of 12 bits are clocked */
	timer_adjust_oneshot(laserdisc_bit_timer, attotime_mul(LASERDISC_CLOCK, 10), (12 << 16) | data);

	/* it also clears bit 4 of the status (will be set when transmission is complete) */
	laserdisc_status &= ~0x10;
}



/*************************************
 *
 *  Laserdisc command/status interfacing
 *
 *************************************/

static TIMER_CALLBACK( laserdisc_philips_callback )
{
	int newcode = laserdisc_get_field_code(laserdisc, (param == 17) ? LASERDISC_CODE_LINE17 : LASERDISC_CODE_LINE18, TRUE);

	/* the PR8210 sends line 17/18 data on each frame; the laserdisc interface
       board receives notification and latches the most recent frame number */

	/* the logic detects a valid code when the top 4 bits are all 1s */
	if ((newcode & 0xf00000) == 0xf00000)
	{
		laserdisc_philips_code = newcode;
		laserdisc_status = (laserdisc_status & ~0x07) | ((newcode >> 16) & 7);
	}

	/* toggle to the next one */
	param = (param == 17) ? 18 : 17;
	timer_adjust_oneshot(laserdisc_philips_timer, machine->primary_screen->time_until_pos(param * 2), param);
}


static TIMER_CALLBACK( laserdisc_bit_off_callback )
{
	/* deassert the control line */
	laserdisc_line_w(laserdisc, LASERDISC_LINE_CONTROL, CLEAR_LINE);
}


static TIMER_CALLBACK( laserdisc_bit_callback )
{
	UINT8 bitsleft = param >> 16;
	UINT8 data = param;
	attotime duration;

	/* assert the line and set a timer for deassertion */
	laserdisc_line_w(laserdisc, LASERDISC_LINE_CONTROL, ASSERT_LINE);
	timer_set(machine, attotime_mul(LASERDISC_CLOCK, 10), NULL, 0, laserdisc_bit_off_callback);

	/* determine how long for the next command; there is a 555 timer with a
       variable resistor controlling the timing of the pulses. Nominally, the
       555 runs at 40083Hz, is divided by 10, and then is divided by 4 for a
       0 bit or 8 for a 1 bit. This gives 998usec per 0 pulse or 1996usec
       per 1 pulse. */
	duration = attotime_mul(LASERDISC_CLOCK, 10 * ((data & 0x80) ? 8 : 4));
	data <<= 1;

	/* if we're not out of bits, set a timer for the next one; else set the ready bit */
	if (bitsleft-- != 0)
		timer_adjust_oneshot(laserdisc_bit_timer, duration, (bitsleft << 16) | data);
	else
		laserdisc_status |= 0x10;
}



/*************************************
 *
 *  Laserdisc sound channel
 *
 *************************************/

INLINE void audio_end_state(void)
{
	/* this occurs either when the "break in transmission" condition is hit (no zero crossings
       for 400usec) or when the entire audio buffer is full */
	laserdisc_status |= 0x08;
	laserdisc_audio_bit_count = 0;
	laserdisc_audio_address = 0;
	if (laserdisc_audio_address != 0)
		printf("Got %d bytes\n", laserdisc_audio_address);
}


static void audio_process_clock(int logit)
{
	/* clock the bit through the shift register */
	laserdisc_audio_bits >>= 1;
	if (laserdisc_zero_seen)
		laserdisc_audio_bits |= 0x80;
	laserdisc_zero_seen = 0;

	/* if the buffer ready flag is set, then we are looking for the magic $67 pattern */
	if (laserdisc_status & 0x08)
	{
		if (laserdisc_audio_bits == 0x67)
		{
			if (logit)
				logerror(" -- got 0x67");
			laserdisc_status &= ~0x08;
			laserdisc_audio_address = 0;
		}
	}

	/* otherwise, we keep clocking bytes into the audio buffer */
	else
	{
		laserdisc_audio_bit_count++;

		/* if we collect 8 bits, add to the buffer */
		if (laserdisc_audio_bit_count == 8)
		{
			if (logit)
				logerror(" -- got new byte %02X", laserdisc_audio_bits);
			laserdisc_audio_bit_count = 0;
			laserdisc_audio_buffer[laserdisc_audio_address++] = laserdisc_audio_bits;

			/* if we collect a full buffer, signal */
			if (laserdisc_audio_address >= AUDIORAM_SIZE)
				audio_end_state();
		}
	}
}


static void audio_handle_zero_crossing(attotime zerotime, int logit)
{
	/* compute time from last clock */
	attotime deltaclock = attotime_sub(zerotime, laserdisc_last_clock);
	if (logit)
		logerror(" -- zero @ %s (delta=%s)", attotime_string(zerotime, 6), attotime_string(deltaclock, 6));

	/* if we are within 150usec, we count as a bit */
	if (attotime_compare(deltaclock, ATTOTIME_IN_USEC(150)) < 0)
	{
		if (logit)
			logerror(" -- count as bit");
		laserdisc_zero_seen++;
		return;
	}

	/* if we are within 215usec, we count as a clock */
	else if (attotime_compare(deltaclock, ATTOTIME_IN_USEC(215)) < 0)
	{
		if (logit)
			logerror(" -- clock, bit=%d", laserdisc_zero_seen);
		laserdisc_last_clock = zerotime;
	}

	/* if we are outside of 215usec, we are technically a missing clock
       however, due to sampling errors, it is best to assume this is just
       an out-of-skew clock, so we correct it if we are within 75usec */
	else if (attotime_compare(deltaclock, ATTOTIME_IN_USEC(275)) < 0)
	{
		if (logit)
			logerror(" -- skewed clock, correcting");
		laserdisc_last_clock = attotime_add(laserdisc_last_clock, ATTOTIME_IN_USEC(200));
	}

	/* we'll count anything more than 275us as an actual missing clock */
	else
	{
		if (logit)
			logerror(" -- missing clock");
		laserdisc_last_clock = zerotime;
	}

	/* we have a clock, process it */
	audio_process_clock(logit);
}


static void laserdisc_audio_process(running_device *device, int samplerate, int samples, const INT16 *ch0, const INT16 *ch1)
{
	int logit = LOG_AUDIO_DECODE && input_code_pressed(device->machine, KEYCODE_L);
	attotime time_per_sample = ATTOTIME_IN_HZ(samplerate);
	attotime curtime = laserdisc_last_time;
	int cursamp;

	if (logit)
		logerror("--------------\n");

	/* if no data, reset it all */
	if (ch1 == NULL)
	{
		laserdisc_last_time = attotime_add(curtime, attotime_mul(time_per_sample, samples));
		return;
	}

	/* iterate over samples */
	for (cursamp = 0; cursamp < samples; cursamp++)
	{
		INT16 sample = ch1[cursamp];

		/* start by logging the current sample and time */
		if (logit)
			logerror("%s: %d", attotime_string(attotime_add(attotime_add(curtime, time_per_sample), time_per_sample), 6), sample);

		/* if we are past the "break in transmission" time, reset everything */
		if (attotime_compare(attotime_sub(curtime, laserdisc_last_clock), ATTOTIME_IN_USEC(400)) > 0)
			audio_end_state();

		/* if this sample reinforces that the previous one ended a zero crossing, count it */
		if ((sample >= 256 && laserdisc_last_samples[1] >= 0 && laserdisc_last_samples[0] < 0) ||
			(sample <= -256 && laserdisc_last_samples[1] <= 0 && laserdisc_last_samples[0] > 0))
		{
			attotime zerotime;
			int fractime;

			/* compute the fractional position of the 0-crossing, between the two samples involved */
			fractime = (-laserdisc_last_samples[0] * 1000) / (laserdisc_last_samples[1] - laserdisc_last_samples[0]);

			/* use this fraction to compute the approximate actual zero crossing time */
			zerotime = attotime_add(curtime, attotime_div(attotime_mul(time_per_sample, fractime), 1000));

			/* determine if this is a clock; if it is, process */
			audio_handle_zero_crossing(zerotime, logit);
		}
		if (logit)
			logerror(" \n");

		/* update our sample tracking and advance time */
		laserdisc_last_samples[0] = laserdisc_last_samples[1];
		laserdisc_last_samples[1] = sample;
		curtime = attotime_add(curtime, time_per_sample);
	}

	/* remember the last time */
	laserdisc_last_time = curtime;
}



 /*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static TIMER_CALLBACK( nmi_clear )
{
	cputag_set_input_line(machine, "maincpu", INPUT_LINE_NMI, CLEAR_LINE);
}


static INTERRUPT_GEN( gottlieb_interrupt )
{
	/* assert the NMI and set a timer to clear it at the first visible line */
	cpu_set_input_line(device, INPUT_LINE_NMI, ASSERT_LINE);
	timer_set(device->machine, device->machine->primary_screen->time_until_pos(0), NULL, 0, nmi_clear);

	/* if we have a laserdisc, update it */
	if (laserdisc != NULL)
	{
		bitmap_t *dummy;

		/* set the "disc ready" bit, which basically indicates whether or not we have a proper video frame */
		if (!laserdisc_get_video(laserdisc, &dummy))
			laserdisc_status &= ~0x20;
		else
			laserdisc_status |= 0x20;
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( reactor_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xffff)
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x20ff) AM_MIRROR(0x0f00) AM_WRITEONLY AM_BASE_GENERIC(spriteram)							/* FRSEL */
	AM_RANGE(0x3000, 0x33ff) AM_MIRROR(0x0c00) AM_RAM_WRITE(gottlieb_videoram_w) AM_BASE_GENERIC(videoram)		/* BRSEL */
	AM_RANGE(0x4000, 0x4fff) AM_RAM_WRITE(gottlieb_charram_w) AM_BASE(&gottlieb_charram)				/* BOJRSEL1 */
/*  AM_RANGE(0x5000, 0x5fff) AM_WRITE() */																/* BOJRSEL2 */
	AM_RANGE(0x6000, 0x601f) AM_MIRROR(0x0fe0) AM_WRITE(gottlieb_paletteram_w) AM_BASE_GENERIC(paletteram)		/* COLSEL */
	AM_RANGE(0x7000, 0x7000) AM_MIRROR(0x0ff8) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x7001, 0x7001) AM_MIRROR(0x0ff8) AM_WRITE(gottlieb_analog_reset_w)						/* A1J2 interface */
	AM_RANGE(0x7002, 0x7002) AM_MIRROR(0x0ff8) AM_WRITE(gottlieb_sh_w)									/* trackball H */
	AM_RANGE(0x7003, 0x7003) AM_MIRROR(0x0ff8) AM_WRITE(reactor_output_w)								/* trackball V */
	AM_RANGE(0x7000, 0x7000) AM_MIRROR(0x0ff8) AM_READ_PORT("DSW")
	AM_RANGE(0x7001, 0x7001) AM_MIRROR(0x0ff8) AM_READ_PORT("IN1")										/* buttons */
	AM_RANGE(0x7002, 0x7002) AM_MIRROR(0x0ff8) AM_READ_PORT("IN2")										/* trackball H */
	AM_RANGE(0x7003, 0x7003) AM_MIRROR(0x0ff8) AM_READ_PORT("IN3")										/* trackball V */
	AM_RANGE(0x7004, 0x7004) AM_MIRROR(0x0ff8) AM_READ_PORT("IN4")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( gottlieb_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xffff)
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x1000, 0x1fff) AM_RAM AM_REGION("maincpu", 0x1000)	/* or ROM */
	AM_RANGE(0x2000, 0x2fff) AM_RAM AM_REGION("maincpu", 0x2000)	/* or ROM */
	AM_RANGE(0x3000, 0x30ff) AM_MIRROR(0x0700) AM_WRITEONLY AM_BASE_GENERIC(spriteram)							/* FRSEL */
	AM_RANGE(0x3800, 0x3bff) AM_MIRROR(0x0400) AM_RAM_WRITE(gottlieb_videoram_w) AM_BASE_GENERIC(videoram)		/* BRSEL */
	AM_RANGE(0x4000, 0x4fff) AM_RAM_WRITE(gottlieb_charram_w) AM_BASE(&gottlieb_charram)				/* BOJRSEL1 */
	AM_RANGE(0x5000, 0x501f) AM_MIRROR(0x07e0) AM_WRITE(gottlieb_paletteram_w) AM_BASE_GENERIC(paletteram)		/* COLSEL */
	AM_RANGE(0x5800, 0x5800) AM_MIRROR(0x07f8) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5801, 0x5801) AM_MIRROR(0x07f8) AM_WRITE(gottlieb_analog_reset_w)						/* A1J2 interface */
	AM_RANGE(0x5802, 0x5802) AM_MIRROR(0x07f8) AM_WRITE(gottlieb_sh_w)									/* OP20-27 */
	AM_RANGE(0x5803, 0x5803) AM_MIRROR(0x07f8) AM_WRITE(general_output_w)								/* OP30-37 */
/*  AM_RANGE(0x5804, 0x5804) AM_MIRROR(0x07f8) AM_WRITE()*/												/* OP40-47 */
	AM_RANGE(0x5800, 0x5800) AM_MIRROR(0x07f8) AM_READ_PORT("DSW")
	AM_RANGE(0x5801, 0x5801) AM_MIRROR(0x07f8) AM_READ_PORT("IN1")										/* IP10-17 */
	AM_RANGE(0x5802, 0x5802) AM_MIRROR(0x07f8) AM_READ_PORT("IN2")										/* trackball H */
	AM_RANGE(0x5803, 0x5803) AM_MIRROR(0x07f8) AM_READ_PORT("IN3")										/* trackball V */
	AM_RANGE(0x5804, 0x5804) AM_MIRROR(0x07f8) AM_READ_PORT("IN4")										/* IP40-47 */
	AM_RANGE(0x6000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( reactor )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Sound with Logos" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Bounce Chambers Points" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x02, "15" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Sound with Instructions" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x40, "12000" )
	PORT_DIPSETTING(    0xc0, "15000" )
	PORT_DIPSETTING(    0x80, "20000" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1)
	PORT_SERVICE_DIPLOC( 0x02, IP_ACTIVE_LOW, "SB1:8" )
	PORT_BIT ( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")	/* trackball H */
	PORT_BIT( 0xff, 0, IPT_SPECIAL ) PORT_CUSTOM(analog_delta_r, "0TRACKX")

	PORT_START("IN3")	/* trackball V */
	PORT_BIT( 0xff, 0, IPT_SPECIAL ) PORT_CUSTOM(analog_delta_r, "1TRACKY")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_INCLUDE(gottlieb1_sound)

	PORT_START("TRACKX")	/* trackball H */
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(15) PORT_KEYDELTA(20)

	PORT_START("TRACKY")	/* trackball V */
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(15) PORT_KEYDELTA(20)
INPUT_PORTS_END


static INPUT_PORTS_START( qbert )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Kicker" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, "Auto Round Advance (Cheat)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "SW5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "SW7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "SW8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	/* 0x40 must be connected to the IP16 line */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1)

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")      /* joystick */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL

	PORT_INCLUDE(gottlieb1_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( insector )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "25000" )
	PORT_DIPSETTING(    0x01, "30000" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Demo mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x50, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_INCLUDE(gottlieb1_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( tylz )
	PORT_START("DSW")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x11, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x11, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x22, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "15000 / 20000" )
	PORT_DIPSETTING(    0x20, "15000 / 30000" )
	PORT_DIPSETTING(    0x02, "20000 / 35000" )
	PORT_DIPSETTING(    0x22, "20000 / 40000" )

	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, "Normal Easy" )
	PORT_DIPSETTING(    0x80, "Normal Hard" )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hard ) )

	PORT_START("IN1")	/* ? */
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1) // cycle through test options, hold to do test
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_DIPNAME( 0x10, 0x10, "1" )  // probably nothing else here
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

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V (dial) */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")	/* ? */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE(gottlieb1_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( argusg )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x22, 0x02, "Bonus Human" )
	PORT_DIPSETTING(    0x00, "15000" )
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPSETTING(    0x20, "25000" )
	PORT_DIPSETTING(    0x22, "30000" )
	PORT_DIPNAME( 0x14, 0x10, "Initial Humans" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x04, "8" )
	PORT_DIPSETTING(    0x14, "10" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard )  )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")	/* trackball H */
	PORT_BIT( 0xff, 0, IPT_SPECIAL ) PORT_CUSTOM(analog_delta_r, "0TRACKX")

	PORT_START("IN3")	/* trackball V */
	PORT_BIT( 0xff, 0, IPT_SPECIAL ) PORT_CUSTOM(analog_delta_r, "1TRACKY")

/* NOTE: Buttons are shared for both players; are mirrored to each side of the controller */
	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("TRACKX")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(15) PORT_KEYDELTA(20)

	PORT_START("TRACKY")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(15) PORT_KEYDELTA(20)

	PORT_INCLUDE(gottlieb1_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( mplanets )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x02, "12000" )
	PORT_DIPNAME( 0x08, 0x00, "Allow Round Select" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x14, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1)
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V (dial) */
	PORT_BIT( 0xff, 0, IPT_SPECIAL ) PORT_CUSTOM(analog_delta_r, "1TRACKY")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_START("TRACKY")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(5) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X)

	PORT_INCLUDE(gottlieb1_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( krull )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPNAME( 0x14, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, "Hexagon" )
	PORT_DIPSETTING(    0x00, "Roving" )
	PORT_DIPSETTING(    0x20, "Stationary" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x40, "30000 30000" )
	PORT_DIPSETTING(    0x00, "30000 50000" )
	PORT_DIPSETTING(    0x80, "40000 50000" )
	PORT_DIPSETTING(    0xc0, "50000 75000" )

	PORT_START("IN1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY

	PORT_INCLUDE(gottlieb1_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( kngtmare )
	PORT_START("DSW")
	PORT_DIPNAME( 0x11, 0x11, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x11, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")	/* ? */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")	/* ? */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_INCLUDE(gottlieb1_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( qbertqub )
	PORT_START("DSW")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x35, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x24, "A 2/1 B 2/1" )
	PORT_DIPSETTING(    0x14, "A 1/1 B 4/1" )
	PORT_DIPSETTING(    0x30, "A 1/1 B 3/1" )
	PORT_DIPSETTING(    0x10, "A 1/1 B 2/1" )
	PORT_DIPSETTING(    0x00, "A 1/1 B 1/1" )
	PORT_DIPSETTING(    0x11, "A 2/3 B 2/1" )
	PORT_DIPSETTING(    0x15, "A 1/2 B 3/1" )
	PORT_DIPSETTING(    0x20, "A 1/2 B 2/1" )
	PORT_DIPSETTING(    0x21, "A 1/2 B 1/1" )
	PORT_DIPSETTING(    0x31, "A 1/2 B 1/5" )
	PORT_DIPSETTING(    0x04, "A 1/3 B 2/1" )
	PORT_DIPSETTING(    0x05, "A 1/3 B 1/1" )
	PORT_DIPSETTING(    0x35, DEF_STR( Free_Play ) )
/* 0x25 DEF_STR( 2C_1C )
   0x01 DEF_STR( 1C_1C )
   0x34 DEF_STR( Free_Play ) */
	PORT_DIPNAME( 0x02, 0x00, "1st Bonus Life" )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x02, "15000" )
	PORT_DIPNAME( 0x40, 0x00, "Additional Bonus Life" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x40, "25000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )

	PORT_START("IN1")      /* buttons */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1)

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")      /* joystick */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_INCLUDE(gottlieb1_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( curvebal )
	PORT_START("DSW")
	PORT_DIPNAME( 0x08, 0x00, "2 Players Game" )
	PORT_DIPSETTING(    0x08, "1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Credits" )
	PORT_DIPNAME( 0x11, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x11, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Coins" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( French ) )
	/* TODO: coinage is different when French is selected */
/* PORT_DIPNAME( 0xc2, 0x00, "French Coinage" )
PORT_DIPSETTING(    0x42, "A 3/1 B 1/2" )
PORT_DIPSETTING(    0x82, "A 1/5 B 1/2" )
PORT_DIPSETTING(    0x02, "A 2/1 B 2/3" )
PORT_DIPSETTING(    0xc0, "A 2/1 B 2/1" )
PORT_DIPSETTING(    0x80, "A 1/1 B 1/2" )
PORT_DIPSETTING(    0x40, "A 1/1 B 1/3" )
PORT_DIPSETTING(    0x00, "A 1/1 B 1/1" )
PORT_DIPSETTING(    0xc2, DEF_STR( Free_Play ) ) */
	PORT_DIPNAME( 0xc2, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x42, "A 4/1 B 1/1" )
	PORT_DIPSETTING(    0x82, "A 3/1 B 1/1" )
	PORT_DIPSETTING(    0x02, "A 2/1 B 1/1" )
	PORT_DIPSETTING(    0xc0, "A 2/1 B 2/1" )
	PORT_DIPSETTING(    0x80, "A 2/1 B 1/2" )
	PORT_DIPSETTING(    0x40, "A 2/1 B 1/3" )
	PORT_DIPSETTING(    0x00, "A 1/1 B 1/1" )
	PORT_DIPSETTING(    0xc2, DEF_STR( Free_Play ) )

	PORT_START("IN1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Swing") PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Pitch Left") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Pitch Right") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Bunt") PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_INCLUDE(gottlieb1_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( screwloo )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Demo mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "1st Bonus Atom at" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x50, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, "1st Bonus Hand at" )
	PORT_DIPSETTING(    0x00, "25000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPNAME( 0x80, 0x00, "Hands" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START("IN1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_8WAY

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Start 2P") PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Start 1P") PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_INCLUDE(gottlieb2_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( mach3 )
	PORT_START("DSW")
	/* TODO: values are different for 5 lives */
	PORT_DIPNAME( 0x09, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x24, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000 10000" )
	PORT_DIPSETTING(    0x04, "10000 20000" )
	PORT_DIPSETTING(    0x20, "10000 40000" )
	PORT_DIPSETTING(    0x24, "20000 60000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Infinite Lives (Cheat)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_INCLUDE(gottlieb2_sound)
INPUT_PORTS_END

static INPUT_PORTS_START( cobram3 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPNAME( 0x14, 0x00, "1st Bonus / 2nd Bonus" )
	PORT_DIPSETTING(    0x00, "20000 / None" )
	PORT_DIPSETTING(    0x10, "15000 / 30000" )
	PORT_DIPSETTING(    0x04, "20000 / 40000" )
	PORT_DIPSETTING(    0x14, "30000 / 50000" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x40, 0x00, "Random 1st Level")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Self Test")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_INCLUDE(gottlieb2_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( usvsthem )
	PORT_START("DSW")
	/* TODO: values are different for 5 lives */
	PORT_DIPNAME( 0x09, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Free_Play ) )
/*  PORT_DIPSETTING(    0x09, DEF_STR( Free_Play ) ) */
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_INCLUDE(gottlieb2_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( 3stooges )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING (   0x01, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING (   0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING (   0x02, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING (   0x00, "3" )
	PORT_DIPSETTING (   0x08, "5" )
	PORT_DIPNAME( 0x14, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING (   0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (   0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (   0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (   0x14, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "1st Bonus Life at" )
	PORT_DIPSETTING (   0x40, "10000" )
	PORT_DIPSETTING (   0x00, "20000" )
	PORT_DIPNAME( 0x80, 0x00, "And Bonus Life Every" )
	PORT_DIPSETTING (   0x80, "10000" )
	PORT_DIPSETTING (   0x00, "20000" )

	PORT_START("IN1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")	/* joystick inputs */
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(stooges_joystick_r, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P2JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("P3JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_INCLUDE(gottlieb2_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( vidvince )
	PORT_START("DSW")
	PORT_DIPNAME( 0x09, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x14, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPSETTING(    0x14, "40000" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard )  )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")	/* ? */
	PORT_SERVICE( 0x01, IP_ACTIVE_HIGH )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1) // cycle through test options, hold to do test
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")	/* ? */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_INCLUDE(gottlieb2_sound)
INPUT_PORTS_END


static INPUT_PORTS_START( wizwarz )
/* TODO: Bonus Life and Bonus Mine values are dependent upon each other */
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000, 30000" )
	PORT_DIPSETTING(    0x01, "25000, 35000" )
//  PORT_DIPSETTING(    0x00, "20000, 35000" )
//  PORT_DIPSETTING(    0x01, "30000, 40000" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Mine" )
	PORT_DIPSETTING(    0x00, "15000, 20000" )
	PORT_DIPSETTING(    0x08, "15000, 25000" )
//  PORT_DIPSETTING(    0x00, "10000, 15000" )
//  PORT_DIPSETTING(    0x08, "10000, 20000" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START("IN1")	/* ? */
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Select in Service Mode") PORT_CODE(KEYCODE_F1) // cycle through test options, hold to do test
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")	/* trackball H not used */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")	/* trackball V is a dial input */
	PORT_BIT( 0xff, 0, IPT_SPECIAL ) PORT_CUSTOM(analog_delta_r, "1TRACKY")

	PORT_START("IN4")	/* ? */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("TRACKY")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X)

	PORT_INCLUDE(gottlieb2_sound)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

/* the games can store char gfx data in either a 4k RAM area (128 chars), or */
/* a 8k ROM area (256 chars). */
static const gfx_layout bg_ram_layout =
{
	8,8,
	128,
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,32) },
	32*8
};

static const gfx_layout bg_rom_layout =
{
	8,8,
	256,
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,32) },
	32*8
};

static const gfx_layout fg_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	32*8
};

static GFXDECODE_START( gfxdecode )
	GFXDECODE_ENTRY( NULL,      0x4000, bg_ram_layout, 0, 1 )	/* the game dynamically modifies this */
	GFXDECODE_ENTRY( "bgtiles", 0x0000, bg_rom_layout, 0, 1 )
	GFXDECODE_ENTRY( "sprites", 0x0000, fg_layout,     0, 1 )
GFXDECODE_END



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const char *const reactor_sample_names[] =
{
	"*reactor",
	"fx_53.wav", /* "8 left" */
	"fx_54.wav", /* "16 left" */
	"fx_55.wav", /* "24 left" */
	"fx_56.wav", /* "32 left" */
	"fx_57.wav", /* "40 left" */
	"fx_58.wav", /* "warning, core unstable" */
	"fx_59.wav", /* "bonus" */
	"fx_31.wav", /* "chamber activated" */
	"fx_39a.wav", /* "2000" */
	"fx_39b.wav", /* "5000" */
	"fx_39c.wav", /* "10000" */
	"fx_39d.wav", /* "15000" */
	"fx_39e.wav", /* "20000" */
	"fx_39f.wav", /* "25000" */
	"fx_39g.wav", /* "30000" */
	"fx_39h.wav", /* "35000" */
	"fx_39i.wav", /* "40000" */
	"fx_39j.wav", /* "45000" */
	"fx_39k.wav", /* "50000" */
	"fx_39l.wav", /* "55000" */
     0	/* end of array */
};

static const char *const qbert_sample_names[] =
{
	"*qbert",
	"fx_17a.wav", /* random speech, voice clock 255 */
	"fx_17b.wav", /* random speech, voice clock 255 */
	"fx_17c.wav", /* random speech, voice clock 255 */
	"fx_17d.wav", /* random speech, voice clock 255 */
	"fx_17e.wav", /* random speech, voice clock 255 */
	"fx_17f.wav", /* random speech, voice clock 255 */
	"fx_17g.wav", /* random speech, voice clock 255 */
	"fx_17h.wav", /* random speech, voice clock 255 */
	"fx_18a.wav", /* random speech, voice clock 176 */
	"fx_18b.wav", /* random speech, voice clock 176 */
	"fx_18c.wav", /* random speech, voice clock 176 */
	"fx_18d.wav", /* random speech, voice clock 176 */
	"fx_18e.wav", /* random speech, voice clock 176 */
	"fx_18f.wav", /* random speech, voice clock 176 */
	"fx_18g.wav", /* random speech, voice clock 176 */
	"fx_18h.wav", /* random speech, voice clock 176 */
	"fx_19a.wav", /* random speech, voice clock 128 */
	"fx_19b.wav", /* random speech, voice clock 128 */
	"fx_19c.wav", /* random speech, voice clock 128 */
	"fx_19d.wav", /* random speech, voice clock 128 */
	"fx_19e.wav", /* random speech, voice clock 128 */
	"fx_19f.wav", /* random speech, voice clock 128 */
	"fx_19g.wav", /* random speech, voice clock 128 */
	"fx_19h.wav", /* random speech, voice clock 128 */
	"fx_20a.wav", /* random speech, voice clock 96 */
	"fx_20b.wav", /* random speech, voice clock 96 */
	"fx_20c.wav", /* random speech, voice clock 96 */
	"fx_20d.wav", /* random speech, voice clock 96 */
	"fx_20e.wav", /* random speech, voice clock 96 */
	"fx_20f.wav", /* random speech, voice clock 96 */
	"fx_20g.wav", /* random speech, voice clock 96 */
	"fx_20h.wav", /* random speech, voice clock 96 */
	"fx_21a.wav", /* random speech, voice clock 62 */
	"fx_21b.wav", /* random speech, voice clock 62 */
	"fx_21c.wav", /* random speech, voice clock 62 */
	"fx_21d.wav", /* random speech, voice clock 62 */
	"fx_21e.wav", /* random speech, voice clock 62 */
	"fx_21f.wav", /* random speech, voice clock 62 */
	"fx_21g.wav", /* random speech, voice clock 62 */
	"fx_21h.wav", /* random speech, voice clock 62 */
	"fx_22.wav", /* EH2 with decreasing voice clock */
	"fx_23.wav", /* O1 with varying voice clock */
	"fx_28.wav",
	"fx_36.wav",
	"knocker.wav",
	0	/* end of array */
};

static const samples_interface qbert_samples_interface =
{
	1,	/* one channel */
	qbert_sample_names
};

static const samples_interface reactor_samples_interface =
{
	1,	/* one channel */
	reactor_sample_names
};



/*************************************
 *
 *  Core machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( gottlieb_core )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", I8088, CPU_CLOCK/3)
	MDRV_CPU_PROGRAM_MAP(gottlieb_map)
	MDRV_CPU_VBLANK_INT("screen", gottlieb_interrupt)

	MDRV_MACHINE_START(gottlieb)
	MDRV_MACHINE_RESET(gottlieb)
	MDRV_NVRAM_HANDLER(generic_1fill)
	MDRV_WATCHDOG_VBLANK_INIT(16)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(SYSTEM_CLOCK/4, GOTTLIEB_VIDEO_HCOUNT, 0, GOTTLIEB_VIDEO_HBLANK, GOTTLIEB_VIDEO_VCOUNT, 0, GOTTLIEB_VIDEO_VBLANK)

	MDRV_GFXDECODE(gfxdecode)
	MDRV_PALETTE_LENGTH(16)

	MDRV_VIDEO_START(gottlieb)
	MDRV_VIDEO_UPDATE(gottlieb)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( gottlieb1 )
	MDRV_IMPORT_FROM(gottlieb_core)
	MDRV_IMPORT_FROM(gottlieb_soundrev1)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( gottlieb2 )
	MDRV_IMPORT_FROM(gottlieb_core)
	MDRV_IMPORT_FROM(gottlieb_soundrev2)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( g2laser )
	MDRV_IMPORT_FROM(gottlieb_core)
	MDRV_IMPORT_FROM(gottlieb_soundrev2)

	MDRV_LASERDISC_ADD("laserdisc", PIONEER_PR8210, "screen", "ldsound")
	MDRV_LASERDISC_AUDIO(laserdisc_audio_process)
	MDRV_LASERDISC_OVERLAY(gottlieb, GOTTLIEB_VIDEO_HCOUNT, GOTTLIEB_VIDEO_VCOUNT, BITMAP_FORMAT_INDEXED16)
	MDRV_LASERDISC_OVERLAY_CLIP(0, GOTTLIEB_VIDEO_HBLANK-1, 0, GOTTLIEB_VIDEO_VBLANK-8)

	MDRV_DEVICE_REMOVE("screen")
	MDRV_LASERDISC_SCREEN_ADD_NTSC("screen", BITMAP_FORMAT_INDEXED16)

	MDRV_SOUND_ADD("ldsound", LASERDISC, 0)
	MDRV_SOUND_ROUTE(0, "mono", 1.0)
	/* right channel is processed as data */
MACHINE_DRIVER_END



/*************************************
 *
 *  Specific machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( reactor )
	MDRV_IMPORT_FROM(gottlieb1)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(reactor_map)

	MDRV_NVRAM_HANDLER(0)

	/* sound hardware */
	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(reactor_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( qbert )
	MDRV_IMPORT_FROM(gottlieb1)

	/* video hardware */
	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(qbert_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( screwloo )
	MDRV_IMPORT_FROM(gottlieb2)

	MDRV_VIDEO_START(screwloo)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cobram3 )
	MDRV_IMPORT_FROM(g2laser)

	/* sound hardware */
	MDRV_SOUND_MODIFY("dac1")
	MDRV_SOUND_ROUTES_RESET()
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( reactor )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom7",         0x8000, 0x1000, CRC(a62d86fd) SHA1(9ffce668c6f0155568ccb62a6f12a9513f3c513f) )
	ROM_LOAD( "rom6",         0x9000, 0x1000, CRC(6ed841f3) SHA1(b94272457e6a2d0ec46b510d71d69b4ab92a44aa) )
	ROM_LOAD( "rom5",         0xa000, 0x1000, CRC(d90576a3) SHA1(52af7164dc59b9f7d8a1b3ef9d4c1fc81496899f) )
	ROM_LOAD( "rom4",         0xb000, 0x1000, CRC(0155daae) SHA1(3a94952c2830c7bea126cf39b84108936d96dc0e) )
	ROM_LOAD( "rom3",         0xc000, 0x1000, CRC(f8881385) SHA1(baa7ef7309aab9b382442b25ede5d7ecd6e2c370) )
	ROM_LOAD( "rom2",         0xd000, 0x1000, CRC(3caba35b) SHA1(4d3b894821a2cff0ef55cec2b1f62f2e33009b4d) )
	ROM_LOAD( "rom1",         0xe000, 0x1000, CRC(944e1ddf) SHA1(6b487f1cb405e2ba9345190e8ab6022c790882c1) )
	ROM_LOAD( "rom0",         0xf000, 0x1000, CRC(55930aed) SHA1(37ed60386935741e8cc0b8750bfcdf6f54c1bf9e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "snd1",         0x7000, 0x800, CRC(d958a0fd) SHA1(3c383076c68a929f96d844e89b09f3075f331906) )
	ROM_LOAD( "snd2",         0x7800, 0x800, CRC(5dc86942) SHA1(a449fcfb25521a0e7523184518b5204dac56e5f8) )

	ROM_REGION( 0x2000, "bgtiles", ROMREGION_ERASE00 )
	/* no ROMs; RAM is used instead */

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "fg3",          0x1000, 0x1000, CRC(8416ad53) SHA1(f868259b97675e58b6a7f1dc3c2a4ecf6aa0570e) )
	ROM_LOAD( "fg2",          0x3000, 0x1000, CRC(5489605a) SHA1(f4bbaaa8cd881dc164b118d1e516edeeea54c1d8) )
	ROM_LOAD( "fg1",          0x5000, 0x1000, CRC(18396c57) SHA1(39d90a842a03091414ed58d4128b524ecc20c9f1) )
	ROM_LOAD( "fg0",          0x7000, 0x1000, CRC(d1f20e15) SHA1(dba9aa0fec8b720a33d78b3dd1d7f74040048f7e) )
ROM_END


ROM_START( qbert )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qb-rom2.bin",  0xa000, 0x2000, CRC(fe434526) SHA1(4cfc5d52dd6c82163e035af82d6112c0c93a3797) )
	ROM_LOAD( "qb-rom1.bin",  0xc000, 0x2000, CRC(55635447) SHA1(ca6acdef1c9e06b33efe1f0a2df2dfb03723cfbe) )
	ROM_LOAD( "qb-rom0.bin",  0xe000, 0x2000, CRC(8e318641) SHA1(7f8f66d1e6a7905e93cce07fc92e8801370b7194) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "qb-snd1.bin",  0x7000, 0x800, CRC(15787c07) SHA1(8b7d03fbf2ebaa71b3a7e2f636a0d1bb9b796e43) )
	ROM_LOAD( "qb-snd2.bin",  0x7800, 0x800, CRC(58437508) SHA1(09d8053e7e99679b602dcda230d64db7fe6cb7f5) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "qb-bg0.bin",   0x0000, 0x1000, CRC(7a9ba824) SHA1(12aa6df499eb6996ee35f56acac403ff6290f844) )
	ROM_LOAD( "qb-bg1.bin",   0x1000, 0x1000, CRC(22e5b891) SHA1(5bb67e333255c0ea679ab4312256a8a71a950db8) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "qb-fg3.bin",   0x0000, 0x2000, CRC(dd436d3a) SHA1(ae16087a6ceec84551b5d7aae4036e0ed432cbb7) )
	ROM_LOAD( "qb-fg2.bin",   0x2000, 0x2000, CRC(f69b9483) SHA1(06894a1474c79c1274efbd32d7371179e7e0a661) )
	ROM_LOAD( "qb-fg1.bin",   0x4000, 0x2000, CRC(224e8356) SHA1(f7f26b879aa8b964ff6311136ed8157e44de736c) )
	ROM_LOAD( "qb-fg0.bin",   0x6000, 0x2000, CRC(2f695b85) SHA1(807d16459838f129e10b913890bbc95065d5dd40) )
ROM_END

ROM_START( qberta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qrom_2.bin",  0xa000, 0x2000, CRC(b54a8ffc) SHA1(5e19690f141d3db8f3bfa6c7de75026256758c1f) )
	ROM_LOAD( "qrom_1.bin",  0xc000, 0x2000, CRC(19d924e3) SHA1(af55ecb5b650e7b069d8be67eb9a9d0f3e69e3f1) )
	ROM_LOAD( "qrom_0.bin",  0xe000, 0x2000, CRC(2e7fad1b) SHA1(5c1feafe00c21ddddde67ab0093e847a5fc9ec2d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "qb-snd1.bin",  0x7000, 0x800, CRC(15787c07) SHA1(8b7d03fbf2ebaa71b3a7e2f636a0d1bb9b796e43) )
	ROM_LOAD( "qb-snd2.bin",  0x7800, 0x800, CRC(58437508) SHA1(09d8053e7e99679b602dcda230d64db7fe6cb7f5) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "qb-bg0.bin",   0x0000, 0x1000, CRC(7a9ba824) SHA1(12aa6df499eb6996ee35f56acac403ff6290f844) )
	ROM_LOAD( "qb-bg1.bin",   0x1000, 0x1000, CRC(22e5b891) SHA1(5bb67e333255c0ea679ab4312256a8a71a950db8) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "qb-fg3.bin",   0x0000, 0x2000, CRC(dd436d3a) SHA1(ae16087a6ceec84551b5d7aae4036e0ed432cbb7) )
	ROM_LOAD( "qb-fg2.bin",   0x2000, 0x2000, CRC(f69b9483) SHA1(06894a1474c79c1274efbd32d7371179e7e0a661) )
	ROM_LOAD( "qb-fg1.bin",   0x4000, 0x2000, CRC(224e8356) SHA1(f7f26b879aa8b964ff6311136ed8157e44de736c) )
	ROM_LOAD( "qb-fg0.bin",   0x6000, 0x2000, CRC(2f695b85) SHA1(807d16459838f129e10b913890bbc95065d5dd40) )
ROM_END

ROM_START( qbertj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qbj-rom2.bin", 0xa000, 0x2000, CRC(67bb1cb2) SHA1(23a7f8c86d6db9220a98b3f630c5d000e80f2d53) )
	ROM_LOAD( "qbj-rom1.bin", 0xc000, 0x2000, CRC(c61216e7) SHA1(e727b85dddc2963e33af6c02b675243f6fbe2710) )
	ROM_LOAD( "qbj-rom0.bin", 0xe000, 0x2000, CRC(69679d5c) SHA1(996d45517d0c01a1952fead05dbe201dbb7dca35) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "qb-snd1.bin",  0x7000, 0x800, CRC(15787c07) SHA1(8b7d03fbf2ebaa71b3a7e2f636a0d1bb9b796e43) )
	ROM_LOAD( "qb-snd2.bin",  0x7800, 0x800, CRC(58437508) SHA1(09d8053e7e99679b602dcda230d64db7fe6cb7f5) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "qb-bg0.bin",   0x0000, 0x1000, CRC(7a9ba824) SHA1(12aa6df499eb6996ee35f56acac403ff6290f844) )
	ROM_LOAD( "qb-bg1.bin",   0x1000, 0x1000, CRC(22e5b891) SHA1(5bb67e333255c0ea679ab4312256a8a71a950db8) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "qb-fg3.bin",   0x0000, 0x2000, CRC(dd436d3a) SHA1(ae16087a6ceec84551b5d7aae4036e0ed432cbb7) )
	ROM_LOAD( "qb-fg2.bin",   0x2000, 0x2000, CRC(f69b9483) SHA1(06894a1474c79c1274efbd32d7371179e7e0a661) )
	ROM_LOAD( "qb-fg1.bin",   0x4000, 0x2000, CRC(224e8356) SHA1(f7f26b879aa8b964ff6311136ed8157e44de736c) )
	ROM_LOAD( "qb-fg0.bin",   0x6000, 0x2000, CRC(2f695b85) SHA1(807d16459838f129e10b913890bbc95065d5dd40) )
ROM_END

ROM_START( myqbert )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mqb-rom2.bin",  0xa000, 0x2000, CRC(6860f957) SHA1(ebd68aeb6d54868295bd20cf64ee0187a52df0e3) )
	ROM_LOAD( "mqb-rom1.bin",  0xc000, 0x2000, CRC(11f0a4e4) SHA1(a805e51c40042fae209ace277abd9b35a990905b) )
	ROM_LOAD( "mqb-rom0.bin",  0xe000, 0x2000, CRC(12a90cb2) SHA1(a33203aea79fe43d1233a16e3fdddaceac6e4a20) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mqb-snd1.bin",  0x7000, 0x800, CRC(495ffcd2) SHA1(b2c16fffbd6af1c17fdb1a99844819e6ee0550ee) )
	ROM_LOAD( "mqb-snd2.bin",  0x7800, 0x800, CRC(9bbaa945) SHA1(13791b69cd6f426ad77a7d0537b10012feb0bc87) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "qb-bg0.bin",   0x0000, 0x1000, CRC(7a9ba824) SHA1(12aa6df499eb6996ee35f56acac403ff6290f844) ) /* chars */
	ROM_LOAD( "qb-bg1.bin",   0x1000, 0x1000, CRC(22e5b891) SHA1(5bb67e333255c0ea679ab4312256a8a71a950db8) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "mqb-fg3.bin",   0x0000, 0x2000, CRC(8b5d0852) SHA1(e2cf1679a7ec9e88b254b0a8d690a74d88db0cdc) ) /* sprites */
	ROM_LOAD( "mqb-fg2.bin",   0x2000, 0x2000, CRC(823f1e57) SHA1(a7305815d71d6e3b1c92c387a675e969edc77b7d) )
	ROM_LOAD( "mqb-fg1.bin",   0x4000, 0x2000, CRC(05343ae6) SHA1(6ae3e6949c9eb0df85216575ffd21adc939df0a2) )
	ROM_LOAD( "mqb-fg0.bin",   0x6000, 0x2000, CRC(abc71bdd) SHA1(b72c6867d8e342a6794a4bbab991761c01cfae44) )
ROM_END

ROM_START( qberttst )
    ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qbtst2.bin",   0xa000, 0x2000, CRC(55307b02) SHA1(8a41820211093779d9010b4c9e7d667ad3a31f23) )
	ROM_LOAD( "qbtst1.bin",   0xc000, 0x2000, CRC(e97fdd78) SHA1(98dd07043a72273240c593650aa9947199347870) )
	ROM_LOAD( "qbtst0.bin",   0xe000, 0x2000, CRC(94c9f588) SHA1(f586bcd8e6762614bed634a007508abea071754c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "qb-snd1.bin",  0x7000, 0x800, CRC(15787c07) SHA1(8b7d03fbf2ebaa71b3a7e2f636a0d1bb9b796e43) )
	ROM_LOAD( "qb-snd2.bin",  0x7800, 0x800, CRC(58437508) SHA1(09d8053e7e99679b602dcda230d64db7fe6cb7f5) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "qb-bg0.bin",   0x0000, 0x1000, CRC(7a9ba824) SHA1(12aa6df499eb6996ee35f56acac403ff6290f844) )
	ROM_LOAD( "qb-bg1.bin",   0x1000, 0x1000, CRC(22e5b891) SHA1(5bb67e333255c0ea679ab4312256a8a71a950db8) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "qb-fg3.bin",   0x0000, 0x2000, CRC(dd436d3a) SHA1(ae16087a6ceec84551b5d7aae4036e0ed432cbb7) )
	ROM_LOAD( "qb-fg2.bin",   0x2000, 0x2000, CRC(f69b9483) SHA1(06894a1474c79c1274efbd32d7371179e7e0a661) )
	ROM_LOAD( "qb-fg1.bin",   0x4000, 0x2000, CRC(224e8356) SHA1(f7f26b879aa8b964ff6311136ed8157e44de736c) )
	ROM_LOAD( "qb-fg0.bin",   0x6000, 0x2000, CRC(2f695b85) SHA1(807d16459838f129e10b913890bbc95065d5dd40) )
ROM_END

/* test rom, not a game */
ROM_START( qbtrktst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qb-rom2.bin",  0xa000, 0x2000, CRC(fe434526) SHA1(4cfc5d52dd6c82163e035af82d6112c0c93a3797) )
	ROM_LOAD( "qb-rom1.bin",  0xc000, 0x2000, CRC(55635447) SHA1(ca6acdef1c9e06b33efe1f0a2df2dfb03723cfbe) )
	ROM_LOAD( "gv103_t-ball-test_rom0_2764.c11c12",  0xe000, 0x2000, CRC(5d390cd2) SHA1(9031926a6f6179e340b67c3a7949062b4a75e3cf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "qb-snd1.bin",  0x7000, 0x800, CRC(15787c07) SHA1(8b7d03fbf2ebaa71b3a7e2f636a0d1bb9b796e43) )
	ROM_LOAD( "qb-snd2.bin",  0x7800, 0x800, CRC(58437508) SHA1(09d8053e7e99679b602dcda230d64db7fe6cb7f5) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "qb-bg0.bin",   0x0000, 0x1000, CRC(7a9ba824) SHA1(12aa6df499eb6996ee35f56acac403ff6290f844) )
	ROM_LOAD( "qb-bg1.bin",   0x1000, 0x1000, CRC(22e5b891) SHA1(5bb67e333255c0ea679ab4312256a8a71a950db8) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "qb-fg3.bin",   0x0000, 0x2000, CRC(dd436d3a) SHA1(ae16087a6ceec84551b5d7aae4036e0ed432cbb7) )
	ROM_LOAD( "qb-fg2.bin",   0x2000, 0x2000, CRC(f69b9483) SHA1(06894a1474c79c1274efbd32d7371179e7e0a661) )
	ROM_LOAD( "qb-fg1.bin",   0x4000, 0x2000, CRC(224e8356) SHA1(f7f26b879aa8b964ff6311136ed8157e44de736c) )
	ROM_LOAD( "qb-fg0.bin",   0x6000, 0x2000, CRC(2f695b85) SHA1(807d16459838f129e10b913890bbc95065d5dd40) )
ROM_END


ROM_START( insector )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom3",         0x8000, 0x2000, CRC(640881fd) SHA1(2832183e41ae7e631b61e4845fa68ce1c49edf29) )
	ROM_LOAD( "rom2",         0xa000, 0x2000, CRC(456bc3f4) SHA1(b61a56a65639f97399a8a3a4293ac2292edfd159) )
	ROM_LOAD( "rom1",         0xc000, 0x2000, CRC(706962af) SHA1(e40b567fdf6a3f7c6485808b4db45cea322c7724) )
	ROM_LOAD( "rom0",         0xe000, 0x2000, CRC(31cee24b) SHA1(3d21f5d530cc022f9633ad487e13a664848dd3e6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gv106s.bin",   0x7000, 0x1000, CRC(25bcc8bc) SHA1(adf401901f1479a5bffaed85135669b1133334b4) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "bg0",          0x0000, 0x1000, CRC(0dc2037e) SHA1(aa3fdec7884aad782e430182326f5b113f59bf00) )
	ROM_LOAD( "bg1",          0x1000, 0x1000, CRC(3dd73b94) SHA1(98b6592a907d6cb6a692c576f757bb612f8d3b72) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "fg3",          0x0000, 0x2000, CRC(9bbf5b6b) SHA1(3866d4f5231140e0b7ffe5daa4752d09d3bd7241) )
	ROM_LOAD( "fg2",          0x2000, 0x2000, CRC(5adf9986) SHA1(2ef0d002d7ab4c9199e40dfcb25747564ecc0495) )
	ROM_LOAD( "fg1",          0x4000, 0x2000, CRC(4bb16111) SHA1(88ac8c957c6968d355c494bf95ccf8a99152e5e9) )
	ROM_LOAD( "fg0",          0x6000, 0x2000, CRC(965f6b76) SHA1(faec9d43f39e730eefd08d4c3337ed4b230bf7e1) )
ROM_END


ROM_START( tylz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tylz.s4t",        0x8000, 0x2000, CRC(28ed146d) SHA1(52d72503b456b1411cd91724c6d524b78ca6fe03) )
	ROM_LOAD( "tylz.s4b",        0xa000, 0x2000, CRC(18ee09f9) SHA1(632896bfe7e14f93665671dbcc17b7cabc754a98) )
	ROM_LOAD( "tylz.r4",         0xc000, 0x2000, CRC(657c3d2e) SHA1(9908a2dd5109e632dff38b8b4b56160615355200) )
	ROM_LOAD( "tylz.n4",         0xe000, 0x2000, CRC(b2a15510) SHA1(15db4d1a2fb70d8111940246cd7a8ae06403cac5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "tylz.f2",   0x7000, 0x1000, CRC(ebcedba9) SHA1(94aee8e32bdc80bbc5dc1423ca97597bdb9d808c) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "tylz.m6",          0x0000, 0x1000, CRC(5e300b9b) SHA1(3b1e0371ba6d76ace893b92728c7a64b0e207d22) )
	ROM_LOAD( "tylz.n6",          0x1000, 0x1000, CRC(af56292e) SHA1(772b2fb8efaaaccd465ab9d61fc295ef09e3a10d) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "tylz.f12",          0x6000, 0x2000, CRC(6d2c5ad8) SHA1(851592d82680f9983f5d8c10c0a13fbaf31cfe41) )
	ROM_LOAD( "tylz.g12",          0x4000, 0x2000, CRC(1eb26e6f) SHA1(bc50069cefaafa302d559a2c2d3ab93f58900a24) )
	ROM_LOAD( "tylz.j12",          0x2000, 0x2000, CRC(bc319067) SHA1(cdaacd4d5a7644468c913076850f08d979cbb004) )
	ROM_LOAD( "tylz.k12",          0x0000, 0x2000, CRC(ff62bc4b) SHA1(71de79a92f23a1fcf4331c7fb1762870d74f7ea9) )
ROM_END


ROM_START( argusg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "arg_ram2_2732.c7",         0x1000, 0x1000, CRC(5d35b83e) SHA1(5a1c3b2ae138d5509b8daaf03036f000bd09d0fc) )
	ROM_LOAD( "arg_ram4_2732.c9c10",      0x2000, 0x1000, CRC(7180e823) SHA1(47124925d863b9b3784c0c990d4a4344e8d09372) )
	ROM_LOAD( "arg_rom4_2764.c16",        0x6000, 0x2000, CRC(2f48bd78) SHA1(b625a03b5a4989b67d5180fca7e9f6b7a24e6d2c) )
	ROM_LOAD( "arg_rom3_2764.c14c15",     0x8000, 0x2000, CRC(4dc2914c) SHA1(8ca0fd2ce1fc9f00afd30a638ff2f8787ef7e3d4) )
	ROM_LOAD( "arg_rom2_2764.c13c14",     0xa000, 0x2000, CRC(b5e9ee77) SHA1(dbdc176e3ca6be17b78eb98c07d5a9b5eaa28ba1) )
	ROM_LOAD( "arg_rom1_2764.c12c13",     0xc000, 0x2000, CRC(733d3d44) SHA1(03c11e89ed6906c0383dc19c0db2d21ebe69b128) )
	ROM_LOAD( "arg_rom0_2764.c11c12",     0xe000, 0x2000, CRC(e1906355) SHA1(4735370ff0dfe381358dfa41d82fab455ec3c016) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "arg_snd1_2716.u5",  0x7000, 0x800, CRC(3a6cf455) SHA1(0c701aa4d956947a101212b494b030cd2df5a2d6) )
	ROM_LOAD( "arg_snd2_2716.u6",  0x7800, 0x800, CRC(ddf32040) SHA1(61ae22faa013b29a5fbd9520073f172a98ca38ec) )

	ROM_REGION( 0x2000, "bgtiles", ROMREGION_ERASE00 )
	/* no ROMs; RAM is used instead */

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "arg_fg3_2764.k7k8",    0x0000, 0x2000, CRC(cdb6e25c) SHA1(d439a4c777c585d1ee89410816c9f7580f7e0ae8) )
	ROM_LOAD( "arg_fg2_2764.k6",      0x2000, 0x2000, CRC(f10af1be) SHA1(e9f9b90de374ff9cb7cc072625b4980cef3ab1a6) )
	ROM_LOAD( "arg_fg1_2764.k5",      0x4000, 0x2000, CRC(5add96e5) SHA1(ba8a1e54f12aab43c4dfce8f3bf8fcf4007b2eff) )
	ROM_LOAD( "arg_fg0_2764.k4",      0x6000, 0x2000, CRC(5b7bd588) SHA1(49ee6a747832f0d4d436c199db2022fd5dfb8d4a))
ROM_END


ROM_START( mplanets )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom4.c16",     0x6000, 0x2000, CRC(5402077f) SHA1(f4e8699ab3c6dfc0f86b6df86d2a5b35caf2ca73) )
	ROM_LOAD( "rom3.c14-15",  0x8000, 0x2000, CRC(5d18d740) SHA1(30307d98704c49dec5aecd0a1ec2f06f1869a5d2) )
	ROM_LOAD( "rom2.c13-14",  0xa000, 0x2000, CRC(960c3bb1) SHA1(305a7904fa8c0b9823ad186d1c5c7460c0900bad) )
	ROM_LOAD( "rom1.c12-13",  0xc000, 0x2000, CRC(eb515f10) SHA1(31c3519328eba7adc4a3b0adcc0384f606d81a57) )
	ROM_LOAD( "rom0.c11-12",  0xe000, 0x2000, CRC(74de78aa) SHA1(7ebd02e660c1413eff284a7ca77feeff41c1e2b7) )

	/* note from f205v: my original Gottlieb PCB only sports one 2732 sound EPROM labeled "snd.3h"
    It contains the two joint roms you can find herefollowing, therefore the sound is identical */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "snd1",         0x7000, 0x0800, CRC(453193a1) SHA1(317ec81f71661eaa92624c0304a52b635dcd5613) )
	ROM_LOAD( "snd2",         0x7800, 0x0800, CRC(f5ffc98f) SHA1(516e895df94942fc51f1b51eb9316d4296df82e7) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "bg0.e11-12",   0x0000, 0x1000, CRC(709aa24c) SHA1(95be072bf63320f4b44feaf88003ba011754e20f) )
	ROM_LOAD( "bg1.e13",      0x1000, 0x1000, CRC(4921e345) SHA1(7b6e03458222be501ed150ffbd489433027fc6cb) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "fg3.k7-8",     0x0000, 0x2000, CRC(c990b39f) SHA1(d1b6060744b78df430e914504b20e8693829bbd5) )
	ROM_LOAD( "fg2.k6",       0x2000, 0x2000, CRC(735e2522) SHA1(9fac59e9b1d81695d3da32107d640726cf96e31a) )
	ROM_LOAD( "fg1.k5",       0x4000, 0x2000, CRC(6456cc1c) SHA1(12c20f0ce49a7d3579049e8ba95e542c4aaee241) )
	ROM_LOAD( "fg0.k4",       0x6000, 0x2000, CRC(a920e325) SHA1(60f15d5014a55d9c18b06c17c7587d45716619e4) )
ROM_END

ROM_START( mplanetsuk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mpt_rom4.bin", 0x6000, 0x2000, CRC(cd88e23c) SHA1(03222e2600f7fb1c6844340d4a56eedfcdeaa3c8) )
	ROM_LOAD( "mpt_rom3.bin", 0x8000, 0x2000, CRC(dc355b2d) SHA1(ae3e376afc7a8cb049d0dd28bf3959cb76780999) )
	ROM_LOAD( "mpt_rom2.bin", 0xa000, 0x2000, CRC(846ddc23) SHA1(eed771d14b195e09f3aac713d19b3919c6c90ed6) )
	ROM_LOAD( "mpt_rom1.bin", 0xc000, 0x2000, CRC(94d67b87) SHA1(2cbf09f0ba3b6769de90d8f61913fec3010553e2) )
	ROM_LOAD( "mpt_rom0.bin", 0xe000, 0x2000, CRC(a9e30ad2) SHA1(39d830dda92ab5a6dbb44943be92bca0464e64e0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mpt_snd1.bin", 0x7000, 0x800, CRC(453193a1) SHA1(317ec81f71661eaa92624c0304a52b635dcd5613) )
	ROM_LOAD( "mpt_snd2.bin", 0x7800, 0x800, CRC(f5ffc98f) SHA1(516e895df94942fc51f1b51eb9316d4296df82e7) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "mpt_bg0.bin",  0x0000, 0x1000, CRC(709aa24c) SHA1(95be072bf63320f4b44feaf88003ba011754e20f) )
	ROM_LOAD( "mpt_bg1.bin",  0x1000, 0x1000, CRC(4921e345) SHA1(7b6e03458222be501ed150ffbd489433027fc6cb) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "mpt_fg3.bin",  0x0000, 0x2000, CRC(c990b39f) SHA1(d1b6060744b78df430e914504b20e8693829bbd5) )
	ROM_LOAD( "mpt_fg2.bin",  0x2000, 0x2000, CRC(735e2522) SHA1(9fac59e9b1d81695d3da32107d640726cf96e31a) )
	ROM_LOAD( "mpt_fg1.bin",  0x4000, 0x2000, CRC(6456cc1c) SHA1(12c20f0ce49a7d3579049e8ba95e542c4aaee241) )
	ROM_LOAD( "mpt_fg0.bin",  0x6000, 0x2000, CRC(a920e325) SHA1(60f15d5014a55d9c18b06c17c7587d45716619e4) )
ROM_END


ROM_START( krull )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ram2.bin",     0x1000, 0x1000, CRC(302feadf) SHA1(9d70de35e4f0490dc4e601070993ad146f250dea) )
	ROM_LOAD( "ram4.bin",     0x2000, 0x1000, CRC(79355a60) SHA1(57ad5c904b9ac4bf7c7d828bf755bbcbba6a4fd7) )
	ROM_LOAD( "rom4.bin",     0x6000, 0x2000, CRC(2b696394) SHA1(b18270f4ad97743f6ff8c4cbc93e523c77a8e794) )
	ROM_LOAD( "rom3.bin",     0x8000, 0x2000, CRC(14b0ee42) SHA1(276c4008a013806b3989c529f41cbc358ec49fd6) )
	ROM_LOAD( "rom2.bin",     0xa000, 0x2000, CRC(b5fad94a) SHA1(1bae895fbdd658cfb56c53cc2139282cc1e778de) )
	ROM_LOAD( "rom1.bin",     0xc000, 0x2000, CRC(1ad956a3) SHA1(f5b74b196fe1bd9ab48336e0051cbf29c650cfc1) )
	ROM_LOAD( "rom0.bin",     0xe000, 0x2000, CRC(a466afae) SHA1(d691cbb46e8c3b71f9b1688d7fcef36df82aa854) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "snd1.bin",     0x6000, 0x1000, CRC(dd2b30b4) SHA1(f01cb64932493bf69d4fc75a7fa809ff6f6e4263) )
	ROM_LOAD( "snd2.bin",     0x7000, 0x1000, CRC(8cab901b) SHA1(b886532828efc8cf442e2ee4ebbfe37acd489f62) )

	ROM_REGION( 0x2000, "bgtiles", ROMREGION_ERASE00 )
	/* no ROMs; RAM is used instead */

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "fg3.bin",      0x0000, 0x2000, CRC(82d77a45) SHA1(753476609c4bf4f0f0cd28d61fd8aef6967bda57) )
	ROM_LOAD( "fg2.bin",      0x2000, 0x2000, CRC(25a24317) SHA1(33d2c23a388b09c4a09b9893648c30fbd5482cc3) )
	ROM_LOAD( "fg1.bin",      0x4000, 0x2000, CRC(7e3ad7b0) SHA1(0de86e632e5a9e6c1ec82550b15dc25a17ab7066) )
	ROM_LOAD( "fg0.bin",      0x6000, 0x2000, CRC(7402dc19) SHA1(d6d1b8aca8e9ee3bdc57f4474d22b405963909ec) )
ROM_END


ROM_START( kngtmare )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gv112_rom3_2764.c14c15", 0x8000, 0x2000, CRC(47351270) SHA1(e6ca0b27b8f703cf73aad5f82d3b986823fbda88) )
	ROM_LOAD( "gv112_rom2_2764.c13c14", 0xa000, 0x2000, CRC(53e01f97) SHA1(0fbb92789609ba1df6e4ae56b2fc77a004e3a4ea) )
	ROM_LOAD( "gv112_rom1_2764.c12c13", 0xc000, 0x2000, CRC(5b340640) SHA1(8ccad017d5b9b748327baf22ff51d30ee96cb25e) )
	ROM_LOAD( "gv112_rom0_2764.c11c12", 0xe000, 0x2000, CRC(620dc629) SHA1(0d94b7c50ef499eb9bb3f4986a8d29547181f7ea) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gv112_snd",              0x7000, 0x1000, NO_DUMP )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "gv112_bg0_2732.e11e12",  0x0000, 0x1000, CRC(a74591fd) SHA1(e6916cfa44cbe4c0d58fb0307c70580a6fabfcf1) )
	ROM_LOAD( "gv112_bg1_2732.e13",     0x1000, 0x1000, CRC(5a226e6a) SHA1(faf119a8db823f2fc57c0e789b5f3486bca1feb1) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "gv112_fg3_2764.k7k8",    0x0000, 0x2000, CRC(d1886658) SHA1(2ba452acfb3548c02137c8732e1f7cf8f4c31275) )
	ROM_LOAD( "gv112_fg2_2764.k6",      0x2000, 0x2000, CRC(e1c73f0c) SHA1(3e91d6184f94b06ab0342504da387ac41d1a83b3) )
	ROM_LOAD( "gv112_fg1_2764.k5",      0x4000, 0x2000, CRC(724bc3ea) SHA1(305945e7224c3463083c7579a826ec7eba846067) )
	ROM_LOAD( "gv112_fg0_2764.k4",      0x6000, 0x2000, CRC(0311bbd9) SHA1(0a4f8268dab696bcb25738a482add24fb1f5f09d) )
ROM_END


ROM_START( sqbert )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qb-rom2.bin",  0xa000, 0x2000, CRC(1e3d4038) SHA1(d4402c5d16c0aa55efbceb83f0b30082b8434df7) )
	ROM_LOAD( "qb-rom1.bin",  0xc000, 0x2000, CRC(eaf3076c) SHA1(749a87b3c40ba0a2ecd2ca962786e066daf63e30) )
	ROM_LOAD( "qb-rom0.bin",  0xe000, 0x2000, CRC(61260a7e) SHA1(e2028a453aa34aaffa2c465f64a963504315df3c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "qb-snd1.bin",  0x7000, 0x800, CRC(15787c07) SHA1(8b7d03fbf2ebaa71b3a7e2f636a0d1bb9b796e43) )
	ROM_LOAD( "qb-snd2.bin",  0x7800, 0x800, CRC(58437508) SHA1(09d8053e7e99679b602dcda230d64db7fe6cb7f5) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "qb-bg0.bin",   0x0000, 0x1000, CRC(c3118eef) SHA1(2c320eb8aae8841046ac3fca3bdaeeba778360e4) )
	ROM_LOAD( "qb-bg1.bin",   0x1000, 0x1000, CRC(4f6d8075) SHA1(dc1897f939c8d837627ff6e06609afe305566a3b) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "qb-fg3.bin",   0x0000, 0x2000, CRC(ee595eda) SHA1(11777d95ba79bd0ec7b964b76c1dc129db857816) )
	ROM_LOAD( "qb-fg2.bin",   0x2000, 0x2000, CRC(59884c78) SHA1(5e77ef46ccd55f79a5fa90521baa7c22e3783fe5) )
	ROM_LOAD( "qb-fg1.bin",   0x4000, 0x2000, CRC(2a60e3ad) SHA1(9ed83017f6c8e44337ad76c68b095f2c3300aadb) )
	ROM_LOAD( "qb-fg0.bin",   0x6000, 0x2000, CRC(b11ad9d8) SHA1(5264598f33aa76455ae4107d0f265c2a372ed67a) )
ROM_END


ROM_START( qbertqub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qq-rom3.bin",  0x8000, 0x2000, CRC(c4dbdcd7) SHA1(34aaa4762073680e2b4d024ce7106315ffc6bcf3) )
	ROM_LOAD( "qq-rom2.bin",  0xa000, 0x2000, CRC(21a6c6cc) SHA1(6d4d81d9ad85be3792584e39dbeaf0dfeeda1503) )
	ROM_LOAD( "qq-rom1.bin",  0xc000, 0x2000, CRC(63e6c43d) SHA1(9435eb06dc069e5bf1c439f0c772fef3183745b0) )
	ROM_LOAD( "qq-rom0.bin",  0xe000, 0x2000, CRC(8ddbe438) SHA1(31112d711af5d4039491e99a0be0c088b3272482) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "qq-snd1.bin",  0x7000, 0x800, CRC(e704b450) SHA1(d509f54658e9f0264b9ab865a6f36e5423a28904) )
	ROM_LOAD( "qq-snd2.bin",  0x7800, 0x800, CRC(c6a98bf8) SHA1(cc5b5bb5966f5d79226f1f665a3f9fc934f4ef7f) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "qq-bg0.bin",   0x0000, 0x1000, CRC(050badde) SHA1(d049367e262cc6080e01d32227e86310166e00bb) )
	ROM_LOAD( "qq-bg1.bin",   0x1000, 0x1000, CRC(8875902f) SHA1(715e05b555d52c0445c4bec7fb6d1c02053505e6) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "qq-fg3.bin",   0x0000, 0x4000, CRC(91a949cc) SHA1(bd27a6bb744f73cb918318f1bc3aa5bde39417c2) )
	ROM_LOAD( "qq-fg2.bin",   0x4000, 0x4000, CRC(782d9431) SHA1(04d46014e9a5b80b8215b5338fa9de27e530e4a9) )
	ROM_LOAD( "qq-fg1.bin",   0x8000, 0x4000, CRC(71c3ac4c) SHA1(ed3444b3c8bd332b83f2700f1d6ea6bc9e185d8c) )
	ROM_LOAD( "qq-fg0.bin",   0xc000, 0x4000, CRC(6192853f) SHA1(907e62a3835bacda4104dcc1d7e946e4e8e821ef) )
ROM_END


ROM_START( curvebal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cb-rom-3.chp", 0x8000, 0x2000, CRC(72ad4d45) SHA1(9537eb360ed1d33d399cc2d8761c36b7d25fdae0) )
	ROM_LOAD( "cb-rom-2.chp", 0xa000, 0x2000, CRC(d46c3db5) SHA1(d4f464a6ebc090d100e890303557f0d05214033b) )
	ROM_LOAD( "cb-rom-1.chp", 0xc000, 0x2000, CRC(eb1e08bd) SHA1(f558664df12e4e15ef2779a0bdf99538f8c43ca3) )
	ROM_LOAD( "cb-rom-0.chp", 0xe000, 0x2000, CRC(401fc7e3) SHA1(86ca53cb6f1d88d5a95baa9524c6b42a1f7fc9c2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "yrom.sbd",     0x6000, 0x1000, CRC(4c313d9b) SHA1(c61a8c827f4b199fdfb6ffc0bc6cca396c73625e) )
	ROM_LOAD( "drom.sbd",     0x7000, 0x1000, CRC(cecece88) SHA1(4c6639f6f89f80b04b6ffbb5004ea2121e71f504) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "cb-bg-0.chp",  0x0000, 0x1000, CRC(d666a179) SHA1(3b9aca5272ae3f3d99ba55f5dc2db4eac82896bc) )
	ROM_LOAD( "cb-bg-1.chp",  0x1000, 0x1000, CRC(5e34ff4e) SHA1(f88234c0f46533540815e05479938810ea4fb4f8) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "cb-fg-3.chp",  0x0000, 0x2000, CRC(9c9452fe) SHA1(4cf5ee2efa5734849781aa57c2b92ed542d5cb4e) )
	ROM_LOAD( "cb-fg-2.chp",  0x2000, 0x2000, CRC(065131af) SHA1(fe78ee907f1d0e9f6fc3c96e4fa4aff390115147) )
	ROM_LOAD( "cb-fg-1.chp",  0x4000, 0x2000, CRC(1b7b7f94) SHA1(ffb0f163531c2f9d184d8733466d23ab9d48ea2e) )
	ROM_LOAD( "cb-fg-0.chp",  0x6000, 0x2000, CRC(e3a8230e) SHA1(c256b5ca25dc15c11d574d0ad823b34093933802) )
ROM_END


ROM_START( screwloo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom4",         0x6000, 0x2000, CRC(744a2513) SHA1(d6e8e016d9af984fae9b6667bedeea069637bd3a) )
	ROM_LOAD( "rom3",         0x8000, 0x2000, CRC(ffde5b5d) SHA1(50ac0e600935b8b1f8b68c40ad21a179b176e535) )
	ROM_LOAD( "rom2",         0xa000, 0x2000, CRC(97932b05) SHA1(6807c6e08edeb93aa7b4969eb78de3cf88a00b72) )
	ROM_LOAD( "rom1",         0xc000, 0x2000, CRC(571b65ca) SHA1(75077f4fab296b3802271fa77af588003570cde6) )
	ROM_LOAD( "rom0",         0xe000, 0x2000, CRC(6447fe54) SHA1(6391c841cafd35dd315d9fac99ed5d8304018747) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "drom1",        0xc000, 0x2000, CRC(ae965ade) SHA1(84a690cba8990fe6406b7cfbd6ea643a48446567) )

	ROM_REGION( 0x10000, "speech", 0 )
	ROM_LOAD( "yrom1",        0xe000, 0x2000, CRC(3719b0b5) SHA1(4f215ca2f15956374c4cd9484b6798f1c4d60fc7) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "bg0",          0x0000, 0x1000, CRC(1fd5b649) SHA1(4e2127a4458d54f094934c6f5f154a6db5d0c8b0) )
	ROM_LOAD( "bg1",          0x1000, 0x1000, CRC(c8ddb8ba) SHA1(58137a5043157d08af4d628cf5384d7f530393f7) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "fg3",          0x0000, 0x2000, CRC(97d4e63b) SHA1(e06ecb73148f16b33132767f8f0dffc37eeba5f1) )
	ROM_LOAD( "fg2",          0x2000, 0x2000, CRC(f76e56ca) SHA1(32252d424f31aa899d6c614f1d1006c2ef49df84) )
	ROM_LOAD( "fg1",          0x4000, 0x2000, CRC(698c395f) SHA1(46c6700b2943c35c9ac45791652eafbb4a4e059a) )
	ROM_LOAD( "fg0",          0x6000, 0x2000, CRC(f23269fb) SHA1(9a418a372da18cf33bcfba07202cf2ac7a1dcd1d) )
ROM_END


ROM_START( mach3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m3rom4.bin",   0x6000, 0x2000, CRC(8bfd5a44) SHA1(61f5c6c39047c1d0296e2cacce2be9525cb47176) )
	ROM_LOAD( "m3rom3.bin",   0x8000, 0x2000, CRC(b1b045af) SHA1(4e71ca4661bf5daaf9e2ffbb930ac3b13e2e57bd) )
	ROM_LOAD( "m3rom2.bin",   0xa000, 0x2000, CRC(fbdfb03d) SHA1(61d587558fc036fc3b55d3e36ebb6940e9eda258) )
	ROM_LOAD( "m3rom1.bin",   0xc000, 0x2000, CRC(3b0ba80b) SHA1(bc7e961311b40f05f2998f10f0a68f2e515c8e66) )
	ROM_LOAD( "m3rom0.bin",   0xe000, 0x2000, CRC(70c12bf4) SHA1(c26127b6e2a16791b3be8abac93be6af4f30fb3b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "m3drom1.bin",  0xd000, 0x1000, CRC(a6e29212) SHA1(a73aafc2efa99e9ae0aecbb6075a10f7178ac938) )

	ROM_REGION( 0x10000, "speech", 0 )
	ROM_LOAD( "m3yrom1.bin",  0xf000, 0x1000, CRC(eddf8872) SHA1(29ed0d1828639849bab826b3e2ab4eefac45fd85) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "mach3bg0.bin", 0x0000, 0x1000, CRC(ea2f5257) SHA1(664502dd2b7ee4ce96820da532615f3902b45557) )
	ROM_LOAD( "mach3bg1.bin", 0x1000, 0x1000, CRC(f543e4ce) SHA1(2a1b7dbbcd9756f836ca2e42973043b98a303082) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "mach3fg3.bin", 0x0000, 0x2000, CRC(472128b4) SHA1(8c6f36cab5ec8abb6db2e6d52530560664b950fe) )
	ROM_LOAD( "mach3fg2.bin", 0x2000, 0x2000, CRC(2a59e99e) SHA1(5c1faa244fc0f53cc2a52c8d4d40fb178706c2ed) )
	ROM_LOAD( "mach3fg1.bin", 0x4000, 0x2000, CRC(9b88767b) SHA1(8071e11906b3f0026f9a210cc5a236d95ca1f659) )
	ROM_LOAD( "mach3fg0.bin", 0x6000, 0x2000, CRC(0bae12a5) SHA1(7bc0b82ccab0e4498a7a2a9dc85f03125f25826e) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "mach3", 0, SHA1(d0f72bded7feff5c360f8749d6c27650a6964847) )
ROM_END


ROM_START( cobram3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bh03",   0x8000, 0x2000, CRC(755cbbf5) SHA1(e3ea146f8c344af1e9bf51548ae4902cb09e589a) )
	ROM_LOAD( "bh02",   0xa000, 0x2000, CRC(928ef670) SHA1(6f56454aaf76418ae94da2bd16b4e8309aca29a6) )
	ROM_LOAD( "bh01",   0xc000, 0x2000, CRC(7d86ab08) SHA1(26b7eb089ca3fe3f8b1531316ce8f95e33b380e5) )
	ROM_LOAD( "bh00",   0xe000, 0x2000, CRC(c19ad038) SHA1(4d20ae70d8ad1eaa61cb91d7a0cff6932fce30d2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "m3drom1.bin",  0xd000, 0x1000, CRC(a6e29212) SHA1(a73aafc2efa99e9ae0aecbb6075a10f7178ac938) )

	ROM_REGION( 0x10000, "speech", 0 )
	ROM_LOAD( "bh04",   0xe000, 0x2000, CRC(c3f61bc9) SHA1(d02374e6e29238def0cfb01c96c78b206f24d77e) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "bh09",   0x0000, 0x1000, CRC(8c5dfac0) SHA1(5be28f807c4eb9df76a8f7519086ae57953d8c6f) )
	ROM_LOAD( "bh0a",   0x1000, 0x1000, CRC(8b8da8dc) SHA1(9f03ac0e6b6396cd44843ea394d55d79848d6a27) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "bh05",   0x0000, 0x2000, CRC(d8f49994) SHA1(0631457264ff7f8d5fb1edc2c0211992a67c73e6) )
	ROM_LOAD( "bh08",   0x4000, 0x2000, CRC(d6439e2f) SHA1(84a6e574f76313ce065d8765f21bdda8fe5a9a7b) )
	ROM_LOAD( "bh07",   0x8000, 0x2000, CRC(f94668d2) SHA1(b5c3a54cf80097ac447a8140bd5877a66712e240) )
	ROM_LOAD( "bh06",   0xc000, 0x2000, CRC(ab6c7cf1) SHA1(3625f2e00a333552036bff99af25edeac5915d78) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "cobra", 0, SHA1(8390498294aca97a5d1769032e7b115d1a42f5d3) )
ROM_END


ROM_START( usvsthem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "usvs.rm4",     0x6000, 0x2000, CRC(0d7a4072) SHA1(84a7eec31037243185ab40ab269be0f83946ebd5) )
	ROM_LOAD( "usvs.rm3",     0x8000, 0x2000, CRC(6f32a671) SHA1(d54c1e09988f78ea706b4b0da675b27d9d407696) )
	ROM_LOAD( "usvs.rm2",     0xa000, 0x2000, CRC(36770716) SHA1(797b48ef48f563ed21ea263297fe0ed61c69f41e) )
	ROM_LOAD( "usvs.rm1",     0xc000, 0x2000, CRC(697bc989) SHA1(ebfc0868f949e5aba1efb8fbce06f795888d8e00) )
	ROM_LOAD( "usvs.rm0",     0xe000, 0x2000, CRC(30cf6bd9) SHA1(527ad3b96ea4a77f6d6f8a89a9215da490292297) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "usvsdrom.1",   0xc000, 0x2000, CRC(c0b5cab0) SHA1(b18e8fd9837bb52d6b3d86f2b652f6573c7cd3b3) )

	ROM_REGION( 0x10000, "speech", 0 )
	ROM_LOAD( "usvsyrom.1",   0xe000, 0x2000, CRC(c3d245ca) SHA1(d281b139ae6c58e855b2914a24ca4bc5f8800681) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "usvs.bg0",     0x0000, 0x1000, CRC(8a0de09f) SHA1(6b56ffbe8569625ff307510645683b4e3f04e753) )
	ROM_LOAD( "usvs.bg1",     0x1000, 0x1000, CRC(6fb64d3c) SHA1(0cf0fd60995e56eddc56c7fac6280abe3b87dbc9) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "usvs.fg3",     0x0000, 0x4000, CRC(98703015) SHA1(120e87ef640db3f23c1d2c0abf4b6acff7714512) )
	ROM_LOAD( "usvs.fg2",     0x4000, 0x4000, CRC(d3990707) SHA1(81d58f6bc6ec04b95036f81c4cd3516d0adf348e) )
	ROM_LOAD( "usvs.fg1",     0x8000, 0x4000, CRC(a2057430) SHA1(e24aa35cb27fa41b75f5c01f4c083dc6eeb04c0d) )
	ROM_LOAD( "usvs.fg0",     0xc000, 0x4000, CRC(7734e53f) SHA1(c1307596ba098c98e741f3c00686b514587e1d0a) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "usvsthem", 0, SHA1(c52530319dc70486f5ced95c121f1cab872e66d8) )
ROM_END


ROM_START( 3stooges )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gv113ram.4",   0x2000, 0x1000, CRC(533bff2a) SHA1(58d0be8add4b02dc3e27cf6b17a05baf4304f3ce) )
	ROM_LOAD( "gv113rom.4",   0x6000, 0x2000, CRC(8b6e52b8) SHA1(6e17e11afce92a7fa1735a724f0c0faf9375ac89) )
	ROM_LOAD( "gv113rom.3",   0x8000, 0x2000, CRC(b816d8c4) SHA1(86e16888492390034ac04e3f6a9f56422575c778) )
	ROM_LOAD( "gv113rom.2",   0xa000, 0x2000, CRC(b45b2a79) SHA1(7d0b19bec462ab67f518361afdf4b6982829ed07) )
	ROM_LOAD( "gv113rom.1",   0xc000, 0x2000, CRC(34ab051e) SHA1(df416aaf34d6bbbdd79ae633842355a292ed935d) )
	ROM_LOAD( "gv113rom.0",   0xe000, 0x2000, CRC(ab124329) SHA1(de1bc721eea74426035eec10a389f77b435aa9b9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "drom1",        0xc000, 0x2000, CRC(87a9fa10) SHA1(9c07837dce1384d6b51b716aa8ceeb5bc247a911) )

	ROM_REGION( 0x10000, "speech", 0 )
	ROM_LOAD( "yrom2",        0xc000, 0x2000, CRC(90f9c940) SHA1(646dacc902cf235948ac9c064c92390e2764370b) )
	ROM_LOAD( "yrom1",        0xe000, 0x2000, CRC(55f8ab30) SHA1(a6b6318f12fd4a1fab61b82cde33759da615d5b2) )

	ROM_REGION( 0x2000, "bgtiles", ROMREGION_ERASE00 )
	/* no ROMs; RAM is used instead */

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "gv113fg3",     0x0000, 0x2000, CRC(28071212) SHA1(33ce5cfae3491658f8b4cb977dc2da0a75dffee4) )
	ROM_LOAD( "gv113fg2",     0x2000, 0x2000, CRC(9fa3dfde) SHA1(693327c60691748bf16e486b2962ebe019009a69) )
	ROM_LOAD( "gv113fg1",     0x4000, 0x2000, CRC(fb223854) SHA1(b1eaad971edd2f3a2aed66c5f9d0f27d6ffd5466) )
	ROM_LOAD( "gv113fg0",     0x6000, 0x2000, CRC(95762c53) SHA1(e486a232e6b62ab21b9c3be521606dea2a451889) )
ROM_END


ROM_START( vidvince )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gv132_ram4_2732.c9c10",   0x2000, 0x1000, CRC(67a4927b) SHA1(41dfd13ea24bb3b0f8f917f4af5f6b33af1bc2e7) )
	ROM_LOAD( "gv132_rom4_2764.c16",     0x6000, 0x2000, CRC(3c5f39f5) SHA1(3722c30bcd60fc0c1c4ca4dd800a3654fba67599) )
	ROM_LOAD( "gv132_rom3_2764.c14c15",  0x8000, 0x2000, CRC(3983cb79) SHA1(3c527ed2428b8cb86a6896a74c873317a9f7b411) )
	ROM_LOAD( "gv132_rom2_2764.c13c14",  0xa000, 0x2000, CRC(0f5ebab9) SHA1(680874b9857565857375096d05203997669a7215) )
	ROM_LOAD( "gv132_rom1_2764.c12c13",  0xc000, 0x2000, CRC(a5bf40b7) SHA1(a5a193173fa7b764706bf8d3eaaaf18c6812e436) )
	ROM_LOAD( "gv132_rom0_2764.c11c12",  0xe000, 0x2000, CRC(2c02b598) SHA1(0c214f6625d6ef88bf89d96776683e15cf4a85c4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gv132_drom_snd_2764.k2",        0xc000, 0x2000, CRC(18d9d72f) SHA1(985007f49885621eb96e86dc51812983bd113550) )

	ROM_REGION( 0x10000, "speech", 0 )
	ROM_LOAD( "gv132_yrom2_snd_2764.k3",        0xc000, 0x2000, CRC(ff59f618) SHA1(c8b2cb1ab3b69f94dd6be87da8bdfc85c6ed8707) )
	ROM_LOAD( "gv132_yrom1_snd_2764.n3",        0xe000, 0x2000, CRC(befa4b97) SHA1(424b40844629631a3f31cc12c61ac7000b5f3eb9) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "gv132_bg0_2732.e11e12",          0x0000, 0x1000, CRC(1521bb4a) SHA1(a3a1209c74f1ca18f0be2d2c7b1fa2af625dfa5f) )
	/* RAM is used for the other half */

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "gv132_fg3_2764.k7k8",   0x0000, 0x2000, CRC(42a78a52) SHA1(7d24006d6746d21939dd0c6241a8d67c42073163) )
	ROM_LOAD( "gv132_fg2_2764.k6",     0x2000, 0x2000, CRC(8ae428ba) SHA1(2e7fe726f106f870ac1a7f3463d6e53174a7bc1b) )
	ROM_LOAD( "gv132_fg1_2764.k5",     0x4000, 0x2000, CRC(ea423550) SHA1(f683ddef80a424cee613ab16334a618d68f4595d) )
	ROM_LOAD( "gv132_fg0_2764.k4",     0x6000, 0x2000, CRC(74c996a6) SHA1(de72c9dcd6f6d42403ecaad5c202eb85e805dcc5) )
ROM_END


ROM_START( wizwarz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gv110_rom4_2764.c16",    0x6000, 0x2000, CRC(e4e6c29b) SHA1(8cc7b04afb613149c3d2fe160dc0ed1d3fe005af) )
	ROM_LOAD( "gv110_rom3_2764.c14c15", 0x8000, 0x2000, CRC(aa8e0fc4) SHA1(7bae2b48d14c097285d0dc1fa2a133fb24091602) )
	ROM_LOAD( "gv110_rom2_2764.c13c14", 0xa000, 0x2000, CRC(16c7d8ba) SHA1(b0950e45548ba991b0415927c8e46c98c4df5b13) )
	ROM_LOAD( "gv110_rom1_2764.c12c13", 0xc000, 0x2000, CRC(358895b5) SHA1(38a4a27849ab491a6e3dd3415fe684d1c71c392d) )
	ROM_LOAD( "gv110_rom0_2764.c11c12", 0xe000, 0x2000, CRC(f7157e17) SHA1(1b155602557ad173d74d4d5cf953b206b262987b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gv110_drom1_snd_2732.k2",0xd000, 0x1000, CRC(05ca79da) SHA1(f9e9b0de02d618aeb73f7218a49b41d7b94c24a4) )

	ROM_REGION( 0x10000, "speech", 0 )
	ROM_LOAD( "gv110_yrom1_snd_2732.n3",0xf000, 0x1000, CRC(1e3de643) SHA1(7717547c6c5b1ff178595c67f19265dc59130d90) )

	ROM_REGION( 0x2000, "bgtiles", 0 )
	ROM_LOAD( "gv110_bg0_2732.e11e12",  0x0000, 0x1000, CRC(7437813c) SHA1(11f93c8b965d861a1ebb0f894a956db4a77ee7e5) )
	ROM_LOAD( "gv110_bg1_2732.e13",     0x1000, 0x1000, CRC(70a54cc5) SHA1(769d17d44c3042df0ea9ff0c20f87f8e44dedc05) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "gv110_fg3_2764.k7k8",    0x0000, 0x2000, CRC(ce0c3e8b) SHA1(5823d4705091ecbd7dcd052719a88fd27f2fb8ee) )
	ROM_LOAD( "gv110_fg2_2764.k6",      0x2000, 0x2000, CRC(e42a166f) SHA1(c0f203cd08e6b5db45bd8eb3cfa05fe9a282a97d) )
	ROM_LOAD( "gv110_fg1_2764.k5",      0x4000, 0x2000, CRC(b947cf84) SHA1(7b04ebb408a230696dcc77c6f5b6b46e5b9ae7ea) )
	ROM_LOAD( "gv110_fg0_2764.k4",      0x6000, 0x2000, CRC(f7ba0fcb) SHA1(73d656ac4a4a2bc8d1f04d00c8af0d808ea31388) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( ramtiles )
{
	gottlieb_gfxcharlo = gottlieb_gfxcharhi = 0;
}


static DRIVER_INIT( romtiles )
{
	gottlieb_gfxcharlo = gottlieb_gfxcharhi = 1;
}


static DRIVER_INIT( stooges )
{
	DRIVER_INIT_CALL(ramtiles);
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x05803, 0x05803, 0, 0x07f8, stooges_output_w);
}


static DRIVER_INIT( screwloo )
{
	gottlieb_gfxcharlo = 0;
	gottlieb_gfxcharhi = 1;
}


static DRIVER_INIT( vidvince )
{
	gottlieb_gfxcharlo = 1;
	gottlieb_gfxcharhi = 0;
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

/* games using rev 1 sound board */
GAME( 1982, reactor,   0,        reactor,   reactor,  ramtiles, ROT0,   "Gottlieb", "Reactor", GAME_IMPERFECT_SOUND )
GAME( 1982, qbert,     0,        qbert,     qbert,    romtiles, ROT270, "Gottlieb", "Q*bert (US set 1)", GAME_IMPERFECT_SOUND )
GAME( 1982, qberta,    qbert,    qbert,     qbert,    romtiles, ROT270, "Gottlieb", "Q*bert (US set 2)", GAME_IMPERFECT_SOUND )
GAME( 1982, qbertj,    qbert,    qbert,     qbert,    romtiles, ROT270, "Gottlieb (Konami license)", "Q*bert (Japan)", GAME_IMPERFECT_SOUND )
GAME( 1982, myqbert,   qbert,    qbert,     qbert,    romtiles, ROT270, "Gottlieb", "Mello Yello Q*bert", GAME_IMPERFECT_SOUND )
GAME( 1982, qberttst,  qbert,    qbert,     qbert,    romtiles, ROT270, "Gottlieb", "Q*bert (early test version)", GAME_IMPERFECT_SOUND )
GAME( 1982, qbtrktst,  qbert,    qbert,     qbert,    romtiles, ROT270, "Gottlieb", "Q*bert Board Input Test Rom", GAME_IMPERFECT_SOUND )
GAME( 1982, insector,  0,        gottlieb1, insector, romtiles, ROT0,   "Gottlieb", "Insector (prototype)", 0 )
GAME( 1982, tylz,      0,        qbert,     tylz,     romtiles, ROT0,   "Mylstar",  "Tylz (prototype)", GAME_IMPERFECT_SOUND ) // modified sound hw?
GAME( 1984, argusg,    0,        gottlieb1, argusg,   ramtiles, ROT0,   "Gottlieb", "Argus (Gottlieb, prototype)" , 0) // aka Guardian / Protector?
GAME( 1983, mplanets,  0,        gottlieb1, mplanets, romtiles, ROT270, "Gottlieb", "Mad Planets", 0 )
GAME( 1983, mplanetsuk,mplanets, gottlieb1, mplanets, romtiles, ROT270, "Gottlieb (Taitel license)", "Mad Planets (UK)", 0 )
GAME( 1983, krull,     0,        gottlieb1, krull,    ramtiles, ROT270, "Gottlieb", "Krull", 0 )
GAME( 1983, kngtmare,  0,        gottlieb1, kngtmare, romtiles, ROT0,   "Gottlieb", "Knightmare (prototype)", GAME_NO_SOUND )
GAME( 1983, sqbert,    0,        qbert,     qbert,    romtiles, ROT270, "Mylstar", "Faster, Harder, More Challenging Q*bert (prototype)", GAME_IMPERFECT_SOUND )
GAME( 1983, qbertqub,  0,        qbert,     qbertqub, romtiles, ROT270, "Mylstar", "Q*bert's Qubes", GAME_IMPERFECT_SOUND )
GAME( 1984, curvebal,  0,        gottlieb1, curvebal, romtiles, ROT270, "Mylstar", "Curve Ball", 0 )

/* games using rev 2 sound board */
GAME( 1983, screwloo, 0,        screwloo,  screwloo, screwloo, ROT0,   "Mylstar", "Screw Loose (prototype)", 0 )
GAME( 1983, mach3,    0,        g2laser,   mach3,    romtiles, ROT0,   "Mylstar", "M.A.C.H. 3", 0 )
GAME( 1984, cobram3,  cobra,    cobram3,   cobram3,  romtiles, ROT0,   "Data East","Cobra Command (M.A.C.H. 3 hardware)", 0 )
GAME( 1984, usvsthem, 0,        g2laser,   usvsthem, romtiles, ROT0,   "Mylstar", "Us vs. Them", 0 )
GAME( 1984, 3stooges, 0,        gottlieb2, 3stooges, stooges,  ROT0,   "Mylstar", "The Three Stooges In Brides Is Brides", 0 )
GAME( 1984, vidvince, 0,        gottlieb2, vidvince, vidvince, ROT0,   "Mylstar", "Video Vince and the Game Factory (prototype)", GAME_IMPERFECT_GRAPHICS ) // sprite wrapping issues
GAME( 1984, wizwarz,  0,        gottlieb2, wizwarz,  romtiles, ROT0,   "Mylstar", "Wiz Warz (prototype)", 0 )
