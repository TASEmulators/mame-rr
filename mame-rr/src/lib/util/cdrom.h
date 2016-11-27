/***************************************************************************

    cdrom.h

    Generic MAME cd-rom implementation

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __CDROM_H__
#define __CDROM_H__

#include "osdcore.h"
#include "chd.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CD_MAX_TRACKS			(99)	/* AFAIK the theoretical limit */
#define CD_MAX_SECTOR_DATA		(2352)
#define CD_MAX_SUBCODE_DATA		(96)

#define CD_FRAME_SIZE			(CD_MAX_SECTOR_DATA + CD_MAX_SUBCODE_DATA)
#define CD_FRAMES_PER_HUNK		(4)

#define CD_METADATA_WORDS		(1+(CD_MAX_TRACKS * 6))

enum
{
	CD_TRACK_MODE1 = 0, 		/* mode 1 2048 bytes/sector */
	CD_TRACK_MODE1_RAW,	    	/* mode 1 2352 bytes/sector */
	CD_TRACK_MODE2,		    	/* mode 2 2336 bytes/sector */
	CD_TRACK_MODE2_FORM1,		/* mode 2 2048 bytes/sector */
	CD_TRACK_MODE2_FORM2,		/* mode 2 2324 bytes/sector */
	CD_TRACK_MODE2_FORM_MIX,	/* mode 2 2336 bytes/sector */
	CD_TRACK_MODE2_RAW,	    	/* mode 2 2352 bytes / sector */
	CD_TRACK_AUDIO,			/* redbook audio track 2352 bytes/sector (588 samples) */

	CD_TRACK_RAW_DONTCARE		/* special flag for cdrom_read_data: just return me whatever is there */
};

enum
{
	CD_SUB_NORMAL = 0,			/* "cooked" 96 bytes per sector */
	CD_SUB_RAW,					/* raw uninterleaved 96 bytes per sector */
	CD_SUB_NONE					/* no subcode data stored */
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _cdrom_file cdrom_file;


typedef struct _cdrom_track_info cdrom_track_info;
struct _cdrom_track_info
{
	/* fields used by CHDMAN and in MAME */
	UINT32 trktype;		/* track type */
	UINT32 subtype;		/* subcode data type */
	UINT32 datasize;	/* size of data in each sector of this track */
	UINT32 subsize;		/* size of subchannel data in each sector of this track */
	UINT32 frames;		/* number of frames in this track */
	UINT32 extraframes;	/* number of "spillage" frames in this track */
	UINT32 pregap;		/* number of pregap frames */
	UINT32 postgap;		/* number of postgap frames */
	UINT32 pgtype;		/* type of sectors in pregap */
	UINT32 pgsub;		/* type of subchannel data in pregap */
	UINT32 pgdatasize;	/* size of data in each sector of the pregap */
	UINT32 pgsubsize;	/* size of subchannel data in each sector of the pregap */

	/* fields used in MAME only */
	UINT32 physframeofs;	/* frame number on the real CD this track starts at */
	UINT32 chdframeofs;	/* frame number this track starts at on the CHD */
};


typedef struct _cdrom_toc cdrom_toc;
struct _cdrom_toc
{
	UINT32 numtrks;		/* number of tracks */
	cdrom_track_info tracks[CD_MAX_TRACKS];
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* base functionality */
cdrom_file *cdrom_open(chd_file *chd);
void cdrom_close(cdrom_file *file);

/* core read access */
UINT32 cdrom_read_data(cdrom_file *file, UINT32 lbasector, void *buffer, UINT32 datatype);
UINT32 cdrom_read_subcode(cdrom_file *file, UINT32 lbasector, void *buffer);

/* handy utilities */
UINT32 cdrom_get_track(cdrom_file *file, UINT32 frame);
UINT32 cdrom_get_track_start(cdrom_file *file, UINT32 track);

/* TOC utilities */
int cdrom_get_last_track(cdrom_file *file);
int cdrom_get_adr_control(cdrom_file *file, int track);
int cdrom_get_track_type(cdrom_file *file, int track);
const cdrom_toc *cdrom_get_toc(cdrom_file *file);

/* extra utilities */
void cdrom_convert_type_string_to_track_info(const char *typestring, cdrom_track_info *info);
void cdrom_convert_type_string_to_pregap_info(const char *typestring, cdrom_track_info *info);
void cdrom_convert_subtype_string_to_track_info(const char *typestring, cdrom_track_info *info);
void cdrom_convert_subtype_string_to_pregap_info(const char *typestring, cdrom_track_info *info);
const char *cdrom_get_type_string(UINT32 trktype);
const char *cdrom_get_subtype_string(UINT32 subtype);
chd_error cdrom_parse_metadata(chd_file *chd, cdrom_toc *toc);
chd_error cdrom_write_metadata(chd_file *chd, const cdrom_toc *toc);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE UINT32 lba_to_msf(UINT32 lba)
{
	UINT8 m, s, f;

	m = lba / (60 * 75);
	lba -= m * (60 * 75);
	s = lba / 75;
	f = lba % 75;

	return ((m / 10) << 20) | ((m % 10) << 16) |
	       ((s / 10) << 12) | ((s % 10) <<  8) |
	       ((f / 10) <<  4) | ((f % 10) <<  0);
}

#endif	// __CDROM_H__
