print("Capcom beat 'em up hitbox viewer")
print("September 24, 2010")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis")
print("Lua hotkey 4: toggle pushboxes") print()

local VULNERABILITY_COLOR    = 0x7777FF40
local ATTACK_COLOR           = 0xFF000060
local PUSH_COLOR             = 0x00FF0040
local WEAK_COLOR             = 0xFFFF0060
local AXIS_COLOR             = 0xFFFFFFFF
local BLANK_COLOR            = 0xFFFFFFFF
local AXIS_SIZE              = 16
local MINI_AXIS_SIZE         = 2
local DRAW_DELAY             = 1

local SCREEN_WIDTH           = 384
local SCREEN_HEIGHT          = 224
local VULNERABILITY_BOX      = 1
local WEAK_BOX               = 2
local ATTACK_BOX             = 3
local PUSH_BOX               = 4
local GAME_PHASE_NOT_PLAYING = 0
local BLANK_SCREEN           = false
local DRAW_AXIS              = false
local DRAW_MINI_AXIS         = false
local DRAW_PUSHBOXES         = true

local profile = {
	{
		games = {"ffight"},
		number = {players = 29, projectiles = 16},
		address = {
			player           = 0xFF8568,
			projectile       = 0xFFB2E8,
			left_screen_edge = 0xFF8412,
			game_phase       = 0xFF8622,
		},
		offset = {
			player_space     = 0xC0,
			projectile_space = 0xC0,
			facing_dir       = 0x2E,
			x_position       = 0x06,
			hitbox_ptr       = {player = 0x38, projectile = 0x38},
			invulnerability  = {},
			hval = 0x0, vval = 0x2, hrad = 0x4, vrad = 0x6,
		},
		boxes = {
			{anim_ptr = 0x24, addr_table = 0x02, id_ptr = 0x04, id_space = 0x08, type = VULNERABILITY_BOX},
			{anim_ptr = 0x24, addr_table = 0x00, id_ptr = 0x05, id_space = 0x10, type = ATTACK_BOX},
		},
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
		elseif box.type == PUSH_BOX then
			box.color = PUSH_COLOR
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
	globals.left_screen_edge = memory.readword(game.address.left_screen_edge)
	globals.top_screen_edge  = memory.readword(game.address.left_screen_edge + 0x4)
	globals.game_phase       = memory.readword(game.address.game_phase)
end


local function game_x_to_mame(x)
	return (x - globals.left_screen_edge)
end


local function game_y_to_mame(y)
	-- Why subtract 17? No idea, the game driver does the same thing.
	return (SCREEN_HEIGHT - (y - 17) + globals.top_screen_edge)
end


local function define_box(obj, entry, base_obj, is_projectile, hitbox_ptr)
	local base_id = base_obj
	if game.boxes[entry].anim_ptr then
		base_id = memory.readdword(base_obj + game.boxes[entry].anim_ptr)
	end
	local curr_id = memory.readbytesigned(base_id + game.boxes[entry].id_ptr)

	if base_id == 0 or curr_id <= 0 then
		obj[entry] = nil
		return
	end
	
	local addr_table
	if not hitbox_ptr then
		addr_table = memory.readdword(base_obj + game.boxes[entry].addr_table)
	else
		local table_offset = is_projectile and game.boxes[entry].p_addr_table or game.boxes[entry].addr_table
		addr_table = memory.readdword(base_obj + hitbox_ptr)
		addr_table = addr_table + memory.readwordsigned(addr_table + table_offset)
	end
	local address = addr_table + curr_id * game.boxes[entry].id_space

	local hval = memory.readwordsigned(address + game.offset.hval)
	local vval = memory.readwordsigned(address + game.offset.vval)
	local hrad = memory.readwordsigned(address + game.offset.hrad)
	local vrad = memory.readwordsigned(address + game.offset.vrad)

	if obj.facing_dir == 1 then
		hval  = -hval
	end

	obj[entry] = {
		left   = game_x_to_mame(obj.pos_x + hval - hrad),
		right  = game_x_to_mame(obj.pos_x + hval + hrad),
		bottom = game_y_to_mame(obj.pos_y + vval + vrad),
		top    = game_y_to_mame(obj.pos_y + vval - vrad),
		hval   = game_x_to_mame(obj.pos_x + hval),
		vval   = game_y_to_mame(obj.pos_y + vval),
		type   = game.boxes[entry].type,
		color  = game.boxes[entry].color,
	}
end


local function update_game_object(obj, base_obj, is_projectile)
	obj.facing_dir   = memory.readbyte(base_obj + game.offset.facing_dir)
	obj.pos_x        = memory.readword(base_obj + game.offset.x_position)
	obj.pos_y        = memory.readword(base_obj + game.offset.x_position + 0x4)
	--obj.opponent_dir = memory.readbyte(base_obj + 0x5D)

	local hitbox_ptr
	if not is_projectile then
		hitbox_ptr = game.offset.hitbox_ptr.player
	else
		hitbox_ptr = game.offset.hitbox_ptr.projectile
	end

	for entry in ipairs(game.boxes) do
		define_box(obj, entry, base_obj, is_projectile, hitbox_ptr)
	end
end


local function read_projectiles()
	local current_projectiles = {}

	for i = 1, game.number.projectiles do
		local base_obj = game.address.projectile + (i-1) * game.offset.projectile_space
		if memory.readword(base_obj) > 0x0100 then
			local obj = {}
			update_game_object(obj, base_obj, true)
			table.insert(current_projectiles, obj)
		end
	end

	return current_projectiles
end


local function update_invulnerability(player, base_obj)
	player.invulnerability = false
	for _,address in ipairs(game.offset.invulnerability) do
		if memory.readbyte(base_obj + address) > 0 then
			player.invulnerability = true
		end
	end
end


local function update_cps2_hitboxes()
	gui.clearuncommitted()
	if not game then return end
	update_globals()

	for p = 1, game.number.players do
		local base_obj = game.address.player + (p-1) * game.offset.player_space
		if memory.readbyte(base_obj) > 0 then
			update_game_object(player[p], base_obj)
			update_invulnerability(player[p], base_obj)
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
	if invulnerability and hb.type == VULNERABILITY_BOX then return end
	if hb.left > hb.right or hb.bottom > hb.top then return end

	if DRAW_MINI_AXIS then
		gui.drawline(hb.hval, hb.vval-MINI_AXIS_SIZE, hb.hval, hb.vval+MINI_AXIS_SIZE, OR(hb.color, 0xFF))
		gui.drawline(hb.hval-MINI_AXIS_SIZE, hb.vval, hb.hval+MINI_AXIS_SIZE, hb.vval, OR(hb.color, 0xFF))
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, hb.color)
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

	if DRAW_AXIS then
		for p = 1, game.number.players do
			draw_game_object(frame_buffer[1][player][p])
		end
		for i in ipairs(frame_buffer[1][projectiles]) do
			draw_game_object(frame_buffer[1][projectiles][i])
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
	render_cps2_hitboxes()
end)