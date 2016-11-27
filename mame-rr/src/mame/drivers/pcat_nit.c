/********************************************************************************************************************

Street Games (c) 1993 New Image Technologies

A modified 386 PC-AT HW

preliminary driver by Angelo Salese and Mariusz Wojcieszek

=====================================================================================================================

These games were made by New Image Technologies. They are touch screen multi games from around 1993.
Guru dumped one of their games a year ago but the pcb was rusted out and I think the roms might
be bad, not sure.

nvram is a potential problem. I have them separated into their own folder for now. I'm not really s
ure if they are needed, but I found this inside a Bonanza rom so they may hold a key:

-------------------------------------------
INVALID KEY
_
Programming DS1204
_
Successfully Programmed DS1204
_
Failed Programming DS1204
_
Verifying DS1204
_
Failed to read secure data
_
Failed to read ID DS1204
_
Verified DS1204
-------------------------------------------

I have them (the real chips) for my Bonanza rom boards, but they are missing on my Street Games boards.
However, the back of one of the manuals mentions game swapping on these boards, and it doesn't mention
this chip as needing to be changed. I included the nvram dump from Guru's Street Games II dump, so maybe
that will work on all the Street Games sets? I don't know. My Dallas chip read good as 2764 and verified
ok. The other one (bq4010yma-150) reads the same crc most reads but won't pass the verify function.
It may still be good. Those are for the Bonanza sets as you'll see in the pics.

I made copies of the manuals for Street Games and Bonanza and included lots of pics.

From the looks of the boards I got, Street Games II looks like it uses the 3 original Street Games roms
and just builds on them. You'll see what I mean in the pics.

I don't know if all 3 vga bioses are needed or not, one is soldered and not dumped, I dumped the other two.
I can get the soldered one dumped in the future if it turns out we need it. When these games got upgraded,
they sometimes came with a new video board. Switching from Street Games to Street Games II probably didn't need
it but switching to Bonanza did. The vga bioses I dumped came off of the 2 different video boards for Bonanza.
I dumped them as 27c256. The one on the Street Games video board is the surface mounted one.

The end numbers on the rom stickers give away the version info. For example, it looks like Guru's old dump
of Street Games II is revision 7C.

The system bios roms I dumped give an error in romcmp but they seem to verify ok so I think they are good.

My 2 rom boards for Bonanza were fortunately 2 different revisions. I ended up with 2 sets of Street Games,
1 set of Street Games II, and 2 different sets of Bonanza. A second set of Street Games II might be able to be
"created" if his dump is good because you will notice in the pics, it looks like Street Games II used 3 roms
from Street Games I. That or one of my rom boards came populated incorrectly.

The ATI keyboard bios is an HT6542. The Jet one I'm not sure. I don't think they can be read. Same for the M5818's.

My set naming logic...

bonanza = Bonanza (Rev 3)
bonanza2 = Bonanza (Rev 2)

streetg = Street Games (Rev 4)
streetg2 = Street Games (Rev 2)

streetgii = Street Games II (Rev 5)

Anything else need to be dumped? More pics needed, etc.? Contact me.


Smitdogg

********************************************************************************************************************/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pic8259.h"
#include "machine/mc146818.h"
#include "machine/pcshare.h"
#include "machine/ins8250.h"
#include "machine/microtch.h"
#include "video/pc_vga.h"
#include "video/pc_video.h"

static void pcat_nit_microtouch_tx_callback(running_machine *machine, UINT8 data)
{
	ins8250_receive(machine->device("ns16450_0"), data);
};

static INS8250_TRANSMIT( pcat_nit_com_transmit )
{
	UINT8 data8 = data;
	microtouch_rx(1, &data8);
}

static INS8250_INTERRUPT( at_com_interrupt_1 )
{
	pic8259_ir4_w(device->machine->device("pic8259_1"), state);
}

static const ins8250_interface pcat_nit_com0_interface =
{
	1843200,
	at_com_interrupt_1,
	pcat_nit_com_transmit,
	NULL,
	NULL
};

/*************************************
 *
 *  ROM banking
 *
 *************************************/

static WRITE8_HANDLER(pcat_nit_rombank_w)
{
	logerror( "rom bank #%02x at PC=%08X\n", data, cpu_get_pc(space->cpu) );
	if ( data & 0x40 )
	{
		// rom bank
		memory_install_read_bank(space, 0x000d8000, 0x000dffff, 0, 0, "rombank" );
		memory_unmap_write(space, 0x000d8000, 0x000dffff, 0, 0);

		if ( data & 0x80 )
		{
			memory_set_bank(space->machine, "rombank", (data & 0x3f) | 0x40 );
		}
		else
		{
			memory_set_bank(space->machine, "rombank", data & 0x3f );
		}
	}
	else
	{
		// nvram bank
		memory_unmap_read(space, 0x000d8000, 0x000dffff, 0, 0);
		memory_unmap_write(space, 0x000d8000, 0x000dffff, 0, 0);

		memory_install_read_bank(space, 0x000d8000, 0x000d9fff, 0, 0, "nvrambank" );
		memory_install_write_bank(space, 0x000d8000, 0x000d9fff, 0, 0, "nvrambank" );

		memory_set_bankptr(space->machine, "nvrambank", space->machine->generic.nvram.u8);

	}
}

static ADDRESS_MAP_START( pcat_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM
	AM_RANGE(0x000c0000, 0x000c7fff) AM_ROM AM_REGION("video_bios", 0)
	AM_RANGE(0x000d0000, 0x000d3fff) AM_RAM AM_REGION("disk_bios", 0)
	AM_RANGE(0x000d7000, 0x000d7003) AM_WRITE8(pcat_nit_rombank_w, 0xff)
	AM_RANGE(0x000d8000, 0x000dffff) AM_ROMBANK("rombank")
	AM_RANGE(0x000f0000, 0x000fffff) AM_RAM AM_REGION("bios", 0 )
	AM_RANGE(0xffff0000, 0xffffffff) AM_ROM AM_REGION("bios", 0 )
ADDRESS_MAP_END

static READ8_HANDLER(pcat_nit_io_r)
{
	switch(offset)
	{
		case 0: /* 278 */
			return 0xff;
		case 1: /* 279 */
			return input_port_read(space->machine, "IN0");
		case 7: /* 27f dips */
			return 0xff;
		default:
			return 0;
	}
}

static ADDRESS_MAP_START( pcat_nit_io, ADDRESS_SPACE_IO, 32 )
	AM_IMPORT_FROM(pcat32_io_common)
	AM_RANGE(0x0278, 0x027f) AM_READ8(pcat_nit_io_r, 0xffffffff) AM_WRITENOP
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ns16450_0", ins8250_r, ins8250_w, 0xffffffff)
ADDRESS_MAP_END

static INPUT_PORTS_START( pcat_nit )
	PORT_INCLUDE(microtouch)

	PORT_START("IN0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Clear") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_SERVICE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN1) PORT_IMPULSE(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN2) PORT_IMPULSE(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN3) PORT_IMPULSE(1)
INPUT_PORTS_END

static void streetg2_set_keyb_int(running_machine *machine, int state)
{
	pic8259_ir1_w(machine->device("pic8259_1"), state);
}

static const struct pc_vga_interface vga_interface =
{
	NULL,
	NULL,
	NULL,
	ADDRESS_SPACE_IO,
	0x0000
};

static MACHINE_START( streetg2 )
{
	cpu_set_irq_callback(machine->device("maincpu"), pcat_irq_callback);

	init_pc_common(machine, PCCOMMON_KEYBOARD_AT, streetg2_set_keyb_int);
	mc146818_init(machine, MC146818_STANDARD);

	memory_configure_bank(machine, "rombank", 0, 0x80, memory_region(machine, "game_prg"), 0x8000 );
	memory_set_bank(machine, "rombank", 0);

	microtouch_init(machine, pcat_nit_microtouch_tx_callback, NULL);
}

static MACHINE_DRIVER_START( pcat_nit )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", I386, 14318180*2)	/* I386 ?? Mhz */
	MDRV_CPU_PROGRAM_MAP(pcat_map)
	MDRV_CPU_IO_MAP(pcat_nit_io)

	/* video hardware */
	MDRV_IMPORT_FROM( pcvideo_vga )

	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */

	MDRV_MACHINE_START(streetg2)
	MDRV_NVRAM_HANDLER( mc146818 )

//  MDRV_IMPORT_FROM( at_kbdc8042 )
	MDRV_IMPORT_FROM( pcat_common )
	MDRV_NS16450_ADD( "ns16450_0", pcat_nit_com0_interface )

MACHINE_DRIVER_END

/***************************************
*
* ROM definitions
*
***************************************/

ROM_START(streetg)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("system-bios-10-0004-01.u6", 0x00000, 0x10000, CRC(e4d6511f) SHA1(d432743f549fa6ecc04bc5bf94999253f86af08c) )

	ROM_REGION(0x08000, "video_bios", 0)
	ROM_LOAD16_BYTE("vga1-bios-ver-b-1.00-07.u8",     0x00000, 0x04000, CRC(a40551d6) SHA1(db38190f06e4af2c2d59ae310e65883bb16cd3d6))
	ROM_CONTINUE(                                     0x00001, 0x04000 )

	ROM_REGION(0x400000, "game_prg", 0)	/* proper game */
	ROM_LOAD("10-0003-04.u11", 0x000000,0x80000, CRC(1471a728) SHA1(5e12a9230f8130282a1be9a10118a3556bafbc37) )
	ROM_LOAD("10-0003-04.u12", 0x080000,0x80000, CRC(5a50f519) SHA1(c07a583b4220d4d5506824def91774fede760e65) )
	ROM_LOAD("10-0003-04.u13", 0x100000,0x80000, CRC(8a609145) SHA1(18fcb58b461aa9149a163b85dd8267dec90da3cd) )

	ROM_REGION(0x08000, "disk_bios", 0)
	ROM_LOAD("disk-bios-10-0001-04.u10",     0x00000, 0x08000, CRC(1b4ce068) SHA1(8570b36acf3eb29f1c59e56a4dad6d38c218748f) )

	ROM_REGION(0x08000, "nvram_data", 0)
	ROM_LOAD("8k_nvram.u9",     0x00000, 0x02000, CRC(44be0b89) SHA1(81666dd369d1d85269833293136d61ffe80e940a))
ROM_END

ROM_START(streetgr3)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("system-bios-10-0004-01.u6", 0x00000, 0x10000, CRC(e4d6511f) SHA1(d432743f549fa6ecc04bc5bf94999253f86af08c) )

	ROM_REGION(0x08000, "video_bios", 0)
	ROM_LOAD16_BYTE("vga1-bios-ver-b-1.00-07.u8",     0x00000, 0x04000, CRC(a40551d6) SHA1(db38190f06e4af2c2d59ae310e65883bb16cd3d6))
	ROM_CONTINUE(                                     0x00001, 0x04000 )

	ROM_REGION(0x400000, "game_prg", 0)	/* proper game */
	ROM_LOAD("10-00003-03.u11", 0x000000,0x80000, CRC(2fbcb12b) SHA1(f6413565fc1289ba32b411de877cdf6526f1fa9d) )
	ROM_LOAD("10-00003-03.u12", 0x080000,0x80000, CRC(b37c7dff) SHA1(cf6318bfeca0bd272734f45c7589a0224863b0f1) )
	ROM_LOAD("10-00003-03.u13", 0x100000,0x80000, CRC(6a9d0771) SHA1(6cd9a56a2413416d0928e5cf9340c94bc0c87c46) )

	ROM_REGION(0x08000, "disk_bios", 0)
	ROM_LOAD("disk-bios-10-0001-04.u10",     0x00000, 0x08000, CRC(1b4ce068) SHA1(8570b36acf3eb29f1c59e56a4dad6d38c218748f) )

	ROM_REGION(0x08000, "nvram_data", 0)
	ROM_LOAD("8k_nvram.u9",     0x00000, 0x02000, CRC(44be0b89) SHA1(81666dd369d1d85269833293136d61ffe80e940a))
ROM_END

ROM_START(bonanza)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("system-bios-sx-10-0004-02.u6", 0x00000, 0x10000, CRC(fa545ba8) SHA1(db64548bd87262cd2e82175a1b66f168b5ae072d) )

	ROM_REGION(0x08000, "video_bios", 0)
	ROM_LOAD16_BYTE("techyosd-isa-bios-v1.2.u8",     0x00000, 0x04000, CRC(6adf7e71) SHA1(2b07d964cc7c2c0aa560625b7c12f38d4537d652) )
	ROM_CONTINUE(                                    0x00001, 0x04000 )

	ROM_REGION(0x400000, "game_prg", 0)	/* proper game */
	ROM_LOAD("10-0018-03-090894.u11", 0x000000,0x80000, CRC(32b6c8bc) SHA1(7f4097990dca268915842d4253d4257654de2cfc) )
	ROM_LOAD("10-0018-03-090894.u12", 0x080000,0x80000, CRC(d7cb191d) SHA1(2047f3668b0e41ad5347107f4e3446c0374c5bb7) )
	ROM_LOAD("10-0018-03-090894.u13", 0x100000,0x80000, CRC(1d3ddeaa) SHA1(8e73fe535882f6d634668733e550281e727fbdbc) )
	ROM_LOAD("10-0018-03-090894.u15", 0x200000,0x80000, CRC(b9b3f442) SHA1(6ea5ce3eb007b95ad3350fdb634625b151ae7bdb) )
	ROM_LOAD("10-0018-03-090894.u16", 0x280000,0x80000, CRC(5b0dd6f5) SHA1(8172118185179ecb7d3f958480186bf9c906785f) )
	ROM_LOAD("10-0018-03-090894.u17", 0x300000,0x80000, CRC(b637eb58) SHA1(7c4615f58118d9b82575d816ef916fccbb1be0f9) )

	ROM_REGION(0x08000, "disk_bios", 0)
	ROM_LOAD("disk-bios-10-0001-04.u10",     0x00000, 0x08000, CRC(1b4ce068) SHA1(8570b36acf3eb29f1c59e56a4dad6d38c218748f) )

	ROM_REGION(0x08000, "nvram_data", 0)
	ROM_LOAD("bq4010yma-150.u9",     0x00000, 0x02000, CRC(f4ca28ee) SHA1(17b852028568fb814df62f5870b91a8303302b55))
ROM_END

ROM_START(bonanzar2)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("system-bios-sx-10-0004-02.u6", 0x00000, 0x10000, CRC(fa545ba8) SHA1(db64548bd87262cd2e82175a1b66f168b5ae072d) )

	ROM_REGION(0x08000, "video_bios", 0)
	ROM_LOAD16_BYTE("techyosd-isa-bios-v1.2.u8",     0x00000, 0x04000, CRC(6adf7e71) SHA1(2b07d964cc7c2c0aa560625b7c12f38d4537d652) )
	ROM_CONTINUE(                                    0x00001, 0x04000 )

	ROM_REGION(0x400000, "game_prg", 0)	/* proper game */
	ROM_LOAD("10-0018-02-081794.u11", 0x000000,0x80000, CRC(f87fa935) SHA1(b06144496406231aa63149ae12a048ffab8f77d0) )
	ROM_LOAD("10-0018-02-081794.u12", 0x080000,0x80000, CRC(bd892e3e) SHA1(1b9174fe2a6eaa7687b543798099b86b9039c049) )
	ROM_LOAD("10-0018-02-081794.u13", 0x100000,0x80000, CRC(626d999e) SHA1(5c27e3b064b0235c0d6e0be8d8f78538a11647a2) )
	ROM_LOAD("10-0018-02-081794.u15", 0x200000,0x80000, CRC(3b28f582) SHA1(3da61fbd92e6cc60e00eaa21d8fb04aa78cce663) )
	ROM_LOAD("10-0018-02-081794.u16", 0x280000,0x80000, CRC(fe29ad76) SHA1(64aaae639f024c50c09fe920bc92e6d45ced5648) )
	ROM_LOAD("10-0018-02-081794.u17", 0x300000,0x80000, CRC(066108fe) SHA1(ef837422a2a81f5ac3375b6ed68f20143ac6caec) )

	ROM_REGION(0x08000, "disk_bios", 0)
	ROM_LOAD("disk-bios-10-0001-04.u10",     0x00000, 0x08000, CRC(1b4ce068) SHA1(8570b36acf3eb29f1c59e56a4dad6d38c218748f) )

	ROM_REGION(0x08000, "nvram_data", 0)
	ROM_LOAD("bq4010yma-150.u9",     0x00000, 0x02000, CRC(f4ca28ee) SHA1(17b852028568fb814df62f5870b91a8303302b55))
ROM_END

ROM_START(streetg2)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("10-0004-01_mb-bios.bin", 0x00000, 0x10000, CRC(e4d6511f) SHA1(d432743f549fa6ecc04bc5bf94999253f86af08c) )

	ROM_REGION(0x08000, "video_bios", 0)
	ROM_LOAD16_BYTE("vga1-bios-ver-b-1.00-07.u8",     0x00000, 0x04000, CRC(a40551d6) SHA1(db38190f06e4af2c2d59ae310e65883bb16cd3d6))
	ROM_CONTINUE(                                     0x00001, 0x04000 )

	ROM_REGION(0x400000, "game_prg", 0)	/* proper game */
	ROM_LOAD("10-0007-07c_083194_rom4.u11", 0x000000,0x80000, CRC(244c2bfa) SHA1(4f2f0fb6923b4e3f1ab4e607e29a27fb15b39fac) )
	ROM_LOAD("10-0007-07c_083194_rom5.u12", 0x080000,0x80000, CRC(c89d5dca) SHA1(212bcbf7a39243f4524b4a855fbedabd387d17f2) )
	ROM_LOAD("10-0007-07c_083194_rom6.u13", 0x100000,0x80000, CRC(6264f65f) SHA1(919a8e5d9861dc642ac0f0885faed544bbafa321) )

	ROM_REGION(0x08000, "disk_bios", 0)
	ROM_LOAD("10-0001-03_disk_bios.u10",     0x00000, 0x08000, CRC(d6ba8b37) SHA1(1d1d984bc15fd154fc07dcfa2132bd44636d7bf1))

	ROM_REGION(0x08000, "nvram_data", 0)
	ROM_LOAD("8k_nvram.u9",     0x00000, 0x02000, CRC(44be0b89) SHA1(81666dd369d1d85269833293136d61ffe80e940a))
ROM_END

ROM_START(streetg2r5)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("10-0004-01_mb-bios.bin", 0x00000, 0x10000, CRC(e4d6511f) SHA1(d432743f549fa6ecc04bc5bf94999253f86af08c) )

	ROM_REGION(0x08000, "video_bios", 0)
	ROM_LOAD16_BYTE("vga1-bios-ver-b-1.00-07.u8",     0x00000, 0x04000, CRC(a40551d6) SHA1(db38190f06e4af2c2d59ae310e65883bb16cd3d6))
	ROM_CONTINUE(                                     0x00001, 0x04000 )

	ROM_REGION(0x400000, "game_prg", 0)	/* proper game */
	ROM_LOAD("10-00007-05-032194.u15", 0x000000,0x80000, CRC(cefa230f) SHA1(91fd30a3def381974fae0edb4d42d452acda19bb) )
	ROM_LOAD("10-00007-05-032194.u16", 0x080000,0x80000, CRC(0be5dd19) SHA1(d0474ff5156e1fa8b4edb502c49b7e1a2b3f6169) )
	ROM_LOAD("10-00007-05-032194.u17", 0x100000,0x80000, CRC(f6c996b9) SHA1(871a8d093b856511a0e2b03334ef5c66a2482622) )

	ROM_REGION(0x08000, "disk_bios", 0)
	ROM_LOAD("10-0001-03_disk_bios.u10",     0x00000, 0x08000, CRC(d6ba8b37) SHA1(1d1d984bc15fd154fc07dcfa2132bd44636d7bf1))

	ROM_REGION(0x08000, "nvram_data", 0)
	ROM_LOAD("8k_nvram.u9",     0x00000, 0x02000, CRC(44be0b89) SHA1(81666dd369d1d85269833293136d61ffe80e940a))
ROM_END

static DRIVER_INIT(pcat_nit)
{
	machine->generic.nvram_size = 0x2000;
	machine->generic.nvram.u8 = auto_alloc_array(machine, UINT8, machine->generic.nvram_size);

	pc_vga_init(machine, &vga_interface, NULL);
}

GAME( 1993, bonanza,    0,		   pcat_nit,  pcat_nit, pcat_nit, ROT0, "New Image Technologies",  "Bonanza (Revision 3)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1993, bonanzar2,  bonanza,   pcat_nit,  pcat_nit, pcat_nit, ROT0, "New Image Technologies",  "Bonanza (Revision 2)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1993, streetg,    0,		   pcat_nit,  pcat_nit, pcat_nit, ROT0, "New Image Technologies",  "Street Games (Revision 4)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1993, streetgr3,  streetg,   pcat_nit,  pcat_nit, pcat_nit, ROT0, "New Image Technologies",  "Street Games (Revision 3)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1993, streetg2,   0,		   pcat_nit,  pcat_nit, pcat_nit, ROT0, "New Image Technologies",  "Street Games II (Revision 7C)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1993, streetg2r5, streetg2,  pcat_nit,  pcat_nit, pcat_nit, ROT0, "New Image Technologies",  "Street Games II (Revision 5)", GAME_NOT_WORKING|GAME_NO_SOUND )
