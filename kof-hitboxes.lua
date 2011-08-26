print("King of Fighters hitbox viewer")
print("August 25, 2011")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis")
print("Lua hotkey 4: toggle pushboxes")
print("Lua hotkey 5: toggle throwable boxes")

local boxes = {
	      ["vulnerability"] = {color = 0x7777FF, fill = 0x40, outline = 0xFF},
	             ["attack"] = {color = 0xFF0000, fill = 0x40, outline = 0xFF},
	["proj. vulnerability"] = {color = 0x00FFFF, fill = 0x40, outline = 0xFF},
	       ["proj. attack"] = {color = 0xFF66FF, fill = 0x40, outline = 0xFF},
	               ["push"] = {color = 0x00FF00, fill = 0x20, outline = 0xFF},
	              ["guard"] = {color = 0xCCCCFF, fill = 0x40, outline = 0xFF},
	              ["throw"] = {color = 0xFFFF00, fill = 0x40, outline = 0xFF},
	          ["throwable"] = {color = 0xF0F0F0, fill = 0x20, outline = 0xFF},
}

local globals = {
	axis_color           = 0xFFFFFFFF,
	blank_color          = 0xFFFFFFFF,
	axis_size            = 12,
	mini_axis_size       = 2,
	blank_screen         = false,
	draw_axis            = true,
	draw_mini_axis       = false,
	draw_pushboxes       = true,
	draw_throwable_boxes = false,
	no_alpha             = false, --fill = 0x00, outline = 0xFF for all box types
}

--------------------------------------------------------------------------------
-- game-specific modules

local a,v,p,g,t = "attack","vulnerability","proj. vulnerability","guard","throw"

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
}


--------------------------------------------------------------------------------
-- post-process modules

for game in ipairs(profile) do
	local g = profile[game]
	g.number_players = 2
	g.ground_level   = 16
	g.address.player = 0x108100
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
		{offset = 0xA4, type = "push"},
		{offset = 0x9F, active_bit = 3, type = "undefined"},
		{offset = 0x9A, active_bit = 2, type = "undefined"},
		{offset = 0x95, active_bit = 1, type = "undefined"},
		{offset = 0x90, active_bit = 0, type = "undefined"},
	}
	for _, box in ipairs(g.box_list) do
		box.active = box.active or box.active_bit and bit.lshift(1, box.active_bit)
	end
	if g.throw_boxes then
		table.insert(g.box_list, {offset = 0x18D, active = 0x7E, type = "throwable"})
		table.insert(g.box_list, {offset = 0x188, id = 0x192, type = "throw"})
	end
end

for _,box in pairs(boxes) do
	box.fill    = bit.lshift(box.color, 8) + (globals.no_alpha and 0x00 or box.fill)
	box.outline = bit.lshift(box.color, 8) + (globals.no_alpha and 0xFF or box.outline)
end
boxes["undefined"] = {}

local game, frame_buffer
emu.update_func = fba and emu.registerafter or emu.registerbefore


--------------------------------------------------------------------------------
-- prepare the hitboxes

local type_check = {
	["undefined"] = function(obj, box_entry, box)
		if bit.band(obj.status, box_entry.active) == 0 then
			return true
		end
		box.type = game.box_types[box.id] or "undefined"
		if box.type == "attack" then
			if box_entry.active_bit == 1 then
				return true --"ghost boxes"?
			elseif obj.projectile then
				box.type = "proj. attack"
			end
		end
	end,

	["push"] = function(obj, box_entry, box)
		if box.id == 0xFF or obj.projectile then
			return true
		end
	end,

	["throw"] = function(obj, box_entry, box)
		box.id = memory.readbyte(obj.base + box_entry.id)
		if box.id == 0x00 then
			return true
		else
			memory.writebyte(obj.base + box_entry.id, 0) --bad
		end
	end,

	["throwable"] = function(obj, box_entry, box)
		box.id = memory.readbyte(obj.base + box_entry.active)
		if box.id == 0x01 then
			return true
		end
	end,
}


local define_box = function(obj, box_entry)
	local box = {
		address = obj.base + box_entry.offset,
		type = box_entry.type,
	}
	box.id = memory.readbyte(box.address)

	if type_check[box.type](obj, box_entry, box) then
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
end


local modify_object = function(obj)
	obj.facing_dir = bit.band(memory.readbyte(obj.base + game.offset.facing_dir), 0x01) > 0 and 1 or -1
	obj.hitbox_ptr = memory.readdword(obj.base + game.offset.hitbox_ptr)
	obj.invulnerable = memory.readbyte(obj.base + game.offset.invulnerable) == 0xFF
	if bit.band(obj.hitbox_ptr, 0xFFFFFF) == 0 then
		obj.num_boxes = 0
	else
		obj.num_boxes = memory.readword(obj.hitbox_ptr)
	end
end


local update_object = function(obj)
	obj.pos_x = memory.readwordsigned(obj.base + game.offset.x_position) - globals.left_screen_edge
	obj.pos_y = memory.readwordsigned(obj.base + game.offset.y_position) - game.ground_level
	obj.facing_dir = bit.band(memory.readbyte(obj.base + game.offset.facing_dir), 1)
	obj.status = memory.readbyte(obj.base + game.offset.status)
	for _, box_entry in ipairs(game.box_list) do
		table.insert(obj, define_box(obj, box_entry))
	end
	return obj
end


local read_projectiles = function(object_list)
	local offset = 0
	while true do
		local obj = {base = memory.readword(game.address.obj_ptr_list + offset)}
		obj.projectile = true
		if obj.base == 0 or memory.readwordsigned(bit.bor(0x100000, obj.base) + 0x6) < 0 then
			return
		end
		for _, old_obj in ipairs(object_list) do
			if obj.base == bit.band(old_obj.base, 0xFFFF) then
				return
			end
		end
		obj.base = bit.bor(0x100000, obj.base)
		table.insert(object_list, update_object(obj))
		offset = offset + 2
	end
end


local function bios_test(address)
	local ram_value = memory.readword(address)
	for _, test_value in ipairs({0x5555, 0xAAAA, bit.band(0xFFFF, address)}) do
		if ram_value == test_value then
			return true
		end
	end
end


local function update_hitboxes()
	if not game or bios_test(game.address.player) then
		return
	end
	globals.game_phase       = memory.readbyte(game.address.game_phase) > 0
	globals.left_screen_edge = memory.readwordsigned(game.address.left_screen_edge) + globals.margin
	globals.top_screen_edge  = memory.readwordsigned(game.address.top_screen_edge)

	frame_buffer = {}

	for p = 1, game.number_players do
		local player = {base = game.address.player + game.offset.player_space * (p-1)}
		table.insert(frame_buffer, update_object(player))
	end
	read_projectiles(frame_buffer)
end


emu.update_func( function()
	globals.register_count = (globals.register_count or 0) + 1
	globals.last_frame = globals.last_frame or emu.framecount()
	if globals.register_count == 1 then
		update_hitboxes()
	end
	if globals.last_frame < emu.framecount() then
		globals.register_count = 0
	end
	globals.last_frame = emu.framecount()
end)


--------------------------------------------------------------------------------
-- draw the hitboxes

local function draw_hitbox(hb)
	if not hb or
		(not globals.draw_pushboxes and hb.type == "push") or
		(not globals.draw_throwable_boxes and hb.type == "throwable") then
		return
	end

	if globals.draw_mini_axis then
		gui.drawline(hb.hval, hb.vval-globals.mini_axis_size, hb.hval, hb.vval+globals.mini_axis_size, boxes[hb.type].outline)
		gui.drawline(hb.hval-globals.mini_axis_size, hb.vval, hb.hval+globals.mini_axis_size, hb.vval, boxes[hb.type].outline)
		--gui.text(hb.hval, hb.vval, string.format("%02X", hb.id or 0xFF)) --debug
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, boxes[hb.type].fill, boxes[hb.type].outline)
end


local function draw_axis(obj)
	gui.drawline(obj.pos_x, obj.pos_y-globals.axis_size, obj.pos_x, obj.pos_y+globals.axis_size, globals.axis_color)
	gui.drawline(obj.pos_x-globals.axis_size, obj.pos_y, obj.pos_x+globals.axis_size, obj.pos_y, globals.axis_color)
	--gui.text(obj.pos_x, obj.pos_y, string.format("%06X", obj.base)) --debug
	--gui.text(obj.pos_x, obj.pos_y-0x08, string.format("%08X", obj.hitbox_ptr)) --debug
	--gui.text(obj.pos_x, obj.pos_y-0x10, string.format("%02X,%02X", obj.status, obj.num_boxes or 0xFF)) --debug
end


local function render_hitboxes()
	gui.clearuncommitted()
	if not game or not globals.game_phase then
		return
	end

	if globals.blank_screen then
		gui.box(0, 0, emu.screenwidth(), emu.screenheight(), globals.blank_color)
	end

	for entry = 1, #game.box_list do
		for _, obj in ipairs(frame_buffer) do
			draw_hitbox(obj[entry])
		end
	end

	if globals.draw_axis then
		for _, obj in ipairs(frame_buffer) do
			draw_axis(obj)
		end
	end
end


gui.register(function()
	render_hitboxes()
end)


--------------------------------------------------------------------------------
-- hotkey functions

input.registerhotkey(1, function()
	globals.blank_screen = not globals.blank_screen
	render_hitboxes()
	emu.message((globals.blank_screen and "activated" or "deactivated") .. " blank screen mode")
end)


input.registerhotkey(2, function()
	globals.draw_axis = not globals.draw_axis
	render_hitboxes()
	emu.message((globals.draw_axis and "showing" or "hiding") .. " object axis")
end)


input.registerhotkey(3, function()
	globals.draw_mini_axis = not globals.draw_mini_axis
	render_hitboxes()
	emu.message((globals.draw_mini_axis and "showing" or "hiding") .. " hitbox axis")
end)


input.registerhotkey(4, function()
	globals.draw_pushboxes = not globals.draw_pushboxes
	render_hitboxes()
	emu.message((globals.draw_pushboxes and "showing" or "hiding") .. " pushboxes")
end)


input.registerhotkey(5, function()
	globals.draw_throwable_boxes = not globals.draw_throwable_boxes
	render_hitboxes()
	emu.message((globals.draw_throwable_boxes and "showing" or "hiding") .. " throwable boxes")
end)


--------------------------------------------------------------------------------
-- initialize on game startup

local function initialize_fb()
	frame_buffer = {}
end


local function whatgame()
	print()
	game = nil
	for _, module in ipairs(profile) do
		for _, shortname in ipairs(module.games) do
			if emu.romname() == shortname or emu.parentname() == shortname then
				print("drawing " .. emu.romname() .. " hitboxes")
				game = module
				initialize_fb()
				globals.game = shortname
				globals.margin = (320 - emu.screenwidth()) / 2 --fba removes the side margins for some games
				if game.pushbox_data then
					if emu.parentname() == "neogeo" or emu.parentname() == "0" then
						globals.pushbox_base = game.pushbox_data.base
					elseif game.pushbox_data[emu.romname()] then
						globals.pushbox_base = game.pushbox_data.base + game.pushbox_data[emu.romname()]
					else
						globals.pushbox_base = nil
						print("Unrecognized version (" .. emu.romname() .. "): cannot draw pushboxes")
					end
				end
				return
			end
		end
	end
	print("not prepared for " .. emu.romname() .. " hitboxes")
end


savestate.registerload(function()
	initialize_fb()
end)


emu.registerstart(function()
	whatgame()
end)