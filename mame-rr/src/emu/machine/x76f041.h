/*
 * x76f041.h
 *
 * Secure SerialFlash
 *
 */

#if !defined( X76F041_H )

#define X76F041_MAXCHIP ( 2 )

extern void x76f041_init( running_machine *machine, int chip, UINT8 *data );
extern void x76f041_cs_write( running_machine *machine, int chip, int cs );
extern void x76f041_rst_write( running_machine *machine, int chip, int rst );
extern void x76f041_scl_write( running_machine *machine, int chip, int scl );
extern void x76f041_sda_write( running_machine *machine, int chip, int sda );
extern int x76f041_sda_read( running_machine *machine, int chip );
extern NVRAM_HANDLER( x76f041_0 );
extern NVRAM_HANDLER( x76f041_1 );

#define X76F041_H
#endif
