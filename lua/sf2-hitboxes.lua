print("Street Fighter II hitbox viewer")
print("September 14, 2010")
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
local DRAW_DELAY              = 2

local SCREEN_WIDTH            = 384
local SCREEN_HEIGHT           = 224
local NUMBER_OF_PLAYERS       = 2
local MAX_GAME_PROJECTILES    = 8
local VULNERABILITY_BOX       = 1
local WEAK_BOX                = 2
local ATTACK_BOX              = 3
local PUSH_BOX                = 4
local GAME_PHASE_NOT_PLAYING  = 0
local BLANK_SCREEN            = false
local DRAW_AXIS               = false
local DRAW_MINI_AXIS          = false
local DRAW_PUSHBOXES          = true


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

for game in ipairs(profile) do
	for entry in ipairs(profile[game].boxes) do
		local box = profile[game].boxes[entry]
		if box.type == VULNERABILITY_BOX then
			box.color = VULNERABILITY_COLOR
		elseif box.type == WEAK_BOX then
			box.color = WEAK_COLOR
		elseif box.type == ATTACK_BOX then
			box.color = ATTACK_COLOR
			box.projectile_color = PROJECTILE_ATTACK_COLOR
		elseif box.type == PUSH_BOX then
			box.color = PUSH_COLOR
			box.projectile_color = PROJECTILE_PUSH_COLOR
		end
	end
end

local game
local globals = {
	game_phase       = 0,
	left_screen_edge = 0,
	top_screen_edge  = 0,
	num_projectiles  = 0,
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
	globals.top_screen_edge  = memory.readword(game.address.left_screen_edge + 0x4)
	globals.game_phase       = memory.readword(game.address.game_phase)
end


local function define_box(obj, entry, animation_ptr, hitbox_ptr)
	local addr_table = hitbox_ptr + memory.readwordsigned(hitbox_ptr + game.boxes[entry].addr_table)
	local curr_id = memory.readbyte(animation_ptr + game.boxes[entry].id_ptr)
	if game.boxes[entry].type == WEAK_BOX and memory.readbyte(animation_ptr + 0x15) ~= 2 then
		curr_id = 0
	end
	local address = addr_table + curr_id * game.boxes[entry].id_space

	local hval, vval, hrad, vrad = game.box_parameter_func(address, game.boxes[entry].type)

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


local function update_game_object(obj, base_obj)
	obj.facing_dir   = memory.readbyte(base_obj + 0x12)
	obj.pos_x        = memory.readword(base_obj + 0x06)
	obj.pos_y        = memory.readword(base_obj + 0x0A)

	local animation_ptr = memory.readdword(base_obj + 0x1A)
	local hitbox_ptr    = memory.readdword(base_obj + 0x34)

	for entry in ipairs(game.boxes) do
		define_box(obj, entry, animation_ptr, hitbox_ptr)
	end
end


local function read_projectiles()
	globals.num_projectiles = 0
	for i = 1, MAX_GAME_PROJECTILES do
		local base_obj = game.address.projectile + (i-1) * 0xC0

		if memory.readbyte(base_obj+1) ~= 0 then
			globals.num_projectiles = globals.num_projectiles+1
			projectiles[globals.num_projectiles] = {}
			update_game_object(projectiles[globals.num_projectiles], base_obj)
		end
	end
end


local function update_cps2_hitboxes()
	if not game then return end
	update_globals()
	if globals.game_phase == GAME_PHASE_NOT_PLAYING then
		return
	end

	for p = 1, NUMBER_OF_PLAYERS do
		local base_obj = game.address.player + (p-1) * game.player_space
		update_game_object(player[p], base_obj)
	end
	read_projectiles()

	for f = 1, DRAW_DELAY do
		for p = 1, NUMBER_OF_PLAYERS do
			frame_buffer_array[f][player][p] = copytable(frame_buffer_array[f+1][player][p])
		end
		for i = 1, globals.num_projectiles do
			frame_buffer_array[f][projectiles][i] = copytable(frame_buffer_array[f+1][projectiles][i])
		end
	end

	for p = 1, NUMBER_OF_PLAYERS do
		frame_buffer_array[DRAW_DELAY+1][player][p] = copytable(player[p])
	end
	for i = 1, globals.num_projectiles do
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
	local left   = game_x_to_mame(hb.left)
	local bottom = game_y_to_mame(hb.bottom)
	local right  = game_x_to_mame(hb.right)
	local top    = game_y_to_mame(hb.top)
	local hval   = game_x_to_mame(hb.hval)
	local vval   = game_y_to_mame(hb.vval)

	if left >= right or bottom >= top then return end

	if DRAW_MINI_AXIS then
		gui.drawline(hval, vval-MINI_AXIS_SIZE, hval, vval+MINI_AXIS_SIZE, OR(color, 0xFF))
		gui.drawline(hval-MINI_AXIS_SIZE, vval, hval+MINI_AXIS_SIZE, vval, OR(color, 0xFF))
	end

	gui.box(left, top, right, bottom, color)
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
		gui.clearuncommitted()
		return
	end

	if BLANK_SCREEN then
		gui.box(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLANK_COLOR)
	end

	if DRAW_AXIS then
		for p = 1, NUMBER_OF_PLAYERS do
			draw_game_object(frame_buffer_array[1][player][p])
		end
		for i = 1, globals.num_projectiles do
			draw_game_object(frame_buffer_array[1][projectiles][i])
		end
	end

	for entry in ipairs(game.boxes) do
		for p = 1, NUMBER_OF_PLAYERS do
			local obj = frame_buffer_array[1][player][p]
			if obj and obj[entry] and not (not DRAW_PUSHBOXES and game.boxes[entry].type == PUSH_BOX) then
				draw_hitbox(obj[entry], game.boxes[entry].color)
			end
		end

		for i = 1, globals.num_projectiles do
			local obj = frame_buffer_array[1][projectiles][i]
			if obj and obj[entry] then
				draw_hitbox(obj[entry], game.boxes[entry].projectile_color)
			end
		end
	end
end


gui.register( function()
	render_cps2_hitboxes()
end)