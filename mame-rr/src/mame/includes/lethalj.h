/*************************************************************************

    The Game Room Lethal Justice hardware

**************************************************************************/

/*----------- defined in video/lethalj.c -----------*/

READ16_HANDLER( lethalj_gun_r );

VIDEO_START( lethalj );

WRITE16_HANDLER( lethalj_blitter_w );

void lethalj_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);
