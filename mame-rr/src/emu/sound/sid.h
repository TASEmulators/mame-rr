#pragma once

#ifndef __SID_H__
#define __SID_H__

/*
  approximation of the sid6581 chip
  this part is for one chip,
*/

#include "sound/sid6581.h"
#include "sidvoice.h"
#include "streams.h"

/* private area */
typedef struct _SID6581
{
    running_device *device;
    sound_stream *mixer_channel; // mame stream/ mixer channel

    int (*ad_read) (running_device *device, int which);
    SIDTYPE type;
    UINT32 clock;

    UINT16 PCMfreq; // samplerate of the current systems soundcard/DAC
    UINT32 PCMsid, PCMsidNoise;

#if 0
	/* following depends on type */
	ptr2sidVoidFunc ModeNormalTable[16];
	ptr2sidVoidFunc ModeRingTable[16];
	// for speed reason it could be better to make them global!
	UINT8* waveform30;
	UINT8* waveform50;
	UINT8* waveform60;
	UINT8* waveform70;
#endif
	int reg[0x20];

//  bool sidKeysOn[0x20], sidKeysOff[0x20];

	UINT8 masterVolume;
	UINT16 masterVolumeAmplIndex;


	struct
	{
		int Enabled;
		UINT8 Type, CurType;
		float Dy, ResDy;
		UINT16 Value;
	} filter;

	sidOperator optr1, optr2, optr3;
    int optr3_outputmask;
} SID6581;

void sid6581_init (SID6581 *This);

int sidEmuReset(SID6581 *This);

int sid6581_port_r (running_machine *machine, SID6581 *This, int offset);
void sid6581_port_w (SID6581 *This, int offset, int data);

void sidEmuFillBuffer(SID6581 *This, stream_sample_t *buffer, UINT32 bufferLen );

#endif /* __SID_H__ */
