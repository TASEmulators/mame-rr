/***************************************************************************

    aviio.c

    AVI movie format parsing helpers.

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

#include <stdlib.h>

#include "aviio.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define FILETYPE_READ			1
#define FILETYPE_CREATE			2

#define MAX_RIFF_SIZE			(2UL * 1024 * 1024 * 1024 - 1024)	/* just under 2GB */
#define MAX_AVI_SIZE_IN_GB		(256)
#define FOUR_GB					((UINT64)1 << 32)

#define MAX_SOUND_CHANNELS		2
#define SOUND_BUFFER_MSEC		2000		/* microseconds of sond buffering */

#define CHUNKTYPE_RIFF			AVI_FOURCC('R','I','F','F')
#define CHUNKTYPE_LIST			AVI_FOURCC('L','I','S','T')
#define CHUNKTYPE_JUNK			AVI_FOURCC('J','U','N','K')
#define CHUNKTYPE_AVIH			AVI_FOURCC('a','v','i','h')
#define CHUNKTYPE_STRH			AVI_FOURCC('s','t','r','h')
#define CHUNKTYPE_STRF			AVI_FOURCC('s','t','r','f')
#define CHUNKTYPE_IDX1			AVI_FOURCC('i','d','x','1')
#define CHUNKTYPE_INDX			AVI_FOURCC('i','n','d','x')
#define CHUNKTYPE_XXDB			AVI_FOURCC(0x00,0x00,'d','b')
#define CHUNKTYPE_XXDC			AVI_FOURCC(0x00,0x00,'d','c')
#define CHUNKTYPE_XXWB			AVI_FOURCC(0x00,0x00,'w','b')
#define CHUNKTYPE_IXXX			AVI_FOURCC('i','x',0x00,0x00)
#define CHUNKTYPE_XX_MASK		AVI_FOURCC(0x00,0x00,0xff,0xff)

#define LISTTYPE_AVI			AVI_FOURCC('A','V','I',' ')
#define LISTTYPE_AVIX			AVI_FOURCC('A','V','I','X')
#define LISTTYPE_HDRL			AVI_FOURCC('h','d','r','l')
#define LISTTYPE_STRL			AVI_FOURCC('s','t','r','l')
#define LISTTYPE_MOVI			AVI_FOURCC('m','o','v','i')

#define STREAMTYPE_VIDS			AVI_FOURCC('v','i','d','s')
#define STREAMTYPE_AUDS			AVI_FOURCC('a','u','d','s')

#define HANDLER_DIB				AVI_FOURCC('D','I','B',' ')
#define HANDLER_HFYU			AVI_FOURCC('h','f','y','u')

/* main AVI header files */
#define AVIF_HASINDEX			0x00000010
#define AVIF_MUSTUSEINDEX		0x00000020
#define AVIF_ISINTERLEAVED		0x00000100
#define AVIF_COPYRIGHTED		0x00010000
#define AVIF_WASCAPTUREFILE		0x00020000

/* index definitions */
#define AVI_INDEX_OF_INDEXES	0x00
#define AVI_INDEX_OF_CHUNKS		0x01
#define AVI_INDEX_IS_DATA		0x80
#define AVI_INDEX_2FIELD		0x01

/* HuffYUV definitions */
#define HUFFYUV_PREDICT_LEFT	 0
#define HUFFYUV_PREDICT_GRADIENT 1
#define HUFFYUV_PREDICT_MEDIAN	 2
#define HUFFYUV_PREDICT_DECORR	 0x40



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _avi_chunk avi_chunk;
struct _avi_chunk
{
	UINT64				offset;					/* file offset of chunk header */
	UINT64				size;					/* size of this chunk */
	UINT32				type;					/* type of this chunk */
	UINT32				listtype;				/* type of this list (if we are a list) */
};


typedef struct _avi_chunk_list avi_chunk_list;
struct _avi_chunk_list
{
	UINT64				offset;					/* offset in the file of header */
	UINT32				length;					/* length of the chunk including header */
};


typedef struct _huffyuv_table huffyuv_table;
struct _huffyuv_table
{
	UINT8				shift[256];				/* bit shift amounts */
	UINT32				bits[256];				/* bit match values */
	UINT32				mask[256];				/* bit mask values */
	UINT16				baselookup[65536];		/* base lookup table */
	UINT16 *			extralookup;			/* extra lookup tables */
};


typedef struct _huffyuv_data huffyuv_data;
struct _huffyuv_data
{
	UINT8				predictor;				/* predictor */
	huffyuv_table		table[3];				/* array of tables */
};


typedef struct _avi_stream avi_stream;
struct _avi_stream
{
	UINT32				type;					/* subtype of stream */
	UINT32				format;					/* format of stream data */

	UINT32				rate;					/* timescale for stream */
	UINT32				scale;					/* duration of one sample in the stream */
	UINT32				samples;				/* number of samples */

	avi_chunk_list *	chunk;					/* list of chunks */
	UINT32				chunks;					/* chunks currently known */
	UINT32				chunksalloc;			/* number of chunks allocated */

	UINT32				width;					/* width of video */
	UINT32				height;					/* height of video */
	UINT32				depth;					/* depth of video */
	UINT8				interlace;				/* interlace parameters */
	huffyuv_data *		huffyuv;				/* huffyuv decompression data */

	UINT16				channels;				/* audio channels */
	UINT16				samplebits;				/* audio bits per sample */
	UINT32				samplerate;				/* audio sample rate */

	/* only used when creating */
	UINT64				saved_strh_offset;		/* writeoffset of strh chunk */
	UINT64				saved_indx_offset;		/* writeoffset of indx chunk */
};


struct _avi_file
{
	/* shared data */
	osd_file *			file;					/* pointer to open file */
	int					type;					/* type of access (read/create) */
	avi_movie_info		info;					/* movie info structure */
	UINT8 *				tempbuffer;				/* temporary buffer */
	UINT32				tempbuffersize;			/* size of the temporary buffer */

	/* only used when reading */
	int					streams;				/* number of streams */
	avi_stream *		stream;					/* allocated array of stream information */
	avi_chunk			rootchunk;				/* dummy root chunk that wraps the whole file */

	/* only used when creating */
	UINT64				writeoffs;				/* current file write offset */
	UINT64				riffbase;				/* base of the current RIFF */

	avi_chunk			chunkstack[8];			/* stack of chunks we are writing */
	int					chunksp;				/* stack pointer for the current chunk */

	UINT64				saved_movi_offset;		/* writeoffset of movi list */
	UINT64				saved_avih_offset;		/* writeoffset of avih chunk */

	INT16 *				soundbuf;				/* buffer for sound data */
	UINT32				soundbuf_samples;		/* length of sound buffer in samples */
	UINT32				soundbuf_chansamples[MAX_SOUND_CHANNELS]; /* samples in buffer for each channel */
	UINT32				soundbuf_chunks;		/* number of chunks completed so far */
	UINT32				soundbuf_frames;		/* number of frames ahead of the video */
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* stream helpers */

/* core chunk read routines */
static avi_error get_first_chunk(avi_file *file, const avi_chunk *parent, avi_chunk *newchunk);
static avi_error get_next_chunk(avi_file *file, const avi_chunk *parent, avi_chunk *newchunk);
static avi_error find_first_chunk(avi_file *file, UINT32 findme, const avi_chunk *container, avi_chunk *result);
static avi_error find_next_chunk(avi_file *file, UINT32 findme, const avi_chunk *container, avi_chunk *result);
static avi_error get_next_chunk_internal(avi_file *file, const avi_chunk *parent, avi_chunk *newchunk, UINT64 offset);
static avi_error read_chunk_data(avi_file *file, const avi_chunk *chunk, UINT8 **buffer);

/* chunk read helpers */
static avi_error read_movie_data(avi_file *file);
static avi_error extract_movie_info(avi_file *file);
static avi_error parse_avih_chunk(avi_file *file, avi_chunk *avih);
static avi_error parse_strh_chunk(avi_file *file, avi_stream *stream, avi_chunk *strh);
static avi_error parse_strf_chunk(avi_file *file, avi_stream *stream, avi_chunk *strf);
static avi_error parse_indx_chunk(avi_file *file, avi_stream *stream, avi_chunk *strf);
static avi_error parse_idx1_chunk(avi_file *file, UINT64 baseoffset, avi_chunk *idx1);

/* core chunk write routines */
static avi_error chunk_open(avi_file *file, UINT32 type, UINT32 listtype, UINT32 estlength);
static avi_error chunk_close(avi_file *file);
static avi_error chunk_write(avi_file *file, UINT32 type, const void *data, UINT32 length);
static avi_error chunk_overwrite(avi_file *file, UINT32 type, const void *data, UINT32 length, UINT64 *offset, int initial_write);

/* chunk write helpers */
static avi_error write_initial_headers(avi_file *file);
static avi_error write_avih_chunk(avi_file *file, int initial_write);
static avi_error write_strh_chunk(avi_file *file, avi_stream *stream, int initial_write);
static avi_error write_strf_chunk(avi_file *file, avi_stream *stream);
static avi_error write_indx_chunk(avi_file *file, avi_stream *stream, int initial_write);
static avi_error write_idx1_chunk(avi_file *file);

/* sound buffering helpers */
static avi_error soundbuf_initialize(avi_file *file);
static avi_error soundbuf_write_chunk(avi_file *file, UINT32 framenum);
static avi_error soundbuf_flush(avi_file *file, int only_flush_full);

/* RGB helpers */
static avi_error rgb32_compress_to_rgb(avi_stream *stream, const bitmap_t *bitmap, UINT8 *data, UINT32 numbytes);

/* YUY helpers */
static avi_error yuv_decompress_to_yuy16(avi_stream *stream, const UINT8 *data, UINT32 numbytes, bitmap_t *bitmap);
static avi_error yuy16_compress_to_yuy(avi_stream *stream, const bitmap_t *bitmap, UINT8 *data, UINT32 numbytes);

/* HuffYUV helpers */
static avi_error huffyuv_extract_tables(avi_stream *stream, const UINT8 *chunkdata, UINT32 size);
static avi_error huffyuv_decompress_to_yuy16(avi_stream *stream, const UINT8 *data, UINT32 numbytes, bitmap_t *bitmap);

/* debugging */
static void printf_chunk_recursive(avi_file *file, avi_chunk *chunk, int indent);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    fetch_16bits - read 16 bits in LSB order
    from the given buffer
-------------------------------------------------*/

INLINE UINT16 fetch_16bits(const UINT8 *data)
{
	return data[0] | (data[1] << 8);
}


/*-------------------------------------------------
    fetch_32bits - read 32 bits in LSB order
    from the given buffer
-------------------------------------------------*/

INLINE UINT32 fetch_32bits(const UINT8 *data)
{
	return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}


/*-------------------------------------------------
    fetch_64bits - read 64 bits in LSB order
    from the given buffer
-------------------------------------------------*/

INLINE UINT64 fetch_64bits(const UINT8 *data)
{
	return (UINT64)data[0] | ((UINT64)data[1] << 8) |
		   ((UINT64)data[2] << 16) | ((UINT64)data[3] << 24) |
		   ((UINT64)data[4] << 32) | ((UINT64)data[5] << 40) |
		   ((UINT64)data[6] << 48) | ((UINT64)data[7] << 56);
}


/*-------------------------------------------------
    put_16bits - write 16 bits in LSB order
    to the given buffer
-------------------------------------------------*/

INLINE void put_16bits(UINT8 *data, UINT16 value)
{
	data[0] = value >> 0;
	data[1] = value >> 8;
}


/*-------------------------------------------------
    put_32bits - write 32 bits in LSB order
    to the given buffer
-------------------------------------------------*/

INLINE void put_32bits(UINT8 *data, UINT32 value)
{
	data[0] = value >> 0;
	data[1] = value >> 8;
	data[2] = value >> 16;
	data[3] = value >> 24;
}


/*-------------------------------------------------
    put_64bits - write 64 bits in LSB order
    to the given buffer
-------------------------------------------------*/

INLINE void put_64bits(UINT8 *data, UINT64 value)
{
	data[0] = value >> 0;
	data[1] = value >> 8;
	data[2] = value >> 16;
	data[3] = value >> 24;
	data[4] = value >> 32;
	data[5] = value >> 40;
	data[6] = value >> 48;
	data[7] = value >> 56;
}


/*-------------------------------------------------
    get_video_stream - return a pointer to the
    video stream
-------------------------------------------------*/

INLINE avi_stream *get_video_stream(avi_file *file)
{
	int streamnum;

	/* find the video stream */
	for (streamnum = 0; streamnum < file->streams; streamnum++)
		if (file->stream[streamnum].type == STREAMTYPE_VIDS)
			return &file->stream[streamnum];

	return NULL;
}


/*-------------------------------------------------
    get_audio_stream - return a pointer to the
    audio stream for the 'n'th channel
-------------------------------------------------*/

INLINE avi_stream *get_audio_stream(avi_file *file, int channel, int *offset)
{
	int streamnum;

	/* find the audios stream */
	for (streamnum = 0; streamnum < file->streams; streamnum++)
		if (file->stream[streamnum].type == STREAMTYPE_AUDS)
		{
			if (channel < file->stream[streamnum].channels)
			{
				if (offset != NULL)
					*offset = channel;
				return &file->stream[streamnum];
			}
			channel -= file->stream[streamnum].channels;
		}

	return NULL;
}


/*-------------------------------------------------
    set_stream_chunk_info - set the chunk info
    for a given chunk within a stream
-------------------------------------------------*/

INLINE avi_error set_stream_chunk_info(avi_stream *stream, UINT32 index, UINT64 offset, UINT32 length)
{
	/* if we need to allocate more, allocate more */
	if (index >= stream->chunksalloc)
	{
		UINT32 newcount = MAX(index, stream->chunksalloc + 1000);
		stream->chunk = (avi_chunk_list *)realloc(stream->chunk, newcount * sizeof(stream->chunk[0]));
		if (stream->chunk == NULL)
			return AVIERR_NO_MEMORY;
		stream->chunksalloc = newcount;
	}

	/* set the data */
	stream->chunk[index].offset = offset;
	stream->chunk[index].length = length;

	/* update the number of chunks */
	stream->chunks = MAX(stream->chunks, index + 1);
	return AVIERR_NONE;
}


/*-------------------------------------------------
    compute_idx1_size - compute the size of the
    idx1 chunk
-------------------------------------------------*/

INLINE UINT32 compute_idx1_size(avi_file *file)
{
	int chunks = 0;
	int strnum;

	/* count chunks in streams */
	for (strnum = 0; strnum < file->streams; strnum++)
		chunks += file->stream[strnum].chunks;

	return chunks * 16 + 8;
}


/*-------------------------------------------------
    get_chunkid_for_stream - make a chunk id for
    a given stream
-------------------------------------------------*/

INLINE UINT32 get_chunkid_for_stream(avi_file *file, avi_stream *stream)
{
	UINT32 chunkid;

	chunkid = AVI_FOURCC('0' + (stream - file->stream) / 10, '0' +  (stream - file->stream) % 10, 0, 0);
	if (stream->type == STREAMTYPE_VIDS)
		chunkid |= (stream->format == 0) ? CHUNKTYPE_XXDB : CHUNKTYPE_XXDC;
	else if (stream->type == STREAMTYPE_AUDS)
		chunkid |= CHUNKTYPE_XXWB;

	return chunkid;
}


/*-------------------------------------------------
    framenum_to_samplenum - given a video frame
    number, get the first sample number
-------------------------------------------------*/

INLINE UINT32 framenum_to_samplenum(avi_file *file, UINT32 framenum)
{
	return ((UINT64)file->info.audio_samplerate * (UINT64)framenum * (UINT64)file->info.video_sampletime + file->info.video_timescale - 1) / (UINT64)file->info.video_timescale;
}


/*-------------------------------------------------
    expand_tempbuffer - expand the file's
    tempbuffer if necessary to contain the
    requested amount of data
-------------------------------------------------*/

INLINE avi_error expand_tempbuffer(avi_file *file, UINT32 length)
{
	/* expand the tempbuffer to hold the data if necessary */
	if (length > file->tempbuffersize)
	{
		file->tempbuffersize = 2 * length;
		file->tempbuffer = (UINT8 *)realloc(file->tempbuffer, file->tempbuffersize);
		if (file->tempbuffer == NULL)
			return AVIERR_NO_MEMORY;
	}
	return AVIERR_NONE;
}



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    avi_open - open an AVI movie file for read
-------------------------------------------------*/

avi_error avi_open(const char *filename, avi_file **file)
{
	avi_file *newfile = NULL;
	file_error filerr;
	avi_error avierr;
	UINT64 length;

	/* allocate the file */
	newfile = (avi_file *)malloc(sizeof(*newfile));
	if (newfile == NULL)
		return AVIERR_NO_MEMORY;
	memset(newfile, 0, sizeof(*newfile));
	newfile->type = FILETYPE_READ;

	/* open the file */
	filerr = osd_open(filename, OPEN_FLAG_READ, &newfile->file, &length);
	if (filerr != FILERR_NONE)
	{
		avierr = AVIERR_CANT_OPEN_FILE;
		goto error;
	}

	/* make a root atom */
	newfile->rootchunk.offset = 0;
	newfile->rootchunk.size = length;
	newfile->rootchunk.type = 0;
	newfile->rootchunk.listtype = 0;

	/* parse the data */
	avierr = read_movie_data(newfile);
	if (avierr != AVIERR_NONE)
		goto error;

	*file = newfile;
	return AVIERR_NONE;

error:
	/* clean up after an error */
	if (newfile != NULL)
	{
		if (newfile->file != NULL)
			osd_close(newfile->file);
		free(newfile);
	}
	return avierr;
}


/*-------------------------------------------------
    avi_create - create a new QuickTime movie file
-------------------------------------------------*/

avi_error avi_create(const char *filename, const avi_movie_info *info, avi_file **file)
{
	avi_file *newfile = NULL;
	file_error filerr;
	avi_stream *stream;
	avi_error avierr;
	UINT64 length;

	/* validate video info */
	if ((info->video_format != 0 && info->video_format != FORMAT_UYVY && info->video_format != FORMAT_VYUY && info->video_format != FORMAT_YUY2)  ||
		info->video_width == 0 ||
		info->video_height == 0 ||
		info->video_depth == 0 || info->video_depth % 8 != 0)
		return AVIERR_UNSUPPORTED_VIDEO_FORMAT;

	/* validate audio info */
	if (info->audio_format != 0 ||
		info->audio_channels > MAX_SOUND_CHANNELS ||
		info->audio_samplebits != 16)
		return AVIERR_UNSUPPORTED_AUDIO_FORMAT;

	/* allocate the file */
	newfile = (avi_file *)malloc(sizeof(*newfile));
	if (newfile == NULL)
		return AVIERR_NO_MEMORY;
	memset(newfile, 0, sizeof(*newfile));
	newfile->type = FILETYPE_CREATE;

	/* open the file */
	filerr = osd_open(filename, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &newfile->file, &length);
	if (filerr != FILERR_NONE)
	{
		avierr = AVIERR_CANT_OPEN_FILE;
		goto error;
	}

	/* copy the movie info */
	newfile->info = *info;
	newfile->info.video_numsamples = 0;
	newfile->info.audio_numsamples = 0;

	/* allocate two streams */
	newfile->stream = (avi_stream *)malloc(2 * sizeof(newfile->stream[0]));
	if (newfile->stream == NULL)
	{
		avierr = AVIERR_NO_MEMORY;
		goto error;
	}
	memset(newfile->stream, 0, 2 * sizeof(newfile->stream[0]));

	/* initialize the video track */
	stream = &newfile->stream[newfile->streams++];
	stream->type = STREAMTYPE_VIDS;
	stream->format = newfile->info.video_format;
	stream->rate = newfile->info.video_timescale;
	stream->scale = newfile->info.video_sampletime;
	stream->width = newfile->info.video_width;
	stream->height = newfile->info.video_height;
	stream->depth = newfile->info.video_depth;

	/* initialize the audio track */
	if (newfile->info.audio_channels > 0)
	{
		stream = &newfile->stream[newfile->streams++];
		stream->type = STREAMTYPE_AUDS;
		stream->format = newfile->info.audio_format;
		stream->rate = newfile->info.audio_timescale;
		stream->scale = newfile->info.audio_sampletime;
		stream->channels = newfile->info.audio_channels;
		stream->samplebits = newfile->info.audio_samplebits;
		stream->samplerate = newfile->info.audio_samplerate;

		/* initialize the sound buffering */
		avierr = soundbuf_initialize(newfile);
		if (avierr != AVIERR_NONE)
			goto error;
	}

	/* write the initial headers */
	avierr = write_initial_headers(newfile);

	*file = newfile;
	return AVIERR_NONE;

error:
	/* clean up after an error */
	if (newfile != NULL)
	{
		if (newfile->stream != NULL)
			free(newfile->stream);
		if (newfile->file != NULL)
		{
			osd_close(newfile->file);
			osd_rmfile(filename);
		}
		free(newfile);
	}
	return avierr;
}


/*-------------------------------------------------
    avi_close - close a QuickTime movie file
-------------------------------------------------*/

avi_error avi_close(avi_file *file)
{
	avi_error avierr = AVIERR_NONE;
	int strnum;

	/* if we're creating a new file, finalize it by writing out the non-media chunks */
	if (file->type == FILETYPE_CREATE)
	{
		/* flush any pending sound data */
		avierr = soundbuf_flush(file, FALSE);

		/* close the movi chunk */
		if (avierr == AVIERR_NONE)
			avierr = chunk_close(file);

		/* if this is the first RIFF chunk, write an idx1 */
		if (avierr == AVIERR_NONE && file->riffbase == 0)
			avierr = write_idx1_chunk(file);

		/* update the strh and indx chunks for each stream */
		for (strnum = 0; strnum < file->streams; strnum++)
		{
			if (avierr == AVIERR_NONE)
				avierr = write_strh_chunk(file, &file->stream[strnum], FALSE);
			if (avierr == AVIERR_NONE)
				avierr = write_indx_chunk(file, &file->stream[strnum], FALSE);
		}

		/* update the avih chunk */
		if (avierr == AVIERR_NONE)
			avierr = write_avih_chunk(file, FALSE);

		/* close the RIFF chunk */
		if (avierr == AVIERR_NONE)
			avierr = chunk_close(file);
	}

	/* close the file */
	osd_close(file->file);

	/* free the stream-specific data */
	for (strnum = 0; strnum < file->streams; strnum++)
	{
		avi_stream *stream = &file->stream[strnum];
		if (stream->huffyuv != NULL)
		{
			huffyuv_data *huffyuv = stream->huffyuv;
			int table;

			for (table = 0; table < ARRAY_LENGTH(huffyuv->table); table++)
				if (huffyuv->table[table].extralookup != NULL)
					free(huffyuv->table[table].extralookup);
			free(huffyuv);
		}
		if (stream->chunk != NULL)
			free(stream->chunk);
	}

	/* free the file itself */
	if (file->soundbuf != NULL)
		free(file->soundbuf);
	if (file->stream != NULL)
		free(file->stream);
	if (file->tempbuffer != NULL)
		free(file->tempbuffer);
	free(file);
	return avierr;
}


/*-------------------------------------------------
    avi_printf_chunks - print the chunks in a file
-------------------------------------------------*/

void avi_printf_chunks(avi_file *file)
{
	printf_chunk_recursive(file, &file->rootchunk, 0);
}


/*-------------------------------------------------
    avi_error_string - get the error string for
    an avi_error
-------------------------------------------------*/

const char *avi_error_string(avi_error err)
{
	switch (err)
	{
		case AVIERR_NONE:						return "success";
		case AVIERR_END:						return "hit end of file";
		case AVIERR_INVALID_DATA:				return "invalid data";
		case AVIERR_NO_MEMORY:					return "out of memory";
		case AVIERR_READ_ERROR:					return "read error";
		case AVIERR_WRITE_ERROR:				return "write error";
		case AVIERR_STACK_TOO_DEEP:				return "stack overflow";
		case AVIERR_UNSUPPORTED_FEATURE:		return "unsupported feature";
		case AVIERR_CANT_OPEN_FILE:				return "unable to open file";
		case AVIERR_INCOMPATIBLE_AUDIO_STREAMS:	return "found incompatible audio streams";
		case AVIERR_INVALID_SAMPLERATE:			return "found invalid sample rate";
		case AVIERR_INVALID_STREAM:				return "invalid stream";
		case AVIERR_INVALID_FRAME:				return "invalid frame index";
		case AVIERR_INVALID_BITMAP:				return "invalid bitmap";
		case AVIERR_UNSUPPORTED_VIDEO_FORMAT:	return "unsupported video format";
		case AVIERR_UNSUPPORTED_AUDIO_FORMAT:	return "unsupported audio format";
		case AVIERR_EXCEEDED_SOUND_BUFFER:		return "sound buffer overflow";
		default:								return "undocumented error";
	}
}


/*-------------------------------------------------
    avi_get_movie_info - return a pointer to the
    movie info
-------------------------------------------------*/

const avi_movie_info *avi_get_movie_info(avi_file *file)
{
	return &file->info;
}


/*-------------------------------------------------
    avi_frame_to_sample - convert a frame index
    to a sample index
-------------------------------------------------*/

UINT32 avi_first_sample_in_frame(avi_file *file, UINT32 framenum)
{
	return framenum_to_samplenum(file, framenum);
}


/*-------------------------------------------------
    avi_read_video_frame_yuy16 - read video data
    for a particular frame from the AVI file,
    converting to YUY16 format
-------------------------------------------------*/

avi_error avi_read_video_frame_yuy16(avi_file *file, UINT32 framenum, bitmap_t *bitmap)
{
	avi_error avierr = AVIERR_NONE;
	UINT32 bytes_read, chunkid;
	file_error filerr;
	avi_stream *stream;

	/* get the video stream */
	stream = get_video_stream(file);
	if (stream == NULL)
		return AVIERR_INVALID_STREAM;

	/* validate our ability to handle the data */
	if (stream->format != FORMAT_UYVY && stream->format != FORMAT_VYUY && stream->format != FORMAT_YUY2 && stream->format != FORMAT_HFYU)
		return AVIERR_UNSUPPORTED_VIDEO_FORMAT;

	/* assume one chunk == one frame */
	if (framenum >= stream->chunks)
		return AVIERR_INVALID_FRAME;

	/* we only support YUY-style bitmaps (16bpp) */
	if (bitmap->format != BITMAP_FORMAT_YUY16 || bitmap->width < stream->width || bitmap->height < stream->height)
		return AVIERR_INVALID_BITMAP;

	/* expand the tempbuffer to hold the data if necessary */
	avierr = expand_tempbuffer(file, stream->chunk[framenum].length);
	if (avierr != AVIERR_NONE)
		return avierr;

	/* read in the data */
	filerr = osd_read(file->file, file->tempbuffer, stream->chunk[framenum].offset, stream->chunk[framenum].length, &bytes_read);
	if (filerr != FILERR_NONE || bytes_read != stream->chunk[framenum].length)
		return AVIERR_READ_ERROR;

	/* validate this is good data */
	chunkid = fetch_32bits(&file->tempbuffer[0]);
	if (chunkid == get_chunkid_for_stream(file, stream))
	{
		/* HuffYUV-compressed */
		if (stream->format == FORMAT_HFYU)
			avierr = huffyuv_decompress_to_yuy16(stream, file->tempbuffer + 8, stream->chunk[framenum].length - 8, bitmap);

		/* other YUV-compressed */
		else
			avierr = yuv_decompress_to_yuy16(stream, file->tempbuffer + 8, stream->chunk[framenum].length - 8, bitmap);
	}
	else
		avierr = AVIERR_INVALID_DATA;

	return avierr;
}


/*-------------------------------------------------
    avi_read_sound_samples - read sound sample
    data from an AVI file
-------------------------------------------------*/

avi_error avi_read_sound_samples(avi_file *file, int channel, UINT32 firstsample, UINT32 numsamples, INT16 *output)
{
	avi_error avierr = AVIERR_NONE;
	UINT32 bytes_per_sample;
	file_error filerr;
	avi_stream *stream;
	int offset;

	/* get the audio stream */
	stream = get_audio_stream(file, channel, &offset);
	if (stream == NULL)
		return AVIERR_INVALID_STREAM;

	/* validate our ability to handle the data */
	if (stream->format != 0 || (stream->samplebits != 8 && stream->samplebits != 16))
		return AVIERR_UNSUPPORTED_AUDIO_FORMAT;

	/* verify we are in range */
	if (firstsample >= stream->samples)
		return AVIERR_INVALID_FRAME;
	if (firstsample + numsamples > stream->samples)
		numsamples = stream->samples - firstsample;

	/* determine bytes per sample */
	bytes_per_sample = (stream->samplebits / 8) * stream->channels;

	/* loop until all samples have been extracted */
	while (numsamples > 0)
	{
		UINT32 chunkbase = 0, chunkend = 0, chunkid;
		UINT32 bytes_read, samples_this_chunk;
		int chunknum, sampnum;

		/* locate the chunk with the first sample */
		for (chunknum = 0; chunknum < stream->chunks; chunknum++)
		{
			chunkend = chunkbase + (stream->chunk[chunknum].length - 8) / bytes_per_sample;
			if (firstsample < chunkend)
				break;
			chunkbase = chunkend;
		}

		/* if we hit the end, fill the rest with silence */
		if (chunknum == stream->chunks)
		{
			memset(output, 0, numsamples * 2);
			break;
		}

		/* expand the tempbuffer to hold the data if necessary */
		avierr = expand_tempbuffer(file, stream->chunk[chunknum].length);
		if (avierr != AVIERR_NONE)
			return avierr;

		/* read in the data */
		filerr = osd_read(file->file, file->tempbuffer, stream->chunk[chunknum].offset, stream->chunk[chunknum].length, &bytes_read);
		if (filerr != FILERR_NONE || bytes_read != stream->chunk[chunknum].length)
			return AVIERR_READ_ERROR;

		/* validate this is good data */
		chunkid = fetch_32bits(&file->tempbuffer[0]);
		if (chunkid != get_chunkid_for_stream(file, stream))
			return AVIERR_INVALID_DATA;

		/* determine how many samples to copy */
		samples_this_chunk = chunkend - firstsample;
		samples_this_chunk = MIN(samples_this_chunk, numsamples);

		/* extract 16-bit samples from the chunk */
		if (stream->samplebits == 16)
		{
			const INT16 *base = (const INT16 *)(file->tempbuffer + 8);
			base += stream->channels * (firstsample - chunkbase) + offset;
			for (sampnum = 0; sampnum < samples_this_chunk; sampnum++)
			{
				*output++ = LITTLE_ENDIANIZE_INT16(*base);
				base += stream->channels;
			}
		}

		/* extract 8-bit samples from the chunk */
		else if (stream->samplebits == 8)
		{
			const UINT8 *base = (const UINT8 *)(file->tempbuffer + 8);
			base += stream->channels * (firstsample - chunkbase) + offset;
			for (sampnum = 0; sampnum < samples_this_chunk; sampnum++)
			{
				*output++ = (*base << 8) - 0x8000;
				base += stream->channels;
			}
		}

		/* update our counters */
		firstsample += samples_this_chunk;
		numsamples -= samples_this_chunk;
	}

	return avierr;
}


/*-------------------------------------------------
    avi_append_video_frame_yuy16 - append a frame
    of video in YUY16 format
-------------------------------------------------*/

avi_error avi_append_video_frame_yuy16(avi_file *file, const bitmap_t *bitmap)
{
	avi_stream *stream = get_video_stream(file);
	avi_error avierr;
	UINT32 maxlength;

	/* validate our ability to handle the data */
	if (stream->format != FORMAT_UYVY && stream->format != FORMAT_VYUY && stream->format != FORMAT_YUY2 && stream->format != FORMAT_HFYU)
		return AVIERR_UNSUPPORTED_VIDEO_FORMAT;

	/* double check bitmap format */
	if (bitmap->format != BITMAP_FORMAT_YUY16)
		return AVIERR_INVALID_BITMAP;

	/* write out any sound data first */
	avierr = soundbuf_write_chunk(file, stream->chunks);
	if (avierr != AVIERR_NONE)
		return avierr;

	/* make sure we have enough room */
	maxlength = 2 * stream->width * stream->height;
	avierr = expand_tempbuffer(file, maxlength);
	if (avierr != AVIERR_NONE)
		return avierr;

	/* now compress the data */
	avierr = yuy16_compress_to_yuy(stream, bitmap, file->tempbuffer, maxlength);
	if (avierr != AVIERR_NONE)
		return avierr;

	/* set the info for this new chunk */
	avierr = set_stream_chunk_info(stream, stream->chunks, file->writeoffs, maxlength + 8);
	if (avierr != AVIERR_NONE)
		return avierr;
	stream->samples = file->info.video_numsamples = stream->chunks;

	/* write the data */
	return chunk_write(file, get_chunkid_for_stream(file, stream), file->tempbuffer, maxlength);
}


/*-------------------------------------------------
    avi_append_video_frame_rgb32 - append a frame
    of video in RGB32 format
-------------------------------------------------*/

avi_error avi_append_video_frame_rgb32(avi_file *file, const bitmap_t *bitmap)
{
	avi_stream *stream = get_video_stream(file);
	avi_error avierr;
	UINT32 maxlength;

	/* validate our ability to handle the data */
	if (stream->format != 0)
		return AVIERR_UNSUPPORTED_VIDEO_FORMAT;

	/* depth must be 24 */
	if (stream->depth != 24)
		return AVIERR_UNSUPPORTED_VIDEO_FORMAT;

	/* double check bitmap format */
	if (bitmap->format != BITMAP_FORMAT_RGB32)
		return AVIERR_INVALID_BITMAP;

	/* write out any sound data first */
	avierr = soundbuf_write_chunk(file, stream->chunks);
	if (avierr != AVIERR_NONE)
		return avierr;

	/* make sure we have enough room */
	maxlength = 3 * stream->width * stream->height;
	avierr = expand_tempbuffer(file, maxlength);
	if (avierr != AVIERR_NONE)
		return avierr;

	/* copy the RGB data to the destination */
	avierr = rgb32_compress_to_rgb(stream, bitmap, file->tempbuffer, maxlength);
	if (avierr != AVIERR_NONE)
		return avierr;

	/* set the info for this new chunk */
	avierr = set_stream_chunk_info(stream, stream->chunks, file->writeoffs, maxlength + 8);
	if (avierr != AVIERR_NONE)
		return avierr;
	stream->samples = file->info.video_numsamples = stream->chunks;

	/* write the data */
	return chunk_write(file, get_chunkid_for_stream(file, stream), file->tempbuffer, maxlength);
}


/*-------------------------------------------------
    avi_append_sound_samples - append sound
    samples
-------------------------------------------------*/

avi_error avi_append_sound_samples(avi_file *file, int channel, const INT16 *samples, UINT32 numsamples, UINT32 sampleskip)
{
	UINT32 sampoffset = file->soundbuf_chansamples[channel];
	UINT32 sampnum;

	/* see if we have enough room in the buffer */
	if (sampoffset + numsamples > file->soundbuf_samples)
		return AVIERR_EXCEEDED_SOUND_BUFFER;

	/* append samples to the buffer in little-endian format */
	for (sampnum = 0; sampnum < numsamples; sampnum++)
	{
		INT16 data = *samples++;
		samples += sampleskip;
		data = LITTLE_ENDIANIZE_INT16(data);
		file->soundbuf[sampoffset++ * file->info.audio_channels + channel] = data;
	}
	file->soundbuf_chansamples[channel] = sampoffset;

	/* flush any full sound chunks to disk */
	return soundbuf_flush(file, TRUE);
}


/*-------------------------------------------------
    read_chunk_data - read a chunk's data into
    memory
-------------------------------------------------*/

static avi_error read_chunk_data(avi_file *file, const avi_chunk *chunk, UINT8 **buffer)
{
	file_error filerr;
	UINT32 bytes_read;

	/* allocate memory for the data */
	*buffer = (UINT8 *)malloc(chunk->size);
	if (*buffer == NULL)
		return AVIERR_NO_MEMORY;

	/* read from the file */
	filerr = osd_read(file->file, *buffer, chunk->offset + 8, chunk->size, &bytes_read);
	if (filerr != FILERR_NONE || bytes_read != chunk->size)
	{
		free(*buffer);
		*buffer = NULL;
		return AVIERR_READ_ERROR;
	}

	return AVIERR_NONE;
}


/*-------------------------------------------------
    get_first_chunk - get information about the
    first chunk in a container
-------------------------------------------------*/

static avi_error get_first_chunk(avi_file *file, const avi_chunk *parent, avi_chunk *newchunk)
{
	UINT64 startoffset = (parent != NULL && parent->type != 0) ? parent->offset + 12 : 0;
	if (parent != NULL && parent->type != CHUNKTYPE_LIST && parent->type != CHUNKTYPE_RIFF && parent->type != 0)
		return AVIERR_INVALID_DATA;
	return get_next_chunk_internal(file, parent, newchunk, startoffset);
}


/*-------------------------------------------------
    get_next_chunk - get information about the
    next chunk in a container
-------------------------------------------------*/

static avi_error get_next_chunk(avi_file *file, const avi_chunk *parent, avi_chunk *newchunk)
{
	UINT64 nextoffset = newchunk->offset + 8 + newchunk->size + (newchunk->size & 1);
	return get_next_chunk_internal(file, parent, newchunk, nextoffset);
}


/*-------------------------------------------------
    find_first_chunk - get information about the
    first chunk of a particular type in a container
-------------------------------------------------*/

static avi_error find_first_chunk(avi_file *file, UINT32 findme, const avi_chunk *container, avi_chunk *result)
{
	avi_error avierr;

	for (avierr = get_first_chunk(file, container, result); avierr == AVIERR_NONE; avierr = get_next_chunk(file, container, result))
		if (result->type == findme)
			return AVIERR_NONE;

	return avierr;
}


/*-------------------------------------------------
    find_next_chunk - get information about the
    next chunk of a particular type in a container
-------------------------------------------------*/

static avi_error find_next_chunk(avi_file *file, UINT32 findme, const avi_chunk *container, avi_chunk *result)
{
	avi_error avierr;

	for (avierr = get_next_chunk(file, container, result); avierr == AVIERR_NONE; avierr = get_next_chunk(file, container, result))
		if (result->type == findme)
			return AVIERR_NONE;

	return avierr;
}


/*-------------------------------------------------
    find_first_list - get information about the
    first list of a particular type in a container
-------------------------------------------------*/

static avi_error find_first_list(avi_file *file, UINT32 findme, const avi_chunk *container, avi_chunk *result)
{
	avi_error avierr;

	for (avierr = find_first_chunk(file, CHUNKTYPE_LIST, container, result); avierr == AVIERR_NONE; avierr = find_next_chunk(file, CHUNKTYPE_LIST, container, result))
		if (result->listtype == findme)
			return AVIERR_NONE;

	return avierr;
}


/*-------------------------------------------------
    find_next_list - get information about the
    next list of a particular type in a container
-------------------------------------------------*/

static avi_error find_next_list(avi_file *file, UINT32 findme, const avi_chunk *container, avi_chunk *result)
{
	avi_error avierr;

	for (avierr = find_next_chunk(file, CHUNKTYPE_LIST, container, result); avierr == AVIERR_NONE; avierr = find_next_chunk(file, CHUNKTYPE_LIST, container, result))
		if (result->listtype == findme)
			return AVIERR_NONE;

	return avierr;
}


/*-------------------------------------------------
    get_next_chunk_internal - fetch the next
    chunk relative to the current one
-------------------------------------------------*/

static avi_error get_next_chunk_internal(avi_file *file, const avi_chunk *parent, avi_chunk *newchunk, UINT64 offset)
{
	file_error filerr;
	UINT8 buffer[12];
	UINT32 bytesread;

	/* NULL parent implies the root */
	if (parent == NULL)
		parent = &file->rootchunk;

	/* start at the current offset */
	newchunk->offset = offset;

	/* if we're past the bounds of the parent, bail */
	if (newchunk->offset + 8 >= parent->offset + 8 + parent->size)
		return AVIERR_END;

	/* read the header */
	filerr = osd_read(file->file, buffer, newchunk->offset, 8, &bytesread);
	if (filerr != FILERR_NONE || bytesread != 8)
		return AVIERR_INVALID_DATA;

	/* fill in the new chunk */
	newchunk->type = fetch_32bits(&buffer[0]);
	newchunk->size = fetch_32bits(&buffer[4]);

	/* if we are a list, fetch the list type */
	if (newchunk->type == CHUNKTYPE_LIST || newchunk->type == CHUNKTYPE_RIFF)
	{
		filerr = osd_read(file->file, &buffer[8], newchunk->offset + 8, 4, &bytesread);
		if (filerr != FILERR_NONE || bytesread != 4)
			return AVIERR_INVALID_DATA;
		newchunk->listtype = fetch_32bits(&buffer[8]);
	}

	return AVIERR_NONE;
}


/*-------------------------------------------------
    read_movie_data - get data about a movie
-------------------------------------------------*/

static avi_error read_movie_data(avi_file *file)
{
	avi_chunk riff, hdrl, avih, strl, strh, strf, indx, movi, idx1;
	avi_error avierr;
	int strindex;

	/* find the RIFF chunk */
	avierr = find_first_chunk(file, CHUNKTYPE_RIFF, NULL, &riff);
	if (avierr != AVIERR_NONE)
		goto error;

	/* verify that the RIFF type is AVI */
	if (riff.listtype != LISTTYPE_AVI)
	{
		avierr = AVIERR_INVALID_DATA;
		goto error;
	}

	/* find the hdrl LIST chunk within the RIFF */
	avierr = find_first_list(file, LISTTYPE_HDRL, &riff, &hdrl);
	if (avierr != AVIERR_NONE)
		goto error;

		/* find the avih chunk */
		avierr = find_first_chunk(file, CHUNKTYPE_AVIH, &hdrl, &avih);
		if (avierr != AVIERR_NONE)
			goto error;

		/* parse the avih chunk */
		avierr = parse_avih_chunk(file, &avih);
		if (avierr != AVIERR_NONE)
			goto error;

		/* loop over strl LIST chunks */
		strindex = 0;
		for (avierr = find_first_list(file, LISTTYPE_STRL, &hdrl, &strl); avierr == AVIERR_NONE; avierr = find_next_list(file, LISTTYPE_STRL, &hdrl, &strl))
		{
			/* if we have too many, it's a bad file */
			if (strindex >= file->streams)
				goto error;

			/* find the strh chunk */
			avierr = find_first_chunk(file, CHUNKTYPE_STRH, &strl, &strh);
			if (avierr != AVIERR_NONE)
				goto error;

			/* parse the data */
			avierr = parse_strh_chunk(file, &file->stream[strindex], &strh);
			if (avierr != AVIERR_NONE)
				goto error;

			/* find the strf chunk */
			avierr = find_first_chunk(file, CHUNKTYPE_STRF, &strl, &strf);
			if (avierr != AVIERR_NONE)
				goto error;

			/* parse the data */
			avierr = parse_strf_chunk(file, &file->stream[strindex], &strf);
			if (avierr != AVIERR_NONE)
				goto error;

			/* find the indx chunk, if present */
			avierr = find_first_chunk(file, CHUNKTYPE_INDX, &strl, &indx);
			if (avierr == AVIERR_NONE)
				avierr = parse_indx_chunk(file, &file->stream[strindex], &indx);

			/* next stream */
			strindex++;
		}

		/* normalize the error after parsing the stream headers */
		if (avierr == AVIERR_END)
			avierr = AVIERR_NONE;
		if (avierr != AVIERR_NONE)
			goto error;

	/* find the base of the movi data */
	avierr = find_first_list(file, LISTTYPE_MOVI, &riff, &movi);
	if (avierr != AVIERR_NONE)
		goto error;

	/* find and parse the idx1 chunk within the RIFF (if present) */
	avierr = find_first_chunk(file, CHUNKTYPE_IDX1, &riff, &idx1);
	if (avierr == AVIERR_NONE)
		avierr = parse_idx1_chunk(file, movi.offset + 8, &idx1);
	avierr = AVIERR_NONE;

	/* now extract the movie info */
	avierr = extract_movie_info(file);

error:
	return avierr;
}


/*-------------------------------------------------
    extract_movie_info - extract the movie info
    from the streams we've read
-------------------------------------------------*/

static avi_error extract_movie_info(avi_file *file)
{
	//avi_stream *audiostream;
	avi_stream *stream;

	/* get the video stream */
	stream = get_video_stream(file);
	if (stream != NULL)
	{
		/* fill in the info */
		file->info.video_format = stream->format;
		file->info.video_timescale = stream->rate;
		file->info.video_sampletime = stream->scale;
		file->info.video_numsamples = stream->samples;
		file->info.video_width = stream->width;
		file->info.video_height = stream->height;
	}

	/* get the first audio stream */
	stream = get_audio_stream(file, 0, NULL);
	if (stream != NULL)
	{
		/* fill in the info */
		file->info.audio_format = stream->format;
		file->info.audio_timescale = stream->rate;
		file->info.audio_sampletime = stream->scale;
		file->info.audio_numsamples = stream->samples;
		file->info.audio_channels = 1;
		file->info.audio_samplebits = stream->samplebits;
		file->info.audio_samplerate = stream->samplerate;
	}

	/* now make sure all other audio streams are valid */
	//audiostream = stream;
	while (1)
	{
		/* get the stream info */
		stream = get_audio_stream(file, file->info.audio_channels, NULL);
		if (stream == NULL)
			break;
		file->info.audio_channels++;

		/* verify compatibility */
		if (file->info.audio_format != stream->format ||
			file->info.audio_timescale != stream->rate ||
			file->info.audio_sampletime != stream->scale ||
			file->info.audio_numsamples != stream->samples ||
			file->info.audio_samplebits != stream->samplebits ||
			file->info.audio_samplerate != stream->samplerate)
			return AVIERR_INCOMPATIBLE_AUDIO_STREAMS;
	}

	return AVIERR_NONE;
}


/*-------------------------------------------------
    parse_avih_chunk - parse an avih header
    chunk
-------------------------------------------------*/

static avi_error parse_avih_chunk(avi_file *file, avi_chunk *avih)
{
	UINT8 *chunkdata = NULL;
	avi_error avierr;

	/* read the data */
	avierr = read_chunk_data(file, avih, &chunkdata);
	if (avierr != AVIERR_NONE)
		goto error;

	/* extract the data */
	file->streams = fetch_32bits(&chunkdata[24]);

	/* allocate memory for the streams */
	file->stream = (avi_stream *)malloc(sizeof(*file->stream) * file->streams);
	if (file->stream == NULL)
		goto error;
	memset(file->stream, 0, sizeof(*file->stream) * file->streams);

error:
	if (chunkdata != NULL)
		free(chunkdata);
	return avierr;
}


/*-------------------------------------------------
    parse_strh_chunk - parse a strh header
    chunk
-------------------------------------------------*/

static avi_error parse_strh_chunk(avi_file *file, avi_stream *stream, avi_chunk *strh)
{
	UINT8 *chunkdata = NULL;
	avi_error avierr;

	/* read the data */
	avierr = read_chunk_data(file, strh, &chunkdata);
	if (avierr != AVIERR_NONE)
		goto error;

	/* extract the data */
	stream->type = fetch_32bits(&chunkdata[0]);
	stream->scale = fetch_32bits(&chunkdata[20]);
	stream->rate = fetch_32bits(&chunkdata[24]);
	stream->samples = fetch_32bits(&chunkdata[32]);

error:
	if (chunkdata != NULL)
		free(chunkdata);
	return avierr;
}


/*-------------------------------------------------
    parse_strf_chunk - parse a strf header
    chunk
-------------------------------------------------*/

static avi_error parse_strf_chunk(avi_file *file, avi_stream *stream, avi_chunk *strf)
{
	UINT8 *chunkdata = NULL;
	avi_error avierr;

	/* read the data */
	avierr = read_chunk_data(file, strf, &chunkdata);
	if (avierr != AVIERR_NONE)
		goto error;

	/* audio and video streams have differing headers */
	if (stream->type == STREAMTYPE_VIDS)
	{
		stream->width = fetch_32bits(&chunkdata[4]);
		stream->height = fetch_32bits(&chunkdata[8]);
		stream->depth = fetch_16bits(&chunkdata[14]);
		stream->format = fetch_32bits(&chunkdata[16]);

		/* extra extraction for HuffYUV data */
		if (stream->format == FORMAT_HFYU && strf->size >= 56)
		{
			avierr = huffyuv_extract_tables(stream, chunkdata, strf->size);
			if (avierr != AVIERR_NONE)
				goto error;
		}
	}
	else if (stream->type == STREAMTYPE_AUDS)
	{
		stream->channels = fetch_16bits(&chunkdata[2]);
		stream->samplebits = fetch_16bits(&chunkdata[14]);
		stream->samplerate = fetch_32bits(&chunkdata[4]);
	}

error:
	if (chunkdata != NULL)
		free(chunkdata);
	return avierr;
}


/*-------------------------------------------------
    parse_indx_chunk - parse an indx chunk
-------------------------------------------------*/

static avi_error parse_indx_chunk(avi_file *file, avi_stream *stream, avi_chunk *strf)
{
	UINT32 entries, entry, id;
	UINT8 *chunkdata = NULL;
	UINT16 longs_per_entry;
	UINT8 subtype, type;
	UINT64 baseoffset;
	avi_error avierr;

	/* read the data */
	avierr = read_chunk_data(file, strf, &chunkdata);
	if (avierr != AVIERR_NONE)
		goto error;

	/* extract the data */
	longs_per_entry = fetch_16bits(&chunkdata[0]);
	subtype = chunkdata[2];
	type = chunkdata[3];
	entries = fetch_32bits(&chunkdata[4]);
	id = fetch_32bits(&chunkdata[8]);
	baseoffset = fetch_64bits(&chunkdata[12]);

	/* if this is a superindex, loop over entries and call ourselves recursively */
	if (type == AVI_INDEX_OF_INDEXES)
	{
		/* validate the size of each entry */
		if (longs_per_entry != 4)
			return AVIERR_INVALID_DATA;

		/* loop over entries and create subchunks for each */
		for (entry = 0; entry < entries; entry++)
		{
			const UINT8 *base = &chunkdata[24 + entry * 16];
			file_error filerr;
			avi_chunk subchunk;
			UINT32 bytes_read;
			UINT8 buffer[8];

			/* go read the subchunk */
			subchunk.offset = fetch_64bits(&base[0]);
			filerr = osd_read(file->file, buffer, subchunk.offset, sizeof(buffer), &bytes_read);
			if (filerr != FILERR_NONE || bytes_read != sizeof(buffer))
			{
				avierr = AVIERR_READ_ERROR;
				break;
			}

			/* parse the data */
			subchunk.type = fetch_32bits(&buffer[0]);
			subchunk.size = fetch_32bits(&buffer[4]);

			/* recursively parse each referenced chunk; stop if we hit an error */
			avierr = parse_indx_chunk(file, stream, &subchunk);
			if (avierr != AVIERR_NONE)
				break;
		}
	}

	/* otherwise, this is a standard index */
	else if (type == AVI_INDEX_OF_CHUNKS)
	{
		/* validate the size of each entry */
		if (longs_per_entry != 2 && longs_per_entry != 3)
			return AVIERR_INVALID_DATA;

		/* loop over entries and parse out the data */
		for (entry = 0; entry < entries; entry++)
		{
			const UINT8 *base = &chunkdata[24 + entry * 4 * longs_per_entry];
			UINT32 offset = fetch_32bits(&base[0]);
			UINT32 size = fetch_32bits(&base[4]);

			/* set the info for this chunk and advance */
			avierr = set_stream_chunk_info(stream, stream->chunks++, baseoffset + offset - 8, size + 8);
			if (avierr != AVIERR_NONE)
				break;
		}
	}

error:
	if (chunkdata != NULL)
		free(chunkdata);
	return avierr;
}


/*-------------------------------------------------
    parse_idx1_chunk - parse an idx1 chunk
-------------------------------------------------*/

static avi_error parse_idx1_chunk(avi_file *file, UINT64 baseoffset, avi_chunk *idx1)
{
	UINT8 *chunkdata = NULL;
	avi_error avierr;
	UINT32 entries;
	UINT32 entry;

	/* read the data */
	avierr = read_chunk_data(file, idx1, &chunkdata);
	if (avierr != AVIERR_NONE)
		goto error;

	/* loop over entries */
	entries = idx1->size / 16;
	for (entry = 0; entry < entries; entry++)
	{
		const UINT8 *base = &chunkdata[entry * 16];
		UINT32 chunkid = fetch_32bits(&base[0]);
		UINT32 offset = fetch_32bits(&base[8]);
		UINT32 size = fetch_32bits(&base[12]);
		avi_stream *stream;
		int streamnum;

		/* determine the stream index */
		streamnum = ((chunkid >> 8) & 0xff) - '0';
		streamnum += 10 * ((chunkid & 0xff) - '0');
		if (streamnum >= file->streams)
		{
			avierr = AVIERR_INVALID_DATA;
			goto error;
		}
		stream = &file->stream[streamnum];

		/* set the appropriate entry */
		avierr = set_stream_chunk_info(stream, stream->chunks++, baseoffset + offset, size + 8);
		if (avierr != AVIERR_NONE)
			goto error;
	}

error:
	if (chunkdata != NULL)
		free(chunkdata);
	return avierr;
}


/*-------------------------------------------------
    chunk_open - open a new chunk for writing
-------------------------------------------------*/

static avi_error chunk_open(avi_file *file, UINT32 type, UINT32 listtype, UINT32 estlength)
{
	file_error filerr;
	avi_chunk *chunk;
	UINT32 written;

	/* if we're out of stack entries, bail */
	if (file->chunksp >= ARRAY_LENGTH(file->chunkstack))
		return AVIERR_STACK_TOO_DEEP;
	chunk = &file->chunkstack[file->chunksp++];

	/* set up the chunk information */
	chunk->offset = file->writeoffs;
	chunk->size = estlength;
	chunk->type = type;
	chunk->listtype = listtype;

	/* non-list types */
	if (type != CHUNKTYPE_RIFF && type != CHUNKTYPE_LIST)
	{
		UINT8 buffer[8];

		/* populate the header */
		put_32bits(&buffer[0], chunk->type);
		put_32bits(&buffer[4], chunk->size);

		/* write the header */
		filerr = osd_write(file->file, buffer, file->writeoffs, sizeof(buffer), &written);
		if (filerr != FILERR_NONE || written != sizeof(buffer))
			return AVIERR_WRITE_ERROR;
		file->writeoffs += written;
	}

	/* list types */
	else
	{
		UINT8 buffer[12];

		/* populate the header */
		put_32bits(&buffer[0], chunk->type);
		put_32bits(&buffer[4], chunk->size);
		put_32bits(&buffer[8], chunk->listtype);

		/* write the header */
		filerr = osd_write(file->file, buffer, file->writeoffs, sizeof(buffer), &written);
		if (filerr != FILERR_NONE || written != sizeof(buffer))
			return AVIERR_WRITE_ERROR;
		file->writeoffs += written;
	}

	return AVIERR_NONE;
}


/*-------------------------------------------------
    chunk_close - finish writing an chunk
-------------------------------------------------*/

static avi_error chunk_close(avi_file *file)
{
	avi_chunk *chunk = &file->chunkstack[--file->chunksp];
	UINT64 chunksize = file->writeoffs - (chunk->offset + 8);
	UINT32 written;

	/* error if we don't fit into 32 bits */
	if (chunksize != (UINT32)chunksize)
		return AVIERR_INVALID_DATA;

	/* write the final size if it is different from the guess */
	if (chunk->size != chunksize)
	{
		file_error filerr;
		UINT8 buffer[4];

		put_32bits(&buffer[0], (UINT32)chunksize);
		filerr = osd_write(file->file, buffer, chunk->offset + 4, sizeof(buffer), &written);
		if (filerr != FILERR_NONE || written != sizeof(buffer))
			return AVIERR_WRITE_ERROR;
	}

	/* round up to the next word */
	file->writeoffs += chunksize & 1;

	return AVIERR_NONE;
}


/*-------------------------------------------------
    chunk_write - write an chunk and its data
-------------------------------------------------*/

static avi_error chunk_write(avi_file *file, UINT32 type, const void *data, UINT32 length)
{
	file_error filerr;
	avi_error avierr;
	UINT32 idxreserve;
	UINT32 written;

	/* if we are the first RIFF, we must reserve enough space for the IDX chunk */
	idxreserve = 0;
	if (file->riffbase == 0 && type != CHUNKTYPE_IDX1)
		idxreserve = compute_idx1_size(file);

	/* if we are getting too big, split the RIFF */
	/* note that we ignore writes before the current RIFF base, as those are assumed to be
       overwrites of a chunk from the previous RIFF */
	if (file->writeoffs >= file->riffbase && file->writeoffs + length + idxreserve - file->riffbase >= MAX_RIFF_SIZE)
	{
		/* close the movi list */
		avierr = chunk_close(file);
		if (avierr != AVIERR_NONE)
			return avierr;

		/* write the idx1 chunk if this is the first */
		if (file->riffbase == 0)
		{
			avierr = write_idx1_chunk(file);
			if (avierr != AVIERR_NONE)
				return avierr;
		}

		/* close the RIFF */
		avierr = chunk_close(file);
		if (avierr != AVIERR_NONE)
			return avierr;

		/* open a new RIFF */
		file->riffbase = file->writeoffs;
		avierr = chunk_open(file, CHUNKTYPE_RIFF, LISTTYPE_AVIX, 0);
		if (avierr != AVIERR_NONE)
			return avierr;

		/* open a nested movi list */
		file->saved_movi_offset = file->writeoffs;
		avierr = chunk_open(file, CHUNKTYPE_LIST, LISTTYPE_MOVI, 0);
		if (avierr != AVIERR_NONE)
			return avierr;
	}

	/* open the chunk */
	avierr = chunk_open(file, type, 0, length);
	if (avierr != AVIERR_NONE)
		return avierr;

	/* write the data */
	filerr = osd_write(file->file, data, file->writeoffs, length, &written);
	if (filerr != FILERR_NONE || written != length)
		return AVIERR_WRITE_ERROR;
	file->writeoffs += written;

	/* close the chunk */
	return chunk_close(file);
}


/*-------------------------------------------------
    chunk_overwrite - write a chunk in two passes;
    first pass writes to the end of file and
    saves the offset; second pass overwrites the
    original
-------------------------------------------------*/

static avi_error chunk_overwrite(avi_file *file, UINT32 type, const void *data, UINT32 length, UINT64 *offset, int initial_write)
{
	UINT64 savedoffset = 0;
	avi_error avierr;

	/* if this is our initial write, save the offset */
	if (initial_write)
		*offset = file->writeoffs;

	/* otherwise, remember the current write offset and replace it with the original */
	else
	{
		savedoffset = file->writeoffs;
		file->writeoffs = *offset;
	}

	/* write the chunk */
	avierr = chunk_write(file, type, data, length);

	/* if this isn't the initial write, restore the previous offset */
	if (!initial_write)
		file->writeoffs = savedoffset;

	return avierr;
}


/*-------------------------------------------------
    write_initial_headers - write out the inital
    set of AVI and stream headers
-------------------------------------------------*/

static avi_error write_initial_headers(avi_file *file)
{
	avi_error avierr;
	int strnum;

	/* reset the write pointer */
	file->writeoffs = 0;

	/* open a RIFF chunk */
	avierr = chunk_open(file, CHUNKTYPE_RIFF, LISTTYPE_AVI, 0);
	if (avierr != AVIERR_NONE)
		return avierr;

	/* open a hdlr LIST */
	avierr = chunk_open(file, CHUNKTYPE_LIST, LISTTYPE_HDRL, 0);
	if (avierr != AVIERR_NONE)
		return avierr;

	/* write an avih chunk */
	avierr = write_avih_chunk(file, TRUE);
	if (avierr != AVIERR_NONE)
		return avierr;

	/* for each stream, write a strl LIST */
	for (strnum = 0; strnum < file->streams; strnum++)
	{
		/* open a strl LIST */
		avierr = chunk_open(file, CHUNKTYPE_LIST, LISTTYPE_STRL, 0);
		if (avierr != AVIERR_NONE)
			return avierr;

		/* write the strh chunk */
		avierr = write_strh_chunk(file, &file->stream[strnum], TRUE);
		if (avierr != AVIERR_NONE)
			return avierr;

		/* write the strf chunk */
		avierr = write_strf_chunk(file, &file->stream[strnum]);
		if (avierr != AVIERR_NONE)
			return avierr;

		/* write the indx chunk */
		avierr = write_indx_chunk(file, &file->stream[strnum], TRUE);
		if (avierr != AVIERR_NONE)
			return avierr;

		/* close the strl LIST */
		avierr = chunk_close(file);
		if (avierr != AVIERR_NONE)
			return avierr;
	}

	/* close the hdlr LIST */
	avierr = chunk_close(file);
	if (avierr != AVIERR_NONE)
		return avierr;

	/* open a movi LIST */
	file->saved_movi_offset = file->writeoffs;
	avierr = chunk_open(file, CHUNKTYPE_LIST, LISTTYPE_MOVI, 0);
	if (avierr != AVIERR_NONE)
		return avierr;

	return avierr;
}


/*-------------------------------------------------
    write_avih_chunk - write the avih header
    chunk
-------------------------------------------------*/

static avi_error write_avih_chunk(avi_file *file, int initial_write)
{
	avi_stream *video = get_video_stream(file);
	UINT8 buffer[56];

	/* reset the buffer */
	memset(buffer, 0, sizeof(buffer));

	put_32bits(&buffer[0], 1000000 * (INT64)video->scale / video->rate); /* dwMicroSecPerFrame */
	put_32bits(&buffer[12], AVIF_HASINDEX | AVIF_ISINTERLEAVED); /* dwFlags */
	put_32bits(&buffer[16], video->samples);			/* dwTotalFrames */
	put_32bits(&buffer[24], file->streams);				/* dwStreams */
	put_32bits(&buffer[32], video->width);				/* dwWidth */
	put_32bits(&buffer[36], video->height);				/* dwHeight */

	/* (over)write the chunk */
	return chunk_overwrite(file, CHUNKTYPE_AVIH, buffer, sizeof(buffer), &file->saved_avih_offset, initial_write);
}


/*-------------------------------------------------
    write_strh_chunk - write the strh header
    chunk
-------------------------------------------------*/

static avi_error write_strh_chunk(avi_file *file, avi_stream *stream, int initial_write)
{
	UINT8 buffer[56];

	/* reset the buffer */
	memset(buffer, 0, sizeof(buffer));

	put_32bits(&buffer[0], stream->type);				/* fccType */
	put_32bits(&buffer[20], stream->scale);				/* dwScale */
	put_32bits(&buffer[24], stream->rate);				/* dwRate */
	put_32bits(&buffer[32], stream->samples);			/* dwLength */
	put_32bits(&buffer[40], 10000);						/* dwQuality */

	/* video-stream specific data */
	if (stream->type == STREAMTYPE_VIDS)
	{
		put_32bits(&buffer[4],							/* fccHandler */
					(stream->format == FORMAT_HFYU) ? HANDLER_HFYU : HANDLER_DIB);
		put_32bits(&buffer[36],							/* dwSuggestedBufferSize */
					stream->width * stream->height * 4);
		put_16bits(&buffer[52], stream->width);			/* rcFrame.right */
		put_16bits(&buffer[54], stream->height);		/* rcFrame.bottom */
	}

	/* audio-stream specific data */
	if (stream->type == STREAMTYPE_AUDS)
	{
		put_32bits(&buffer[36],							/* dwSuggestedBufferSize */
					stream->samplerate * stream->channels * (stream->samplebits / 8));
		put_32bits(&buffer[44], 						/* dwSampleSize */
					stream->channels * (stream->samplebits / 8));
	}

	/* write the chunk */
	return chunk_overwrite(file, CHUNKTYPE_STRH, buffer, sizeof(buffer), &stream->saved_strh_offset, initial_write);
}


/*-------------------------------------------------
    write_strf_chunk - write the strf header
    chunk
-------------------------------------------------*/

static avi_error write_strf_chunk(avi_file *file, avi_stream *stream)
{
	/* video stream */
	if (stream->type == STREAMTYPE_VIDS)
	{
		UINT8 buffer[40];

		/* reset the buffer */
		memset(buffer, 0, sizeof(buffer));

		put_32bits(&buffer[0], sizeof(buffer));			/* biSize */
		put_32bits(&buffer[4], stream->width);			/* biWidth */
		put_32bits(&buffer[8], stream->height);			/* biHeight */
		put_16bits(&buffer[12], 1);						/* biPlanes */
		put_16bits(&buffer[14], stream->depth);			/* biBitCount */
		put_32bits(&buffer[16], stream->format);		/* biCompression */
		put_32bits(&buffer[20], 						/* biSizeImage */
					stream->width * stream->height * (stream->depth + 7) / 8);

		/* write the chunk */
		return chunk_write(file, CHUNKTYPE_STRF, buffer, sizeof(buffer));
	}

	/* audio stream */
	if (stream->type == STREAMTYPE_AUDS)
	{
		UINT8 buffer[16];

		/* reset the buffer */
		memset(buffer, 0, sizeof(buffer));

		put_16bits(&buffer[0], 1);						/* wFormatTag */
		put_16bits(&buffer[2], stream->channels);		/* nChannels */
		put_32bits(&buffer[4], stream->samplerate);		/* nSamplesPerSec */
		put_32bits(&buffer[8],							/* nAvgBytesPerSec */
					stream->samplerate * stream->channels * (stream->samplebits / 8));
		put_16bits(&buffer[12],							/* nBlockAlign */
					stream->channels * (stream->samplebits / 8));
		put_16bits(&buffer[14], stream->samplebits);	/* wBitsPerSample */

		/* write the chunk */
		return chunk_write(file, CHUNKTYPE_STRF, buffer, sizeof(buffer));
	}

	return AVIERR_INVALID_DATA;
}


/*-------------------------------------------------
    write_indx_chunk - write the indx header
    chunk
-------------------------------------------------*/

static avi_error write_indx_chunk(avi_file *file, avi_stream *stream, int initial_write)
{
	UINT8 buffer[24 + 16 * MAX_AVI_SIZE_IN_GB / 4];
	UINT32 chunkid, indexchunkid;
	UINT32 master_entries = 0;

	/* reset the buffer */
	memset(buffer, 0, sizeof(buffer));

	/* construct the chunk ID and index chunk ID */
	chunkid = get_chunkid_for_stream(file, stream);
	indexchunkid = AVI_FOURCC('i', 'x', '0' + (stream - file->stream) / 10, '0' + (stream - file->stream) % 10);

	/* loop over chunks of 4GB and write out indexes for them first */
	if (!initial_write && file->riffbase != 0)
	{
		UINT64 currentbase;
		for (currentbase = 0; currentbase < file->writeoffs; currentbase += FOUR_GB)
		{
			UINT64 currentend = currentbase + FOUR_GB;
			UINT32 chunks_this_index = 0;
			UINT32 bytes_this_index = 0;
			avi_error avierr;
			UINT32 chunknum;
			UINT8 *tempbuf;

			/* count chunks that are in this region */
			for (chunknum = 0; chunknum < stream->chunks; chunknum++)
				if (stream->chunk[chunknum].offset >= currentbase && stream->chunk[chunknum].offset < currentend)
					chunks_this_index++;

			/* if no chunks, skip */
			if (chunks_this_index == 0)
				continue;

			/* allocate memory */
			tempbuf = (UINT8 *)malloc(24 + 8 * chunks_this_index);
			if (tempbuf == NULL)
				return AVIERR_NO_MEMORY;
			memset(tempbuf, 0, 24 + 8 * chunks_this_index);

			/* make a regular index */
			put_16bits(&tempbuf[0], 2);						/* wLongsPerEntry */
			tempbuf[2] = 0;									/* bIndexSubType */
			tempbuf[3] = AVI_INDEX_OF_CHUNKS;				/* bIndexType */
			put_32bits(&tempbuf[4], chunks_this_index);		/* nEntriesInUse */
			put_32bits(&tempbuf[8], chunkid);				/* dwChunkId */
			put_64bits(&tempbuf[12], currentbase);			/* qwBaseOffset */

			/* now fill in the indexes */
			chunks_this_index = 0;
			for (chunknum = 0; chunknum < stream->chunks; chunknum++)
				if (stream->chunk[chunknum].offset >= currentbase && stream->chunk[chunknum].offset < currentend)
				{
					put_32bits(&tempbuf[24 + 8 * chunks_this_index + 0], stream->chunk[chunknum].offset + 8 - currentbase);
					put_32bits(&tempbuf[24 + 8 * chunks_this_index + 4], stream->chunk[chunknum].length - 8);
					bytes_this_index += stream->chunk[chunknum].length;
					chunks_this_index++;
				}

			/* write the offset of this index to the master table */
			put_64bits(&buffer[24 + 16 * master_entries + 0], file->writeoffs);
			put_32bits(&buffer[24 + 16 * master_entries + 8], 24 + 8 * chunks_this_index + 8);
			if (stream->type == STREAMTYPE_VIDS)
				put_32bits(&buffer[24 + 16 * master_entries + 12], chunks_this_index);
			else if (stream->type == STREAMTYPE_AUDS)
				put_32bits(&buffer[24 + 16 * master_entries + 12], bytes_this_index / ((stream->samplebits / 8) * stream->channels));
			master_entries++;

			/* write the index */
			avierr = chunk_write(file, indexchunkid, tempbuf, 24 + 8 * chunks_this_index);
			free(tempbuf);
			if (avierr != AVIERR_NONE)
				return avierr;
		}
	}

	/* build up the master index */
	if (master_entries != 0)
	{
		put_16bits(&buffer[0], 4);						/* wLongsPerEntry */
		buffer[2] = 0;									/* bIndexSubType */
		buffer[3] = AVI_INDEX_OF_INDEXES;				/* bIndexType */
		put_32bits(&buffer[4], master_entries);			/* nEntriesInUse */
		put_32bits(&buffer[8], chunkid);				/* dwChunkId */
	}

	/* (over)write the chunk */
	return chunk_overwrite(file, (master_entries == 0) ? CHUNKTYPE_JUNK : CHUNKTYPE_INDX, buffer, sizeof(buffer), &stream->saved_indx_offset, initial_write);
}


/*-------------------------------------------------
    write_idx1_chunk - write the idx1 chunk
-------------------------------------------------*/

static avi_error write_idx1_chunk(avi_file *file)
{
	UINT32 tempbuflength = compute_idx1_size(file) - 8;
	UINT32 curchunk[2] = { 0 };
	UINT32 curoffset;
	avi_error avierr;
	UINT8 *tempbuf;

	/* allocate a temporary buffer */
	tempbuf = (UINT8 *)malloc(tempbuflength);
	if (tempbuf == NULL)
		return AVIERR_NO_MEMORY;

	/* fill it in */
	for (curoffset = 0; curoffset < tempbuflength; curoffset += 16)
	{
		UINT64 minoffset = ~(UINT64)0;
		int strnum, minstr = 0;

		/* determine which stream has the next chunk */
		for (strnum = 0; strnum < file->streams; strnum++)
			if (curchunk[strnum] < file->stream[strnum].chunks && file->stream[strnum].chunk[curchunk[strnum]].offset < minoffset)
			{
				minoffset = file->stream[strnum].chunk[curchunk[strnum]].offset;
				minstr = strnum;
			}

		/* make an entry for this index */
		put_32bits(&tempbuf[curoffset + 0], get_chunkid_for_stream(file, &file->stream[minstr]));
		put_32bits(&tempbuf[curoffset + 4], 0x0010 /* AVIIF_KEYFRAME */);
		put_32bits(&tempbuf[curoffset + 8], minoffset - (file->saved_movi_offset + 8));
		put_32bits(&tempbuf[curoffset + 12], file->stream[minstr].chunk[curchunk[minstr]].length - 8);

		/* advance the chunk counter for this stream */
		curchunk[minstr]++;
	}

	/* write the chunk */
	avierr = chunk_write(file, CHUNKTYPE_IDX1, tempbuf, tempbuflength);
	free(tempbuf);
	return avierr;
}


/*-------------------------------------------------
    soundbuf_initialize - initialize the sound
    buffering system
-------------------------------------------------*/

static avi_error soundbuf_initialize(avi_file *file)
{
	avi_stream *audio = get_audio_stream(file, 0, NULL);
	avi_stream *video = get_video_stream(file);

	/* we require a video stream */
	if (video == NULL)
		return AVIERR_UNSUPPORTED_VIDEO_FORMAT;

	/* skip if no audio stream */
	if (audio == NULL)
		return AVIERR_NONE;

	/* determine the number of samples we want in our buffer; 2 seconds should be enough */
	file->soundbuf_samples = file->info.audio_samplerate * SOUND_BUFFER_MSEC / 1000;

	/* allocate a buffer */
	file->soundbuf = (INT16 *)malloc(file->soundbuf_samples * file->info.audio_channels * sizeof(file->soundbuf[0]));
	if (file->soundbuf == NULL)
		return AVIERR_NO_MEMORY;
	memset(file->soundbuf, 0, file->soundbuf_samples * file->info.audio_channels * sizeof(file->soundbuf[0]));

	/* determine the number of frames to be ahead (0.75secs) */
	file->soundbuf_frames = ((UINT64)video->rate * 75) / ((UINT64)video->scale * 100) + 1;
	return AVIERR_NONE;
}


/*-------------------------------------------------
    soundbuf_write_chunk - write the initial
    chunk data
-------------------------------------------------*/

static avi_error soundbuf_write_chunk(avi_file *file, UINT32 framenum)
{
	avi_stream *stream = get_audio_stream(file, 0, NULL);
	avi_error avierr;
	UINT32 chunknum;
	UINT32 length;

	/* skip if no audio stream */
	if (stream == NULL)
		return AVIERR_NONE;

	/* determine the length of this chunk */
	if (framenum == 0)
		length = framenum_to_samplenum(file, file->soundbuf_frames);
	else
		length = framenum_to_samplenum(file, framenum + 1 + file->soundbuf_frames) - framenum_to_samplenum(file, framenum + file->soundbuf_frames);
	length *= stream->channels * sizeof(INT16);

	/* set the info for this new chunk */
	chunknum = stream->chunks;
	avierr = set_stream_chunk_info(stream, chunknum, file->writeoffs, length + 8);
	if (avierr != AVIERR_NONE)
		return avierr;

	/* then do the initial write */
	return chunk_overwrite(file, get_chunkid_for_stream(file, stream), file->soundbuf, length, &stream->chunk[chunknum].offset, TRUE);
}


/*-------------------------------------------------
    soundbuf_flush - flush data from the sound
    buffers
-------------------------------------------------*/

static avi_error soundbuf_flush(avi_file *file, int only_flush_full)
{
	avi_stream *stream = get_audio_stream(file, 0, NULL);
	INT32 channelsamples = file->soundbuf_samples;
	INT32 processedsamples = 0;
	UINT32 bytes_per_sample;
	UINT32 finalchunks;
	avi_error avierr;
	UINT32 chunknum;
	UINT32 chunkid;
	int channel;

	/* skip if no stream */
	if (stream == NULL)
		return AVIERR_NONE;

	/* get the chunk ID for this stream */
	chunkid = get_chunkid_for_stream(file, stream);
	bytes_per_sample = stream->channels * sizeof(INT16);
	finalchunks = stream->chunks;

	/* find out how many samples we've accumulated */
	for (channel = 0; channel < stream->channels; channel++)
		channelsamples = MIN(channelsamples, file->soundbuf_chansamples[channel]);

	/* loop over pending sound chunks */
	for (chunknum = file->soundbuf_chunks; chunknum < stream->chunks; chunknum++)
	{
		avi_chunk_list *chunk = &stream->chunk[chunknum];
		UINT32 chunksamples = (chunk->length - 8) / bytes_per_sample;

		/* stop if we don't have enough to satisfy this chunk */
		if (only_flush_full && channelsamples < chunksamples)
			break;

		/* if we don't have all the samples we need, pad with 0's */
		if (channelsamples > 0 && channelsamples < chunksamples)
		{
			if (processedsamples + chunksamples > file->soundbuf_samples)
				return AVIERR_EXCEEDED_SOUND_BUFFER;
			memset(&file->soundbuf[(processedsamples + channelsamples) * stream->channels], 0, (chunksamples - channelsamples) * bytes_per_sample);
		}

		/* if we're completely out of samples, clear the buffer entirely and use the end */
		else if (channelsamples <= 0)
		{
			processedsamples = file->soundbuf_samples - chunksamples;
			memset(&file->soundbuf[processedsamples * stream->channels], 0, chunksamples * bytes_per_sample);
			chunkid = CHUNKTYPE_JUNK;
			finalchunks--;
		}

		/* copy the sample data in */
		avierr = chunk_overwrite(file, chunkid, &file->soundbuf[processedsamples * stream->channels], chunk->length - 8, &chunk->offset, FALSE);
		if (avierr != AVIERR_NONE)
			return avierr;

		/* add up the samples */
		if (channelsamples > chunksamples)
			file->info.audio_numsamples = stream->samples += chunksamples;
		else if (channelsamples > 0)
			file->info.audio_numsamples = stream->samples += channelsamples;

		/* advance past those */
		processedsamples += chunksamples;
		channelsamples -= chunksamples;
		channelsamples = MAX(0, channelsamples);
	}

	/* if we have a non-zero offset, shift everything down */
	if (processedsamples > 0)
	{
		/* first account for the samples we processed */
		memmove(&file->soundbuf[0], &file->soundbuf[processedsamples * stream->channels], (file->soundbuf_samples - processedsamples) * bytes_per_sample);
		for (channel = 0; channel < stream->channels; channel++)
			file->soundbuf_chansamples[channel] -= processedsamples;
	}

	/* update the final chunk count */
	if (!only_flush_full)
		stream->chunks = finalchunks;

	/* account for flushed chunks */
	file->soundbuf_chunks = chunknum;
	return AVIERR_NONE;
}


/*-------------------------------------------------
    rgb32_compress_to_rgb - "compress" an RGB32
    bitmap to an RGB encoded frame
-------------------------------------------------*/

static avi_error rgb32_compress_to_rgb(avi_stream *stream, const bitmap_t *bitmap, UINT8 *data, UINT32 numbytes)
{
	int height = MIN(stream->height, bitmap->height);
	int width = MIN(stream->width, bitmap->width);
	UINT8 *dataend = data + numbytes;
	int x, y;

	/* compressed video */
	for (y = 0; y < height; y++)
	{
		const UINT32 *source = (UINT32 *)bitmap->base + y * bitmap->rowpixels;
		UINT8 *dest = data + (stream->height - 1 - y) * stream->width * 3;

		for (x = 0; x < width && dest < dataend; x++)
		{
			UINT32 pix = *source++;
			*dest++ = RGB_BLUE(pix);
			*dest++ = RGB_GREEN(pix);
			*dest++ = RGB_RED(pix);
		}

		/* fill in any blank space on the right */
		for ( ; x < stream->width && dest < dataend; x++)
		{
			*dest++ = 0;
			*dest++ = 0;
			*dest++ = 0;
		}
	}

	/* fill in any blank space on the bottom */
	for ( ; y < stream->height; y++)
	{
		UINT8 *dest = data + (stream->height - 1 - y) * stream->width * 3;
		for (x = 0; x < stream->width && dest < dataend; x++)
		{
			*dest++ = 0;
			*dest++ = 0;
			*dest++ = 0;
		}
	}

	return AVIERR_NONE;
}


/*-------------------------------------------------
    yuv_decompress_to_yuy16 - decompress a YUV
    encoded frame to a YUY16 bitmap
-------------------------------------------------*/

static avi_error yuv_decompress_to_yuy16(avi_stream *stream, const UINT8 *data, UINT32 numbytes, bitmap_t *bitmap)
{
	const UINT16 *dataend = (const UINT16 *)(data + numbytes);
	int x, y;

	/* compressed video */
	for (y = 0; y < stream->height; y++)
	{
		const UINT16 *source = (const UINT16 *)data + y * stream->width;
		UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;

		/* switch off the compression */
		switch (stream->format)
		{
			case FORMAT_UYVY:
				for (x = 0; x < stream->width && source < dataend; x++)
					*dest++ = *source++;
				break;

			case FORMAT_VYUY:
			case FORMAT_YUY2:
				for (x = 0; x < stream->width && source < dataend; x++)
				{
					UINT16 pix = *source++;
					*dest++ = (pix >> 8) | (pix << 8);
				}
				break;
		}
	}

	return AVIERR_NONE;
}


/*-------------------------------------------------
    yuy16_compress_to_yuy - "compress" a YUY16
    bitmap to a YUV encoded frame
-------------------------------------------------*/

static avi_error yuy16_compress_to_yuy(avi_stream *stream, const bitmap_t *bitmap, UINT8 *data, UINT32 numbytes)
{
	const UINT16 *dataend = (const UINT16 *)(data + numbytes);
	int x, y;

	/* compressed video */
	for (y = 0; y < stream->height; y++)
	{
		const UINT16 *source = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
		UINT16 *dest = (UINT16 *)data + y * stream->width;

		/* switch off the compression */
		switch (stream->format)
		{
			case FORMAT_UYVY:
				for (x = 0; x < stream->width && dest < dataend; x++)
					*dest++ = *source++;
				break;

			case FORMAT_VYUY:
			case FORMAT_YUY2:
				for (x = 0; x < stream->width && source < dataend; x++)
				{
					UINT16 pix = *source++;
					*dest++ = (pix >> 8) | (pix << 8);
				}
				break;
		}
	}

	return AVIERR_NONE;
}


/*-------------------------------------------------
    huffyuv_extract_tables - extract HuffYUV
    tables
-------------------------------------------------*/

static avi_error huffyuv_extract_tables(avi_stream *stream, const UINT8 *chunkdata, UINT32 size)
{
	const UINT8 *chunkend = chunkdata + size;
	avi_error avierr = AVIERR_NONE;
	int tabnum;

	/* allocate memory for the data */
	stream->huffyuv = (huffyuv_data *)malloc(sizeof(*stream->huffyuv));
	if (stream->huffyuv == NULL)
	{
		avierr = AVIERR_NO_MEMORY;
		goto error;
	}

	/* extract predictor information */
	if (&chunkdata[40] >= chunkend)
		return AVIERR_INVALID_DATA;
	stream->huffyuv->predictor = chunkdata[40];

	/* make sure it's the left predictor */
	if ((stream->huffyuv->predictor & ~HUFFYUV_PREDICT_DECORR) != HUFFYUV_PREDICT_LEFT)
		return AVIERR_UNSUPPORTED_VIDEO_FORMAT;

	/* make sure it's 16bpp YUV data */
	if (chunkdata[41] != 16)
		return AVIERR_UNSUPPORTED_VIDEO_FORMAT;
	chunkdata += 44;

	/* loop over tables */
	for (tabnum = 0; tabnum < 3; tabnum++)
	{
		huffyuv_table *table = &stream->huffyuv->table[tabnum];
		UINT32 curbits, bitadd;
		UINT16 bitsat16 = 0;
		int offset = 0, bits;

		/* loop until we populate the whole table */
		while (offset < 256)
		{
			int data, shift, count, i;

			/* extract the next run */
			if (chunkdata >= chunkend)
			{
				avierr = AVIERR_INVALID_DATA;
				goto error;
			}
			data = *chunkdata++;
			shift = data & 0x1f;
			count = data >> 5;

			/* zero count means next whole byte is a count */
			if (count == 0)
			{
				if (chunkdata >= chunkend)
				{
					avierr = AVIERR_INVALID_DATA;
					goto error;
				}
				count = *chunkdata++;
			}
			for (i = 0; i < count; i++)
				table->shift[offset++] = shift;
		}

		/* now determine bit patterns and masks */
		curbits = 0;
		for (bits = 31; bits >= 0; bits--)
		{
			bitadd = 1 << (32 - bits);

			/* make sure we've cleared out all the bits below */
			if ((curbits & (bitadd - 1)) != 0)
			{
				avierr = AVIERR_INVALID_DATA;
				goto error;
			}

			/* find all entries with this shift count and assign them */
			for (offset = 0; offset < 256; offset++)
				if (table->shift[offset] == bits)
				{
					table->bits[offset] = curbits;
					table->mask[offset] = ~(bitadd - 1);
					curbits += bitadd;
				}

			/* remember the bit pattern when we complete all the 17-bit codes */
			if (bits == 17)
				bitsat16 = curbits >> 16;
		}

		/* allocate the number of extra lookup tables we need */
		if (bitsat16 > 0)
		{
			table->extralookup = (UINT16 *)malloc(bitsat16 * 65536 * sizeof(table->extralookup[0]));
			if (table->extralookup == NULL)
			{
				avierr = AVIERR_NO_MEMORY;
				goto error;
			}
			for (offset = 0; offset < bitsat16; offset++)
				table->baselookup[offset] = (offset << 8) | 0;
		}

		/* then create lookup tables */
		for (offset = 0; offset < 256; offset++)
			if (table->shift[offset] > 16)
			{
				UINT16 *tablebase = table->extralookup + (table->bits[offset] >> 16) * 65536;
				UINT32 start = table->bits[offset] & 0xffff;
				UINT32 end = start + ((1 << (32 - table->shift[offset])) - 1);
				while (start <= end)
					tablebase[start++] = (offset << 8) | (table->shift[offset] - 16);
			}
			else if (table->shift[offset] > 0)
			{
				UINT32 start = table->bits[offset] >> 16;
				UINT32 end = start + ((1 << (16 - table->shift[offset])) - 1);
				while (start <= end)
					table->baselookup[start++] = (offset << 8) | table->shift[offset];
			}
	}

error:
	if (avierr != AVIERR_NONE && stream->huffyuv != NULL)
	{
		free(stream->huffyuv);
		stream->huffyuv = NULL;
	}
	return avierr;
}


/*-------------------------------------------------
    huffyuv_decompress_to_yuy16 - decompress a
    HuffYUV-encoded frame to a YUY16 bitmap
-------------------------------------------------*/

static avi_error huffyuv_decompress_to_yuy16(avi_stream *stream, const UINT8 *data, UINT32 numbytes, bitmap_t *bitmap)
{
	huffyuv_data *huffyuv = stream->huffyuv;
	int prevlines = (stream->height > 288) ? 2 : 1;
	UINT8 lastprevy = 0, lastprevcb = 0, lastprevcr = 0;
	UINT8 lasty = 0, lastcb = 0, lastcr = 0;
	UINT8 bitsinbuffer = 0;
	UINT32 bitbuffer = 0;
	UINT32 dataoffs = 0;
	int x, y;

	/* compressed video */
	for (y = 0; y < stream->height; y++)
	{
		UINT16 *dest = BITMAP_ADDR16(bitmap, y, 0);

		/* handle the first four bytes independently */
		x = 0;
		if (y == 0)
		{
			/* first DWORD is stored as YUY2 */
			lasty = data[dataoffs++];
			lastcb = data[dataoffs++];
			*dest++ = (lasty << 8) | lastcb;
			lasty = data[dataoffs++];
			lastcr = data[dataoffs++];
			*dest++ = (lasty << 8) | lastcr;
			x = 2;
		}

		/* loop over pixels */
		for ( ; x < stream->width; x++)
		{
			huffyuv_table *ytable = &huffyuv->table[0];
			huffyuv_table *ctable = &huffyuv->table[1 + (x & 1)];
			UINT16 pixel, huffdata;
			int shift;

			/* fill up the buffer; they store little-endian DWORDs, so we XOR with 3 */
			while (bitsinbuffer <= 24 && dataoffs < numbytes)
			{
				bitbuffer |= data[dataoffs++ ^ 3] << (24 - bitsinbuffer);
				bitsinbuffer += 8;
			}

			/* look up the Y component */
			huffdata = ytable->baselookup[bitbuffer >> 16];
			shift = huffdata & 0xff;
			if (shift == 0)
			{
				bitsinbuffer -= 16;
				bitbuffer <<= 16;

				/* fill up the buffer; they store little-endian DWORDs, so we XOR with 3 */
				while (bitsinbuffer <= 24 && dataoffs < numbytes)
				{
					bitbuffer |= data[dataoffs++ ^ 3] << (24 - bitsinbuffer);
					bitsinbuffer += 8;
				}

				huffdata = ytable->extralookup[(huffdata >> 8) * 65536 + (bitbuffer >> 16)];
				shift = huffdata & 0xff;
			}
			bitsinbuffer -= shift;
			bitbuffer <<= shift;
			pixel = huffdata & 0xff00;

			/* fill up the buffer; they store little-endian DWORDs, so we XOR with 3 */
			while (bitsinbuffer <= 24 && dataoffs < numbytes)
			{
				bitbuffer |= data[dataoffs++ ^ 3] << (24 - bitsinbuffer);
				bitsinbuffer += 8;
			}

			/* look up the Cb/Cr component */
			huffdata = ctable->baselookup[bitbuffer >> 16];
			shift = huffdata & 0xff;
			if (shift == 0)
			{
				bitsinbuffer -= 16;
				bitbuffer <<= 16;

				/* fill up the buffer; they store little-endian DWORDs, so we XOR with 3 */
				while (bitsinbuffer <= 24 && dataoffs < numbytes)
				{
					bitbuffer |= data[dataoffs++ ^ 3] << (24 - bitsinbuffer);
					bitsinbuffer += 8;
				}

				huffdata = ctable->extralookup[(huffdata >> 8) * 65536 + (bitbuffer >> 16)];
				shift = huffdata & 0xff;
			}
			bitsinbuffer -= shift;
			bitbuffer <<= shift;
			*dest++ = pixel | (huffdata >> 8);
		}
	}

	/* apply deltas */
	lastprevy = lastprevcb = lastprevcr = 0;
	for (y = 0; y < stream->height; y++)
	{
		UINT16 *prevrow = BITMAP_ADDR16(bitmap, y - prevlines, 0);
		UINT16 *dest = BITMAP_ADDR16(bitmap, y, 0);

		/* handle the first four bytes independently */
		x = 0;
		if (y == 0)
		{
			/* lasty, lastcr, lastcb are set up previously */
			x = 2;
		}

		/* left predict or gradient predict */
		if ((huffyuv->predictor & ~HUFFYUV_PREDICT_DECORR) == HUFFYUV_PREDICT_LEFT ||
			((huffyuv->predictor & ~HUFFYUV_PREDICT_DECORR) == HUFFYUV_PREDICT_GRADIENT))
		{
			/* first do left deltas */
			for ( ; x < stream->width; x += 2)
			{
				UINT16 pixel0 = dest[x + 0];
				UINT16 pixel1 = dest[x + 1];

				lasty += pixel0 >> 8;
				lastcb += pixel0;
				dest[x + 0] = (lasty << 8) | lastcb;

				lasty += pixel1 >> 8;
				lastcr += pixel1;
				dest[x + 1] = (lasty << 8) | lastcr;
			}

			/* for gradient, we then add in the previous row */
			if ((huffyuv->predictor & ~HUFFYUV_PREDICT_DECORR) == HUFFYUV_PREDICT_GRADIENT && y >= prevlines)
				for (x = 0; x < stream->width; x++)
				{
					UINT16 curpix = dest[x];
					UINT16 prevpix = prevrow[x];
					UINT8 ysum = (curpix >> 8) + (prevpix >> 8);
					UINT8 csum = curpix + prevpix;
					dest[x] = (ysum << 8) | csum;
				}
		}

		/* median predict on rows > 0 */
		else if ((huffyuv->predictor & ~HUFFYUV_PREDICT_DECORR) == HUFFYUV_PREDICT_MEDIAN && y >= prevlines)
		{
			for ( ; x < stream->width; x += 2)
			{
				UINT16 prevpixel0 = prevrow[x + 0];
				UINT16 prevpixel1 = prevrow[x + 1];
				UINT16 pixel0 = dest[x + 0];
				UINT16 pixel1 = dest[x + 1];
				UINT8 a, b, c;

				/* compute previous, above, and (prev + above - above-left) */
				a = lasty;
				b = prevpixel0 >> 8;
				c = lastprevy;
				lastprevy = b;
				if (a > b) { UINT8 tmp = a; a = b; b = tmp; }
				if (a > c) { UINT8 tmp = a; a = c; c = tmp; }
				if (b > c) { UINT8 tmp = b; b = c; c = tmp; }
				lasty = (pixel0 >> 8) + b;

				/* compute previous, above, and (prev + above - above-left) */
				a = lastcb;
				b = prevpixel0 & 0xff;
				c = lastprevcb;
				lastprevcb = b;
				if (a > b) { UINT8 tmp = a; a = b; b = tmp; }
				if (a > c) { UINT8 tmp = a; a = c; c = tmp; }
				if (b > c) { UINT8 tmp = b; b = c; c = tmp; }
				lastcb = (pixel0 & 0xff) + b;
				dest[x + 0] = (lasty << 8) | lastcb;

				/* compute previous, above, and (prev + above - above-left) */
				a = lasty;
				b = prevpixel1 >> 8;
				c = lastprevy;
				lastprevy = b;
				if (a > b) { UINT8 tmp = a; a = b; b = tmp; }
				if (a > c) { UINT8 tmp = a; a = c; c = tmp; }
				if (b > c) { UINT8 tmp = b; b = c; c = tmp; }
				lasty = (pixel1 >> 8) + b;

				/* compute previous, above, and (prev + above - above-left) */
				a = lastcr;
				b = prevpixel1 & 0xff;
				c = lastprevcr;
				lastprevcr = b;
				if (a > b) { UINT8 tmp = a; a = b; b = tmp; }
				if (a > c) { UINT8 tmp = a; a = c; c = tmp; }
				if (b > c) { UINT8 tmp = b; b = c; c = tmp; }
				lastcr = (pixel1 & 0xff) + b;
				dest[x + 1] = (lasty << 8) | lastcr;
			}
		}
	}

	return AVIERR_NONE;
}


static void u64toa(UINT64 val, char *output)
{
	UINT32 lo = (UINT32)(val & 0xffffffff);
	UINT32 hi = (UINT32)(val >> 32);
	if (hi != 0)
		sprintf(output, "%X%08X", hi, lo);
	else
		sprintf(output, "%X", lo);
}


/*-------------------------------------------------
    printf_chunk_recursive - print information
    about a chunk recursively
-------------------------------------------------*/

static void printf_chunk_recursive(avi_file *file, avi_chunk *container, int indent)
{
	char size[20], offset[20];
	avi_chunk curchunk;
	int avierr;

	/* iterate over chunks in this container */
	for (avierr = get_first_chunk(file, container, &curchunk); avierr == AVIERR_NONE; avierr = get_next_chunk(file, container, &curchunk))
	{
		UINT32 chunksize = curchunk.size;
		int recurse = FALSE;

		u64toa(curchunk.size, size);
		u64toa(curchunk.offset, offset);
		printf("%*schunk = %c%c%c%c, size=%s (%s)\n", indent, "",
				(UINT8)(curchunk.type >> 0),
				(UINT8)(curchunk.type >> 8),
				(UINT8)(curchunk.type >> 16),
				(UINT8)(curchunk.type >> 24),
				size, offset);

		/* certain chunks are just containers; recurse into them */
		switch (curchunk.type)
		{
			/* basic containers */
			case CHUNKTYPE_RIFF:
			case CHUNKTYPE_LIST:
				printf("%*stype = %c%c%c%c\n", indent, "",
						(UINT8)(curchunk.listtype >> 0),
						(UINT8)(curchunk.listtype >> 8),
						(UINT8)(curchunk.listtype >> 16),
						(UINT8)(curchunk.listtype >> 24));
				recurse = TRUE;
				chunksize = 0;
				break;
		}

		/* print data within the chunk */
		if (chunksize > 0 && curchunk.size < 1024 * 1024)
		{
			UINT8 *data = NULL;
			int i;

			/* read the data for a chunk */
			avierr = read_chunk_data(file, &curchunk, &data);
			if (avierr == AVIERR_NONE)
			{
				int bytes = MIN(512, chunksize);
				for (i = 0; i < bytes; i++)
				{
					if (i % 16 == 0) printf("%*s   ", indent, "");
					printf("%02X ", data[i]);
					if (i % 16 == 15) printf("\n");
				}
				if (chunksize % 16 != 0) printf("\n");
				free(data);
			}
		}

		/* if we're recursing, dive down */
		if (recurse)
			printf_chunk_recursive(file, &curchunk, indent + 4);
	}

	/* if we didn't get a legitimate error, indicate that */
	if (avierr != AVIERR_END)
		printf("[chunk error %d]\n", avierr);
}
