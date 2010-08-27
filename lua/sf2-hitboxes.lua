print("Street Fighter II hitbox viewer")
print("August 26, 2010")
print("http://code.google.com/p/mame-rr/") print()

local DRAW_DELAY            = 2
local VULNERABILITY_COLOUR  = 0x0000FF40
local ATTACK_COLOUR         = 0xFF000060
local PUSH_COLOUR           = 0x00FF0060
local WEAK_COLOUR           = 0xFFFF0060
local PROJ_ATTACK_COLOUR    = 0xFF000060
local PROJ_PUSH_COLOUR      = 0x0000FF40
local AXIS_COLOUR           = 0xFFFFFFFF
local MINI_AXIS_COLOUR      = 0xFFFF00FF
local DRAW_AXIS             = false
local DRAW_MINI_AXIS        = false
local AXIS_SIZE             = 16
local MINI_AXIS_SIZE        = 2
local BLANK_COLOUR          = 0xFFFFFFFF

local SCREEN_WIDTH          = 384
local SCREEN_HEIGHT         = 224
local NUMBER_OF_PLAYERS     = 2
local MAX_GAME_PROJECTILES  = 8
local HITBOX_VULNERABILITY  = 0
local HITBOX_ATTACK         = 1
local HITBOX_PUSH           = 2
local HITBOX_WEAK           = 3
local GAME_PHASE_NOT_PLAYING= 0
local BLANK_SCREEN          = false

local profile = {
	{
		games = {"sf2"},
		delay = 0,
		address = {
			player           = 0xFF83C6,
			projectile       = 0xFF938A,
			left_screen_edge = 0xFF8BD8,
			top_screen_edge  = 0xFF8BDC,
			game_phase       = 0xFF83F7,
		},
		offset = {
			player_space     = 0x300,
			parameter_space  = 0x1,
			v_hb_addr_table  = 0x0,
			w_hb_addr_table  = 0x6,
			a_hb_addr_table  = 0x8,
			p_hb_addr_table  = 0xA,
			v_hb_curr_id     = 0x8,
			w_hb_curr_id     = 0xB,
			a_hb_curr_id     = 0xC,
			p_hb_curr_id     = 0xD,
			a_hb_id_space    = 0x0C,
		},
	},
	{
		games = {"sf2ce","sf2hf"},
		delay = 0,
		address = {
			player           = 0xFF83BE,
			projectile       = 0xFF9376,
			left_screen_edge = 0xFF8BC4,
			top_screen_edge  = 0xFF8BC8,
			game_phase       = 0xFF83EF,
		},
		offset = {
			player_space     = 0x300,
			parameter_space  = 0x1,
			v_hb_addr_table  = 0x0,
			w_hb_addr_table  = 0x6,
			a_hb_addr_table  = 0x8,
			p_hb_addr_table  = 0xA,
			v_hb_curr_id     = 0x8,
			w_hb_curr_id     = 0xB,
			a_hb_curr_id     = 0xC,
			p_hb_curr_id     = 0xD,
			a_hb_id_space    = 0x0C,
		},
	},
	{
		games = {"ssf2"},
		delay = 0,
		address = {
			player           = 0xFF83CE,
			projectile       = 0xFF96A2,
			left_screen_edge = 0xFF8DD4,
			top_screen_edge  = 0xFF8DD8,
			game_phase       = 0xFF83FF,
		},
		offset = {
			player_space     = 0x400,
			parameter_space  = 0x1,
			v_hb_addr_table  = 0x0,
			a_hb_addr_table  = 0x6,
			p_hb_addr_table  = 0x8,
			v_hb_curr_id     = 0x8,
			a_hb_curr_id     = 0xC,
			p_hb_curr_id     = 0xD,
			a_hb_id_space    = 0x0C,
		},
	},
	{
		games = {"ssf2t"},
		delay = 1,
		address = {
			player           = 0xFF844E,
			projectile       = 0xFF97A2,
			left_screen_edge = 0xFF8ED4,
			top_screen_edge  = 0xFF8ED8,
			game_phase       = 0xFF847F,
		},
		offset = {
			player_space     = 0x400,
			parameter_space  = 0x1,
			v_hb_addr_table  = 0x0,
			a_hb_addr_table  = 0x6,
			p_hb_addr_table  = 0x8,
			v_hb_curr_id     = 0x8,
			a_hb_curr_id     = 0xC,
			p_hb_curr_id     = 0xD,
			a_hb_id_space    = 0x10,
		},
	},
	{
		games = {"hsf2"},
		delay = 1,
		address = {
			player           = 0xFF833C,
			projectile       = 0xFF9554,
			left_screen_edge = 0xFF8CC2,
			top_screen_edge  = 0xFF8CC6,
			game_phase       = 0xFF836D,
		},
		offset = {
			player_space     = 0x400,
			parameter_space  = 0x2,
			v_hb_addr_table  = 0x0,
			w_hb_addr_table  = 0x6,
			a_hb_addr_table  = 0x8,
			p_hb_addr_table  = 0xA,
			v_hb_curr_id     = 0x8,
			w_hb_curr_id     = 0xB,
			a_hb_curr_id     = 0xC,
			p_hb_curr_id     = 0xD,
			a_hb_id_space    = 0x14,
		},
	},
}

local address, offset, effective_delay
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
end)


local function update_globals()
	globals.left_screen_edge = memory.readword(address.left_screen_edge)
	globals.top_screen_edge  = memory.readword(address.top_screen_edge)
	globals.game_phase       = memory.readword(address.game_phase)
end


local function hitbox_load(obj, i, type, facing_dir, offset_x, offset_y, addr)
	local hval, hval2, vval, hrad, vrad

	if offset.parameter_space == 1 then
		hval   = memory.readbytesigned(addr + 0)
		hval2  = memory.readbyte(addr + 5)
		if type == HITBOX_ATTACK and hval2 >= 0x80 then
			hval = -hval2
		end
		vval   = memory.readbytesigned(addr + 1)
		hrad   = memory.readbytesigned(addr + 2)
		vrad   = memory.readbytesigned(addr + 3)
	elseif offset.parameter_space == 2 then
		hval   = memory.readwordsigned(addr + 0)
		vval   = memory.readwordsigned(addr + 2)
		hrad   = memory.readwordsigned(addr + 4)
		vrad   = memory.readwordsigned(addr + 6)
	end

	if facing_dir == 1 then
		hval   = -hval
	end

	local box_dimensions  = {
		left   = offset_x + hval - hrad,
		right  = offset_x + hval + hrad,
		bottom = offset_y + vval + vrad,
		top    = offset_y + vval - vrad,
		hval   = offset_x + hval,
		vval   = offset_y + vval,
	}

	if type == HITBOX_VULNERABILITY then
		obj[HITBOX_VULNERABILITY][i] = box_dimensions
	else
		obj[type] = box_dimensions
	end
end


local function get_vulnbox(obj, base, is_projectile, animation_ptr)
	-- Load the vulnerability hitboxes
	obj[HITBOX_VULNERABILITY] = {}
	for i = 0, 2 do
		local v_hb_addr_table = memory.readdword(base + 0x34)
		v_hb_addr_table = memory.readword(v_hb_addr_table + offset.v_hb_addr_table + i*2) + v_hb_addr_table
		local v_hb_curr_id = memory.readbyte(animation_ptr + offset.v_hb_curr_id + i)
		hitbox_load(obj, i, HITBOX_VULNERABILITY, obj.facing_dir, obj.pos_x, obj.pos_y, v_hb_addr_table + v_hb_curr_id*4*offset.parameter_space)
	end
end


local function get_weakbox(obj, base, is_projectile, animation_ptr)
	local w_hb_addr_table = memory.readdword(base + 0x34)
	w_hb_addr_table = memory.readword(w_hb_addr_table + offset.w_hb_addr_table) + w_hb_addr_table
	local w_hb_curr_id = memory.readbyte(animation_ptr + offset.w_hb_curr_id)
	hitbox_load(obj, 0, HITBOX_WEAK, obj.facing_dir, obj.pos_x, obj.pos_y, w_hb_addr_table + w_hb_curr_id*4*offset.parameter_space)
end


local function get_hurtbox(obj, base, is_projectile, animation_ptr)
	-- Load the attack hitbox
	local a_hb_addr_table = memory.readdword(base + 0x34)
	a_hb_addr_table = memory.readword(a_hb_addr_table + offset.a_hb_addr_table) + a_hb_addr_table
	local a_hb_curr_id = memory.readbyte(animation_ptr + offset.a_hb_curr_id)
	hitbox_load(obj, 0, HITBOX_ATTACK, obj.facing_dir, obj.pos_x, obj.pos_y, a_hb_addr_table + a_hb_curr_id*offset.a_hb_id_space)
end


local function get_pushbox(obj, base, is_projectile, animation_ptr)
	-- Load the push hitbox
	local p_hb_addr_table = memory.readdword(base + 0x34)
	p_hb_addr_table = memory.readword(p_hb_addr_table + offset.p_hb_addr_table) + p_hb_addr_table
	local p_hb_curr_id = memory.readbyte(animation_ptr + offset.p_hb_curr_id)
	hitbox_load(obj, 0, HITBOX_PUSH, obj.facing_dir, obj.pos_x, obj.pos_y, p_hb_addr_table + p_hb_curr_id*4*offset.parameter_space)
end


local function update_game_object(obj, base, is_projectile)
	obj.facing_dir   = memory.readbyte(base + 0x12)
	obj.pos_x        = memory.readword(base + 0x06)
	obj.pos_y        = memory.readword(base + 0x0A)

	local animation_ptr = memory.readdword(base + 0x1A)

	get_vulnbox(obj, base, is_projectile, animation_ptr)
	if offset.w_hb_addr_table then
		get_weakbox(obj, base, is_projectile, animation_ptr)
	end
	get_hurtbox(obj, base, is_projectile, animation_ptr)
	get_pushbox(obj, base, is_projectile, animation_ptr)
end


local function read_projectiles()
	globals.num_projectiles = 0
	for i = 1,MAX_GAME_PROJECTILES do
		local base = address.projectile + (i-1) * 0xC0

		if memory.readbyte(base+1) ~= 0 then
			projectiles[globals.num_projectiles] = {}
			update_game_object(projectiles[globals.num_projectiles], base, true)
			globals.num_projectiles = globals.num_projectiles+1
		end
	end
end


local function game_x_to_mame(x)
	return (x - globals.left_screen_edge)
end


local function game_y_to_mame(y)
	-- Why subtract 17? No idea, the game driver does the same thing.
	return (SCREEN_HEIGHT - (y - 17) + globals.top_screen_edge)
end


local function draw_hitbox(hb, colour)
	if not hb then return end

	local left   = game_x_to_mame(hb.left)
	local bottom = game_y_to_mame(hb.bottom)
	local right  = game_x_to_mame(hb.right)
	local top    = game_y_to_mame(hb.top)
	local hval   = game_x_to_mame(hb.hval)
	local vval   = game_y_to_mame(hb.vval)

	if left - right == 0 and top - bottom == 0 then
		return
	end

	if DRAW_MINI_AXIS then
		gui.drawline(hval, vval-MINI_AXIS_SIZE, hval, vval+MINI_AXIS_SIZE, MINI_AXIS_COLOUR)
		gui.drawline(hval-MINI_AXIS_SIZE, vval, hval+MINI_AXIS_SIZE, vval, MINI_AXIS_COLOUR)
	end
	gui.box(left, top, right, bottom, colour)
end


local function draw_game_object(obj, is_projectile)
	if not obj then return end

	local x = game_x_to_mame(obj.pos_x)
	local y = game_y_to_mame(obj.pos_y)

	draw_hitbox(obj[HITBOX_PUSH], is_projectile and PROJ_PUSH_COLOUR or PUSH_COLOUR)
	for i = 0, 2 do
		draw_hitbox(obj[HITBOX_VULNERABILITY][i], VULNERABILITY_COLOUR)
	end
	draw_hitbox(obj[HITBOX_WEAK], WEAK_COLOUR)
	draw_hitbox(obj[HITBOX_ATTACK], is_projectile and PROJ_ATTACK_COLOUR or ATTACK_COLOUR)

	if DRAW_AXIS then
		gui.drawline(x, y-AXIS_SIZE, x, y+AXIS_SIZE, AXIS_COLOUR)
		gui.drawline(x-AXIS_SIZE, y, x+AXIS_SIZE, y, AXIS_COLOUR)
	end
end


local function whatgame()
	address, offset, effective_delay = nil, nil, nil
	for n, module in ipairs(profile) do
		for m, shortname in ipairs(module.games) do
			if emu.romname() == shortname or emu.parentname() == shortname then
				print("drawing " .. shortname .. " hitboxes")
				address = module.address
				offset = module.offset
				effective_delay = DRAW_DELAY + module.delay
				for p = 1, NUMBER_OF_PLAYERS do
					player[p] = {}
				end
				for f = 1, effective_delay+1 do
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


local function update_sf2_hitboxes()
	if not address then return end
	update_globals()
	if globals.game_phase == GAME_PHASE_NOT_PLAYING then
		return
	end

	for p = 1, NUMBER_OF_PLAYERS do
		update_game_object(player[p], address.player + (p-1) * offset.player_space)
	end
	read_projectiles()

	for f = 1, effective_delay do
		for p = 1, NUMBER_OF_PLAYERS do
			frame_buffer_array[f][player][p] = copytable(frame_buffer_array[f+1][player][p])
		end
		for i = 0, globals.num_projectiles-1 do
			frame_buffer_array[f][projectiles][i] = copytable(frame_buffer_array[f+1][projectiles][i])
		end
	end

	for p = 1, NUMBER_OF_PLAYERS do
		frame_buffer_array[effective_delay+1][player][p] = copytable(player[p])
	end
	for i = 0, globals.num_projectiles-1 do
		frame_buffer_array[effective_delay+1][projectiles][i] = copytable(projectiles[i])
	end
end


local function render_sf2_hitboxes()
	if not address or globals.game_phase == GAME_PHASE_NOT_PLAYING then
		gui.clearuncommitted()
		return
	end

	if BLANK_SCREEN then
		gui.box(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLANK_COLOUR)
	end

	for p = 1, NUMBER_OF_PLAYERS do
		draw_game_object(frame_buffer_array[1][player][p])
	end

	for i = 0, globals.num_projectiles-1 do
		draw_game_object(frame_buffer_array[1][projectiles][i], true)
	end
end


emu.registerstart( function()
	whatgame()
end)


emu.registerafter( function()
	update_sf2_hitboxes()
end)


gui.register( function()
	render_sf2_hitboxes()
end)