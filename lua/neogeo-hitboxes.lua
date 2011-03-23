print("NeoGeo fighting game hitbox viewer")
print("March 23, 2011")
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

local a,v,p,g = ATTACK_BOX,VULNERABILITY_BOX,PROJ_VULNERABILITY_BOX,GUARD_BOX

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
		games = {"rbff1"},
		address = {
			game_phase   = 0x100B5F,
			obj_ptr_list = 0x100890,
			left_screen_edge = 0x100B20,
			top_screen_edge  = 0x100B28,
		},
		pushbox_base = {
			[0x06C244] = {"rbff1"},
			[0x06C25E] = {"rbff1a"},
		},
	},
	{
		games = {"rbffspec"},
		address = {
			game_phase   = 0x1096FA,
			obj_ptr_list = 0x100C92,
		},
		pushbox_base = {
			[0x072E5E] = {"rbffspeck"},
			[0x072F7A] = {"rbffspec"},
		},
	},
	{
		games = {"rbff2"},
		address = {
			game_phase   = 0x10B1A4,
			obj_ptr_list = 0x100C92,
		},
		pushbox_base = {
			[0x05C898] = {"rbff2k"},
			[0x05C99C] = {"rbff2"},
			[0x05C9BC] = {"rbff2h"},
		},
	},
	{
		games = {"garou"},
		address = {
			game_phase   = 0x107835,
			obj_ptr_list = 0x100C88,
		},
		pushbox_base = {
			[0x0356BE] = {"garoup", "garoubl"},
			[0x0358B0] = {"garou", "garouo"},
		},
	},
}

for game in ipairs(profile) do
	local g = profile[game]
	g.engine = (g.box_types and "king of fighters") or (g.pushbox_base and "fatal fury")
	g.number_players = g.number_players or 2
	if g.engine == "king of fighters" then
		g.y_value        = "direct"
		g.ptr_size       = 2
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
		g.box = {
			radius_read = memory.readbyte,
			offset_read = memory.readbytesigned,
			hval = 0x1, vval = 0x2, hrad = 0x3, vrad = 0x4,
		}
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
	elseif g.engine == "fatal fury" then
		g.y_value        = "absolute"
		g.ptr_size       = 4
		g.ground_level   = 23
		g.front_plane    = 0x18
		g.address.player           = 0x100400
		g.address.left_screen_edge = g.address.left_screen_edge or 0x100E20
		g.address.top_screen_edge  = g.address.top_screen_edge  or 0x100E28
		g.offset = {
			player_space = 0x100,
			char_id      = 0x10,
			x_position   = 0x20,
			z_position   = 0x24,
			y_position   = 0x28,
			facing_dir   = 0x71,
			hitbox_ptr   = 0x7A,
		}
		g.box_list = {
			{type = PUSH_BOX},
			--{offset = 0x0, type = VULNERABILITY_BOX},
			--{offset = 0x5, type = VULNERABILITY_BOX},
			--{offset = 0xA, type = VULNERABILITY_BOX},
			--{offset = 0xF, type = ATTACK_BOX},
		}
	end
end

local game
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
-- prepare the hitboxes

local function get_x(x)
	return x - globals.left_screen_edge
end


local function get_z(z)
	return z - game.front_plane
end


local get_y = {
	["absolute"] = function(y)
		return emu.screenheight() - (y + game.ground_level) + globals.top_screen_edge
	end,

	["direct"] = function(y)
		return y - game.ground_level
	end,
}


local type_check = {
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
}


local get_pushbox_offset = {
	["rbff1"] = function(obj)
		if memory.readdword(obj.base + 0x28) > 0 then --off ground
			if bit.band(memory.readdword(obj.base + 0xC6), 0xFFFF0000) == 0 then
				return 0x110 --???
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
				return 0x150 --???
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
		local d1, d2, d3 = memory.readdword(obj.base + 0xC0), memory.readdword(obj.base + 0xC8), memory.readdword(obj.base + 0x28)
		if (obj.char_id ~= 5 or bit.band(d2, bit.lshift(1, 0xF)) == 0) then
			if bit.band(d1, bit.lshift(1, 0x1)) == 0 and d3 > 0 then
				local char_table_lookup = memory.readdword(globals.pushbox_base - 0x294 + obj.char_id * 4)
				if bit.band(char_table_lookup, d2) == 0 and bit.band(d2, 0xFFFF0000) > 0 then
					return 0x240 --???
				else
					return 0x180 --jumping or juggled
				end
			end
			if (bit.band(d1, bit.lshift(1, 0x1)) ~= 0 or d3 == 0) and bit.band(d1, 0x14000046) ~= 0 then
				return 0x0C0 --crouching or knocked down
			end
		end
		if (obj.char_id == 5 and bit.band(d2, bit.lshift(1, 0xF)) > 0) or 
			(bit.band(d1, bit.lshift(1, 0x1)) > 0 or d3 == 0 and bit.band(d1, 0x14000046) == 0) then
			return 0x000 --standing
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
				return 0x300 --???
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


local define_box = {
	["king of fighters"] = function(obj, entry)
		local box = {
			address = obj.base + game.box_list[entry].offset,
			type = game.box_list[entry].type,
		}
		box.id = memory.readbyte(box.address)

		if type_check[box.type](obj, entry, box) then
			return nil
		end

		box.hrad = game.box.radius_read(box.address + game.box.hrad)
		box.vrad = game.box.radius_read(box.address + game.box.vrad)
		if box.hrad == 0 and box.vrad == 0 then
			return nil
		end
		box.hval = game.box.offset_read(box.address + game.box.hval)
		box.vval = game.box.offset_read(box.address + game.box.vval)

		box.hval   = obj.pos_x + box.hval * (obj.facing_dir > 0 and -1 or 1)
		box.vval   = obj.pos_y + box.vval
		box.left   = box.hval - box.hrad
		box.right  = box.hval + box.hrad - 1
		box.top    = box.vval - box.vrad
		box.bottom = box.vval + box.vrad - 1

		return box
	end,

	["fatal fury"] = function(obj, entry)
		local box = {
			type = game.box_list[entry].type,
		}
		
		if box.type == PUSH_BOX then
			if obj.projectile or not globals.pushbox_base or globals.same_plane == false then
				return nil
			else
				box.address = globals.pushbox_base + get_pushbox_offset[globals.game](obj) + bit.lshift(obj.char_id, 3)
			end
		else
			box.address = obj.hitbox_ptr + game.box_list[entry].offset
		end

		box.hval   = obj.pos_x
		box.vval   = obj.pos_y
		box.top    = box.vval - memory.readbytesigned(box.address + 1) * obj.scale
		box.bottom = box.vval - memory.readbytesigned(box.address + 2) * obj.scale
		box.left   = box.hval - memory.readbytesigned(box.address + 3) * 4 * (obj.facing_dir > 0 and 1 or -1)
		box.right  = box.hval - memory.readbytesigned(box.address + 4) * 4 * (obj.facing_dir > 0 and 1 or -1)
		
		return box
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
			obj[entry] = define_box[game.engine](obj, entry)
		end
		return obj
	end,

	["fatal fury"] = function(address, projectile)
		local obj = {base = address, projectile = projectile}
		obj.pos_x = get_x(memory.readwordsigned(obj.base + game.offset.x_position))
		obj.pos_z = get_z(memory.readword(obj.base + game.offset.z_position))
		obj.pos_y = get_y[game.y_value](memory.readwordsigned(obj.base + game.offset.y_position)) - obj.pos_z - 1
		obj.scale = obj.pos_z and 4 / (obj.pos_z/0x80 + 1) or 4 --lazy guess
		obj.facing_dir = bit.band(memory.readbyte(obj.base + game.offset.facing_dir), 1)
		obj.hitbox_ptr = memory.readdword(obj.base + game.offset.hitbox_ptr)
		obj.char_id    = memory.readword(obj.base + game.offset.char_id)
		for entry in ipairs(game.box_list) do
			obj[entry] = define_box[game.engine](obj, entry)
		end
		return obj
	end,
}


local read_objects = {
	[2] = function(objects)
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
			table.insert(objects, add_object[game.engine](bit.bor(0x100000, address), true))
			offset = offset + 2
		end
	end,

	[4] = function(objects)
		local offset = 0
		while true do
			local address = memory.readdword(game.address.obj_ptr_list + offset)
			if address == 0 or memory.readword(address + 0x60) == 0 then
				return
			end
			for _, object in ipairs(objects) do
				if address == object.base then
					return
				end
			end
			table.insert(objects, add_object[game.engine](address, true))
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
	globals.left_screen_edge = memory.readword(game.address.left_screen_edge)
	globals.top_screen_edge  = memory.readword(game.address.top_screen_edge)
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
	read_objects[game.ptr_size](frame_buffer[DRAW_DELAY+1])
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
		--gui.text(hb.hval, hb.vval, string.format("%02X", hb.id)) --debug
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, fill[hb.type], outline[hb.type])
end


local function draw_axis(obj)
	if not (obj.pos_x and obj.pos_y) then
		return
	end
	
	gui.drawline(obj.pos_x, obj.pos_y-AXIS_SIZE, obj.pos_x, obj.pos_y+AXIS_SIZE, AXIS_COLOR)
	gui.drawline(obj.pos_x-AXIS_SIZE, obj.pos_y, obj.pos_x+AXIS_SIZE, obj.pos_y, AXIS_COLOR)
	--gui.text(obj.pos_x, obj.pos_y, string.format("%06X", obj.base)) --debug
	--gui.text(obj.pos_x, obj.pos_y+8, string.format("%04X,%04X", obj.pos_x, obj.pos_y)) --debug
	--gui.text(obj.pos_x, obj.pos_y+8, string.format("%08X", obj.hitbox_ptr)) --debug
	--gui.text(obj.pos_x, obj.pos_y+8, string.format("%02X", obj.char_id or obj.status)) --debug
end


local function render_neogeo_hitboxes()
	gui.clearuncommitted()
	if not game or globals.game_phase == GAME_PHASE_NOT_PLAYING then
		return
	end

	if BLANK_SCREEN then
		gui.box(0, 0, emu.screenwidth(), emu.screenheight(), BLANK_COLOR)
	end

	for entry in ipairs(game.box_list) do
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
	if not game.pushbox_base then
		return nil
	end
	for base,version_set in pairs(game.pushbox_base) do
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
				for f = 1, DRAW_DELAY + 1 do
					frame_buffer[f] = {}
				end
				globals.game = shortname
				globals.pushbox_base = whatversion(game)
				globals.same_plane = nil
				return
			end
		end
	end
	print("not prepared for " .. emu.romname() .. " hitboxes")
end


emu.registerstart( function()
	whatgame()
end)