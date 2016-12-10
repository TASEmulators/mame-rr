/***************************************************************************

    Atari Black Widow hardware

    Games supported:
        * Space Duel
        * Black Widow
        * Gravitar

****************************************************************************

    Black Widow memory map (preliminary)

    0000-04ff RAM
    0800      COIN_IN
    0a00      IN1
    0c00      IN2

    2000-27ff Vector generator RAM
    5000-7fff ROM



    BLACK WIDOW SWITCH SETTINGS (Atari, 1983)
    -----------------------------------------

    -------------------------------------------------------------------------------
    Settings of 8-Toggle Switch on Black Widow CPU PCB (at D4)
     8   7   6   5   4   3   2   1   Option
    -------------------------------------------------------------------------------
    Off Off                          1 coin/1 credit <
    On  On                           1 coin/2 credits
    On  Off                          2 coins/1 credit
    Off On                           Free play

            Off Off                  Right coin mechanism x 1 <
            On  Off                  Right coin mechanism x 4
            Off On                   Right coin mechanism x 5
            On  On                   Right coin mechanism x 6

                    Off              Left coin mechanism x 1 <
                    On               Left coin mechanism x 2

                        Off Off Off  No bonus coins (0)* <
                        Off On  On   No bonus coins (6)
                        On  On  On   No bonus coins (7)

                        On  Off Off  For every 2 coins inserted,
                                     logic adds 1 more coin (1)
                        Off On  Off  For every 4 coins inserted,
                                     logic adds 1 more coin (2)
                        On  On  Off  For every 4 coins inserted,
                                     logic adds 2 more coins (3)
                        Off Off On   For every 5 coins inserted,
                                     logic adds 1 more coin (4)
                        On  Off On   For every 3 coins inserted,
                                     logic adds 1 more coin (5)

    -------------------------------------------------------------------------------

    * The numbers in parentheses will appear on the BONUS ADDER line in the
      Operator Information Display (Figure 2-1) for these settings.
    < Manufacturer's recommended setting

    -------------------------------------------------------------------------------
    Settings of 8-Toggle Switch on Black Widow CPU PCB (at B4)
     8   7   6   5   4   3   2   1   Option

    Note: The bits are the exact opposite of the switch numbers - switch 8 is bit 0.
    -------------------------------------------------------------------------------
    Off Off                          Maximum start at level 13
    On  Off                          Maximum start at level 21 <
    Off On                           Maximum start at level 37
    On  On                           Maximum start at level 53

            Off Off                  3 spiders per game <
            On  Off                  4 spiders per game
            Off On                   5 spiders per game
            On  On                   6 spiders per game

                    Off Off          Easy game play
                    On  Off          Medium game play <
                    Off On           Hard game play
                    On  On           Demonstration mode

                            Off Off  Bonus spider every 20,000 points <
                            On  Off  Bonus spider every 30,000 points
                            Off On   Bonus spider every 40,000 points
                            On  On   No bonus

    -------------------------------------------------------------------------------

    < Manufacturer's recommended setting


    GRAVITAR SWITCH SETTINGS (Atari, 1982)
    --------------------------------------

    -------------------------------------------------------------------------------
    Settings of 8-Toggle Switch on Gravitar PCB (at B4)
     8   7   6   5   4   3   2   1   Option
    -------------------------------------------------------------------------------
    Off On                           Free play
    On  On                           1 coin for 2 credits
    Off Off                          1 coin for 1 credit <
    On  Off                          2 coins for 1 credit

            Off Off                  Right coin mechanism x 1 <
            On  Off                  Right coin mechanism x 4
            Off On                   Right coin mechanism x 5
            On  On                   Right coin mechanism x 6

                    Off              Left coin mechanism x 1 <
                    On               Left coin mechanism x 2

                        Off Off Off  No bonus coins <

                        Off On  Off  For every 4 coins inserted,
                                     logic adds 1 more coin
                        On  On  Off  For every 4 coins inserted,
                                     logic adds 2 more coins
                        Off Off On   For every 5 coins inserted,
                                     logic adds 1 more coin
                        On  Off On   For every 3 coins inserted,
                                     logic adds 1 more coin

                        Off On  On   No bonus coins
                        On  Off Off  ??? (not in manual!)
                        On  On  On   No bonus coins

    -------------------------------------------------------------------------------

    < Manufacturer's recommended setting

    -------------------------------------------------------------------------------
    Settings of 8-Toggle Switch on Gravitar PCB (at D4)
     8   7   6   5   4   3   2   1   Option
    -------------------------------------------------------------------------------
                            On  On   No bonus
                            Off Off  Bonus ship every 10,000 points <
     d   d               d  On  Off  Bonus ship every 20,000 points
     e   e               e  Off On   Bonus ship every 30,000 points
     s   s               s
     U   U          On   U           Easy game play <
                    Off              Hard game play
     t   t               t
     o   o  Off Off      o           3 ships per game
     N   N  On  Off      N           4 ships per game <
            Off On                   5 ships per game
            On  On                   6 ships per game

    -------------------------------------------------------------------------------

    < Manufacturer's recommended setting

    Space Duel Settings
    -------------------

    (Settings of 8-Toggle Switch on Space Duel game PCB at D4)
    Note: The bits are the exact opposite of the switch numbers - switch 8 is bit 0.

     8   7   6   5   4   3   2   1       Option
    On  Off                         3 ships per game
    Off Off                         4 ships per game $
    On  On                          5 ships per game
    Off On                          6 ships per game
            On  Off                *Easy game difficulty
            Off Off                 Normal game difficulty $
            On  On                  Medium game difficulty
            Off On                  Hard game difficulty
                    Off Off         English $
                    On  Off         German
                    On  On          Spanish
                    Off On          French
                                    Bonus life granted every:
                            Off On  8,000 points
                            Off Off 10,000 points
                            On  Off 15,000 points
                            On  On  No bonus life

    $Manufacturer's suggested settings
    *Easy-In the beginning of the first wave, 3 targets appear on the
    screen.  Targets increase by one in each new wave.
    Normal-Space station action is the same as 'Easy'.  Fighter action has
    4 targets in the beginning of the first wave.  Targets increase by 2
    in each new wave.  Targets move faster and more targets enter.
    Medium and Hard-In the beginning of the first wave, 4 targets appear
    on the screen.  Targets increase by 2 in each new wave.  As difficulty
    increases, targets move faster, and more targets enter.


    (Settings of 8-Toggle Switch on Space Duel game PCB at B4)
     8   7   6   5   4   3   2   1       Option
    Off On                          Free play
    Off Off                        *1 coin for 1 game (or 1 player) $
    On  On                          1 coin for 2 game (or 2 players)
    On  Off                         2 coins for 1 game (or 1 player)
            Off Off                 Right coin mech x 1 $
            On  Off                 Right coin mech x 4
            Off On                  Right coin mech x 5
            On  On                  Right coin mech x 6
                    Off             Left coin mech x 1 $
                    On              Left coin mech x 2
                        Off Off Off No bonus coins $
                        Off On  Off For every 4 coins, game logic adds 1 more coin
                        On  On  Off For every 4 coins, game logic adds 2 more coin
                        Off On  On  For every 5 coins, game logic adds 1 more coin
                        On  Off On**For every 3 coins, game logic adds 1 more coin

    $Manufacturer's suggested settings

    **In operator Information Display, this option displays same as no bonus.

    2008-07
    Dip locations added from the notes above

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "video/vector.h"
#include "video/avgdvg.h"
#include "machine/atari_vg.h"
#include "sound/pokey.h"

#define MASTER_CLOCK (12096000)
#define CLOCK_3KHZ  (MASTER_CLOCK / 4096)

#define IN_LEFT	(1 << 0)
#define IN_RIGHT (1 << 1)
#define IN_FIRE (1 << 2)
#define IN_SHIELD (1 << 3)
#define IN_THRUST (1 << 4)
#define IN_P1 (1 << 5)
#define IN_P2 (1 << 6)
#define OPTION_1_PLAYER_GAME_ONLY (1 << 2)
#define OPTION_2_CREDIT_MINIMUM (1 << 1)
#define OPTION_CHARGE_BY_ (1 << 0)


/*************************************
 *
 *  Input ports
 *
 *************************************/

/*

These 7 memory locations are used to read the 2 players' controls as well
as sharing some dipswitch info in the lower 4 bits pertaining to coins/credits
Typically, only the high 2 bits are read.

*/

static READ8_HANDLER( spacduel_IN3_r )
{
	int res;
	int res1;
	int res2;
	int res3;

	res1 = input_port_read(space->machine, "IN3");
	res2 = input_port_read(space->machine, "IN4");
	res3 = input_port_read_safe(space->machine, "DSW2", 0);
	res = 0x00;

	switch (offset & 0x07)
	{
		case 0:
			if (res1 & IN_SHIELD) res |= 0x80;
			if (res1 & IN_FIRE) res |= 0x40;
			break;
		case 1: /* Player 2 */
			if (res2 & IN_SHIELD) res |= 0x80;
			if (res2 & IN_FIRE) res |= 0x40;
			break;
		case 2:
			if (res1 & IN_LEFT) res |= 0x80;
			if (res1 & IN_RIGHT) res |= 0x40;
			break;
		case 3: /* Player 2 */
			if (res2 & IN_LEFT) res |= 0x80;
			if (res2 & IN_RIGHT) res |= 0x40;
			break;
		case 4:
			if (res1 & IN_THRUST) res |= 0x80;
			if (res1 & IN_P1) res |= 0x40;
			break;
		case 5:  /* Player 2 */
			if (res2 & IN_THRUST) res |= 0x80;
			if ((res3 & OPTION_CHARGE_BY_) == 0) res |= 0x40;
			break;
		case 6:
			if (res1 & IN_P2) res |= 0x80;
			if ((res3 & OPTION_2_CREDIT_MINIMUM) == 0) res |= 0x40;
			break;
		case 7:
			res = (0x00 /* upright */ | (0 & 0x40));
			if ((res3 & OPTION_1_PLAYER_GAME_ONLY) == 0) res |= 0x40;
			break;
	}

	return res;
}

static CUSTOM_INPUT( clock_r )
{
	return (field->port->machine->device<cpu_device>("maincpu")->total_cycles() & 0x100) ? 1 : 0;
}


/*************************************
 *
 *  Output ports
 *
 *************************************/

static WRITE8_HANDLER( bwidow_misc_w )
{
	/*
        0x10 = p1 led
        0x20 = p2 led
        0x01 = coin counter 1
        0x02 = coin counter 2
    */
	static int lastdata;

	if (data == lastdata) return;
	set_led_status(space->machine, 0,~data & 0x10);
	set_led_status(space->machine, 1,~data & 0x20);
	coin_counter_w(space->machine, 0, data & 0x01);
	coin_counter_w(space->machine, 1, data & 0x02);
	lastdata = data;
}

/*************************************
 *
 *  Interrupt ack
 *
 *************************************/

static WRITE8_HANDLER( irq_ack_w )
{
	cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( bwidow_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x2000, 0x27ff) AM_RAM AM_BASE(&vectorram) AM_SIZE(&vectorram_size) AM_REGION("maincpu", 0x2000)
	AM_RANGE(0x2800, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_DEVREADWRITE("pokey1", pokey_r, pokey_w)
	AM_RANGE(0x6800, 0x6fff) AM_DEVREADWRITE("pokey2", pokey_r, pokey_w)
	AM_RANGE(0x7000, 0x7000) AM_DEVREAD("earom", atari_vg_earom_r)
	AM_RANGE(0x7800, 0x7800) AM_READ_PORT("IN0")
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("IN3")
	AM_RANGE(0x8800, 0x8800) AM_READ_PORT("IN4")
	AM_RANGE(0x8800, 0x8800) AM_WRITE(bwidow_misc_w) /* coin counters, leds */
	AM_RANGE(0x8840, 0x8840) AM_WRITE(avgdvg_go_w)
	AM_RANGE(0x8880, 0x8880) AM_WRITE(avgdvg_reset_w)
	AM_RANGE(0x88c0, 0x88c0) AM_WRITE(irq_ack_w) /* interrupt acknowledge */
	AM_RANGE(0x8900, 0x8900) AM_DEVWRITE("earom", atari_vg_earom_ctrl_w)
	AM_RANGE(0x8940, 0x897f) AM_DEVWRITE("earom", atari_vg_earom_w)
	AM_RANGE(0x8980, 0x89ed) AM_WRITENOP /* watchdog clear */
	AM_RANGE(0x9000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( spacduel_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x0800, 0x0800) AM_READ_PORT("IN0")
	AM_RANGE(0x0900, 0x0907) AM_READ(spacduel_IN3_r)	/* IN1 */
	AM_RANGE(0x0905, 0x0906) AM_WRITENOP /* ignore? */
	AM_RANGE(0x0a00, 0x0a00) AM_DEVREAD("earom", atari_vg_earom_r)
//  AM_RANGE(0x0c00, 0x0c00) AM_WRITE(coin_counter_w) /* coin out */
	AM_RANGE(0x0c80, 0x0c80) AM_WRITE(avgdvg_go_w)
	AM_RANGE(0x0d00, 0x0d00) AM_WRITENOP /* watchdog clear */
	AM_RANGE(0x0d80, 0x0d80) AM_WRITE(avgdvg_reset_w)
	AM_RANGE(0x0e00, 0x0e00) AM_WRITE(irq_ack_w) /* interrupt acknowledge */
	AM_RANGE(0x0e80, 0x0e80) AM_DEVWRITE("earom", atari_vg_earom_ctrl_w)
	AM_RANGE(0x0f00, 0x0f3f) AM_DEVWRITE("earom", atari_vg_earom_w)
	AM_RANGE(0x1000, 0x100f) AM_DEVREADWRITE("pokey1", pokey_r, pokey_w)
	AM_RANGE(0x1400, 0x140f) AM_DEVREADWRITE("pokey2", pokey_r, pokey_w)
	AM_RANGE(0x2000, 0x27ff) AM_RAM AM_BASE(&vectorram) AM_SIZE(&vectorram_size) AM_REGION("maincpu", 0x2000)
	AM_RANGE(0x2800, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( bwidow )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )	// To fit "Coin B" Dip Switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )	// To fit "Coin A" Dip Switch
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Diagnostic Step") PORT_CODE(KEYCODE_F1)
	/* bit 6 is the VG HALT bit. We set it to "low" */
	/* per default (busy vector processor). */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(avgdvg_done_r, NULL)
	/* bit 7 is tied to a 3kHz clock */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(clock_r, NULL)

	PORT_START("DSW0")
	PORT_DIPNAME(0x03, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("D4:!7,!8")
	PORT_DIPSETTING (  0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (  0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (  0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (  0x02, DEF_STR( Free_Play ) )
	PORT_DIPNAME(0x0c, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("D4:!5,!6")
	PORT_DIPSETTING (  0x00, "*1" )
	PORT_DIPSETTING (  0x04, "*4" )
	PORT_DIPSETTING (  0x08, "*5" )
	PORT_DIPSETTING (  0x0c, "*6" )
	PORT_DIPNAME(0x10, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("D4:!4")
	PORT_DIPSETTING (  0x00, "*1" )
	PORT_DIPSETTING (  0x10, "*2" )
	PORT_DIPNAME(0xe0, 0x00, "Bonus Coins" ) PORT_DIPLOCATION("D4:!1,!2,!3")
	PORT_DIPSETTING (  0x80, "1 each 5" )
	PORT_DIPSETTING (  0x60, "2 each 4" )
	PORT_DIPSETTING (  0x40, "1 each 4" )
	PORT_DIPSETTING (  0xa0, "1 each 3" )
	PORT_DIPSETTING (  0x20, "1 each 2" )
	PORT_DIPSETTING (  0x00, DEF_STR( None ) )

	PORT_START("DSW1")
	PORT_DIPNAME(0x03, 0x01, "Max Start" ) PORT_DIPLOCATION("B4:!7,!8")
	PORT_DIPSETTING (  0x00, "Lev 13" )
	PORT_DIPSETTING (  0x01, "Lev 21" )
	PORT_DIPSETTING (  0x02, "Lev 37" )
	PORT_DIPSETTING (  0x03, "Lev 53" )
	PORT_DIPNAME(0x0c, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("B4:!5,!6")
	PORT_DIPSETTING (  0x00, "3" )
	PORT_DIPSETTING (  0x04, "4" )
	PORT_DIPSETTING (  0x08, "5" )
	PORT_DIPSETTING (  0x0c, "6" )
	PORT_DIPNAME(0x30, 0x10, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("B4:!3,!4")
	PORT_DIPSETTING (  0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING (  0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING (  0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING (  0x30, "Demo" )
	PORT_DIPNAME(0xc0, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("B4:!1,!2")
	PORT_DIPSETTING (  0x00, "20000" )
	PORT_DIPSETTING (  0x40, "30000" )
	PORT_DIPSETTING (  0x80, "40000" )
	PORT_DIPSETTING (  0xc0, DEF_STR( None ) )

	PORT_START("IN3")	/* IN3 - Movement joystick */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")	/* IN4 - Firing joystick */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( gravitar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )	// To fit "Coin B" Dip Switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )	// To fit "Coin A" Dip Switch
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Diagnostic Step") PORT_CODE(KEYCODE_F1)
	/* bit 6 is the VG HALT bit. We set it to "low" */
	/* per default (busy vector processor). */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(avgdvg_done_r, NULL)
	/* bit 7 is tied to a 3kHz clock */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(clock_r, NULL)

	PORT_START("DSW0")
	PORT_DIPUNUSED_DIPLOC( 0x03, IP_ACTIVE_HIGH, "D4:!7,!8" )
	PORT_DIPNAME(0x0c, 0x04, DEF_STR( Lives ) ) PORT_DIPLOCATION("D4:!5,!6")
	PORT_DIPSETTING (  0x00, "3" )
	PORT_DIPSETTING (  0x04, "4" )
	PORT_DIPSETTING (  0x08, "5" )
	PORT_DIPSETTING (  0x0c, "6" )
	PORT_DIPNAME(0x10, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("D4:!4")
	PORT_DIPSETTING (  0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING (  0x10, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_HIGH, "D4:!3" )
	PORT_DIPNAME(0xc0, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("D4:!1,!2")
	PORT_DIPSETTING (  0x00, "10000" )
	PORT_DIPSETTING (  0x40, "20000" )
	PORT_DIPSETTING (  0x80, "30000" )
	PORT_DIPSETTING (  0xc0, DEF_STR( None ) )

	PORT_START("DSW1")
	PORT_DIPNAME(0x03, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("B4:!7,!8")
	PORT_DIPSETTING (  0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (  0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (  0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (  0x02, DEF_STR( Free_Play ) )
	PORT_DIPNAME(0x0c, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("B4:!5,!6")
	PORT_DIPSETTING (  0x00, "*1" )
	PORT_DIPSETTING (  0x04, "*4" )
	PORT_DIPSETTING (  0x08, "*5" )
	PORT_DIPSETTING (  0x0c, "*6" )
	PORT_DIPNAME(0x10, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("B4:!4")
	PORT_DIPSETTING (  0x00, "*1" )
	PORT_DIPSETTING (  0x10, "*2" )
	PORT_DIPNAME(0xe0, 0x00, "Bonus Coins" ) PORT_DIPLOCATION("B4:!1,!2,!3")
	PORT_DIPSETTING (  0x80, "1 each 5" )
	PORT_DIPSETTING (  0x60, "2 each 4" )
	PORT_DIPSETTING (  0x40, "1 each 4" )
	PORT_DIPSETTING (  0xa0, "1 each 3" )
	PORT_DIPSETTING (  0x20, "1 each 2" )
	PORT_DIPSETTING (  0x00, DEF_STR( None ) )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( lunarbat )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )	// To be similar with other games
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )	// To be similar with other games
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* bit 6 is the VG HALT bit. We set it to "low" */
	/* per default (busy vector processor). */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(avgdvg_done_r, NULL)
	/* bit 7 is tied to a 3kHz clock */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(clock_r, NULL)

	PORT_START("DSW0")	/* DSW0 - Not read */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")	/* DSW1 - Not read */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN4")	/* IN4 - Not read */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( spacduel )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )	// To fit "Coin B" Dip Switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )	// To fit "Coin A" Dip Switch
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Diagnostic Step") PORT_CODE(KEYCODE_F1)
	/* bit 6 is the VG HALT bit. We set it to "low" */
	/* per default (busy vector processor). */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(avgdvg_done_r, NULL)
	/* bit 7 is tied to a 3kHz clock */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(clock_r, NULL)

	PORT_START("DSW0")
	PORT_DIPNAME(0x03, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("D4:!7,!8")
	PORT_DIPSETTING (  0x01, "3" )
	PORT_DIPSETTING (  0x00, "4" )
	PORT_DIPSETTING (  0x03, "5" )
	PORT_DIPSETTING (  0x02, "6" )
	PORT_DIPNAME(0x0c, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("D4:!5,!6")
	PORT_DIPSETTING (  0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING (  0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING (  0x0c, DEF_STR( Medium ) )
	PORT_DIPSETTING (  0x08, DEF_STR( Hard ) )
	PORT_DIPNAME(0x30, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("D4:!3,!4")
	PORT_DIPSETTING (  0x00, DEF_STR( English ) )
	PORT_DIPSETTING (  0x10, DEF_STR( German ) )
	PORT_DIPSETTING (  0x20, DEF_STR( French ) )
	PORT_DIPSETTING (  0x30, DEF_STR( Spanish ) )
	PORT_DIPNAME(0xc0, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("D4:!1,!2")
	PORT_DIPSETTING (  0xc0, "8000" )
	PORT_DIPSETTING (  0x00, "10000" )
	PORT_DIPSETTING (  0x40, "15000" )
	PORT_DIPSETTING (  0x80, DEF_STR( None ) )

	PORT_START("DSW1")
	PORT_DIPNAME(0x03, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("B4:!7,!8")
	PORT_DIPSETTING (  0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (  0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (  0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (  0x02, DEF_STR( Free_Play ) )
	PORT_DIPNAME(0x0c, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("B4:!5,!6")
	PORT_DIPSETTING (  0x00, "*1" )
	PORT_DIPSETTING (  0x04, "*4" )
	PORT_DIPSETTING (  0x08, "*5" )
	PORT_DIPSETTING (  0x0c, "*6" )
	PORT_DIPNAME(0x10, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("B4:!4")
	PORT_DIPSETTING (  0x00, "*1" )
	PORT_DIPSETTING (  0x10, "*2" )
	PORT_DIPNAME(0xe0, 0x00, "Bonus Coins" ) PORT_DIPLOCATION("B4:!1,!2,!3")
	PORT_DIPSETTING (  0x80, "1 each 5" )
	PORT_DIPSETTING (  0x60, "2 each 4" )
	PORT_DIPSETTING (  0x40, "1 each 4" )
	PORT_DIPSETTING (  0xa0, "1 each 3" )
	PORT_DIPSETTING (  0x20, "1 each 2" )
	PORT_DIPSETTING (  0x00, DEF_STR( None ) )

	PORT_START("DSW2")
	// Although a dip switch 1 setting is shown in the Space Duel - Operation, Maintenance, and Service Manual,
	// this switch is not connected to anything on the PCB or in the schematics
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "P10/11:!1")
	PORT_DIPNAME( 0x04, 0x04, "1-player game only" ) PORT_DIPLOCATION("P10/11:!2")
	PORT_DIPSETTING (  0x04, DEF_STR( Off ) )
	PORT_DIPSETTING (  0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "2-credit minimum" ) PORT_DIPLOCATION("P10/11:!3")
	PORT_DIPSETTING (  0x02, DEF_STR( Off ) )
	PORT_DIPSETTING (  0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "Charge by ..." ) PORT_DIPLOCATION("P10/11:!4")
	PORT_DIPSETTING (  0x01, "player" )
	PORT_DIPSETTING (  0x00, "game" )

	/* See machine/spacduel.c for more info on these 2 ports */
	PORT_START("IN3")	/* IN3 - Player 1 - spread over 8 memory locations */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Select")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN4")	/* IN4 - Player 2 - spread over 8 memory locations */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const pokey_interface pokey_interface_1 =
{
	{ DEVCB_NULL },
	DEVCB_INPUT_PORT("DSW0")
};


static const pokey_interface pokey_interface_2 =
{
	{ DEVCB_NULL },
	DEVCB_INPUT_PORT("DSW1")
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( bwidow )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6502, MASTER_CLOCK / 8)
	MDRV_CPU_PROGRAM_MAP(bwidow_map)
	MDRV_CPU_PERIODIC_INT(irq0_line_assert, (double)MASTER_CLOCK / 4096 / 12)

	MDRV_ATARIVGEAROM_ADD("earom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", VECTOR)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_SIZE(400, 300)
	MDRV_SCREEN_VISIBLE_AREA(0, 480, 0, 440)

	MDRV_VIDEO_START(avg)
	MDRV_VIDEO_UPDATE(vector)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("pokey1", POKEY, MASTER_CLOCK / 8)
	MDRV_SOUND_CONFIG(pokey_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("pokey2", POKEY, MASTER_CLOCK / 8)
	MDRV_SOUND_CONFIG(pokey_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( gravitar )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(bwidow)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(0, 420, 0, 400)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( lunarbat )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(gravitar)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(spacduel_map)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_REFRESH_RATE(45)
	MDRV_SCREEN_VISIBLE_AREA(0, 500, 0, 440)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( spacduel )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(gravitar)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(spacduel_map)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_REFRESH_RATE(45)
	MDRV_SCREEN_VISIBLE_AREA(0, 540, 0, 400)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( bwidow )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* Vector ROM */
	ROM_LOAD( "136017.107",   0x2800, 0x0800, CRC(97f6000c) SHA1(bbae93058228820ee67b05f23e45fb54ee0963ff) )
	ROM_LOAD( "136017.108",   0x3000, 0x1000, CRC(3da354ed) SHA1(935295d66ad40ad702eb7a694296e836f53d22ec) )
	ROM_LOAD( "136017.109",   0x4000, 0x1000, CRC(2fc4ce79) SHA1(2b324877bf55151747eaacd9a58f846712bfbc14) )
	ROM_LOAD( "136017.110",   0x5000, 0x1000, CRC(0dd52987) SHA1(72aa1d24f20cc86701189df486488edc434b1be1) )
	/* Program ROM */
	ROM_LOAD( "136017.101",   0x9000, 0x1000, CRC(fe3febb7) SHA1(b62f7622ca60248e1b8376ee135ae3d94d0b4437) )
	ROM_LOAD( "136017.102",   0xa000, 0x1000, CRC(10ad0376) SHA1(614c74daa468a7430ed965a3a9d07b6ad846016c) )
	ROM_LOAD( "136017.103",   0xb000, 0x1000, CRC(8a1430ee) SHA1(3aa6c40721a4289c1cf01f37c89b6b0a96336c68) )
	ROM_LOAD( "136017.104",   0xc000, 0x1000, CRC(44f9943f) SHA1(e83d8242e4592149719be6a68cf3aba46116072f) )
	ROM_LOAD( "136017.105",   0xd000, 0x1000, CRC(1fdf801c) SHA1(33da2ba3cefa3d0dddc8647f9b6caf5d5bfe9b3b) )
	ROM_LOAD( "136017.106",   0xe000, 0x1000, CRC(ccc9b26c) SHA1(f1398e3ff2b62af1509bc117028845b671ff1ca2) )
	ROM_RELOAD(               0xf000, 0x1000 )	/* for reset/interrupt vectors */
	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.n4",	 0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )
ROM_END

ROM_START( gravitar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* Vector ROM */
	ROM_LOAD( "136010.210",   0x2800, 0x0800, CRC(debcb243) SHA1(2c50cd38d60739c126f1d0d8e7fbd46a0bde6e1c) )
	ROM_LOAD( "136010.207",   0x3000, 0x1000, CRC(4135629a) SHA1(301ddb7a34b38140a1fdffc060cb08ff57f10cf1) )
	ROM_LOAD( "136010.208",   0x4000, 0x1000, CRC(358f25d9) SHA1(9c2920cf2b73a93ac2808be654b08505037f53b1) )
	ROM_LOAD( "136010.309",   0x5000, 0x1000, CRC(4ac78df4) SHA1(5164f2a54244ce1e863d1ec0dd29bc9da7103a85) )
	/* Program ROM */
	ROM_LOAD( "136010.301",   0x9000, 0x1000, CRC(a2a55013) SHA1(800b52ead9f56a3e372797fbc698c8fc791398da) )
	ROM_LOAD( "136010.302",   0xa000, 0x1000, CRC(d3700b3c) SHA1(b9e846db14fa23f8d2def97030d8b072b2bbc0be) )
	ROM_LOAD( "136010.303",   0xb000, 0x1000, CRC(8e12e3e0) SHA1(e09f58f6f36de6bf6724a1ab14ab35acbb0b3876) )
	ROM_LOAD( "136010.304",   0xc000, 0x1000, CRC(467ad5da) SHA1(822b06be6f2d6298b2b10161fbabbb2caa74b2ef) )
	ROM_LOAD( "136010.305",   0xd000, 0x1000, CRC(840603af) SHA1(4a7124f91d3ee940686c51374a861efe6cb5d282) )
	ROM_LOAD( "136010.306",   0xe000, 0x1000, CRC(3f3805ad) SHA1(baf080deaa8eea43af2f3be71dacc63e4666c453) )
	ROM_RELOAD(              0xf000, 0x1000 )	/* for reset/interrupt vectors */
	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.n4",	 0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )
ROM_END

ROM_START( gravitar2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* Vector ROM */
	ROM_LOAD( "136010.210",   0x2800, 0x0800, CRC(debcb243) SHA1(2c50cd38d60739c126f1d0d8e7fbd46a0bde6e1c) )
	ROM_LOAD( "136010.207",   0x3000, 0x1000, CRC(4135629a) SHA1(301ddb7a34b38140a1fdffc060cb08ff57f10cf1) )
	ROM_LOAD( "136010.208",   0x4000, 0x1000, CRC(358f25d9) SHA1(9c2920cf2b73a93ac2808be654b08505037f53b1) )
	ROM_LOAD( "136010.209",   0x5000, 0x1000, CRC(37034287) SHA1(4de7478fb566fd75f99533507228611cecb1f11a) )
	/* Program ROM */
	ROM_LOAD( "136010.201",   0x9000, 0x1000, CRC(167315e4) SHA1(35613e5a503fac7f451c201675669f417e15241b) )
	ROM_LOAD( "136010.202",   0xa000, 0x1000, CRC(aaa9e62c) SHA1(87fc660adb22f812a764efc46ffcf5f934d5e333) )
	ROM_LOAD( "136010.203",   0xb000, 0x1000, CRC(ae437253) SHA1(e2402dc5fa755a05fa1d531a31b78e39e67f5cbe) )
	ROM_LOAD( "136010.204",   0xc000, 0x1000, CRC(5d6bc29e) SHA1(fdd442644209ab858eb4ed1b4cdeb1db26f80108) )
	ROM_LOAD( "136010.205",   0xd000, 0x1000, CRC(0db1ff34) SHA1(288d9ffff9d18025621be249ea25a7444f58f3a9) )
	ROM_LOAD( "136010.206",   0xe000, 0x1000, CRC(4521ca48) SHA1(5770cb46c4ac28d632ad5910723a9edda8283ce5) )
	ROM_RELOAD(              0xf000, 0x1000 )	/* for reset/interrupt vectors */
	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.n4",	 0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )
ROM_END

ROM_START( gravp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* Vector ROM */
	ROM_LOAD( "l7.bin",   0x2800, 0x0800, CRC(1da0d845) SHA1(99bccae0521c105388784175c475035bf19270a7) )
	ROM_LOAD( "mn7.bin",  0x3000, 0x1000, CRC(650ba31e) SHA1(7f855ea13e2041a87b64fdff4b7ee0d7d97e4401) )
	ROM_LOAD( "np7.bin",  0x4000, 0x1000, CRC(5119c0b2) SHA1(ec8a7072d2b8e2626a19c0451ea5ddb27ad80594) )
	ROM_LOAD( "r7.bin",   0x5000, 0x1000, CRC(defa8cbc) SHA1(ffd618d846c219fb641311f1d95ffc9f6fb5a240) )
	/* Program ROM */
	ROM_LOAD( "d1.bin",   0x9000, 0x1000, CRC(acbc0e2c) SHA1(2e0ff3b7ac9c0813e71942492146372bba382f1f) )
	ROM_LOAD( "ef1.bin",  0xa000, 0x1000, CRC(88f98f8f) SHA1(c2174deed61ae8519a02c2ac8e2969f357733cfd) )
	ROM_LOAD( "h1.bin",   0xb000, 0x1000, CRC(68a85703) SHA1(8a7956578cb6ebbeb74facedcbcb46f86ec92000) )
	ROM_LOAD( "j1.bin",   0xc000, 0x1000, CRC(33d19ef6) SHA1(68f95e237427959d6ef64a5b4dd1e03db7389271) )
	ROM_LOAD( "kl1.bin",  0xd000, 0x1000, CRC(032b5806) SHA1(b719792a177e74ec49e6952e445b9cdeaca7505f) )
	ROM_LOAD( "m1.bin",   0xe000, 0x1000, CRC(47fe97a0) SHA1(7cbde4b59abde679c28d7547700b342f25762e4a) )
	ROM_RELOAD(           0xf000, 0x1000 )	/* for reset/interrupt vectors */
	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.n4",	 0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )
ROM_END

ROM_START( lunarbat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* Vector ROM */
	ROM_LOAD( "010.010",      0x2800, 0x0800, CRC(48fd38aa) SHA1(e6ec31e784c2965369161c33d00903ba027f7f20) )
	ROM_LOAD( "007.010",      0x3000, 0x1000, CRC(9754830e) SHA1(2e6885155a93d4eaf9a405f3eb740f2f4b30bc23) )
	ROM_LOAD( "008.010",      0x4000, 0x1000, CRC(084aa8db) SHA1(80050f981b9a673d336bbcf712faf21b7be7e042) )
	/* Program ROM */
	ROM_LOAD( "001.010",      0x9000, 0x1000, CRC(cd7e1780) SHA1(92265a548485d140b73ef542ad66dc32cb52d42b) )
	ROM_LOAD( "002.010",      0xa000, 0x1000, CRC(dc813a54) SHA1(c543cae3a3ba5b00e5a8714a42b2557bc6e730cf) )
	ROM_LOAD( "003.010",      0xb000, 0x1000, CRC(8e1fecd3) SHA1(a43cb4ea77e095227590fcefa778688093dcf135) )
	ROM_LOAD( "004.010",      0xc000, 0x1000, CRC(c407764f) SHA1(f202a9fe6c10975bb124b4b1e902341da578da8f) )
	ROM_LOAD( "005.010",      0xd000, 0x1000, CRC(4feb6f81) SHA1(b852f1093e56343225c1b2b2554a93c88fc58637) )
	ROM_LOAD( "006.010",      0xe000, 0x1000, CRC(f8ad139d) SHA1(e9e0dcb0872b19af09825a979f8b3747c9632091) )
	ROM_RELOAD(               0xf000, 0x1000 )	/* for reset/interrupt vectors */
	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.n4",	 0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )
ROM_END

ROM_START( lunarba1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* Vector ROM */
	ROM_LOAD( "vrom1.bin",   0x2800, 0x0800, CRC(c60634d9) SHA1(b94f056b5e73a2e015ba9a4be66dc2abee325016) )
	ROM_LOAD( "vrom2.bin",   0x3000, 0x1000, CRC(53d9a8a2) SHA1(c33766658dd3523e99e664ef42a4ba4ab884fa80) )
	/* Program ROM */
	ROM_LOAD( "rom0.bin",    0x4000, 0x1000, CRC(cc4691c6) SHA1(72f75c75ec3f36c5c455c82593659961b5882f9f) )
	ROM_LOAD( "rom1.bin",    0x5000, 0x1000, CRC(4df71d07) SHA1(5d71750594885641fb347bf4106b3b6ace822fb9) )
	ROM_LOAD( "rom2.bin",    0x6000, 0x1000, CRC(c6ff04cb) SHA1(33477abacb9dfeadca20f9b9c1c2840cf31be7c3) )
	ROM_LOAD( "rom3.bin",    0x7000, 0x1000, CRC(a7dc9d1b) SHA1(991f5b943f5c82027deadda0c4230b70f2a8ca10) )
	ROM_LOAD( "rom4.bin",    0x8000, 0x1000, CRC(788bf976) SHA1(4dc2d92bbd232625fd8e828c876c4182ddde125d) )
	ROM_LOAD( "rom5.bin",    0x9000, 0x1000, CRC(16121e13) SHA1(9ef59f4ffc22d5f9457b57d2f3a67d883995be98) )
	ROM_RELOAD(              0xa000, 0x1000 )
	ROM_RELOAD(              0xb000, 0x1000 )
	ROM_RELOAD(              0xc000, 0x1000 )
	ROM_RELOAD(              0xd000, 0x1000 )
	ROM_RELOAD(              0xe000, 0x1000 )
	ROM_RELOAD(              0xf000, 0x1000 )	/* for reset/interrupt vectors */
	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.n4",	 0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )
ROM_END

ROM_START( spacduel )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* Vector ROM */
	ROM_LOAD( "136006.106",   0x2800, 0x0800, CRC(691122fe) SHA1(f53be76a49dba319050ca7767de3441521910e83) )
	ROM_LOAD( "136006.107",   0x3000, 0x1000, CRC(d8dd0461) SHA1(58060b20b2511d30d2ec06479d21840bdd0b53c6) )
	/* Program ROM */
	ROM_LOAD( "136006.201",   0x4000, 0x1000, CRC(f4037b6e) SHA1(9bacb64d257edd31f53db878477604f50681d78f) )
	ROM_LOAD( "136006.102",   0x5000, 0x1000, CRC(4c451e8a) SHA1(c05c52bb08acccb60950a15f05c960c3bc163d3e) )
	ROM_LOAD( "136006.103",   0x6000, 0x1000, CRC(ee72da63) SHA1(d36d62cdf7fe76ee9cdbfc2e76ac5d90f22986ba) )
	ROM_LOAD( "136006.104",   0x7000, 0x1000, CRC(e41b38a3) SHA1(9e8773e78d65d74db824cfd7108e7038f26757db) )
	ROM_LOAD( "136006.105",   0x8000, 0x1000, CRC(5652710f) SHA1(b15891d22a47ac3448d2ced40c04d0ab80606c7d) )
	ROM_RELOAD(              0x9000, 0x1000 )
	ROM_RELOAD(              0xa000, 0x1000 )
	ROM_RELOAD(              0xb000, 0x1000 )
	ROM_RELOAD(              0xc000, 0x1000 )
	ROM_RELOAD(              0xd000, 0x1000 )
	ROM_RELOAD(              0xe000, 0x1000 )

	ROM_RELOAD(              0xf000, 0x1000 )	/* for reset/interrupt vectors */
	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.n4",	 0x0000, 0x0100, CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1980, spacduel, 0,        spacduel, spacduel, 0, ROT0, "Atari", "Space Duel", GAME_SUPPORTS_SAVE )
GAME( 1982, bwidow,   0,        bwidow,   bwidow,   0, ROT0, "Atari", "Black Widow", GAME_SUPPORTS_SAVE )
GAME( 1982, gravitar, 0,        gravitar, gravitar, 0, ROT0, "Atari", "Gravitar (version 3)", GAME_SUPPORTS_SAVE )
GAME( 1982, gravitar2,gravitar, gravitar, gravitar, 0, ROT0, "Atari", "Gravitar (version 2)", GAME_SUPPORTS_SAVE )
GAME( 1982, gravp,    gravitar, gravitar, gravitar, 0, ROT0, "Atari", "Gravitar (prototype)", GAME_SUPPORTS_SAVE )
GAME( 1982, lunarbat, gravitar, gravitar, gravitar, 0, ROT0, "Atari", "Lunar Battle (prototype, later)", GAME_SUPPORTS_SAVE )
GAME( 1982, lunarba1, gravitar, lunarbat, lunarbat, 0, ROT0, "Atari", "Lunar Battle (prototype, earlier)", GAME_SUPPORTS_SAVE )
