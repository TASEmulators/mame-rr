/***************************************************************************

  The Konami_1 CPU is a 6809 with opcodes scrambled. Here is how to
  descramble them.

***************************************************************************/

#include "emu.h"
#include "konami1.h"


static UINT8 konami1_decodebyte( UINT8 opcode, UINT16 address )
{
/*
>
> CPU_D7 = (EPROM_D7 & ~ADDRESS_1) | (~EPROM_D7 & ADDRESS_1)  >
> CPU_D6 = EPROM_D6
>
> CPU_D5 = (EPROM_D5 & ADDRESS_1) | (~EPROM_D5 & ~ADDRESS_1) >
> CPU_D4 = EPROM_D4
>
> CPU_D3 = (EPROM_D3 & ~ADDRESS_3) | (~EPROM_D3 & ADDRESS_3) >
> CPU_D2 = EPROM_D2
>
> CPU_D1 = (EPROM_D1 & ADDRESS_3) | (~EPROM_D1 & ~ADDRESS_3) >
> CPU_D0 = EPROM_D0
>
*/
	UINT8 xormask;


	xormask = 0;
	if (address & 0x02) xormask |= 0x80;
	else xormask |= 0x20;
	if (address & 0x08) xormask |= 0x08;
	else xormask |= 0x02;

	return opcode ^ xormask;
}



UINT8 *konami1_decode(running_machine *machine, const char *cpu)
{
	const address_space *space = cputag_get_address_space(machine, cpu, ADDRESS_SPACE_PROGRAM);
	const UINT8 *rom = memory_region(machine, cpu);
	int size = memory_region_length(machine, cpu);
	int A;

	UINT8 *decrypted = auto_alloc_array(machine, UINT8, size);
	memory_set_decrypted_region(space, 0x0000, 0xffff, decrypted);

	for (A = 0;A < size;A++)
	{
		decrypted[A] = konami1_decodebyte(rom[A],A);
	}
	return decrypted;
}
