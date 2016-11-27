/***************************************************************************

    zippath.c

    File/directory/path operations that work with ZIP files

***************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include "zippath.h"
#include "unzip.h"
#include "corestr.h"
#include "osdcore.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _zippath_returned_directory zippath_returned_directory;
struct _zippath_returned_directory
{
	zippath_returned_directory *next;
	char name[1];
};



struct _zippath_directory
{
	/* common */
	unsigned int returned_parent : 1;
	osd_directory_entry returned_entry;

	/* specific to normal directories */
	osd_directory *directory;

	/* specific to ZIP directories */
	unsigned int called_zip_first : 1;
	zip_file *zipfile;
	astring *zipprefix;
	zippath_returned_directory *returned_dirlist;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static const zip_file_header *zippath_find_sub_path(zip_file *zipfile, const char *subpath, osd_dir_entry_type *type);
static int is_zip_file(const char *path);
static int is_zip_file_separator(char c);


/***************************************************************************
    PATH OPERATIONS
***************************************************************************/

//============================================================
//  is_path_separator
//============================================================

int is_path_separator(char c)
{
	return (c == '/') || (c == '\\');
}

/*-------------------------------------------------
    parse_parent_path - parses out the parent path
-------------------------------------------------*/

static void parse_parent_path(const char *path, int *beginpos, int *endpos)
{
	int length = strlen(path);
	int pos;

	/* skip over trailing path separators */
	pos = length - 1;
	while((pos > 0) && is_path_separator(path[pos]))
		pos--;

	/* return endpos */
	if (endpos != NULL)
		*endpos = pos;

	/* now skip until we find a path separator */
	while((pos >= 0) && !is_path_separator(path[pos]))
		pos--;

	/* return beginpos */
	if (beginpos != NULL)
		*beginpos = pos;
}



/*-------------------------------------------------
    zippath_parent - retrieves the parent directory
-------------------------------------------------*/

astring *zippath_parent(astring *dst, const char *path)
{
	int pos;
	parse_parent_path(path, &pos, NULL);

	/* return the result */
	return (pos >= 0) ? astring_cpych(dst, path, pos + 1) : astring_cpyc(dst, "");
}



/*-------------------------------------------------
    zippath_parent_basename - retrieves the parent
    directory basename
-------------------------------------------------*/

astring *zippath_parent_basename(astring *dst, const char *path)
{
	int beginpos, endpos;
	parse_parent_path(path, &beginpos, &endpos);

	return astring_cpych(dst, path + beginpos + 1, endpos - beginpos);
}



/*-------------------------------------------------
    zippath_combine - combines two paths
-------------------------------------------------*/

astring *zippath_combine(astring *dst, const char *path1, const char *path2)
{
	astring *result;

	if (!strcmp(path2, "."))
	{
		result = astring_cpyc(dst, path1);
	}
	else if (!strcmp(path2, ".."))
	{
		result = zippath_parent(dst, path1);
	}
	else if (osd_is_absolute_path(path2))
	{
		result = astring_cpyc(dst, path2);
	}
	else if ((path1[0] != '\0') && !is_path_separator(path1[strlen(path1) - 1]))
	{
		result = astring_assemble_3(dst, path1, PATH_SEPARATOR, path2);
	}
	else
	{
		result = astring_assemble_2(dst, path1, path2);
	}
	return result;
}



/***************************************************************************
    FILE OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    file_error_from_zip_error - translates a
    file_error to a zip_error
-------------------------------------------------*/

static file_error file_error_from_zip_error(zip_error ziperr)
{
	file_error filerr;
	switch(ziperr)
	{
		case ZIPERR_NONE:
			filerr = FILERR_NONE;
			break;
		case ZIPERR_OUT_OF_MEMORY:
			filerr = FILERR_OUT_OF_MEMORY;
			break;
		case ZIPERR_BAD_SIGNATURE:
		case ZIPERR_DECOMPRESS_ERROR:
		case ZIPERR_FILE_TRUNCATED:
		case ZIPERR_FILE_CORRUPT:
		case ZIPERR_UNSUPPORTED:
		case ZIPERR_FILE_ERROR:
			filerr = FILERR_INVALID_DATA;
			break;
		case ZIPERR_BUFFER_TOO_SMALL:
		default:
			filerr = FILERR_FAILURE;
			break;
	}
	return filerr;
}


/*-------------------------------------------------
    create_core_file_from_zip - creates a core_file
    from a zip file entry
-------------------------------------------------*/

static file_error create_core_file_from_zip(zip_file *zip, const zip_file_header *header, core_file **file)
{
	file_error filerr;
	zip_error ziperr;
	void *ptr;

	ptr = malloc(header->uncompressed_length);
	if (ptr == NULL)
	{
		filerr = FILERR_OUT_OF_MEMORY;
		goto done;
	}

	ziperr = zip_file_decompress(zip, ptr, header->uncompressed_length);
	if (ziperr != ZIPERR_NONE)
	{
		filerr = file_error_from_zip_error(ziperr);
		goto done;
	}

	filerr = core_fopen_ram_copy(ptr, header->uncompressed_length, OPEN_FLAG_READ, file);
	if (filerr != FILERR_NONE)
		goto done;

done:
	if (ptr != NULL)
		free(ptr);
	return filerr;
}


/*-------------------------------------------------
    zippath_fopen - opens a zip path file
-------------------------------------------------*/

file_error zippath_fopen(const char *filename, UINT32 openflags, core_file **file, astring *revised_path)
{
	file_error filerr = FILERR_NOT_FOUND;
	zip_error ziperr;
	zip_file *zip = NULL;
	const zip_file_header *header;
	osd_dir_entry_type entry_type;
	astring *mainpath;
	astring *subpath;
	astring *temp;
	astring *temp2;
	char *alloc_fullpath = NULL;
	int len;

	/* first, set up the two types of paths */
	mainpath = astring_cpyc(astring_alloc(), filename);
	subpath = astring_alloc();
	temp = astring_alloc();
	temp2 = astring_alloc();
	*file = NULL;

	/* loop through */
	while((*file == NULL) && (astring_len(mainpath) > 0)
		&& ((openflags == OPEN_FLAG_READ) || (astring_len(subpath) == 0)))
	{
		/* is the mainpath a ZIP path? */
		if (is_zip_file(astring_c(mainpath)))
		{
			/* this file might be a zip file - lets take a look */
			ziperr = zip_file_open(astring_c(mainpath), &zip);
			if (ziperr == ZIPERR_NONE)
			{
				/* it is a zip file - error if we're not opening for reading */
				if (openflags != OPEN_FLAG_READ)
				{
					filerr = FILERR_ACCESS_DENIED;
					goto done;
				}

				if (astring_len(subpath) > 0)
					header = zippath_find_sub_path(zip, astring_c(subpath), &entry_type);
				else
					header = zip_file_first_file(zip);

				if (header == NULL)
				{
					filerr = FILERR_NOT_FOUND;
					goto done;
				}

				/* attempt to read the file */
				filerr = create_core_file_from_zip(zip, header, file);
				if (filerr != FILERR_NONE)
					goto done;

				/* update subpath, if appropriate */
				if (astring_len(subpath) == 0)
					astring_cpyc(subpath, header->filename);

				/* we're done */
				goto done;
			}
		}

		if (astring_len(subpath) == 0)
			filerr = core_fopen(filename, openflags, file);
		else
			filerr = FILERR_NOT_FOUND;

		/* if we errored, then go up a directory */
		if (filerr != FILERR_NONE)
		{
			/* go up a directory */
			zippath_parent(temp, astring_c(mainpath));

			/* append to the sub path */
			if (astring_len(subpath) > 0)
			{
				astring_assemble_3(temp2, astring_c(mainpath) + astring_len(temp), PATH_SEPARATOR, astring_c(subpath));
				astring_cpy(subpath, temp2);
			}
			else
			{
				astring_cpyc(subpath, astring_c(mainpath) + astring_len(temp));
			}

			/* get the new main path, truncating path separators */
			len = astring_len(temp);
			while((len > 0) && is_zip_file_separator(astring_c(temp)[len - 1]))
				len--;
			astring_cpych(mainpath, astring_c(temp), len);
		}
	}

done:
	/* store the revised path if appropriate */
	if (revised_path != NULL)
	{
		astring_cpyc(revised_path, "");
		if (filerr == FILERR_NONE)
		{
			/* cannonicalize mainpath */
			filerr = osd_get_full_path(&alloc_fullpath, astring_c(mainpath));
			if (filerr == FILERR_NONE)
			{
				if (astring_len(subpath) > 0)
					astring_assemble_3(revised_path, alloc_fullpath, PATH_SEPARATOR, astring_c(subpath));
				else
					astring_cpyc(revised_path, alloc_fullpath);
			}
		}
	}

	if (zip != NULL)
		zip_file_close(zip);
	if (mainpath != NULL)
		astring_free(mainpath);
	if (subpath != NULL)
		astring_free(subpath);
	if (temp != NULL)
		astring_free(temp);
	if (temp2 != NULL)
		astring_free(temp2);
	if (alloc_fullpath != NULL)
		osd_free(alloc_fullpath);
	return filerr;
}


/***************************************************************************
    DIRECTORY OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    is_root - tests to see if this path is the root
-------------------------------------------------*/

static int is_root(const char *path)
{
	int i = 0;

	/* skip drive letter */
	if (isalpha(path[i]) && (path[i + 1] == ':'))
		i += 2;

	/* skip path separators */
	while (is_path_separator(path[i]))
		i++;

	return path[i] == '\0';
}



/*-------------------------------------------------
    is_zip_file - tests to see if this file is a
    ZIP file
-------------------------------------------------*/

static int is_zip_file(const char *path)
{
	const char *s = strrchr(path, '.');
	return (s != NULL) && !core_stricmp(s, ".zip");
}



/*-------------------------------------------------
    is_zip_file_separator - returns whether this
    character is a path separator within a ZIP file
-------------------------------------------------*/

static int is_zip_file_separator(char c)
{
	return (c == '/') || (c == '\\');
}



/*-------------------------------------------------
    is_zip_path_separator - returns whether this
    character is a path separator within a ZIP path
-------------------------------------------------*/

static int is_zip_path_separator(char c)
{
	return is_zip_file_separator(c) || is_path_separator(c);
}



/*-------------------------------------------------
    next_path_char - lexes out the next path
    character, normalizing separators as '/'
-------------------------------------------------*/

static char next_path_char(const char *s, int *pos)
{
	char result;

	/* skip over any initial separators */
	if (*pos == 0)
	{
		while(is_zip_file_separator(s[*pos]))
			(*pos)++;
	}

	/* are we at a path separator? */
	if (is_zip_file_separator(s[*pos]))
	{
		/* skip over path separators */
		while(is_zip_file_separator(s[*pos]))
			(*pos)++;

		/* normalize as '/' */
		result = '/';
	}
	else if (s[*pos] != '\0')
	{
		/* return character */
		result = tolower(s[(*pos)++]);
	}
	else
	{
		/* return NUL */
		result = '\0';
	}
	return result;
}




/*-------------------------------------------------
    zippath_find_sub_path - attempts to identify the
    type of a sub path in a zip file
-------------------------------------------------*/

static const zip_file_header *zippath_find_sub_path(zip_file *zipfile, const char *subpath, osd_dir_entry_type *type)
{
	int i, j;
	char c1, c2, last_char;
	const zip_file_header *header;

	for (header = zip_file_first_file(zipfile); header != NULL; header = zip_file_next_file(zipfile))
	{
		/* special case */
		if (subpath == NULL)
		{
			if (type != NULL)
				*type = ENTTYPE_FILE;
			return header;
		}

		i = 0;
		j = 0;
		last_char = '/';
                while(((c1 = next_path_char(header->filename, &i)) == (c2 = next_path_char(subpath, &j))) &&
                        ( c1 != '\0' && c2 != '\0' ))
                        last_char = c2;


		if (c2 == '\0')
		{
			if (c1 == '\0')
			{
				if (type != NULL)
					*type = ENTTYPE_FILE;
				return header;
			}
			else if ((last_char == '/') || (c1 == '/'))
			{
				if (type != NULL)
					*type = ENTTYPE_DIR;
				return header;
			}
		}
	}

	if (type != NULL)
		*type = ENTTYPE_NONE;
	return NULL;
}



/*-------------------------------------------------
    zippath_resolve - separates a ZIP path out into
    true path and ZIP entry components
-------------------------------------------------*/

static file_error zippath_resolve(const char *path, osd_dir_entry_type *entry_type,
	zip_file **zipfile, astring *newpath)
{
	file_error err;
	osd_directory_entry *current_entry = NULL;
	osd_dir_entry_type current_entry_type;
	astring *apath = astring_cpyc(astring_alloc(), path);
	astring *apath_trimmed = astring_alloc();
	astring *parent = NULL;
	int went_up = FALSE;
	int i;

	/* be conservative */
	*entry_type = ENTTYPE_NONE;
	*zipfile = NULL;

	do
	{
		/* trim the path of trailing path separators */
		i = astring_len(apath);
		while((i > 1) && is_path_separator(astring_c(apath)[i - 1]))
			i--;
		apath_trimmed = astring_cpysubstr(apath_trimmed, apath, 0, i);

		/* stat the path */
		current_entry = osd_stat(astring_c(apath_trimmed));

		/* did we find anything? */
		if (current_entry != NULL)
		{
			/* get the entry type and free the stat entry */
			current_entry_type = current_entry->type;
			free(current_entry);
			current_entry = NULL;
		}
		else
		{
			/* if we have not found the file or directory, go up */
			current_entry_type = ENTTYPE_NONE;
			went_up = TRUE;
			parent = zippath_parent(astring_alloc(), astring_c(apath));
			astring_free(apath);
			apath = parent;
		}
	}
	while((current_entry_type == ENTTYPE_NONE) && (apath != NULL) && !is_root(astring_c(apath)));

	/* if we did not find anything, then error out */
	if (current_entry_type == ENTTYPE_NONE)
	{
		err = FILERR_NOT_FOUND;
		goto done;
	}

	/* is this file a ZIP file? */
	if ((current_entry_type == ENTTYPE_FILE) && is_zip_file(astring_c(apath_trimmed))
		&& (zip_file_open(astring_c(apath_trimmed), zipfile) == ZIPERR_NONE))
	{
		i = strlen(path + astring_len(apath));
		while((i > 0) && is_zip_path_separator(path[astring_len(apath) + i - 1]))
			i--;
		astring_cpych(newpath, path + astring_len(apath), i);

		/* this was a true ZIP path - attempt to identify the type of path */
		zippath_find_sub_path(*zipfile, astring_c(newpath), &current_entry_type);
		if (current_entry_type == ENTTYPE_NONE)
		{
			err = FILERR_NOT_FOUND;
			goto done;
		}
	}
	else
	{
		/* this was a normal path */
		if (went_up)
		{
			err = FILERR_NOT_FOUND;
			goto done;
		}
		astring_cpyc(newpath, path);
	}

	/* success! */
	*entry_type = current_entry_type;
	err = FILERR_NONE;

done:
	if (apath != NULL)
		astring_free(apath);
	if (apath_trimmed != NULL)
		astring_free(apath_trimmed);
	return err;
}


/*-------------------------------------------------
    zippath_opendir - opens a directory
-------------------------------------------------*/

file_error zippath_opendir(const char *path, zippath_directory **directory)
{
	file_error err;
	osd_dir_entry_type entry_type;
	astring *newpath = astring_alloc();
	zippath_directory *result;

	/* allocate a directory */
	result = (zippath_directory *) malloc(sizeof(*result));
	if (result == NULL)
	{
		err = FILERR_OUT_OF_MEMORY;
		goto done;
	}
	memset(result, 0, sizeof(*result));

	/* resolve the path */
	err = zippath_resolve(path, &entry_type, &result->zipfile, newpath);
	if (err != FILERR_NONE)
		goto done;

	/* we have to be a directory */
	if (entry_type != ENTTYPE_DIR)
	{
		err = FILERR_NOT_FOUND;
		goto done;
	}

	/* was the result a ZIP? */
	if (result->zipfile != NULL)
	{
		result->zipprefix = newpath;
		newpath = NULL;
	}
	else
	{
		/* a conventional directory */
		result->directory = osd_opendir(path);
		if (result->directory == NULL)
		{
			err = FILERR_FAILURE;
			goto done;
		}

		/* is this path the root? if so, pretend we've already returned the parent */
		if (is_root(path))
			result->returned_parent = TRUE;
	}

done:
	if (((directory == NULL) || (err != FILERR_NONE)) && (result != NULL))
	{
		zippath_closedir(result);
		result = NULL;
	}
	if (newpath != NULL)
	{
		astring_free(newpath);
		newpath = NULL;
	}
	if (directory != NULL)
		*directory = result;
	return err;
}


/*-------------------------------------------------
    zippath_closedir - closes a directory
-------------------------------------------------*/

void zippath_closedir(zippath_directory *directory)
{
	zippath_returned_directory *dirlist;

	if (directory->directory != NULL)
		osd_closedir(directory->directory);

	if (directory->zipfile != NULL)
		zip_file_close(directory->zipfile);

	if (directory->zipprefix != NULL)
		astring_free(directory->zipprefix);

	while(directory->returned_dirlist != NULL)
	{
		dirlist = directory->returned_dirlist;
		directory->returned_dirlist = directory->returned_dirlist->next;
		free(dirlist);
	}

	free(directory);
}


/*-------------------------------------------------
    get_relative_path - checks to see if a specified
    header is in the zippath_directory, and if so
    returns the relative path
-------------------------------------------------*/

static const char *get_relative_path(zippath_directory *directory, const zip_file_header *header)
{
	const char *result = NULL;
	int len = astring_len(directory->zipprefix);

	if ((len <= strlen(header->filename))
		&& !strncmp(astring_c(directory->zipprefix), header->filename, len))
	{
		result = &header->filename[len];
		while(is_zip_file_separator(*result))
			result++;
	}

	return result;
}


/*-------------------------------------------------
    zippath_readdir - reads a directory
-------------------------------------------------*/

const osd_directory_entry *zippath_readdir(zippath_directory *directory)
{
	const osd_directory_entry *result = NULL;
	const zip_file_header *header;
	const char *relpath;
	const char *separator;
	const char *s;
	zippath_returned_directory *rdent;

	if (!directory->returned_parent)
	{
		/* first thing's first - return parent directory */
		directory->returned_parent = TRUE;
		memset(&directory->returned_entry, 0, sizeof(directory->returned_entry));
		directory->returned_entry.name = "..";
		directory->returned_entry.type = ENTTYPE_DIR;
		result = &directory->returned_entry;
	}
	else if (directory->directory != NULL)
	{
		/* a normal directory read */
		do
		{
			result = osd_readdir(directory->directory);
		}
		while((result != NULL) && (!strcmp(result->name, ".") || !strcmp(result->name, "..")));

		/* special case - is this entry a ZIP file?  if so we need to return it as a "directory" */
		if ((result != NULL) && is_zip_file(result->name))
		{
			/* copy; but change the entry type */
			directory->returned_entry = *result;
			directory->returned_entry.type = ENTTYPE_DIR;
			result = &directory->returned_entry;
		}
	}
	else if (directory->zipfile != NULL)
	{
		do
		{
			/* a zip file read */
			do
			{
				if (!directory->called_zip_first)
					header = zip_file_first_file(directory->zipfile);
				else
					header = zip_file_next_file(directory->zipfile);
				directory->called_zip_first = TRUE;
				relpath = NULL;
			}
			while((header != NULL) && ((relpath = get_relative_path(directory, header)) == NULL));

			if (relpath != NULL)
			{
				/* we've found a ZIP entry; but this may be an entry deep within the target directory */
				for (s = relpath; *s && !is_zip_file_separator(*s); s++)
					;
				separator = *s ? s : NULL;

				if (separator != NULL)
				{
					/* a nested entry; loop through returned_dirlist to see if we've returned the parent directory */
					for (rdent = directory->returned_dirlist; rdent != NULL; rdent = rdent->next)
					{
						if (!core_strnicmp(rdent->name, relpath, separator - relpath))
							break;
					}

					if (rdent == NULL)
					{
						/* we've found a new directory; add this to returned_dirlist */
						rdent = (zippath_returned_directory *)malloc(sizeof(*rdent) + (separator - relpath));
						rdent->next = directory->returned_dirlist;
						memcpy(rdent->name, relpath, (separator - relpath) * sizeof(rdent->name[0]));
						rdent->name[separator - relpath] = '\0';
						directory->returned_dirlist = rdent;

						/* ...and return it */
						memset(&directory->returned_entry, 0, sizeof(directory->returned_entry));
						directory->returned_entry.name = rdent->name;
						directory->returned_entry.type = ENTTYPE_DIR;
						result = &directory->returned_entry;
					}
				}
				else
				{
					/* a real file */
					memset(&directory->returned_entry, 0, sizeof(directory->returned_entry));
					directory->returned_entry.name = relpath;
					directory->returned_entry.type = ENTTYPE_FILE;
					directory->returned_entry.size = header->uncompressed_length;
					result = &directory->returned_entry;
				}
			}
		}
		while((relpath != NULL) && (result == NULL));
	}
	return result;
}



/*-------------------------------------------------
    zippath_is_zip - returns TRUE if this path is
    a ZIP path or FALSE if not
-------------------------------------------------*/

int zippath_is_zip(zippath_directory *directory)
{
	return directory->zipfile != NULL;
}
