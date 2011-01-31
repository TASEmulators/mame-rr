print("Capcom beat 'em up hitbox viewer")
print("January 30, 2011")
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

--invulnerable: if > 0, don't draw blue
--alive: if == 0, don't draw blue
--hp: if < 0, don't draw blue
--harmless: never draw red
--active: if none == 0, don't draw red

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
			{address = 0xFF8568, number = 0x1D, space = 0xC0, hp = 0x18}, --players and enemies
			{address = 0xFFB2E8, number = 0x10, space = 0xC0, hp = 0x18}, --containers
			{address = 0xFFBEE8, number = 0x0A, space = 0xC0}, --items
		},
		boxes = {
			{anim_ptr = 0x24, addr_table = 0x02, id_ptr = 0x04, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x24, addr_table = 0x00, id_ptr = 0x05, id_space = 0x10, type = ATTACK_BOX},
		},
		id_read   = memory.readbytesigned,
		box_read  = memory.readwordsigned,
		radius    = 2,
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
			{address = 0xFFA994, number = 0x04, space = 0x100}, --players
			{address = 0xFFAD94, number = 0x50, space = 0x0C0}, --etc
		},
		boxes = {
			--{anim_ptr = 0x1C, addr_table = 0x00, id_ptr = 0x04, id_space = 0x08, type = VULNERABILITY_BOX},
			--{anim_ptr = 0x1C, addr_table = 0x00, id_ptr = 0x05, id_space = 0x10, type = ATTACK_BOX},
		},
		id_read   = memory.readbyte,--?
		box_read  = memory.readwordsigned,--?
		radius    = 2,--?
	},
	{
		games = {"dino"},
		address = {
			left_screen_edge = 0xFF8744,
			game_phase       = 0xFF0A4B,
			special_table    = 0x106000,
		},
		offset = {
			facing_dir       = 0x24,
			x_position       = 0x08,
			z_position       = 0x10,
			hitbox_ptr       = 0x44,
			hval = 0x4, vval = 0x8, hrad = 0x6, vrad = 0xA,
		},
		objects = {
			{address = 0xFF8874, number = 0x18, space = 0x0C0, active = {0x14,0x16,0x4C,0xB0}}, --items
			{address = 0xFFB274, number = 0x03, space = 0x180, alive = 0x6C, invulnerable = 0x118}, --players
			{address = 0xFFB6F4, number = 0x18, space = 0x0C0, alive = 0x6C}, --etc
			{address = 0xFFC8F4, number = 0x18, space = 0x0E0, alive = 0x6C}, --enemies
		},
		boxes = {
			{anim_ptr = nil, addr_table = nil, id_ptr = 0x48, id_space = 0x0C, type = VULNERABILITY_BOX},
			{anim_ptr = nil, addr_table = nil, id_ptr = 0x49, id_space = 0x0C, type = ATTACK_BOX},
			{anim_ptr = nil, addr_table = nil, id_ptr = 0x48, id_space = 0x02, type = VULNERABILITY_BOX, special_offset = 0x2},
			{anim_ptr = nil, addr_table = nil, id_ptr = 0x48, id_space = 0x02, type = VULNERABILITY_BOX, special_offset = 0xE},
			{anim_ptr = nil, addr_table = nil, id_ptr = 0x49, id_space = 0x02, type = ATTACK_BOX, special_offset = 0x2},
		},
		id_read   = memory.readbyte,
		box_read  = memory.readwordsigned,
		radius    = 1,
		exist_val = 0x0101,
	},
	{
		games = {"punisher"},
		address = {
			camera_mode      = 0xFF5BD3,
			left_screen_edge = {[1] = 0xFF7376, [88] = 0xFF747C},
			game_phase       = 0xFF4E81,
		},
		offset = {
			facing_dir       = 0x07,
			x_position       = 0x20,
			z_position       = 0x28,
			hitbox_ptr       = 0x30,
			hval = 0x4, vval = 0x8, hrad = 0x6, vrad = 0xA,
		},
		objects = {
			{address = 0xFF8E68, number = 0x02, space = 0x100}, --players
			{address = 0xFF9068, number = 0x20, space = 0x0C0}, --shots
			{address = 0xFFA868, number = 0x10, space = 0x0C0, hp = 0x36, alive = 0x16, active = 0x1A}, --enemies
			{address = 0xFFB468, number = 0x19, space = 0x0C0, active = {0x1C,0x86}}, --big items
			{address = 0xFFC728, number = 0x38, space = 0x0C0, alive = 0x0A}, --shots & small items
		},
		boxes = {
			{anim_ptr = nil, addr_table = nil, id_ptr = 0x3E, id_space = 0x01, type = VULNERABILITY_BOX},
			{anim_ptr = nil, addr_table = nil, id_ptr = 0x3C, id_space = 0x01, type = ATTACK_BOX},
		},
		id_read   = memory.readword,
		box_read  = memory.readwordsigned,
		radius    = 1,
		laser     = {offset = 0x1A, id = 0x2C, box_count = 0x0, dx = 0x2, dy = 0x4},
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
			{address = 0xFF8380, number = 0x03, space = 0x100}, --players
			{address = 0xFF8680, number = 0x18, space = 0x080}, --player projectiles
			{address = 0xFF9280, number = 0x18, space = 0x0C0, hp = 0x40}, --enemies
			{address = 0xFFA480, number = 0x18, space = 0x080}, --enemy projectiles
			{address = 0xFFB080, number = 0x3C, space = 0x080, harmless = true}, --etc.
		},
		boxes = {
			{anim_ptr = 0x1C, addr_table = 0x00, id_ptr = 0x08, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x1C, addr_table = 0x02, id_ptr = 0x09, id_space = 0x10, type = ATTACK_BOX},
		},
		id_read   = memory.readbyte,
		box_read  = memory.readbytesigned,
		radius    = 2,
	},
	{
		games = {"ddtod"},
		address = {
			left_screen_edge = 0xFF8630,
			game_phase       = 0xFF805F,
		},
		offset = {
			facing_dir       = 0x3C,
			x_position       = 0x08,
			z_position       = 0x10,
			hitbox_ptr       = nil,
			hval = 0x4, vval = 0x8, hrad = 0x6, vrad = 0xA,
		},
		objects = {
			{address = 0xFF86C8, number = 0x04, space = 0x100}, --players
			{address = 0xFF8AC8, number = 0x18, space = 0x060}, --player projectiles
			{address = 0xFFA5C8, number = 0x18, space = 0x0C0, alive = 0x62}, --enemies
			{address = 0xFFB7C8, number = 0x20, space = 0x060}, --enemy projectiles
			{address = 0xFFDFC8, number = 0x10, space = 0x080, alive = 0x62}, --items
			{address = 0xFFE7C8, number = 0x10, space = 0x080}, --chests, signs
		},
		boxes = {
			{anim_ptr = nil, addr_table = 0x13050A, id_ptr = 0x22, id_space = 0x01, type = VULNERABILITY_BOX},
			{anim_ptr = nil, addr_table = 0x13164A, id_ptr = 0x24, id_space = 0x01, type = ATTACK_BOX},
		},
		base_offset = {
			[0x000] = {"ddtodj"}, --940412
			[0x130] = {"ddtodjr2"}, --940113
			[0x138] = {"ddtodjr1"}, --940125
			[0x86C] = {"ddtodh"}, --940412
			[0x902] = {"ddtodur1"}, --940113
			[0x90A] = {"ddtodu"}, --940125
			[0x936] = {"ddtoda"}, --940113
			[0x948] = {"ddtod","ddtodd"}, --940412
			[0x99C] = {"ddtodhr2"}, --940113
			[0x9A4] = {"ddtodhr1"}, --940125
			[0xA78] = {"ddtodr1"}, --940113
		},
		id_read   = memory.readword,
		box_read  = memory.readwordsigned,
		radius    = 2,
	},
	{
		games = {"ddsom"},
		address = {
			camera_mode      = 0xFF8036,
			left_screen_edge = {[0] = 0xFF8506, [1] = 0xFF8556},
			game_phase       = 0xFF8043,
		},
		offset = {
			facing_dir       = 0x2C,
			x_position       = 0x08,
			z_position       = 0x10,
			hitbox_ptr       = nil,
			char_id          = 0x2E,
			hval = 0x0, vval = 0x2, hrad = 0x1, vrad = 0x3,
		},
		objects = {
			{address = 0xFF85DE, number = 0x04, space = 0x200, invulnerable = 0x196}, --players
			{address = 0xFF90DE, number = 0x10, space = 0x060, addr_table = 0x11EFA2}, --player projectiles
			{address = 0xFFA99E, number = 0x18, space = 0x0C0, alive = 0x62}, --enemies
			{address = 0xFFBC5E, number = 0x20, space = 0x060}, --enemy projectiles
			--{address = 0xFFE2DE, number = 0x10, space = 0x060}, --items
			{address = 0xFFEEDE, number = 0x10, space = 0x080}, --chests, signs
		},
		boxes = {
			{anim_ptr = nil, addr_table = 0x118FD4, id_ptr = 0x38, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = nil, addr_table = 0x119214, id_ptr = 0x3A, id_space = 0x08, type = ATTACK_BOX},
		},
		base_offset = {
			[0x0000] = {"ddsomh","ddsomb"}, --960223
			[0x1952] = {"ddsomjr1"}, --960206
			[0x1D18] = {"ddsomur1"}, --960209
			[0x215A] = {"ddsoma","ddsomj","ddsomu","ddsomud"}, --960619
			[0x2AAA] = {"ddsomr3"}, --960208
			[0x2BA8] = {"ddsomr2"}, --960209
			[0x301A] = {"ddsomr1"}, --960223
			[0x3026] = {"ddsom"}, --960619
		},
		id_read   = memory.readbyte,
		box_read  = memory.readbytesigned,
		radius    = 2,
	},
	{
		games = {"batcir"},
		address = {
			left_screen_edge = 0xFF5840,
			game_phase       = 0xFF5997,
		},
		offset = {
			facing_dir       = 0x07,
			x_position       = 0x20,
			z_position       = 0x28,
			hitbox_ptr       = nil,
			hval = 0x4, vval = 0x8, hrad = 0x6, vrad = 0xA,
		},
		objects = {
			{address = 0xFF8268, number = 0x04, space = 0x140, invulnerable = 0xA4}, --players
			{address = 0xFF8768, number = 0x08, space = 0x0C0}, --player projectiles
			{address = 0xFF9F68, number = 0x14, space = 0x0C0, hp = 0x82}, --enemies
			{address = 0xFFAE68, number = 0x10, space = 0x0C0}, --enemy projectiles
			{address = 0xFFBA68, number = 0x10, space = 0x0C0}, --containers
			{address = 0xFFC668, number = 0x20, space = 0x0C0}, --enemy projectiles
			{address = 0xFFDE68, number = 0x08, space = 0x0C0}, --items
			{address = 0xFFE468, number = 0x20, space = 0x0C0}, --items
		},
		boxes = {
			{anim_ptr = nil, addr_table = 0x370000, id_ptr = 0x40, id_space = 0x01, type = VULNERABILITY_BOX},
			{anim_ptr = nil, addr_table = 0x370000, id_ptr = 0x42, id_space = 0x01, type = ATTACK_BOX},
		},
		base_offset = {
			[0] = {"batcir","batcird","batcirdj","batcirdja"} --970319
		},
		id_read   = memory.readword,
		box_read  = memory.readwordsigned,
		radius    = 1,
	},
}

for game in ipairs(profile) do
	local g = profile[game]
	g.rad_read = g.box_read == memory.readbytesigned and memory.readbyte or memory.readword
	g.offset.y_position = g.offset.y_position or g.offset.x_position + 0x4
	g.exist_val = g.exist_val or 0x0100
	for entry in ipairs(g.objects) do
		if type(g.objects[entry].active) == "number" then
			g.objects[entry].active = {g.objects[entry].active}
		end
	end
	if type(g.address.left_screen_edge) ~= "table" then
		g.address.left_screen_edge = {g.address.left_screen_edge}
	end
end

local game, base_offset
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

local function whatversion(game)
	for base,version_set in pairs(game.base_offset) do
		for _,version in ipairs(version_set) do
			if emu.romname() == version then
				return base
			end
		end
	end
	print("unrecognized version (" .. emu.romname() .. "): cannot draw boxes")
	return nil
end


local function whatgame()
	game = nil
	for n, module in ipairs(profile) do
		for m, shortname in ipairs(module.games) do
			if emu.romname() == shortname or emu.parentname() == shortname then
				print("drawing " .. emu.romname() .. " hitboxes")
				game = module
				for f = 1, DRAW_DELAY + 1 do
					frame_buffer[f] = {}
				end
				if game.base_offset then
					base_offset = whatversion(game)
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
	local camera_mode = game.address.camera_mode and memory.readbyte(game.address.camera_mode) or 1
	if not game.address.left_screen_edge[camera_mode] then camera_mode = 1 end
	globals.left_screen_edge = memory.readwordsigned(game.address.left_screen_edge[camera_mode])
	globals.top_screen_edge  = memory.readwordsigned(game.address.left_screen_edge[camera_mode] + 0x4)
	globals.game_phase       = memory.readbyte(game.address.game_phase)
end


local function game_x_to_mame(x)
	return x - globals.left_screen_edge
end


local function game_y_to_mame(y)
	return SCREEN_HEIGHT - (y - 15) + globals.top_screen_edge
end


local read_box_vals = {
	function(obj, entry, hval, vval, hrad, vrad)
		hval = hval * obj.facing_dir
		hrad = hrad * obj.facing_dir

		return {
			left   = obj.pos_x + hval,
			right  = obj.pos_x + hval + hrad,
			top    = obj.pos_y - vval - vrad,
			bottom = obj.pos_y - vval,
			hval   = obj.pos_x + hval + hrad/2,
			vval   = obj.pos_y - vval - vrad/2,
			type   = game.boxes[entry].type,
		}
	end,

	function(obj, entry, hval, vval, hrad, vrad)
		hval = hval * obj.facing_dir

		return {
			left   = obj.pos_x + hval - hrad,
			right  = obj.pos_x + hval + hrad,
			top    = obj.pos_y - vval - vrad,
			bottom = obj.pos_y - vval + vrad,
			hval   = obj.pos_x + hval,
			vval   = obj.pos_y - vval,
			type   = game.boxes[entry].type,
		}
	end,
}


local function define_box(obj, entry, space)
	if (obj.invulnerable and game.boxes[entry].type == VULNERABILITY_BOX) or
		(obj.harmless and game.boxes[entry].type == ATTACK_BOX) then
		return nil
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
	
	local addr_table, address
	if obj.addr_table then --ddsom player projectiles
		addr_table = obj.addr_table + base_offset
	elseif game.offset.char_id then --ddsom other
		addr_table = memory.readdword(game.boxes[entry].addr_table + base_offset + memory.readbyte(obj.base + game.offset.char_id) * 4)
	elseif not game.offset.hitbox_ptr then --ddtod, batcir
		addr_table = game.boxes[entry].addr_table + base_offset
	else
		addr_table = memory.readdword(obj.base + game.offset.hitbox_ptr)
		if game.boxes[entry].addr_table then
			addr_table = addr_table + memory.readwordsigned(addr_table + game.boxes[entry].addr_table)
		elseif game.laser and memory.readword(obj.base + game.laser.offset) == game.laser.id and game.boxes[entry].type == ATTACK_BOX then --punisher laser
			addr_table = addr_table + curr_id * game.boxes[entry].id_space
			return {
				dir       = obj.facing_dir,
				init_x    = obj.pos_x,
				init_y    = obj.pos_y,
				box_count = game.box_read(addr_table + game.laser.box_count),
				dx        = game.box_read(addr_table + game.laser.dx),
				dy        = game.box_read(addr_table + game.laser.dy),
				type      = game.boxes[entry].type,
			}
		elseif game.address.special_table then --dino
			if (game.boxes[entry].special_offset and addr_table ~= game.address.special_table) or
				(not game.boxes[entry].special_offset and addr_table == game.address.special_table) then
				return
			elseif game.boxes[entry].special_offset and addr_table == game.address.special_table then
				addr_table = addr_table + memory.readword(addr_table + curr_id * game.boxes[entry].id_space)
				if memory.readword(addr_table) == 0 and game.boxes[entry].special_offset > 0x2 then
					return
				end
				address = addr_table + game.boxes[entry].special_offset --dinosaurs
			end
		end
	end
	address = address or addr_table + curr_id * game.boxes[entry].id_space

	local hval = game.box_read(address + game.offset.hval)
	local vval = game.box_read(address + game.offset.vval)
	local hrad = game.rad_read(address + game.offset.hrad)
	local vrad = game.rad_read(address + game.offset.vrad)

	return read_box_vals[game.radius](obj, entry, hval, vval, hrad, vrad)
end


local function update_game_object(obj, space)
	obj.facing_dir = memory.readbyte(obj.base + game.offset.facing_dir) > 0 and -1 or 1
	obj.pos_z      = game.offset.z_position and memory.readwordsigned(obj.base + game.offset.z_position) or 0
	obj.pos_x      = game_x_to_mame(memory.readwordsigned(obj.base + game.offset.x_position))
	obj.pos_y      = game_y_to_mame(memory.readwordsigned(obj.base + game.offset.y_position) + obj.pos_z)

	for entry in ipairs(game.boxes) do
		obj[entry] = define_box(obj, entry, space)
	end
end

local function inactive(base, active)
	for _,offset in ipairs(active) do
		if memory.readword(base + offset) > 0 then
			return false
		end
	end
	return true
end


local function update_beatemup_hitboxes()
	gui.clearuncommitted()
	if not game then
		return
	end
	update_globals()

	local objects = {}
	for _,set in ipairs(game.objects) do
		for n = 1, set.number do
			local obj = {base = set.address + (n-1) * set.space}
			if memory.readwordsigned(obj.base) >= game.exist_val then
				obj.invulnerable = (set.hp and memory.readwordsigned(obj.base + set.hp) < 0) or
					(set.alive and memory.readword(obj.base + set.alive) == 0) or
					(set.invulnerable and memory.readword(obj.base + set.invulnerable) > 0)
				obj.harmless = set.harmless or (set.active and inactive(obj.base, set.active))
				obj.addr_table = set.addr_table
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
	if hb.box_count then --punisher laser
		for n = 1, hb.box_count do
			gui.box(hb.init_x + hb.dx * (n-1) * hb.dir, hb.init_y - hb.dy * (n-1), 
				hb.init_x + hb.dx * n * hb.dir, hb.init_y - hb.dy * n, fill[hb.type], outline[hb.type])
		end
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
	if not (obj and obj.pos_x) then
		return
	end
	
	gui.drawline(obj.pos_x, obj.pos_y-AXIS_SIZE, obj.pos_x, obj.pos_y+AXIS_SIZE, AXIS_COLOR)
	gui.drawline(obj.pos_x-AXIS_SIZE, obj.pos_y, obj.pos_x+AXIS_SIZE, obj.pos_y, AXIS_COLOR)
	--gui.text(obj.pos_x, obj.pos_y, string.format("%06X",obj.base)) --debug
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
			if obj[entry] then
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