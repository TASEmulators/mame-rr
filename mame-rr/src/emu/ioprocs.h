/*********************************************************************

    ioprocs.h

    File IO abstraction layer

*********************************************************************/

#ifndef IOPROCS_H
#define IOPROCS_H

#include <stdlib.h>
#include "emu.h"



/***************************************************************************

    Type definitions

***************************************************************************/

struct io_procs
{
	void (*closeproc)(void *file);
	int (*seekproc)(void *file, INT64 offset, int whence);
	size_t (*readproc)(void *file, void *buffer, size_t length);
	size_t (*writeproc)(void *file, const void *buffer, size_t length);
	UINT64 (*filesizeproc)(void *file);
};



struct io_generic
{
	const struct io_procs *procs;
	void *file;
	UINT8 filler;
};


/***************************************************************************

    Globals

***************************************************************************/

extern const struct io_procs stdio_ioprocs;
extern const struct io_procs stdio_ioprocs_noclose;



/***************************************************************************

    Prototypes

***************************************************************************/



void io_generic_close(struct io_generic *generic);
void io_generic_read(struct io_generic *generic, void *buffer, UINT64 offset, size_t length);
void io_generic_write(struct io_generic *generic, const void *buffer, UINT64 offset, size_t length);
void io_generic_write_filler(struct io_generic *generic, UINT8 filler, UINT64 offset, size_t length);
UINT64 io_generic_size(struct io_generic *generic);



#endif /* IOPROCS_H */
