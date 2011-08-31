print("King of Fighters hitbox viewer")
print("August 31, 2011")
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
	         ["axis throw"] = {color = 0xFFAA00, fill = 0x40, outline = 0xFF}, --kof94, kof95
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

bit.btst = function(bit_number, value)
	return bit.band(bit.lshift(1, bit_number), value)
end

local function any_true(condition)
	for n in ipairs(condition) do
		if condition[n] == true then return true end
	end
end

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
		unthrowable = function(obj) return any_true({
			bit.btst(5, memory.readbyte(obj.base + 0xE3)) > 0,
			memory.readdword(obj.base + 0xAA) > 0,
			bit.btst(3, memory.readbyte(obj.base + 0x7B)) > 0,
		}) end,
		special_throws = {
			{subroutine = 0x00DD40, range_offset = 0x50, lsr = 1}, --heidern 63214+C
			{subroutine = 0x00F516, range_offset = 0x50, lsr = 1}, --ralf 41236+D
			{subroutine = 0x010704, range_offset = 0x50, lsr = 1}, --clark 41236+D
			{subroutine = 0x010F46, range_offset = 0x6C, lsr = 1}, --clark DM
			{subroutine = 0x0188A0, range_offset = 0x4E, lsr = 0}, --daimon 6123+D
			{subroutine = 0x01955E, range_offset = 0x4E, lsr = 0}, --daimon 632146+C
			{subroutine = 0x019EB2, range_offset = 0x6A, lsr = 0}, --daimon DM
			{subroutine = 0x029142, range_offset = 0x40}, --takuma 63214+D 
			{subroutine = 0x02ACF0, range_offset = 0x40}, --yuri 623+P
			table_base = 0x06C0F0,
		},
		breakpoints = {
			{base = 0x00A3DA, cmd = "maincpu.pb@(a4+196)=maincpu.pb@(a0+2)"}, --ground throw
			{base = 0x00A410, cmd = "maincpu.pb@(a4+196)=maincpu.pb@(a0+2)"}, --air throw
			{base = 0x00786C, cmd = "maincpu.pd@(a4+192)=a0"}, --special move
		},
	},
	{
		games = {"kof95"},
		address = {game_phase = 0x10B088},
		box_types = {a,g,v,v,a},
		unthrowable = function(obj) return any_true({
			bit.btst(5, memory.readbyte(obj.base + 0xE3)) > 0,
			memory.readdword(obj.base + 0xAA) > 0,
			bit.btst(3, memory.readbyte(obj.base + 0x7D)) > 0,
		}) end,
		special_throws = {
			{subroutine = 0x00E834, range_offset = 0x50, lsr = 1}, --heidern 63214+C
			{subroutine = 0x01014A, range_offset = 0x50, lsr = 1}, --ralf 41236+D
			{subroutine = 0x0115AE, range_offset = 0x50, lsr = 1}, --clark 41236+D
			{subroutine = 0x0120FC, range_offset = 0x6C, lsr = 1}, --clark DM
			{subroutine = 0x01AE82, range_offset = 0x4E, lsr = 0}, --daimon 6123+D
			{subroutine = 0x01BB68, range_offset = 0x4E, lsr = 0}, --daimon 632146+C
			{subroutine = 0x01CBEA, range_offset = 0x6A, lsr = 0}, --daimon DM
			{subroutine = 0x01195A, range_offset = 0x1A, delay = 0x7C}, --clark 41236+C 
			{subroutine = 0x02EB72, range_offset = 0x12}, --takuma 63214+D 
			{subroutine = 0x0304F2, range_offset = 0x40}, --yuri 41236+A
			{subroutine = 0x030872, range_offset = 0x12}, --yuri 41236+C 
			table_base = 0x079FC0,
		},
		breakpoints = {
			{base = 0x00A62A, cmd = "maincpu.pb@(a4+196)=maincpu.pb@(a0+2)"}, --ground throw
			{base = 0x00A660, cmd = "maincpu.pb@(a4+196)=maincpu.pb@(a0+2)"}, --air throw
			{base = 0x00758C, cmd = "maincpu.pd@(a4+192)=a0"}, --cmd throw (opponent-independent range)
			{base = 0x0077DC, cmd = "maincpu.pd@(a4+192)=a0"}, --cmd throw (opponent-dependent range)
		},
	},
	{
		games = {"kof96"},
		address = {game_phase = 0x10B08E},
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
		{offset = 0x9F, type = "undefined", active_bit = 3},
		{offset = 0x9A, type = "undefined", active_bit = 2},
		{offset = 0x95, type = "undefined", active_bit = 1},
		{offset = 0x90, type = "undefined", active_bit = 0},
	}
	g.throw_type = g.special_throws and "old" or "new"
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
		if bit.btst(box_entry.active_bit, obj.status) == 0 then
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
		if box.id == 0 then
			return true
		end
		memory.writebyte(obj.base + box_entry.id, 0) --bad
	end,

	["throwable"] = function(obj, box_entry, box)
		return any_true({
			bit.btst(5, memory.readbyte(obj.base + 0xE3)) > 0,
			memory.readbyte(obj.base + 0x1D4) > 0,
			memory.readbytesigned(obj.base + 0x18D) < 0,
			bit.band(3, memory.readbyte(obj.base + 0x7E)) == 1,
		})
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

	box.hval   = obj.pos_x + box.hval * obj.facing_dir
	box.vval   = obj.pos_y + box.vval
	box.left   = box.hval - box.hrad
	box.right  = box.hval + box.hrad - 1
	box.top    = box.vval - box.vrad
	box.bottom = box.vval + box.vrad - 1

	return box
end


local update_object = function(obj)
	obj.pos_x = memory.readwordsigned(obj.base + game.offset.x_position) - globals.left_screen_edge
	obj.pos_y = memory.readwordsigned(obj.base + game.offset.y_position) - game.ground_level
	obj.facing_dir = bit.btst(0, memory.readbyte(obj.base + game.offset.facing_dir)) > 0 and -1 or 1
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


local function old_throwable_box(obj)
	obj.air = bit.btst(0, memory.readbyte(obj.base + 0xE1))
	obj.throw_ptr = (memory.readdword(obj.base + 0x1A0))
	obj.opp_id = memory.readword(memory.readdword(obj.base + 0xB6) + 0x182)
	if game.unthrowable(obj) then
		return
	end
	local range = {
		h = memory.readword(obj.throw_ptr + 0x0E + obj.air * 2),
		v = memory.readword(obj.throw_ptr + 0x12),
	}
	local box = {
		type   = "throwable",
		hval   = obj.pos_x,
		left   = obj.pos_x - range.h,
		right  = obj.pos_x + range.h,
		bottom = obj.pos_y + range.v * obj.air,
	}
	box.top = box.bottom - range.v * 2
	box.vval = (box.top + box.bottom)/2
	return box
end


local function old_throw_box(obj)
	local box
	local throw_id = memory.readbyte(obj.base + 0x196)
	if throw_id ~= 0 then --normal throws
		memory.writebyte(obj.base + 0x196, 0)
		box = {type = "throw", range = throw_id}
		if obj.air > 0 then --air throw
			box.top = obj.pos_y
		end
	else
		throw_id = memory.readdword(obj.base + 0x192)
		if throw_id == 0 then
			return
		end
		memory.writedword(obj.base + 0x192, 0)
		for _, type in ipairs(game.special_throws) do
			if throw_id == type.subroutine then
				box = {type = "axis throw", range = memory.readword(type.subroutine + type.range_offset)}
				if type.lsr then --opponent-dependent range
					box.range = box.range + bit.rshift(memory.readword(game.special_throws.table_base + obj.opp_id), type.lsr)
				elseif type.delay and memory.readbytesigned(obj.base + type.delay) > 0 then
					return
				end
				break
			end
		end
	end
	if box then
		box.left   = obj.pos_x
		box.right  = obj.pos_x - box.range * obj.facing_dir
		box.top    = box.top or obj.pos_y - memory.readword(obj.throw_ptr + 0x12) * 2
		box.bottom = obj.pos_y
		box.hval   = (box.left + box.right)/2
		box.vval   = (box.top + box.bottom)/2
		return box
	end
end


local insert_throws = {
	["old"] = function(obj)
		table.insert(obj, old_throwable_box(obj) or nil)
		table.insert(obj, old_throw_box(obj) or nil)
	end,

	["new"] = function(obj)
		table.insert(obj, define_box(obj, {offset = 0x18D, type = "throwable"}))
		table.insert(obj, define_box(obj, {offset = 0x188, id = 0x192, type = "throw"}))
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


local function update_hitboxes()
	frame_buffer = {match_active = game and not bios_test(game.address.player) and memory.readbyte(game.address.game_phase) > 0}

	if not frame_buffer.match_active then
		return
	end

	globals.left_screen_edge = memory.readwordsigned(game.address.left_screen_edge) + globals.margin
	globals.top_screen_edge  = memory.readwordsigned(game.address.top_screen_edge)

	for p = 1, game.number_players do
		local player = {base = game.address.player + game.offset.player_space * (p-1)}
		table.insert(frame_buffer, update_object(player))
	end
	read_projectiles(frame_buffer)

	for _, prev_frame in ipairs(frame_buffer or {}) do
		if prev_frame.projectile then
			break
		end
		insert_throws[game.throw_type](prev_frame)
	end
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
	if not hb or any_true({
		not globals.draw_pushboxes and hb.type == "push",
		not globals.draw_throwable_boxes and hb.type == "throwable",
		}) then return
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
	if not frame_buffer.match_active then
		return
	end

	if globals.blank_screen then
		gui.box(0, 0, emu.screenwidth(), emu.screenheight(), globals.blank_color)
	end

	for entry = 1, #game.box_list + 2 do --add 2 for the throwboxes
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

local function whatgame()
	print()
	game = nil
	frame_buffer = {}
	for _, module in ipairs(profile) do
		for _, shortname in ipairs(module.games) do
			if emu.romname() == shortname or emu.parentname() == shortname then
				print("drawing " .. emu.romname() .. " hitboxes")
				game = module
				globals.game = shortname
				globals.margin = (320 - emu.screenwidth()) / 2 --fba removes the side margins for some games
				if game.breakpoints then
					if not mame then
						print("(MAME-rr can show more accurate throwboxes and pushboxes with this script.)")
						return
					end
					print("Copy this line into the MAME-rr debugger to show throwboxes:") print()
					local bpstring = ""
					for _, bp in ipairs(game.breakpoints) do
						bpstring = bpstring .. string.format("bp %06X, 1, {%s; g}; ", bp.base, bp.cmd)
					end
					print(bpstring:sub(1, -3))
				end
				return
			end
		end
	end
	print("not prepared for " .. emu.romname() .. " hitboxes")
end


savestate.registerload(function()
	frame_buffer = {}
end)


emu.registerstart(function()
	whatgame()
end)