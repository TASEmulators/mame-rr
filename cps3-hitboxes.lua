print("CPS-3 fighting game hitbox viewer")
print("October 20, 2011")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis")
print("Lua hotkey 4: toggle pushboxes")
print("Lua hotkey 5: toggle throwable boxes")

local boxes = {
	      ["vulnerability"] = {color = 0x7777FF, fill = 0x40, outline = 0xFF},
	 ["ext. vulnerability"] = {color = 0x7777FF, fill = 0x40, outline = 0xFF}, --extended limbs during attacks
	             ["attack"] = {color = 0xFF0000, fill = 0x40, outline = 0xFF},
	["proj. vulnerability"] = {color = 0x00FFFF, fill = 0x40, outline = 0xFF},
	       ["proj. attack"] = {color = 0xFF66FF, fill = 0x40, outline = 0xFF},
	               ["push"] = {color = 0x00FF00, fill = 0x20, outline = 0xFF},
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

local any_true = function(condition)
	for n = 1, #condition do
		if condition[n] == true then return true end
	end
end


local profile = {
	{	games = {"sfiii"},
		address = {
			player          = 0x0200D18C,
			object          = 0x0201DE04,
			object_index    = 0x0205DF0A,
			screen_center_x = 0x0201BF0C,
			screen_center_y = 0x0201BF10,
		},
		offset = {
			player_space = 0x3D8,
		},
		ptr = {
			valid_object = 0x258,
			{offset = 0x270, type = "push"},
			{offset = 0x268, type = "throwable"},
			{offset = 0x258, type = "vulnerability", number = 5},
			{offset = 0x260, type = "attack", initial = 2, number = 3},
			{offset = 0x260, type = "throw"},
		},
		match_active = function() 
			local addr = 0x02012C54
			return memory.readdword(addr) > 0x00010000 and memory.readword(addr) ~= 0x0009
		end,
	},
	{	games = {"sfiii2"},
		address = {
			player          = 0x0200E504,
			object          = 0x02024210,
			object_index    = 0x02064316,
			screen_center_x = 0x0202225C,
			screen_center_y = 0x02022260,
		},
		offset = {
			player_space = 0x40C,
		},
		ptr = {
			valid_object = 0x260,
			{offset = 0x294, type = "push"},
			{offset = 0x280, type = "throwable"},
			{offset = 0x260, type = "vulnerability", number = 4},
			{offset = 0x268, type = "ext. vulnerability", number = 2},
			{offset = 0x288, type = "attack", number = 2},
			{offset = 0x278, type = "throw"},
		},
		match_active = function() 
			local addr = 0x02014298
			return memory.readdword(addr) > 0x00010000 and memory.readword(addr) ~= 0x0009
		end,
	},
	{	games = {"sfiii3"},
		address = {
			player          = 0x02068C6C,
			object          = 0x02028990,
			object_index    = 0x02068A96,
			screen_center_x = 0x02026CB0,
			screen_center_y = 0x02026CB4,
		},
		offset = {
			player_space = 0x498,
		},
		ptr = {
			valid_object = 0x2A0,
			{offset = 0x2D4, type = "push"},
			{offset = 0x2C0, type = "throwable"},
			{offset = 0x2A0, type = "vulnerability", number = 4},
			{offset = 0x2A8, type = "ext. vulnerability", number = 4},
			{offset = 0x2C8, type = "attack", number = 4},
			{offset = 0x2B8, type = "throw"},
		},
		match_active = function() 
			local addr = 0x020154A6
			return memory.readdword(addr) > 0x00010000 and memory.readword(addr) ~= 0x0009
		end,
	},
}

--------------------------------------------------------------------------------
-- post-process modules

for game in ipairs(profile) do
	local g = profile[game]
	for _, box in ipairs(g.ptr) do
		box.initial = box.initial or 1
		box.number  = box.number  or 1
	end
end

for _,box in pairs(boxes) do
	box.fill    = bit.lshift(box.color, 8) + (globals.no_alpha and 0x00 or box.fill)
	box.outline = bit.lshift(box.color, 8) + (globals.no_alpha and 0xFF or box.outline)
end

local game
local frame_buffer = {}
local DRAW_DELAY        = 1
local NUMBER_OF_PLAYERS = 2
local GROUND_OFFSET     = -23
local MAX_OBJECTS       = 30

--------------------------------------------------------------------------------
-- prepare the hitboxes

local projectile_type = {
	            ["attack"] = "proj. attack",
	     ["vulnerability"] = "proj. vulnerability",
	["ext. vulnerability"] = "proj. vulnerability",
	              ["push"] = "push",
	             ["throw"] = "throw",
	         ["throwable"] = "throwable",
}

local define_box = function(obj, ptr, type)
	if obj.friends > 1 then --Yang SA3
		if type ~= "attack" then
			return
		end
	elseif obj.projectile then
		type = projectile_type[type]
	end

	local box = {
		left   = -memory.readwordsigned(ptr + 0x0),
		right  = -memory.readwordsigned(ptr + 0x2),
		bottom = -memory.readwordsigned(ptr + 0x4),
		top    = -memory.readwordsigned(ptr + 0x6),
		type   = type,
	}

	if box.left == 0 and box.right == 0 and box.top == 0 and box.bottom == 0 then
		return
	elseif obj.facing_dir == 0 then
		box.left  = -box.left
		box.right = -box.right
	end

	box.left   = box.left   + obj.pos_x
	box.right  = box.right  + box.left
	box.bottom = box.bottom + obj.pos_y
	box.top    = box.top    + box.bottom
	box.hval   = (box.left + box.right)/2
	box.vval   = (box.bottom + box.top)/2

	table.insert(obj, box)
end


local update_game_object = function(f, obj)
	if memory.readdword(obj.base + game.ptr.valid_object) == 0 then --invalid objects
		return
	end

	obj.friends      = memory.readbyte(obj.base + 0x1)
	obj.facing_dir   = memory.readbyte(obj.base + 0xA)
	obj.opponent_dir = memory.readbyte(obj.base + 0xB)
	obj.anim_frame   = memory.readword(obj.base + 0x21A)
	obj.pos_x = memory.readwordsigned(obj.base + 0x64)
	obj.pos_y = memory.readwordsigned(obj.base + 0x68)
	obj.pos_x =  obj.pos_x - f.screen_center_x + emu.screenwidth()/2
	obj.pos_y = -obj.pos_y + f.screen_center_y + emu.screenheight() + GROUND_OFFSET

	for _, box in ipairs(game.ptr) do
		for i = box.initial, box.number do
			define_box(obj, memory.readdword(obj.base + box.offset) + (i-1)*8, box.type)
		end
	end

	table.insert(f, obj)
end


local read_misc_objects = function(f)
	-- This function reads all game objects other than the two player characters.
	-- This includes all projectiles and even Yang's Seiei-Enbu shadows.

	-- The game uses the same structure all over the place and groups them
	-- into lists with each element containing an index to the next element
	-- in that list. An index of -1 signals the end of the list.

	-- I believe there are at least 7 lists (0-6) but it seems everything we need
	-- (and lots we don't) is in list 3.
	local list = 3
	local obj_index = memory.readwordsigned(game.address.object_index + (list * 2))
		
	local obj_slot = 1
	while obj_slot <= MAX_OBJECTS and obj_index ~= -1 do
		local obj = {base = game.address.object + bit.lshift(obj_index, 11), projectile = obj_slot}
		update_game_object(f, obj)

		-- Get the index to the next object in this list.
		obj_index = memory.readwordsigned(obj.base + 0x1C)
		obj_slot = obj_slot + 1
	end
end


local update_hitboxes = function()
	for f = 1, DRAW_DELAY do
		frame_buffer[f] = copytable(frame_buffer[f+1])
	end

	frame_buffer[DRAW_DELAY+1] = {match_active = game and game.match_active()}
	local f = frame_buffer[DRAW_DELAY+1]

	if not f.match_active then
		return
	end

	f.screen_center_x = memory.readwordsigned(game.address.screen_center_x)
	f.screen_center_y = memory.readwordsigned(game.address.screen_center_y)

	for p = 1, NUMBER_OF_PLAYERS do
		local player = {base = game.address.player + (p-1)*game.offset.player_space}
		update_game_object(f, player)
	end
	read_misc_objects(f)

	f.max_boxes = 0
	for _, obj in ipairs(f) do
		f.max_boxes = math.max(f.max_boxes, #obj)
	end
end


emu.registerafter(function()
	update_hitboxes()
end)

--------------------------------------------------------------------------------
-- draw the hitboxes

local draw_hitbox = function(hb)
	if not hb or any_true({
		not globals.draw_pushboxes and hb.type == "push",
		not globals.draw_throwable_boxes and hb.type == "throwable",
	}) then return
	end

	if globals.draw_mini_axis then
		gui.drawline(hb.hval, hb.vval-globals.mini_axis_size, hb.hval, hb.vval+globals.mini_axis_size, boxes[hb.type].outline)
		gui.drawline(hb.hval-globals.mini_axis_size, hb.vval, hb.hval+globals.mini_axis_size, hb.vval, boxes[hb.type].outline)
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, boxes[hb.type].fill, boxes[hb.type].outline)
end


local draw_axis = function(obj)
	gui.drawline(obj.pos_x, obj.pos_y-globals.axis_size, obj.pos_x, obj.pos_y+globals.axis_size, globals.axis_color)
	gui.drawline(obj.pos_x-globals.axis_size, obj.pos_y, obj.pos_x+globals.axis_size, obj.pos_y, globals.axis_color)
	--gui.text(obj.pos_x+4, obj.pos_y-0x08, string.format("%08x", obj.base))
	--gui.text(obj.pos_x+4, obj.pos_y+0x00, string.format("%08x", memory.readdword(obj.base)))
	--gui.text(obj.pos_x+4, obj.pos_y+0, obj.projectile or "")
end


local render_hitboxes = function()
	gui.clearuncommitted()

	local f = frame_buffer[1]
	if not f.match_active then
		return
	end

	if globals.blank_screen then
		gui.box(0, 0, emu.screenwidth(), emu.screenheight(), globals.blank_color)
	end

	for entry = 1, f.max_boxes do
		for _, obj in ipairs(f) do
			draw_hitbox(obj[entry])
		end
	end

	if globals.draw_axis then
		for _, obj in ipairs(f) do
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

local initialize_fb = function()
	for f = 1, DRAW_DELAY+1 do
		frame_buffer[f] = {}
	end
end


local whatgame = function()
	print()
	game = nil
	initialize_fb()
	for _, module in ipairs(profile) do
		for _, shortname in ipairs(module.games) do
			if emu.romname() == shortname or emu.parentname() == shortname then
				print("drawing hitboxes for: " .. emu.gamename())
				game = module
				return
			end
		end
	end
	print("not prepared for: " .. emu.gamename())
end


savestate.registerload(function()
	initialize_fb()
end)


emu.registerstart(function()
	whatgame()
end)