/*********************************************************************

    generic.h

    Generic simple machine functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __MACHINE_GENERIC_H__
#define __MACHINE_GENERIC_H__



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* total # of coin counters */
#define COIN_COUNTERS			8

/* memory card actions */
#define MEMCARD_CREATE			0
#define MEMCARD_INSERT			1
#define MEMCARD_EJECT			2



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- initialization ----- */

/* set up all the common systems */
void generic_machine_init(running_machine *machine);



/* ----- tickets ----- */

/* return the number of tickets dispensed */
int get_dispensed_tickets(running_machine *machine);

/* increment the number of dispensed tickets */
void increment_dispensed_tickets(running_machine *machine, int delta);



/* ----- coin counters ----- */

/* write to a particular coin counter (clocks on active high edge) */
void coin_counter_w(running_machine *machine, int num, int on);

/* return the coin count for a given coin */
int coin_counter_get_count(running_machine *machine, int num);

/* enable/disable coin lockout for a particular coin */
void coin_lockout_w(running_machine *machine, int num, int on);

/* return current lockout state for a particular coin */
int coin_lockout_get_state(running_machine *machine, int num);

/* enable/disable global coin lockout */
void coin_lockout_global_w(running_machine *machine, int on);



/* ----- NVRAM management ----- */

/* open an NVRAM file directly */
mame_file *nvram_fopen(running_machine *machine, UINT32 openflags);

/* load NVRAM from a file */
void nvram_load(running_machine *machine);

/* save NVRAM to a file */
void nvram_save(running_machine *machine);

/* generic NVRAM handler that defaults to a 0 fill */
NVRAM_HANDLER( generic_0fill );

/* generic NVRAM handler that defaults to a 1 fill */
NVRAM_HANDLER( generic_1fill );

/* generic NVRAM handler that defaults to a random fill */
NVRAM_HANDLER( generic_randfill );



/* ----- memory card management ----- */

/* create a new memory card with the given index */
int memcard_create(running_machine *machine, int index, int overwrite);

/* "insert" a memory card with the given index and load its data */
int memcard_insert(running_machine *machine, int index);

/* "eject" a memory card and save its data */
void memcard_eject(running_machine &machine);

/* returns the index of the current memory card, or -1 if none */
int memcard_present(running_machine *machine);



/* ----- miscellaneous bits & pieces ----- */

/* set the status of an LED */
void set_led_status(running_machine *machine, int num, int value);



/* ----- interrupt enable and vector helpers ----- */

void generic_pulse_irq_line(running_device *device, int irqline);
void generic_pulse_irq_line_and_vector(running_device *device, int irqline, int vector);
void cpu_interrupt_enable(running_device *device, int enabled);
READ8_HANDLER( interrupt_enable_r );
WRITE8_HANDLER( interrupt_enable_w );



/* ----- common interrupt callbacks ----- */

INTERRUPT_GEN( nmi_line_pulse );
INTERRUPT_GEN( nmi_line_assert );

INTERRUPT_GEN( irq0_line_hold );
INTERRUPT_GEN( irq0_line_pulse );
INTERRUPT_GEN( irq0_line_assert );

INTERRUPT_GEN( irq1_line_hold );
INTERRUPT_GEN( irq1_line_pulse );
INTERRUPT_GEN( irq1_line_assert );

INTERRUPT_GEN( irq2_line_hold );
INTERRUPT_GEN( irq2_line_pulse );
INTERRUPT_GEN( irq2_line_assert );

INTERRUPT_GEN( irq3_line_hold );
INTERRUPT_GEN( irq3_line_pulse );
INTERRUPT_GEN( irq3_line_assert );

INTERRUPT_GEN( irq4_line_hold );
INTERRUPT_GEN( irq4_line_pulse );
INTERRUPT_GEN( irq4_line_assert );

INTERRUPT_GEN( irq5_line_hold );
INTERRUPT_GEN( irq5_line_pulse );
INTERRUPT_GEN( irq5_line_assert );

INTERRUPT_GEN( irq6_line_hold );
INTERRUPT_GEN( irq6_line_pulse );
INTERRUPT_GEN( irq6_line_assert );

INTERRUPT_GEN( irq7_line_hold );
INTERRUPT_GEN( irq7_line_pulse );
INTERRUPT_GEN( irq7_line_assert );



/* ----- generic watchdog reset handlers ----- */

/* 8-bit watchdog read/write handlers */
WRITE8_HANDLER( watchdog_reset_w );
READ8_HANDLER( watchdog_reset_r );

/* 16-bit watchdog read/write handlers */
WRITE16_HANDLER( watchdog_reset16_w );
READ16_HANDLER( watchdog_reset16_r );

/* 32-bit watchdog read/write handlers */
WRITE32_HANDLER( watchdog_reset32_w );
READ32_HANDLER( watchdog_reset32_r );



/* ----- generic input port helpers ----- */

/* custom handler */
CUSTOM_INPUT( custom_port_read );

#endif	/* __MACHINE_GENERIC_H__ */
