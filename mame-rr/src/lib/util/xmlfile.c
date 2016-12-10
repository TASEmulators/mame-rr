/***************************************************************************

    xmlfile.c

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

#include "xmlfile.h"
#include <ctype.h>
#include <expat.h>


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define TEMP_BUFFER_SIZE		4096



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _xml_parse_info xml_parse_info;
struct _xml_parse_info
{
	XML_Parser			parser;
	xml_data_node *		rootnode;
	xml_data_node *		curnode;
	UINT32				flags;
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* expat interfaces */
static int expat_setup_parser(xml_parse_info *parse_info, xml_parse_options *opts);
static void expat_element_start(void *data, const XML_Char *name, const XML_Char **attributes);
static void expat_data(void *data, const XML_Char *s, int len);
static void expat_element_end(void *data, const XML_Char *name);

/* node/attributes additions */
static xml_data_node *add_child(xml_data_node *parent, const char *name, const char *value);
static xml_attribute_node *add_attribute(xml_data_node *node, const char *name, const char *value);

/* recursive tree operations */
static void write_node_recursive(xml_data_node *node, int indent, core_file *file);
static void free_node_recursive(xml_data_node *node);



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    copystring - make an allocated copy of a
    string
-------------------------------------------------*/

static const char *copystring(const char *input)
{
	char *newstr;

	/* NULL just passes through */
	if (input == NULL)
		return NULL;

	/* make a lower-case copy if the allocation worked */
	newstr = (char *)malloc(strlen(input) + 1);
	if (newstr != NULL)
		strcpy(newstr, input);

	return newstr;
}


/*-------------------------------------------------
    copystring_lower - make an allocated copy of
    a string and convert it to lowercase along
    the way
-------------------------------------------------*/

static const char *copystring_lower(const char *input)
{
	char *newstr;
	int i;

	/* NULL just passes through */
	if (input == NULL)
		return NULL;

	/* make a lower-case copy if the allocation worked */
	newstr = (char *)malloc(strlen(input) + 1);
	if (newstr != NULL)
	{
		for (i = 0; input[i] != 0; i++)
			newstr[i] = tolower((UINT8)input[i]);
		newstr[i] = 0;
	}

	return newstr;
}



/***************************************************************************
    XML FILE OBJECTS
***************************************************************************/

/*-------------------------------------------------
    xml_file_create - create a new xml file
    object
-------------------------------------------------*/

xml_data_node *xml_file_create(void)
{
	xml_data_node *rootnode;

	/* create a root node */
	rootnode = (xml_data_node *)malloc(sizeof(*rootnode));
	if (rootnode == NULL)
		return NULL;
	memset(rootnode, 0, sizeof(*rootnode));
	return rootnode;
}


/*-------------------------------------------------
    xml_file_read - parse an XML file into its
    nodes
-------------------------------------------------*/

xml_data_node *xml_file_read(core_file *file, xml_parse_options *opts)
{
	xml_parse_info parse_info;
	int done;

	/* set up the parser */
	if (!expat_setup_parser(&parse_info, opts))
		return NULL;

	/* loop through the file and parse it */
	do
	{
		char tempbuf[TEMP_BUFFER_SIZE];

		/* read as much as we can */
		int bytes = core_fread(file, tempbuf, sizeof(tempbuf));
		done = core_feof(file);

		/* parse the data */
		if (XML_Parse(parse_info.parser, tempbuf, bytes, done) == XML_STATUS_ERROR)
		{
			if (opts != NULL && opts->error != NULL)
			{
				opts->error->error_message = XML_ErrorString(XML_GetErrorCode(parse_info.parser));
				opts->error->error_line = XML_GetCurrentLineNumber(parse_info.parser);
				opts->error->error_column = XML_GetCurrentColumnNumber(parse_info.parser);
			}

			xml_file_free(parse_info.rootnode);
			XML_ParserFree(parse_info.parser);
			return NULL;
		}

	} while (!done);

	/* free the parser */
	XML_ParserFree(parse_info.parser);

	/* return the root node */
	return parse_info.rootnode;
}


/*-------------------------------------------------
    xml_string_read - parse an XML string into its
    nodes
-------------------------------------------------*/

xml_data_node *xml_string_read(const char *string, xml_parse_options *opts)
{
	xml_parse_info parse_info;
	int length = (int)strlen(string);

	/* set up the parser */
	if (!expat_setup_parser(&parse_info, opts))
		return NULL;

	/* parse the data */
	if (XML_Parse(parse_info.parser, string, length, TRUE) == XML_STATUS_ERROR)
	{
		if (opts != NULL && opts->error != NULL)
		{
			opts->error->error_message = XML_ErrorString(XML_GetErrorCode(parse_info.parser));
			opts->error->error_line = XML_GetCurrentLineNumber(parse_info.parser);
			opts->error->error_column = XML_GetCurrentColumnNumber(parse_info.parser);
		}

		xml_file_free(parse_info.rootnode);
		XML_ParserFree(parse_info.parser);
		return NULL;
	}

	/* free the parser */
	XML_ParserFree(parse_info.parser);

	/* return the root node */
	return parse_info.rootnode;
}


/*-------------------------------------------------
    xml_file_write - write an XML tree to a file
-------------------------------------------------*/

void xml_file_write(xml_data_node *node, core_file *file)
{
	/* ensure this is a root node */
	if (node->name != NULL)
		return;

	/* output a simple header */
	core_fprintf(file, "<?xml version=\"1.0\"?>\n");
	core_fprintf(file, "<!-- This file is autogenerated; comments and unknown tags will be stripped -->\n");

	/* loop over children of the root and output */
	for (node = node->child; node; node = node->next)
		write_node_recursive(node, 0, file);
}


/*-------------------------------------------------
    xml_file_free - free an XML file object
-------------------------------------------------*/

void xml_file_free(xml_data_node *node)
{
	/* ensure this is a root node */
	if (node->name != NULL)
		return;

	free_node_recursive(node);
}



/***************************************************************************
    XML NODE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    xml_count_children - count the number of
    child nodes
-------------------------------------------------*/

int xml_count_children(xml_data_node *node)
{
	int count = 0;

	/* loop over children and count */
	for (node = node->child; node; node = node->next)
		count++;
	return count;
}


/*-------------------------------------------------
    xml_get_sibling - find the next sibling of
    the specified node with the specified tag
-------------------------------------------------*/

xml_data_node *xml_get_sibling(xml_data_node *node, const char *name)
{
	/* loop over siblings and find a matching name */
	for ( ; node; node = node->next)
		if (strcmp(node->name, name) == 0)
			return node;
	return NULL;
}


/*-------------------------------------------------
    xml_find_matching_sibling - find the next
    sibling of the specified node with the
    specified tag or attribute/value pair
-------------------------------------------------*/

xml_data_node *xml_find_matching_sibling(xml_data_node *node, const char *name, const char *attribute, const char *matchval)
{
	/* loop over siblings and find a matching attribute */
	for ( ; node; node = node->next)
	{
		/* can pass NULL as a wildcard for the node name */
		if (name == NULL || strcmp(name, node->name) == 0)
		{
			/* find a matching attribute */
			xml_attribute_node *attr = xml_get_attribute(node, attribute);
			if (attr != NULL && strcmp(attr->value, matchval) == 0)
				return node;
		}
	}
	return NULL;
}


/*-------------------------------------------------
    xml_add_child - add a new child node to the
    given node
-------------------------------------------------*/

xml_data_node *xml_add_child(xml_data_node *node, const char *name, const char *value)
{
	/* just a standard add child */
	return add_child(node, name, value);
}


/*-------------------------------------------------
    xml_get_or_add_child - find a child node of
    the specified type; if not found, add one
-------------------------------------------------*/

xml_data_node *xml_get_or_add_child(xml_data_node *node, const char *name, const char *value)
{
	xml_data_node *child;

	/* find the child first */
	child = xml_get_sibling(node->child, name);
	if (child != NULL)
		return child;

	/* if not found, do a standard add child */
	return add_child(node, name, value);
}


/*-------------------------------------------------
    xml_delete_node - delete a node and its
    children
-------------------------------------------------*/

void xml_delete_node(xml_data_node *node)
{
	xml_data_node **pnode;

	/* first unhook us from the list of children of our parent */
	for (pnode = &node->parent->child; *pnode; pnode = &(*pnode)->next)
		if (*pnode == node)
		{
			*pnode = node->next;
			break;
		}

	/* now free ourselves and our children */
	free_node_recursive(node);
}



/***************************************************************************
    XML ATTRIBUTE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    xml_get_attribute - get the value of the
    specified attribute, or NULL if not found
-------------------------------------------------*/

xml_attribute_node *xml_get_attribute(xml_data_node *node, const char *attribute)
{
	xml_attribute_node *anode;

	/* loop over attributes and find a match */
	for (anode = node->attribute; anode; anode = anode->next)
		if (strcmp(anode->name, attribute) == 0)
			return anode;
	return NULL;
}


/*-------------------------------------------------
    xml_get_attribute_string - get the string
    value of the specified attribute; if not
    found, return = the provided default
-------------------------------------------------*/

const char *xml_get_attribute_string(xml_data_node *node, const char *attribute, const char *defvalue)
{
	xml_attribute_node *attr = xml_get_attribute(node, attribute);
	return attr ? attr->value : defvalue;
}


/*-------------------------------------------------
    xml_get_attribute_int - get the integer
    value of the specified attribute; if not
    found, return = the provided default
-------------------------------------------------*/

int xml_get_attribute_int(xml_data_node *node, const char *attribute, int defvalue)
{
	const char *string = xml_get_attribute_string(node, attribute, NULL);
	int value;

	if (string == NULL)
		return defvalue;
	if (string[0] == '$')
		return (sscanf(&string[1], "%X", &value) == 1) ? value : defvalue;
	if (string[0] == '0' && string[1] == 'x')
		return (sscanf(&string[2], "%X", &value) == 1) ? value : defvalue;
	if (string[0] == '#')
		return (sscanf(&string[1], "%d", &value) == 1) ? value : defvalue;
	return (sscanf(&string[0], "%d", &value) == 1) ? value : defvalue;
}


/*-------------------------------------------------
    xml_get_attribute_int_format - return the
    format of the given integer attribute
-------------------------------------------------*/

int xml_get_attribute_int_format(xml_data_node *node, const char *attribute)
{
	const char *string = xml_get_attribute_string(node, attribute, NULL);

	if (string == NULL)
		return XML_INT_FORMAT_DECIMAL;
	if (string[0] == '$')
		return XML_INT_FORMAT_HEX_DOLLAR;
	if (string[0] == '0' && string[1] == 'x')
		return XML_INT_FORMAT_HEX_C;
	if (string[0] == '#')
		return XML_INT_FORMAT_DECIMAL_POUND;
	return XML_INT_FORMAT_DECIMAL;
}


/*-------------------------------------------------
    xml_get_attribute_float - get the float
    value of the specified attribute; if not
    found, return = the provided default
-------------------------------------------------*/

float xml_get_attribute_float(xml_data_node *node, const char *attribute, float defvalue)
{
	const char *string = xml_get_attribute_string(node, attribute, NULL);
	float value;

	if (string == NULL || sscanf(string, "%f", &value) != 1)
		return defvalue;
	return value;
}


/*-------------------------------------------------
    xml_set_attribute - set a new attribute and
    string value on the node
-------------------------------------------------*/

xml_attribute_node *xml_set_attribute(xml_data_node *node, const char *name, const char *value)
{
	xml_attribute_node *anode;

	/* first find an existing one to replace */
	anode = xml_get_attribute(node, name);

	/* if we found it, free the old value and replace it */
	if (anode != NULL)
	{
		if (anode->value != NULL)
			free((void *)anode->value);
		anode->value = copystring(value);
	}

	/* otherwise, create a new node */
	else
		anode = add_attribute(node, name, value);

	return anode;
}


/*-------------------------------------------------
    xml_set_attribute_int - set a new attribute and
    integer value on the node
-------------------------------------------------*/

xml_attribute_node *xml_set_attribute_int(xml_data_node *node, const char *name, int value)
{
	char buffer[100];
	sprintf(buffer, "%d", value);
	return xml_set_attribute(node, name, buffer);
}


/*-------------------------------------------------
    xml_set_attribute_int - set a new attribute and
    float value on the node
-------------------------------------------------*/

xml_attribute_node *xml_set_attribute_float(xml_data_node *node, const char *name, float value)
{
	char buffer[100];
	sprintf(buffer, "%f", value);
	return xml_set_attribute(node, name, buffer);
}



/***************************************************************************
    MISCELLANEOUS INTERFACES
***************************************************************************/

/*-------------------------------------------------
    xml_normalize_string - normalize a string
    to ensure it doesn't contain embedded tags
-------------------------------------------------*/

const char *xml_normalize_string(const char *string)
{
	static char buffer[1024];
	char *d = &buffer[0];

	if (string != NULL)
	{
		while (*string)
		{
			switch (*string)
			{
				case '\"' : d += sprintf(d, "&quot;"); break;
				case '&'  : d += sprintf(d, "&amp;"); break;
				case '<'  : d += sprintf(d, "&lt;"); break;
				case '>'  : d += sprintf(d, "&gt;"); break;
				default:
					*d++ = *string;
			}
			++string;
		}
	}
	*d++ = 0;
	return buffer;
}



/***************************************************************************
    EXPAT INTERFACES
***************************************************************************/

/*-------------------------------------------------
    expat_malloc/expat_realloc/expat_free -
    wrappers for memory allocation functions so
    that they pass through out memory tracking
    systems
-------------------------------------------------*/

static void *expat_malloc(size_t size)
{
	return malloc(size);
}

static void *expat_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

static void expat_free(void *ptr)
{
	free(ptr);
}


/*-------------------------------------------------
    expat_setup_parser - set up expat for parsing
-------------------------------------------------*/

static int expat_setup_parser(xml_parse_info *parse_info, xml_parse_options *opts)
{
	XML_Memory_Handling_Suite memcallbacks;

	/* setup parse_info structure */
	memset(parse_info, 0, sizeof(*parse_info));
	if (opts != NULL)
	{
		parse_info->flags = opts->flags;
		if (opts->error != NULL)
		{
			opts->error->error_message = NULL;
			opts->error->error_line = 0;
			opts->error->error_column = 0;
		}
	}

	/* create a root node */
	parse_info->rootnode = xml_file_create();
	if (parse_info->rootnode == NULL)
		return FALSE;
	parse_info->curnode = parse_info->rootnode;

	/* create the XML parser */
	memcallbacks.malloc_fcn = expat_malloc;
	memcallbacks.realloc_fcn = expat_realloc;
	memcallbacks.free_fcn = expat_free;
	parse_info->parser = XML_ParserCreate_MM(NULL, &memcallbacks, NULL);
	if (parse_info->parser == NULL)
	{
		free(parse_info->rootnode);
		return FALSE;
	}

	/* configure the parser */
	XML_SetElementHandler(parse_info->parser, expat_element_start, expat_element_end);
	XML_SetCharacterDataHandler(parse_info->parser, expat_data);
	XML_SetUserData(parse_info->parser, parse_info);

	/* optional parser initialization step */
	if (opts != NULL && opts->init_parser != NULL)
		(*opts->init_parser)(parse_info->parser);
	return TRUE;
}


/*-------------------------------------------------
    expat_element_start - expat callback for a new
    element
-------------------------------------------------*/

static void expat_element_start(void *data, const XML_Char *name, const XML_Char **attributes)
{
	xml_parse_info *parse_info = (xml_parse_info *) data;
	xml_data_node **curnode = &parse_info->curnode;
	xml_data_node *newnode;
	int attr;

	/* add a new child node to the current node */
	newnode = add_child(*curnode, name, NULL);
	if (newnode == NULL)
		return;

	/* remember the line number */
	newnode->line = XML_GetCurrentLineNumber(parse_info->parser);

	/* add all the attributes as well */
	for (attr = 0; attributes[attr]; attr += 2)
		add_attribute(newnode, attributes[attr+0], attributes[attr+1]);

	/* set us up as the current node */
	*curnode = newnode;
}


/*-------------------------------------------------
    expat_data - expat callback for a additional
    element data
-------------------------------------------------*/

static void expat_data(void *data, const XML_Char *s, int len)
{
	xml_parse_info *parse_info = (xml_parse_info *) data;
	xml_data_node **curnode = &parse_info->curnode;
	int oldlen = 0;
	char *newdata;

	/* if no data, skip */
	if (len == 0)
		return;

	/* determine how much data we currently have */
	if ((*curnode)->value != NULL)
		oldlen = (int)strlen((*curnode)->value);

	/* realloc */
	newdata = (char *)realloc((void *)(*curnode)->value, oldlen + len + 1);
	if (newdata == NULL)
		return;

	/* copy in the new data a NULL-terminate */
	memcpy(&newdata[oldlen], s, len);
	newdata[oldlen + len] = 0;
	(*curnode)->value = newdata;
}


/*-------------------------------------------------
    expat_element_end - expat callback for the end
    of an element
-------------------------------------------------*/

static void expat_element_end(void *data, const XML_Char *name)
{
	xml_parse_info *parse_info = (xml_parse_info *) data;
	xml_data_node **curnode = &parse_info->curnode;
	char *orig;

	/* strip leading/trailing spaces from the value data */
	orig = (char *)(*curnode)->value;
	if (orig != NULL && !(parse_info->flags & XML_PARSE_FLAG_WHITESPACE_SIGNIFICANT))
	{
		char *start = orig;
		char *end = start + strlen(start);

		/* first strip leading spaces */
		while (*start && isspace((UINT8)*start))
			start++;

		/* then strip trailing spaces */
		while (end > start && isspace((UINT8)end[-1]))
			end--;

		/* if nothing left, just free it */
		if (start == end)
		{
			free(orig);
			(*curnode)->value = NULL;
		}

		/* otherwise, memmove the data */
		else
		{
			memmove(orig, start, end - start);
			orig[end - start] = 0;
		}
	}

	/* back us up a node */
	*curnode = (*curnode)->parent;
}



/***************************************************************************
    NODE/ATTRIBUTE ADDITIONS
***************************************************************************/

/*-------------------------------------------------
    add_child - add a new node to the parent
-------------------------------------------------*/

static xml_data_node *add_child(xml_data_node *parent, const char *name, const char *value)
{
	xml_data_node **pnode;
	xml_data_node *node;

	/* new element: create a new node */
	node = (xml_data_node *)malloc(sizeof(*node));
	if (node == NULL)
		return NULL;

	/* initialize the members */
	node->next = NULL;
	node->parent = parent;
	node->child = NULL;
	node->name = copystring_lower(name);
	if (node->name == NULL)
	{
		free(node);
		return NULL;
	}
	node->value = copystring(value);
	if (node->value == NULL && value != NULL)
	{
		free((void *)node->name);
		free(node);
		return NULL;
	}
	node->attribute = NULL;

	/* add us to the end of the list of siblings */
	for (pnode = &parent->child; *pnode; pnode = &(*pnode)->next) ;
	*pnode = node;

	return node;
}


/*-------------------------------------------------
    add_attribute - add a new attribute to the
    given node
-------------------------------------------------*/

static xml_attribute_node *add_attribute(xml_data_node *node, const char *name, const char *value)
{
	xml_attribute_node *anode, **panode;

	/* allocate a new attribute node */
	anode = (xml_attribute_node *)malloc(sizeof(*anode));
	if (anode == NULL)
		return NULL;

	/* fill it in */
	anode->next = NULL;
	anode->name = copystring_lower(name);
	if (anode->name == NULL)
	{
		free(anode);
		return NULL;
	}
	anode->value = copystring(value);
	if (anode->value == NULL)
	{
		free((void *)anode->name);
		free(anode);
		return NULL;
	}

	/* add us to the end of the list of attributes */
	for (panode = &node->attribute; *panode; panode = &(*panode)->next) ;
	*panode = anode;

	return anode;
}



/***************************************************************************
    RECURSIVE TREE OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    write_node_recursive - recursively write
    an XML node and its children to a file
-------------------------------------------------*/

static void write_node_recursive(xml_data_node *node, int indent, core_file *file)
{
	xml_attribute_node *anode;
	xml_data_node *child;

	/* output this tag */
	core_fprintf(file, "%*s<%s", indent, "", node->name);

	/* output any attributes */
	for (anode = node->attribute; anode; anode = anode->next)
		core_fprintf(file, " %s=\"%s\"", anode->name, anode->value);

	/* if there are no children and no value, end the tag here */
	if (node->child == NULL && node->value == NULL)
		core_fprintf(file, " />\n");

	/* otherwise, close this tag and output more stuff */
	else
	{
		core_fprintf(file, ">\n");

		/* if there is a value, output that here */
		if (node->value != NULL)
			core_fprintf(file, "%*s%s\n", indent + 4, "", node->value);

		/* loop over children and output them as well */
		if (node->child != NULL)
		{
			for (child = node->child; child; child = child->next)
				write_node_recursive(child, indent + 4, file);
		}

		/* write a closing tag */
		core_fprintf(file, "%*s</%s>\n", indent, "", node->name);
	}
}


/*-------------------------------------------------
    free_node_recursive - recursively free
    the data allocated to an XML node
-------------------------------------------------*/

static void free_node_recursive(xml_data_node *node)
{
	xml_attribute_node *anode, *nanode;
	xml_data_node *child, *nchild;

	/* free name/value */
	if (node->name != NULL)
		free((void *)node->name);
	if (node->value != NULL)
		free((void *)node->value);

	/* free attributes */
	for (anode = node->attribute; anode; anode = nanode)
	{
		/* free name/value */
		if (anode->name != NULL)
			free((void *)anode->name);
		if (anode->value != NULL)
			free((void *)anode->value);

		/* note the next node and free this node */
		nanode = anode->next;
		free(anode);
	}

	/* free the children */
	for (child = node->child; child; child = nchild)
	{
		/* note the next node and free this node */
		nchild = child->next;
		free_node_recursive(child);
	}

	/* finally free ourself */
	free(node);
}
