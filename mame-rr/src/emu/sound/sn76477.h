/*****************************************************************************

    Texas Instruments SN76477 emulator

    SN76477 pin layout. There is a corresponding interface variable with the
    same name. The only exception is noise_clock which must be programmatically
    set.  The other pins have programmatic equivalents as well.
    The name of the function is SN76477_<pinname>_w.
    All capacitor functions can also specify a fixed voltage on the cap.
    The name of this function is SN76477_<pinname>_voltage_w

                      +-------------------+
          envelope_1  | 1      | |      28|  envelope_2
                      | 2 GND   -       27|  mixer_c
         noise_clock  | 3               26|  mixer_a
     noise_clock_res  | 4               25|  mixer_b
    noise_filter_res  | 5               24|  one_shot_res
    noise_filter_cap  | 6               23|  one_shot_cap
           decay_res  | 7               22|  vco
    attack_decay_cap  | 8               21|  slf_cap
              enable o| 9               20|  slf_res
          attack_res  |10               19|  pitch_voltage
       amplitude_res  |11               18|  vco_res
        feedback_res  |12               17|  vco_cap
                      |13 OUTPUT        16|  vco_voltage
                      |14 Vcc   +5V OUT 15|
                      +-------------------+

    All resistor values in Ohms
    All capacitor values in Farads
    Use RES_K, RES_M and CAP_U, CAP_N, CAP_P macros in rescap.h to convert
    magnitudes, eg. 220k = RES_K(220), 47nF = CAP_N(47)

 *****************************************************************************/

#pragma once

#ifndef __SN76477_H__
#define __SN76477_H__

#include "devlegcy.h"
#include "machine/rescap.h"



/*****************************************************************************
 *
 *  Interface definition
 *
 *****************************************************************************/

typedef struct _sn76477_interface sn76477_interface;
struct _sn76477_interface
{
	double noise_clock_res;
	double noise_filter_res;
	double noise_filter_cap;
	double decay_res;
	double attack_decay_cap;
	double attack_res;
	double amplitude_res;
	double feedback_res;
	double vco_voltage;
	double vco_cap;
	double vco_res;
	double pitch_voltage;
	double slf_res;
	double slf_cap;
	double one_shot_cap;
	double one_shot_res;
	UINT32 vco;
	UINT32 mixer_a;
	UINT32 mixer_b;
	UINT32 mixer_c;
	UINT32 envelope_1;
	UINT32 envelope_2;
	UINT32 enable;
};



/*****************************************************************************
 *
 *  Functions to set a pin's value
 *
 *****************************************************************************/

/* these functions take 0 or 1 as a logic input */
void sn76477_enable_w(running_device *device, UINT32 data);		/* active LO, 0 = enabled, 1 = disabled */
void sn76477_mixer_a_w(running_device *device, UINT32 data);
void sn76477_mixer_b_w(running_device *device, UINT32 data);
void sn76477_mixer_c_w(running_device *device, UINT32 data);
void sn76477_envelope_1_w(running_device *device, UINT32 data);
void sn76477_envelope_2_w(running_device *device, UINT32 data);
void sn76477_vco_w(running_device *device, UINT32 data);			/* 0 = external, 1 = controlled by SLF */
void sn76477_noise_clock_w(running_device *device, UINT32 data);  /* noise clock write, if noise_clock_res = 0 */

/* these functions take a resistor value in Ohms */
void sn76477_one_shot_res_w(running_device *device, double data);
void sn76477_slf_res_w(running_device *device, double data);
void sn76477_vco_res_w(running_device *device, double data);
void sn76477_noise_clock_res_w(running_device *device, double data);  /* = 0 if the noise gen is clocked via noise_clock */
void sn76477_noise_filter_res_w(running_device *device, double data);
void sn76477_decay_res_w(running_device *device, double data);
void sn76477_attack_res_w(running_device *device, double data);
void sn76477_amplitude_res_w(running_device *device, double data);
void sn76477_feedback_res_w(running_device *device, double data);

/* these functions take a capacitor value in Farads or the voltage on it in Volts */
#define SN76477_EXTERNAL_VOLTAGE_DISCONNECT   (-1.0)	/* indicates that the voltage is internally computed,
                                                           can be used in all the functions that take a
                                                           voltage on a capacitor */
void sn76477_one_shot_cap_w(running_device *device, double data);
void sn76477_one_shot_cap_voltage_w(running_device *device, double data);
void sn76477_slf_cap_w(running_device *device, double data);
void sn76477_slf_cap_voltage_w(running_device *device, double data);
void sn76477_vco_cap_w(running_device *device, double data);
void sn76477_vco_cap_voltage_w(running_device *device, double data);
void sn76477_noise_filter_cap_w(running_device *device, double data);
void sn76477_noise_filter_cap_voltage_w(running_device *device, double data);
void sn76477_attack_decay_cap_w(running_device *device, double data);
void sn76477_attack_decay_cap_voltage_w(running_device *device, double data);

/* these functions take a voltage value in Volts */
void sn76477_vco_voltage_w(running_device *device, double data);
void sn76477_pitch_voltage_w(running_device *device, double data);

DECLARE_LEGACY_SOUND_DEVICE(SN76477, sn76477);

#endif/* __SN76477_H__ */
