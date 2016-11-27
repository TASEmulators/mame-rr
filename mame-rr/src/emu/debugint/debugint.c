/*********************************************************************

    debugint.c

    Internal debugger frontend using render interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "ui.h"
#include "rendfont.h"
#include "uimenu.h"
#include "uiinput.h"
#include "video.h"
#include "osdepend.h"

#include "debug/debugvw.h"
#include "debug/dvdisasm.h"
#include "debug/dvmemory.h"
#include "debug/dvstate.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define BORDER_YTHICKNESS 1
#define BORDER_XTHICKNESS 1
#define HSB_HEIGHT 20
#define VSB_WIDTH 20
#define TITLE_HEIGHT 20

enum
{
	RECT_DVIEW,
	RECT_DVIEW_CLIENT,
	RECT_DVIEW_TITLE,
	RECT_DVIEW_HSB,
	RECT_DVIEW_VSB,
	RECT_DVIEW_SIZE,
};

enum
{
	VIEW_STATE_BUTTON			= 0x01,
	VIEW_STATE_MOVING			= 0x02,
	VIEW_STATE_SIZING			= 0x04,
	VIEW_STATE_NEEDS_UPDATE 	= 0x08,
	VIEW_STATE_FOLLOW_CPU		= 0x10,
};

/***************************************************************************
    MACROS
***************************************************************************/

//#define NX(_dv, _x) ((float) (_x)/(float)rect_get_width(&(_dv)->bounds))
//#define NY(_dv, _y) ((float) (_y)/(float)rect_get_height(&(_dv)->bounds))
#define NX(_dv, _x) ((float) (_x)/(float) (dv)->rt_width)
#define NY(_dv, _y) ((float) (_y)/(float) (dv)->rt_height)


#define LIST_ADD_FRONT(_list, _elem, _type) \
	do { \
		(_elem)->next = _list; \
		_list = _elem; \
	} while (0)

#define LIST_GET_PREVIOUS(_list, _elem, _prev) \
	do { \
		_prev = NULL; \
		if (_list != _elem) \
			for (_prev = _list; _prev != NULL; _prev = _prev->next) \
				if ((_prev)->next == _elem) \
					break; \
	} while (0)

#define LIST_GET_LAST(_list, _last) \
	do { \
		for (_last = _list; _last != NULL; _last = _last->next) \
			if ((_last)->next == NULL) \
				break; \
	} while (0)

#define LIST_REMOVE(_list, _elem, _type) \
	do { \
		_type *_hlp; \
		LIST_GET_PREVIOUS(_list, _elem, _hlp); \
		if (_hlp != NULL) \
			(_hlp)->next = (_elem)->next; \
		else \
			_list = (_elem)->next; \
	} while (0)

#define LIST_ADD_BACK(_list, _elem, _type) \
	do { \
		_type *_hlp; \
		LIST_GET_LAST(_list, _hlp); \
		if (_hlp != NULL) \
			(_hlp)->next = _elem; \
		else \
			_list = _elem; \
	} while (0)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _adjustment adjustment;
struct _adjustment
{
	int		visible;
	int		lower;
	int		upper;
	int		value;
	int		step_increment;
	int		page_increment;
	int		page_size;
};

class DView;

class DView_edit
{
	DISABLE_COPYING(DView_edit);

public:
	DView_edit()
	{ }
	~DView_edit()
	{ }
	int					active;
	render_container *	container;
	astring				str;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void dview_update(debug_view &dw, void *osdprivate);
static int map_point(DView *dv, INT32 target_x, INT32 target_y, INT32 *mapped_x, INT32 *mapped_y);

class DView
{
	DISABLE_COPYING(DView);

public:
	DView(render_target *target, running_machine *machine, debug_view_type type, int flags)
		: next(NULL),
		  type(0),
		  state(0),
		  ofs_x(0),
		  ofs_y(0)
	  {
		this->target = target;
		//dv->container = render_target_get_component_container(target, name, &pos);
		this->container = render_debug_alloc(target);
		this->view = machine->m_debug_view->alloc_view(type, dview_update, this);
		this->type = type;
		this->machine = machine;
		this->state = flags | VIEW_STATE_NEEDS_UPDATE;

		// initial size
		this->bounds.min_x = 0;
		this->bounds.min_y = 0;
		this->bounds.max_x = 300;
		this->bounds.max_y = 300;

		/* specials */
		switch (type)
		{
		case DVT_DISASSEMBLY:
			/* set up disasm view */
			downcast<debug_view_disasm *>(this->view)->set_expression("curpc");
			//debug_view_  property_UINT32(dv->view, DVP_DASM_TRACK_LIVE, 1);
			break;
		default:
			break;
		}
	  }
	~DView()
	{
		render_debug_free(this->target, this->container);
		machine->m_debug_view->free_view(*this->view);
	}

	DView *				next;

	int 				type;
	debug_view *		view;
	render_container *	container;
	render_target *		target;
	running_machine *	machine;
	int					state;
	// drawing
	rectangle			bounds;
	int					ofs_x;
	int					ofs_y;
	astring				title;
	int					last_x;
	int					last_y;
	// Scrollbars
	adjustment			hsb;
	adjustment			vsb;
	// render target tracking
	INT32				rt_width;
	INT32				rt_height;
	//optional
	DView_edit			editor;
};


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE int rect_get_width(rectangle *r)
{
	return r->max_x - r->min_x + 1;
}

INLINE int rect_get_height(rectangle *r)
{
	return r->max_y - r->min_y + 1;
}

INLINE void rect_set_width(rectangle *r, int width)
{
	r->max_x = r->min_x + width - 1;
}

INLINE void rect_set_height(rectangle *r, int height)
{
	r->max_y = r->min_y + height - 1;
}

INLINE void rect_move(rectangle *r, int x, int y)
{
	int dx = x - r->min_x;
	int dy = y - r->min_y;

	r->min_x += dx;
	r->max_x += dx;
	r->min_y += dy;
	r->max_y += dy;
}

INLINE int dview_is_state(DView *dv, int state)
{
	return ((dv->state & state) ? TRUE : FALSE);
}

INLINE int dview_is_state_all(DView *dv, int state)
{
	return ((dv->state & state) == state ? TRUE : FALSE);
}

INLINE void dview_set_state(DView *dv, int state, int onoff)
{
	if (onoff)
		dv->state |= state;
	else
		dv->state &= ~state;
}

/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

static render_font *	debug_font;
static int				debug_font_width;
static int				debug_font_height;
static float			debug_font_aspect;
static DView *			list;
static DView *			focus_view;

static ui_menu *		menu;
static DView_edit *		cur_editor;

static void set_focus_view(DView *dv)
{
	if (focus_view != NULL)
		dview_set_state(focus_view, VIEW_STATE_NEEDS_UPDATE, TRUE);

	if (dv != NULL)
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);

	if (focus_view != dv)
	{
		focus_view = dv;
		LIST_REMOVE(list, dv, DView);
		LIST_ADD_FRONT(list, dv, DView);
		render_debug_top(dv->target, dv->container);
	}
}

static DView *dview_alloc(render_target *target, running_machine *machine, debug_view_type type, int flags)
{
	DView *dv;

	dv = auto_alloc(machine, DView(target, machine, type, flags));

	/* add to list */

	LIST_ADD_BACK(list, dv, DView);

	return dv;
}

static void dview_free(DView *dv)
{
	//astring_free(dv->title);
	LIST_REMOVE(list, dv, DView);
	auto_free(dv->machine, dv);
}

static void dview_get_rect(DView *dv, int type, rectangle *rect)
{
	*rect = dv->bounds;
	switch (type)
	{
	case RECT_DVIEW:
		break;
	case RECT_DVIEW_CLIENT:
		rect->min_x += BORDER_XTHICKNESS;
		rect->max_x -= (BORDER_XTHICKNESS + dv->vsb.visible * VSB_WIDTH);
		rect->min_y += 2 * BORDER_YTHICKNESS + TITLE_HEIGHT;
		rect->max_y -= (BORDER_YTHICKNESS + dv->hsb.visible * HSB_HEIGHT);
		break;
	case RECT_DVIEW_HSB:
		rect->min_x += 0;
		rect->max_x -= /* dv->vsb.visible * */ VSB_WIDTH;
		rect->min_y = dv->bounds.max_y - HSB_HEIGHT;
		rect->max_y -= 0;
		break;
	case RECT_DVIEW_VSB:
		rect->min_x = dv->bounds.max_x - VSB_WIDTH;
		rect->max_x -= 0;
		rect->min_y += TITLE_HEIGHT;
		rect->max_y -= /* dv->hsb.visible * */ HSB_HEIGHT;
		break;
	case RECT_DVIEW_SIZE:
		rect->min_x = dv->bounds.max_x - VSB_WIDTH;
		rect->max_x -= 0;
		rect->min_y = dv->bounds.max_y - HSB_HEIGHT;
		rect->max_y -= 0;
		break;
	case RECT_DVIEW_TITLE:
		rect->min_x += 0;
		rect->max_x -= 0;
		rect->min_y += 0;
		rect->max_y = rect->min_y + TITLE_HEIGHT - 1;
		break;
	default:
		assert_always(FALSE, "unknown rectangle type");
	}
}


static void dview_clear(DView *dv)
{
	render_container_empty(dv->container);
}

static void dview_draw_outlined_box(DView *dv, int rtype, int x, int y, int w, int h, rgb_t bg)
{
	rectangle r;

	dview_get_rect(dv, rtype, &r);
	ui_draw_outlined_box(dv->container, NX(dv, x + r.min_x), NY(dv, y + r.min_y),
			NX(dv, x + r.min_x + w), NY(dv, y + r.min_y + h), bg);
}

static void dview_draw_box(DView *dv, int rtype, int x, int y, int w, int h, rgb_t col)
{
	rectangle r;

	dview_get_rect(dv, rtype, &r);
	render_container_add_rect(dv->container, NX(dv, x + r.min_x), NY(dv, y + r.min_y),
			NX(dv, x + r.min_x + w), NY(dv, y + r.min_y + h), col,
			PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}

static void dview_draw_char(DView *dv, int rtype, int x, int y, int h, rgb_t col, UINT16 ch)
{
	rectangle r;

	dview_get_rect(dv, rtype, &r);
	render_container_add_char(dv->container,
			NX(dv, x + r.min_x),
			NY(dv, y + r.min_y),
			NY(dv, h),
			debug_font_aspect,
			//(float) rect_get_height(&dv->bounds) / (float) rect_get_width(&dv->bounds), //render_get_ui_aspect(),
			col,
			debug_font,
			ch);
}

static int dview_xy_in_rect(DView *dv, int type, int x, int y)
{
	rectangle r;

	dview_get_rect(dv, type, &r);
	if (x >= r.min_x && x <= r.max_x && y >= r.min_y && y <= r.max_y)
		return TRUE;
	return FALSE;
}

static void dview_draw_hsb(DView *dv)
{
	int vt;
	int ts;
	//int sz = SLIDER_SIZE;
	int sz;
	rectangle r;
	adjustment *sb = &dv->hsb;

	dview_get_rect(dv, RECT_DVIEW_HSB, &r);

	dview_draw_outlined_box(dv, RECT_DVIEW_HSB, 0, 0, VSB_WIDTH,HSB_HEIGHT, MAKE_ARGB(0xff, 0xff, 0x00, 0x00));
	dview_draw_outlined_box(dv, RECT_DVIEW_HSB, rect_get_width(&r) - VSB_WIDTH, 0, VSB_WIDTH, HSB_HEIGHT, MAKE_ARGB(0xff, 0xff, 0x00, 0x00));

	ts = (r.max_x - r.min_x + 1) - 2 * VSB_WIDTH;

	sz = (ts * (sb->page_size)) / (sb->upper - sb->lower);
	ts = ts - sz;

	vt = (ts * (sb->value - sb->lower)) / (sb->upper - sb->lower - sb->page_size) + sz / 2 + r.min_x + VSB_WIDTH;

	dview_draw_outlined_box(dv, RECT_DVIEW_HSB, vt - sz / 2, 0, sz, HSB_HEIGHT, MAKE_ARGB(0xff, 0xff, 0x00, 0x00));
}

static void dview_draw_vsb(DView *dv)
{
	int vt;
	int ts;
	//int sz = SLIDER_SIZE;
	int sz;
	rectangle r;
	adjustment *sb = &dv->vsb;

	dview_get_rect(dv, RECT_DVIEW_VSB, &r);

	dview_draw_outlined_box(dv, RECT_DVIEW_VSB, 0, rect_get_height(&r) - HSB_HEIGHT, VSB_WIDTH, HSB_HEIGHT, MAKE_ARGB(0xff, 0xff, 0x00, 0x00));
	dview_draw_outlined_box(dv, RECT_DVIEW_VSB, 0, 0,               VSB_WIDTH, HSB_HEIGHT, MAKE_ARGB(0xff, 0xff, 0x00, 0x00));

	ts = (r.max_y - r.min_y + 1) - 2 * HSB_HEIGHT;

	sz = (ts * (sb->page_size)) / (sb->upper - sb->lower);
	ts = ts - sz;

	vt = (ts * (sb->value - sb->lower)) / (sb->upper - sb->lower - sb->page_size) + sz / 2 + HSB_HEIGHT;

	dview_draw_outlined_box(dv, RECT_DVIEW_VSB, 0, vt - sz / 2, VSB_WIDTH, sz, MAKE_ARGB(0xff, 0xff, 0x00, 0x00));
}

static void dview_draw_size(DView *dv)
{
	rectangle r;

	dview_get_rect(dv, RECT_DVIEW_SIZE, &r);

	dview_draw_outlined_box(dv, RECT_DVIEW_SIZE, 0, 0,
			rect_get_width(&r),rect_get_height(&r), MAKE_ARGB(0xff, 0xff, 0xff, 0x00));
}

static void dview_set_title(DView *dv, astring title)
{
	if (dv->title.cmp(title) != 0)
	{
		dv->title = title;
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
	}
}

static void dview_draw_title(DView *dv)
{
	int i;
	rgb_t col = MAKE_ARGB(0xff,0x00,0x00,0xff);
	rectangle r;

	dview_get_rect(dv, RECT_DVIEW_TITLE, &r);

	if (dv == focus_view)
		col = MAKE_ARGB(0xff,0x00,0x7f,0x00);

	dview_draw_outlined_box(dv, RECT_DVIEW_TITLE, 0, 0, rect_get_width(&dv->bounds), TITLE_HEIGHT, col);

	if (dv->title == NULL)
		return;

	for (i=0; i<strlen(dv->title); i++)
	{
		dview_draw_char(dv, RECT_DVIEW_TITLE, i * debug_font_width + BORDER_XTHICKNESS,
				BORDER_YTHICKNESS, debug_font_height, //r.max_y - 2 * BORDER_YTHICKNESS,
				MAKE_ARGB(0xff,0xff,0xff,0xff), (UINT16) dv->title[i] );
	}
}

static int dview_on_mouse(DView *dv, int mx, int my, int button)
{
	int clicked = (button && !dview_is_state(dv, VIEW_STATE_BUTTON));
	int handled = TRUE;
	int x,y;

	if (button && dview_is_state_all(dv, VIEW_STATE_BUTTON | VIEW_STATE_MOVING))
	{
		int dx = mx - dv->last_x;
		int dy = my - dv->last_y;

		dv->ofs_x += dx;
		dv->ofs_y += dy;
		dv->last_x = mx;
		dv->last_y = my;
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
		return TRUE;
	}
	else if (button && dview_is_state_all(dv, VIEW_STATE_BUTTON | VIEW_STATE_SIZING))
	{
		int dx = mx - dv->last_x;
		int dy = my - dv->last_y;

		dv->bounds.max_x += dx;
		dv->bounds.max_y += dy;
		dv->last_x = mx;
		dv->last_y = my;
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
		return TRUE;
	}
	else
		dview_set_state(dv, VIEW_STATE_MOVING | VIEW_STATE_SIZING, FALSE);

	if (!map_point(dv, mx, my, &x, &y))
		return FALSE;

	if (dview_xy_in_rect(dv, RECT_DVIEW_TITLE, x, y))
	{
		/* on title, do nothing */
		if (clicked) {
			dv->last_x = mx;
			dv->last_y = my;
			set_focus_view(dv);
			dview_set_state(dv, VIEW_STATE_MOVING, TRUE);
		}
	}
	else if (dview_xy_in_rect(dv, RECT_DVIEW_HSB, x, y))
	{
		/* on horizontal scrollbar */
		debug_view_xy pos;
		adjustment *sb = &dv->hsb;

		if (clicked)
		{
			rectangle r;
			int xt;

			dview_get_rect(dv, RECT_DVIEW_HSB, &r);
			x -= r.min_x;

			xt = (x - VSB_WIDTH) * (sb->upper - sb->lower) / (rect_get_width(&r) - 2 * dv->vsb.visible * VSB_WIDTH) + sb->lower;
			if (x < VSB_WIDTH)
				sb->value -= sb->step_increment;
			else if (x > rect_get_width(&r) - VSB_WIDTH)
				sb->value += sb->step_increment;
			else if (xt < sb->value)
				sb->value -= sb->page_increment;
			else if (xt > sb->value)
				sb->value += sb->page_increment;

			if (sb->value < sb->lower)
				sb->value = sb->lower;
			if (sb->value > sb->upper)
				sb->value = sb->upper;
		}

		pos = dv->view->visible_position();

		if (sb->value != pos.x)
		{
			pos.x = sb->value;
			dv->view->set_visible_position(pos);
			dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
		}
	}
	else if (dview_xy_in_rect(dv, RECT_DVIEW_VSB, x, y) )
	{
		/* on vertical scrollbar */
		debug_view_xy pos;
		adjustment *sb = &dv->vsb;

		if (clicked)
		{
			rectangle r;
			int yt;

			dview_get_rect(dv, RECT_DVIEW_VSB, &r);
			y -= r.min_y;
			yt = (y - HSB_HEIGHT) * (sb->upper - sb->lower) / (rect_get_height(&r) - 2 * HSB_HEIGHT) + sb->lower;

			if (y < HSB_HEIGHT)
				sb->value -= sb->step_increment;
			else if (y > rect_get_height(&r) - HSB_HEIGHT)
				sb->value += sb->step_increment;
			else if (yt < sb->value)
				sb->value -= sb->page_increment;
			else if (yt > sb->value)
				sb->value += sb->page_increment;

			if (sb->value < sb->lower)
				sb->value = sb->lower;
			if (sb->value > sb->upper)
				sb->value = sb->upper;
		}

		pos = dv->view->visible_position();

		if (sb->value != pos.y)
		{
			pos.y = sb->value;
			dv->view->set_visible_position(pos);
			dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
		}
	}
	else if (dview_xy_in_rect(dv, RECT_DVIEW_SIZE, x, y))
	{
		/* on sizing area */
		if (clicked)
		{
			dv->last_x = mx;
			dv->last_y = my;
			set_focus_view(dv);
			dview_set_state(dv, VIEW_STATE_SIZING, TRUE);
		}
	}
	else if (dview_xy_in_rect(dv, RECT_DVIEW_CLIENT, x, y))
	{
		y -= TITLE_HEIGHT;
		if (dv->view->cursor_supported() && clicked && y >= 0)
		{
			debug_view_xy topleft = dv->view->visible_position();
			debug_view_xy newpos;
			newpos.x = topleft.x + x / debug_font_width;
			newpos.y = topleft.y + y / debug_font_height;
			dv->view->set_cursor_position(newpos);
			dv->view->set_cursor_visible(true);
		}
		if (clicked)
			set_focus_view(dv);
	}
	else
	{
		handled = FALSE;
	}
	dview_set_state(dv, VIEW_STATE_BUTTON, button);
	return handled;

}

INLINE void map_attr_to_fg_bg(unsigned char attr, rgb_t *fg, rgb_t *bg)
{

	*bg = MAKE_ARGB(0xff,0xff,0xff,0xff);
	*fg = MAKE_ARGB(0xff,0x00,0x00,0x00);

	if(attr & DCA_ANCILLARY)
		*bg = MAKE_ARGB(0xff,0xe0,0xe0,0xe0);
	if(attr & DCA_SELECTED) {
		*bg = MAKE_ARGB(0xff,0xff,0x80,0x80);
	}
	if(attr & DCA_CURRENT) {
		*bg = MAKE_ARGB(0xff,0xff,0xff,0x00);
	}
	if(attr & DCA_CHANGED) {
		*fg = MAKE_ARGB(0xff,0xff,0x00,0x00);
	}
	if(attr & DCA_INVALID) {
		*fg = MAKE_ARGB(0xff,0x00,0x00,0xff);
	}
	if(attr & DCA_DISABLED) {
		*fg = MAKE_ARGB(RGB_ALPHA(*fg),
				(RGB_RED(*fg)+RGB_RED(*bg)) >> 1,
				(RGB_GREEN(*fg)+RGB_GREEN(*bg)) >> 1,
				(RGB_BLUE(*fg)+RGB_BLUE(*bg)) >> 1);
	}
	if(attr & DCA_COMMENT) {
		*fg = MAKE_ARGB(0xff,0x00,0x80,0x00);
	}
}

static void dview_draw(DView *dv)
{
	const debug_view_char *viewdata;
	debug_view_xy vsize;
	UINT32 i, j, xx, yy;
	rgb_t bg_base, bg, fg;
	rectangle r;

	vsize = dv->view->visible_size();

	bg_base = MAKE_ARGB(0xff,0xff,0xff,0xff);

	/* always start clean */
	dview_clear(dv);

	dview_draw_title(dv);

	dview_get_rect(dv, RECT_DVIEW_CLIENT, &r);

	dview_draw_outlined_box(dv, RECT_DVIEW_CLIENT, 0, 0,
			rect_get_width(&r) /*- (dv->vs ? VSB_WIDTH : 0)*/,
			rect_get_height(&r) /*- (dv->hsb.visible ? HSB_HEIGHT : 0)*/, bg_base);

	/* background first */
	viewdata = dv->view->viewdata();

	yy = BORDER_YTHICKNESS;
	for(j=0; j<vsize.y; j++)
	{
		xx = BORDER_XTHICKNESS;
		for(i=0; i<vsize.x; i++)
		{
			map_attr_to_fg_bg(viewdata->attrib, &fg, &bg);

			if (bg != bg_base)
				dview_draw_box(dv, RECT_DVIEW_CLIENT, xx, yy,
						debug_font_width, debug_font_height, bg);
			xx += debug_font_width;
			viewdata++;
		}
		yy += debug_font_height;
	}

	/* now the text */
	viewdata = dv->view->viewdata();

	yy = BORDER_YTHICKNESS;
	for(j=0; j<vsize.y; j++)
	{
		xx = BORDER_XTHICKNESS;
		for(i=0; i<vsize.x; i++)
		{
			UINT16 s;
			unsigned char v = viewdata->byte;

			if (v != ' ')
			{
				if(v < 128) {
					s = v;
				} else {
					s = 0xc0 | (v>>6);
					s |= (0x80 | (v & 0x3f));
				}
				map_attr_to_fg_bg(viewdata->attrib, &fg, &bg);

				dview_draw_char(dv, RECT_DVIEW_CLIENT, xx, yy, debug_font_height, fg, s);
			}
			xx += debug_font_width;
			viewdata++;
		}
		yy += debug_font_height;
	}

	if(dv->hsb.visible)
		dview_draw_hsb(dv);
	if(dv->vsb.visible)
		dview_draw_vsb(dv);
	dview_draw_size(dv);
}


static void dview_size_allocate(DView *dv)
{
	debug_view_xy size, pos, col, vsize;
	render_container_user_settings rcus;
	rectangle r;

	render_container_get_user_settings(dv->container, &rcus);
	rcus.xoffset = (float) dv->ofs_x / (float) dv->rt_width;
	rcus.yoffset = (float) dv->ofs_y / (float) dv->rt_height;
	rcus.xscale = 1.0; //(float) rect_get_width(&dv->bounds) / (float) dv->rt_width;
	rcus.yscale = 1.0; //(float) rect_get_height(&dv->bounds) / (float) dv->rt_height;
	render_container_set_user_settings(dv->container, &rcus);
	//printf("%d %d %d %d\n", wpos.min_x, wpos.max_x, wpos.min_y, wpos.max_y);

	pos = dv->view->visible_position();
	size = dv->view->total_size();

	dv->hsb.visible = 0;
	dv->vsb.visible = 0;
	dview_get_rect(dv, RECT_DVIEW_CLIENT, &r);

	dv->hsb.visible = (size.x * debug_font_width > rect_get_width(&r) ? 1 : 0);
	dv->vsb.visible = (size.y * debug_font_height > rect_get_height(&r) ? 1 : 0);
	dview_get_rect(dv, RECT_DVIEW_CLIENT, &r);

	dv->hsb.visible = (size.x * debug_font_width > rect_get_width(&r) ? 1 : 0);
	dv->vsb.visible = (size.y * debug_font_height > rect_get_height(&r) ? 1 : 0);
	dview_get_rect(dv, RECT_DVIEW_CLIENT, &r);

	col.y = (rect_get_height(&r) - 2 * BORDER_YTHICKNESS /*+ debug_font_height  - 1*/) / debug_font_height;
	col.x = (rect_get_width(&r) - 2 * BORDER_XTHICKNESS /*+ debug_font_width - 1*/) / debug_font_width;

	vsize.y = size.y - pos.y;
	vsize.x = size.x - pos.x;
	if(vsize.y > col.y)
		vsize.y = col.y;
	else if(vsize.y < col.y) {
		pos.y = size.y-col.y;
		if(pos.y < 0)
			pos.y = 0;
		vsize.y = size.y-pos.y;
	}
	if(vsize.x > col.x)
		vsize.x = col.x;
	else if(vsize.x < col.x) {
		pos.x = size.x-col.x;
		if(pos.x < 0)
			pos.x = 0;
		vsize.x = size.x-pos.x;
	}

	dv->view->set_visible_position(pos);
	dv->view->set_visible_size(vsize);

	if(dv->hsb.visible) {
		int span = (rect_get_width(&r) - 2 * BORDER_XTHICKNESS) / debug_font_width;

		if(pos.x + span > size.x)
			pos.x = size.x - span;
		if(pos.x < 0)
			pos.x = 0;
		dv->hsb.lower = 0;
		dv->hsb.upper = size.x;
		dv->hsb.value = pos.x;
		dv->hsb.step_increment = 1;
		dv->hsb.page_increment = span;
		dv->hsb.page_size = span;

		dv->view->set_visible_position(pos);
	}

	if(dv->vsb.visible) {
		int span = (rect_get_height(&r) - 2 * BORDER_YTHICKNESS) / debug_font_height;

		if(pos.y + span > size.y)
			pos.y = size.y - span;
		if(pos.y < 0)
			pos.y = 0;
		dv->vsb.lower = 0;
		dv->vsb.upper = size.y;
		dv->vsb.value = pos.y;
		dv->vsb.step_increment = 1;
		dv->vsb.page_increment = span;
		dv->vsb.page_size = span;

		dv->view->set_visible_position(pos);
	}
}

static void dview_update(debug_view &dw, void *osdprivate)
{
	DView *dv = (DView *) osdprivate;

	dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);

#if 0
	debug_view_xy size = dw.total_size();

	if((dv->tr != size.y) || (dv->tc != size.x))
		gtk_widget_queue_resize(GTK_WIDGET(dv));
	else
		gtk_widget_queue_draw(GTK_WIDGET(dv));
#endif
}

static void debugint_exit(running_machine &machine)
{
	for (DView *ndv = list; ndv != NULL; )
	{
		DView *temp = ndv;
		ndv = ndv->next;
		dview_free(temp);
	}
	if (debug_font != NULL)
	{
		render_font_free(debug_font);
		debug_font = NULL;
	}

}

void debugint_init(running_machine *machine)
{
	unicode_char ch;
	int chw;
	debug_font = render_font_alloc("ui.bdf"); //ui_get_font();
	debug_font_width = 0;
	debug_font_height = 15;

	menu = NULL;
	cur_editor = NULL;
	list = NULL;
	focus_view = NULL;

	debug_font_aspect = render_get_ui_aspect();

	for (ch=0;ch<=127;ch++)
	{
		chw = render_font_get_char_width(debug_font, debug_font_height, debug_font_aspect, ch);
		if (chw>debug_font_width)
			debug_font_width = chw;
	}
	debug_font_width++;
	/* FIXME: above does not really work */
	debug_font_width = 10;
	machine->add_notifier(MACHINE_NOTIFY_EXIT, debugint_exit);
}

#if 0
static void set_view_by_name(render_target *target, const char *name)
{
	int i = 0;
	const char *s;

	for (i = 0; ; i++ )
	{
		s = render_target_get_view_name(target, i);
		if (s == NULL)
			return;
		//printf("%d %s\n", i, s);
		if (strcmp(name, s) == 0)
		{
			render_target_set_view(target, i);
			//printf("%d\n", render_target_get_view(target) );
			return;
		}
	}
}
#endif

/*-------------------------------------------------
    Menu Callbacks
  -------------------------------------------------*/

static void process_string(DView *dv, const char *str)
{
	switch (dv->type)
	{
	case DVT_DISASSEMBLY:
		downcast<debug_view_disasm *>(dv->view)->set_expression(str);
		break;
	case DVT_CONSOLE:
		if(!dv->editor.str[0])
			debug_cpu_get_visible_cpu(dv->machine)->debug()->single_step();
		else
			debug_console_execute_command(dv->machine, str, 1);
		break;
	case DVT_MEMORY:
		downcast<debug_view_memory *>(dv->view)->set_expression(str);
		break;
	}
}

static void on_memory_window_activate(DView *dv, const ui_menu_event *event)
{
}

static void on_disassembly_window_activate(DView *dv, const ui_menu_event *event)
{
	DView *ndv;
	render_target *target;
	const debug_view_source *source;

	target = render_get_ui_target();

	ndv = dview_alloc(target, dv->machine, DVT_DISASSEMBLY, 0);
	ndv->editor.active = TRUE;
	ndv->editor.container = render_container_get_ui();
	source = ndv->view->source();
	dview_set_title(ndv, source->name());
	set_focus_view(ndv);

}

static void on_disasm_cpu_activate(DView *dv, const ui_menu_event *event)
{
	const debug_view_source *current = dv->view->source();

	if (event->iptkey == IPT_UI_RIGHT)
	{
		current = current->next();
		if (current == NULL)
		{
			current = dv->view->source_list().head();
		}
		dv->view->set_source(*current);
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
		dview_set_title(dv, current->name());
	}
}

static void on_log_window_activate(DView *dv, const ui_menu_event *event)
{
	DView *ndv;
	render_target *target;

	target = render_get_ui_target();
	ndv = dview_alloc(target, dv->machine, DVT_LOG, 0);
	dview_set_title(ndv, "Log");
	set_focus_view(ndv);
}

static void on_close_activate(DView *dv, const ui_menu_event *event)
{
	if (focus_view == dv)
		set_focus_view(dv->next);
	dview_free(dv);
}

static void on_run_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine)->debug()->go();
}

#if 0
void on_run_h_activate(DView *dv, const ui_menu_event *event)
{
	debugwin_show(0);
	debug_cpu_get_visible_cpu(dv->machine)->debug()->go();
}
#endif

static void on_run_cpu_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine)->debug()->go_next_device();
}

static void on_run_irq_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine)->debug()->go_interrupt();
}

static void on_run_vbl_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine)->debug()->go_vblank();
}

static void on_step_into_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine)->debug()->single_step();
}

static void on_step_over_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine)->debug()->single_step_over();
}

#ifdef UNUSED_CODE
static void on_step_out_activate(DView *dv, const ui_menu_event *event)
{
	debug_cpu_get_visible_cpu(dv->machine)->debug()->single_step_out();
}
#endif

static void on_hard_reset_activate(DView *dv, const ui_menu_event *event)
{
	dv->machine->schedule_hard_reset();
}

static void on_soft_reset_activate(DView *dv, const ui_menu_event *event)
{
	dv->machine->schedule_soft_reset();
	debug_cpu_get_visible_cpu(dv->machine)->debug()->go();
}

static void on_exit_activate(DView *dv, const ui_menu_event *event)
{
	dv->machine->schedule_exit();
}

static void on_view_opcodes_activate(DView *dv, const ui_menu_event *event)
{
	debug_view_disasm *dasmview = downcast<debug_view_disasm *>(focus_view->view);
	disasm_right_column rc = dasmview->right_column();
	disasm_right_column new_rc = DASM_RIGHTCOL_NONE;

	if (event->iptkey == IPT_UI_RIGHT)
	{
		switch (rc)
		{
		case DASM_RIGHTCOL_RAW:			new_rc = DASM_RIGHTCOL_ENCRYPTED; break;
		case DASM_RIGHTCOL_ENCRYPTED:	new_rc = DASM_RIGHTCOL_COMMENTS; break;
		case DASM_RIGHTCOL_COMMENTS:	new_rc = DASM_RIGHTCOL_RAW; break;
		default:						break;
		}
		dasmview->set_right_column(new_rc);
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, TRUE);
	}
}

static void on_run_to_cursor_activate(DView *dv, const ui_menu_event *event)
{
	char command[64];

	if (dv->view->cursor_visible() && debug_cpu_get_visible_cpu(dv->machine) == dv->view->source()->device())
	{
		offs_t address = downcast<debug_view_disasm *>(dv->view)->selected_address();
		sprintf(command, "go %X", address);
		debug_console_execute_command(dv->machine, command, 1);
	}
}

/*-------------------------------------------------
    editor
  -------------------------------------------------*/

static void render_editor(DView_edit *editor)
{
	float width, maxwidth;
	float x1, y1, x2, y2;

	render_container_empty(editor->container);
	/* get the size of the text */
	ui_draw_text_full(editor->container, editor->str, 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
					  DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(width, 0.5);

	/* compute our bounds */
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = 0.25f;
	y2 = 0.45f - UI_BOX_TB_BORDER;

	/* draw a box */
	ui_draw_outlined_box(editor->container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	/* take off the borders */
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	/* draw the text within it */
	ui_draw_text_full(editor->container, editor->str, x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
					  DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

}

/*-------------------------------------------------
    menu_main_populate - populate the main menu
  -------------------------------------------------*/

static void CreateMainMenu(running_machine *machine)
{
	const char *subtext = "";
	int rc;
	astring title;

	if (menu != NULL)
		ui_menu_free(menu);
	menu = ui_menu_alloc(machine, render_container_get_ui(),NULL,NULL);

	switch (focus_view->type)
	{
	case DVT_DISASSEMBLY:
		title = "Disassembly:";
		break;
	case DVT_CONSOLE:
		title = "Console:";
		break;
	case DVT_LOG:
		title = "Log:";
		break;
	case DVT_MEMORY:
		title = "Memory:";
		break;
	case DVT_STATE:
		title = "State:";
		break;
	}

	ui_menu_item_append(menu, title.cat(focus_view->title), NULL, MENU_FLAG_DISABLE, NULL);
	ui_menu_item_append(menu, MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	switch (focus_view->type)
	{
	case DVT_DISASSEMBLY:
	{
		rc = downcast<debug_view_disasm *>(focus_view->view)->right_column();
		switch(rc)
		{
		case DASM_RIGHTCOL_RAW:			subtext = "Raw Opcodes"; break;
		case DASM_RIGHTCOL_ENCRYPTED:	subtext = "Enc Opcodes"; break;
		case DASM_RIGHTCOL_COMMENTS:		subtext = "Comments"; break;
		}
		ui_menu_item_append(menu, "View", subtext, MENU_FLAG_RIGHT_ARROW, (void *)on_view_opcodes_activate);
		ui_menu_item_append(menu, "Run to cursor", NULL, 0, (void *)on_run_to_cursor_activate);

		if (!dview_is_state(focus_view, VIEW_STATE_FOLLOW_CPU))
		{
			ui_menu_item_append(menu, "CPU", focus_view->view->source()->name(), MENU_FLAG_RIGHT_ARROW, (void *)on_disasm_cpu_activate);
		}
		ui_menu_item_append(menu, MENU_SEPARATOR_ITEM, NULL, 0, NULL);
		break;
	}
	}

	/* add input menu items */

	ui_menu_item_append(menu, "New Memory Window", NULL, 0, (void *)on_memory_window_activate);
	ui_menu_item_append(menu, "New Disassembly Window", NULL, 0, (void *)on_disassembly_window_activate);
	ui_menu_item_append(menu, "New Error Log Window", NULL, 0, (void *)on_log_window_activate);
	ui_menu_item_append(menu, MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	ui_menu_item_append(menu, "Run", NULL, 0, (void *)on_run_activate);
	ui_menu_item_append(menu, "Run to Next CPU", NULL, 0, (void *)on_run_cpu_activate);
	ui_menu_item_append(menu, "Run until Next Interrupt on This CPU", NULL, 0, (void *)on_run_irq_activate);
	ui_menu_item_append(menu, "Run until Next VBLANK", NULL, 0, (void *)on_run_vbl_activate);
	ui_menu_item_append(menu, MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	ui_menu_item_append(menu, "Step Into", NULL, 0, (void *)on_step_into_activate);
	ui_menu_item_append(menu, "Step Over", NULL, 0, (void *)on_step_over_activate);
	ui_menu_item_append(menu, MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	ui_menu_item_append(menu, "Soft Reset", NULL, 0, (void *)on_soft_reset_activate);
	ui_menu_item_append(menu, "Hard Reset", NULL, 0, (void *)on_hard_reset_activate);
	ui_menu_item_append(menu, MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	if (!dview_is_state(focus_view, VIEW_STATE_FOLLOW_CPU))
		ui_menu_item_append(menu, "Close Window", NULL, 0, (void *)on_close_activate);
	ui_menu_item_append(menu, "Exit", NULL, 0, (void *)on_exit_activate);
}

static int map_point(DView *dv, INT32 target_x, INT32 target_y, INT32 *mapped_x, INT32 *mapped_y)
{
	rectangle pos;

	/* default to point not mapped */
	*mapped_x = -1;
	*mapped_y = -1;

	pos = dv->bounds;
	pos.min_x += dv->ofs_x;
	pos.max_x += dv->ofs_x;
	pos.min_y += dv->ofs_y;
	pos.max_y += dv->ofs_y;
	//render_target_get_component_container(target, name, &pos);

	if (target_x >= pos.min_x && target_x <= pos.max_x && target_y >= pos.min_y && target_y <= pos.max_y)
	{
		*mapped_x = target_x - pos.min_x;
		*mapped_y = target_y - pos.min_y;
		return TRUE;
	}
	return FALSE;
}

static void handle_mouse(running_machine *machine)
{
	render_target *	mouse_target;
	INT32			x,y;
	int				button;

	if (menu != NULL)
		return;

	mouse_target = ui_input_find_mouse(machine, &x, &y, &button);

	if (mouse_target == NULL)
		return;
	//printf("mouse %d %d %d\n", x, y, button);

	for (DView *dv = list; dv != NULL; dv = dv->next)
	{
		if (mouse_target == dv->target)
		{
			if (dview_on_mouse(dv, x, y, button))
				break;
		}
	}
}


/*-------------------------------------------------
    handle_editor - handle the editor
-------------------------------------------------*/

static void handle_editor(running_machine *machine)
{
	if (focus_view->editor.active)
	{
		ui_event event;

		/* loop while we have interesting events */
		while (ui_input_pop_event(machine, &event))
		{
			switch (event.event_type)
			{
			case UI_EVENT_CHAR:
				/* if it's a backspace and we can handle it, do so */
				if ((event.ch == 8 || event.ch == 0x7f) && focus_view->editor.str.len() > 0)
				{
					/* autoschow */
					cur_editor = &focus_view->editor;
					cur_editor->str = cur_editor->str.substr(0, cur_editor->str.len()-1);
				}
				/* if it's any other key and we're not maxed out, update */
				else if (event.ch >= ' ' && event.ch < 0x7f)
				{
					char buf[10];
					int ret;
					/* autoschow */
					cur_editor = &focus_view->editor;
					ret = utf8_from_uchar(buf, 10, event.ch);
					buf[ret] = 0;
					cur_editor->str = cur_editor->str.cat(buf);
				}
				break;
			default:
				break;
			}
		}
		if (cur_editor != NULL)
		{
			render_editor(cur_editor);
			if (ui_input_pressed(machine, IPT_UI_SELECT))
			{
				process_string(focus_view, focus_view->editor.str);
				focus_view->editor.str = "";
				cur_editor = NULL;
			}
			if (ui_input_pressed(machine, IPT_UI_CANCEL))
				cur_editor = NULL;
		}
	}

}


/*-------------------------------------------------
    menu_main - handle the main menu
-------------------------------------------------*/

static void handle_menus(running_machine *machine)
{
	const ui_menu_event *event;

	render_container_empty(render_container_get_ui());
	ui_input_frame_update(*machine);
	if (menu != NULL)
	{
		/* process the menu */
		event = ui_menu_process(machine, menu, 0);
		if (event != NULL && (event->iptkey == IPT_UI_SELECT || (event->iptkey == IPT_UI_RIGHT)))
		{
			//ui_menu_free(menu);
			//menu = NULL;
			((void (*)(DView *, const ui_menu_event *)) event->itemref)(focus_view, event);
			//ui_menu_stack_push(ui_menu_alloc(machine, menu->container, (ui_menu_handler_func)event->itemref, NULL));
			CreateMainMenu(machine);
		}
		else if (ui_input_pressed(machine, IPT_UI_CONFIGURE))
		{
			ui_menu_free(menu);
			menu = NULL;
		}
	}
	else
	{
		/* turn on menus if requested */
		if (ui_input_pressed(machine, IPT_UI_CONFIGURE))
			CreateMainMenu(machine);
		/* turn on editor if requested */
		//if (ui_input_pressed(machine, IPT_UI_UP) && focus_view->editor.active)
		//  cur_editor = &focus_view->editor;
		handle_editor(machine);
	}
}

//============================================================
//  followers_set_cpu
//============================================================

static void followers_set_cpu(running_device *device)
{
	astring title;

	for (DView *dv = list; dv != NULL; dv = dv->next)
	{
		if (dview_is_state(dv, VIEW_STATE_FOLLOW_CPU))
		{
			const debug_view_source *source = dv->view->source_list().match_device(device);
			switch (dv->type)
			{
			case DVT_DISASSEMBLY:
			case DVT_STATE:
				dv->view->set_source(*source);
				title.printf("%s", source->name());
				dview_set_title(dv, title);
				break;
			}
		}
	}
	// and recompute the children
	//console_recompute_children(main_console);
}


static void dview_update_view(DView *dv)
{
	INT32 old_rt_width = dv->rt_width;
	INT32 old_rt_height = dv->rt_height;

	render_target_get_bounds(dv->target, &dv->rt_width, &dv->rt_height, NULL);
	if (dview_is_state(dv, VIEW_STATE_NEEDS_UPDATE) || dv->rt_width != old_rt_width || dv->rt_height != old_rt_height)
	{
		dview_size_allocate(dv);
		dview_draw(dv);
		dview_set_state(dv, VIEW_STATE_NEEDS_UPDATE, FALSE);
	}
}


static void update_views(void)
{
	DView *dv, *prev;

	LIST_GET_LAST(list, dv);
	while (dv != NULL)
	{
		dview_update_view(dv);
		LIST_GET_PREVIOUS(list, dv, prev);
		dv = prev;
	}
}


void debugint_wait_for_debugger(running_device *device, int firststop)
{

	if (firststop && list == NULL)
	{
		DView *dv;
		render_target *target;

		target = render_get_ui_target();

		//set_view_by_name(target, "Debug");

		dv = dview_alloc(target, device->machine, DVT_DISASSEMBLY, VIEW_STATE_FOLLOW_CPU);
		dv->editor.active = TRUE;
		dv->editor.container = render_container_get_ui();
		dv = dview_alloc(target, device->machine, DVT_STATE, VIEW_STATE_FOLLOW_CPU);
		dv = dview_alloc(target, device->machine, DVT_CONSOLE, VIEW_STATE_FOLLOW_CPU);
		dview_set_title(dv, "Console");
		dv->editor.active = TRUE;
		dv->editor.container = render_container_get_ui();
		set_focus_view(dv);
	}

	followers_set_cpu(device);

	//ui_update_and_render(device->machine, render_container_get_ui());
	update_views();
	osd_update(device->machine, FALSE);
	handle_menus(device->machine);
	handle_mouse(device->machine);
	//osd_sleep(osd_ticks_per_second()/60);

}

void debugint_update_during_game(running_machine *machine)
{
	if (!debug_cpu_is_stopped(machine) && machine->phase() == MACHINE_PHASE_RUNNING)
	{
		update_views();
	}
}
