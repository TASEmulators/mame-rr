/***************************************************************************

  Poly-Play
  (c) 1985 by VEB Polytechnik Karl-Marx-Stadt

  video hardware

  driver written by Martin Buchholz (buchholz@mail.uni-greifswald.de)

***************************************************************************/

#include "emu.h"
#include "includes/polyplay.h"


UINT8 *polyplay_characterram;



PALETTE_INIT( polyplay )
{
	palette_set_color(machine,0,MAKE_RGB(0x00,0x00,0x00));
	palette_set_color(machine,1,MAKE_RGB(0xff,0xff,0xff));

	palette_set_color(machine,2,MAKE_RGB(0x00,0x00,0x00));
	palette_set_color(machine,3,MAKE_RGB(0xff,0x00,0x00));
	palette_set_color(machine,4,MAKE_RGB(0x00,0xff,0x00));
	palette_set_color(machine,5,MAKE_RGB(0xff,0xff,0x00));
	palette_set_color(machine,6,MAKE_RGB(0x00,0x00,0xff));
	palette_set_color(machine,7,MAKE_RGB(0xff,0x00,0xff));
	palette_set_color(machine,8,MAKE_RGB(0x00,0xff,0xff));
	palette_set_color(machine,9,MAKE_RGB(0xff,0xff,0xff));
}


WRITE8_HANDLER( polyplay_characterram_w )
{
	if (polyplay_characterram[offset] != data)
	{
		gfx_element_mark_dirty(space->machine->gfx[1], (offset >> 3) & 0x7f);

		polyplay_characterram[offset] = data;
	}
}

VIDEO_START( polyplay )
{
	gfx_element_set_source(machine->gfx[1], polyplay_characterram);
}


VIDEO_UPDATE( polyplay )
{
	offs_t offs;


	for (offs = 0; offs < screen->machine->generic.videoram_size; offs++)
	{
		int sx = (offs & 0x3f) << 3;
		int sy = offs >> 6 << 3;
		UINT8 code = screen->machine->generic.videoram.u8[offs];

		drawgfx_opaque(bitmap,cliprect, screen->machine->gfx[(code >> 7) & 0x01],
				code, 0, 0, 0, sx, sy);
	}

	return 0;
}
