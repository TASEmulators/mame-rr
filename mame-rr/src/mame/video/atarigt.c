/***************************************************************************

    Atari GT hardware

*****************************************************************************

    MO data has 12 bits total: MVID0-11
    MVID9-11 form the priority
    MVID0-9 form the color bits

    PF data has 13 bits total: PF.VID0-12
    PF.VID10-12 form the priority
    PF.VID0-9 form the color bits

    Upper bits come from the low 5 bits of the HSCROLL value in alpha RAM
    Playfield bank comes from low 2 bits of the VSCROLL value in alpha RAM
    For GX2, there are 4 bits of bank

****************************************************************************/


#include "emu.h"
#include "machine/atarigen.h"
#include "includes/atarigt.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define RSHIFT		16
#define GSHIFT		8
#define BSHIFT		0



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	atarigt_state *state = (atarigt_state *)machine->driver_data;
	UINT16 data = state->atarigen.alpha32[tile_index / 2] >> (16 * (~tile_index & 1));
	int code = data & 0xfff;
	int color = (data >> 12) & 0x0f;
	SET_TILE_INFO(1, code, color, 0);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	atarigt_state *state = (atarigt_state *)machine->driver_data;
	UINT16 data = state->atarigen.playfield32[tile_index / 2] >> (16 * (~tile_index & 1));
	int code = (state->playfield_tile_bank << 12) | (data & 0xfff);
	int color = (data >> 12) & 7;
	SET_TILE_INFO(0, code, color, (data >> 15) & 1);
}


static TILEMAP_MAPPER( atarigt_playfield_scan )
{
	int bank = 1 - (col / (num_cols / 2));
	return bank * (num_rows * num_cols / 2) + row * (num_cols / 2) + (col % (num_cols / 2));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( atarigt )
{
	static const atarirle_desc modesc =
	{
		"gfx3",		/* region where the GFX data lives */
		256,		/* number of entries in sprite RAM */
		0,			/* left clip coordinate */
		0,			/* right clip coordinate */

		0x0000,		/* base palette entry */
		0x1000,		/* maximum number of colors */

		{{ 0x7fff,0,0,0,0,0,0,0 }},	/* mask for the code index */
		{{ 0,0x0ff0,0,0,0,0,0,0 }},	/* mask for the color */
		{{ 0,0,0xffc0,0,0,0,0,0 }},	/* mask for the X position */
		{{ 0,0,0,0xffc0,0,0,0,0 }},	/* mask for the Y position */
		{{ 0,0,0,0,0xffff,0,0,0 }},	/* mask for the scale factor */
		{{ 0x8000,0,0,0,0,0,0,0 }},	/* mask for the horizontal flip */
		{{ 0,0,0,0,0,0,0x00ff,0 }},	/* mask for the order */
		{{ 0,0x0e00,0,0,0,0,0,0 }},	/* mask for the priority */
		{{ 0,0x8000,0,0,0,0,0,0 }}	/* mask for the VRAM target */
	};
	atarigt_state *state = (atarigt_state *)machine->driver_data;
	atarirle_desc adjusted_modesc = modesc;
	pen_t *substitute_pens;
	int i, width, height;

	/* blend the playfields and free the temporary one */
	atarigen_blend_gfx(machine, 0, 2, 0x0f, 0x30);

	/* initialize the playfield */
	state->atarigen.playfield_tilemap = tilemap_create(machine, get_playfield_tile_info, atarigt_playfield_scan,  8,8, 128,64);

	/* initialize the motion objects */
	atarirle_init(machine, 0, &adjusted_modesc);

	/* initialize the alphanumerics */
	state->atarigen.alpha_tilemap = tilemap_create(machine, get_alpha_tile_info, tilemap_scan_rows,  8,8, 64,32);

	/* allocate temp bitmaps */
	width = machine->primary_screen->width();
	height = machine->primary_screen->height();

	state->pf_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16);
	state->an_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16);

	/* map pens 1:1 */
	substitute_pens = auto_alloc_array(machine, pen_t, 65536);
	for (i = 0; i < machine->total_colors(); i++)
		substitute_pens[i] = i;
	machine->pens = substitute_pens;

	/* reset statics */
	memset(state->colorram, 0, 0x80000);

	/* save states */
	state_save_register_global(machine, state->playfield_tile_bank);
	state_save_register_global(machine, state->playfield_color_bank);
	state_save_register_global(machine, state->playfield_xscroll);
	state_save_register_global(machine, state->playfield_yscroll);
	state_save_register_global(machine, state->tram_checksum);
	state_save_register_global_array(machine, state->expanded_mram);
}



/*************************************
 *
 *  Color RAM access
 *
 *************************************/

void atarigt_colorram_w(atarigt_state *state, offs_t address, UINT16 data, UINT16 mem_mask)
{
	UINT16 olddata;

	/* update the raw data */
	address = (address & 0x7ffff) / 2;
	olddata = state->colorram[address];
	COMBINE_DATA(&state->colorram[address]);

	/* update the TRAM checksum */
	if (address >= 0x10000 && address < 0x14000)
		state->tram_checksum += state->colorram[address] - olddata;

	/* update expanded MRAM */
	else if (address >= 0x20000 && address < 0x28000)
	{
		state->expanded_mram[0 * MRAM_ENTRIES + (address & 0x7fff)] = (state->colorram[address] >> 8) << RSHIFT;
		state->expanded_mram[1 * MRAM_ENTRIES + (address & 0x7fff)] = (state->colorram[address] & 0xff) << GSHIFT;
	}
	else if (address >= 0x30000 && address < 0x38000)
		state->expanded_mram[2 * MRAM_ENTRIES + (address & 0x7fff)] = (state->colorram[address] & 0xff) << BSHIFT;
}


UINT16 atarigt_colorram_r(atarigt_state *state, offs_t address)
{
	address &= 0x7ffff;
	return state->colorram[address / 2];
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

void atarigt_scanline_update(screen_device &screen, int scanline)
{
	atarigt_state *state = (atarigt_state *)screen.machine->driver_data;
	UINT32 *base = &state->atarigen.alpha32[(scanline / 8) * 32 + 24];
	int i;

	/* keep in range */
	if (base >= &state->atarigen.alpha32[0x400])
		return;

	/* update the playfield scrolls */
	for (i = 0; i < 8; i++)
	{
		UINT32 word = *base++;

		if (word & 0x80000000)
		{
			int newscroll = (word >> 21) & 0x3ff;
			int newbank = (word >> 16) & 0x1f;
			if (newscroll != state->playfield_xscroll)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				tilemap_set_scrollx(state->atarigen.playfield_tilemap, 0, newscroll);
				state->playfield_xscroll = newscroll;
			}
			if (newbank != state->playfield_color_bank)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				tilemap_set_palette_offset(state->atarigen.playfield_tilemap, (newbank & 0x1f) << 8);
				state->playfield_color_bank = newbank;
			}
		}

		if (word & 0x00008000)
		{
			int newscroll = ((word >> 6) - (scanline + i)) & 0x1ff;
			int newbank = word & 15;
			if (newscroll != state->playfield_yscroll)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				tilemap_set_scrolly(state->atarigen.playfield_tilemap, 0, newscroll);
				state->playfield_yscroll = newscroll;
			}
			if (newbank != state->playfield_tile_bank)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				tilemap_mark_all_tiles_dirty(state->atarigen.playfield_tilemap);
				state->playfield_tile_bank = newbank;
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

/*

    How it works:

        Incoming data from AN = AN.VID0-7
        Incoming data from PF = PF.VID0-12
        Incoming data from MO = MVID0-11
        Incoming data from TMO = TVID0-11

    !MGEP = 1 if:
        MVID9-11 < PF.VID10-12
        or (PF.VID12 and R188 is installed [yes on T-Mek & Rage])
    !MGEP = 0 if:
        R177 is installed [no on T-Mek & Rage]

    First, the CRAM:

        GAL @ 13M, takes as input:
            ANZ (AN.VID0-3 == 0)
            MOZ (MVID0-5 == 0)
            PFZ (PF.VID0-5 == 0)
            MVID10
            MVID11
            PF.VID10
            PF.VID11
            AN.VID7
            !MGEP
        And outputs:
            CRA10-12
            CRMUXA-B

        Index to the CRAM:
            CRA13  = LATCH & 0x08
            CRA12 \
            CRA11  = output from GAL
            CRA10 /
            CRA9 \
            CRA8  |
            CRA7  |
            CRA6  |
            CRA5   = output from MUX, as selected by GAL, either PF.VID0-9, MVID0-9, or AN.VID0-7
            CRA4  |
            CRA3  |
            CRA2  |
            CRA1  |
            CRA0 /
        Output from CRAM (16 bits):
            /TLEN
            CRR0-4
            CRG0-4
            CRB0-4

    Next, the TRAM:

        GAL @ 14K, takes as input:
            ANZ (AN.VID0-3 == 0)
            MOZ (MVID0-5 == 0)
            PFZ (PF.VID0-5 == 0)
            TVID8
            TVID9
            TVID10
            PF.VID10
            PF.VID11
            AN.VID7
            !MGEP
            !68.TRAN
        And outputs:
            TRA8-11
            TRMUXA-B

        Index to the TRAM:
            TRA13  = LATCH & 0x20
            TRA12  = LATCH & 0x10
            TRA11 \
            TRA10  = output from GAL
            TRA9   |
            TRA8  /
            TRA7  \
            TRA6  |
            TRA5  |
            TRA4   = output from MUX, as selected by GAL, either PF.VID0-7, MVID0-7, or AN.VID0-7
            TRA3  |
            TRA2  |
            TRA1  |
            TRA0 /
        Output from TRAM (16 bits):
            TLPRI
            TRR0-4
            TRG0-4
            TRB0-4

    Finally, the MRAM:

        GAL @ 5F, takes as input:
            TLPRI
            !TLEN
            PFZ
            PF.VID12
            TVID9-11
        And outputs:
            TLDIS
            (PFPRI)
            (TLPRID)
            enable for CRAM latches
            MRA10-12

        The CRAM outputs are gated by the enable signal from 5F; it outputs
            BMRA0-4
            GMRA0-4
            RMRA0-4

        The TRAM outputs are gated by the TLDIS signal from 5F
            BMRA5-9
            GMRA5-9
            RMRA5-9

        Index to MRAM:
            MRA14  = LATCH & 0x80
            MRA13  = LATCH & 0x40
            MRA12 \
            MRA11  = output from GAL
            MRA10 /
            MRA9 \
            MRA8  |
            MRA7   = TRAM output
            MRA6  |
            MRA5 /
            MRA4 \
            MRA3  |
            MRA2   = CRAM output
            MRA1  |
            MRA0 /

    And even beyond that:

        LATCH & 0x04 -> LCRD2
        LATCH & 0x02 -> LCRD1
        LATCH & 0x01 -> LCRD0

        GAL @ 22E, takes as input:
            LCRD0-2
            PF.VID12
            PFZ
            ANZ
            MVZ

        And outputs SUMRED, SUMGRN, SUMBLU,
        which bypass everything!

-------------------------------------------------

TMEK GALs:

    13M:
        CRA12=68.A13*!7M3                           -- when writing with 68020's A13 == 1
           +MGEP*!AN.VID7*ANZ*!MVZ*7M3              -- !opaque_alpha && apix==0 && mopix!=0 && pfpix==0
           +PFZ*!AN.VID7*ANZ*!MVZ*7M3               -- !opaque_alpha && apix==0 && mopix!=0 && mopri>=pfpri

        CRA11=68.A12*!7M3                           -- when writing with 68020's A12 == 1
           +MGEP*!AN.VID7*ANZ*!MVZ*MVID11*7M3       -- !opaque_alpha && apix==0 && mopix!=0 && mopri>=pfpri && mopix11!=0
           +PFZ*!AN.VID7*ANZ*!MVZ*MVID11*7M3        -- !opaque_alpha && apix==0 && mopix!=0 && pfpix==0 && mopix11!=0
           +!AN.VID7*ANZ*MVZ*PF.VID11*7M3           -- !opaque_alpha && apix==0 && mopix==0 && pfpix11!=0
           +!MGEP*!PFZ*!AN.VID7*ANZ*PF.VID11*7M3    -- !opaque_alpha && apix==0 && pfpix!=0 && mopri<pfpri && pfpix11!=0

        CRA10=68.A11*!7M3                           -- when writing with 68020's A11 == 1
           +MGEP*!AN.VID7*ANZ*!MVZ*MVID10*7M3       -- !opaque_alpha && apix==0 && mopix!=0 && mopri>=pfpri && mopix10!=0
           +PFZ*!AN.VID7*ANZ*!MVZ*MVID10*7M3        -- !opaque_alpha && apix==0 && mopix!=0 && pfpix==0 && mopix10!=0
           +!AN.VID7*ANZ*MVZ*PF.VID10*7M3           -- !opaque_alpha && apix==0 && mopix==0 && pfpix10!=0
           +!MGEP*!PFZ*!AN.VID7*ANZ*PF.VID10*7M3    -- !opaque_alpha && apix==0 && pfpix!=0 && mopri<pfpri && pfpix10!=0

        CRMUXB=!AN.VID7*ANZ*7M3                     -- !opaque_alpha && apix==0

        !CRMUXA=!7M3
            +MGEP*!AN.VID7*ANZ*!MVZ
            +PFZ*!AN.VID7*ANZ*!MVZ


    14K:
        TRA11=68.A12*!7M1                   -- when writing with 68020's A12 == 1

        TRA10=68.A11*!7M1                   -- when writing with 68020's A11 == 1
           +!AN.VID7*ANZ*PFZ*7M1*!MVZ       -- !opaque_alpha && apix==0 && pfpix==0 && mopix!=0
           +MGEP*!AN.VID7*ANZ*7M1*!MVZ      -- !opaque_alpha && apix==0 && mopri>=pfpri && mopix!=0

        TRA9=68.A10*!7M1                    -- when writing with 68020's A10 == 1
           +!AN.VID7*ANZ*TVID9*7M1          -- !opaque_alpha && apix==0 && tvid9==1

        TRA8=68.A9*!7M1                     -- when writing with 68020's A9 == 1
           +!AN.VID7*ANZ*TVID8*7M1          -- !opaque_alpha && apix==0 && tvid8==1

        TRMUXB=7M1                          -- 1
        TRMUXA=GND                          -- 0

    5F:
        MRA12:=TVID11
        MRA11:=TVID10
        MRA10:=TVID9
        (PFPRI):=PF.VID12

        !TLDIS=PFZ*TLEN                     -- enabled if pfpix==0 && tlen
            +!(PFPRI)*TLEN                  -- or if pf.vid12==0 && tlen

        !(CRADIS)=!(PFPRI)*TLPRI            -- enabled if pfvid.12==0 && tlpri
        (TLPRID):=TLPRI

    22E:
        LCRD0 LCRD1 LCRD2 MVZ ANZ PFZ PF.VID12 LLD3 LLA3 GND J SUMBLU1K SUMGRN1K SUMRED1K (OE2) (OE1) SUMBLU SUMGRN SUMRED VCC
        SUMRED.OE=(OE1)
        !SUMRED=!SUMGRN
        SUMGRN.OE=(OE1)
        SUMGRN=!LCRD1*LCRD2*LLA3
           +LCRD1*LCRD0*LCRD2*J
           +LCRD1*LCRD0*!LCRD2*!LLD3
           +!LCRD1*LCRD0*!LCRD2*LLD3
           +!LCRD0*LCRD2*LLA3
           +LCRD1*!LCRD0*!LCRD2*LLD3
        SUMBLU.OE=(OE1)
        !SUMBLU=!SUMGRN

        (OE1)=!LCRD1*LCRD2*!PF.VID12*LLD3*J             -- (LCR & 6)==4 && LLD3 && !PF.VID12    [LCR==4 || LCR==5]
           +!LCRD1*LCRD2*PFZ*LLD3*J                     -- (LCR & 6)==4 && LLD3 && PFZ          [LCR==4 || LCR==5]
           +!LCRD0*LCRD2*!PF.VID12*LLD3                 -- (LCR & 5)==4 && LLD3 && !PF.VID12    [LCR==4 || LCR==6]
           +!LCRD0*LCRD2*PFZ*LLD3                       -- (LCR & 5)==4 && LLD3 && PFZ          [LCR==4 || LCR==6]
           +!LCRD1*!LCRD0*LCRD2*!PF.VID12*J             -- LCR==4 && !PF.VID12                  [LCR==4]
           +!LCRD1*!LCRD0*LCRD2*PFZ*J                   -- LCR==4 && PFZ                        [LCR==4]

        (OE2)=LCRD1*LCRD0*!LCRD2*!PF.VID12*LLA3*J       -- LCR==3 && LLA3 && !PF.VID12          [LCR==3]
           +LCRD1*LCRD0*!LCRD2*PFZ*LLA3*J               -- LCR==3 && LLA3 && PFZ                [LCR==3]
           +!LCRD1*LCRD0*!LCRD2*!MVZ*!PF.VID12*J        -- LCR==1 && !MVZ && !PF.VID12          [LCR==1]
           +!LCRD1*LCRD0*!LCRD2*!MVZ*PFZ*J              -- LCR==1 && !MVZ && PFZ                [LCR==1]
           +LCRD1*!LCRD0*LCRD2*!PF.VID12*LLD3           -- LCR==6 && LLD3 && !PF.VID12          [LCR==6]
           +LCRD1*!LCRD0*LCRD2*PFZ*LLD3                 -- LCR==6 && LLD3 && PFZ                [LCR==6]

        SUMRED1K.OE=(OE2)
        !SUMRED1K=!LCRD1*!LCRD2*!LLD3                   -- (LCR & 6)==0 && !LLD3
            +LCRD0*LCRD2*!LLA3                          -- (LCR & 5)==5 && !LLA3
            +LCRD1*LCRD0*!LCRD2*LLD3                    -- LCR==3 && LLD3
            +!LCRD0*!LCRD2*!LLD3                        -- (LCR & 5)==0 && !LLD3
            +LCRD1*!LCRD0*LCRD2*!J                      -- LCR==6 && !J
            +!LCRD1*!LCRD0*LLD3                         -- (LCR & 3)==0 && LLD3
        SUMGRN1K.OE=(OE2)
        !SUMGRN1K=!SUMRED1K
        SUMBLU1K.OE=(OE2)
        !SUMBLU1K=!SUMRED1K

-------------------------------------------------

PrimRage GALs:

    13M:
        CRA12=68.A13*!7M3                                   -- when writing with 68020's A13 == 1
           +!AN.VID7*ANZ*!MVZ*MVID11*7M3                    -- !opaque_alpha && apix==0 && mopix!=0 && mvid11!=0
           +MGEP*!AN.VID7*ANZ*!MVZ*7M3                      -- !opaque_alpha && apix==0 && mopix!=0 && pfpix==0
           +PFZ*!AN.VID7*ANZ*!MVZ*7M3                       -- !opaque_alpha && apix==0 && mopix!=0 && mopri>=pfpri

        CRA11=68.A12*!7M3                                   -- when writing with 68020's A12 == 1
           +!AN.VID7*ANZ*MVZ*PF.VID11*7M3                   -- !opaque_alpha && apix==0 && mopix==0 && pfpix11!=0
           +!MGEP*!PFZ*!AN.VID7*ANZ*PF.VID11*!MVID11*7M3    -- !opaque_alpha && apix==0 && pfpix!=0 && mopri<pfpri && pfpix11!=0 && mvid11==0

        CRA10=68.A11*!7M3                                   -- when writing with 68020's A11 == 1
           +!AN.VID7*ANZ*MVZ*PF.VID10*7M3                   -- !opaque_alpha && apix==0 && mopix==0 && pfpix10!=0
           +!AN.VID7*ANZ*!MVZ*MVID11*MVID10*7M3             *- !opaque_alpha && apix==0 && mopix!=0 && mvid11 && mvid10
           +MGEP*!AN.VID7*ANZ*!MVZ*MVID10*7M3               -- !opaque_alpha && apix==0 && mopix!=0 && mopri>=pfpri && mopix10!=0
           +PFZ*!AN.VID7*ANZ*!MVZ*MVID10*7M3                -- !opaque_alpha && apix==0 && mopix!=0 && pfpix==0 && mopix10!=0
           +!MGEP*!PFZ*!AN.VID7*ANZ*PF.VID10*!MVID11*7M3    *- !opaque_alpha && apix==0 && pfpix!=0 && mopri<pfpri && mopix11==0

        CRMUXB=!AN.VID7*ANZ*7M3

        !CRMUXA=!7M3
            +!AN.VID7*ANZ*!MVZ*MVID11
            +MGEP*!AN.VID7*ANZ*!MVZ
            +PFZ*!AN.VID7*ANZ*!MVZ

*/


VIDEO_UPDATE( atarigt )
{
	atarigt_state *state = (atarigt_state *)screen->machine->driver_data;
	bitmap_t *mo_bitmap = atarirle_get_vram(0, 0);
	bitmap_t *tm_bitmap = atarirle_get_vram(0, 1);
	UINT16 *cram, *tram;
	int color_latch;
	UINT32 *mram;
	int x, y;

	/* draw the playfield */
	tilemap_draw(state->pf_bitmap, cliprect, state->atarigen.playfield_tilemap, 0, 0);

	/* draw the alpha layer */
	tilemap_draw(state->an_bitmap, cliprect, state->atarigen.alpha_tilemap, 0, 0);

	/* cache pointers */
	color_latch = state->colorram[0x30000/2];
	cram = (UINT16 *)&state->colorram[0x00000/2] + 0x2000 * ((color_latch >> 3) & 1);
	tram = (UINT16 *)&state->colorram[0x20000/2] + 0x1000 * ((color_latch >> 4) & 3);
	mram = state->expanded_mram + 0x2000 * ((color_latch >> 6) & 3);

	/* now do the nasty blend */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *an = (UINT16 *)state->an_bitmap->base + y * state->an_bitmap->rowpixels;
		UINT16 *pf = (UINT16 *)state->pf_bitmap->base + y * state->pf_bitmap->rowpixels;
		UINT16 *mo = (UINT16 *)mo_bitmap->base + y * mo_bitmap->rowpixels;
		UINT16 *tm = (UINT16 *)tm_bitmap->base + y * tm_bitmap->rowpixels;
		UINT32 *dst = (UINT32 *)bitmap->base + y * bitmap->rowpixels;

		/* Primal Rage: no TRAM, slightly different priorities */
		if (state->is_primrage)
		{
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				UINT8 pfpri = (pf[x] >> 10) & 7;
				UINT8 mopri = mo[x] >> ATARIRLE_PRIORITY_SHIFT;
				UINT8 mgep = (mopri >= pfpri) && !(pfpri & 4);
				UINT16 cra;
				UINT32 rgb;

				/* compute CRA -- unlike T-Mek, MVID11 enforces MO priority and is ignored */
				if (an[x] & 0x8f)
					cra = an[x] & 0xff;
				else if ((mo[x] & 0x3f) && ((mo[x] & 0x800) || mgep || !(pf[x] & 0x3f)))
					cra = 0x1000 | (mo[x] & 0x7ff);
				else
					cra = pf[x] & 0xfff;
				cra = cram[cra];

				/* compute the result */
				rgb  = mram[0 * MRAM_ENTRIES + ((cra >> 10) & 0x01f)];
				rgb |= mram[1 * MRAM_ENTRIES + ((cra >>  5) & 0x01f)];
				rgb |= mram[2 * MRAM_ENTRIES + ((cra >>  0) & 0x01f)];

				/* final override */
				if (color_latch & 7)
					if (!(pf[x] & 0x3f) || !(pf[x] & 0x2000))
						rgb = (0xff << RSHIFT) | (0xff << GSHIFT) | (0xff << BSHIFT);

				dst[x] = rgb;
			}
		}

		/* T-Mek: full TRAM and all effects */
		else
		{
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				UINT8 pfpri = (pf[x] >> 10) & 7;
				UINT8 mopri = mo[x] >> ATARIRLE_PRIORITY_SHIFT;
				UINT8 mgep = (mopri >= pfpri) && !(pfpri & 4);
				int no_tra = 0, no_cra = 0;
				UINT16 cra, tra, mra;
				UINT32 rgb;

				/* compute CRA/TRA */
				if (an[x] & 0x8f)
				{
					cra = an[x] & 0xff;
					tra = tm[x] & 0xff;
				}
				else if ((mo[x] & 0x3f) && (mgep || !(pf[x] & 0x3f)))
				{
					cra = 0x1000 | (mo[x] & 0xfff);
					tra = 0x400 | (tm[x] & 0x3ff);
				}
				else
				{
					cra = pf[x] & 0xfff;
					tra = tm[x] & 0x3ff;
				}
				cra = cram[cra];
				tra = tram[tra];

				/* compute MRA */
				mra = (tm[x] & 0xe00) << 1;

				/* turn off CRA/TRA as appropriate */
				if (!(pf[x] & 0x1000) && (tra & 0x8000))
					no_cra = 1;
				if (!(!(cra & 0x8000) && (!(pf[x] & 0x1000) || !(pf[x] & 0x3f))))
					no_tra = 1;
				if (no_cra)
					cra = 0;
				if (no_tra)
					tra = 0;

				/* compute the result */
				rgb  = mram[0 * MRAM_ENTRIES + mra + ((cra >> 10) & 0x01f) + ((tra >> 5) & 0x3e0)];
				rgb |= mram[1 * MRAM_ENTRIES + mra + ((cra >>  5) & 0x01f) + ((tra >> 0) & 0x3e0)];
				rgb |= mram[2 * MRAM_ENTRIES + mra + ((cra >>  0) & 0x01f) + ((tra << 5) & 0x3e0)];

				/* final override */
				if (color_latch & 7)
					if (!(pf[x] & 0x3f) || !(pf[x] & 0x2000))
						rgb = (0xff << RSHIFT) | (0xff << GSHIFT) | (0xff << BSHIFT);

				dst[x] = rgb;
			}
		}
	}
	return 0;
}
