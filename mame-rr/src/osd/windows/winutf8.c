//============================================================
//
//  winutf8.c - Win32 OSD core utility functions
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>

// MAMEOS headers
#include "winutf8.h"
#include "strconv.h"


//============================================================
//  win_output_debug_string_utf8
//============================================================

void win_output_debug_string_utf8(const char *string)
{
	TCHAR *t_string = tstring_from_utf8(string);
	if (t_string != NULL)
	{
		OutputDebugString(t_string);
		osd_free(t_string);
	}
}



//============================================================
//  win_get_module_file_name_utf8
//============================================================

DWORD win_get_module_file_name_utf8(HMODULE module, char *filename, DWORD size)
{
	TCHAR t_filename[MAX_PATH];
	char *utf8_filename;

	if (GetModuleFileName(module, t_filename, ARRAY_LENGTH(t_filename)) == 0)
		return 0;

	utf8_filename = utf8_from_tstring(t_filename);
	if (!utf8_filename)
		return 0;

	size = (DWORD) snprintf(filename, size, "%s", utf8_filename);
	osd_free(utf8_filename);
	return size;
}



//============================================================
//  win_set_file_attributes_utf8
//============================================================

BOOL win_set_file_attributes_utf8(const char* filename, DWORD fileattributes)
{
	BOOL result = FALSE;
	TCHAR* t_filename = tstring_from_utf8(filename);
	if( !t_filename )
		return result;

	result = SetFileAttributes(t_filename, fileattributes);

	osd_free(t_filename);

	return result;
}



//============================================================
//  win_copy_file_utf8
//============================================================

BOOL win_copy_file_utf8(const char* existingfilename, const char* newfilename, BOOL failifexists)
{
	TCHAR* t_existingfilename;
	TCHAR* t_newfilename;
	BOOL result = FALSE;

	t_existingfilename = tstring_from_utf8(existingfilename);
	if( !t_existingfilename )
		return result;

	t_newfilename = tstring_from_utf8(newfilename);
	if( !t_newfilename ) {
		osd_free(t_existingfilename);
		return result;
	}

	result = CopyFile(t_existingfilename, t_newfilename, failifexists);

	osd_free(t_newfilename);
	osd_free(t_existingfilename);

	return result;
}



//============================================================
//  win_move_file_utf8
//============================================================

BOOL win_move_file_utf8(const char* existingfilename, const char* newfilename)
{
	TCHAR* t_existingfilename;
	TCHAR* t_newfilename;
	BOOL result = FALSE;

	t_existingfilename = tstring_from_utf8(existingfilename);
	if( !t_existingfilename )
		return result;

	t_newfilename = tstring_from_utf8(newfilename);
	if( !t_newfilename ) {
		free(t_existingfilename);
		return result;
	}

	result = MoveFile(t_existingfilename, t_newfilename);

	osd_free(t_newfilename);
	osd_free(t_existingfilename);

	return result;
}



//============================================================
//  win_message_box_utf8
//============================================================

int win_message_box_utf8(HWND window, const char *text, const char *caption, UINT type)
{
	int result = IDNO;
	TCHAR *t_text = NULL;
	TCHAR *t_caption = NULL;

	if (text)
	{
		t_text = tstring_from_utf8(text);
		if (!t_text)
			goto done;
	}

	if (caption)
	{
		t_caption = tstring_from_utf8(caption);
		if (!t_caption)
			goto done;
	}

	result = MessageBox(window, t_text, t_caption, type);

done:
	if (t_text)
		osd_free(t_text);
	if (t_caption)
		osd_free(t_caption);
	return result;
}



//============================================================
//  win_set_window_text_utf8
//============================================================

BOOL win_set_window_text_utf8(HWND window, const char *text)
{
	BOOL result = FALSE;
	TCHAR *t_text = NULL;

	if (text)
	{
		t_text = tstring_from_utf8(text);
		if (!t_text)
			goto done;
	}

	result = SetWindowText(window, t_text);

done:
	if (t_text)
		osd_free(t_text);
	return result;
}



//============================================================
//  win_get_window_text_utf8
//============================================================

int win_get_window_text_utf8(HWND window, char *buffer, size_t buffer_size)
{
	int result = 0;
	char *utf8_buffer = NULL;
	TCHAR t_buffer[256];

	t_buffer[0] = '\0';

	// invoke the core Win32 API
	GetWindowText(window, t_buffer, ARRAY_LENGTH(t_buffer));

	utf8_buffer = utf8_from_tstring(t_buffer);
	if (!utf8_buffer)
		goto done;

	result = snprintf(buffer, buffer_size, "%s", utf8_buffer);

done:
	if (utf8_buffer)
		osd_free(utf8_buffer);
	return result;
}



//============================================================
//  win_create_window_ex_utf8
//============================================================

HWND win_create_window_ex_utf8(DWORD exstyle, const char* classname, const char* windowname, DWORD style,
							   int x, int y, int width, int height, HWND parent, HMENU menu,
							   HINSTANCE instance, void* param)
{
	TCHAR* t_classname;
	TCHAR* t_windowname;
	HWND result = 0;

	t_classname = tstring_from_utf8(classname);
	if( !t_classname )
		return result;

	t_windowname = tstring_from_utf8(windowname);
	if( !t_windowname ) {
		osd_free(t_classname);
		return result;
	}

	result = CreateWindowEx(exstyle, t_classname, t_windowname, style, x, y, width, height, parent,
							menu, instance, param);

	osd_free(t_windowname);
	osd_free(t_classname);

	return result;
}
