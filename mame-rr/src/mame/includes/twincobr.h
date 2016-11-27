/***************************************************************************
        Twincobr/Flying Shark/Wardner  game hardware from 1986-1987
        -----------------------------------------------------------
****************************************************************************/


#include "video/mc6845.h"


/*----------- defined in drivers/wardner.c -----------*/

extern STATE_POSTLOAD( wardner_restore_bank );

/*----------- defined in machine/twincobr.c -----------*/

INTERRUPT_GEN( twincobr_interrupt );
INTERRUPT_GEN( wardner_interrupt );

WRITE16_HANDLER( twincobr_dsp_addrsel_w );
READ16_HANDLER(  twincobr_dsp_r );
WRITE16_HANDLER( twincobr_dsp_w );
WRITE16_HANDLER( twincobr_dsp_bio_w );
READ16_HANDLER ( twincobr_BIO_r );
WRITE16_HANDLER( twincobr_control_w );
READ16_HANDLER(  twincobr_sharedram_r );
WRITE16_HANDLER( twincobr_sharedram_w );
WRITE8_HANDLER(   twincobr_coin_w );
READ16_HANDLER(  fsharkbt_dsp_r );
WRITE16_HANDLER( fsharkbt_dsp_w );
WRITE16_HANDLER( fshark_coin_dsp_w );
WRITE16_HANDLER( wardner_dsp_addrsel_w );
READ16_HANDLER(  wardner_dsp_r );
WRITE16_HANDLER( wardner_dsp_w );
WRITE8_HANDLER(   wardner_control_w );
WRITE8_HANDLER(   wardner_coin_dsp_w );

MACHINE_RESET( twincobr );
MACHINE_RESET( fsharkbt );
MACHINE_RESET( wardner );

extern void twincobr_driver_savestate(running_machine *machine);
extern void wardner_driver_savestate(running_machine *machine);

extern int toaplan_main_cpu;	/* Main CPU type.  0 = 68000, 1 = Z80 */
extern int wardner_membank;

extern UINT8 *twincobr_sharedram;


/*----------- defined in video/twincobr.c -----------*/
extern const mc6845_interface twincobr_mc6845_intf;

extern void twincobr_flipscreen(running_machine *machine, int flip);
extern void twincobr_display(int enable);

READ16_HANDLER(  twincobr_txram_r );
READ16_HANDLER(  twincobr_bgram_r );
READ16_HANDLER(  twincobr_fgram_r );
WRITE16_HANDLER( twincobr_txram_w );
WRITE16_HANDLER( twincobr_bgram_w );
WRITE16_HANDLER( twincobr_fgram_w );
WRITE16_HANDLER( twincobr_txscroll_w );
WRITE16_HANDLER( twincobr_bgscroll_w );
WRITE16_HANDLER( twincobr_fgscroll_w );
WRITE16_HANDLER( twincobr_exscroll_w );
WRITE16_HANDLER( twincobr_txoffs_w );
WRITE16_HANDLER( twincobr_bgoffs_w );
WRITE16_HANDLER( twincobr_fgoffs_w );
WRITE8_HANDLER( wardner_videoram_w );
READ8_HANDLER(  wardner_videoram_r );
WRITE8_HANDLER( wardner_bglayer_w );
WRITE8_HANDLER( wardner_fglayer_w );
WRITE8_HANDLER( wardner_txlayer_w );
WRITE8_HANDLER( wardner_bgscroll_w );
WRITE8_HANDLER( wardner_fgscroll_w );
WRITE8_HANDLER( wardner_txscroll_w );
WRITE8_HANDLER( wardner_exscroll_w );
READ8_HANDLER(  wardner_sprite_r );
WRITE8_HANDLER( wardner_sprite_w );

extern INT32 twincobr_fg_rom_bank;
extern INT32 twincobr_bg_ram_bank;
extern INT32 wardner_sprite_hack;

VIDEO_START( toaplan0 );
VIDEO_UPDATE( toaplan0 );
VIDEO_EOF( toaplan0 );
