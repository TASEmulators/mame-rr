print("Street Fighter II hitbox viewer")
print("January 19, 2011")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis")
print("Lua hotkey 4: toggle pushboxes") print()

local VULNERABILITY_COLOR     = 0x7777FF40
local ATTACK_COLOR            = 0xFF000060
local PUSH_COLOR              = 0x00FF0040
local WEAK_COLOR              = 0xFFFF0060
local PROJECTILE_ATTACK_COLOR = 0xFF000060
local PROJECTILE_PUSH_COLOR   = 0x7777FF40
local AXIS_COLOR              = 0xFFFFFFFF
local BLANK_COLOR             = 0xFFFFFFFF
local AXIS_SIZE               = 16
local MINI_AXIS_SIZE          = 2
local DRAW_DELAY              = 1

local SCREEN_WIDTH            = 384
local SCREEN_HEIGHT           = 224
local NUMBER_OF_PLAYERS       = 2
local MAX_GAME_PROJECTILES    = 8
local VULNERABILITY_BOX       = 1
local ATTACK_BOX              = 2
local PUSH_BOX                = 3
local WEAK_BOX                = 4
local PROJECTILE_ATTACK_BOX   = 5
local PROJECTILE_PUSH_BOX     = 6
local GAME_PHASE_NOT_PLAYING  = 0
local BLANK_SCREEN            = false
local DRAW_AXIS               = false
local DRAW_MINI_AXIS          = false
local DRAW_PUSHBOXES          = true

local fill = {
	[VULNERABILITY_BOX]     = VULNERABILITY_COLOR,
	[ATTACK_BOX]            = ATTACK_COLOR,
	[PUSH_BOX]              = PUSH_COLOR,
	[WEAK_BOX]              = WEAK_COLOR,
	[PROJECTILE_ATTACK_BOX] = PROJECTILE_ATTACK_COLOR,
	[PROJECTILE_PUSH_BOX]   = PROJECTILE_PUSH_COLOR,
}

local outline = {
	[VULNERABILITY_BOX]     = OR(VULNERABILITY_COLOR,     0xFF),
	[ATTACK_BOX]            = OR(ATTACK_COLOR,            0xFF),
	[PUSH_BOX]              = OR(PUSH_COLOR,              0xC0),
	[WEAK_BOX]              = OR(WEAK_COLOR,              0xFF),
	[PROJECTILE_ATTACK_BOX] = OR(PROJECTILE_ATTACK_COLOR, 0xFF),
	[PROJECTILE_PUSH_BOX]   = OR(PROJECTILE_PUSH_COLOR,   0xC0),
}

local function onebyte(address, type)
	local hval   = memory.readbytesigned(address + 0)
	local hval2  = memory.readbyte(address + 5)
	if hval2 >= 0x80 and type == ATTACK_BOX then
		hval = -hval2
	end
	local vval   = memory.readbytesigned(address + 1)
	local hrad   = memory.readbytesigned(address + 2)
	local vrad   = memory.readbytesigned(address + 3)
	return hval, vval, hrad, vrad
end

local function twobyte(address)
	local hval   = memory.readwordsigned(address + 0)
	local vval   = memory.readwordsigned(address + 2)
	local hrad   = memory.readwordsigned(address + 4)
	local vrad   = memory.readwordsigned(address + 6)
	return hval, vval, hrad, vrad
end


local profile = {
	{
		games = {"sf2"},
		address = {
			player           = 0xFF83C6,
			projectile       = 0xFF938A,
			left_screen_edge = 0xFF8BD8,
			game_phase       = 0xFF83F7,
		},
		player_space       = 0x300,
		boxes = {
			{addr_table = 0xA, id_ptr = 0xD, id_space = 0x04, type = PUSH_BOX},
			{addr_table = 0x0, id_ptr = 0x8, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x2, id_ptr = 0x9, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x4, id_ptr = 0xA, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x6, id_ptr = 0xB, id_space = 0x04, type = WEAK_BOX},
			{addr_table = 0x8, id_ptr = 0xC, id_space = 0x0C, type = ATTACK_BOX},
		},
		box_parameter_func = onebyte,
	},
	{
		games = {"sf2ce","sf2hf"},
		address = {
			player           = 0xFF83BE,
			projectile       = 0xFF9376,
			left_screen_edge = 0xFF8BC4,
			game_phase       = 0xFF83EF,
		},
		player_space       = 0x300,
		boxes = {
			{addr_table = 0xA, id_ptr = 0xD, id_space = 0x04, type = PUSH_BOX},
			{addr_table = 0x0, id_ptr = 0x8, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x2, id_ptr = 0x9, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x4, id_ptr = 0xA, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x6, id_ptr = 0xB, id_space = 0x04, type = WEAK_BOX},
			{addr_table = 0x8, id_ptr = 0xC, id_space = 0x0C, type = ATTACK_BOX},
		},
		box_parameter_func = onebyte,
	},
	{
		games = {"ssf2t"},
		address = {
			player           = 0xFF844E,
			projectile       = 0xFF97A2,
			left_screen_edge = 0xFF8ED4,
			game_phase       = 0xFF847F,
			stage            = 0xFFE18B,
		},
		player_space       = 0x400,
		boxes = {
			{addr_table = 0x8, id_ptr = 0xD, id_space = 0x04, type = PUSH_BOX},
			{addr_table = 0x0, id_ptr = 0x8, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x2, id_ptr = 0x9, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x4, id_ptr = 0xA, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x6, id_ptr = 0xC, id_space = 0x10, type = ATTACK_BOX},
		},
		box_parameter_func = onebyte,
	},
	{
		games = {"ssf2"},
		address = {
			player           = 0xFF83CE,
			projectile       = 0xFF96A2,
			left_screen_edge = 0xFF8DD4,
			game_phase       = 0xFF83FF,
			stage            = 0xFFE08B,
		},
		player_space       = 0x400,
		boxes = {
			{addr_table = 0x8, id_ptr = 0xD, id_space = 0x04, type = PUSH_BOX},
			{addr_table = 0x0, id_ptr = 0x8, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x2, id_ptr = 0x9, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x4, id_ptr = 0xA, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x6, id_ptr = 0xC, id_space = 0x0C, type = ATTACK_BOX},
		},
		box_parameter_func = onebyte,
	},
	{
		games = {"hsf2"},
		address = {
			player           = 0xFF833C,
			projectile       = 0xFF9554,
			left_screen_edge = 0xFF8CC2,
			game_phase       = 0xFF836D,
			stage            = 0xFF8B65,
		},
		player_space       = 0x400,
		boxes = {
			{addr_table = 0xA, id_ptr = 0xD, id_space = 0x08, type = PUSH_BOX},
			{addr_table = 0x0, id_ptr = 0x8, id_space = 0x08, type = VULNERABILITY_BOX},
			{addr_table = 0x2, id_ptr = 0x9, id_space = 0x08, type = VULNERABILITY_BOX},
			{addr_table = 0x4, id_ptr = 0xA, id_space = 0x08, type = VULNERABILITY_BOX},
			{addr_table = 0x6, id_ptr = 0xB, id_space = 0x08, type = WEAK_BOX},
			{addr_table = 0x8, id_ptr = 0xC, id_space = 0x14, type = ATTACK_BOX},
		},
		box_parameter_func = twobyte,
	},
}

local game, effective_delay
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


--------------------------------------------------------------------------------
-- initialize on game startup

local function whatgame()
	game = nil
	for n, module in ipairs(profile) do
		for m, shortname in ipairs(module.games) do
			if emu.romname() == shortname or emu.parentname() == shortname then
				print("drawing " .. shortname .. " hitboxes")
				game = module
				for p = 1, NUMBER_OF_PLAYERS do
					player[p] = {}
				end
				for f = 1, DRAW_DELAY + 2 do
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


--------------------------------------------------------------------------------
-- prepare the hitboxes

local function adjust_delay(address)
	if not address or not mame then
		return DRAW_DELAY
	end
	local stage = memory.readbyte(address)
	for _, val in ipairs({0xA, 0xC, 0xD, 0xF}) do
		if stage == val then
			return DRAW_DELAY + 1
		end
	end
	return DRAW_DELAY
end


local function update_globals()
	globals.left_screen_edge = memory.readword(game.address.left_screen_edge)
	globals.top_screen_edge  = memory.readword(game.address.left_screen_edge + 0x4)
	globals.game_phase       = memory.readword(game.address.game_phase)
end


local function game_x_to_mame(x)
	return x - globals.left_screen_edge
end


local function game_y_to_mame(y)
	return SCREEN_HEIGHT - (y - 15) + globals.top_screen_edge
end


local function define_box(obj, entry, animation_ptr, hitbox_ptr)
	local curr_id = memory.readbyte(animation_ptr + game.boxes[entry].id_ptr)
	if game.boxes[entry].type == WEAK_BOX and memory.readbyte(animation_ptr + 0x15) ~= 2 then
		curr_id = 0
	end

	if curr_id == 0 then
		obj[entry] = nil
		return
	end
	
	local addr_table = hitbox_ptr + memory.readwordsigned(hitbox_ptr + game.boxes[entry].addr_table)
	local address = addr_table + curr_id * game.boxes[entry].id_space

	local hval, vval, hrad, vrad = game.box_parameter_func(address, game.boxes[entry].type)

	if obj.facing_dir == 1 then
		hval  = -hval
	end

	local box_type
	if obj.is_projectile then
		if game.boxes[entry].type == ATTACK_BOX then
			box_type = PROJECTILE_ATTACK_BOX
		else
			box_type = PROJECTILE_PUSH_BOX
		end
	end

	return {
		left   = obj.pos_x + hval - hrad,
		right  = obj.pos_x + hval + hrad,
		top    = obj.pos_y - vval - vrad,
		bottom = obj.pos_y - vval + vrad,
		hval   = obj.pos_x + hval,
		vval   = obj.pos_y - vval,
		type   = box_type or game.boxes[entry].type,
	}
end


local function update_game_object(obj)
	obj.facing_dir   = memory.readbyte(obj.base + 0x12)
	obj.pos_x        = game_x_to_mame(memory.readword(obj.base + 0x06))
	obj.pos_y        = game_y_to_mame(memory.readword(obj.base + 0x0A))

	local animation_ptr = memory.readdword(obj.base + 0x1A)
	local hitbox_ptr    = memory.readdword(obj.base + 0x34)

	for entry in ipairs(game.boxes) do
		obj[entry] = define_box(obj, entry, animation_ptr, hitbox_ptr)
	end
end


local function read_projectiles()
	local current_projectiles = {}

	for i = 1, MAX_GAME_PROJECTILES do
		local obj = {base = game.address.projectile + (i-1) * 0xC0}
		if memory.readword(obj.base) == 0x0101 then
			obj.is_projectile = true
			update_game_object(obj)
			table.insert(current_projectiles, obj)
		end
	end

	return current_projectiles
end


local function update_sf2_hitboxes()
	gui.clearuncommitted()
	if not game then
		return
	end
	effective_delay = adjust_delay(game.address.stage)
	update_globals()

	for p = 1, NUMBER_OF_PLAYERS do
		player[p] = {base = game.address.player + (p-1) * game.player_space}
		update_game_object(player[p])
	end

	for f = 1, effective_delay do
		for p = 1, NUMBER_OF_PLAYERS do
			frame_buffer[f][player][p] = copytable(frame_buffer[f+1][player][p])
		end
		frame_buffer[f][projectiles] = copytable(frame_buffer[f+1][projectiles])
	end

	for p = 1, NUMBER_OF_PLAYERS do
		frame_buffer[effective_delay+1][player][p] = copytable(player[p])
	end
	frame_buffer[effective_delay+1][projectiles] = read_projectiles()
end

emu.registerafter( function()
	update_sf2_hitboxes()
end)


--------------------------------------------------------------------------------
-- draw the hitboxes

local function draw_hitbox(obj, entry)
	local hb = obj[entry]

	if DRAW_MINI_AXIS then
		gui.drawline(hb.hval, hb.vval-MINI_AXIS_SIZE, hb.hval, hb.vval+MINI_AXIS_SIZE, OR(fill[hb.type], 0xFF))
		gui.drawline(hb.hval-MINI_AXIS_SIZE, hb.vval, hb.hval+MINI_AXIS_SIZE, hb.vval, OR(fill[hb.type], 0xFF))
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, fill[hb.type], outline[hb.type])
end


local function draw_game_object(obj)
	if not obj or not obj.pos_x then return end
	
	gui.drawline(obj.pos_x, obj.pos_y-AXIS_SIZE, obj.pos_x, obj.pos_y+AXIS_SIZE, AXIS_COLOR)
	gui.drawline(obj.pos_x-AXIS_SIZE, obj.pos_y, obj.pos_x+AXIS_SIZE, obj.pos_y, AXIS_COLOR)
	--gui.text(obj.pos_x, obj.pos_y, string.format("%06X",obj.base)) --debug
end


local function render_sf2_hitboxes()
	if not game or globals.game_phase == GAME_PHASE_NOT_PLAYING then
		return
	end

	if BLANK_SCREEN then
		gui.box(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLANK_COLOR)
	end

	for entry in ipairs(game.boxes) do
		for p = 1, NUMBER_OF_PLAYERS do
			local obj = frame_buffer[1][player][p]
			if obj and obj[entry] and not (not DRAW_PUSHBOXES and game.boxes[entry].type == PUSH_BOX) then
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
		for p = 1, NUMBER_OF_PLAYERS do
			draw_game_object(frame_buffer[1][player][p])
		end
		for i in ipairs(frame_buffer[1][projectiles]) do
			draw_game_object(frame_buffer[1][projectiles][i])
		end
	end
end


gui.register( function()
	render_sf2_hitboxes()
end)