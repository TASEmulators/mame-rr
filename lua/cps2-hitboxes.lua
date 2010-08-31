print("CPS-2 hitbox viewer")
print("August 31, 2010")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis") print()

local VULNERABILITY_COLOR   = 0x0000FF40
local ATTACK_COLOR          = 0xFF000060
local PUSH_COLOR            = 0x00FF0060
local WEAK_COLOR            = 0xFFFF0060
local AXIS_COLOR            = 0xFFFFFFFF
local BLANK_COLOR           = 0xFFFFFFFF
local AXIS_SIZE             = 16
local MINI_AXIS_SIZE        = 2
local DRAW_DELAY            = 2

local SCREEN_WIDTH          = 384
local SCREEN_HEIGHT         = 224
local MAX_GAME_PROJECTILES  = 16
local GAME_PHASE_NOT_PLAYING= 0
local DRAW_AXIS             = false
local DRAW_MINI_AXIS        = false
local BLANK_SCREEN          = false

local profile = {
	{
		games = {"sfa2","sfz2al"},
		nplayers = 3,
		address = {
			player           = 0xFF8400,
			projectile       = 0xFF9900,
			left_screen_edge = 0xFF8026,
			top_screen_edge  = 0xFF8028,
			game_phase       = 0xFF812D,
		},
		offset = {
			player_space     = 0x400,
			projectile_space = 0x80,
			facing_dir       = 0xB,
			hitbox_ptr       = 0x60,
			invulnerability  = {},
			hval = 0x0, vval = 0x2, hrad = 0x4, vrad = 0x6,
		},
		boxes = {
			{anim_ptr = 0x1C, addr_table = 0x120, p_addr_table = 0x4, id_ptr = 0x0C, id_space = 0x08, color = PUSH_COLOR},
			{anim_ptr = 0x1C, addr_table = 0x110, p_addr_table = 0x0, id_ptr = 0x08, id_space = 0x08, color = VULNERABILITY_COLOR},
			{anim_ptr = 0x1C, addr_table = 0x114, p_addr_table = 0x0, id_ptr = 0x09, id_space = 0x08, color = VULNERABILITY_COLOR},
			{anim_ptr = 0x1C, addr_table = 0x118, p_addr_table = 0x0, id_ptr = 0x0A, id_space = 0x08, color = VULNERABILITY_COLOR},
			{anim_ptr = 0x1C, addr_table = 0x11C, p_addr_table = 0x2, id_ptr = 0x0B, id_space = 0x20, color = ATTACK_COLOR},
		},
	},
	{
		games = {"sfa3"},
		nplayers = 4,
		address = {
			player           = 0xFF8400,
			projectile       = 0xFF9D00,
			left_screen_edge = 0xFF8026,
			top_screen_edge  = 0xFF8028,
			game_phase       = 0xFF812D,
		},
		offset = {
			player_space     = 0x400,
			projectile_space = 0x100,
			facing_dir       = 0xB,
			hitbox_ptr       = nil,
			invulnerability  = {0xD6,0x25D},
			hval = 0x0, vval = 0x2, hrad = 0x4, vrad = 0x6,
		},
		boxes = {
			{anim_ptr =  nil, addr_table = 0x9C, id_ptr = 0xCB, id_space = 0x08, color = PUSH_COLOR},
			{anim_ptr =  nil, addr_table = 0x90, id_ptr = 0xC8, id_space = 0x08, color = VULNERABILITY_COLOR},
			{anim_ptr =  nil, addr_table = 0x94, id_ptr = 0xC9, id_space = 0x08, color = VULNERABILITY_COLOR},
			{anim_ptr =  nil, addr_table = 0x98, id_ptr = 0xCA, id_space = 0x08, color = VULNERABILITY_COLOR},
			{anim_ptr = 0x1C, addr_table = 0xA0, id_ptr = 0x09, id_space = 0x20, color = ATTACK_COLOR},
		},
	},
	{
		games = {"nwarr"},
		nplayers = 2,
		address = {
			player           = 0xFF8388,
			projectile       = 0xFFA86E,
			left_screen_edge = 0xFF8F18,
			top_screen_edge  = 0xFF8F1C,
			game_phase       = 0xFF988B,
			special_projectiles = {0xFFA06E, 0xFFA76E}
		},
		offset = {
			player_space     = 0x500,
			projectile_space = 0xC0,
			facing_dir       = 0x9,
			hitbox_ptr       = 0x5C,
			invulnerability  = {},
			hval = 0x0, vval = 0x4, hrad = 0x2, vrad = 0x6,
		},
		boxes = {
			{anim_ptr = 0x1C, addr_table = 0x0A, id_ptr = 0x15, id_space = 0x08, color = PUSH_COLOR},
			{anim_ptr = 0x1C, addr_table = 0x00, id_ptr = 0x10, id_space = 0x08, color = VULNERABILITY_COLOR},
			{anim_ptr = 0x1C, addr_table = 0x02, id_ptr = 0x11, id_space = 0x08, color = VULNERABILITY_COLOR},
			{anim_ptr = 0x1C, addr_table = 0x04, id_ptr = 0x12, id_space = 0x08, color = VULNERABILITY_COLOR},
			{anim_ptr = 0x1C, addr_table = 0x08, id_ptr = 0x14, id_space = 0x10, color = ATTACK_COLOR},
			{anim_ptr = 0x1C, addr_table = 0x06, id_ptr = 0x13, id_space = 0x08, color = WEAK_COLOR},
		},
	},
	{
		games = {"vsav","vhunt2","vsav2"},
		nplayers = 2,
		address = {
			player           = 0xFF8400,
			projectile       = 0xFF9400,
			left_screen_edge = 0xFF8290,
			top_screen_edge  = 0xFF8028,
			game_phase       = 0xFF812D,
		},
		offset = {
			player_space     = 0x400,
			projectile_space = 0x100,
			facing_dir       = 0xB,
			hitbox_ptr       = nil,
			invulnerability  = {0x147},
			hval = 0x0, vval = 0x2, hrad = 0x4, vrad = 0x6,
		},
		boxes = {
			{anim_ptr =  nil, addr_table = 0x90, id_ptr = 0x97, id_space = 0x08, color = PUSH_COLOR},
			{anim_ptr =  nil, addr_table = 0x80, id_ptr = 0x94, id_space = 0x08, color = VULNERABILITY_COLOR},
			{anim_ptr =  nil, addr_table = 0x84, id_ptr = 0x95, id_space = 0x08, color = VULNERABILITY_COLOR},
			{anim_ptr =  nil, addr_table = 0x88, id_ptr = 0x96, id_space = 0x08, color = VULNERABILITY_COLOR},
			{anim_ptr = 0x1C, addr_table = 0x8C, id_ptr = 0x0A, id_space = 0x20, color = ATTACK_COLOR},
		},
	},
}

local game
local globals = {
	game_phase       = 0,
	left_screen_edge = 0,
	top_screen_edge  = 0,
	num_projectiles  = 0,
	player           = {},
}
local player      = {}
local projectiles = {}
local frame_buffer_array = {}
if mame ~= nil then DRAW_DELAY = DRAW_DELAY - 1 end


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
				for p = 1, game.nplayers do
					player[p] = {}
				end
				for f = 1, DRAW_DELAY + 1 do
					frame_buffer_array[f] = {}
					frame_buffer_array[f][player] = {}
					frame_buffer_array[f][projectiles] = {}
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
	globals.left_screen_edge = memory.readword(game.address.left_screen_edge)
	globals.top_screen_edge  = memory.readword(game.address.top_screen_edge)
	globals.game_phase       = memory.readword(game.address.game_phase)
	for p = 1, game.nplayers do
		globals.player[p]      = memory.readbyte(game.address.player + (p-1) * game.offset.player_space)
	end
end


local function define_box(obj, entry, base_obj, is_projectile)
	for _,address in ipairs(game.offset.invulnerability) do
		if memory.readbyte(base_obj + address) > 0 then
			obj[entry] = nil
			return
		end
	end

	local addr_table
	if not game.offset.hitbox_ptr or (game.boxes[entry].p_addr_table and not is_projectile) then --vsav/sfa3 all; sfa2 players
		addr_table = memory.readdword(base_obj + game.boxes[entry].addr_table)
	elseif game.offset.hitbox_ptr and game.boxes[entry].p_addr_table then --sfa2 projectiles
		addr_table = memory.readdword(base_obj + game.offset.hitbox_ptr)
		addr_table = addr_table + memory.readwordsigned(addr_table + game.boxes[entry].p_addr_table)
	else --nwarr all
		addr_table = memory.readdword(base_obj + game.offset.hitbox_ptr)
		addr_table = addr_table + memory.readwordsigned(addr_table + game.boxes[entry].addr_table)
	end

	local base_id = base_obj
	if game.boxes[entry].anim_ptr then
		base_id = memory.readdword(base_obj + game.boxes[entry].anim_ptr)
	end

	local curr_id = memory.readbyte(base_id + game.boxes[entry].id_ptr)
	local address = addr_table + curr_id * game.boxes[entry].id_space

	local hval = memory.readwordsigned(address + game.offset.hval)
	local vval = memory.readwordsigned(address + game.offset.vval)
	local hrad = memory.readwordsigned(address + game.offset.hrad)
	local vrad = memory.readwordsigned(address + game.offset.vrad)

	if obj.facing_dir == 1 then
		hval  = -hval
	end

	obj[entry] = {
		left   = obj.pos_x + hval - hrad,
		right  = obj.pos_x + hval + hrad,
		bottom = obj.pos_y + vval + vrad,
		top    = obj.pos_y + vval - vrad,
		hval   = obj.pos_x + hval,
		vval   = obj.pos_y + vval,
	}
end


local function update_game_object(obj, base, is_projectile)
	obj.facing_dir   = memory.readbyte(base + game.offset.facing_dir)
	obj.pos_x        = memory.readword(base + 0x10)
	obj.pos_y        = memory.readword(base + 0x14)
	--obj.opponent_dir = memory.readbyte(base + 0x5D)

	for n in ipairs(game.boxes) do
		define_box(obj, n, base, is_projectile)
	end
end


local function read_projectiles()
	globals.num_projectiles = 0
	for i = 1,MAX_GAME_PROJECTILES do
		local base_obj = game.address.projectile + (i-1) * game.offset.projectile_space

		if memory.readbyte(base_obj+1) ~= 0 then
			projectiles[globals.num_projectiles] = {}
			update_game_object(projectiles[globals.num_projectiles], base_obj, true)
			globals.num_projectiles = globals.num_projectiles+1
		end
	end

	if game.address.special_projectiles then
		for _,base_obj in ipairs(game.address.special_projectiles) do
			if memory.readbyte(base_obj+1) ~= 0 then
				projectiles[globals.num_projectiles] = {}
				update_game_object(projectiles[globals.num_projectiles], base_obj, true)
				globals.num_projectiles = globals.num_projectiles+1
			end
		end
	end
end


local function update_cps2_hitboxes()
	if not game then return end
	update_globals()
	if globals.game_phase == GAME_PHASE_NOT_PLAYING then
		return
	end

	for p = 1, game.nplayers do
		if globals.player[p] > 0 then
			update_game_object(player[p], game.address.player + (p-1) * game.offset.player_space)
		end
	end
	read_projectiles()

	for f = 1, DRAW_DELAY do
		for p = 1, game.nplayers do
			frame_buffer_array[f][player][p] = copytable(frame_buffer_array[f+1][player][p])
		end
		for i = 0, globals.num_projectiles-1 do
			frame_buffer_array[f][projectiles][i] = copytable(frame_buffer_array[f+1][projectiles][i])
		end
	end

	for p = 1, game.nplayers do
		frame_buffer_array[DRAW_DELAY+1][player][p] = copytable(player[p])
	end
	for i = 0, globals.num_projectiles-1 do
		frame_buffer_array[DRAW_DELAY+1][projectiles][i] = copytable(projectiles[i])
	end
end


emu.registerafter( function()
	update_cps2_hitboxes()
end)


--------------------------------------------------------------------------------
-- draw the hitboxes

local function game_x_to_mame(x)
	return (x - globals.left_screen_edge)
end


local function game_y_to_mame(y)
	-- Why subtract 17? No idea, the game driver does the same thing.
	return (SCREEN_HEIGHT - (y - 17) + globals.top_screen_edge)
end


local function draw_hitbox(hb, color)
	if not hb then return end

	local left   = game_x_to_mame(hb.left)
	local bottom = game_y_to_mame(hb.bottom)
	local right  = game_x_to_mame(hb.right)
	local top    = game_y_to_mame(hb.top)
	local hval   = game_x_to_mame(hb.hval)
	local vval   = game_y_to_mame(hb.vval)

	if left - right == 0 and top - bottom == 0 then return end

	if DRAW_MINI_AXIS then
		gui.drawline(hval, vval-MINI_AXIS_SIZE, hval, vval+MINI_AXIS_SIZE, OR(color, 0xFF))
		gui.drawline(hval-MINI_AXIS_SIZE, vval, hval+MINI_AXIS_SIZE, vval, OR(color, 0xFF))
	end
	gui.box(left, top, right, bottom, color)
end


local function draw_game_object(obj, is_projectile)
	if not obj or not obj.pos_x then return end

	local x = game_x_to_mame(obj.pos_x)
	local y = game_y_to_mame(obj.pos_y)

	for entry in ipairs(obj) do
		draw_hitbox(obj[entry], game.boxes[entry].color)
	end

	if DRAW_AXIS then
		gui.drawline(x, y-AXIS_SIZE, x, y+AXIS_SIZE, AXIS_COLOR)
		gui.drawline(x-AXIS_SIZE, y, x+AXIS_SIZE, y, AXIS_COLOR)
	end
end


local function render_cps2_hitboxes()
	if not game or globals.game_phase == GAME_PHASE_NOT_PLAYING then
		gui.clearuncommitted()
		return
	end

	if BLANK_SCREEN then
		gui.box(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLANK_COLOR)
	end

	for p = 1, game.nplayers do
		draw_game_object(frame_buffer_array[1][player][p])
	end

	for i = 0, globals.num_projectiles-1 do
		draw_game_object(frame_buffer_array[1][projectiles][i], true)
	end
end


gui.register( function()
	render_cps2_hitboxes()
end)