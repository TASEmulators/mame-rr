/*********************************************************************

    debugvw.h

    Debugger view engine.

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

#ifndef __DEBUGVIEW_H__
#define __DEBUGVIEW_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// types passed to debug_view_manager::alloc_view()
enum debug_view_type
{
	DVT_NONE,
	DVT_CONSOLE,
	DVT_STATE,
	DVT_DISASSEMBLY,
	DVT_MEMORY,
	DVT_LOG,
	DVT_TIMERS,
	DVT_ALLOCS
};


// notifications passed to view_notify()
enum debug_view_notification
{
	VIEW_NOTIFY_NONE,
	VIEW_NOTIFY_VISIBLE_CHANGED,
	VIEW_NOTIFY_CURSOR_CHANGED,
	VIEW_NOTIFY_SOURCE_CHANGED
};


// attribute bits for debug_view_char.attrib
const UINT8 DCA_NORMAL		= 0x00;		// in Windows: black on white
const UINT8 DCA_CHANGED		= 0x01;		// in Windows: red foreground
const UINT8 DCA_SELECTED	= 0x02;		// in Windows: light red background
const UINT8 DCA_INVALID		= 0x04;		// in Windows: dark blue foreground
const UINT8 DCA_DISABLED	= 0x08;		// in Windows: darker foreground
const UINT8 DCA_ANCILLARY	= 0x10;		// in Windows: grey background
const UINT8 DCA_CURRENT		= 0x20;		// in Windows: yellow background
const UINT8 DCA_COMMENT		= 0x40;		// in Windows: green foreground


// special characters that can be passed to process_char()
const int DCH_UP			= 1;		// up arrow
const int DCH_DOWN			= 2;		// down arrow
const int DCH_LEFT			= 3;		// left arrow
const int DCH_RIGHT			= 4;		// right arrow
const int DCH_PUP			= 5;		// page up
const int DCH_PDOWN			= 6;		// page down
const int DCH_HOME			= 7;		// home
const int DCH_CTRLHOME		= 8;		// ctrl+home
const int DCH_END			= 9;		// end
const int DCH_CTRLEND		= 10;		// ctrl+end
const int DCH_CTRLRIGHT		= 11;		// ctrl+right
const int DCH_CTRLLEFT		= 12;		// ctrl+left



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
class debug_view;
typedef struct _symbol_table symbol_table;
typedef struct _parsed_expression parsed_expression;


// OSD callback function for a view
typedef void (*debug_view_osd_update_func)(debug_view &view, void *osdprivate);


// a single "character" in the debug view has an ASCII value and an attribute byte
struct debug_view_char
{
	UINT8				byte;
	UINT8				attrib;
};


// pair of X,Y coordinates for sizing
class debug_view_xy
{
public:
	debug_view_xy(int _x = 0, int _y = 0) : x(_x), y(_y) { }

	INT32					x;
	INT32					y;
};


// debug_view_sources select from multiple sources available within a view
class debug_view_source
{
	DISABLE_COPYING(debug_view_source);

	friend class debug_view_source_list;

public:
	// construction/destruction
	debug_view_source(const char *name, device_t *device = NULL);
	virtual ~debug_view_source();

	// getters
	const char *name() const { return m_name; }
	debug_view_source *next() const { return m_next; }
	device_t *device() const { return m_device; }

private:
	// internal state
	debug_view_source *		m_next;					// link to next item
	astring					m_name;					// name of the source item
	device_t *				m_device;				// associated device (if applicable)
};


// a debug_view_source_list contains a list of debug_view_sources
class debug_view_source_list
{
	DISABLE_COPYING(debug_view_source_list);

public:
	// construction/destruction
	debug_view_source_list(running_machine &machine);
	~debug_view_source_list();

	// getters
	const debug_view_source *head() const { return m_head; }
	int count() const { return m_count; }
	int index(const debug_view_source &source) const;
	const debug_view_source *by_index(int index) const;

	// operations
	void reset();
	void append(debug_view_source &view_source);
	const debug_view_source *match_device(device_t *device) const;
	int match_device_index(device_t *device) const { return index(*match_device(device)); }

private:
	// internal state
	running_machine &		m_machine;				// reference to our machine
	debug_view_source *		m_head;					// head of the list
	debug_view_source *		m_tail;					// end of the tail
	UINT32					m_count;				// number of items in the list
};


// debug_view describes a single text-based view
class debug_view
{
	friend class debug_view_manager;

protected:
	// construction/destruction
	debug_view(running_machine &machine, debug_view_type type, debug_view_osd_update_func osdupdate, void *osdprivate);
	virtual ~debug_view();

public:
	// getters
	running_machine &machine() const { return m_machine; }
	debug_view *next() const { return m_next; }
	debug_view_type type() const { return m_type; }
	const debug_view_char *viewdata() const { return m_viewdata; }
	debug_view_xy total_size() { flush_updates(); return m_total; }
	debug_view_xy visible_size() { flush_updates(); return m_visible; }
	debug_view_xy visible_position() { flush_updates(); return m_topleft; }
	debug_view_xy cursor_position() { flush_updates(); return m_cursor; }
	bool cursor_supported() { flush_updates(); return m_supports_cursor; }
	bool cursor_visible() { flush_updates(); return m_cursor_visible; }
	const debug_view_source *source() const { return m_source; }
	const debug_view_source_list &source_list() const { return m_source_list; }

	// setters
	void set_size(int width, int height);
	void set_visible_size(debug_view_xy size);
	void set_visible_position(debug_view_xy pos);
	void set_cursor_position(debug_view_xy pos);
	void set_cursor_visible(bool visible = true);
	void set_source(const debug_view_source &source);
	void process_char(int character) { view_char(character); }

protected:
	// internal updating helpers
	void begin_update() { m_update_level++; }
	void end_update();
	void force_update() { begin_update(); m_update_pending = true; end_update(); }
	void flush_updates() { begin_update(); end_update(); }
	void flush_osd_updates();

	// cursor management helpers
	void adjust_visible_x_for_cursor();
	void adjust_visible_y_for_cursor();

	// overridables
	virtual void view_update() = 0;
	virtual void view_notify(debug_view_notification type);
	virtual void view_char(int chval);

protected:
	// core view data
	debug_view *			m_next;				// link to the next view
	running_machine &		m_machine;			// machine associated with this view
	debug_view_type			m_type;				// type of view
	const debug_view_source *m_source;			// currently selected data source
	debug_view_source_list	m_source_list;		// list of available data sources

	// OSD data
	debug_view_osd_update_func m_osdupdate;		// callback for the update
	void *					m_osdprivate;		// OSD-managed private data

	// visibility info
	debug_view_xy			m_visible;			// visible size (in rows and columns)
	debug_view_xy			m_total;			// total size (in rows and columns)
	debug_view_xy			m_topleft;			// top-left visible position (in rows and columns)
	debug_view_xy			m_cursor;			// cursor position
	bool					m_supports_cursor;	// does this view support a cursor?
	bool					m_cursor_visible;	// is the cursor visible?

	// update info
	bool					m_recompute;		// does this view require a recomputation?
	UINT8					m_update_level;		// update level; updates when this hits 0
	bool					m_update_pending;	// true if there is a pending update
	bool					m_osd_update_pending; // true if there is a pending update
	debug_view_char *		m_viewdata;			// current array of view data
	int						m_viewdata_size;	// number of elements of the viewdata array
};


// debug_view_manager manages all the views
class debug_view_manager
{
public:
	// construction/destruction
	debug_view_manager(running_machine &machine);
	~debug_view_manager();

	// view allocation
	debug_view *alloc_view(debug_view_type type, debug_view_osd_update_func osdupdate, void *osdprivate);
	void free_view(debug_view &view);

	// update helpers
	void update_all(debug_view_type type = DVT_NONE);
	void flush_osd_updates();

private:
	// private helpers
	debug_view *append(debug_view *view);

	// internal state
	running_machine &	m_machine;				// reference to our machine
	debug_view *		m_viewlist;				// list of views
};


// debug_view_expression is a helper for handling embedded expressions
class debug_view_expression
{
public:
	// construction/destruction
	debug_view_expression(running_machine &machine);
	~debug_view_expression();

	// getters
	bool dirty() const { return m_dirty; }
	UINT64 last_value() const { return m_result; }
	UINT64 value() { recompute(); return m_result; }
	const char *string() const { return m_string; }
	symbol_table *context() const { return m_context; }

	// setters
	void mark_dirty() { m_dirty = true; }
	void set_string(const char *string);
	void set_context(symbol_table *context);

private:
	// internal helpers
	bool recompute();

	// internal state
	running_machine &	m_machine;				// reference to the machine
	bool				m_dirty;				// true if the expression needs to be re-evaluated
	UINT64				m_result;				// last result from the expression
	parsed_expression *	m_parsed;				// parsed expression data
	astring				m_string;				// copy of the expression string
	symbol_table *		m_context;				// context we are using
};


#endif
