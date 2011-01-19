print("Capcom beat 'em up hitbox viewer")
print("January 19, 2011")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis") print()

local VULNERABILITY_COLOR    = 0x7777FF40
local ATTACK_COLOR           = 0xFF000040
local AXIS_COLOR             = 0xFFFFFFFF
local BLANK_COLOR            = 0xFFFFFFFF
local AXIS_SIZE              = 16
local MINI_AXIS_SIZE         = 2
local DRAW_DELAY             = 1

local SCREEN_WIDTH           = 384
local SCREEN_HEIGHT          = 224
local VULNERABILITY_BOX      = 1
local ATTACK_BOX             = 2
local GAME_PHASE_NOT_PLAYING = 0
local BLANK_SCREEN           = false
local DRAW_AXIS              = false
local DRAW_MINI_AXIS         = false

local fill = {
	[VULNERABILITY_BOX] = VULNERABILITY_COLOR,
	[ATTACK_BOX]        = ATTACK_COLOR,
}

local outline = {
	[VULNERABILITY_BOX] = OR(VULNERABILITY_COLOR, 0xFF),
	[ATTACK_BOX]        = OR(ATTACK_COLOR,        0xFF),
}

local profile = {
	{
		games = {"ffight"},
		address = {
			left_screen_edge = 0xFF8412,
			game_phase       = 0xFF8622,
		},
		offset = {
			facing_dir       = 0x2E,
			x_position       = 0x06,
			hitbox_ptr       = 0x38,
			hval = 0x0, vval = 0x2, hrad = 0x4, vrad = 0x6,
		},
		objects = {
			{address = 0xFF8568, number = 0x1D, offset = 0xC0}, --players and enemies
			{address = 0xFFB2E8, number = 0x10, offset = 0xC0}, --etc
		},
		boxes = {
			{anim_ptr = 0x24, addr_table = 0x02, id_ptr = 0x04, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x24, addr_table = 0x00, id_ptr = 0x05, id_space = 0x10, type = ATTACK_BOX},
		},
		id_read  = memory.readbytesigned,
		box_read = memory.readwordsigned,
	},
	{
		games = {"captcomm"}, --not working
		address = {
			left_screen_edge = 0xFFE99E,
			game_phase       = 0xFFF86F,
		},
		offset = {
			facing_dir       = 0x7B,
			x_position       = 0x0A,
			z_position       = 0x12,
			hitbox_ptr       = 0x38,--?
			hval = 0x0, vval = 0x4, hrad = 0x2, vrad = 0x6,--?
		},
		objects = {
			{address = 0xFFA994, number = 0x04, offset = 0x100}, --players
			{address = 0xFFAD94, number = 0x50, offset = 0x0C0}, --etc
		},
		boxes = {
			--{anim_ptr = 0x1C, addr_table = 0x00, id_ptr = 0x04, id_space = 0x08, type = VULNERABILITY_BOX},
			--{anim_ptr = 0x1C, addr_table = 0x00, id_ptr = 0x05, id_space = 0x10, type = ATTACK_BOX},
		},
		id_read  = memory.readbyte,--?
		box_read = memory.readwordsigned,--?
	},
	{
		games = {"dino"},
		address = {
			left_screen_edge = 0xFF8744,
			game_phase       = 0xFFDD55,
		},
		offset = {
			facing_dir       = 0x24,
			x_position       = 0x08,
			z_position       = 0x10,
			hitbox_ptr       = 0x44,
			hval = 0x4, vval = 0x8, hrad = 0x6, vrad = 0xA,
		},
		objects = {
			{address = 0xFF8874, number = 0x18, offset = 0x0C0}, --items
			{address = 0xFFB274, number = 0x03, offset = 0x180}, --players
			{address = 0xFFB6F4, number = 0x18, offset = 0x0C0}, --etc
			{address = 0xFFC8F4, number = 0x18, offset = 0x0E0}, --enemies
		},
		boxes = {
			{anim_ptr = nil, addr_table = 0x00, id_ptr = 0x48, id_space = 0x0C, type = VULNERABILITY_BOX},
			{anim_ptr = nil, addr_table = 0x00, id_ptr = 0x49, id_space = 0x0C, type = ATTACK_BOX},
		},
		id_read  = memory.readbyte,
		box_read = memory.readwordsigned,
		no_double_radius = true,
	},
	{
		games = {"punisher"},
		address = {
			left_screen_edge = 0xFF7376,
			game_phase       = 0xFF4E80,
		},
		offset = {
			facing_dir       = 0x07,
			x_position       = 0x20,
			z_position       = 0x28,
			hitbox_ptr       = 0x30,
			hval = 0x4, vval = 0x8, hrad = 0x6, vrad = 0xA,
		},
		objects = {
			{address = 0xFF8E68, number = 0x02, offset = 0x100}, --players
			{address = 0xFF9068, number = 0x90, offset = 0x0C0}, --etc
		},
		boxes = {
			{anim_ptr = nil, addr_table = nil, id_ptr = 0x3E, id_space = 0x01, type = VULNERABILITY_BOX,
				invalid = {
					{offset = 0x4D, value = 0x00, equal = false}, 
					{offset = 0xC3, value = 0x00, equal = false},
				}
			},
			{anim_ptr = nil, addr_table = nil, id_ptr = 0x3C, id_space = 0x01, type = ATTACK_BOX,
				invalid = {{offset = 0x1B, value = 0x00, equal = true}}
			},
		},
		id_read  = memory.readword,
		box_read = memory.readwordsigned,
		no_double_radius = true,
	},
	{
		games = {"avsp"},
		address = {
			left_screen_edge = 0xFF8288,
			game_phase       = 0xFFEE0A,
		},
		offset = {
			facing_dir       = 0x0F,
			x_position       = 0x10,
			hitbox_ptr       = 0x50,
			hval = 0x0, vval = 0x1, hrad = 0x2, vrad = 0x3,
		},
		objects = {
			{address = 0xFF8380, number = 0x03, offset = 0x100}, --players
			{address = 0xFF8680, number = 0x18, offset = 0x080}, --player projectiles
			{address = 0xFF9280, number = 0x18, offset = 0x0C0}, --enemies
			{address = 0xFFA480, number = 0x18, offset = 0x080}, --enemy projectiles
			{address = 0xFFB080, number = 0x3C, offset = 0x080, harmless = true}, --etc.
		},
		boxes = {
			{anim_ptr = 0x1C, addr_table = 0x00, id_ptr = 0x08, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table = 0x02, id_ptr = 0x09, id_space = 0x10, type = ATTACK_BOX},
		},
		id_read  = memory.readbyte,
		box_read = memory.readbytesigned,
	},
}

for game in ipairs(profile) do
	for entry in ipairs(profile[game].boxes) do
		local box = profile[game].boxes[entry]
		box.invalid = box.invalid or {}
	end
end

local game
local globals = {
	game_phase       = 0,
	left_screen_edge = 0,
	top_screen_edge  = 0,
}
local player       = {}
local projectiles  = {}
local frame_buffer = {}
if fba then
	DRAW_DELAY = DRAW_DELAY + 1
end


--------------------------------------------------------------------------------
-- hotkey functions

input.registerhotkey(1, function()
	BLANK_SCREEN = not BLANK_SCREEN
	print((BLANK_SCREEN and "activated" or "deactivated") .. " blank screen mode")
end)


input.registerhotkey(2, function()
	DRAW_AXIS = not DRAW_AXIS
	print((DRAW_AXIS and "showing" or "hiding") .. " object axis")
end)


input.registerhotkey(3, function()
	DRAW_MINI_AXIS = not DRAW_MINI_AXIS
	print((DRAW_MINI_AXIS and "showing" or "hiding") .. " hitbox axis")
end)


--------------------------------------------------------------------------------
-- initialize on game startup

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
				return
			end
		end
	end
	print("not prepared for " .. emu.romname() .. " hitboxes")
end


emu.registerstart( function()
	whatgame()
end)


--------------------------------------------------------------------------------
-- prepare the hitboxes

local function update_globals()
	globals.left_screen_edge = memory.readwordsigned(game.address.left_screen_edge)
	globals.top_screen_edge  = memory.readwordsigned(game.address.left_screen_edge + 0x4)
	globals.game_phase       = memory.readword(game.address.game_phase)
end


local function game_x_to_mame(x)
	return x - globals.left_screen_edge
end


local function game_y_to_mame(y)
	return SCREEN_HEIGHT - (y - 15) + globals.top_screen_edge
end


local function define_box(obj, entry, space)
	for _,check in ipairs(game.boxes[entry].invalid) do
		local no_draw = memory.readbyte(obj.base + check.offset) == check.value
		if not check.equal then
			no_draw = not no_draw
		end
		if no_draw and check.offset < space then
			return nil
		end
	end

	local base_id = obj.base
	if game.boxes[entry].anim_ptr then
		base_id = memory.readdword(obj.base + game.boxes[entry].anim_ptr)
	end
	local curr_id = game.id_read(base_id + game.boxes[entry].id_ptr)

	if base_id == 0 or curr_id <= 0 then
		obj[entry] = nil
		return
	end
	
	local addr_table
	if not game.offset.hitbox_ptr then
		addr_table = memory.readdword(obj.base + game.boxes[entry].addr_table)
	else
		addr_table = memory.readdword(obj.base + game.offset.hitbox_ptr)
		if game.boxes[entry].addr_table then
			addr_table = addr_table + memory.readwordsigned(addr_table + game.boxes[entry].addr_table)
		end
	end
	local address = addr_table + curr_id * game.boxes[entry].id_space
	--local address = 0xf4d8a + memory.readword(obj.base+0x26)*4 + memory.readword(obj.base+0x7A) --captcomm test
	--emu.message(string.format("%X",0xf4d8a + memory.readword(obj.base+0x26)*4 + memory.readword(obj.base+0x7A)))

	local hval = game.box_read(address + game.offset.hval)
	local vval = game.box_read(address + game.offset.vval)
	local hrad = game.box_read(address + game.offset.hrad)
	local vrad = game.box_read(address + game.offset.vrad)

	if game.no_double_radius then
		if obj.facing_dir > 0 then
			hval = -hval
			hrad = -hrad
		end

		obj[entry] = {
			left   = obj.pos_x + hval,
			right  = obj.pos_x + hval + hrad,
			top    = obj.pos_y - vval - vrad,
			bottom = obj.pos_y - vval,
			hval   = obj.pos_x + hval + hrad/2,
			vval   = obj.pos_y - vval - vrad/2,
			type   = game.boxes[entry].type,
		}
	else
		if obj.facing_dir > 0 then
			hval  = -hval
		end

		obj[entry] = {
			left   = obj.pos_x + hval - hrad,
			right  = obj.pos_x + hval + hrad,
			top    = obj.pos_y - vval - vrad,
			bottom = obj.pos_y - vval + vrad,
			hval   = obj.pos_x + hval,
			vval   = obj.pos_y - vval,
			type   = game.boxes[entry].type,
		}
	end
end


local function update_game_object(obj, space)
	obj.facing_dir = memory.readbyte(obj.base + game.offset.facing_dir)
	obj.pos_z      = game.offset.z_position and memory.readwordsigned(obj.base + game.offset.z_position) or 0
	obj.pos_x      = game_x_to_mame(memory.readwordsigned(obj.base + game.offset.x_position))
	obj.pos_y      = game_y_to_mame(memory.readwordsigned(obj.base + game.offset.x_position + 4) + obj.pos_z)

	for entry in ipairs(game.boxes) do
		define_box(obj, entry, space)
	end
end


local function update_beatemup_hitboxes()
	gui.clearuncommitted()
	if not game then return end
	update_globals()

	local objects = {}
	for _,set in ipairs(game.objects) do
		for n = 1, set.number do
			local obj = {base = set.address + (n-1) * set.offset}
			if memory.readword(obj.base) >= 0x0100 then
				obj.harmless = set.harmless
				update_game_object(obj, set.offset)
				table.insert(objects, obj)
			end
		end
	end

	for f = 1, DRAW_DELAY do
		frame_buffer[f] = copytable(frame_buffer[f+1])
	end

	frame_buffer[DRAW_DELAY+1] = copytable(objects)
end


emu.registerafter( function()
	update_beatemup_hitboxes()
end)


--------------------------------------------------------------------------------
-- draw the hitboxes

local function draw_hitbox(hb)
	--if hb.left > hb.right or hb.bottom > hb.top then return end

	if DRAW_MINI_AXIS then
		gui.drawline(hb.hval, hb.vval-MINI_AXIS_SIZE, hb.hval, hb.vval+MINI_AXIS_SIZE, OR(fill[hb.type], 0xFF))
		gui.drawline(hb.hval-MINI_AXIS_SIZE, hb.vval, hb.hval+MINI_AXIS_SIZE, hb.vval, OR(fill[hb.type], 0xFF))
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, fill[hb.type], outline[hb.type])
end


local function draw_game_object(obj)
	if not (obj and obj.pos_x) then
		return
	end
	
	gui.drawline(obj.pos_x, obj.pos_y-AXIS_SIZE, obj.pos_x, obj.pos_y+AXIS_SIZE, AXIS_COLOR)
	gui.drawline(obj.pos_x-AXIS_SIZE, obj.pos_y, obj.pos_x+AXIS_SIZE, obj.pos_y, AXIS_COLOR)
end


local function render_beatemup_hitboxes()
	if not game or globals.game_phase == GAME_PHASE_NOT_PLAYING then
		return
	end

	if BLANK_SCREEN then
		gui.box(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLANK_COLOR)
	end

	for entry in ipairs(game.boxes) do
		for _,obj in ipairs(frame_buffer[1]) do
			if obj[entry] and not (obj.harmless and game.boxes[entry].type == ATTACK_BOX) then
				draw_hitbox(obj[entry])
			end
		end
	end

	if DRAW_AXIS then
		for _,obj in ipairs(frame_buffer[1]) do
			draw_game_object(obj)
		end
	end
end


gui.register( function()
	render_beatemup_hitboxes()
end)