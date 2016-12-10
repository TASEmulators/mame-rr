/***************************************************************************

    unzip.c

    Functions to manipulate data within ZIP files.

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

#include "osdcore.h"
#include "unzip.h"

#include <ctype.h>
#include <stdlib.h>
#include <zlib.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* number of open files to cache */
#define ZIP_CACHE_SIZE	8

/* offsets in end of central directory structure */
#define ZIPESIG			0x00
#define ZIPEDSK			0x04
#define ZIPECEN			0x06
#define ZIPENUM			0x08
#define ZIPECENN		0x0a
#define ZIPECSZ			0x0c
#define ZIPEOFST		0x10
#define ZIPECOML		0x14
#define ZIPECOM			0x16

/* offsets in central directory entry structure */
#define ZIPCENSIG		0x00
#define ZIPCVER			0x04
#define ZIPCOS			0x05
#define	ZIPCVXT			0x06
#define	ZIPCEXOS		0x07
#define ZIPCFLG			0x08
#define ZIPCMTHD		0x0a
#define ZIPCTIM			0x0c
#define ZIPCDAT			0x0e
#define ZIPCCRC			0x10
#define ZIPCSIZ			0x14
#define ZIPCUNC			0x18
#define ZIPCFNL			0x1c
#define ZIPCXTL			0x1e
#define ZIPCCML			0x20
#define ZIPDSK			0x22
#define ZIPINT			0x24
#define ZIPEXT			0x26
#define ZIPOFST			0x2a
#define ZIPCFN			0x2e

/* offsets in local file header structure */
#define ZIPLOCSIG		0x00
#define ZIPVER			0x04
#define ZIPGENFLG		0x06
#define ZIPMTHD			0x08
#define ZIPTIME			0x0a
#define ZIPDATE			0x0c
#define ZIPCRC			0x0e
#define ZIPSIZE			0x12
#define ZIPUNCMP		0x16
#define ZIPFNLN			0x1a
#define ZIPXTRALN		0x1c
#define ZIPNAME			0x1e



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE UINT16 read_word(UINT8 *buf)
{
	return (buf[1] << 8) | buf[0];
}

INLINE UINT32 read_dword(UINT8 *buf)
{
	return (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
}



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static zip_file *zip_cache[ZIP_CACHE_SIZE];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* cache management */
static void free_zip_file(zip_file *zip);

/* ZIP file parsing */
static zip_error read_ecd(zip_file *zip);
static zip_error get_compressed_data_offset(zip_file *zip, UINT64 *offset);

/* decompression interfaces */
static zip_error decompress_data_type_0(zip_file *zip, UINT64 offset, void *buffer, UINT32 length);
static zip_error decompress_data_type_8(zip_file *zip, UINT64 offset, void *buffer, UINT32 length);



/***************************************************************************
    ZIP FILE ACCESS
***************************************************************************/

/*-------------------------------------------------
    zip_file_open - opens a ZIP file for reading
-------------------------------------------------*/

zip_error zip_file_open(const char *filename, zip_file **zip)
{
	zip_error ziperr = ZIPERR_NONE;
	file_error filerr;
	UINT32 read_length;
	zip_file *newzip;
	char *string;
	int cachenum;

	/* ensure we start with a NULL result */
	*zip = NULL;

	/* see if we are in the cache, and reopen if so */
	for (cachenum = 0; cachenum < ARRAY_LENGTH(zip_cache); cachenum++)
	{
		zip_file *cached = zip_cache[cachenum];

		/* if we have a valid entry and it matches our filename, use it and remove from the cache */
		if (cached != NULL && cached->filename != NULL && strcmp(filename, cached->filename) == 0)
		{
			*zip = cached;
			zip_cache[cachenum] = NULL;
			return ZIPERR_NONE;
		}
	}

	/* allocate memory for the zip_file structure */
	newzip = (zip_file *)malloc(sizeof(*newzip));
	if (newzip == NULL)
		return ZIPERR_OUT_OF_MEMORY;
	memset(newzip, 0, sizeof(*newzip));

	/* open the file */
	filerr = osd_open(filename, OPEN_FLAG_READ, &newzip->file, &newzip->length);
	if (filerr != FILERR_NONE)
	{
		ziperr = ZIPERR_FILE_ERROR;
		goto error;
	}

	/* read ecd data */
	ziperr = read_ecd(newzip);
	if (ziperr != ZIPERR_NONE)
		goto error;

	/* verify that we can work with this zipfile (no disk spanning allowed) */
	if (newzip->ecd.disk_number != newzip->ecd.cd_start_disk_number || newzip->ecd.cd_disk_entries != newzip->ecd.cd_total_entries)
	{
		ziperr = ZIPERR_UNSUPPORTED;
		goto error;
	}

	/* allocate memory for the central directory */
	newzip->cd = (UINT8 *)malloc(newzip->ecd.cd_size + 1);
	if (newzip->cd == NULL)
	{
		ziperr = ZIPERR_OUT_OF_MEMORY;
		goto error;
	}

	/* read the central directory */
	filerr = osd_read(newzip->file, newzip->cd, newzip->ecd.cd_start_disk_offset, newzip->ecd.cd_size, &read_length);
	if (filerr != FILERR_NONE || read_length != newzip->ecd.cd_size)
	{
		ziperr = (filerr == FILERR_NONE) ? ZIPERR_FILE_TRUNCATED : ZIPERR_FILE_ERROR;
		goto error;
	}

	/* make a copy of the filename for caching purposes */
	string = (char *)malloc(strlen(filename) + 1);
	if (string == NULL)
	{
		ziperr = ZIPERR_OUT_OF_MEMORY;
		goto error;
	}
	strcpy(string, filename);
	newzip->filename = string;
	*zip = newzip;
	return ZIPERR_NONE;

error:
	free_zip_file(newzip);
	return ziperr;
}


/*-------------------------------------------------
    zip_file_close - close a ZIP file and add it
    to the cache
-------------------------------------------------*/

void zip_file_close(zip_file *zip)
{
	int cachenum;

	/* close the open files */
	if (zip->file != NULL)
		osd_close(zip->file);
	zip->file = NULL;

	/* find the first NULL entry in the cache */
	for (cachenum = 0; cachenum < ARRAY_LENGTH(zip_cache); cachenum++)
		if (zip_cache[cachenum] == NULL)
			break;

	/* if no room left in the cache, free the bottommost entry */
	if (cachenum == ARRAY_LENGTH(zip_cache))
		free_zip_file(zip_cache[--cachenum]);

	/* move everyone else down and place us at the top */
	if (cachenum != 0)
		memmove(&zip_cache[1], &zip_cache[0], cachenum * sizeof(zip_cache[0]));
	zip_cache[0] = zip;
}


/*-------------------------------------------------
    zip_file_cache_clear - clear the ZIP file
    cache and free all memory
-------------------------------------------------*/

void zip_file_cache_clear(void)
{
	int cachenum;

	/* clear call cache entries */
	for (cachenum = 0; cachenum < ARRAY_LENGTH(zip_cache); cachenum++)
		if (zip_cache[cachenum] != NULL)
		{
			free_zip_file(zip_cache[cachenum]);
			zip_cache[cachenum] = NULL;
		}
}



/***************************************************************************
    CONTAINED FILE ACCESS
***************************************************************************/

/*-------------------------------------------------
    zip_file_first_entry - return the first entry
    in the ZIP
-------------------------------------------------*/

const zip_file_header *zip_file_first_file(zip_file *zip)
{
	/* reset the position and go from there */
	zip->cd_pos = 0;
	return zip_file_next_file(zip);
}


/*-------------------------------------------------
    zip_file_next_entry - return the next entry
    in the ZIP
-------------------------------------------------*/

const zip_file_header *zip_file_next_file(zip_file *zip)
{
	/* fix up any modified data */
	if (zip->header.raw != NULL)
	{
		zip->header.raw[ZIPCFN + zip->header.filename_length] = zip->header.saved;
		zip->header.raw = NULL;
	}

	/* if we're at or past the end, we're done */
	if (zip->cd_pos >= zip->ecd.cd_size)
		return NULL;

	/* extract file header info */
	zip->header.raw                 = zip->cd + zip->cd_pos;
	zip->header.rawlength           = ZIPCFN;
	zip->header.signature           = read_dword(zip->header.raw + ZIPCENSIG);
	zip->header.version_created     = read_word (zip->header.raw + ZIPCVER);
	zip->header.version_needed      = read_word (zip->header.raw + ZIPCVXT);
	zip->header.bit_flag            = read_word (zip->header.raw + ZIPCFLG);
	zip->header.compression         = read_word (zip->header.raw + ZIPCMTHD);
	zip->header.file_time           = read_word (zip->header.raw + ZIPCTIM);
	zip->header.file_date           = read_word (zip->header.raw + ZIPCDAT);
	zip->header.crc                 = read_dword(zip->header.raw + ZIPCCRC);
	zip->header.compressed_length   = read_dword(zip->header.raw + ZIPCSIZ);
	zip->header.uncompressed_length = read_dword(zip->header.raw + ZIPCUNC);
	zip->header.filename_length     = read_word (zip->header.raw + ZIPCFNL);
	zip->header.extra_field_length  = read_word (zip->header.raw + ZIPCXTL);
	zip->header.file_comment_length = read_word (zip->header.raw + ZIPCCML);
	zip->header.start_disk_number   = read_word (zip->header.raw + ZIPDSK);
	zip->header.internal_attributes = read_word (zip->header.raw + ZIPINT);
	zip->header.external_attributes = read_dword(zip->header.raw + ZIPEXT);
	zip->header.local_header_offset = read_dword(zip->header.raw + ZIPOFST);
	zip->header.filename            = (char *)zip->header.raw + ZIPCFN;

	/* make sure we have enough data */
	zip->header.rawlength += zip->header.filename_length;
	zip->header.rawlength += zip->header.extra_field_length;
	zip->header.rawlength += zip->header.file_comment_length;
	if (zip->cd_pos + zip->header.rawlength > zip->ecd.cd_size)
		return NULL;

	/* NULL terminate the filename */
	zip->header.saved = zip->header.raw[ZIPCFN + zip->header.filename_length];
	zip->header.raw[ZIPCFN + zip->header.filename_length] = 0;

	/* advance the position */
	zip->cd_pos += zip->header.rawlength;
	return &zip->header;
}


/*-------------------------------------------------
    zip_file_decompress - decompress a file
    from a ZIP into the target buffer
-------------------------------------------------*/

zip_error zip_file_decompress(zip_file *zip, void *buffer, UINT32 length)
{
    zip_error ziperr;
    UINT64 offset;

    /* if we don't have enough buffer, error */
    if (length < zip->header.uncompressed_length)
    	return ZIPERR_BUFFER_TOO_SMALL;

    /* make sure the info in the header aligns with what we know */
	if (zip->header.start_disk_number != zip->ecd.disk_number)
		return ZIPERR_UNSUPPORTED;

    /* get the compressed data offset */
    ziperr = get_compressed_data_offset(zip, &offset);
    if (ziperr != ZIPERR_NONE)
    	return ziperr;

    /* handle compression types */
    switch (zip->header.compression)
    {
    	case 0:
    		ziperr = decompress_data_type_0(zip, offset, buffer, length);
    		break;

		case 8:
    		ziperr = decompress_data_type_8(zip, offset, buffer, length);
    		break;

    	default:
    		ziperr = ZIPERR_UNSUPPORTED;
    		break;
    }
	return ziperr;
}



/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    free_zip_file - free all the data for a
    zip_file
-------------------------------------------------*/

static void free_zip_file(zip_file *zip)
{
	if (zip != NULL)
	{
		if (zip->file != NULL)
			osd_close(zip->file);
		if (zip->filename != NULL)
			free((void *)zip->filename);
		if (zip->ecd.raw != NULL)
			free(zip->ecd.raw);
		if (zip->cd != NULL)
			free(zip->cd);
		free(zip);
	}
}



/***************************************************************************
    ZIP FILE PARSING
***************************************************************************/

/*-------------------------------------------------
    read_ecd - read the ECD data
-------------------------------------------------*/

static zip_error read_ecd(zip_file *zip)
{
	UINT32 buflen = 1024;
	UINT8 *buffer;

	/* we may need multiple tries */
	while (buflen < 65536)
	{
		file_error error;
		UINT32 read_length;
		INT32 offset;

		/* max out the buffer length at the size of the file */
		if (buflen > zip->length)
			buflen = zip->length;

		/* allocate buffer */
		buffer = (UINT8 *)malloc(buflen + 1);
		if (buffer == NULL)
			return ZIPERR_OUT_OF_MEMORY;

		/* read in one buffers' worth of data */
		error = osd_read(zip->file, buffer, zip->length - buflen, buflen, &read_length);
		if (error != FILERR_NONE || read_length != buflen)
		{
			free(buffer);
			return ZIPERR_FILE_ERROR;
		}

		/* find the ECD signature */
		for (offset = buflen - 22; offset >= 0; offset--)
			if (buffer[offset + 0] == 'P' && buffer[offset + 1] == 'K' && buffer[offset + 2] == 0x05 && buffer[offset + 3] == 0x06)
				break;

		/* if we found it, fill out the data */
		if (offset >= 0)
		{
			/* reuse the buffer as our ECD buffer */
			zip->ecd.raw = buffer;
			zip->ecd.rawlength = buflen - offset;

			/* append a NULL terminator to the comment */
			memmove(&buffer[0], &buffer[offset], zip->ecd.rawlength);
			zip->ecd.raw[zip->ecd.rawlength] = 0;

			/* extract ecd info */
			zip->ecd.signature            = read_dword(zip->ecd.raw + ZIPESIG);
			zip->ecd.disk_number          = read_word (zip->ecd.raw + ZIPEDSK);
			zip->ecd.cd_start_disk_number = read_word (zip->ecd.raw + ZIPECEN);
			zip->ecd.cd_disk_entries      = read_word (zip->ecd.raw + ZIPENUM);
			zip->ecd.cd_total_entries     = read_word (zip->ecd.raw + ZIPECENN);
			zip->ecd.cd_size              = read_dword(zip->ecd.raw + ZIPECSZ);
			zip->ecd.cd_start_disk_offset = read_dword(zip->ecd.raw + ZIPEOFST);
			zip->ecd.comment_length       = read_word (zip->ecd.raw + ZIPECOML);
			zip->ecd.comment              = (const char *)(zip->ecd.raw + ZIPECOM);
			return ZIPERR_NONE;
		}

		/* didn't find it; free this buffer and expand our search */
		free(buffer);
		if (buflen < zip->length)
			buflen *= 2;
		else
			return ZIPERR_BAD_SIGNATURE;
	}
	return ZIPERR_OUT_OF_MEMORY;
}


/*-------------------------------------------------
    get_compressed_data_offset - return the
    offset of the compressed data
-------------------------------------------------*/

static zip_error get_compressed_data_offset(zip_file *zip, UINT64 *offset)
{
	file_error error;
	UINT32 read_length;

	/* make sure the file handle is open */
	if (zip->file == NULL)
	{
		int filerr = osd_open(zip->filename, OPEN_FLAG_READ, &zip->file, &zip->length);
		if (filerr != FILERR_NONE)
			return ZIPERR_FILE_ERROR;
	}

	/* now go read the fixed-sized part of the local file header */
	error = osd_read(zip->file, zip->buffer, zip->header.local_header_offset, ZIPNAME, &read_length);
	if (error != FILERR_NONE || read_length != ZIPNAME)
		return (error == FILERR_NONE) ? ZIPERR_FILE_TRUNCATED : ZIPERR_FILE_ERROR;

	/* compute the final offset */
	*offset = zip->header.local_header_offset + ZIPNAME;
	*offset += read_word(zip->buffer + ZIPFNLN);
	*offset += read_word(zip->buffer + ZIPXTRALN);

	return ZIPERR_NONE;
}



/***************************************************************************
    DECOMPRESSION INTERFACES
***************************************************************************/

/*-------------------------------------------------
    decompress_data_type_0 - "decompress"
    type 0 data (which is uncompressed)
-------------------------------------------------*/

static zip_error decompress_data_type_0(zip_file *zip, UINT64 offset, void *buffer, UINT32 length)
{
	file_error filerr;
	UINT32 read_length;

	/* the data is uncompressed; just read it */
	filerr = osd_read(zip->file, buffer, offset, zip->header.compressed_length, &read_length);
	if (filerr != FILERR_NONE)
		return ZIPERR_FILE_ERROR;
	else if (read_length != zip->header.compressed_length)
		return ZIPERR_FILE_TRUNCATED;
	else
		return ZIPERR_NONE;
}


/*-------------------------------------------------
    decompress_data_type_8 - decompress
    type 8 data (which is deflated)
-------------------------------------------------*/

static zip_error decompress_data_type_8(zip_file *zip, UINT64 offset, void *buffer, UINT32 length)
{
    UINT32 input_remaining = zip->header.compressed_length;
    UINT32 read_length;
    z_stream stream;
    int filerr;
    int zerr;

	/* make sure we don't need a newer mechanism */
	if (zip->header.version_needed > 0x14)
		return ZIPERR_UNSUPPORTED;

    /* reset the stream */
    memset(&stream, 0, sizeof(stream));
    stream.next_out = (Bytef *)buffer;
    stream.avail_out = length;

    /* initialize the decompressor */
    zerr = inflateInit2(&stream, -MAX_WBITS);
    if (zerr != Z_OK)
    	return ZIPERR_DECOMPRESS_ERROR;

    /* loop until we're done */
    while (1)
	{
		/* read in the next chunk of data */
		filerr = osd_read(zip->file, zip->buffer, offset, MIN(input_remaining, sizeof(zip->buffer)), &read_length);
		if (filerr != FILERR_NONE)
		{
			inflateEnd(&stream);
	    	return ZIPERR_FILE_ERROR;
	    }
		offset += read_length;

		/* if we read nothing, but still have data left, the file is truncated */
		if (read_length == 0 && input_remaining > 0)
		{
			inflateEnd(&stream);
			return ZIPERR_FILE_TRUNCATED;
		}

		/* fill out the input data */
		stream.next_in = zip->buffer;
		stream.avail_in = read_length;
		input_remaining -= read_length;

		/* add a dummy byte at end of compressed data */
		if (input_remaining == 0)
			stream.avail_in++;

		/* now inflate */
        zerr = inflate(&stream, Z_NO_FLUSH);
        if (zerr == Z_STREAM_END)
			break;
		if (zerr != Z_OK)
		{
			inflateEnd(&stream);
			return ZIPERR_DECOMPRESS_ERROR;
		}
    }

	/* finish decompression */
	zerr = inflateEnd(&stream);
	if (zerr != Z_OK)
		return ZIPERR_DECOMPRESS_ERROR;

	/* if anything looks funny, report an error */
	if (stream.avail_out > 0 || input_remaining > 0)
		return ZIPERR_DECOMPRESS_ERROR;

	return ZIPERR_NONE;
}
