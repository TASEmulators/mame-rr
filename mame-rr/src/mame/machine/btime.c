#include "emu.h"
#include "includes/btime.h"


#define BASE  0xb000

READ8_HANDLER( mmonkey_protection_r )
{
	btime_state *state = (btime_state *)space->machine->driver_data;
	UINT8 *RAM = memory_region(space->machine, "maincpu");
	int ret = 0;

	if (offset == 0x0000)
		ret = state->protection_status;
	else if (offset == 0x0e00)
		ret = state->protection_ret;
	else if (offset >= 0x0d00 && offset <= 0x0d02)
		ret = RAM[BASE + offset];  /* addition result */
	else
		logerror("Unknown protection read.  PC=%04X  Offset=%04X\n", cpu_get_pc(space->cpu), offset);

	return ret;
}

WRITE8_HANDLER( mmonkey_protection_w )
{
	btime_state *state = (btime_state *)space->machine->driver_data;
	UINT8 *RAM = memory_region(space->machine, "maincpu");

	if (offset == 0)
	{
		/* protection trigger */
		if (data == 0)
		{
			int i, s1, s2, r;

			switch (state->protection_command)
			{
			case 0:	/* score addition */

				s1 = (1 * (RAM[BASE + 0x0d00] & 0x0f)) + (10 * (RAM[BASE + 0x0d00] >> 4)) +
					(100 * (RAM[BASE + 0x0d01] & 0x0f)) + (1000 * (RAM[BASE + 0x0d01] >> 4)) +
					(10000 * (RAM[BASE + 0x0d02] & 0x0f)) + (100000 * (RAM[BASE + 0x0d02] >> 4));

				s2 = (1 * (RAM[BASE + 0x0d03] & 0x0f)) + (10 * (RAM[BASE + 0x0d03] >> 4)) +
					(100 * (RAM[BASE + 0x0d04] & 0x0f)) + (1000 * (RAM[BASE + 0x0d04] >> 4)) +
					(10000 * (RAM[BASE + 0x0d05] & 0x0f)) + (100000 * (RAM[BASE + 0x0d05] >> 4));

				r = s1 + s2;

				RAM[BASE + 0x0d00]  =  (r % 10);        r /= 10;
				RAM[BASE + 0x0d00] |= ((r % 10) << 4);  r /= 10;
				RAM[BASE + 0x0d01]  =  (r % 10);        r /= 10;
				RAM[BASE + 0x0d01] |= ((r % 10) << 4);  r /= 10;
				RAM[BASE + 0x0d02]  =  (r % 10);        r /= 10;
				RAM[BASE + 0x0d02] |= ((r % 10) << 4);

				break;

			case 1:	/* decryption */

				/* Compute return value by searching the decryption table. */
				/* During the search the status should be 2, but we're done */
				/* instanteniously in emulation time */
				for (i = 0; i < 0x100; i++)
				{
					if (RAM[BASE + 0x0f00 + i] == state->protection_value)
					{
						state->protection_ret = i;
						break;
					}
				}
				break;

			default:
				logerror("Unemulated protection command=%02X.  PC=%04X\n", state->protection_command, cpu_get_pc(space->cpu));
				break;
			}

			state->protection_status = 0;
		}
	}
	else if (offset == 0x0c00)
		state->protection_command = data;
	else if (offset == 0x0e00)
		state->protection_value = data;
	else if (offset >= 0x0f00)
		RAM[BASE + offset] = data;   /* decrypt table */
	else if (offset >= 0x0d00 && offset <= 0x0d05)
		RAM[BASE + offset] = data;   /* source table */
	else
		logerror("Unknown protection write=%02X.  PC=%04X  Offset=%04X\n", data, cpu_get_pc(space->cpu), offset);
}

