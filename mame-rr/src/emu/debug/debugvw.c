/*********************************************************************

    debugvw.c

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

#include "emu.h"
#include "debugvw.h"
#include "dvtext.h"
#include "dvstate.h"
#include "dvdisasm.h"
#include "dvmemory.h"
#include "debugcmd.h"
#include "debugcmt.h"
#include "debugcpu.h"
#include "debugcon.h"
#include "express.h"
#include <ctype.h>



//**************************************************************************
//  DEBUG VIEW SOURCE
//**************************************************************************

//-------------------------------------------------
//  debug_view_source - constructor
//-------------------------------------------------

debug_view_source::debug_view_source(const char *name, device_t *device)
	: m_next(NULL),
	  m_name(name),
	  m_device(device)
{
}


//-------------------------------------------------
//  ~debug_view_source - destructor
//-------------------------------------------------

debug_view_source::~debug_view_source()
{
}



//**************************************************************************
//  DEBUG VIEW SOURCE LIST
//**************************************************************************

//-------------------------------------------------
//  debug_view_source_list - constructor
//-------------------------------------------------

debug_view_source_list::debug_view_source_list(running_machine &machine)
	: m_machine(machine),
	  m_head(NULL),
	  m_tail(NULL),
	  m_count(0)
{
}


//-------------------------------------------------
//  ~debug_view_source_list - destructor
//-------------------------------------------------

debug_view_source_list::~debug_view_source_list()
{
	reset();
}


//-------------------------------------------------
//  index - return the index of a source
//-------------------------------------------------

int debug_view_source_list::index(const debug_view_source &source) const
{
	int result = 0;
	for (debug_view_source *cursource = m_head; cursource != NULL; cursource = cursource->m_next)
	{
		if (cursource == &source)
			break;
		result++;
	}
	return result;
}


//-------------------------------------------------
//  by_index - return a source given an index
//-------------------------------------------------

const debug_view_source *debug_view_source_list::by_index(int index) const
{
	if (m_head == NULL)
		return NULL;
	const debug_view_source *result;
	for (result = m_head; index > 0 && result->m_next != NULL; result = result->m_next)
		index--;
	return result;
}


//-------------------------------------------------
//  reset - free all the view_sources
//-------------------------------------------------

void debug_view_source_list::reset()
{
	// free from the head
	while (m_head != NULL)
	{
		debug_view_source *source = m_head;
		m_head = source->m_next;
		auto_free(&m_machine, source);
	}

	// reset the tail pointer and index
	m_tail = NULL;
	m_count = 0;
}


//-------------------------------------------------
//  append - add a view_source to the end of the
//  list
//-------------------------------------------------

void debug_view_source_list::append(debug_view_source &source)
{
	// set the next and index values
	source.m_next = NULL;

	// append to the end
	if (m_tail == NULL)
		m_head = m_tail = &source;
	else
		m_tail->m_next = &source;
	m_tail = &source;
	m_count++;
}


//-------------------------------------------------
//  match_device - find the first view that
//  matches the given device
//-------------------------------------------------

const debug_view_source *debug_view_source_list::match_device(device_t *device) const
{
	for (debug_view_source *source = m_head; source != NULL; source = source->m_next)
		if (device == source->m_device)
			return source;
	return m_head;
}



//**************************************************************************
//  DEBUG VIEW
//**************************************************************************

//-------------------------------------------------
//  debug_view - constructor
//-------------------------------------------------

debug_view::debug_view(running_machine &machine, debug_view_type type, debug_view_osd_update_func osdupdate, void *osdprivate)
	: m_next(NULL),
	  m_machine(machine),
	  m_type(type),
	  m_source(NULL),
	  m_source_list(machine),
	  m_osdupdate(osdupdate),
	  m_osdprivate(osdprivate),
	  m_visible(10,10),
	  m_total(10,10),
	  m_topleft(0,0),
	  m_cursor(0,0),
	  m_supports_cursor(false),
	  m_cursor_visible(false),
	  m_recompute(true),
	  m_update_level(0),
	  m_update_pending(true),
	  m_osd_update_pending(true),
	  m_viewdata(NULL),
	  m_viewdata_size(0)
{
	// allocate memory for the buffer
	m_viewdata_size = m_visible.y * m_visible.x;
	m_viewdata = auto_alloc_array(&machine, debug_view_char, m_viewdata_size);
}


//-------------------------------------------------
//  ~debug_view - destructor
//-------------------------------------------------

debug_view::~debug_view()
{
}


//-------------------------------------------------
//  end_update - bracket a sequence of changes so
//  that only one update occurs
//-------------------------------------------------

void debug_view::end_update()
{
	/* if we hit zero, call the update function */
	if (m_update_level == 1)
	{
		while (m_update_pending)
		{
			// no longer pending, but flag for the OSD
			m_update_pending = false;
			m_osd_update_pending = true;

			// resize the viewdata if needed
			int size = m_visible.x * m_visible.y;
			if (size > m_viewdata_size)
			{
				m_viewdata_size = size;
				auto_free(&m_machine, m_viewdata);
				m_viewdata = auto_alloc_array(&m_machine, debug_view_char, m_viewdata_size);
			}

			// update the view
			view_update();
		}
	}

	// decrement the level
	m_update_level--;
}


//-------------------------------------------------
//  flush_osd_updates - notify the OSD of any
//  pending updates
//-------------------------------------------------

void debug_view::flush_osd_updates()
{
	if (m_osd_update_pending && m_osdupdate != NULL)
		(*m_osdupdate)(*this, m_osdprivate);
	m_osd_update_pending = false;
}


//-------------------------------------------------
//  set_visible_size - set the visible size in
//  rows and columns
//-------------------------------------------------*/

void debug_view::set_visible_size(debug_view_xy size)
{
	if (size.x != m_visible.x || size.y != m_visible.y)
	{
		begin_update();
		m_visible = size;
		m_update_pending = true;
		view_notify(VIEW_NOTIFY_VISIBLE_CHANGED);
		end_update();
	}
}


//-------------------------------------------------
//  set_visible_position - set the top left
//  position of the visible area in rows and
//  columns
//-------------------------------------------------

void debug_view::set_visible_position(debug_view_xy pos)
{
	if (pos.x != m_topleft.x || pos.y != m_topleft.y)
	{
		begin_update();
		m_topleft = pos;
		m_update_pending = true;
		view_notify(VIEW_NOTIFY_VISIBLE_CHANGED);
		end_update();
	}
}


//-------------------------------------------------
//  set_cursor_position - set the current cursor
//  position as a row and column
//-------------------------------------------------

void debug_view::set_cursor_position(debug_view_xy pos)
{
	if (pos.x != m_cursor.x || pos.y != m_cursor.y)
	{
		begin_update();
		m_cursor = pos;
		m_update_pending = true;
		view_notify(VIEW_NOTIFY_CURSOR_CHANGED);
		end_update();
	}
}


//-------------------------------------------------
//  set_cursor_visible - set the visible state of
//  the cursor
//-------------------------------------------------

void debug_view::set_cursor_visible(bool visible)
{
	if (visible != m_cursor_visible)
	{
		begin_update();
		m_cursor_visible = visible;
		m_update_pending = true;
		view_notify(VIEW_NOTIFY_CURSOR_CHANGED);
		end_update();
	}
}


//-------------------------------------------------
//  set_subview - set the current subview
//-------------------------------------------------

void debug_view::set_source(const debug_view_source &source)
{
	if (&source != m_source)
	{
		begin_update();
		m_source = &source;
		m_update_pending = true;
		view_notify(VIEW_NOTIFY_SOURCE_CHANGED);
		end_update();
	}
}


//-------------------------------------------------
//  adjust_visible_x_for_cursor - adjust a view's
//  visible X position to ensure the cursor is
//  visible
//-------------------------------------------------

void debug_view::adjust_visible_x_for_cursor()
{
	if (m_cursor.x < m_topleft.x)
		m_topleft.x = m_cursor.x;
	else if (m_cursor.x >= m_topleft.x + m_visible.x - 1)
		m_topleft.x = m_cursor.x - m_visible.x + 2;
}


//-------------------------------------------------
//  adjust_visible_y_for_cursor - adjust a view's
//  visible Y position to ensure the cursor is
//  visible
//-------------------------------------------------

void debug_view::adjust_visible_y_for_cursor()
{
	if (m_cursor.y < m_topleft.y)
		m_topleft.y = m_cursor.y;
	else if (m_cursor.y >= m_topleft.y + m_visible.y - 1)
		m_topleft.y = m_cursor.y - m_visible.y + 2;
}


//-------------------------------------------------
//  view_notify - handle notification of updates
//-------------------------------------------------

void debug_view::view_notify(debug_view_notification type)
{
	// default does nothing
}


//-------------------------------------------------
//  view_char - handle a character typed within
//  the current view
//-------------------------------------------------

void debug_view::view_char(int chval)
{
	// default does nothing
}



//**************************************************************************
//  DEBUG VIEW MANAGER
//**************************************************************************

//-------------------------------------------------
//  debug_view_manager - constructor
//-------------------------------------------------

debug_view_manager::debug_view_manager(running_machine &machine)
	: m_machine(machine),
	  m_viewlist(NULL)
{
}


//-------------------------------------------------
//  ~debug_view_manager - destructor
//-------------------------------------------------

debug_view_manager::~debug_view_manager()
{
	while (m_viewlist != NULL)
	{
		debug_view *oldhead = m_viewlist;
		m_viewlist = oldhead->m_next;
		auto_free(&m_machine, oldhead);
	}
}


//-------------------------------------------------
//  alloc_view - create a new view
//-------------------------------------------------

debug_view *debug_view_manager::alloc_view(debug_view_type type, debug_view_osd_update_func osdupdate, void *osdprivate)
{
	switch (type)
	{
		case DVT_CONSOLE:
			return append(auto_alloc(&m_machine, debug_view_console(m_machine, osdupdate, osdprivate)));

		case DVT_STATE:
			return append(auto_alloc(&m_machine, debug_view_state(m_machine, osdupdate, osdprivate)));

		case DVT_DISASSEMBLY:
			return append(auto_alloc(&m_machine, debug_view_disasm(m_machine, osdupdate, osdprivate)));

		case DVT_MEMORY:
			return append(auto_alloc(&m_machine, debug_view_memory(m_machine, osdupdate, osdprivate)));

		case DVT_LOG:
			return append(auto_alloc(&m_machine, debug_view_log(m_machine, osdupdate, osdprivate)));

		case DVT_TIMERS:
//          return append(auto_alloc(&m_machine, debug_view_timers(m_machine, osdupdate, osdprivate)));

		case DVT_ALLOCS:
//          return append(auto_alloc(&m_machine, debug_view_allocs(m_machine, osdupdate, osdprivate)));

		default:
			fatalerror("Attempt to create invalid debug view type %d\n", type);
	}
	return NULL;
}


//-------------------------------------------------
//  free_view - free a view
//-------------------------------------------------

void debug_view_manager::free_view(debug_view &view)
{
	// free us but only if we're in the list
	for (debug_view **viewptr = &m_viewlist; *viewptr != NULL; viewptr = &(*viewptr)->m_next)
		if (*viewptr == &view)
		{
			*viewptr = view.m_next;
			auto_free(&m_machine, &view);
			break;
		}
}


//-------------------------------------------------
//  update_all - force all views to refresh
//-------------------------------------------------

void debug_view_manager::update_all(debug_view_type type)
{
	// loop over each view and force an update
	for (debug_view *view = m_viewlist; view != NULL; view = view->next())
		if (type == DVT_NONE || type == view->type())
			view->force_update();
}


//-------------------------------------------------
//  flush_osd_updates - flush all pending OSD
//  updates
//-------------------------------------------------

void debug_view_manager::flush_osd_updates()
{
	for (debug_view *view = m_viewlist; view != NULL; view = view->m_next)
		view->flush_osd_updates();
}


//-------------------------------------------------
//  append - append a view to the end of our list
//-------------------------------------------------

debug_view *debug_view_manager::append(debug_view *view)
{
	debug_view **viewptr;
	for (viewptr = &m_viewlist; *viewptr != NULL; viewptr = &(*viewptr)->m_next) ;
	*viewptr = view;
	return view;
}



//**************************************************************************
//  DEBUG VIEW EXPRESSION
//**************************************************************************

//-------------------------------------------------
//  debug_view_expression - constructor
//-------------------------------------------------

debug_view_expression::debug_view_expression(running_machine &machine)
	: m_machine(machine),
	  m_dirty(true),
	  m_result(0),
	  m_parsed(NULL),
	  m_string("0"),
	  m_context(debug_cpu_get_global_symtable(&machine))
{
}


//-------------------------------------------------
//  ~debug_view_expression - destructor
//-------------------------------------------------

debug_view_expression::~debug_view_expression()
{
	// free our parsed expression
	if (m_parsed != NULL)
		expression_free(m_parsed);
}


//-------------------------------------------------
//  set_string - set the expression string
//-------------------------------------------------

void debug_view_expression::set_string(const char *string)
{
	m_string = string;
	m_dirty = true;
}


//-------------------------------------------------
//  set_context - set the context for the
//  expression
//-------------------------------------------------

void debug_view_expression::set_context(symbol_table *context)
{
	m_context = (context != NULL) ? context : debug_cpu_get_global_symtable(&m_machine);
	m_dirty = true;
}


//-------------------------------------------------
//  recompute - recompute the value of an
//  expression
//-------------------------------------------------

bool debug_view_expression::recompute()
{
	bool changed = m_dirty;

	// if dirty, re-evaluate
	if (m_dirty)
	{
		// parse the new expression
		parsed_expression *expr;
		EXPRERR exprerr = expression_parse(m_string, m_context, &debug_expression_callbacks, &m_machine, &expr);

		// if it worked, update the expression
		if (exprerr == EXPRERR_NONE)
		{
			if (m_parsed != NULL)
				expression_free(m_parsed);
			m_parsed = expr;
		}
	}

	// if we have a parsed expression, evalute it
	if (m_parsed != NULL)
	{
		UINT64 oldresult = m_result;

		// recompute the value of the expression
		expression_execute(m_parsed, &m_result);
		if (m_result != oldresult)
			changed = true;
	}

	// expression no longer dirty by definition
	m_dirty = false;
	return changed;
}
