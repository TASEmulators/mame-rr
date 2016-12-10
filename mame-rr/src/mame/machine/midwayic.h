/***************************************************************************

    Emulation of various Midway ICs

***************************************************************************/


/* 1st generation Midway serial PIC */
void midway_serial_pic_init(running_machine *machine, int upper);
void midway_serial_pic_reset_w(int state);
UINT8 midway_serial_pic_status_r(void);
UINT8 midway_serial_pic_r(const address_space *space);
void midway_serial_pic_w(const address_space *space, UINT8 data);


/* 2nd generation Midway serial/NVRAM/RTC PIC */
void midway_serial_pic2_init(running_machine *machine, int upper, int yearoffs);
void midway_serial_pic2_set_default_nvram(const UINT8 *nvram);
UINT8 midway_serial_pic2_status_r(const address_space *space);
UINT8 midway_serial_pic2_r(const address_space *space);
void midway_serial_pic2_w(const address_space *space, UINT8 data);
NVRAM_HANDLER( midway_serial_pic2 );


/* I/O ASIC connected to 2nd generation PIC */
void midway_ioasic_init(running_machine *machine, int shuffle, int upper, int yearoffs, void (*irq_callback)(running_machine *, int));
void midway_ioasic_set_auto_ack(int auto_ack);
void midway_ioasic_set_shuffle_state(int state);
void midway_ioasic_reset(running_machine *machine);
void midway_ioasic_fifo_w(running_machine *machine, UINT16 data);
void midway_ioasic_fifo_reset_w(running_machine *machine, int state);
void midway_ioasic_fifo_full_w(running_machine *machine, UINT16 data);
READ32_HANDLER( midway_ioasic_r );
WRITE32_HANDLER( midway_ioasic_w );
READ32_HANDLER( midway_ioasic_packed_r );
WRITE32_HANDLER( midway_ioasic_packed_w );

enum
{
	MIDWAY_IOASIC_STANDARD = 0,
	MIDWAY_IOASIC_BLITZ99,
	MIDWAY_IOASIC_CARNEVIL,
	MIDWAY_IOASIC_CALSPEED,
	MIDWAY_IOASIC_MACE,
	MIDWAY_IOASIC_GAUNTDL,
	MIDWAY_IOASIC_VAPORTRX,
	MIDWAY_IOASIC_SFRUSHRK,
	MIDWAY_IOASIC_HYPRDRIV
};



/* IDE ASIC maps the IDE registers */
READ32_DEVICE_HANDLER( midway_ide_asic_r );
WRITE32_DEVICE_HANDLER( midway_ide_asic_w );
