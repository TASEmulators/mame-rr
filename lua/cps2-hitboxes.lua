print("CPS-2 hitbox viewer")
print("October 14, 2010")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis")
print("Lua hotkey 4: toggle pushboxes")
print("Lua hotkey 5: toggle throwboxes") print()

local VULNERABILITY_COLOR    = 0x7777FF40
local ATTACK_COLOR           = 0xFF000060
local PUSH_COLOR             = 0x00FF0040
local THROW_COLOR            = 0xFFFF0060
local THROWABLE_COLOR        = 0xFFFFFF00
local AXIS_COLOR             = 0xFFFFFFFF
local BLANK_COLOR            = 0xFFFFFFFF
local AXIS_SIZE              = 16
local MINI_AXIS_SIZE         = 2
local DRAW_DELAY             = 1

local SCREEN_WIDTH           = 384
local SCREEN_HEIGHT          = 224
local VULNERABILITY_BOX      = 1
local ATTACK_BOX             = 2
local PUSH_BOX               = 3
local THROW_BOX              = 4
local THROWABLE_BOX          = 5
local GAME_PHASE_NOT_PLAYING = 0
local BLANK_SCREEN           = false
local DRAW_AXIS              = false
local DRAW_MINI_AXIS         = false
local DRAW_PUSHBOXES         = true
local DRAW_THROWBOXES        = true

local fill = {
	[VULNERABILITY_BOX] = VULNERABILITY_COLOR,
	[ATTACK_BOX]        = ATTACK_COLOR,
	[PUSH_BOX]          = PUSH_COLOR,
	[THROW_BOX]         = THROW_COLOR,
	[THROWABLE_BOX]     = THROWABLE_COLOR,
}

local outline = {
	[VULNERABILITY_BOX] = OR(VULNERABILITY_COLOR, 0xFF),
	[ATTACK_BOX]        = OR(ATTACK_COLOR,        0xFF),
	[PUSH_BOX]          = OR(PUSH_COLOR,          0xC0),
	[THROW_BOX]         = OR(THROW_COLOR,         0xFF),
	[THROWABLE_BOX]     = OR(THROWABLE_COLOR,     0xE0),
}

local profile = {
	{
		games = {"sfa"},
		number = {players = 3, projectiles = 8},
		address = {
			player           = 0xFF8400,
			projectile       = 0xFF9000,
			left_screen_edge = 0xFF8290,
			game_phase       = 0xFF8280,
		},
		offset = {
			projectile_space = 0x80,
			facing_dir       = 0x0B,
			hitbox_ptr       = 0x50,
			invulnerability  = {},
			hval = 0x0, vval = 0x1, hrad = 0x2, vrad = 0x3,
		},
		boxes = {
			{anim_ptr = 0x20, addr_table_ptr = 0x08, p_addr_table_ptr = 0x4, id_ptr = 0x0C, id_space = 0x04, type = PUSH_BOX},
			{anim_ptr = 0x20, addr_table_ptr = 0x00, p_addr_table_ptr = 0x0, id_ptr = 0x08, id_space = 0x04, type = VULNERABILITY_BOX},
			{anim_ptr = 0x20, addr_table_ptr = 0x02, p_addr_table_ptr = 0x0, id_ptr = 0x09, id_space = 0x04, type = VULNERABILITY_BOX},
			{anim_ptr = 0x20, addr_table_ptr = 0x04, p_addr_table_ptr = 0x0, id_ptr = 0x0A, id_space = 0x04, type = VULNERABILITY_BOX},
			{anim_ptr = 0x20, addr_table_ptr = 0x06, p_addr_table_ptr = 0x2, id_ptr = 0x0B, id_space = 0x10, type = ATTACK_BOX},
		},
		box_parameter_func = memory.readbytesigned,
	},
	{
		games = {"sfa2","sfz2al"},
		number = {players = 3, projectiles = 26},
		address = {
			player           = 0xFF8400,
			projectile       = 0xFF9400,
			left_screen_edge = 0xFF8290,
			game_phase       = 0xFF812D,
		},
		offset = {
			projectile_space = 0x80,
			facing_dir       = 0x0B,
			hitbox_ptr       = {player = nil, projectile = 0x60},
			invulnerability  = {0x273},
			hval = 0x0, vval = 0x2, hrad = 0x4, vrad = 0x6,
		},
		boxes = {
			{anim_ptr = 0x1C, addr_table_ptr = 0x120, p_addr_table_ptr = 0x4, id_ptr = 0x0C, id_space = 0x08, type = PUSH_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x110, p_addr_table_ptr = 0x0, id_ptr = 0x08, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x114, p_addr_table_ptr = 0x0, id_ptr = 0x09, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x118, p_addr_table_ptr = 0x0, id_ptr = 0x0A, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x11C, p_addr_table_ptr = 0x2, id_ptr = 0x0B, id_space = 0x20, type = ATTACK_BOX},
		},
	},
	{
		games = {"sfa3"},
		number = {players = 4, projectiles = 24},
		address = {
			player           = 0xFF8400,
			projectile       = 0xFF9400,
			left_screen_edge = 0xFF8290,
			game_phase       = 0xFF812D,
		},
		offset = {
			projectile_space = 0x100,
			status           = 0x04,
			facing_dir       = 0x0B,
			hitbox_ptr       = nil,
			invulnerability  = {0xD6, 0x25D},
			hval = 0x0, vval = 0x2, hrad = 0x4, vrad = 0x6,
		},
		boxes = {
			{anim_ptr =  nil, addr_table_ptr = 0x9C, id_ptr =  0xCB, id_space = 0x08, type = PUSH_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr =  0xC8, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x94, id_ptr =  0xC9, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x98, id_ptr =  0xCA, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0xA0, id_ptr =   0x9, id_space = 0x20, type = ATTACK_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0xA0, id_ptr = 0x32F, id_space = 0x20, type = THROW_BOX, zero = 0x32F},
			{anim_ptr =  nil, addr_table_ptr = 0xA0, id_ptr =  0x82, id_space = 0x20, type = ATTACK_BOX, tripwire = 0x1E4},
		},
	},
	{
		games = {"dstlk"},
		number = {players = 2, projectiles = 4},
		address = {
			player           = 0xFF8388,
			projectile       = 0xFFAA2E,
			left_screen_edge = 0xFF9518,
			game_phase       = 0xFF9475,
		},
		offset = {
			projectile_space = 0xC0,
			facing_dir       = 0x09,
			hitbox_ptr       = 0x5C,
			invulnerability  = {0x11D},
			hval = 0x0, vval = 0x4, hrad = 0x2, vrad = 0x6,
		},
		boxes = {
			{anim_ptr = 0x1C, addr_table_ptr = 0x0A, id_ptr = 0x15, id_space = 0x08, type = PUSH_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x00, id_ptr = 0x10, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x02, id_ptr = 0x11, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x04, id_ptr = 0x12, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x06, id_ptr = 0x13, id_space = 0x08, type = THROW_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x08, id_ptr = 0x14, id_space = 0x10, type = ATTACK_BOX},
		},
	},
	{
		games = {"nwarr"},
		number = {players = 2, projectiles = 12},
		address = {
			player           = 0xFF8388,
			projectile       = 0xFFA86E,
			left_screen_edge = 0xFF8F18,
			game_phase       = 0xFF988B,
		},
		offset = {
			player_space     = 0x500,
			projectile_space = 0xC0,
			facing_dir       = 0x09,
			hitbox_ptr       = 0x5C,
			invulnerability  = {0x11D},
			hval = 0x0, vval = 0x4, hrad = 0x2, vrad = 0x6,
		},
		boxes = {
			{anim_ptr = 0x1C, addr_table_ptr = 0x0A, id_ptr = 0x15, id_space = 0x08, type = PUSH_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x00, id_ptr = 0x10, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x02, id_ptr = 0x11, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x04, id_ptr = 0x12, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x06, id_ptr = 0x13, id_space = 0x08, type = THROW_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x08, id_ptr = 0x14, id_space = 0x10, type = ATTACK_BOX},
		},
		special_projectiles = {start = 0xFF9A6E, space = 0x80, number = 28},
	},
	{
		games = {"vsav","vhunt2","vsav2"},
		number = {players = 2, projectiles = 32},
		address = {
			player           = 0xFF8400,
			projectile       = 0xFF9400,
			left_screen_edge = 0xFF8290,
			game_phase       = 0xFF812D,
		},
		offset = {
			projectile_space = 0x100,
			facing_dir       = 0x0B,
			hitbox_ptr       = nil,
			invulnerability  = {0x147},
			hval = 0x0, vval = 0x2, hrad = 0x4, vrad = 0x6,
		},
		boxes = {
			{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr = 0x97, id_space = 0x08, type = PUSH_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x80, id_ptr = 0x94, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x84, id_ptr = 0x95, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x96, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x8C, id_ptr = 0x0A, id_space = 0x20, type = ATTACK_BOX},
			--{anim_ptr =  nil, addr_table_ptr = 0x8C, id_ptr = "?", id_space = 0x20, type = THROW_BOX},
		},
	},
	{
		games = {"ringdest"},
		number = {players = 2, projectiles = 28},
		address = {
			player           = 0xFF8000,
			projectile       = 0xFF9000,
			left_screen_edge = 0xFF72D2,
			top_screen_edge  = 0xFF72D4,
			game_phase       = 0xFF72D2,
		},
		offset = {
			top_screen_edge  = 23,
			projectile_space = 0x100,
			facing_dir       = 0x38,
			id_ptr           = 0x4A,
			invulnerability  = {},
			hval = 0x0, vval = 0x4, hrad = 0x2, vrad = 0x6,
		},
		boxes = {
			{addr_table_ptr = 0x2D8, type = PUSH_BOX},
			{addr_table_offset = 0xC956, id_space = 0x04, type = THROWABLE_BOX},
			{addr_table_offset = 0xC92E, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table_offset = 0xC936, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table_offset = 0xC93E, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table_offset = 0xC946, id_space = 0x04, type = ATTACK_BOX},
			{addr_table_offset = 0xC94E, id_space = 0x04, type = THROW_BOX},
		},
	},
	{
		games = {"cybots"},
		number = {players = 2, projectiles = 16},
		address = {
			player           = 0xFF81A0,
			projectile       = 0xFF92A0,
			left_screen_edge = 0xFFECF4,
			game_phase       = 0xFF89A0,
		},
		offset = {
			projectile_space = 0xC0,
			facing_dir       = 0x09,
			x_position       = 0x1A,
			hitbox_ptr       = 0x32,
			invulnerability  = {},
			hval = 0x0, vval = 0x2, hrad = 0x4, vrad = 0x6,
		},
		boxes = {
			{anim_ptr = nil, addr_table_ptr = 0x08, id_ptr = 0x66, id_space = 0x08, type = PUSH_BOX},
			{anim_ptr = nil, addr_table_ptr = 0x02, id_ptr = 0x63, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = nil, addr_table_ptr = 0x04, id_ptr = 0x64, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = nil, addr_table_ptr = 0x06, id_ptr = 0x65, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = nil, addr_table_ptr = 0x00, id_ptr = 0x62, id_space = 0x10, type = ATTACK_BOX},
		},
	},
	{
		games = {"sgemf"},
		number = {players = 2, projectiles = 14},
		address = {
			player           = 0xFF8400,
			projectile       = 0xFF8C00,
			left_screen_edge = 0xFF8290,
			game_phase       = 0xFFCBBC,
		},
		offset = {
			projectile_space = 0x100,
			facing_dir       = 0x0B,
			hitbox_ptr       = nil,
			invulnerability  = {0x147},
			hval = 0x0, vval = 0x2, hrad = 0x4, vrad = 0x6,
		},
		boxes = {
			{anim_ptr =  nil, addr_table_ptr = 0x8C, id_ptr = 0x93, id_space = 0x08, type = PUSH_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x8C, id_ptr = 0x0B, id_space = 0x08, type = THROWABLE_BOX}, --same as pushbox?
			{anim_ptr =  nil, addr_table_ptr = 0x80, id_ptr = 0x90, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x84, id_ptr = 0x91, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x92, id_space = 0x20, type = ATTACK_BOX},
			--{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = "?", id_space = 0x20, type = THROW_BOX},
		},
		box_parameter_radscale = 2,
	},
}

for game in ipairs(profile) do
	local g = profile[game]
	if type(g.offset.hitbox_ptr) == "number" then
		local ptr = g.offset.hitbox_ptr
		g.offset.hitbox_ptr = {player = ptr, projectile = ptr}
	end
	g.address.top_screen_edge = g.address.top_screen_edge or g.address.left_screen_edge + 0x4
	g.offset.top_screen_edge  = g.offset.top_screen_edge  or -17
	g.offset.player_space     = g.offset.player_space     or 0x400
	g.offset.x_position       = g.offset.x_position       or 0x10
	g.offset.y_position       = g.offset.y_position       or g.offset.x_position + 0x4
	g.offset.hitbox_ptr       = g.offset.hitbox_ptr       or {}
	g.box_parameter_func      = g.box_parameter_func      or memory.readwordsigned
	g.box_parameter_radscale  = g.box_parameter_radscale  or 1
	g.special_projectiles     = g.special_projectiles     or {number = 0}
end

local game, prepare_boxes
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


input.registerhotkey(4, function()
	DRAW_PUSHBOXES = not DRAW_PUSHBOXES
	print((DRAW_PUSHBOXES and "showing" or "hiding") .. " pushboxes")
end)


input.registerhotkey(5, function()
	DRAW_THROWBOXES = not DRAW_THROWBOXES
	print((DRAW_THROWBOXES and "showing" or "hiding") .. " throwboxes")
end)


--------------------------------------------------------------------------------
-- prepare the hitboxes

local function update_globals()
	globals.left_screen_edge = memory.readword(game.address.left_screen_edge)
	globals.top_screen_edge  = memory.readword(game.address.top_screen_edge)
	globals.game_phase       = memory.readword(game.address.game_phase)
end


local function game_x_to_mame(x)
	return (x - globals.left_screen_edge)
end


local function game_y_to_mame(y)
	-- Why subtract 17? No idea, the game driver does the same thing.
	return SCREEN_HEIGHT - (y + game.offset.top_screen_edge) + globals.top_screen_edge
end


local function define_box(obj, entry, hitbox_ptr, is_projectile)
	if game.boxes[entry].type == THROW_BOX and is_projectile then
		return nil
	end

	local base_id = obj.base
	if game.boxes[entry].anim_ptr then
		base_id = memory.readdword(obj.base + game.boxes[entry].anim_ptr)
	end
	local curr_id = memory.readbyte(base_id + game.boxes[entry].id_ptr)

	if game.boxes[entry].zero then
		memory.writebyte(base_id + game.boxes[entry].zero, 0x0)
	end

	local wire_pos
	if game.boxes[entry].tripwire then
		curr_id = curr_id / 2 + 0x3E
		wire_pos = memory.readwordsigned(obj.base + game.boxes[entry].tripwire)
		if wire_pos == 0 or memory.readbyte(obj.base + 0x102) ~= 0xE then
			return nil
		elseif memory.readbyte(obj.base + 0x216) == 0x0 then
			memory.writeword(obj.base + game.boxes[entry].tripwire, 0x0)
		end
	end

	if curr_id == 0 then
		return nil
	end

	local addr_table
	if not hitbox_ptr then
		addr_table = memory.readdword(obj.base + game.boxes[entry].addr_table_ptr)
	else
		local table_offset = is_projectile and game.boxes[entry].p_addr_table_ptr or game.boxes[entry].addr_table_ptr
		addr_table = memory.readdword(obj.base + hitbox_ptr)
		addr_table = addr_table + memory.readwordsigned(addr_table + table_offset)
	end
	local address = addr_table + curr_id * game.boxes[entry].id_space

	local hval = game.box_parameter_func(address + game.offset.hval)
	local vval = game.box_parameter_func(address + game.offset.vval)
	local hrad = game.box_parameter_func(address + game.offset.hrad)/game.box_parameter_radscale
	local vrad = game.box_parameter_func(address + game.offset.vrad)/game.box_parameter_radscale

	if obj.facing_dir == 1 then
		hval  = -hval
	end
	hval = hval + (wire_pos or 0)

	return {
		left   = game_x_to_mame(obj.pos_x + hval - hrad),
		right  = game_x_to_mame(obj.pos_x + hval + hrad),
		bottom = game_y_to_mame(obj.pos_y + vval + vrad),
		top    = game_y_to_mame(obj.pos_y + vval - vrad),
		hval   = game_x_to_mame(obj.pos_x + hval),
		vval   = game_y_to_mame(obj.pos_y + vval),
		type   = game.boxes[entry].type,
	}
end


local function define_id_offset_box(obj, entry, id_offset, is_projectile) --for ringdest only
	if game.boxes[entry].type == PUSH_BOX and (is_projectile or memory.readbyte(obj.base + 0x71) > 0 or
	(memory.readword(obj.base + 0x2C0) == 0xFF and memory.readword(obj.base + 0x2D2) < 0x8)) then
		return nil
	end

	local address
	if game.boxes[entry].addr_table_offset then
		address = game.boxes[entry].addr_table_offset + game.boxes[entry].id_space * id_offset
	else
		address = memory.readdword(obj.base + game.boxes[entry].addr_table_ptr)
	end

	local hval = memory.readwordsigned(address + game.offset.hval)
	local vval = memory.readwordsigned(address + game.offset.vval)
	local hrad = memory.readwordsigned(address + game.offset.hrad)
	local vrad = memory.readwordsigned(address + game.offset.vrad)

	if hrad == 0 or vrad == 0 then
		return nil
	end

	if obj.facing_dir > 0 then
		hval = -hval
		hrad = -hrad
	end

	return {
		left   = game_x_to_mame(obj.pos_x + hval),
		right  = game_x_to_mame(obj.pos_x + hval + hrad),
		top    = game_y_to_mame(obj.pos_y + vval),
		bottom = game_y_to_mame(obj.pos_y + vval + vrad),
		hval   = game_x_to_mame(obj.pos_x + hval + hrad/2),
		vval   = game_y_to_mame(obj.pos_y + vval + vrad/2),
		type   = game.boxes[entry].type,
	}
end


local function prepare_normal_boxes(obj, is_projectile)
	local hitbox_ptr = is_projectile and game.offset.hitbox_ptr.projectile or game.offset.hitbox_ptr.player
	for entry in ipairs(game.boxes) do
		obj[entry] = define_box(obj, entry, hitbox_ptr, is_projectile)
	end
end


local function prepare_id_offset_boxes(obj, is_projectile)
	local id_offset = memory.readword(obj.base + game.offset.id_ptr)
	for entry in ipairs(game.boxes) do
		obj[entry] = define_id_offset_box(obj, entry, id_offset, is_projectile)
	end
end


local function update_game_object(obj, is_projectile)
	obj.facing_dir   = memory.readbyte(obj.base + game.offset.facing_dir)
	obj.pos_x        = memory.readwordsigned(obj.base + game.offset.x_position)
	obj.pos_y        = memory.readwordsigned(obj.base + game.offset.y_position)
	prepare_boxes(obj, is_projectile)
end


local function read_projectiles()
	local current_projectiles = {}

	for i = 1, game.number.projectiles do
		local obj = {base = game.address.projectile + (i-1) * game.offset.projectile_space}
		if memory.readword(obj.base) > 0x0100 then
			update_game_object(obj, true)
			table.insert(current_projectiles, obj)
		end
	end

	for i = 1, game.special_projectiles.number do --for nwarr only
		local obj = {base = game.special_projectiles.start + (i-1) * game.special_projectiles.space}
		if memory.readword(obj.base) >= 0x0100 and 
		memory.readdword(obj.base + game.boxes[1].anim_ptr) > 0 and
		memory.readdword(obj.base + game.offset.hitbox_ptr.projectile) > 0 then
			update_game_object(obj, true)
			table.insert(current_projectiles, obj)
		end
	end

	return current_projectiles
end


local function update_invulnerability(player)
	player.invulnerability = false
	for _,address in ipairs(game.offset.invulnerability) do
		if memory.readbyte(player.base + address) > 0 then
			player.invulnerability = true
		end
	end
end


local function update_cps2_hitboxes()
	gui.clearuncommitted()
	if not game then return end
	update_globals()

	for p = 1, game.number.players do
		player[p] = {base = game.address.player + (p-1) * game.offset.player_space}
		if memory.readbyte(player[p].base) > 0 then
			update_game_object(player[p])
			update_invulnerability(player[p])
		else
			player[p] = {}
		end
	end

	for f = 1, DRAW_DELAY do
		for p = 1, game.number.players do
			frame_buffer[f][player][p] = copytable(frame_buffer[f+1][player][p])
		end
		frame_buffer[f][projectiles] = copytable(frame_buffer[f+1][projectiles])
	end

	for p = 1, game.number.players do
		frame_buffer[DRAW_DELAY+1][player][p] = copytable(player[p])
	end
	frame_buffer[DRAW_DELAY+1][projectiles] = read_projectiles()

end


emu.registerafter( function()
	update_cps2_hitboxes()
end)


--------------------------------------------------------------------------------
-- draw the hitboxes

local function draw_hitbox(hb, invulnerability)
	if (not DRAW_PUSHBOXES and hb.type == PUSH_BOX)
	or (not DRAW_THROWBOXES and (hb.type == THROW_BOX or hb.type == THROWABLE_BOX))
	or (invulnerability and hb.type == VULNERABILITY_BOX) then
		return
	end
	--if hb.left > hb.right or hb.bottom > hb.top then return end

	if DRAW_MINI_AXIS then
		gui.drawline(hb.hval, hb.vval-MINI_AXIS_SIZE, hb.hval, hb.vval+MINI_AXIS_SIZE, OR(fill[hb.type], 0xFF))
		gui.drawline(hb.hval-MINI_AXIS_SIZE, hb.vval, hb.hval+MINI_AXIS_SIZE, hb.vval, OR(fill[hb.type], 0xFF))
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, fill[hb.type], outline[hb.type])
end


local function draw_game_object(obj)
	if not obj or not obj.pos_x then return end
	
	local x = game_x_to_mame(obj.pos_x)
	local y = game_y_to_mame(obj.pos_y)
	gui.drawline(x, y-AXIS_SIZE, x, y+AXIS_SIZE, AXIS_COLOR)
	gui.drawline(x-AXIS_SIZE, y, x+AXIS_SIZE, y, AXIS_COLOR)
end


local function render_cps2_hitboxes()
	if not game or globals.game_phase == GAME_PHASE_NOT_PLAYING then
		return
	end

	if BLANK_SCREEN then
		gui.box(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLANK_COLOR)
	end

	for entry in ipairs(game.boxes) do
		for p = 1, game.number.players do
			local obj = frame_buffer[1][player][p]
			if obj and obj[entry] then
				draw_hitbox(obj[entry], obj.invulnerability)
			end
		end

		for i in ipairs(frame_buffer[1][projectiles]) do
			local obj = frame_buffer[1][projectiles][i]
			if obj[entry] then
				draw_hitbox(obj[entry])
			end
		end
	end

	if DRAW_AXIS then
		for p = 1, game.number.players do
			draw_game_object(frame_buffer[1][player][p])
		end
		for i in ipairs(frame_buffer[1][projectiles]) do
			draw_game_object(frame_buffer[1][projectiles][i])
		end
	end
end


gui.register( function()
	render_cps2_hitboxes()
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
				prepare_boxes = game.offset.id_ptr and prepare_id_offset_boxes or prepare_normal_boxes
				for p = 1, game.number.players do
					player[p] = {}
				end
				for f = 1, DRAW_DELAY + 1 do
					frame_buffer[f] = {}
					frame_buffer[f][player] = {}
					frame_buffer[f][projectiles] = {}
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