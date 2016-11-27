/*
 *  Beatmania DJ Main Board (GX753)
 *  emulate video hardware
 */

#include "emu.h"
#include "video/konicdev.h"

#define NUM_SPRITES	(0x800 / 16)
#define NUM_LAYERS	2

UINT32 *djmain_obj_ram;


static void draw_sprites(running_machine* machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	running_device *k055555 = machine->device("k055555");
	int offs, pri_code;
	int sortedlist[NUM_SPRITES];

	machine->gfx[0]->color_base = k055555_read_register(k055555, K55_PALBASE_SUB2) * 0x400;

	for (offs = 0; offs < NUM_SPRITES; offs++)
		sortedlist[offs] = -1;

	/* prebuild a sorted table */
	for (offs = 0; offs < NUM_SPRITES * 4; offs += 4)
	{
		if (djmain_obj_ram[offs] & 0x00008000)
		{
			if (djmain_obj_ram[offs] & 0x80000000)
				continue;

			pri_code = djmain_obj_ram[offs] & (NUM_SPRITES - 1);
			sortedlist[pri_code] = offs;
		}
	}

	for (pri_code = NUM_SPRITES - 1; pri_code >= 0; pri_code--)
	{
		static const int xoffset[8] = { 0, 1, 4, 5, 16, 17, 20, 21 };
		static const int yoffset[8] = { 0, 2, 8, 10, 32, 34, 40, 42 };
		static const int sizetab[4] =  { 1, 2, 4, 8 };
		int x, y;
		int ox, oy;
		int flipx, flipy;
		int xscale, yscale;
		int code;
		int color;
		int size;

		offs = sortedlist[pri_code];
		if (offs == -1) continue;

		code = djmain_obj_ram[offs] >> 16;
		flipx = (djmain_obj_ram[offs] >> 10) & 1;
		flipy = (djmain_obj_ram[offs] >> 11) & 1;
		size = sizetab[(djmain_obj_ram[offs] >> 8) & 3];

		ox = (INT16)(djmain_obj_ram[offs + 1] & 0xffff);
		oy = (INT16)(djmain_obj_ram[offs + 1] >> 16);

		xscale = djmain_obj_ram[offs + 2] >> 16;
		yscale = djmain_obj_ram[offs + 2] & 0xffff;

		if (!xscale || !yscale)
			continue;

		xscale = (0x40 << 16) / xscale;
		yscale = (0x40 << 16) / yscale;
		ox -= (size * xscale) >> 13;
		oy -= (size * yscale) >> 13;

		color = (djmain_obj_ram[offs + 3] >> 16) & 15;

		for (x = 0; x < size; x++)
			for (y = 0; y < size; y++)
			{
				int c = code;

				if (flipx)
					c += xoffset[size - x - 1];
				else
					c += xoffset[x];

				if (flipy)
					c += yoffset[size - y - 1];
				else
					c += yoffset[y];

				if (xscale != 0x10000 || yscale != 0x10000)
				{
					int sx = ox + ((x * xscale + (1 << 11)) >> 12);
					int sy = oy + ((y * yscale + (1 << 11)) >> 12);
					int zw = ox + (((x + 1) * xscale + (1 << 11)) >> 12) - sx;
					int zh = oy + (((y + 1) * yscale + (1 << 11)) >> 12) - sy;

					drawgfxzoom_transpen(bitmap,
					            cliprect,
					            machine->gfx[0],
					            c,
					            color,
					            flipx,
					            flipy,
					            sx,
					            sy,
					            (zw << 16) / 16,
					            (zh << 16) / 16,
					            0);
				}
				else
				{
					int sx = ox + (x << 4);
					int sy = oy + (y << 4);

					drawgfx_transpen(bitmap,
					        cliprect,
					        machine->gfx[0],
					        c,
					        color,
					        flipx,
					        flipy,
					        sx,
					        sy,
					        0);
				}
			}
	}
}


void djmain_tile_callback(running_machine* machine, int layer, int *code, int *color, int *flags)
{
}

VIDEO_START( djmain )
{
	running_device *k056832 = machine->device("k056832");

	k056832_set_layer_offs(k056832, 0, -92, -27);
	// k056832_set_layer_offs(k056832, 1, -87, -27);
	k056832_set_layer_offs(k056832, 1, -88, -27);
}

VIDEO_UPDATE( djmain )
{
	running_device *k056832 = screen->machine->device("k056832");
	running_device *k055555 = screen->machine->device("k055555");
	int enables = k055555_read_register(k055555, K55_INPUT_ENABLES);
	int pri[NUM_LAYERS + 1];
	int order[NUM_LAYERS + 1];
	int i, j;

	for (i = 0; i < NUM_LAYERS; i++)
		pri[i] = k055555_read_register(k055555, K55_PRIINP_0 + i * 3);
	pri[i] = k055555_read_register(k055555, K55_PRIINP_10);

	for (i = 0; i < NUM_LAYERS + 1; i++)
		order[i] = i;

	for (i = 0; i < NUM_LAYERS; i++)
		for (j = i + 1; j < NUM_LAYERS + 1; j++)
			if (pri[order[i]] > pri[order[j]])
			{
				int temp = order[i];

				order[i] = order[j];
				order[j] = temp;
			}

	bitmap_fill(bitmap, cliprect, screen->machine->pens[0]);

	for (i = 0; i < NUM_LAYERS + 1; i++)
	{
		int layer = order[i];

		if (layer == NUM_LAYERS)
		{
			if (enables & K55_INP_SUB2)
				draw_sprites(screen->machine, bitmap, cliprect);
		}
		else
		{
			if (enables & (K55_INP_VRAM_A << layer))
				k056832_tilemap_draw_dj(k056832, bitmap, cliprect, layer, 0, 1 << i);
		}
	}
	return 0;
}
