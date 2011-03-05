print("NeoGeo hitbox viewer")
print("March 5, 2011")
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
local DRAW_DELAY             = 0

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
local DRAW_THROWBOXES        = true

local fill = {
	[VULNERABILITY_BOX] = VULNERABILITY_COLOR,
	[ATTACK_BOX]        = ATTACK_COLOR,
	[PUSH_BOX]          = PUSH_COLOR,
	[THROW_BOX]         = THROW_COLOR,
	[THROWABLE_BOX]     = THROWABLE_COLOR,
}

local outline = {
	[VULNERABILITY_BOX] = bit.bor(VULNERABILITY_COLOR, 0xFF),
	[ATTACK_BOX]        = bit.bor(ATTACK_COLOR,        0xFF),
	[PUSH_BOX]          = bit.bor(PUSH_COLOR,          0xC0),
	[THROW_BOX]         = bit.bor(THROW_COLOR,         0xFF),
	[THROWABLE_BOX]     = bit.bor(THROWABLE_COLOR,     0xE0),
}

local profile = {
	{
		games = {"kof2002"},
		nplayers = 2,
		address = {
			player           = 0x108100,
			obj_ptr_list     = 0x10BEE6,
			left_screen_edge = 0x10B08E,
			top_screen_edge  = 0x10B096,
			game_phase       = 0x10B056,
		},
		offset = {
			player_space     = 0x200,
			x_position       = 0x18,
			y_position       = 0x20,
			facing_dir       = 0x31,
			status           = 0x7C,
			invulnerability  = {},
			hval = 0x1, vval = 0x2, hrad = 0x3, vrad = 0x4,
		},
		boxes = {
			{offset = 0xA4, type = PUSH_BOX},
			{offset = 0x95, active_bit = 1, type = VULNERABILITY_BOX},
			{offset = 0x9A, active_bit = 2, type = VULNERABILITY_BOX},
			{offset = 0x9F, active_bit = 3, type = VULNERABILITY_BOX},
			{offset = 0x90, active_bit = 0, type = ATTACK_BOX},
		},
		box_radius_read = memory.readbyte,
		inactive_projectile = 0x80,
	},
}

for game in ipairs(profile) do
	local g = profile[game]
	g.address.top_screen_edge = g.address.top_screen_edge or g.address.left_screen_edge + 0x4
	g.offset.top_screen_edge  = g.offset.top_screen_edge  or 21
	g.offset.x_position       = g.offset.x_position       or 0x10
	g.offset.y_position       = g.offset.y_position       or g.offset.x_position + 0x4
	g.offset.unpushability    = g.offset.unpushability    or {}
	g.box_offset_read         = g.box_radius_read == memory.readbyte and memory.readbytesigned or memory.readwordsigned
	for _,box in ipairs(g.boxes) do
		box.active = box.active_bit and bit.lshift(1, box.active_bit)
	end
end

local game, prepare_boxes
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
-- prepare the hitboxes

local function update_globals()
	globals.left_screen_edge = memory.readword(game.address.left_screen_edge)
	globals.top_screen_edge  = memory.readword(game.address.top_screen_edge)
	globals.game_phase       = memory.readword(game.address.game_phase)
end


local function game_x_to_mame(x)
	return x - globals.left_screen_edge
end


local function game_y_to_mame(y)
	return emu.screenheight() - (y + game.offset.top_screen_edge) + globals.top_screen_edge
end


local function define_box(obj, entry)
	if game.boxes[entry].type == PUSH_BOX then
		if memory.readbytesigned(obj.base + game.boxes[entry].offset) < 0 then
			return nil
		end
	elseif game.boxes[entry].active and bit.band(obj.status, game.boxes[entry].active) == 0 then
		return nil
	end

	local address = obj.base + game.boxes[entry].offset

	local hval = game.box_offset_read(address + game.offset.hval)
	local vval = game.box_offset_read(address + game.offset.vval)
	local hrad = game.box_radius_read(address + game.offset.hrad)
	local vrad = game.box_radius_read(address + game.offset.vrad)

	if bit.band(obj.facing_dir, 1) > 0 then
		hval = -hval
	end

	return {
		left   = obj.pos_x + hval - hrad,
		right  = obj.pos_x + hval + hrad,
		top    = obj.pos_y + vval - vrad,
		bottom = obj.pos_y + vval + vrad,
		hval   = obj.pos_x + hval,
		vval   = obj.pos_y + vval,
		type   = game.boxes[entry].type,
	}
end


local function add_object(address, projectile)
	local obj = {base = address}
	obj.status = memory.readbyte(obj.base + game.offset.status)
	if projectile and bit.band(obj.status, game.inactive_projectile) > 0 then
		return nil
	end
	obj.facing_dir = bit.band(memory.readbyte(obj.base + game.offset.facing_dir), 1)
	obj.pos_x = game_x_to_mame(memory.readwordsigned(obj.base + game.offset.x_position))
	obj.pos_y = game_y_to_mame(memory.readwordsigned(obj.base + game.offset.y_position))
	for entry in ipairs(game.boxes) do
		obj[entry] = define_box(obj, entry)
	end
	return obj
end


local function update_invulnerability(player)
	player.invulnerability = false
	for _,address in ipairs(game.offset.invulnerability) do
		if memory.readbyte(player.base + address) > 0 then
			player.invulnerability = true
		end
	end
end


local function update_unpushability(player)
	player.unpushability = false
	for _,address in ipairs(game.offset.unpushability) do
		if memory.readbyte(player.base + address) > 0 then
			player.unpushability = true
		end
	end
end


local function read_objects()
	local offset, objects = 0, {}
	while true do
		local address = 0x100000 + memory.readword(game.address.obj_ptr_list + offset)
		if address == 0x100000 then
			return objects
		end
		for _, object in ipairs(objects) do
			if address == object.base then
				return objects
			end
		end
		table.insert(objects, add_object(address, true))
		offset = offset + 2
	end
end


local function update_neogeo_hitboxes()
	gui.clearuncommitted()
	if not game then
		return
	end
	update_globals()

	for f = 1, DRAW_DELAY do
		frame_buffer[f] = copytable(frame_buffer[f+1])
	end
	frame_buffer[DRAW_DELAY+1] = read_objects()

	for p = 1, game.nplayers do
		table.insert(frame_buffer[DRAW_DELAY+1], add_object(game.address.player + game.offset.player_space * (p-1)))
	end
end


emu.registerafter( function()
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
		gui.drawline(hb.hval, hb.vval-MINI_AXIS_SIZE, hb.hval, hb.vval+MINI_AXIS_SIZE, bit.bor(fill[hb.type], 0xFF))
		gui.drawline(hb.hval-MINI_AXIS_SIZE, hb.vval, hb.hval+MINI_AXIS_SIZE, hb.vval, bit.bor(fill[hb.type], 0xFF))
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, fill[hb.type], outline[hb.type])
end


local function draw_axis(obj)
	if not (obj.pos_x and obj.pos_y) then
		return
	end
	
	gui.drawline(obj.pos_x, obj.pos_y-AXIS_SIZE, obj.pos_x, obj.pos_y+AXIS_SIZE, AXIS_COLOR)
	gui.drawline(obj.pos_x-AXIS_SIZE, obj.pos_y, obj.pos_x+AXIS_SIZE, obj.pos_y, AXIS_COLOR)
	gui.text(obj.pos_x, obj.pos_y, string.format("%06X",obj.base)) --debug
	gui.text(obj.pos_x, obj.pos_y+8, string.format("%X",obj.status)) --debug
end


local function render_neogeo_hitboxes()
	if not game or globals.game_phase == GAME_PHASE_NOT_PLAYING then
		return
	end

	if BLANK_SCREEN then
		gui.box(0, 0, emu.screenwidth(), emu.screenheight(), BLANK_COLOR)
	end

	for entry in ipairs(game.boxes) do
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