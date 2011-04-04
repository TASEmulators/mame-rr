print("NeoGeo fighting game hitbox viewer")
print("April 4, 2011")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis")
print("Lua hotkey 4: toggle pushboxes")
print("Lua hotkey 5: toggle throwable boxes") print()

local VULNERABILITY_COLOR      = 0x7777FF40
local ATTACK_COLOR             = 0xFF000040
local PROJ_VULNERABILITY_COLOR = 0x00FFFF40
local PROJ_ATTACK_COLOR        = 0xFF66FF60
local PUSH_COLOR               = 0x00FF0020
local GUARD_COLOR              = 0xCCCCFF40
local THROW_COLOR              = 0xFFFF0060
local THROWABLE_COLOR          = 0xFFFFFF20
local AXIS_COLOR               = 0xFFFFFFFF
local BLANK_COLOR              = 0xFFFFFFFF
local AXIS_SIZE                = 12
local MINI_AXIS_SIZE           = 2
local GROUND_THROW_HEIGHT      = 32
local DRAW_DELAY               = 0
local BLANK_SCREEN             = false
local DRAW_AXIS                = false
local DRAW_MINI_AXIS           = false
local DRAW_PUSHBOXES           = true
local DRAW_THROWABLE_BOXES     = false

local UNDEFINED_BOX          = 0
local VULNERABILITY_BOX      = 1
local ATTACK_BOX             = 2
local PROJ_VULNERABILITY_BOX = 3
local PROJ_ATTACK_BOX        = 4
local PUSH_BOX               = 5
local GUARD_BOX              = 6
local THROW_BOX              = 7
local THROWABLE_BOX          = 8
local GAME_PHASE_NOT_PLAYING = 0

local fill = {
	VULNERABILITY_COLOR,
	ATTACK_COLOR,
	PROJ_VULNERABILITY_COLOR,
	PROJ_ATTACK_COLOR,
	PUSH_COLOR,
	GUARD_COLOR,
	THROW_COLOR,
	THROWABLE_COLOR,
}

local outline = {
	bit.bor(0xFF, VULNERABILITY_COLOR),
	bit.bor(0xFF, ATTACK_COLOR),
	bit.bor(0xFF, PROJ_VULNERABILITY_COLOR),
	bit.bor(0xFF, PROJ_ATTACK_COLOR),
	bit.bor(0xC0, PUSH_COLOR),
	bit.bor(0xFF, GUARD_COLOR),
	bit.bor(0xFF, THROW_COLOR),
	bit.bor(0xC0, THROWABLE_COLOR),
}

local a,v,p,g,t = ATTACK_BOX,VULNERABILITY_BOX,PROJ_VULNERABILITY_BOX,GUARD_BOX,THROW_BOX
-- x = nil (unknown)

local profile = {
	{
		games = {"kof94"},
		address = {
			game_phase       = 0x1090B3,
			left_screen_edge = 0x10904C,
			top_screen_edge  = 0x10905C,
			obj_ptr_list     = 0x1097A6,
		},
		offset = {status = 0x7A},
		box_types = {a,g,v,v,a},
	},
	{
		games = {"kof95"},
		address = {game_phase = 0x10B088},
		box_types = {a,g,v,v,a},
	},
	{
		games = {"kof96"},
		address = {game_phase = 0x10B08E},
		throw_boxes = true,
		box_types = {
			v,v,v,v,v,v,v,v,g,g,v,a,a,a,a,a,
			a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,
			a,a,a,a,a,a,a,a,a,a,a,a,g,g,p,p,
			p,p,p,p
		},
	},
	{
		games = {"kof97"},
		address = {game_phase = 0x10B092},
		throw_boxes = true,
		box_types = {
			v,v,v,v,v,v,v,v,v,g,g,a,a,a,a,a,
			a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,
			a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,
			a,a,a,a,a,a,a,g,g,p,p,p,p,p,p
		},
	},
	{
		games = {"kof98"},
		address = {game_phase = 0x10B094},
		throw_boxes = true,
		box_types = {
			v,v,v,v,v,v,v,v,v,g,g,a,a,a,a,a,
			a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,
			a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,
			a,a,a,a,a,a,a,g,g,p,p,p,p,p,p
		},
	},
	{
		games = {"kof99", "kof2000"},
		address = {game_phase = 0x10B048},
		throw_boxes = true,
		box_types = {
			v,v,v,v,v,v,v,v,v,g,g,a,a,a,a,a,
			a,a,a,a,a,a,v,a,a,a,a,a,a,a,a,a,
			a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,
			a,a,a,a,a,a,a,g,g,p,p,p,p,p,p
		},
	},
	{
		games = {"kof2001", "kof2002"},
		address = {game_phase = 0x10B056},
		throw_boxes = true,
		box_types = {
			v,v,v,v,v,v,v,v,v,g,g,a,a,a,a,a,
			a,a,a,a,a,a,v,a,a,a,a,a,a,a,a,a,
			a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,
			a,a,a,a,a,a,a,g,g,p,p,p,p,p,p
		},
	},
	{
		games = {"fatfury1"},
		number_players = 3,
		y_value = "absolute subtract",
		box_engine = "fatal fury 1",
		obj_engine = "fatal fury 1",
		address = {
			game_phase       = 0x1042CC,
			left_screen_edge = 0x104120,
			top_screen_edge  = 0x104128,
			player           = 0x100300,
		},
		offset = {
			facing_dir        = 0x35,
			status            = 0x36,
			hitbox_ptr        = 0xB2,
			invulnerable      = 0xCD,
		},
		active_status = 0x40,
		versions = {--[[
			[{"fatfury1"}] = +0x000
				active_breakpoints = {0x0102B2, 0x0102BC},
				box_breakpoints = {},
			]]
		},
		box_types = {
			PUSH_BOX,g,g,a,a,x,a, [0]=v
		},
	},
	{
		games = {"fatfury2"},
		address = {
			game_phase       = 0x100B89,
			left_screen_edge = 0x100B20,
			top_screen_edge  = 0x100B28,
			player           = 0x100300,
		},
		offset = {
			facing_dir        = 0x67,
			status            = 0x7A,
			hitbox_ptr_offset = 0x88,
			hitbox_ptr        = 0x8A,
			obj_ptr           = 0xBA,
			obj_inactive      = 0xFA,
			invulnerable      = nil, --find
		},
		active_status = 0x20,
		versions = {--[[
			[{"fatfury2"}] = +0x000
				active_breakpoints = {0x006930, 0x00695A},
				box_breakpoints = {},
			]]
		},
		box_types = {
			v,v,g,g,a,a,a,a,g,x,x,x,x,x,a,v,
			[0]=PUSH_BOX
		},
	},
	{
		games = {"fatfursp"},
		address = {
			game_phase       = 0x100A62,
			left_screen_edge = 0x100B20,
			top_screen_edge  = 0x100B28,
		},
		offset = {
			facing_dir   = 0x62,
			status       = 0x7C,
			hitbox_ptr   = 0x8A,
			obj_ptr      = 0xAC,
			obj_inactive = 0xFA,
			invulnerable = nil, --find
		},
		active_status = 0x20,
		reverse = true,
		versions = {--[[
			[{"fatfursp, fatfursa, fatfurspa"}] = +0x000,
				active_breakpoints = {0x01A424, 0x01A440},
				box_breakpoints = {},
			]]
		},
		box_types = {
			v,v,g,g,a,a,a,a,g,x,x,x,x,x,a,v,
			x,x,x,x,x,x,a, [0]=PUSH_BOX
		},
	},
	{
		games = {"fatfury3"},
		address = {
			game_phase       = 0x100B24,
			obj_ptr_list     = 0x10088A,
			left_screen_edge = 0x100B20,
			top_screen_edge  = 0x100B28,
		},
		offset = {
			status       = 0x6A,
			obj_inactive = 0xA8,
			invulnerable = 0xAF,
			no_push      = 0xEA,
		},
		versions = {
			[{"fatfury3"}] = 0x0648EE, --[[
				active_breakpoints = {0x02D416, 0x02D3D0},
				box_breakpoints = {
					[0x0681F8] = "rbff1 airthrow",
				},]]
		},
		box_types = {
			v,v,v,g,g,v,a,a,a,a,a,a,a,a,a,a,
			a,t,g,g,a,a,a,a,a,a,a, [0]=v
		},
	},
	{
		games = {"rbff1"},
		address = {
			game_phase       = 0x106D75,
			obj_ptr_list     = 0x100890,
			left_screen_edge = 0x100B20,
			top_screen_edge  = 0x100B28,
		},
		offset = {
			status       = 0x6A,
			obj_inactive = 0xA8,
			invulnerable = 0xAF,
			no_push      = 0xEA,
		},
		versions = {
			[{"rbff1" }] = 0x06C244, --[[ +0x00
				active_breakpoints = {0x023AFE, 0x023B44},
				box_breakpoints = {
					[0x0285B8] = "rbff1 vulnerability",
					[0x06CB58] = "rbff1 throw",
					[0x0709BA] = "rbff1 airthrow",
					[0x043D22] = "rbff1 spiderthrow",
				},]]
			[{"rbff1a"}] = 0x06C25E, --[[ +0x1A
				active_breakpoints = {0x023B18, 0x023B5E},
				box_breakpoints = {
					[0x0285D2] = "rbff1 vulnerability",
					[0x06CB72] = "rbff1 throw",
					[0x0709D4] = "rbff1 airthrow",
					[0x043D3C] = "rbff1 spiderthrow",
				},]]
		},
		box_types = {
			v,v,v,g,g,v,a,a,a,a,a,a,a,a,a,a,
			a,a,g,g,a,a,a,a,a,a,a,a,a,a,a,a,
			a,a,a,a,a,a,a, [0]=v
		},
	},
	{
		games = {"rbffspec"},
		no_boxes = true,
		address = {
			game_phase   = 0x1096FA,
			obj_ptr_list = 0x100C92,
		},
		offset = {
			status       = 0x76,
			obj_inactive = 0xAA,
			invulnerable = 0xB1,
			no_push      = 0xEC,
		},
		versions = {
			[{"rbffspeck"}] = 0x072E5E, --[[ +0x000
				active_breakpoints = {0x02555A, 0x0255A0},
				box_breakpoints = {
					[0x073D58] = "rbff2 throw",
					[0x076BF2] = "rbff2 airthrow",
					[0x045554] = "rbff2 spiderthrow",
				},]]
			[{"rbffspec" }] = 0x072F7A, --[[ +0x11C
				active_breakpoints = {0x025682, 0x0256C8},
				box_breakpoints = {
					[0x073E74] = "rbff2 throw",
					[0x076D0E] = "rbff2 airthrow",
					[0x045670] = "rbff2 spiderthrow",
				},]]
		},
	},
	{
		games = {"rbff2"},
		no_boxes = true,
		address = {
			game_phase   = 0x10B1A4,
			obj_ptr_list = 0x100C92,
		},
		offset = {
			status       = 0x76,
			obj_inactive = 0xAA,
			invulnerable = 0xB1,
			no_push      = 0xEC,
		},
		versions = {
			[{"rbff2k"}] = 0x05C898, --[[ +0x000
				active_breakpoints = {0x01E980, 0x01E9C6},
				box_breakpoints = {
					[0x05D700] = "rbff2 throw",
					[0x060334] = "rbff2 airthrow",
				},]]
			[{"rbff2" }] = 0x05C99C, --[[ +0x104
				active_breakpoints = {0x01E958, 0x01E99E},
				box_breakpoints = {
					[0x05D804] = "rbff2 throw",
					[0x060438] = "rbff2 airthrow",
				},]]
			[{"rbff2h"}] = 0x05C9BC, --[[ +0x124
				active_breakpoints = {0x01E958, 0x01E99E},
				box_breakpoints = {
					[0x05D824] = "rbff2 throw",
					[0x060458] = "rbff2 airthrow",
				},]]
		},
	},
	{
		games = {"garou"},
		no_boxes = true,
		address = {
			game_phase   = 0x10B259,
			obj_ptr_list = 0x100C88,
		},
		offset = {
			status       = 0x76,
			obj_inactive = 0xAC,
			invulnerable = 0xB3,
			no_push      = 0xEE,
		},
		versions = {
			[{"garoup","garoubl"}] = 0x0356BE, --[[
				active_breakpoints = {0x01C42A, 0x01C466},
				box_breakpoints = {
					[0x028B14] = "garou throw",
				},]]
			[{"garou", "garouo" }] = 0x0358B0, --[[
				active_breakpoints = {0x01BC24, 0x01BC74},
				box_breakpoints = {
					[0x028928] = "garou throw",
				},]]
		},
	},
}


local get_box_params = {
	["fatal fury 1"] = function()
		return {top = 0x6, bottom = 0x8, left = 0x2, right = 0x4, space = 0xA, scale = 1, header = 2, read = memory.readwordsigned}
	end,

	["fatal fury 2"] = function()
		return {top = 0x2, bottom = 0x3, left = 0x4, right = 0x5, space = 0x6, scale = 4, header = 0, read = memory.readbytesigned}
	end,

	["garou"] = function()
		return {top = 0x1, bottom = 0x2, left = 0x3, right = 0x4, space = 0x5, scale = 4, header = 0, read = memory.readbytesigned}
	end,
}

local prepare_module = {
	["king of fighters"] = function(g)
		g.obj_engine     = "king of fighters"
		g.y_value        = "direct"
		g.ground_level   = 16
		g.address.player           = 0x108100
		g.address.left_screen_edge = g.address.left_screen_edge or g.address.game_phase + 0x038
		g.address.top_screen_edge  = g.address.top_screen_edge  or g.address.game_phase + 0x040
		g.address.obj_ptr_list     = g.address.obj_ptr_list     or g.address.game_phase + 0xE90
		g.offset = g.offset or {}
		g.offset.player_space = g.offset.player_space or 0x200
		g.offset.x_position   = g.offset.x_position   or 0x18
		g.offset.y_position   = g.offset.y_position   or 0x26
		g.offset.facing_dir   = g.offset.facing_dir   or 0x31
		g.offset.status       = g.offset.status       or 0x7C
		g.box_list = {
			{offset = 0xA4, type = PUSH_BOX},
			{offset = 0x9F, active_bit = 3, type = UNDEFINED_BOX},
			{offset = 0x9A, active_bit = 2, type = UNDEFINED_BOX},
			{offset = 0x95, active_bit = 1, type = UNDEFINED_BOX},
			{offset = 0x90, active_bit = 0, type = UNDEFINED_BOX},
		}
		for _, box in ipairs(g.box_list) do
			box.active = box.active or box.active_bit and bit.lshift(1, box.active_bit)
		end
		if g.throw_boxes then
			table.insert(g.box_list, {offset = 0x18D, active = 0x7E, type = THROWABLE_BOX})
			table.insert(g.box_list, {offset = 0x188, id = 0x192, type = THROW_BOX})
		end
	end,

	["fatal fury"] = function(g)
		g.box_engine     = g.box_engine or (g.offset.hitbox_ptr_offset and "fatal fury 2") or "garou"
		g.obj_engine     = g.obj_engine or (g.offset.obj_ptr and "fatal fury 2") or (g.address.obj_ptr_list and "garou")
		g.y_value        = g.y_value or "absolute"
		g.ground_level   = 23
		g.front_plane    = 0x18
		g.address.player = g.address.player or 0x100400
		g.address.left_screen_edge = g.address.left_screen_edge or 0x100E20
		g.address.top_screen_edge  = g.address.top_screen_edge  or 0x100E28
		g.offset.player_space = 0x100
		g.offset.char_id      = 0x10
		g.offset.x_position   = 0x20
		g.offset.z_position   = 0x24
		g.offset.y_position   = 0x28
		g.offset.facing_dir   = g.offset.facing_dir or 0x71
		g.offset.hitbox_ptr   = g.offset.hitbox_ptr or 0x7A
		g.reverse_facing = g.reverse and -1 or 1
		g.active_status  = g.active_status or 0x10
		g.box = get_box_params[g.box_engine]()
	end,
}


for game in ipairs(profile) do
	local g = profile[game]
	g.engine = g.engine or 
		(g.games[1]:find("kof") and "king of fighters") or
		((g.games[1]:find("fatfur") or g.games[1]:find("rbff") or g.games[1]:find("garou")) and "fatal fury")
	g.number_players = g.number_players or 2
	prepare_module[g.engine](g)
end

local game, version
local globals = {
	register_count   = 0,
	last_frame       = 0,
	game_phase       = 0,
	left_screen_edge = 0,
	top_screen_edge  = 0,
}
local frame_buffer = {}
emu.update_func = fba and emu.registerafter or emu.registerbefore
--memory.writebyte(0x100000, 2)


--------------------------------------------------------------------------------
-- update functions to be called by breakpoints (not implemented in Lua yet)

local get_player = function(address)
  for p = 1, game.number_players do
    if game.address.player + (p-1)*game.offset.address == address then
      return frame_buffer[DRAW_DELAY+1][p]
    end
  end
end


local breakpoint = {
	["rbff1 throw"] = function() --grabs axis
		local thrower = get_player(memory.getregister("a4"))
		if not thrower then
			return
		end
		local range = memory.getregister("d4") * thrower.facing_dir
		table.insert(thrower, {
			type   = THROW_BOX,
			top    = thrower.pos_y - GROUND_THROW_HEIGHT,
			bottom = thrower.pos_y,
			left   = thrower.pos_x,
			right  = thrower.pos_x + range,
		})
	end,

	["rbff1 airthrow"] = function() --grabs axis
		local thrower = get_player(memory.getregister("a4"))
		if not thrower then
			return
		end
		local range_ptr = memory.getregister("a0")
		table.insert(thrower, {
			type   = THROW_BOX,
			top    = thrower.pos_y - memory.readwordsigned(range_ptr - 8),
			bottom = thrower.pos_y - memory.readwordsigned(range_ptr - 6),
			left   = thrower.pos_x - memory.readwordsigned(range_ptr - 4) * thrower.facing_dir,
			right  = thrower.pos_x - memory.readwordsigned(range_ptr - 2) * thrower.facing_dir,
		})
	end,

	["rbff1 spiderthrow"] = function(bp) --grabs axis
		local thrower = get_player(memory.getregister("a4"))
		if not thrower or thrower.pos_y > memory.readword(bp + 0x12) then
			return
		end
		local range = memory.readword(bp + 0x02) * thrower.facing_dir
		table.insert(thrower, {
			type   = THROW_BOX,
			top    = thrower.pos_y - GROUND_THROW_HEIGHT,
			bottom = thrower.pos_y,
			left   = thrower.pos_x,
			right  = thrower.pos_x + range,
		})
	end,

	["rbff2 throw"] = function() --grabs pushbox
		local thrower = get_player(memory.getregister("a4"))
		if not thrower then
			return
		end
		local range = memory.getregister("d5") * thrower.facing_dir --pushbox front edge
		range = range + memory.getregister("d4") * thrower.facing_dir
		table.insert(thrower, {
			type   = THROW_BOX,
			top    = thrower.pos_y - GROUND_THROW_HEIGHT,
			bottom = thrower.pos_y,
			left   = thrower.pos_x,
			right  = thrower.pos_x + range,
		})
	end,

	["rbff2 airthrow"] = function() --grabs axis
		local thrower = get_player(memory.getregister("a4"))
		if not thrower then
			return
		end
		local range_ptr = memory.getregister("a0")
		table.insert(thrower, {
			type   = THROW_BOX,
			top    = thrower.pos_y - memory.readword(range_ptr + 0x2),
			bottom = thrower.pos_y + memory.readword(range_ptr + 0x2),
			left   = thrower.pos_x,
			right  = thrower.pos_x - memory.readword(range_ptr + 0x0) * thrower.facing_dir,
		})
	end,

	["rbff2 spiderthrow"] = function() --grabs axis
		local thrower = get_player(memory.getregister("a4"))
		if not thrower then
			return
		end
		local range = memory.getregister("d7") * thrower.facing_dir
		table.insert(thrower, {
			type   = THROW_BOX,
			top    = thrower.pos_y - GROUND_THROW_HEIGHT,
			bottom = thrower.pos_y,
			left   = thrower.pos_x,
			right  = thrower.pos_x + range,
		})
	end,

	["garou throw"] = function() --grabs pushbox
		local thrower = get_player(memory.getregister("a4"))
		if not thrower then
			return
		end
		local range_ptr = memory.getregister("a1")
		local top, bottom = memory.readbyte(range_ptr + 0x4), memory.readbyte(range_ptr + 0x5)
		if top == 0 and bottom == 0 then
			top = GROUND_THROW_HEIGHT
		end
		table.insert(thrower, {
			type   = THROW_BOX,
			top    = thrower.pos_y - top,
			bottom = thrower.pos_y - bottom,
			left   = thrower.pos_x - memory.readbyte(range_ptr + 0x2) * thrower.facing_dir,
			right  = thrower.pos_x + memory.readbyte(range_ptr + 0x3) * thrower.facing_dir,
		})
	end,
}


--------------------------------------------------------------------------------
-- prepare the hitboxes

local function get_x(x)
	return x - globals.left_screen_edge
end


local function get_z(z)
	return z - game.front_plane
end


local get_y = {
	["absolute subtract"] = function(y)
		return emu.screenheight() - (y + game.ground_level) - globals.top_screen_edge
	end,

	["absolute"] = function(y)
		return emu.screenheight() - (y + game.ground_level) + globals.top_screen_edge
	end,

	["direct"] = function(y)
		return y - game.ground_level
	end,
}


local get_pushbox_offset = {
	["fatfury3"] = function(obj)
		if memory.readdword(obj.base + 0x28) > 0 then --off ground
			if bit.band(memory.readdword(obj.base + 0xC6), 0xFFC000) > 0 then
				return 0x1C0 --air special moves
			else
				return 0x0E0 --jumping or juggled
			end
		else --on ground
			local be = memory.readdword(obj.base + 0xBE)
			if bit.band(be, 0x1C000044) > 0 then
				return 0x070 --crouching or knocked down
			elseif bit.band(be, 0xE000) > 0 then
				return 0x150 --???
			else
				return 0x000 --standing
			end
		end
	end,

	["rbff1"] = function(obj)
		if memory.readdword(obj.base + 0x28) > 0 then --off ground
			if bit.band(memory.readdword(obj.base + 0xC6), 0xFFFF0000) == 0 then
				return 0x110 --air special moves
			else
				return 0x220 --jumping or juggled
			end
		else --on ground
			if bit.band(memory.readdword(obj.base + 0xBE), 0x14000044) > 0 then
				return 0x088 --crouching or knocked down
			else
				return 0x000 --standing
			end
		end
	end,

	["rbffspec"] = function(obj)
		if memory.readdword(obj.base + 0x28) > 0 then --off ground
			local d2 = memory.readdword(obj.base + 0xC8)
			if (obj.char_id == 7 and bit.band(d2, 0x2000000) > 0 or bit.band(d2, 0xFFFF0000) == 0) or bit.band(d2, 0xFFFF0000) == 0 then
				return 0x150 --air special moves
			else
				return 0x1F8 --jumping or juggled
			end
		else --on ground
			if bit.band(memory.readdword(obj.base + 0xC0), 0x14000046) > 0 then
				return 0x0A8 --crouching or knocked down
			else
				return 0x000 --standing
			end
		end
	end,

	["rbff2"] = function(obj)
		local d1, d2 = memory.readdword(obj.base + 0xC0), memory.readdword(obj.base + 0xC8)
		if obj.char_id == 5 and bit.band(d2, bit.lshift(1, 0xF)) > 0 then
			return 0x000 --standing?
		elseif bit.band(d1, bit.lshift(1, 0x1)) > 0 then
			return 0x0C0 --crouching or knocked down?
		elseif memory.readdword(obj.base + 0x28) > 0 then --off ground
			local char_table_lookup = memory.readdword(globals.pushbox_base - 0x294 + obj.char_id * 4)
			if memory.readdword(char_table_lookup) == 0 and bit.band(d2, 0xFFFF0000) > 0 then
				return 0x240 --air special moves
			else
				return 0x180 --jumping or juggled
			end
		else --on ground
			if bit.band(d1, 0x14000046) > 0 then
				return 0x0C0 --crouching or knocked down
			else
				return 0x000 --standing
			end
		end
	end,

	["garou"] = function(obj)
		if bit.band(memory.readdword(obj.base + 0xC2), 0x2) > 0 then
			return 0x100
		end
		if memory.readdword(obj.base + 0x28) > 0 then --off ground
			local c6, ca, bb = memory.readdword(obj.base + 0xC6), memory.readdword(obj.base + 0xCA), memory.readbyte(obj.base + 0xBB)
			if (bit.band(c6, 0xFFFFFFFF) > 0 or bit.band(ca, 0xFFFFFFDE) > 0)
				and bit.band(bb, bit.lshift(1, 0x4)) == 0 and bit.band(ca, 0xFFFFE01E) > 0 then
				return 0x300 --air special moves
			else
				return 0x200 --jumping or juggled
			end
		else --on ground
			if bit.band(memory.readdword(obj.base + 0xC2), 0x14000046) > 0 then
				return 0x100 --crouching or knocked down
			else
				return 0x000 --standing
			end
		end
	end,
}


local get_box_id = {
	["fatal fury 1"] = function(address)
		return memory.readword(address)
	end,

	["fatal fury 2"] = function(address)
		return bit.band(memory.readword(address), 0x000F)
	end,

	["garou"] = function(address)
		return memory.readbyte(address)
	end,
}


local type_check = {
	["king of fighters"] = {
		[UNDEFINED_BOX] = function(obj, entry, box)
			if bit.band(obj.status, game.box_list[entry].active) == 0 then
				return true
			end
			box.type = game.box_types[box.id]
			if box.type == ATTACK_BOX then
				if game.box_list[entry].active_bit > 0 then
					return true
				elseif obj.projectile then
					box.type = PROJ_ATTACK_BOX
				end
			end
		end,

		[PUSH_BOX] = function(obj, entry, box)
			if box.id == 0xFF or obj.projectile then
				return true
			end
		end,

		[THROW_BOX] = function(obj, entry, box)
			box.id = memory.readbyte(obj.base + game.box_list[entry].id)
			if box.id == 0x00 then
				return true
			else
				memory.writebyte(obj.base + game.box_list[entry].id, 0) --bad
			end
		end,

		[THROWABLE_BOX] = function(obj, entry, box)
			box.id = memory.readbyte(obj.base + game.box_list[entry].active)
			if box.id == 0x01 then
				return true
			end
		end,
	},

	["fatal fury"] = {
		[PUSH_BOX] = function(obj, box)
			if obj.projectile or obj.no_push or globals.same_plane == false or not globals.pushbox_base then
				return true
			else
				box.address = globals.pushbox_base + get_pushbox_offset[globals.game](obj) + bit.lshift(obj.char_id, 3)
				box.id = memory.readbyte(box.address)
			end
		end,

		[UNDEFINED_BOX] = function(obj, box, offset)
			box.address = obj.hitbox_ptr + offset + game.box.header
			box.id = get_box_id[game.box_engine](box.address)
			box.type = game.box_types and game.box_types[box.id] or UNDEFINED_BOX
			if box.type == VULNERABILITY_BOX and obj.invulnerable then
				return true
			elseif box.type == ATTACK_BOX and bit.band(obj.status, game.active_status) == 0 then
				return true
			elseif box.type == ATTACK_BOX and obj.projectile then
				box.type = PROJ_ATTACK_BOX
			--elseif box.type == UNDEFINED_BOX then
				--print(string.format("%02x",box.id))
				--emu.pause()
			end
		end,
	},
}


local define_box = {
	["king of fighters"] = function(obj, entry)
		local box = {
			address = obj.base + game.box_list[entry].offset,
			type = game.box_list[entry].type,
		}
		box.id = memory.readbyte(box.address)

		if type_check[game.engine][box.type](obj, entry, box) then
			return nil
		end

		box.hrad = memory.readbyte(box.address + 0x3)
		box.vrad = memory.readbyte(box.address + 0x4)
		if box.hrad == 0 and box.vrad == 0 then
			return nil
		end
		box.hval = memory.readbytesigned(box.address + 0x1)
		box.vval = memory.readbytesigned(box.address + 0x2)

		box.hval   = obj.pos_x + box.hval * (obj.facing_dir > 0 and -1 or 1)
		box.vval   = obj.pos_y + box.vval
		box.left   = box.hval - box.hrad
		box.right  = box.hval + box.hrad - 1
		box.top    = box.vval - box.vrad
		box.bottom = box.vval + box.vrad - 1

		return box
	end,

	["fatal fury"] = function(obj, type, offset)
		local box = {type = type}
		
		if type_check[game.engine][box.type](obj, box, offset) then
			return nil
		end

		box.top    = obj.pos_y - game.box.read(box.address + game.box.top)    * game.box.scale--obj.scale
		box.bottom = obj.pos_y - game.box.read(box.address + game.box.bottom) * game.box.scale--obj.scale
		box.left   = obj.pos_x - game.box.read(box.address + game.box.left)   * game.box.scale * obj.facing_dir
		box.right  = obj.pos_x - game.box.read(box.address + game.box.right)  * game.box.scale * obj.facing_dir
		if box.top == box.bottom and box.left == box.right then
			return nil
		end
		box.hval   = (box.right + box.left)/2
		box.vval   = (box.bottom + box.top)/2

		return box
	end,
}


local modify_object = {
	["fatal fury 1"] = function(obj)
		obj.facing_dir = memory.readbyte(obj.base + game.offset.facing_dir) > 0 and -1 or 1
		obj.hitbox_ptr = memory.readdword(obj.base + game.offset.hitbox_ptr)
		obj.invulnerable = memory.readbyte(obj.base + game.offset.invulnerable) == 0xFF
		if bit.band(obj.hitbox_ptr, 0xFFFFFF) == 0 then
			obj.num_boxes = 0
		else
			obj.num_boxes = memory.readword(obj.hitbox_ptr)
		end
	end,

	["fatal fury 2"] = function(obj)
		obj.facing_dir = memory.readbyte(obj.base + game.offset.facing_dir) > 0 and 1 or -1
		obj.hitbox_ptr_offset = memory.readword(obj.base + game.offset.hitbox_ptr_offset)
		obj.hitbox_ptr = memory.readdword(obj.base + game.offset.hitbox_ptr) + bit.band(obj.hitbox_ptr_offset, 0x0FFF) * 6
		obj.num_boxes = bit.rshift(bit.band(obj.hitbox_ptr_offset, 0xF000), 12)
		obj.invulnerable = game.offset.invulnerable and memory.readbyte(obj.base + game.offset.invulnerable) == 0xFF
	end,

	["garou"] = function(obj)
		obj.facing_dir = memory.readbyte(obj.base + game.offset.facing_dir)
		obj.facing_dir = (obj.facing_dir > 0 and 1 or -1) * (bit.band(obj.status, 0x80) > 0 and -1 or 1) * game.reverse_facing
		obj.hitbox_ptr = memory.readdword(obj.base + game.offset.hitbox_ptr)
		obj.num_boxes  = bit.rshift(obj.hitbox_ptr, 24)
		obj.char_id = memory.readword(obj.base + game.offset.char_id)
		obj.invulnerable = game.offset.invulnerable and memory.readbyte(obj.base + game.offset.invulnerable) == 0xFF
		if game.offset.no_push then
			obj.no_push = memory.readbyte(obj.base + game.offset.no_push) == 0xFF
			table.insert(obj, define_box["fatal fury"](obj, PUSH_BOX))
		end
	end,
}


local add_object = {
	["king of fighters"] = function(address, projectile)
		local obj = {base = address, projectile = projectile}
		obj.pos_x = get_x(memory.readwordsigned(obj.base + game.offset.x_position))
		obj.pos_y = get_y[game.y_value](memory.readwordsigned(obj.base + game.offset.y_position))
		obj.facing_dir = bit.band(memory.readbyte(obj.base + game.offset.facing_dir), 1)
		obj.status = memory.readbyte(obj.base + game.offset.status)
		for entry in ipairs(game.box_list) do
			obj[entry] = define_box["king of fighters"](obj, entry)
		end
		return obj
	end,

	["fatal fury"] = function(address, projectile)
		local obj = {base = address, projectile = projectile}
		obj.pos_x = get_x(memory.readwordsigned(obj.base + game.offset.x_position))
		obj.pos_z = get_z(memory.readword(obj.base + game.offset.z_position))
		obj.pos_y = get_y[game.y_value](memory.readwordsigned(obj.base + game.offset.y_position)) - obj.pos_z - 1
		obj.status = memory.readbyte(obj.base + game.offset.status)
		--obj.scale = obj.pos_z and 4 / (obj.pos_z/0x80 + 1) or 4 --lazy guess
		modify_object[game.box_engine](obj)

		if game.no_boxes then
			return obj
		end
		for n = obj.num_boxes, 1, -1 do
			table.insert(obj, define_box["fatal fury"](obj, UNDEFINED_BOX, (n-1)*game.box.space))
		end
		return obj
	end,
}


local read_objects = {
	["king of fighters"] = function(objects)
		local offset = 0
		while true do
			local address = memory.readword(game.address.obj_ptr_list + offset)
			if address == 0 or memory.readwordsigned(bit.bor(0x100000, address) + 0x6) < 0 then
				return
			end
			for _, object in ipairs(objects) do
				if address == bit.band(object.base, 0xFFFF) then
					return
				end
			end
			table.insert(objects, add_object["king of fighters"](bit.bor(0x100000, address), true))
			offset = offset + 2
		end
	end,

	["fatal fury 1"] = function(objects)
		local prev_address = game.address.player + game.offset.player_space * (game.number_players-1)
		while true do
			local address = 0x100100 + memory.readword(prev_address + 0x4)
			if address == game.address.player or address == 0x100100 then
				return
			end
			prev_address = address
			local hitbox_ptr = bit.band(memory.readdword(address + game.offset.hitbox_ptr), 0xFFFFFF)
			if hitbox_ptr > 0 and memory.readword(hitbox_ptr + game.box.header) ~= 0x0006 then --back plane obstacle
				table.insert(objects, add_object["fatal fury"](address, true))
			end
		end
	end,

	["fatal fury 2"] = function(objects)
		for p = 1, game.number_players do
			repeat
			local address = memory.readdword(game.address.player + game.offset.player_space * (p-1) + game.offset.obj_ptr)
			globals.inactive_obj.new[address] = memory.readword(memory.readdword(address)) == 0x4E75 --rts instruction
			local inactive_flag = memory.readword(address + game.offset.obj_inactive) == 0
			if address == 0 or inactive_flag or globals.inactive_obj.old[address] then
				globals.inactive_obj.old = copytable(globals.inactive_obj.new)
				break
			end
			globals.inactive_obj.old = copytable(globals.inactive_obj.new)
			for _, object in ipairs(objects) do
				if address == object.base then
					break
				end
			end
			table.insert(objects, add_object["fatal fury"](address, true))
			until true
		end
	end,

	["garou"] = function(objects)
		local offset = 0
		while true do
			local address = memory.readdword(game.address.obj_ptr_list + offset)
			globals.inactive_obj.new[address] = memory.readword(memory.readdword(address)) == 0x4E75 --rts instruction
			local inactive_flag = memory.readword(address + game.offset.obj_inactive) > 0
			if address == 0 or inactive_flag or globals.inactive_obj.old[address] then
				globals.inactive_obj.old = copytable(globals.inactive_obj.new)
				return
			end
			globals.inactive_obj.old = copytable(globals.inactive_obj.new)
			for _, object in ipairs(objects) do
				if address == object.base then
					return
				end
			end
			table.insert(objects, add_object["fatal fury"](address, true))
			offset = offset + 4
		end
	end,
}


local function bios_test(address)
	local ram_value = memory.readword(address)
	for _, test_value in ipairs({0x5555, 0xAAAA, bit.band(0xFFFF, address)}) do
		if ram_value == test_value then
			return true
		end
	end
end


local function update_neogeo_hitboxes()
	if not game or bios_test(game.address.player) then
		return
	end
	globals.game_phase       = memory.readbyte(game.address.game_phase)
	globals.left_screen_edge = memory.readwordsigned(game.address.left_screen_edge)
	globals.top_screen_edge  = memory.readwordsigned(game.address.top_screen_edge)
	if fba then --why is this necessary?
		globals.left_screen_edge = globals.left_screen_edge + 8
	end
	if game.engine == "fatal fury" then
		globals.same_plane = memory.readword(game.address.player + game.offset.z_position) ==
			memory.readword(game.address.player + game.offset.player_space + game.offset.z_position)
	end

	for f = 1, DRAW_DELAY do
		frame_buffer[f] = copytable(frame_buffer[f+1])
	end

	frame_buffer[DRAW_DELAY+1] = {}
	for p = 1, game.number_players do
		frame_buffer[DRAW_DELAY+1][p] = add_object[game.engine](game.address.player + game.offset.player_space * (p-1))
	end
	read_objects[game.obj_engine](frame_buffer[DRAW_DELAY+1])
end


emu.update_func( function()
	globals.register_count = globals.register_count + 1
	if globals.register_count == 1 then
		update_neogeo_hitboxes()
	end
	if globals.last_frame < emu.framecount() then
		globals.register_count = 0
	end
	globals.last_frame = emu.framecount()
end)


--------------------------------------------------------------------------------
-- draw the hitboxes

local function draw_hitbox(obj, entry)
	local hb = obj[entry]
	if not hb or
		(not DRAW_PUSHBOXES and hb.type == PUSH_BOX) or
		(not DRAW_THROWABLE_BOXES and hb.type == THROWABLE_BOX) then
		return
	end

	if DRAW_MINI_AXIS then
		gui.drawline(hb.hval, hb.vval-MINI_AXIS_SIZE, hb.hval, hb.vval+MINI_AXIS_SIZE, outline[hb.type])
		gui.drawline(hb.hval-MINI_AXIS_SIZE, hb.vval, hb.hval+MINI_AXIS_SIZE, hb.vval, outline[hb.type])
		--gui.text(hb.hval, hb.vval, string.format("%02X", hb.id or 0xFF)) --debug
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, fill[hb.type], outline[hb.type])
end


local get_number_entries = {
	["king of fighters"] = function()
		return #game.box_list
	end,

	["fatal fury"] = function()
		local max_entries = 0
		for _, obj in ipairs(frame_buffer[1]) do
			max_entries = math.max(max_entries, #obj)
		end
		return max_entries
	end,
}


local function draw_axis(obj)
	if not (obj.pos_x and obj.pos_y) then
		return
	end
	
	gui.drawline(obj.pos_x, obj.pos_y-AXIS_SIZE, obj.pos_x, obj.pos_y+AXIS_SIZE, AXIS_COLOR)
	gui.drawline(obj.pos_x-AXIS_SIZE, obj.pos_y, obj.pos_x+AXIS_SIZE, obj.pos_y, AXIS_COLOR)
	--gui.text(obj.pos_x, obj.pos_y, string.format("%04X", bit.band(obj.base,0xffff))) --debug
	--gui.text(obj.pos_x, obj.pos_y+8, string.format("%08X", obj.hitbox_ptr)) --debug
	--gui.text(obj.pos_x, obj.pos_y+8, string.format("%02X,%02X", obj.status, obj.num_boxes or 0xFF)) --debug
end


local function render_neogeo_hitboxes()
	gui.clearuncommitted()
	if not game or globals.game_phase == GAME_PHASE_NOT_PLAYING then
		return
	end

	if BLANK_SCREEN then
		gui.box(0, 0, emu.screenwidth(), emu.screenheight(), BLANK_COLOR)
	end

	for entry = 1, get_number_entries[game.engine]() do
		for _, obj in ipairs(frame_buffer[1]) do
			draw_hitbox(obj, entry)
		end
	end

	if DRAW_AXIS then
		for _, obj in ipairs(frame_buffer[1]) do
			draw_axis(obj)
		end
	end
end


gui.register( function()
	render_neogeo_hitboxes()
end)


--------------------------------------------------------------------------------
-- hotkey functions

input.registerhotkey(1, function()
	BLANK_SCREEN = not BLANK_SCREEN
	render_neogeo_hitboxes()
	print((BLANK_SCREEN and "activated" or "deactivated") .. " blank screen mode")
end)


input.registerhotkey(2, function()
	DRAW_AXIS = not DRAW_AXIS
	render_neogeo_hitboxes()
	print((DRAW_AXIS and "showing" or "hiding") .. " object axis")
end)


input.registerhotkey(3, function()
	DRAW_MINI_AXIS = not DRAW_MINI_AXIS
	render_neogeo_hitboxes()
	print((DRAW_MINI_AXIS and "showing" or "hiding") .. " hitbox axis")
end)


input.registerhotkey(4, function()
	DRAW_PUSHBOXES = not DRAW_PUSHBOXES
	render_neogeo_hitboxes()
	print((DRAW_PUSHBOXES and "showing" or "hiding") .. " pushboxes")
end)


input.registerhotkey(5, function()
	DRAW_THROWABLE_BOXES = not DRAW_THROWABLE_BOXES
	render_neogeo_hitboxes()
	print((DRAW_THROWABLE_BOXES and "showing" or "hiding") .. " throwable boxes")
end)


--------------------------------------------------------------------------------
-- initialize on game startup

local function whatversion(game)
	if not game.no_push then
		return nil
	end
	for version_set,base in pairs(game.versions) do
		for _,version in ipairs(version_set) do
			if emu.romname() == version then
				return base
			end
		end
	end
	print("unrecognized version (" .. emu.romname() .. "): cannot draw pushboxes")
	return nil
end

local function whatgame()
	game = nil
	for n, module in ipairs(profile) do
		for m, shortname in ipairs(module.games) do
			if emu.romname() == shortname or emu.parentname() == shortname then
				print("drawing " .. shortname .. " hitboxes")
				game = module
				globals.pushbox_base = whatversion(game)
				for f = 1, DRAW_DELAY + 1 do
					frame_buffer[f] = {}
				end
				globals.game = shortname
				globals.same_plane = nil
				globals.inactive_obj = {old = {}, new = {}}
				return
			end
		end
	end
	print("not prepared for " .. emu.romname() .. " hitboxes")
end


emu.registerstart( function()
	whatgame()
end)