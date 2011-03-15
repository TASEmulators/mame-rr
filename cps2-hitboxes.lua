print("CPS-2 hitbox viewer")
print("March 14, 2011")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis")
print("Lua hotkey 4: toggle pushboxes")
print("Lua hotkey 5: toggle throwboxes") print()

local VULNERABILITY_COLOR      = 0x7777FF40
local ATTACK_COLOR             = 0xFF000060
local PROJ_VULNERABILITY_COLOR = 0x00FFFF40
local PROJ_ATTACK_COLOR        = 0xFF66FF60
local PUSH_COLOR               = 0x00FF0040
local THROW_COLOR              = 0xFFFF0060
local THROWABLE_COLOR          = 0xFFFFFF20
local AXIS_COLOR               = 0xFFFFFFFF
local BLANK_COLOR              = 0xFFFFFFFF
local AXIS_SIZE                = 16
local MINI_AXIS_SIZE           = 2
local DRAW_DELAY               = 1
local BLANK_SCREEN             = false
local DRAW_AXIS                = false
local DRAW_MINI_AXIS           = false
local DRAW_PUSHBOXES           = true
local DRAW_THROWBOXES          = true

local GAME_PHASE_NOT_PLAYING = 0
local VULNERABILITY_BOX      = 1
local ATTACK_BOX             = 2
local PROJ_VULNERABILITY_BOX = 3
local PROJ_ATTACK_BOX        = 4
local PUSH_BOX               = 5
local THROW_BOX              = 6
local THROWABLE_BOX          = 7

local fill = {
	VULNERABILITY_COLOR,
	ATTACK_COLOR,
	PROJ_VULNERABILITY_COLOR,
	PROJ_ATTACK_COLOR,
	PUSH_COLOR,
	THROW_COLOR,
	THROWABLE_COLOR,
}

local outline = {
	bit.bor(0xFF, VULNERABILITY_COLOR),
	bit.bor(0xFF, ATTACK_COLOR),
	bit.bor(0xFF, PROJ_VULNERABILITY_COLOR),
	bit.bor(0xFF, PROJ_ATTACK_COLOR),
	bit.bor(0xC0, PUSH_COLOR),
	bit.bor(0xFF, THROW_COLOR),
	bit.bor(0xFF, THROWABLE_COLOR),
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
		},
		box = {
			radius_read = memory.readbyte,
			offset_read = memory.readbytesigned,
			hval = 0x0, vval = 0x1, hrad = 0x2, vrad = 0x3,
		},
		box_list = {
			{anim_ptr = 0x20, addr_table_ptr = 0x08, p_addr_table_ptr = 0x4, id_ptr = 0x0C, id_space = 0x04, type = PUSH_BOX},
			{anim_ptr = 0x20, addr_table_ptr = 0x00, p_addr_table_ptr = 0x0, id_ptr = 0x08, id_space = 0x04, type = VULNERABILITY_BOX},
			{anim_ptr = 0x20, addr_table_ptr = 0x02, p_addr_table_ptr = 0x0, id_ptr = 0x09, id_space = 0x04, type = VULNERABILITY_BOX},
			{anim_ptr = 0x20, addr_table_ptr = 0x04, p_addr_table_ptr = 0x0, id_ptr = 0x0A, id_space = 0x04, type = VULNERABILITY_BOX},
			{anim_ptr = 0x20, addr_table_ptr = 0x06, p_addr_table_ptr = 0x2, id_ptr = 0x0B, id_space = 0x10, type = ATTACK_BOX},
		},
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
		},
		box_list = {
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
			invulnerability  = {0x67, 0xD6, 0x25D},
			unpushability    = {0x67},
		},
		box_list = {
			{anim_ptr =  nil, addr_table_ptr = 0x9C, id_ptr =  0xCB, id_space = 0x08, type = PUSH_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr =  0xC8, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x94, id_ptr =  0xC9, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x98, id_ptr =  0xCA, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0xA0, id_ptr =   0x9, id_space = 0x20, type = ATTACK_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0xA0, id_ptr =  0x82, id_space = 0x20, type = ATTACK_BOX, tripwire = 0x1E4},
			{anim_ptr =  nil, addr_table_ptr = 0xA0, id_ptr = 0x32F, id_space = 0x20, type = THROW_BOX, zero = true},
		},
		check_projectiles = true,
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
		},
		box = {hval = 0x0, vval = 0x4, hrad = 0x2, vrad = 0x6},
		box_list = {
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
		},
		box = {hval = 0x0, vval = 0x4, hrad = 0x2, vrad = 0x6},
		box_list = {
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
			invulnerability  = {0x134, 0x147},
			unpushability    = {0x134},
			friends          = 0xB2,
		},
		box_list = {
			{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr = 0x97, id_space = 0x08, type = PUSH_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x80, id_ptr = 0x94, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x84, id_ptr = 0x95, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x96, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table_ptr = 0x8C, id_ptr = 0x0A, id_space = 0x20, type = ATTACK_BOX},
			--{anim_ptr =  nil, addr_table_ptr = 0x8C, id_ptr = "?", id_space = 0x20, type = THROW_BOX},
		},
		check_projectiles = true,
	},
	{
		games = {"ringdest"},
		number = {players = 2, projectiles = 28},
		ground_level = 23,
		address = {
			player           = 0xFF8000,
			projectile       = 0xFF9000,
			left_screen_edge = 0xFF72D2,
			top_screen_edge  = 0xFF72D4,
			game_phase       = 0xFF72D2,
		},
		offset = {
			projectile_space = 0x100,
			facing_dir       = 0x38,
			id_ptr           = 0x4A,
			invulnerability  = {},
		},
		box = {
			radscale = 2,
			hval = 0x0, vval = 0x4, hrad = 0x2, vrad = 0x6,
		},
		box_list = {
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
		},
		box_list = {
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
		},
		box = {radscale = 2},
		box_list = {
			{anim_ptr =  nil, addr_table_ptr = 0x8C, id_ptr = 0x93, id_space = 0x08, type = PUSH_BOX},
			--{anim_ptr = 0x1C, addr_table_ptr = 0x8C, id_ptr = 0x0B, id_space = 0x08, type = THROWABLE_BOX}, --same as pushbox?
			{anim_ptr =  nil, addr_table_ptr = 0x80, id_ptr = 0x90, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x84, id_ptr = 0x91, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x92, id_space = 0x20, type = ATTACK_BOX},
			--{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = "?", id_space = 0x20, type = THROW_BOX},
		},
	},
}

for game in ipairs(profile) do
	local g = profile[game]
	if type(g.offset.hitbox_ptr) == "number" then
		local ptr = g.offset.hitbox_ptr
		g.offset.hitbox_ptr = {player = ptr, projectile = ptr}
	end
	g.ground_level = g.ground_level or -15
	g.address.top_screen_edge = g.address.top_screen_edge or g.address.left_screen_edge + 0x4
	g.offset.player_space  = g.offset.player_space  or 0x400
	g.offset.x_position    = g.offset.x_position    or 0x10
	g.offset.y_position    = g.offset.y_position    or g.offset.x_position + 0x4
	g.offset.id_ptr        = g.offset.id_ptr        or 0
	g.offset.hitbox_ptr    = g.offset.hitbox_ptr    or {}
	g.offset.unpushability = g.offset.unpushability or {}
	g.box = g.box or {}
	g.box.radius_read = g.box.radius_read or memory.readword
	g.box.offset_read = g.box.radius_read == memory.readword and memory.readwordsigned or memory.readbytesigned
	g.box.hval        = g.box.hval or 0x0
	g.box.vval        = g.box.vval or 0x2
	g.box.hrad        = g.box.hrad or 0x4
	g.box.vrad        = g.box.vrad or 0x6
	g.box.radscale    = g.box.radscale or 1
	g.special_projectiles = g.special_projectiles or {number = 0}
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
-- prepare the hitboxes

local function update_globals()
	globals.left_screen_edge = memory.readword(game.address.left_screen_edge)
	globals.top_screen_edge  = memory.readword(game.address.top_screen_edge)
	globals.game_phase       = memory.readword(game.address.game_phase)
end


local function get_x(x)
	return x - globals.left_screen_edge
end


local function get_y(y)
	return emu.screenheight() - (y + game.ground_level) + globals.top_screen_edge
end


local define_box = {
	[0] = function(obj, entry, hitbox_ptr)
		local box = {type = game.box_list[entry].type}
		if obj.projectile then
			if box.type == THROW_BOX then
				return nil
			elseif box.type == ATTACK_BOX then
				box.type = PROJ_ATTACK_BOX
			elseif box.type == VULNERABILITY_BOX then
				box.type = PROJ_VULNERABILITY_BOX
			end
		end

		local base_id = obj.base
		if game.box_list[entry].anim_ptr then
			base_id = memory.readdword(obj.base + game.box_list[entry].anim_ptr)
		end
		local curr_id = memory.readbyte(base_id + game.box_list[entry].id_ptr)

		if game.box_list[entry].zero then
			memory.writebyte(base_id + game.box_list[entry].id_ptr, 0x0)
		end

		local wire_pos = 0
		if game.box_list[entry].tripwire then
			curr_id = curr_id / 2 + 0x3E
			wire_pos = memory.readwordsigned(obj.base + game.box_list[entry].tripwire)
			if wire_pos == 0 or memory.readbyte(obj.base + 0x102) ~= 0xE then
				return nil
			elseif memory.readbyte(obj.base + 0x216) == 0x0 then
				memory.writeword(obj.base + game.box_list[entry].tripwire, 0x0)
			end
		end

		if curr_id == 0 then
			return nil
		end

		local addr_table
		if not hitbox_ptr then
			addr_table = memory.readdword(obj.base + game.box_list[entry].addr_table_ptr)
		else
			local table_offset = obj.projectile and game.box_list[entry].p_addr_table_ptr or game.box_list[entry].addr_table_ptr
			addr_table = memory.readdword(obj.base + hitbox_ptr)
			addr_table = addr_table + memory.readwordsigned(addr_table + table_offset)
		end
		box.address = addr_table + curr_id * game.box_list[entry].id_space

		box.hrad = game.box.radius_read(box.address + game.box.hrad)/game.box.radscale
		box.vrad = game.box.radius_read(box.address + game.box.vrad)/game.box.radscale
		if (box.hrad == 0 or box.vrad == 0) and wire_pos == 0 then
			return nil
		end
		box.hval = game.box.offset_read(box.address + game.box.hval)
		box.vval = game.box.offset_read(box.address + game.box.vval)

		box.hval   = obj.pos_x + box.hval * (obj.facing_dir == 1 and -1 or 1) + wire_pos
		box.vval   = obj.pos_y - box.vval
		box.left   = box.hval - box.hrad
		box.right  = box.hval + box.hrad
		box.top    = box.vval - box.vrad
		box.bottom = box.vval + box.vrad

		return box
	end,

	[0x4A] = function(obj, entry, id_offset) --for ringdest only
		local box = {type = game.box_list[entry].type}

		if box.type == PUSH_BOX then
			if obj.projectile or memory.readbyte(obj.base + 0x71) > 0 or
				(memory.readword(obj.base + 0x2C0) == 0xFF and memory.readword(obj.base + 0x2D2) < 0x8) then
				return nil
			end
		elseif obj.projectile and box.type == ATTACK_BOX then
			box.type = PROJ_ATTACK_BOX
		elseif obj.projectile and box.type == VULNERABILITY_BOX then
			box.type = PROJ_VULNERABILITY_BOX
		end

		if game.box_list[entry].addr_table_offset then
			box.address = game.box_list[entry].addr_table_offset + game.box_list[entry].id_space * id_offset
		else
			box.address = memory.readdword(obj.base + game.box_list[entry].addr_table_ptr)
		end

		box.hrad = game.box.radius_read(box.address + game.box.hrad)/game.box.radscale
		box.vrad = game.box.radius_read(box.address + game.box.vrad)/game.box.radscale
		if box.hrad == 0 or box.vrad == 0 then
			return nil
		end
		box.hval = game.box.offset_read(box.address + game.box.hval)
		box.vval = game.box.offset_read(box.address + game.box.vval)

		box.hval   = obj.pos_x + (box.hrad + box.hval) * (obj.facing_dir > 0 and -1 or 1)
		box.vval   = obj.pos_y - box.vrad - box.vval
		box.left   = box.hval - box.hrad
		box.right  = box.hval + box.hrad
		box.top    = box.vval - box.vrad
		box.bottom = box.vval + box.vrad

		return box
	end,
}


local prepare_boxes = {
	[0] = function(obj)
		local hitbox_ptr = obj.projectile and game.offset.hitbox_ptr.projectile or game.offset.hitbox_ptr.player
		for entry in ipairs(game.box_list) do
			obj[entry] = define_box[game.offset.id_ptr](obj, entry, hitbox_ptr)
		end
	end,

	[0x4A] = function(obj)
		local id_offset = memory.readword(obj.base + game.offset.id_ptr)
		for entry in ipairs(game.box_list) do
			obj[entry] = define_box[game.offset.id_ptr](obj, entry, id_offset)
		end
	end,
}


local function update_game_object(obj)
	obj.facing_dir = memory.readbyte(obj.base + game.offset.facing_dir)
	obj.pos_x      = get_x(memory.readwordsigned(obj.base + game.offset.x_position))
	obj.pos_y      = get_y(memory.readwordsigned(obj.base + game.offset.y_position))
	prepare_boxes[game.offset.id_ptr](obj)
end


local function read_projectiles()
	local current_projectiles = {}

	for i = 1, game.number.projectiles do
		local obj = {base = game.address.projectile + (i-1) * game.offset.projectile_space}
		if memory.readword(obj.base) > 0x0100 and
		(not game.check_projectiles or memory.readbyte(obj.base + 0x04) <= 0x02) then
			obj.projectile = true
			if game.offset.friends and memory.readbyte(obj.base + game.offset.friends) > 0 then
				obj.invulnerability, obj.unpushability = true, true
			end
			update_game_object(obj)
			table.insert(current_projectiles, obj)
		end
	end

	for i = 1, game.special_projectiles.number do --for nwarr only
		local obj = {base = game.special_projectiles.start + (i-1) * game.special_projectiles.space}
		local status = memory.readbyte(obj.base + 0x04)
		if status == 0x02 or status == 0x04 then
			obj.projectile, obj.invulnerability, obj.unpushability = true, true, true
			update_game_object(obj)
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


local function update_unpushability(player)
	player.unpushability = false
	for _,address in ipairs(game.offset.unpushability) do
		if memory.readbyte(player.base + address) > 0 then
			player.unpushability = true
		end
	end
end


local function update_cps2_hitboxes()
	if not game then
		return
	end
	update_globals()

	for p = 1, game.number.players do
		player[p] = {base = game.address.player + (p-1) * game.offset.player_space}
		if memory.readbyte(player[p].base) > 0 then
			update_game_object(player[p])
			update_invulnerability(player[p])
			update_unpushability(player[p])
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

local function draw_hitbox(obj, entry)
	local hb = obj[entry]
	if (not DRAW_PUSHBOXES and hb.type == PUSH_BOX)
	or (not DRAW_THROWBOXES and (hb.type == THROW_BOX or hb.type == THROWABLE_BOX))
	or (obj.invulnerability and hb.type == VULNERABILITY_BOX)
	or (obj.unpushability and hb.type == PUSH_BOX) then
		return
	end

	if DRAW_MINI_AXIS then
		gui.drawline(hb.hval, hb.vval-MINI_AXIS_SIZE, hb.hval, hb.vval+MINI_AXIS_SIZE, outline[hb.type])
		gui.drawline(hb.hval-MINI_AXIS_SIZE, hb.vval, hb.hval+MINI_AXIS_SIZE, hb.vval, outline[hb.type])
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, fill[hb.type], outline[hb.type])
end


local function draw_game_object(obj)
	if not obj or not obj.pos_x then
		return
	end
	
	gui.drawline(obj.pos_x, obj.pos_y-AXIS_SIZE, obj.pos_x, obj.pos_y+AXIS_SIZE, AXIS_COLOR)
	gui.drawline(obj.pos_x-AXIS_SIZE, obj.pos_y, obj.pos_x+AXIS_SIZE, obj.pos_y, AXIS_COLOR)
	--gui.text(obj.pos_x, obj.pos_y, string.format("%06X",obj.base)) --debug
end


local function render_cps2_hitboxes()
	gui.clearuncommitted()
	if not game or globals.game_phase == GAME_PHASE_NOT_PLAYING then
		return
	end

	if BLANK_SCREEN then
		gui.box(0, 0, emu.screenwidth(), emu.screenheight(), BLANK_COLOR)
	end

	for entry in ipairs(game.box_list) do
		for p = 1, game.number.players do
			local obj = frame_buffer[1][player][p]
			if obj and obj[entry] then
				draw_hitbox(obj, entry)
			end
		end

		for i in ipairs(frame_buffer[1][projectiles]) do
			local obj = frame_buffer[1][projectiles][i]
			if obj[entry] then
				draw_hitbox(obj, entry)
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
-- hotkey functions

input.registerhotkey(1, function()
	BLANK_SCREEN = not BLANK_SCREEN
	render_cps2_hitboxes()
	print((BLANK_SCREEN and "activated" or "deactivated") .. " blank screen mode")
end)


input.registerhotkey(2, function()
	DRAW_AXIS = not DRAW_AXIS
	render_cps2_hitboxes()
	print((DRAW_AXIS and "showing" or "hiding") .. " object axis")
end)


input.registerhotkey(3, function()
	DRAW_MINI_AXIS = not DRAW_MINI_AXIS
	render_cps2_hitboxes()
	print((DRAW_MINI_AXIS and "showing" or "hiding") .. " hitbox axis")
end)


input.registerhotkey(4, function()
	DRAW_PUSHBOXES = not DRAW_PUSHBOXES
	render_cps2_hitboxes()
	print((DRAW_PUSHBOXES and "showing" or "hiding") .. " pushboxes")
end)


input.registerhotkey(5, function()
	DRAW_THROWBOXES = not DRAW_THROWBOXES
	render_cps2_hitboxes()
	print((DRAW_THROWBOXES and "showing" or "hiding") .. " throwboxes")
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