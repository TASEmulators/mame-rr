print("NeoGeo hitbox viewer")
print("March 8, 2011")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis")
print("Lua hotkey 4: toggle pushboxes")
print("Lua hotkey 5: toggle throwboxes") print()

local VULNERABILITY_COLOR      = 0x7777FF40
local ATTACK_COLOR             = 0xFF000060
local PROJ_VULNERABILITY_COLOR = 0x77CCFF40
local PROJ_ATTACK_COLOR        = 0xFF66FF60
local PUSH_COLOR               = 0x00FF0040
local AUTOGUARD_COLOR          = 0xCCCCCC60
local THROW_COLOR              = 0xFFFF0060
local AXIS_COLOR               = 0xFFFFFFFF
local BLANK_COLOR              = 0xFFFFFFFF
local AXIS_SIZE                = 16
local MINI_AXIS_SIZE           = 2
local DRAW_DELAY               = 0
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
local AUTOGUARD_BOX          = 6
local THROW_BOX              = 7

local fill = {
	VULNERABILITY_COLOR,
	ATTACK_COLOR,
	PROJ_VULNERABILITY_COLOR,
	PROJ_ATTACK_COLOR,
	PUSH_COLOR,
	AUTOGUARD_COLOR,
	THROW_COLOR,
}

local outline = {
	bit.bor(0xFF, VULNERABILITY_COLOR),
	bit.bor(0xFF, ATTACK_COLOR),
	bit.bor(0xFF, PROJ_VULNERABILITY_COLOR),
	bit.bor(0xFF, PROJ_ATTACK_COLOR),
	bit.bor(0xC0, PUSH_COLOR),
	bit.bor(0xC0, AUTOGUARD_COLOR),
	bit.bor(0xFF, THROW_COLOR),
}

local profile = {
	{
		games = {"kof94"},
		address = {
			player           = 0x108100,
			game_phase       = 0x1090B3,
			left_screen_edge = 0x10904C,
			top_screen_edge  = 0x10905C,
			obj_ptr_list     = 0x1097A6,
		},
		offset = {status = 0x7A},
	},
	{
		games = {"kof95"},
		address = {game_phase = 0x10B088},
	},
	{
		games = {"kof96"},
		address = {game_phase = 0x10B08E},
	},
	{
		games = {"kof97"},
		address = {game_phase = 0x10B092},
	},
	{
		games = {"kof98"},
		address = {game_phase = 0x10B094},
	},
	{
		games = {"kof99", "kof2000"},
		address = {game_phase = 0x10B048},
	},
	{
		games = {"kof2001", "kof2002"},
		address = {game_phase = 0x10B056},
	},
}

for game in ipairs(profile) do
	local g = profile[game]
	g.nplayers     = g.nplayers or 2
	g.ground_level = g.ground_level or 16
	g.address.player           = g.address.player or 0x108100
	g.address.left_screen_edge = g.address.left_screen_edge or g.address.game_phase + 0x038
	g.address.top_screen_edge  = g.address.top_screen_edge or g.address.game_phase + 0x040
	g.address.obj_ptr_list     = g.address.obj_ptr_list or g.address.game_phase + 0xE90
	g.offset = g.offset or {}
	g.offset.player_space    = g.offset.player_space or 0x200
	g.offset.x_position      = g.offset.x_position or 0x18
	g.offset.y_position      = g.offset.y_position or 0x26
	g.offset.facing_dir      = g.offset.facing_dir or 0x31
	g.offset.status          = g.offset.status or 0x7C
	g.offset.invulnerability = g.offset.invulnerability or {}
	g.offset.unpushability   = g.offset.unpushability or {}
	g.box = g.box or {
		radius_read = memory.readbyte,
		offset_read = memory.readbytesigned,
		hval = 0x1, vval = 0x2, hrad = 0x3, vrad = 0x4,
	}
	g.box_list = g.box_list or {
		{offset = 0xA4, type = PUSH_BOX},
		{offset = 0x95, active_bit = 1, type = VULNERABILITY_BOX},
		{offset = 0x9A, active_bit = 2, type = VULNERABILITY_BOX},
		{offset = 0x9F, active_bit = 3, type = VULNERABILITY_BOX},
		{offset = 0x90, active_bit = 0, type = ATTACK_BOX},
	}
	for _, box in ipairs(g.box_list) do
		box.active = box.active_bit and bit.lshift(1, box.active_bit)
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
--memory.writebyte(0x100000, 2)

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
-- prepare the hitboxes

local function game_x_to_mame(x)
	return x - globals.left_screen_edge
end


local function game_y_to_mame(y)
	--return emu.screenheight() - (y + game.ground_level) + globals.top_screen_edge
	return y - game.ground_level
end


local function define_box(obj, entry)
	local box_type = game.box_list[entry].type

	if box_type == PUSH_BOX then
		if memory.readbytesigned(obj.base + game.box_list[entry].offset) < 0 then
			return nil
		end
	elseif game.box_list[entry].active and bit.band(obj.status, game.box_list[entry].active) == 0 then
		return nil
	end

	local address = obj.base + game.box_list[entry].offset
	local id = memory.readbyte(address)
	for _, value in ipairs({0x2,0x3,0x4,0x9,0xA}) do
		if id == value then
			box_type = VULNERABILITY_BOX
			break
		end
	end
	if id == 0xB then
		box_type = AUTOGUARD_BOX
	end

	if obj.projectile then
		if box_type == VULNERABILITY_BOX then
			box_type = PROJ_VULNERABILITY_BOX
		elseif box_type == ATTACK_BOX then
			box_type = PROJ_ATTACK_BOX
		else
			return nil
		end
	end

	local hval = game.box.offset_read(address + game.box.hval)
	local vval = game.box.offset_read(address + game.box.vval)
	local hrad = game.box.radius_read(address + game.box.hrad)
	local vrad = game.box.radius_read(address + game.box.vrad)
	hval = obj.pos_x + hval * (bit.band(obj.facing_dir, 1) > 0 and -1 or 1)
	vval = obj.pos_y + vval

	return {
		type   = box_type,
		id     = id,
		pos_x  = hval,
		pos_y  = vval,
		left   = hval - hrad,
		right  = hval + hrad - 1,
		top    = vval - vrad,
		bottom = vval + vrad - 1,
	}
end


local function update_invulnerability(obj)
	obj.invulnerability = false
	for _, address in ipairs(game.offset.invulnerability) do
		if memory.readbyte(obj.base + address) > 0 then
			obj.invulnerability = true
		end
	end
end


local function update_unpushability(obj)
	obj.unpushability = false
	for _, address in ipairs(game.offset.unpushability) do
		if memory.readbyte(obj.base + address) > 0 then
			obj.unpushability = true
		end
	end
end


local function add_object(address, projectile)
	local obj = {base = address, projectile = projectile}
	obj.status = memory.readbyte(obj.base + game.offset.status)
	obj.pos_x = game_x_to_mame(memory.readwordsigned(obj.base + game.offset.x_position))
	obj.pos_y = game_y_to_mame(memory.readwordsigned(obj.base + game.offset.y_position))
	obj.facing_dir = bit.band(memory.readbyte(obj.base + game.offset.facing_dir), 1)
	for entry in ipairs(game.box_list) do
		obj[entry] = define_box(obj, entry)
	end
	return obj
end


local function read_objects(objects)
	local offset = 0
	while true do
		local address = memory.readword(game.address.obj_ptr_list + offset)
		if address == 0 or memory.readwordsigned(bit.bor(0x100000, address) + 0x6) < 0 then
			return
		end
		for _, object in ipairs(objects) do
			if address == bit.band(object.base, 0xFFFF) then
				return
			end
		end
		table.insert(objects, add_object(bit.bor(0x100000, address), true))
		offset = offset + 2
	end
end


local function bios_test(address)
	local ram_value = memory.readword(address)
	for _, test_value in ipairs({0x5555, 0xAAAA, bit.band(0xFFFF, address)}) do
		if ram_value == test_value then
			return true
		end
	end
end


local function update_neogeo_hitboxes()
	gui.clearuncommitted()
	if not game or bios_test(game.address.obj_ptr_list) then
		return
	end
	globals.game_phase       = memory.readbyte(game.address.game_phase)
	globals.left_screen_edge = memory.readword(game.address.left_screen_edge)
	globals.top_screen_edge  = memory.readword(game.address.top_screen_edge)

	for f = 1, DRAW_DELAY do
		frame_buffer[f] = copytable(frame_buffer[f+1])
	end

	frame_buffer[DRAW_DELAY+1] = {}
	for p = 1, game.nplayers do
		table.insert(frame_buffer[DRAW_DELAY+1], add_object(game.address.player + game.offset.player_space * (p-1)))
	end
	read_objects(frame_buffer[DRAW_DELAY+1])
end


emu.registerbefore( function()
	update_neogeo_hitboxes()
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
		gui.drawline(hb.pos_x, hb.pos_y-MINI_AXIS_SIZE, hb.pos_x, hb.pos_y+MINI_AXIS_SIZE, outline[hb.type])
		gui.drawline(hb.pos_x-MINI_AXIS_SIZE, hb.pos_y, hb.pos_x+MINI_AXIS_SIZE, hb.pos_y, outline[hb.type])
		gui.text(hb.pos_x, hb.pos_y, string.format("%02X", hb.id)) --debug
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, fill[hb.type], outline[hb.type])
end


local function draw_axis(obj)
	if not (obj.pos_x and obj.pos_y) then
		return
	end
	
	gui.drawline(obj.pos_x, obj.pos_y-AXIS_SIZE, obj.pos_x, obj.pos_y+AXIS_SIZE, AXIS_COLOR)
	gui.drawline(obj.pos_x-AXIS_SIZE, obj.pos_y, obj.pos_x+AXIS_SIZE, obj.pos_y, AXIS_COLOR)
	gui.text(obj.pos_x, obj.pos_y, string.format("%06X", obj.base)) --debug
	gui.text(obj.pos_x, obj.pos_y+8, string.format("%02X", obj.status)) --debug
end


local function render_neogeo_hitboxes()
	if not game or not globals.game_phase or globals.game_phase == GAME_PHASE_NOT_PLAYING then
		return
	end

	if BLANK_SCREEN then
		gui.box(0, 0, emu.screenwidth(), emu.screenheight(), BLANK_COLOR)
	end

	for entry in ipairs(game.box_list) do
		for _, obj in ipairs(frame_buffer[1]) do
			if obj[entry] then
				draw_hitbox(obj, entry)
			end
		end
	end

	if DRAW_AXIS then
		for _, obj in ipairs(frame_buffer[1]) do
			draw_axis(obj)
		end
	end
end


gui.register( function()
	render_neogeo_hitboxes()
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
				for f = 1, DRAW_DELAY + 1 do
					frame_buffer[f] = {}
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