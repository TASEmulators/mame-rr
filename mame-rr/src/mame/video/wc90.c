#include "emu.h"
#include "includes/wc90.h"


UINT8 *wc90_fgvideoram,*wc90_bgvideoram,*wc90_txvideoram;


UINT8 *wc90_scroll0xlo, *wc90_scroll0xhi;
UINT8 *wc90_scroll1xlo, *wc90_scroll1xhi;
UINT8 *wc90_scroll2xlo, *wc90_scroll2xhi;

UINT8 *wc90_scroll0ylo, *wc90_scroll0yhi;
UINT8 *wc90_scroll1ylo, *wc90_scroll1yhi;
UINT8 *wc90_scroll2ylo, *wc90_scroll2yhi;


static tilemap_t *tx_tilemap,*fg_tilemap,*bg_tilemap;



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = wc90_bgvideoram[tile_index];
	int tile = wc90_bgvideoram[tile_index + 0x800] +
					256 * ((attr & 3) + ((attr >> 1) & 4));
	SET_TILE_INFO(
			2,
			tile,
			attr >> 4,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int attr = wc90_fgvideoram[tile_index];
	int tile = wc90_fgvideoram[tile_index + 0x800] +
					256 * ((attr & 3) + ((attr >> 1) & 4));
	SET_TILE_INFO(
			1,
			tile,
			attr >> 4,
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	SET_TILE_INFO(
			0,
			wc90_txvideoram[tile_index + 0x800] + ((wc90_txvideoram[tile_index] & 0x07) << 8),
			wc90_txvideoram[tile_index] >> 4,
			0);
}

static TILE_GET_INFO( track_get_bg_tile_info )
{
	int attr = wc90_bgvideoram[tile_index];
	int tile = wc90_bgvideoram[tile_index + 0x800] +
					256 * (attr & 7);
	SET_TILE_INFO(
			2,
			tile,
			attr >> 4,
			0);
}

static TILE_GET_INFO( track_get_fg_tile_info )
{
	int attr = wc90_fgvideoram[tile_index];
	int tile = wc90_fgvideoram[tile_index + 0x800] +
					256 * (attr & 7);
	SET_TILE_INFO(
			1,
			tile,
			attr >> 4,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( wc90 )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,     16,16,64,32);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,16,16,64,32);
	tx_tilemap = tilemap_create(machine, get_tx_tile_info,tilemap_scan_rows, 8, 8,64,32);

	tilemap_set_transparent_pen(fg_tilemap,0);
	tilemap_set_transparent_pen(tx_tilemap,0);
}

VIDEO_START( wc90t )
{
	bg_tilemap = tilemap_create(machine, track_get_bg_tile_info,tilemap_scan_rows,     16,16,64,32);
	fg_tilemap = tilemap_create(machine, track_get_fg_tile_info,tilemap_scan_rows,16,16,64,32);
	tx_tilemap = tilemap_create(machine, get_tx_tile_info,tilemap_scan_rows, 8, 8,64,32);

	tilemap_set_transparent_pen(fg_tilemap,0);
	tilemap_set_transparent_pen(tx_tilemap,0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( wc90_bgvideoram_w )
{
	wc90_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x7ff);
}

WRITE8_HANDLER( wc90_fgvideoram_w )
{
	wc90_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset & 0x7ff);
}

WRITE8_HANDLER( wc90_txvideoram_w )
{
	wc90_txvideoram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap,offset & 0x7ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

#define WC90_DRAW_SPRITE( code, sx, sy ) \
					drawgfx_transpen( bitmap, cliprect, machine->gfx[3], code, flags >> 4, \
					bank&1, bank&2, sx, sy, 0 )

static const char p32x32[4][4] = {
	{ 0, 1, 2, 3 },
	{ 1, 0, 3, 2 },
	{ 2, 3, 0, 1 },
	{ 3, 2, 1, 0 }
};

static const char p32x64[4][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 5, 4, 7, 6, 1, 0, 3, 2 },
	{ 2, 3, 0, 1, 6, 7, 4, 5 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 }
};

static const char p64x32[4][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 1, 0, 3, 2, 5, 4, 7, 6 },
	{ 6, 7, 4, 5, 2, 3, 0, 1 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 }
};

static const char p64x64[4][16] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 5, 4, 7, 6, 1, 0, 3, 2, 13, 12, 15, 14, 9, 8, 11, 10 },
	{ 10, 11, 8, 9, 14, 15, 12, 13, 2, 3, 0, 1, 6, 7, 4, 5 },
	{ 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 }
};

static void draw_sprite_16x16(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int code,
							  int sx, int sy, int bank, int flags ) {
	WC90_DRAW_SPRITE( code, sx, sy );
}

static void draw_sprite_16x32(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int code,
							  int sx, int sy, int bank, int flags ) {
	if ( bank & 2 ) {
		WC90_DRAW_SPRITE( code+1, sx, sy+16 );
		WC90_DRAW_SPRITE( code, sx, sy );
	} else {
		WC90_DRAW_SPRITE( code, sx, sy );
		WC90_DRAW_SPRITE( code+1, sx, sy+16 );
	}
}

static void draw_sprite_16x64(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int code,
							  int sx, int sy, int bank, int flags ) {
	if ( bank & 2 ) {
		WC90_DRAW_SPRITE( code+3, sx, sy+48 );
		WC90_DRAW_SPRITE( code+2, sx, sy+32 );
		WC90_DRAW_SPRITE( code+1, sx, sy+16 );
		WC90_DRAW_SPRITE( code, sx, sy );
	} else {
		WC90_DRAW_SPRITE( code, sx, sy );
		WC90_DRAW_SPRITE( code+1, sx, sy+16 );
		WC90_DRAW_SPRITE( code+2, sx, sy+32 );
		WC90_DRAW_SPRITE( code+3, sx, sy+48 );
	}
}

static void draw_sprite_32x16(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int code,
							  int sx, int sy, int bank, int flags ) {
	if ( bank & 1 ) {
		WC90_DRAW_SPRITE( code+1, sx+16, sy );
		WC90_DRAW_SPRITE( code, sx, sy );
	} else {
		WC90_DRAW_SPRITE( code, sx, sy );
		WC90_DRAW_SPRITE( code+1, sx+16, sy );
	}
}

static void draw_sprite_32x32(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int code,
							  int sx, int sy, int bank, int flags ) {

	const char *p = p32x32[ bank&3 ];

	WC90_DRAW_SPRITE( code+p[0], sx, sy );
	WC90_DRAW_SPRITE( code+p[1], sx+16, sy );
	WC90_DRAW_SPRITE( code+p[2], sx, sy+16 );
	WC90_DRAW_SPRITE( code+p[3], sx+16, sy+16 );
}

static void draw_sprite_32x64(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int code,
							  int sx, int sy, int bank, int flags ) {

	const char *p = p32x64[ bank&3 ];

	WC90_DRAW_SPRITE( code+p[0], sx, sy );
	WC90_DRAW_SPRITE( code+p[1], sx+16, sy );
	WC90_DRAW_SPRITE( code+p[2], sx, sy+16 );
	WC90_DRAW_SPRITE( code+p[3], sx+16, sy+16 );
	WC90_DRAW_SPRITE( code+p[4], sx, sy+32 );
	WC90_DRAW_SPRITE( code+p[5], sx+16, sy+32 );
	WC90_DRAW_SPRITE( code+p[6], sx, sy+48 );
	WC90_DRAW_SPRITE( code+p[7], sx+16, sy+48 );
}

static void draw_sprite_64x16(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int code,
							  int sx, int sy, int bank, int flags ) {
	if ( bank & 1 ) {
		WC90_DRAW_SPRITE( code+3, sx+48, sy );
		WC90_DRAW_SPRITE( code+2, sx+32, sy );
		WC90_DRAW_SPRITE( code+1, sx+16, sy );
		WC90_DRAW_SPRITE( code, sx, sy );
	} else {
		WC90_DRAW_SPRITE( code, sx, sy );
		WC90_DRAW_SPRITE( code+1, sx+16, sy );
		WC90_DRAW_SPRITE( code+2, sx+32, sy );
		WC90_DRAW_SPRITE( code+3, sx+48, sy );
	}
}

static void draw_sprite_64x32(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int code,
							  int sx, int sy, int bank, int flags ) {

	const char *p = p64x32[ bank&3 ];

	WC90_DRAW_SPRITE( code+p[0], sx, sy );
	WC90_DRAW_SPRITE( code+p[1], sx+16, sy );
	WC90_DRAW_SPRITE( code+p[2], sx, sy+16 );
	WC90_DRAW_SPRITE( code+p[3], sx+16, sy+16 );
	WC90_DRAW_SPRITE( code+p[4], sx+32, sy );
	WC90_DRAW_SPRITE( code+p[5], sx+48, sy );
	WC90_DRAW_SPRITE( code+p[6], sx+32, sy+16 );
	WC90_DRAW_SPRITE( code+p[7], sx+48, sy+16 );
}

static void draw_sprite_64x64(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int code,
							  int sx, int sy, int bank, int flags ) {

	const char *p = p64x64[ bank&3 ];

	WC90_DRAW_SPRITE( code+p[0], sx, sy );
	WC90_DRAW_SPRITE( code+p[1], sx+16, sy );
	WC90_DRAW_SPRITE( code+p[2], sx, sy+16 );
	WC90_DRAW_SPRITE( code+p[3], sx+16, sy+16 );
	WC90_DRAW_SPRITE( code+p[4], sx+32, sy );
	WC90_DRAW_SPRITE( code+p[5], sx+48, sy );
	WC90_DRAW_SPRITE( code+p[6], sx+32, sy+16 );
	WC90_DRAW_SPRITE( code+p[7], sx+48, sy+16 );

	WC90_DRAW_SPRITE( code+p[8], sx, sy+32 );
	WC90_DRAW_SPRITE( code+p[9], sx+16, sy+32 );
	WC90_DRAW_SPRITE( code+p[10], sx, sy+48 );
	WC90_DRAW_SPRITE( code+p[11], sx+16, sy+48 );
	WC90_DRAW_SPRITE( code+p[12], sx+32, sy+32 );
	WC90_DRAW_SPRITE( code+p[13], sx+48, sy+32 );
	WC90_DRAW_SPRITE( code+p[14], sx+32, sy+48 );
	WC90_DRAW_SPRITE( code+p[15], sx+48, sy+48 );
}

static void draw_sprite_invalid(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int code,
											int sx, int sy, int bank, int flags ) {
	logerror("8 pixel sprite size not supported\n" );
}

typedef void (*draw_sprites_func)(running_machine *, bitmap_t *, const rectangle *, int, int, int, int, int );

static const draw_sprites_func draw_sprites_proc[16] = {
	draw_sprite_invalid,	/* 0000 = 08x08 */
	draw_sprite_invalid,	/* 0001 = 16x08 */
	draw_sprite_invalid,	/* 0010 = 32x08 */
	draw_sprite_invalid,	/* 0011 = 64x08 */
	draw_sprite_invalid,	/* 0100 = 08x16 */
	draw_sprite_16x16,		/* 0101 = 16x16 */
	draw_sprite_32x16,		/* 0110 = 32x16 */
	draw_sprite_64x16,		/* 0111 = 64x16 */
	draw_sprite_invalid,	/* 1000 = 08x32 */
	draw_sprite_16x32,		/* 1001 = 16x32 */
	draw_sprite_32x32,		/* 1010 = 32x32 */
	draw_sprite_64x32,		/* 1011 = 64x32 */
	draw_sprite_invalid,	/* 1100 = 08x64 */
	draw_sprite_16x64,		/* 1101 = 16x64 */
	draw_sprite_32x64,		/* 1110 = 32x64 */
	draw_sprite_64x64		/* 1111 = 64x64 */
};

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs, sx,sy, flags, which;

	/* draw all visible sprites of specified priority */
	for (offs = 0;offs < machine->generic.spriteram_size;offs += 16){
		int bank = spriteram[offs+0];

		if ( ( bank >> 4 ) == priority ) {

			if ( bank & 4 ) { /* visible */
				which = ( spriteram[offs+2] >> 2 ) + ( spriteram[offs+3] << 6 );

				sx = spriteram[offs + 8] + ( (spriteram[offs + 9] & 3 ) << 8 );
				sy = spriteram[offs + 6] + ( (spriteram[offs + 7] & 1 ) << 8 );

				if (sx >= 0x0300) sx -= 0x0400;

				flags = spriteram[offs+4];
				( *( draw_sprites_proc[ flags & 0x0f ] ) )(machine, bitmap,cliprect, which, sx, sy, bank, flags );
			}
		}
	}
}

#undef WC90_DRAW_SPRITE


VIDEO_UPDATE( wc90 )
{
	tilemap_set_scrollx(bg_tilemap,0,wc90_scroll2xlo[0] + 256 * wc90_scroll2xhi[0]);
	tilemap_set_scrolly(bg_tilemap,0,wc90_scroll2ylo[0] + 256 * wc90_scroll2yhi[0]);
	tilemap_set_scrollx(fg_tilemap,0,wc90_scroll1xlo[0] + 256 * wc90_scroll1xhi[0]);
	tilemap_set_scrolly(fg_tilemap,0,wc90_scroll1ylo[0] + 256 * wc90_scroll1yhi[0]);
	tilemap_set_scrollx(tx_tilemap,0,wc90_scroll0xlo[0] + 256 * wc90_scroll0xhi[0]);
	tilemap_set_scrolly(tx_tilemap,0,wc90_scroll0ylo[0] + 256 * wc90_scroll0yhi[0]);

//  draw_sprites(screen->machine, bitmap,cliprect, 3 );
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(screen->machine, bitmap,cliprect, 2 );
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	draw_sprites(screen->machine, bitmap,cliprect, 1 );
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	draw_sprites(screen->machine, bitmap,cliprect, 0 );
	return 0;
}
