print("Street Fighter II hitbox viewer")
print("March 17, 2011")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis")
print("Lua hotkey 4: toggle pushboxes") print()

local VULNERABILITY_COLOR      = 0x7777FF40
local ATTACK_COLOR             = 0xFF000040
local PROJ_VULNERABILITY_COLOR = 0x00FFFF40
local PROJ_ATTACK_COLOR        = 0xFF66FF60
local PUSH_COLOR               = 0x00FF0020
local WEAK_COLOR               = 0xFFFF0060
local AXIS_COLOR               = 0xFFFFFFFF
local BLANK_COLOR              = 0xFFFFFFFF
local AXIS_SIZE                = 12
local MINI_AXIS_SIZE           = 2
local DRAW_DELAY               = 1
local BLANK_SCREEN             = false
local DRAW_AXIS                = false
local DRAW_MINI_AXIS           = false
local DRAW_PUSHBOXES           = true

local GAME_PHASE_NOT_PLAYING = 0
local NUMBER_OF_PLAYERS      = 2
local MAX_GAME_PROJECTILES   = 8
local MAX_BONUS_OBJECTS      = 16
local VULNERABILITY_BOX      = 1
local ATTACK_BOX             = 2
local PROJ_VULNERABILITY_BOX = 3
local PROJ_ATTACK_BOX        = 4
local PUSH_BOX               = 5
local WEAK_BOX               = 6

local fill = {
	VULNERABILITY_COLOR,
	ATTACK_COLOR,
	PROJ_VULNERABILITY_COLOR,
	PROJ_ATTACK_COLOR,
	PUSH_COLOR,
	WEAK_COLOR,
}

local outline = {
	bit.bor(0xFF, VULNERABILITY_COLOR),
	bit.bor(0xFF, ATTACK_COLOR),
	bit.bor(0xFF, PROJ_VULNERABILITY_COLOR),
	bit.bor(0xFF, PROJ_ATTACK_COLOR),
	bit.bor(0xC0, PUSH_COLOR),
	bit.bor(0xFF, WEAK_COLOR),
}

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
		box_parameter_size = 1,
		box_list = {
			{addr_table = 0xA, id_ptr = 0xD, id_space = 0x04, type = PUSH_BOX},
			{addr_table = 0x0, id_ptr = 0x8, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x2, id_ptr = 0x9, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x4, id_ptr = 0xA, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x6, id_ptr = 0xB, id_space = 0x04, type = WEAK_BOX},
			{addr_table = 0x8, id_ptr = 0xC, id_space = 0x0C, type = ATTACK_BOX},
		},
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
		box_parameter_size = 1,
		box_list = {
			{addr_table = 0xA, id_ptr = 0xD, id_space = 0x04, type = PUSH_BOX},
			{addr_table = 0x0, id_ptr = 0x8, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x2, id_ptr = 0x9, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x4, id_ptr = 0xA, id_space = 0x04, type = VULNERABILITY_BOX},
			--{addr_table = 0x6, id_ptr = 0xB, id_space = 0x04, type = WEAK_BOX},
			{addr_table = 0x8, id_ptr = 0xC, id_space = 0x0C, type = ATTACK_BOX},
		},
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
		box_parameter_size = 1,
		box_list = {
			{addr_table = 0x8, id_ptr = 0xD, id_space = 0x04, type = PUSH_BOX},
			{addr_table = 0x0, id_ptr = 0x8, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x2, id_ptr = 0x9, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x4, id_ptr = 0xA, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x6, id_ptr = 0xC, id_space = 0x10, type = ATTACK_BOX},
		},
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
		box_parameter_size = 1,
		box_list = {
			{addr_table = 0x8, id_ptr = 0xD, id_space = 0x04, type = PUSH_BOX},
			{addr_table = 0x0, id_ptr = 0x8, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x2, id_ptr = 0x9, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x4, id_ptr = 0xA, id_space = 0x04, type = VULNERABILITY_BOX},
			{addr_table = 0x6, id_ptr = 0xC, id_space = 0x0C, type = ATTACK_BOX},
		},
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
		char_mode          = 0x32A,
		player_space       = 0x400,
		box_parameter_size = 2,
		box_list = {
			{addr_table = 0xA, id_ptr = 0xD, id_space = 0x08, type = PUSH_BOX},
			{addr_table = 0x0, id_ptr = 0x8, id_space = 0x08, type = VULNERABILITY_BOX},
			{addr_table = 0x2, id_ptr = 0x9, id_space = 0x08, type = VULNERABILITY_BOX},
			{addr_table = 0x4, id_ptr = 0xA, id_space = 0x08, type = VULNERABILITY_BOX},
			{addr_table = 0x6, id_ptr = 0xB, id_space = 0x08, type = WEAK_BOX},
			{addr_table = 0x8, id_ptr = 0xC, id_space = 0x14, type = ATTACK_BOX},
		},
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
-- prepare the hitboxes

local function adjust_delay(address)
	if not address or not mame then
		return DRAW_DELAY
	end
	local stage = memory.readbyte(address)
	for _, val in ipairs({
		0xA, --Boxer
		0xC, --Cammy
		0xD, --T.Hawk
		0xF, --Dee Jay
	}) do
		if stage == val then
			return DRAW_DELAY + 1 --these stages have an extra frame of lag
		end
	end
	return DRAW_DELAY
end


local function update_globals()
	globals.left_screen_edge = memory.readword(game.address.left_screen_edge)
	globals.top_screen_edge  = memory.readword(game.address.left_screen_edge + 0x4)
	globals.game_phase       = memory.readword(game.address.game_phase)
end


local function get_x(x)
	return x - globals.left_screen_edge
end


local function get_y(y)
	return emu.screenheight() - (y - 15) + globals.top_screen_edge
end


local get_box_parameters = {
	[1] = function(box)
		box.hval   = memory.readbytesigned(box.address + 0)
		box.hval2  = memory.readbyte(box.address + 5)
		if box.hval2 >= 0x80 and box.type == ATTACK_BOX then
			box.hval = -box.hval2
		end
		box.vval   = memory.readbytesigned(box.address + 1)
		box.hrad   = memory.readbyte(box.address + 2)
		box.vrad   = memory.readbyte(box.address + 3)
	end,

	[2] = function(box)
		box.hval   = memory.readwordsigned(box.address + 0)
		box.vval   = memory.readwordsigned(box.address + 2)
		box.hrad   = memory.readword(box.address + 4)
		box.vrad   = memory.readword(box.address + 6)
	end,
}


local function define_box(obj, entry)
	local box = {
		type = game.box_list[entry].type,
		id = memory.readbyte(obj.animation_ptr + game.box_list[entry].id_ptr),
	}

	if box.id == 0 then
		return
	elseif obj.projectile then
		if box.type == ATTACK_BOX then
			box.type = PROJ_ATTACK_BOX
		else
			box.type = PROJ_VULNERABILITY_BOX
		end
	end

	local addr_table = obj.hitbox_ptr + memory.readwordsigned(obj.hitbox_ptr + game.box_list[entry].addr_table)
	box.address = addr_table + box.id * game.box_list[entry].id_space

	get_box_parameters[game.box_parameter_size](box)

	box.hval   = obj.pos_x + box.hval * (obj.facing_dir == 1 and -1 or 1)
	box.vval   = obj.pos_y - box.vval
	box.left   = box.hval - box.hrad
	box.right  = box.hval + box.hrad
	box.top    = box.vval - box.vrad
	box.bottom = box.vval + box.vrad

	return box
end


local function update_game_object(obj)
	obj.facing_dir    = memory.readbyte(obj.base + 0x12)
	obj.pos_x         = get_x(memory.readwordsigned(obj.base + 0x06))
	obj.pos_y         = get_y(memory.readwordsigned(obj.base + 0x0A))
	obj.animation_ptr = memory.readdword(obj.base + 0x1A)
	obj.hitbox_ptr    = memory.readdword(obj.base + 0x34)

	for entry in ipairs(game.box_list) do
		obj[entry] = define_box(obj, entry, animation_ptr, hitbox_ptr)
	end
end


local function read_projectiles()
	local current_projectiles = {}

	for i = 1, MAX_GAME_PROJECTILES do
		local obj = {base = game.address.projectile + (i-1) * 0xC0}
		if memory.readword(obj.base) == 0x0101 then
			obj.projectile = true
			update_game_object(obj)
			table.insert(current_projectiles, obj)
		end
	end

	for i = 1, MAX_BONUS_OBJECTS do
		local obj = {base = game.address.projectile + (MAX_GAME_PROJECTILES + i-1) * 0xC0}
		if bit.band(0xff00, memory.readword(obj.base)) == 0x0100 then
			obj.no_attack = memory.readbyte(obj.base + 0x03) == 0
			obj.projectile = true
			update_game_object(obj)
			table.insert(current_projectiles, obj)
		end
	end

	return current_projectiles
end


local function update_sf2_hitboxes()
	if not game then
		return
	end
	effective_delay = adjust_delay(game.address.stage)
	update_globals()

	for p = 1, NUMBER_OF_PLAYERS do
		player[p] = {base = game.address.player + (p-1) * game.player_space}
		if memory.readword(player[p].base) > 0x0100 then
			player[p].no_weak = game.char_mode and memory.readbyte(player[p].base + game.char_mode) ~= 0x4
			update_game_object(player[p])
		end
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
	if (not DRAW_PUSHBOXES and hb.type == PUSH_BOX) or
		(hb.type == PROJ_ATTACK_BOX and obj.no_attack) or
		(hb.type == WEAK_BOX and (obj.no_weak or memory.readbyte(obj.animation_ptr + 0x15) ~= 2)) then
		return
	end

	if DRAW_MINI_AXIS then
		gui.drawline(hb.hval, hb.vval-MINI_AXIS_SIZE, hb.hval, hb.vval+MINI_AXIS_SIZE, outline[hb.type])
		gui.drawline(hb.hval-MINI_AXIS_SIZE, hb.vval, hb.hval+MINI_AXIS_SIZE, hb.vval, outline[hb.type])
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, fill[hb.type], outline[hb.type])
end


local function draw_axis(obj)
	if not obj or not obj.pos_x then
		return
	end
	
	gui.drawline(obj.pos_x, obj.pos_y-AXIS_SIZE, obj.pos_x, obj.pos_y+AXIS_SIZE, AXIS_COLOR)
	gui.drawline(obj.pos_x-AXIS_SIZE, obj.pos_y, obj.pos_x+AXIS_SIZE, obj.pos_y, AXIS_COLOR)
	--gui.text(obj.pos_x, obj.pos_y, string.format("%06X",obj.base)) --debug
end


local function render_sf2_hitboxes()
	gui.clearuncommitted()
	if not game or globals.game_phase == GAME_PHASE_NOT_PLAYING then
		return
	end

	if BLANK_SCREEN then
		gui.box(0, 0, emu.screenwidth(), emu.screenheight(), BLANK_COLOR)
	end

	for entry in ipairs(game.box_list) do
		for i in ipairs(frame_buffer[1][projectiles]) do
			local obj = frame_buffer[1][projectiles][i]
			if obj[entry] then
				draw_hitbox(obj, entry)
			end
		end

		for p = 1, NUMBER_OF_PLAYERS do
			local obj = frame_buffer[1][player][p]
			if obj and obj[entry] then
				draw_hitbox(obj, entry)
			end
		end
	end

	if DRAW_AXIS then
		for p = 1, NUMBER_OF_PLAYERS do
			draw_axis(frame_buffer[1][player][p])
		end
		for i in ipairs(frame_buffer[1][projectiles]) do
			draw_axis(frame_buffer[1][projectiles][i])
		end
	end
end


gui.register( function()
	render_sf2_hitboxes()
end)


--------------------------------------------------------------------------------
-- hotkey functions

input.registerhotkey(1, function()
	BLANK_SCREEN = not BLANK_SCREEN
	render_sf2_hitboxes()
	print((BLANK_SCREEN and "activated" or "deactivated") .. " blank screen mode")
end)


input.registerhotkey(2, function()
	DRAW_AXIS = not DRAW_AXIS
	render_sf2_hitboxes()
	print((DRAW_AXIS and "showing" or "hiding") .. " object axis")
end)


input.registerhotkey(3, function()
	DRAW_MINI_AXIS = not DRAW_MINI_AXIS
	render_sf2_hitboxes()
	print((DRAW_MINI_AXIS and "showing" or "hiding") .. " hitbox axis")
end)


input.registerhotkey(4, function()
	DRAW_PUSHBOXES = not DRAW_PUSHBOXES
	render_sf2_hitboxes()
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