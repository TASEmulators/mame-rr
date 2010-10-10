print("CPS-2 Marvel hitbox viewer")
print("October 10, 2010")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis")
print("Lua hotkey 4: toggle pushboxes") print()

local VULNERABILITY_COLOR    = 0x7777FF40
local ATTACK_COLOR           = 0xFF000060
local PUSH_COLOR             = 0x00FF0040
local THROW_COLOR            = 0xFFFF0060
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

local profile = {
	{
		games = {"xmcota"},
		number = {players = 2, projectiles = 0x40},--
		address = {
			player           = 0xFF4000,
			projectile       = 0xFF9924,
			game_phase       = 0xFF6F14,
			stage            = 0xFF488F,
			stage_camera = {
				[0x0] = 0xFF498C, --Wolverine
				[0x1] = 0xFF4A0C, --Psylocke
				[0x2] = 0xFF4A0C, --Colossus
				[0x3] = 0xFF4A0C, --Cyclops
				[0x4] = 0xFF4A0C, --Storm
				[0x5] = 0xFF4A0C, --Iceman
				[0x6] = 0xFF4A0C, --Spiral
				[0x7] = 0xFF4A0C, --Silver Samurai
				[0x8] = 0xFF4A0C, --Omega Red
				[0x9] = 0xFF4A0C, --Sentinel
				[0xA] = 0xFF4A0C, --Juggernaut
				[0xB] = 0xFF498C, --Magneto
			},
		},
		offset = {
			player_space     = 0x400,
			projectile_space = 0x0E0,
			facing_dir       = 0x4D,
			x_position       = 0x0C,
			hitbox_ptr       = nil,
			invulnerability  = {},
			hval = 0x0, hrad = 0x2, vval = 0x4, vrad = 0x6,
		},
		boxes = {
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x7C, id_space = 0x08, type = PUSH_BOX}, --?
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x74, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x76, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x78, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x7A, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x70, id_space = 0x08, type = ATTACK_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x72, id_space = 0x08, type = ATTACK_BOX},
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
			player_space     = 0x400,
			projectile_space = 0x0E0,
			facing_dir       = 0x4D,
			x_position       = 0x0C,
			hitbox_ptr       = nil,
			invulnerability  = {},
			hval = 0x0, hrad = 0x2, vval = 0x4, vrad = 0x6,
		},
		boxes = {
			--{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr = 0x80, id_space = 0x08, type = PUSH_BOX}, --?
			{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr = 0x78, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr = 0x7A, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr = 0x7C, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr = 0x7E, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr = 0x74, id_space = 0x08, type = ATTACK_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr = 0x76, id_space = 0x08, type = ATTACK_BOX},
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
			player_space     = 0x400,
			projectile_space = 0x0C0,
			facing_dir       = 0x4B,
			x_position       = 0x0C,
			hitbox_ptr       = nil,
			invulnerability  = {},
			hval = 0x0, hrad = 0x2, vval = 0x4, vrad = 0x6,
		},
		boxes = {
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x7C, id_space = 0x08, type = PUSH_BOX}, --?
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x74, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x76, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x78, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x7A, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x70, id_space = 0x08, type = ATTACK_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x72, id_space = 0x08, type = ATTACK_BOX},
		},
	},
	{
		games = {"mshvsf"},
		number = {players = 4, projectiles = 0x40},
		address = {
			player           = 0xFF3800,
			projectile       = 0xFFAB96,
			game_phase       = 0xFF9BD8,
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
			player_space     = 0x400,
			projectile_space = 0x0C0,
			facing_dir       = 0x4B,
			x_position       = 0x0C,
			hitbox_ptr       = nil,
			invulnerability  = {},
			hval = 0x0, hrad = 0x2, vval = 0x4, vrad = 0x6,
		},
		boxes = {
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x7C, id_space = 0x08, type = PUSH_BOX}, --?
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x74, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x76, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x78, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x7A, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x70, id_space = 0x08, type = ATTACK_BOX},
			{anim_ptr =  nil, addr_table_ptr = 0x6C, id_ptr = 0x72, id_space = 0x08, type = ATTACK_BOX},
		},
	},
}

for game in ipairs(profile) do
	local g = profile[game]
	if type(g.offset.hitbox_ptr) == "number" then
		local ptr = g.offset.hitbox_ptr
		g.offset.hitbox_ptr = {player = ptr, projectile = ptr}
	end
	g.offset.hitbox_ptr  = g.offset.hitbox_ptr  or {}
	g.offset.y_position  = g.offset.y_position  or g.offset.x_position + 0x4
	g.box_parameter_func = g.box_parameter_func or memory.readwordsigned
	for entry in ipairs(g.boxes) do
		local box = profile[game].boxes[entry]
		if box.type == VULNERABILITY_BOX then
			box.color = VULNERABILITY_COLOR
		elseif box.type == THROW_BOX then
			box.color = THROW_COLOR
		elseif box.type == ATTACK_BOX then
			box.color = ATTACK_COLOR
		elseif box.type == PUSH_BOX then
			box.color = PUSH_COLOR
			box.outline = OR(box.color, 0xC0)
		end
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


local function define_box(obj, entry, hitbox_ptr, is_projectile)
	local base_id = obj.base
	if game.boxes[entry].anim_ptr then
		base_id = memory.readdword(obj.base + game.boxes[entry].anim_ptr)
	end
	local curr_id = memory.readbyte(base_id + game.boxes[entry].id_ptr + 1)

	if curr_id == 0 or math.floor(memory.readbyte(base_id + game.boxes[entry].id_ptr)/0x10) == 0x8 then
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
	local hrad = game.box_parameter_func(address + game.offset.hrad)
	local vrad = game.box_parameter_func(address + game.offset.vrad)

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
		type   = game.boxes[entry].type,
		color  = game.boxes[entry].color,
		outline= game.boxes[entry].outline,
	}
end


local function update_game_object(obj, is_projectile)
	obj.facing_dir   = memory.readbyte(obj.base + game.offset.facing_dir)
	obj.pos_x        = memory.readwordsigned(obj.base + game.offset.x_position)
	obj.pos_y        = memory.readwordsigned(obj.base + game.offset.y_position)

	local hitbox_ptr = is_projectile and game.offset.hitbox_ptr.projectile or game.offset.hitbox_ptr.player
	for entry in ipairs(game.boxes) do
		obj[entry] = define_box(obj, entry, hitbox_ptr, is_projectile)
	end
end


local function read_projectiles()
	local current_projectiles = {}

	for i = 1, game.number.projectiles do
		local obj = {base = game.address.projectile + (i-1) * game.offset.projectile_space}
		if memory.readword(obj.base + 0x04) == 0x0002 then
	--[[for address = 0xFFDE16, 0xFFE1DA, 0x4 do
		local obj = {base = memory.readdword(address)}
		if address > 0xFF0000 and address < 0xFFFFFF then]]
			update_game_object(obj, true)
			table.insert(current_projectiles, obj)
		end
	end

	return current_projectiles
end


local function update_invulnerability(player)
	player.invulnerability = false
	for _,address in ipairs(game.offset.invulnerability) do
		if memory.readbyte(obj.base + address) > 0 then
			player.invulnerability = true
		end
	end
end


local function update_marvel_hitboxes()
	gui.clearuncommitted()
	if not game then return end
	update_globals()

	for p = 1, game.number.players do
		player[p] = {base = game.address.player + (p-1) * game.offset.player_space}
		--if memory.readword(player[p].base) >= 0x0100 then
		if true then
			update_game_object(player[p])
			update_invulnerability(player[p])
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

local function draw_hitbox(hb, invulnerability)
	if invulnerability and hb.type == VULNERABILITY_BOX then return end
	--if hb.left > hb.right or hb.bottom > hb.top then return end

	if DRAW_MINI_AXIS then
		gui.drawline(hb.hval, hb.vval-MINI_AXIS_SIZE, hb.hval, hb.vval+MINI_AXIS_SIZE, OR(hb.color, 0xFF))
		gui.drawline(hb.hval-MINI_AXIS_SIZE, hb.vval, hb.hval+MINI_AXIS_SIZE, hb.vval, OR(hb.color, 0xFF))
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, hb.color, hb.outline)
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

	if DRAW_AXIS then
		for p = 1, game.number.players do
			draw_game_object(frame_buffer[1][player][p])
		end
		for i,obj in ipairs(frame_buffer[1][projectiles]) do
			draw_game_object(frame_buffer[1][projectiles][i])
			gui.text(game_x_to_mame(obj.pos_x), game_y_to_mame(obj.pos_y), string.format("%X",obj.base)) --debug
		end
	end

	for entry in ipairs(game.boxes) do
		for p = 1, game.number.players do
			local obj = frame_buffer[1][player][p]
			if obj and obj[entry] and not (not DRAW_PUSHBOXES and game.boxes[entry].type == PUSH_BOX) then
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

end


gui.register( function()
	render_marvel_hitboxes()
end)