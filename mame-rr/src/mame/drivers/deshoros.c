/***************************************************************************

Destiny Horoscope (c) 1983 Data East Corporation

driver by Angelo Salese

A fortune-teller machine with 24 characters LED-array and a printer.

TODO:
-Emulate the graphics with genuine artwork display;
-Printer emulation;

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"

static UINT8 *io_ram;
/*Temporary,to show something on screen...*/
static char led_array[21];

static VIDEO_START( deshoros )
{
	static UINT8 i;
	for(i=0;i<20;i++)
		led_array[i] = 0x20;
	led_array[20] = 0;
}

static VIDEO_UPDATE( deshoros )
{
	popmessage("%s",led_array);
	return 0;
}

/*I don't know it this is 100% correct,might be different...*/
static void update_led_array(UINT8 new_data)
{
	static UINT8 i;
	/*scroll the data*/
	for(i=0;i<19;i++)
		led_array[i] = led_array[i+1];
	/*update the data*/
	led_array[19] = new_data;
}

static UINT8 bank;

static void answer_bankswitch(running_machine *machine,UINT8 new_bank)
{
	if(bank!=new_bank)
	{
		UINT8 *ROM = memory_region(machine, "data");
		UINT32 bankaddress;

		bank = new_bank;
		bankaddress = 0 + 0x6000 * bank;
		memory_set_bankptr(machine, "bank1", &ROM[bankaddress]);
	}
}

static READ8_HANDLER( io_r )
{
	switch(offset)
	{
		case 0x00: return 0xff; //printer read
		case 0x03: return input_port_read(space->machine, "KEY0" );
		case 0x04: return input_port_read(space->machine, "KEY1" );
		case 0x05: return input_port_read(space->machine, "SYSTEM" );
		case 0x0a: return io_ram[offset]; //"buzzer" 0 read
		case 0x0b: return io_ram[offset]; //"buzzer" 1 read
	}
//  printf("R -> [%02x]\n",offset);

	return io_ram[offset];
}

static WRITE8_HANDLER( io_w )
{
	switch(offset)
	{
		case 0x00: /*Printer data*/						return;
		case 0x02: update_led_array(data);              return;
		case 0x05: coin_lockout_w(space->machine, 0,io_ram[offset] & 1);return;
		case 0x06: /*Printer IRQ enable*/   		    return;
//      case 0x0a: "buzzer" 0 write
//      case 0x0b: "buzzer" 1 write
		case 0x0c: answer_bankswitch(space->machine,data&0x03); return; //data & 0x10 enabled too,dunno if it is worth to shift the data...
	}
	io_ram[offset] = data;
//  printf("%02x -> [%02x]\n",data,offset);
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x900f) AM_READWRITE(io_r,io_w) AM_BASE(&io_ram) //i/o area
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( deshoros )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Key Male") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 )  PORT_NAME("Key 3")  PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )  PORT_NAME("Key 2")  PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )  PORT_NAME("Key 1")  PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Key Female") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 )  PORT_NAME("Key 6")  PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 )  PORT_NAME("Key 5")  PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )  PORT_NAME("Key 4")  PORT_CODE(KEYCODE_4_PAD)
	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Key 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON9 )  PORT_NAME("Key 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON8 )  PORT_NAME("Key 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 )  PORT_NAME("Key 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Key Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("Key Cancel") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_DIPNAME( 0x01,   0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02,   0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04,   0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08,   0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10,   0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x10, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_DIPNAME( 0xc0,   0x00, "Operation Mode" )
	PORT_DIPSETTING(      0x00, "Normal Mode" )
	PORT_DIPSETTING(      0x80, "Test Mode" )
	PORT_DIPSETTING(      0xc0, "I/O Test" )
	//                    0x40, Normal Mode again
INPUT_PORTS_END

/*Is it there an IRQ mask?*/
static INTERRUPT_GEN( deshoros_irq )
{
	cputag_set_input_line(device->machine, "maincpu", M6809_IRQ_LINE, HOLD_LINE);
}

static MACHINE_RESET( deshoros )
{
	bank = -1;
}

static MACHINE_DRIVER_START( deshoros )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",M6809,2000000)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen",deshoros_irq)

	MDRV_MACHINE_RESET(deshoros)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(48*8, 16*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 0*8, 16*8-1)
	MDRV_PALETTE_LENGTH(16)

	MDRV_VIDEO_START(deshoros)
	MDRV_VIDEO_UPDATE(deshoros)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( deshoros )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ag12-4",   0xc000, 0x2000, CRC(03b2c850) SHA1(4e2c49a8d80bc559d0f406caddddb85bc107aac0) )
	ROM_LOAD( "ag13-4",   0xe000, 0x2000, CRC(36959ef6) SHA1(9b3ed44416fcda6a8e89d11ad6e713abd4f63d83) )

	ROM_REGION( 0x18000, "data", 0 ) //answers data
	ROM_LOAD( "ag00",   0x00000, 0x2000, CRC(77f5bce0) SHA1(20b5257710c5e848893fec107f0d87a473a4ba24) )
	ROM_LOAD( "ag01",   0x02000, 0x2000, CRC(c08e6a74) SHA1(88679ed8bd2b6b8698258baddf8433c0f60a1b64) )
	ROM_LOAD( "ag02",   0x04000, 0x2000, CRC(687c72b5) SHA1(3f2768c9b6247e96d11b4159f6f5c0dfeb2c5075) )
	ROM_LOAD( "ag03",   0x06000, 0x2000, CRC(535dbe83) SHA1(29336539c57d1fa7d42a0ce01884b29e1707e9ad) )
	ROM_LOAD( "ag04",   0x08000, 0x2000, CRC(e6ae8eb7) SHA1(d0e20e438dcfeac9d844d1fd98701a443ea5e4f7) )
	ROM_LOAD( "ag05",   0x0a000, 0x2000, CRC(c2485e40) SHA1(03f6d7c63a45d430a7965e28aaf07e053ecac7a1) )
	ROM_LOAD( "ag06",   0x0c000, 0x2000, CRC(e6e0bbd1) SHA1(fe693d038b05ae18a3c0cfb25a4649dbb10ab2c7) )
	ROM_LOAD( "ag07",   0x0e000, 0x2000, CRC(a62d879d) SHA1(94d07e774df4c9e4e34ae386714372b53b255530) )
	ROM_LOAD( "ag08",   0x10000, 0x2000, CRC(f5822738) SHA1(afe53e875057317033cdd5f4b7614c96cd11193b) )
	ROM_LOAD( "ag09",   0x12000, 0x2000, CRC(ad3c9f2c) SHA1(f665efb65c072a3d3d2e19844ebe0b352c0251d3) )
	ROM_LOAD( "ag10",   0x14000, 0x2000, CRC(c498754a) SHA1(90e215e8e41d32237d1f4b074d93e20eade92e4e) )
	ROM_LOAD( "ag11",   0x16000, 0x2000, CRC(5f7bf9f9) SHA1(281f89c0bccfcc2bdc1d4d0a5b9cc9a8ab2e7869) )
ROM_END

GAME( 1983, deshoros,  0,       deshoros,  deshoros,  0, ROT0, "Data East Corporation", "Destiny Horoscope", GAME_NO_SOUND | GAME_NOT_WORKING )
