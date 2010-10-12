print("CPS-2 Marvel hitbox viewer")
print("October 11, 2010")
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
local DRAW_THROWBOXES        = false

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
		games = {"xmcota"},
		number = {players = 2, projectiles = 0x40},--
		address = {
			player           = 0xFF4000,
			projectile       = 0xFF9924,
			game_phase       = 0xFF4BA4,
			stage            = 0xFF488F,
			stage_camera = {
				[0x0] = 0xFF498C, --Savage Land (Wolverine)
				[0x1] = 0xFF4A0C, --Moon Night (Psylocke)
				[0x2] = 0xFF4A0C, --Mutant Hunting (Colossus)
				[0x3] = 0xFF4A0C, --Danger Room (Cyclops)
				[0x4] = 0xFF4A0C, --On the Blackbird (Storm)
				[0x5] = 0xFF4A0C, --Ice on the Beach (Iceman)
				[0x6] = 0xFF4A0C, --Mojo World (Spiral)
				[0x7] = 0xFF4A0C, --Samurai Shrine (Silver Samurai)
				[0x8] = 0xFF4A0C, --The Deep (Omega Red)
				[0x9] = 0xFF4A0C, --Genosha (Sentinel)
				[0xA] = 0xFF4A0C, --Space Port (Juggernaut)
				[0xB] = 0xFF498C, --Avalon (Magneto)
			},
		},
		offset = {
			projectile_space = 0xE0,
			facing_dir       = 0x4D,
			char_id          = 0x50,
		},
		boxes = {
			{addr_table = 0x0C15E2, id_ptr = 0xA2, type = PUSH_BOX},
			{addr_table_ptr = 0x88, id_ptr = 0x7C, type = THROWABLE_BOX},
			{addr_table_ptr = 0x88, id_ptr = 0x74, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x88, id_ptr = 0x76, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x88, id_ptr = 0x78, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x88, id_ptr = 0x7A, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x88, id_ptr = 0x70, type = ATTACK_BOX},
			{addr_table_ptr = 0x88, id_ptr = 0x72, type = ATTACK_BOX},
		},
	},
	{
		games = {"msh"},
		number = {players = 2, projectiles = 0x40},--
		address = {
			player           = 0xFF4000,
			projectile       = 0xFFAB50,
			game_phase       = 0xFF8EA0,
			stage            = 0xFF4893,
			stage_camera = {
				[0x0] = 0xFF4A0C, --Spider-Man
				[0x1] = 0xFF4A0C, --Captain America
				[0x2] = 0xFF4A0C, --Hulk
				[0x3] = 0xFF4A0C, --Iron Man
				[0x4] = 0xFF498C, --Wolverine
				[0x5] = 0xFF4A0C, --Psylocke
				[0x6] = 0xFF4B0C, --BlackHeart
				[0x7] = 0xFF4A0C, --Shuma-Gorath
				[0x8] = 0xFF4A0C, --Juggernaut
				[0x9] = 0xFF4A0C, --Magneto
				[0xA] = 0xFF4A0C, --Dr.Doom
				[0xA] = 0xFF4A0C, --Thanos
			},
		},
		offset = {
			projectile_space = 0xE0,
			facing_dir       = 0x4D,
			char_id          = 0x50,
		},
		boxes = {
			{addr_table = 0x09E82C, id_ptr = 0xA2, type = PUSH_BOX},
			{addr_table_ptr = 0x90, id_ptr = 0x80, type = THROWABLE_BOX},
			{addr_table_ptr = 0x90, id_ptr = 0x78, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x90, id_ptr = 0x7A, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x90, id_ptr = 0x7C, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x90, id_ptr = 0x7E, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x90, id_ptr = 0x74, type = ATTACK_BOX},
			{addr_table_ptr = 0x90, id_ptr = 0x76, type = ATTACK_BOX},
		},
	},
	{
		games = {"xmvsf"},
		number = {players = 4, projectiles = 0x40},
		address = {
			player           = 0xFF4000,
			projectile       = 0xFFAC80,
			game_phase       = 0xFF9FD0,
			stage            = 0xFF5113,
			stage_camera = {
				[0x0] = 0xFF534C, --Apocalypse Now!
				[0x1] = 0xFF524C, --Showdown in the Park
				[0x2] = 0xFF524C, --Death Valley
				[0x3] = 0xFF52CC, --The Cataract
				[0x4] = 0xFF524C, --The Temple of Fists
				[0x5] = 0xFF524C, --On the Hilltop
				[0x6] = 0xFF534C, --Manhattan
				[0x7] = 0xFF524C, --Raging Inferno
				[0x8] = 0xFF52CC, --Code Red
				[0x9] = 0xFF524C, --Dead or Live: The Show
				[0xA] = 0xFF524C, --Mall Mayhem
			},
		},
		offset = {
			projectile_space = 0xC0,
			facing_dir       = 0x4B,
			char_id          = 0x52,
		},
		boxes = {
			{addr_table = 0x08B022, id_ptr = 0xA4, type = PUSH_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x7C, type = THROWABLE_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x74, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x76, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x78, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x7A, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x70, type = ATTACK_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x72, type = ATTACK_BOX},
		},
	},
	{
		games = {"mshvsf"},
		number = {players = 4, projectiles = 0x40},
		address = {
			player           = 0xFF3800,
			projectile       = 0xFFAB96,
			game_phase       = 0xFF9BD8,--
			stage            = 0xFF4913,
			stage_camera = {
				[0x0] = 0xFF4B4C, --Apocalypse Now!
				[0x1] = 0xFF4A4C, --Showdown in the Park
				[0x2] = 0xFF4A4C, --Death Valley
				[0x3] = 0xFF4ACC, --The Cataract
				[0x4] = 0xFF4A4C, --The Temple of Fists
				[0x5] = 0xFF4A4C, --On the Hilltop
				[0x6] = 0xFF4B4C, --Manhattan
				[0x7] = 0xFF4A4C, --Raging Inferno
				[0x8] = 0xFF4ACC, --Code Red
				[0x9] = 0xFF4A4C, --Dead or Live: The Show
				[0xA] = 0xFF4A4C, --Mall Mayhem
				[0xB] = 0xFF4A4C, --Raging Inferno 2
				[0xC] = 0xFF4A4C, --Death Valley 2
			},
		},
		offset = {
			projectile_space = 0xC0,
			facing_dir       = 0x4B,
			char_id          = 0x52,
		},
		boxes = {
			{addr_table = 0x137EE2, id_ptr = 0xA4, type = PUSH_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x7C, type = THROWABLE_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x74, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x76, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x78, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x7A, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x70, type = ATTACK_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x72, type = ATTACK_BOX},
		},
	},
	{
		games = {"mvsc"},
		number = {players = 4, projectiles = 0x40},
		address = {
			player           = 0xFF3000,
			projectile       = 0xFFA3BA,
			game_phase       = 0xFF6120,
			stage            = 0xFF4113,
			stage_camera = {
				[0x0] = 0xFF426C, --Bath house
				[0x1] = 0xFF426C, --Rapter stage
				[0x2] = 0xFF426C, --Strider mosque
				[0x3] = 0xFF42EC, --Dr. Wily
				[0x4] = 0xFF42EC, --Marvel stuff
				[0x5] = 0xFF426C, --Avengers HQ
				[0x6] = 0xFF426C, --Moon base
				[0x7] = 0xFF41EC, --Daily Bugle
				[0x8] = 0xFF41EC, --Mountain
				[0x9] = 0xFF436C, --Onslaught
			},
		},
		offset = {
			projectile_space = 0xD0,
			facing_dir       = 0x4B,
			char_id          = 0x52,
		},
		boxes = {
			{addr_table = 0x0E6FEE, id_ptr = 0xB4, type = PUSH_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x7C, type = THROWABLE_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x74, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x76, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x78, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x7A, type = VULNERABILITY_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x70, type = ATTACK_BOX},
			{addr_table_ptr = 0x6C, id_ptr = 0x72, type = ATTACK_BOX},
		},
	},
}

for game in ipairs(profile) do
	local g = profile[game]
	g.player_status = g.number.players > 2 and 0x100 or 0x1
	g.offset.player_space = g.offset.player_space or 0x400
	g.offset.x_position   = g.offset.x_position   or 0x0C
	g.offset.y_position   = g.offset.y_position   or 0x10
	g.offset.hurt         = g.offset.hurt         or 0x06
	g.offset.hval         = g.offset.hval         or 0x0
	g.offset.hrad         = g.offset.hvad         or 0x2
	g.offset.vval         = g.offset.vval         or 0x4
	g.offset.vrad         = g.offset.vrad         or 0x6
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


input.registerhotkey(4, function()
	DRAW_PUSHBOXES = not DRAW_PUSHBOXES
	print((DRAW_PUSHBOXES and "showing" or "hiding") .. " pushboxes")
end)


input.registerhotkey(5, function()
	DRAW_THROWBOXES = not DRAW_THROWBOXES
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


--------------------------------------------------------------------------------
-- prepare the hitboxes

local function update_globals()
	local curr_stage = memory.readbyte(game.address.stage)
	if not game.address.stage_camera[curr_stage] then
		curr_stage = 1
	end
	globals.left_screen_edge = memory.readwordsigned(game.address.stage_camera[curr_stage]) + 0x40
	globals.top_screen_edge  = memory.readwordsigned(game.address.stage_camera[curr_stage] + 0x4)
	globals.game_phase       = memory.readbyte(game.address.game_phase)
end


local function game_x_to_mame(x)
	return x - globals.left_screen_edge
end


local function game_y_to_mame(y)
	-- Why subtract 17? No idea, the game driver does the same thing.
	return y - globals.top_screen_edge - 17
end


local function define_box(obj, entry, is_projectile)
	local address, box_type

	if game.boxes[entry].addr_table_ptr then
		local curr_id = memory.readword(obj.base + game.boxes[entry].id_ptr)
		if curr_id == 0 then
			return nil
		end

		if game.boxes[entry].type == ATTACK_BOX and math.floor(curr_id/0x1000) == 0x8 then
			box_type = THROW_BOX
			if memory.readword(obj.base + game.offset.hurt) > 0 then
				return nil
			end
		end
		curr_id = curr_id % 0x1000

		local addr_table = memory.readdword(obj.base + game.boxes[entry].addr_table_ptr)
		address = addr_table + curr_id * 8

	else --pushbox
		if is_projectile then--or memory.readbytesigned(obj.base + 0xE3) >= 0 then
			return nil
		end

		local curr_id = memory.readbyte(obj.base + game.boxes[entry].id_ptr)
		local addr_table = game.boxes[entry].addr_table
		address = memory.readdword(addr_table + curr_id * 2) + memory.readword(obj.base + game.offset.char_id) * 4
	end

	local hval = memory.readwordsigned(address + game.offset.hval)
	local vval = memory.readwordsigned(address + game.offset.vval)
	local hrad = memory.readwordsigned(address + game.offset.hrad)
	local vrad = memory.readwordsigned(address + game.offset.vrad)

	if obj.facing_dir == 1 then
		hval  = -hval
	end

	return {
		left   = game_x_to_mame(obj.pos_x + hval - hrad),
		right  = game_x_to_mame(obj.pos_x + hval + hrad),
		bottom = game_y_to_mame(obj.pos_y + vval + vrad),
		top    = game_y_to_mame(obj.pos_y + vval - vrad),
		hval   = game_x_to_mame(obj.pos_x + hval),
		vval   = game_y_to_mame(obj.pos_y + vval),
		type   = box_type or game.boxes[entry].type,
	}
end


local function update_game_object(obj, is_projectile)
	obj.facing_dir   = memory.readbyte(obj.base + game.offset.facing_dir)
	obj.pos_x        = memory.readwordsigned(obj.base + game.offset.x_position)
	obj.pos_y        = memory.readwordsigned(obj.base + game.offset.y_position)

	for entry in ipairs(game.boxes) do
		obj[entry] = define_box(obj, entry, is_projectile)
	end
end


local function read_projectiles()
	local current_projectiles = {}

	for i = 1, game.number.projectiles do
		local obj = {base = game.address.projectile + (i-1) * game.offset.projectile_space}
		if memory.readword(obj.base + 0x04) == 0x0002 then
			update_game_object(obj, true)
			table.insert(current_projectiles, obj)
		end
	end

--[[
	for i = 1, 0x20 do
		local obj = {base = memory.readdword(0xFFE32E - i * 4)} --mshvsf
		if obj.base == 0 then
			break
		elseif obj.base ~= 0xFF3800 and obj.base ~= 0xFF4000 then
			update_game_object(obj, true)
			table.insert(current_projectiles, obj)
			if DRAW_AXIS then
				gui.text(0,(i-1)*8,i .. "\t" .. string.format("%X",obj.base),"yellow")
			end
		end
	end

	local address = 0xFFDE16 --mshvsf
	for i = 1, 0xE0 do
		local obj = {base = memory.readdword(address + (i-1)*4)}
		if obj.base > 0xFF0000 and obj.base < 0xFFFFFF and memory.readword(obj.base + 0x02) > 0 then
			update_game_object(obj, true)
			table.insert(current_projectiles, obj)
		end
	end

]]
	return current_projectiles
end


local function update_marvel_hitboxes()
	gui.clearuncommitted()
	if not game then return end
	update_globals()

	for p = 1, game.number.players do
		player[p] = {base = game.address.player + (p-1) * game.offset.player_space}
		if memory.readword(player[p].base) >= game.player_status then
			update_game_object(player[p])
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
	update_marvel_hitboxes()
end)


--------------------------------------------------------------------------------
-- draw the hitboxes

local function draw_hitbox(hb)
	if not DRAW_PUSHBOXES and hb.type == PUSH_BOX then
		return
	elseif not DRAW_THROWBOXES and (hb.type == THROW_BOX or hb.type == THROWABLE_BOX) then
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


local function render_marvel_hitboxes()
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
				draw_hitbox(obj[entry])
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
		for i,obj in ipairs(frame_buffer[1][projectiles]) do
			draw_game_object(frame_buffer[1][projectiles][i])
			gui.text(game_x_to_mame(obj.pos_x), game_y_to_mame(obj.pos_y), string.format("%X",obj.base)) --debug
		end
	end

end


gui.register( function()
	render_marvel_hitboxes()
end)