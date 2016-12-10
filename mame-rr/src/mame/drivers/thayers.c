/*

    TODO:

    - LDV1000 mode
    - PR7820 INT/_EXT line
    - coin counter
    - convert SSI-263 to a sound device
    - dump laserdisc

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "render.h"
#include "machine/laserdsc.h"
#include "cpu/cop400/cop400.h"
//#include "dlair.lh"

extern const char layout_dlair[];

static running_device *laserdisc;
static UINT8 laserdisc_data;

static int rx_bit;
static int keylatch;

static UINT8 cop_data_latch;
static int cop_data_latch_enable;
static UINT8 cop_l;
static UINT8 cop_cmd_latch;

static int timer_int;
static int data_rdy_int;
static int ssi_data_request;

static int cart_present;
static int pr7820_enter;

static const UINT8 led_map[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x00 };

/* Interrupts */

static void check_interrupt(running_machine *machine)
{
	if (!timer_int || !data_rdy_int || !ssi_data_request)
	{
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_IRQ0, HOLD_LINE);
	}
	else
	{
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}

static TIMER_CALLBACK( intrq_tick )
{
	cputag_set_input_line(machine, "maincpu", INPUT_LINE_IRQ0, CLEAR_LINE);
}

static WRITE8_HANDLER( intrq_w )
{
	// T = 1.1 * R30 * C53 = 1.1 * 750K * 0.01uF = 8.25 ms

	cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_IRQ0, HOLD_LINE);

	timer_set(space->machine, ATTOTIME_IN_USEC(8250), NULL, 0, intrq_tick);
}

static READ8_HANDLER( irqstate_r )
{
	/*

        bit     description

        0
        1
        2       SSI263 A/_R
        3       tied to +5V
        4       _TIMER INT
        5       _DATA RDY INT
        6       _CART PRES
        7

    */

	return (data_rdy_int << 5) | (timer_int << 4) | 0x08 | (ssi_data_request << 2);
}

static WRITE8_HANDLER( timer_int_ack_w )
{
	timer_int = 1;

	check_interrupt(space->machine);
}

static WRITE8_HANDLER( data_rdy_int_ack_w )
{
	data_rdy_int = 1;

	check_interrupt(space->machine);
}

static WRITE8_HANDLER( cop_d_w )
{
	/*

        bit     description

        D0      _TIMER INT
        D1      _DATA RDY INT
        D2
        D3

    */

	if (!BIT(data, 0))
	{
		timer_int = 0;
	}

	if (!BIT(data, 1))
	{
		data_rdy_int = 0;
	}

	check_interrupt(space->machine);
}

/* COP Communication */

static READ8_HANDLER( cop_data_r )
{
	if (!cop_data_latch_enable)
	{
		return cop_data_latch;
	}
	else
	{
		return cop_l;
	}
}

static WRITE8_HANDLER( cop_data_w )
{
	cop_data_latch = data;
}

static READ8_HANDLER( cop_l_r )
{
	if (!cop_data_latch_enable)
	{
		return cop_data_latch;
	}
	else
	{
		return 0;
	}
}

static WRITE8_HANDLER( cop_l_w )
{
	cop_l = data;
}

static READ8_HANDLER( cop_g_r )
{
	/*

        bit     description

        G0      U16 Q0
        G1      U16 Q1
        G2      U16 Q2
        G3

    */

	return cop_cmd_latch;
}

static WRITE8_HANDLER( control_w )
{
	/*

        bit     description

        0
        1       _CS128A
        2       _BANKSEL1
        3
        4
        5       COP G0
        6       COP G1
        7       COP G2

    */

	cop_cmd_latch = (data >> 5) & 0x07;
}

static WRITE8_HANDLER( cop_g_w )
{
	/*

        bit     description

        G0
        G1
        G2
        G3      U17 enable

    */

	cop_data_latch_enable = BIT(data, 3);
}

/* Keyboard */

static READ8_HANDLER(cop_si_r)
{
	/* keyboard data */

	/*

        Serial communications format

        1, 1, 0, 1, Q8, P0, P1, P2, P3, 0

    */

	switch (rx_bit)
	{
	case 0:
	case 1:
	case 3:
		return 1;

	case 4:
		return (keylatch == 9);

	case 5:
	case 6:
	case 7:
	case 8:
		{
			UINT8 data;
			char port[4];

			sprintf(port, "R%d", keylatch);

			data = BIT(input_port_read(space->machine, port), rx_bit - 5);

			return data;
		}

	default:
		return 0;
	}
}

static WRITE8_HANDLER( cop_so_w )
{
	/* keyboard clock */

	if (data)
	{
		rx_bit++;

		if (rx_bit == 10)
		{
			rx_bit = 0;

			keylatch++;

			if (keylatch == 10)
			{
				keylatch = 0;
			}
		}
	}
}

/* I/O Board */

static WRITE8_HANDLER( control2_w )
{
	/*

        bit     description

        0
        1       _RESOI (?)
        2       _ENCARTDET
        3
        4
        5
        6
        7

    */

	if ((!BIT(data, 2)) & cart_present)
	{
		cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_NMI, HOLD_LINE);
	}
}

static READ8_HANDLER( dsw_b_r )
{
	return (input_port_read(space->machine, "COIN") & 0xf0) | (input_port_read(space->machine, "DSWB") & 0x0f);
}

static READ8_HANDLER( laserdsc_data_r )
{
	return laserdisc_data_r(laserdisc);
}

static WRITE8_HANDLER( laserdsc_data_w )
{
	laserdisc_data = data;
}

static WRITE8_HANDLER( laserdsc_control_w )
{
	/*

        bit     description

        0
        1
        2
        3
        4       coin counter
        5       U16 output enable
        6       ENTER if switch B5 closed
        7       INT/_EXT

    */

	coin_counter_w(space->machine, 0, BIT(data, 4));

	if (BIT(data, 5))
	{
		laserdisc_data_w(laserdisc, laserdisc_data);
	}

	switch (laserdisc_get_type(laserdisc))
	{
		case LASERDISC_TYPE_PIONEER_PR7820:
			pr7820_enter = BIT(data, 6) ? CLEAR_LINE : ASSERT_LINE;

			laserdisc_line_w(laserdisc, LASERDISC_LINE_ENTER, pr7820_enter);

			// BIT(data, 7) is INT/_EXT, but there is no such input line in laserdsc.h
			break;

		case LASERDISC_TYPE_PIONEER_LDV1000:
			laserdisc_line_w(laserdisc, LASERDISC_LINE_ENTER, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE);
			break;
	}
}

static WRITE8_HANDLER( den1_w )
{
	/*

        bit     description

        0       DD0
        1       DD1
        2       DD2
        3       DD3
        4       DA0
        5       DA1
        6       DA2
        7       DA3

    */

	output_set_digit_value(data >> 4, led_map[data & 0x0f]);
}

static WRITE8_HANDLER( den2_w )
{
	/*

        bit     description

        0       DD0
        1       DD1
        2       DD2
        3       DD3
        4       DA0
        5       DA1
        6       DA2
        7       DA3

    */

	output_set_digit_value(8 + (data >> 4), led_map[data & 0x0f]);
}

/* SSI-263 */

/*

    The following information is from the SSI-263A data sheet.

    Thayer's Quest uses an SSI-263, so this might be inaccurate, but it works for now

*/

#define SSI263_CLOCK (XTAL_4MHz/2)

static const char SSI263_PHONEMES[0x40][5] =
{
	"PA", "E", "E1", "Y", "YI", "AY", "IE", "I", "A", "AI", "EH", "EH1", "AE", "AE1", "AH", "AH1", "W", "O", "OU", "OO", "IU", "IU1", "U", "U1", "UH", "UH1", "UH2", "UH3", "ER", "R", "R1", "R2",
	"L", "L1", "LF", "W", "B", "D", "KV", "P", "T", "K", "HV", "HVC", "HF", "HFC", "HN", "Z", "S", "J", "SCH", "V", "F", "THV", "TH", "M", "N", "NG", ":A", ":OH", ":U", ":UH", "E2", "LB"
};

static struct SSI263
{
	UINT8 dr;
	UINT8 p;
	UINT16 i;
	UINT8 r;
	UINT8 t;
	UINT8 c;
	UINT8 a;
	UINT8 f;
	UINT8 mode;
} ssi263;

static TIMER_CALLBACK( ssi263_phoneme_tick )
{
	ssi_data_request = 0;
	check_interrupt(machine);
}

static WRITE8_HANDLER( ssi263_register_w )
{
	switch (offset)
	{
	case 0:
		{
		int frame_time = ((4096 * (16 - ssi263.r)) / 2); // us, /2 should actually be /SSI263_CLOCK, but this way we get microseconds directly
		int phoneme_time = frame_time * (4 - ssi263.dr); // us

		// duration/phoneme register
		ssi263.dr = (data >> 5) & 0x03;
		ssi263.p = data & 0x3f;

		ssi_data_request = 1;
		check_interrupt(space->machine);

		switch (ssi263.mode)
		{
		case 0:
		case 1:
			// phoneme timing response
			timer_set(space->machine, ATTOTIME_IN_USEC(phoneme_time), NULL, 0, ssi263_phoneme_tick);
			break;
		case 2:
			// frame timing response
			timer_set(space->machine, ATTOTIME_IN_USEC(frame_time), NULL, 0, ssi263_phoneme_tick);
			break;
		case 3:
			// disable A/_R output
			break;
		}

		//logerror("SSI263 Phoneme Duration: %u\n", ssi263.dr);
		//logerror("SSI263 Phoneme: %02x %s\n", ssi263.p, SSI263_PHONEMES[ssi263.p]);
		}
		break;

	case 1:
		// inflection register
		ssi263.i = (data << 3) | (ssi263.i & 0x403);

		//logerror("SSI263 Inflection: %u\n", ssi263.i);
		break;

	case 2:
		// rate/inflection register
		ssi263.i = (BIT(data, 4) << 11) | (ssi263.i & 0x7f8) | (data & 0x07);
		ssi263.r = data >> 4;

		//logerror("SSI263 Inflection: %u\n", ssi263.i);
		//logerror("SSI263 Rate: %u\n", ssi263.r);
		break;

	case 3:
		// control/articulation/amplitude register
		if (ssi263.c && !BIT(data, 7))
		{
			ssi263.mode = ssi263.dr;

			switch (ssi263.mode)
			{
			case 0:
				//logerror("SSI263 Phoneme Timing Response, Transitioned Inflection\n");
				break;

			case 1:
				//logerror("SSI263 Phoneme Timing Response, Immediate Inflection\n");
				break;

			case 2:
				// activate A/_R
				//logerror("SSI263 Frame Timing Response, Immediate Inflection\n");
				break;

			case 3:
				// disable A/_R output
				//logerror("SSI263 A/R Output Disabled\n");
				break;
			}
		}

		ssi263.c = BIT(data, 7);
		ssi263.t = (data >> 4) & 0x07;
		ssi263.a = data & 0x0f;

		//logerror("SSI263 Control: %u\n", ssi263.c);
		//logerror("SSI263 Articulation: %u\n", ssi263.t);
		//logerror("SSI263 Amplitude: %u\n", ssi263.a);
		break;

	case 4:
	case 5:
	case 6:
	case 7:
		// filter frequency register
		ssi263.f = data;
		//logerror("SSI263 Filter Frequency: %u\n", ssi263.f);
		break;
	}
}

static READ8_HANDLER( ssi263_register_r )
{
	// D7 becomes an output, as the inverted state of A/_R. The register address bits are ignored.

	return !ssi_data_request << 7;
}

/* Memory Maps */

static ADDRESS_MAP_START( thayers_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xdfff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( thayers_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x07) AM_READWRITE(ssi263_register_r, ssi263_register_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(control_w)
	AM_RANGE(0x40, 0x40) AM_READWRITE(irqstate_r, control2_w)
	AM_RANGE(0x80, 0x80) AM_READWRITE(cop_data_r, cop_data_w)
	AM_RANGE(0xa0, 0xa0) AM_WRITE(timer_int_ack_w)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(data_rdy_int_ack_w)
	AM_RANGE(0xf0, 0xf0) AM_READ(laserdsc_data_r)
	AM_RANGE(0xf1, 0xf1) AM_READ(dsw_b_r)
	AM_RANGE(0xf2, 0xf2) AM_READ_PORT("DSWA")
	AM_RANGE(0xf3, 0xf3) AM_WRITE(intrq_w)
	AM_RANGE(0xf4, 0xf4) AM_WRITE(laserdsc_data_w)
	AM_RANGE(0xf5, 0xf5) AM_WRITE(laserdsc_control_w)
	AM_RANGE(0xf6, 0xf6) AM_WRITE(den1_w)
	AM_RANGE(0xf7, 0xf7) AM_WRITE(den2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( thayers_cop_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(COP400_PORT_L, COP400_PORT_L) AM_READWRITE(cop_l_r, cop_l_w)
	AM_RANGE(COP400_PORT_G, COP400_PORT_G) AM_READWRITE(cop_g_r, cop_g_w)
	AM_RANGE(COP400_PORT_D, COP400_PORT_D) AM_WRITE(cop_d_w)
	AM_RANGE(COP400_PORT_SK, COP400_PORT_SK) AM_WRITENOP
	AM_RANGE(COP400_PORT_SIO, COP400_PORT_SIO) AM_READ(cop_si_r) AM_WRITE(cop_so_w)
ADDRESS_MAP_END

/* Input Ports */

static CUSTOM_INPUT( laserdisc_enter_r )
{
	switch (laserdisc_get_type(laserdisc))
	{
		case LASERDISC_TYPE_PIONEER_PR7820:
			return pr7820_enter;

		case LASERDISC_TYPE_PIONEER_LDV1000:
			return (laserdisc_line_r(laserdisc, LASERDISC_LINE_STATUS) == ASSERT_LINE) ? 0 : 1;
	}

	return 0;
}

static CUSTOM_INPUT( laserdisc_ready_r )
{
	switch (laserdisc_get_type(laserdisc))
	{
		case LASERDISC_TYPE_PIONEER_PR7820:
			return (laserdisc_line_r(laserdisc, LASERDISC_LINE_READY) == ASSERT_LINE) ? 0 : 1;

		case LASERDISC_TYPE_PIONEER_LDV1000:
			return (laserdisc_line_r(laserdisc, LASERDISC_LINE_COMMAND) == ASSERT_LINE) ? 0 : 1;
	}

	return 0;
}

static INPUT_PORTS_START( thayers )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Time Per Coin" ) PORT_DIPLOCATION( "A:3,2,1" )
	PORT_DIPSETTING(    0x07, "110 Seconds" )
	PORT_DIPSETTING(    0x06, "95 Seconds" )
	PORT_DIPSETTING(    0x05, "80 Seconds" )
	PORT_DIPSETTING(    0x04, "70 Seconds" )
	PORT_DIPSETTING(    0x03, "60 Seconds" )
	PORT_DIPSETTING(    0x02, "45 Seconds" )
	PORT_DIPSETTING(    0x01, "30 Seconds" )
	PORT_DIPSETTING(    0x00, DEF_STR ( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) ) PORT_DIPLOCATION( "A:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) ) PORT_DIPLOCATION( "A:5" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION( "A:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Attract Mode Audio" ) PORT_DIPLOCATION( "A:7" )
	PORT_DIPSETTING(    0x40, "Always Playing" )
	PORT_DIPSETTING(    0x00, "One Out of 8 Times" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "A:8" )

	PORT_START("DSWB")
	PORT_SERVICE_DIPLOC( 0x01, 0x01, "B:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "B:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "B:3" )
	PORT_DIPNAME( 0x18, 0x00, "LD Player" ) PORT_DIPLOCATION( "B:5,4" )
	PORT_DIPSETTING(    0x18, "LDV-1000" )
	PORT_DIPSETTING(    0x00, "PR-7820" )
	PORT_DIPUNUSED_DIPLOC( 0xe0, IP_ACTIVE_LOW, "B:8,7,6" )

	PORT_START("COIN")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(laserdisc_enter_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(laserdisc_ready_r, NULL)

	PORT_START("R0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "2" ) PORT_CODE( KEYCODE_F2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "1 - Clear" ) PORT_CODE( KEYCODE_BACKSPACE )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Q" ) PORT_CODE( KEYCODE_Q )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( DEF_STR( Yes ) ) PORT_CODE( KEYCODE_0_PAD )

	PORT_START("R1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Z - Spell of Release" ) PORT_CODE( KEYCODE_Z )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "A" ) PORT_CODE( KEYCODE_A )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "W - Amulet" ) PORT_CODE( KEYCODE_W )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Items" ) PORT_CODE( KEYCODE_1_PAD )

	PORT_START("R2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "X - Scepter" ) PORT_CODE( KEYCODE_X )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "S - Dagger" ) PORT_CODE( KEYCODE_S )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "E - Black Mace" ) PORT_CODE( KEYCODE_E )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Drop Item" ) PORT_CODE( KEYCODE_2_PAD )

	PORT_START("R3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "C - Spell of Seeing" ) PORT_CODE( KEYCODE_C )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "D - Great Circlet" ) PORT_CODE( KEYCODE_D )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "R - Blood Sword" ) PORT_CODE( KEYCODE_R )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Give Score" ) PORT_CODE( KEYCODE_3_PAD )

	PORT_START("R4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "V - Shield" ) PORT_CODE( KEYCODE_V )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "F - Hunting Horn" ) PORT_CODE( KEYCODE_F )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "T - Chalice" ) PORT_CODE( KEYCODE_T )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Replay" ) PORT_CODE( KEYCODE_4_PAD )

	PORT_START("R5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "B - Silver Wheat" ) PORT_CODE( KEYCODE_B )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "G - Long Bow" ) PORT_CODE( KEYCODE_G )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Y - Coins" ) PORT_CODE( KEYCODE_Y )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Combine Action" ) PORT_CODE( KEYCODE_6_PAD )

	PORT_START("R6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "N - Staff" ) PORT_CODE( KEYCODE_N )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "H - Medallion" ) PORT_CODE( KEYCODE_H )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "U - Cold Fire" ) PORT_CODE( KEYCODE_U )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Save Game" ) PORT_CODE( KEYCODE_7_PAD )

	PORT_START("R7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "M - Spell of Understanding" ) PORT_CODE( KEYCODE_M )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "J - Onyx Seal" ) PORT_CODE( KEYCODE_J )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "I - Crown" ) PORT_CODE( KEYCODE_I )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Update" ) PORT_CODE( KEYCODE_8_PAD )

	PORT_START("R8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "3 - Enter" ) PORT_CODE( KEYCODE_ENTER )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "K - Orb of Quoid" ) PORT_CODE( KEYCODE_K )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "O - Crystal" ) PORT_CODE( KEYCODE_O )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Hint" ) PORT_CODE( KEYCODE_9_PAD )

	PORT_START("R9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "4 - Space" ) PORT_CODE( KEYCODE_SPACE )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "L" ) PORT_CODE( KEYCODE_L )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "P" ) PORT_CODE( KEYCODE_P )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( DEF_STR( No ) ) PORT_CODE( KEYCODE_0_PAD )
INPUT_PORTS_END

/* Machine Initialization */

static MACHINE_START( thayers )
{
	laserdisc = machine->device("laserdisc");
	memset(&ssi263, 0, sizeof(ssi263));
}

static MACHINE_RESET( thayers )
{
	int newtype;

	laserdisc_data = 0;

	rx_bit = 0;
	keylatch = 0;

	cop_data_latch = 0;
	cop_data_latch_enable = 0;
	cop_l = 0;
	cop_cmd_latch = 0;

	timer_int = 1;
	data_rdy_int = 1;
	ssi_data_request = 1;

	cart_present = 0;
	pr7820_enter = 0;

	newtype = (input_port_read(machine, "DSWB") & 0x18) ? LASERDISC_TYPE_PIONEER_LDV1000 : LASERDISC_TYPE_PIONEER_PR7820;
	laserdisc_set_type(laserdisc, newtype);
}

/* COP400 Interface */

static COP400_INTERFACE( thayers_cop_intf )
{
	COP400_CKI_DIVISOR_4, // ???
	COP400_CKO_OSCILLATOR_OUTPUT, // ???
	COP400_MICROBUS_DISABLED
};

/* Machine Driver */

static MACHINE_DRIVER_START( thayers )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, XTAL_4MHz)
	MDRV_CPU_PROGRAM_MAP(thayers_map)
	MDRV_CPU_IO_MAP(thayers_io_map)

	MDRV_CPU_ADD("mcu", COP421, XTAL_4MHz/2) // COP421L-PCA/N
	MDRV_CPU_IO_MAP(thayers_cop_io_map)
	MDRV_CPU_CONFIG(thayers_cop_intf)

	MDRV_MACHINE_START(thayers)
	MDRV_MACHINE_RESET(thayers)

	MDRV_LASERDISC_ADD("laserdisc", PIONEER_PR7820, "screen", "ldsound")

	/* video hardware */
	MDRV_LASERDISC_SCREEN_ADD_NTSC("screen", BITMAP_FORMAT_RGB32)

	MDRV_PALETTE_LENGTH(256)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	// SSI 263 @ 2MHz

	MDRV_SOUND_ADD("ldsound", LASERDISC, 0)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

/* ROMs */

ROM_START( thayers )
	ROM_REGION( 0xe000, "maincpu", 0 )
	ROM_LOAD( "tq_u33.bin", 0x0000, 0x8000, CRC(82df5d89) SHA1(58dfd62bf8c5a55d1eba397d2c284e99a4685a3f) )
	ROM_LOAD( "tq_u1.bin",  0xc000, 0x2000, CRC(e8e7f566) SHA1(df7b83ef465c65446c8418bc6007447693b75021) )

	ROM_REGION( 0x400, "mcu", 0 )
	ROM_LOAD( "tq_cop.bin", 0x000, 0x400, CRC(6748e6b3) SHA1(5d7d1ecb57c1501ef6a2d9691eecc9970586606b) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "thayers", 0, NO_DUMP )
ROM_END

ROM_START( thayersa )
	ROM_REGION( 0xe000, "maincpu", 0 )
	ROM_LOAD( "tq_u33.bin", 0x0000, 0x8000, CRC(82df5d89) SHA1(58dfd62bf8c5a55d1eba397d2c284e99a4685a3f) )
	ROM_LOAD( "tq_u1.bin",  0xc000, 0x2000, CRC(33817e25) SHA1(f9750da863dd57fe2f5b6e8fce9c6695dc5c9adc) )

	ROM_REGION( 0x400, "mcu", 0 )
	ROM_LOAD( "tq_cop.bin", 0x000, 0x400, CRC(6748e6b3) SHA1(5d7d1ecb57c1501ef6a2d9691eecc9970586606b) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "thayers", 0, NO_DUMP )
ROM_END

/* Game Drivers */

/*     YEAR  NAME      PARENT   MACHINE  INPUT    INIT  MONITOR  COMPANY               FULLNAME                           FLAGS                             LAYOUT */
GAMEL( 1984, thayers,  0,       thayers, thayers, 0,	ROT0,    "RDI Video Systems",  "Thayer's Quest",                  GAME_NOT_WORKING | GAME_NO_SOUND, layout_dlair)
GAMEL( 1984, thayersa, thayers, thayers, thayers, 0,	ROT0,    "RDI Video Systems",  "Thayer's Quest (Alternate Set)",  GAME_NOT_WORKING | GAME_NO_SOUND, layout_dlair)
