/***************************************************************************

    dsp32.h
    Interface file for the portable DSP32 emulator.
    Written by Aaron Giles

***************************************************************************/

#pragma once

#ifndef __DSP32_H__
#define __DSP32_H__


/***************************************************************************
    COMPILE-TIME DEFINITIONS
***************************************************************************/



/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	/* CAU */
	DSP32_PC=1,
	DSP32_R0,DSP32_R1,DSP32_R2,DSP32_R3,
	DSP32_R4,DSP32_R5,DSP32_R6,DSP32_R7,
	DSP32_R8,DSP32_R9,DSP32_R10,DSP32_R11,
	DSP32_R12,DSP32_R13,DSP32_R14,DSP32_R15,
	DSP32_R16,DSP32_R17,DSP32_R18,DSP32_R19,
	DSP32_R20,DSP32_R21,DSP32_R22,
	DSP32_PIN,DSP32_POUT,DSP32_IVTP,

	/* DAU */
	DSP32_A0,DSP32_A1,DSP32_A2,DSP32_A3,
	DSP32_DAUC,

	/* PIO */
	DSP32_PAR,DSP32_PDR,DSP32_PIR,DSP32_PCR,
	DSP32_EMR,DSP32_ESR,DSP32_PCW,DSP32_PIOP,

	/* SIO */
	DSP32_IBUF,DSP32_ISR,DSP32_OBUF,DSP32_OSR,
	DSP32_IOC
};



/***************************************************************************
    INTERRUPT CONSTANTS
***************************************************************************/

#define DSP32_IRQ0		0		/* IRQ0 */
#define DSP32_IRQ1		1		/* IRQ1 */



/***************************************************************************
    CONFIGURATION
***************************************************************************/

#define DSP32_OUTPUT_PIF		0x01

typedef struct _dsp32_config dsp32_config;
struct _dsp32_config
{
	void	(*output_pins_changed)(running_device *device, UINT32 pins);	/* a change has occurred on an output pin */
};



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

DECLARE_LEGACY_CPU_DEVICE(DSP32C, dsp32c);

extern void dsp32c_pio_w(running_device *device, int reg, int data);
extern int dsp32c_pio_r(running_device *device, int reg);

#endif /* __DSP32_H__ */
