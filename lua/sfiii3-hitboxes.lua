local SCREEN_WIDTH          = 384
local SCREEN_HEIGHT         = 224
local GROUND_OFFSET         = 40
local MAX_GAME_OBJECTS      = 30
local AXIS_COLOUR           = 0xFFFFFFFF
local AXIS_SIZE             = 25
local HITBOX_PASSIVE        = 0
local HITBOX_ACTIVE         = 1
local HITBOX_PASSIVE_COLOUR = 0x00008080
local HITBOX_ACTIVE_COLOUR  = 0x00FF00C0
local GAME_PHASE_PLAYING    = 2

local address = {
	player1         = 0x02068C6C,
	player2         = 0x02069104,
	screen_center_x = 0x02026CB0,
	game_phase      = 0x020154A6
}
local globals = {
	game_phase      = 0,
	screen_center_x = 0,
	num_misc_objs   = 0
}
local player1 = {}
local player2 = {}
local misc_objs = {}


function update_globals()
	globals.screen_center_x = memory.readword(address.screen_center_x)
	globals.game_phase      = memory.readword(address.game_phase)
end


function hitbox_load(obj, i, type, facing_dir, offset_x, offset_y, addr)
	local left   = memory.readwordsigned(addr)
	local right  = memory.readwordsigned(addr + 2)
	local bottom = memory.readwordsigned(addr + 4)
	local top    = memory.readwordsigned(addr + 6)

	if facing_dir == 1 then
		left  = -left
		right = -right
	end

	left   = left   + offset_x
	right  = right  + left
	bottom = bottom + offset_y
	top    = top    + bottom

	if type == HITBOX_PASSIVE then
		obj.p_hboxes[i] = {
			left   = left,
			right  = right,
			bottom = bottom,
			top    = top,
			type   = type
		}
	else
		obj.a_hboxes[i] = {
			left   = left,
			right  = right,
			bottom = bottom,
			top    = top,
			type   = type
		}
	end
end


function update_game_object(obj, base)
	obj.p_hboxes = {}
	obj.a_hboxes = {}

	obj.facing_dir   = memory.readbyte(base + 0xA)
	obj.opponent_dir = memory.readbyte(base + 0xB)
	obj.pos_x        = memory.readword(base + 0x64)
	obj.pos_y        = memory.readword(base + 0x68)
	obj.anim_frame   = memory.readword(base + 0x21A)

	-- Load the passive hitboxes
	local p_hb_addr = memory.readdword(base + 0x2A0)
	for i = 1, 4 do
		hitbox_load(obj, i, HITBOX_PASSIVE, obj.facing_dir, obj.pos_x, obj.pos_y, p_hb_addr)
		p_hb_addr = p_hb_addr + 8
	end

	-- Load the active hitboxes
	local a_hb_addr = memory.readdword(base + 0x2C8)
	for i = 1, 4 do
		hitbox_load(obj, i, HITBOX_ACTIVE, obj.facing_dir, obj.pos_x, obj.pos_y, a_hb_addr)
		a_hb_addr = a_hb_addr + 8
	end
end


function read_misc_objects()
	local obj_index
	local obj_addr

	local p_hb_addr
	local a_hb_addr

	-- This function reads all game objects other than the two player characters.
	-- This includes all projectiles and even Yang's Seiei-Enbu shadows.

	-- The game uses the same structure all over the place and groups them
	-- into lists with each element containing an index to the next element
	-- in that list. An index of -1 signals the end of the list.

	-- I believe there are at least 7 lists (0-6) but it seems everything we need
	-- (and lots we don't) is in list 3.
	local list = 3

	num_misc_objs = 1
	obj_index = memory.readwordsigned(0x02068A96 + (list * 2))

	while num_misc_objs <= MAX_GAME_OBJECTS and obj_index ~= -1 do
		obj_addr = 0x02028990 + (obj_index * 0x800)

		-- I don't really know how to tell different game objects types apart yet so
		-- just read everything that has non-zero hitbox addresses. Seems to
		-- work fine...
		p_hb_addr = memory.readdword(obj_addr + 0x2A0)
		a_hb_addr = memory.readdword(obj_addr + 0x2C8)

		if p_hb_addr ~= 0 and a_hb_addr ~= 0 then
			misc_objs[num_misc_objs] = {}
			update_game_object(misc_objs[num_misc_objs], obj_addr)
			num_misc_objs = num_misc_objs + 1
		end

		-- Get the index to the next object in this list.
		obj_index = memory.readwordsigned(obj_addr + 0x1C)
	end
end


function game_x_to_mame(x)
	local left_edge = globals.screen_center_x - (SCREEN_WIDTH / 2)
	return (x - left_edge)
end


function game_y_to_mame(y)
	-- Why subtract 17? No idea, the game driver does the same thing.
	return (SCREEN_HEIGHT - (y + GROUND_OFFSET - 17))
end


function draw_hitbox(hb)
	local left   = game_x_to_mame(hb.left)
	local bottom = game_y_to_mame(hb.bottom)
	local right  = game_x_to_mame(hb.right)
	local top    = game_y_to_mame(hb.top)

	if(hb.type == HITBOX_PASSIVE) then
		colour = HITBOX_PASSIVE_COLOUR
	else
		colour = HITBOX_ACTIVE_COLOUR
	end

	gui.box(left, top, right, bottom, colour)
end


function draw_game_object(obj)
	local x = game_x_to_mame(obj.pos_x)
	local y = game_y_to_mame(obj.pos_y)

	for i = 1, 4 do
		draw_hitbox(obj.p_hboxes[i])
		draw_hitbox(obj.a_hboxes[i])
	end

	gui.drawline(x, y-AXIS_SIZE, x, y+AXIS_SIZE, AXIS_COLOUR)
	gui.drawline(x-AXIS_SIZE, y, x+AXIS_SIZE, y, AXIS_COLOUR)
end


function render_sfiii_hitboxes()
	update_globals()
	if globals.game_phase ~= GAME_PHASE_PLAYING then
		gui.clearuncommitted()
		return
	end

	update_game_object(player1, address.player1)
	draw_game_object(player1)
	update_game_object(player2, address.player2)
	draw_game_object(player2)

	read_misc_objects()
	for i = 1, num_misc_objs-1 do
		draw_game_object(misc_objs[i])
	end
end


gui.register( function()
	render_sfiii_hitboxes()
end)