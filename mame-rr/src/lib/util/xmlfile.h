/***************************************************************************

    xmlfile.h

    XML file parsing code.

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

#ifndef __XMLFILE_H__
#define __XMLFILE_H__

#include "osdcore.h"
#include "corefile.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	XML_PARSE_FLAG_WHITESPACE_SIGNIFICANT = 1
};


enum
{
	XML_INT_FORMAT_DECIMAL,
	XML_INT_FORMAT_DECIMAL_POUND,
	XML_INT_FORMAT_HEX_DOLLAR,
	XML_INT_FORMAT_HEX_C
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* forward type declarations */
struct XML_ParserStruct;


/* a node representing an attribute */
typedef struct _xml_attribute_node xml_attribute_node;
struct _xml_attribute_node
{
	xml_attribute_node *	next;			/* pointer to next attribute node */
	const char *			name;			/* pointer to copy of tag name */
	const char *			value;			/* pointer to copy of value string */
};


/* a node representing a data item and its relationships */
typedef struct _xml_data_node xml_data_node;
struct _xml_data_node
{
	xml_data_node *			next;			/* pointer to next sibling node */
	xml_data_node *			parent;			/* pointer to parent node */
	xml_data_node *			child;			/* pointer to first child node */
	const char *			name;			/* pointer to copy of tag name */
	const char *			value;			/* pointer to copy of value string */
	xml_attribute_node *	attribute;		/* pointer to array of attribute nodes */
	int						line;			/* line number for this node's start */
};


/* extended error information from parsing */
typedef struct _xml_parse_error xml_parse_error;
struct _xml_parse_error
{
	const char *			error_message;
	int						error_line;
	int						error_column;
};


/* parsing options */
typedef struct _xml_parse_options xml_parse_options;
struct _xml_parse_options
{
	xml_parse_error *		error;
	void					(*init_parser)(struct XML_ParserStruct *parser);
	UINT32					flags;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- XML file objects ----- */

/* create a new empty xml file object */
xml_data_node *xml_file_create(void);

/* parse an XML file into its nodes */
xml_data_node *xml_file_read(core_file *file, xml_parse_options *opts);

/* parse an XML string into its nodes */
xml_data_node *xml_string_read(const char *string, xml_parse_options *opts);

/* write an XML tree to a file */
void xml_file_write(xml_data_node *node, core_file *file);

/* free an XML file object */
void xml_file_free(xml_data_node *node);



/* ----- XML node management ----- */

/* count the number of child nodes */
int xml_count_children(xml_data_node *node);

/* find the next sibling with the given tag */
xml_data_node *xml_get_sibling(xml_data_node *node, const char *name);

/* find the next sibling with the given tag and/or attribute/value pair */
xml_data_node *xml_find_matching_sibling(xml_data_node *node, const char *name, const char *attribute, const char *matchval);

/* add a new child node */
xml_data_node *xml_add_child(xml_data_node *node, const char *name, const char *value);

/* either return an existing child node or create one if it doesn't exist */
xml_data_node *xml_get_or_add_child(xml_data_node *node, const char *name, const char *value);

/* delete a node and its children */
void xml_delete_node(xml_data_node *node);



/* ----- XML attribute management ----- */

/* find an attribute node with the specified tag */
xml_attribute_node *xml_get_attribute(xml_data_node *node, const char *attribute);

/* return the string value of an attribute, or the specified default if not present */
const char *xml_get_attribute_string(xml_data_node *node, const char *attribute, const char *defvalue);

/* return the integer value of an attribute, or the specified default if not present */
int xml_get_attribute_int(xml_data_node *node, const char *attribute, int defvalue);

/* return the format of the given integer attribute */
int xml_get_attribute_int_format(xml_data_node *node, const char *attribute);

/* return the float value of an attribute, or the specified default if not present */
float xml_get_attribute_float(xml_data_node *node, const char *attribute, float defvalue);

/* set the string value of an attribute */
xml_attribute_node *xml_set_attribute(xml_data_node *node, const char *name, const char *value);

/* set the integer value of an attribute */
xml_attribute_node *xml_set_attribute_int(xml_data_node *node, const char *name, int value);

/* set the float value of an attribute */
xml_attribute_node *xml_set_attribute_float(xml_data_node *node, const char *name, float value);



/* ----- miscellaneous interfaces ----- */

/* normalize a string into something that can be written to an XML file */
const char *xml_normalize_string(const char *string);

#endif	/* __XMLFILE_H__ */
