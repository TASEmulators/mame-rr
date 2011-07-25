print("CPS-2 fighting game hitbox viewer")
print("July 24, 2011")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis")
print("Lua hotkey 4: toggle pushboxes")
print("Lua hotkey 5: toggle throwable boxes")

local boxes = {
	      ["vulnerability"] = {color = 0x7777FF, fill = 0x40, outline = 0xFF},
	             ["attack"] = {color = 0xFF0000, fill = 0x40, outline = 0xFF},
	["proj. vulnerability"] = {color = 0x00FFFF, fill = 0x40, outline = 0xFF},
	       ["proj. attack"] = {color = 0xFF66FF, fill = 0x40, outline = 0xFF},
	               ["push"] = {color = 0x00FF00, fill = 0x20, outline = 0xFF},
	           ["tripwire"] = {color = 0xFF66FF, fill = 0x40, outline = 0xFF}, --sfa3
	             ["negate"] = {color = 0xFFFF00, fill = 0x40, outline = 0xFF}, --dstlk, nwarr
	              ["throw"] = {color = 0xFFFF00, fill = 0x40, outline = 0xFF},
	         ["axis throw"] = {color = 0xFFAA00, fill = 0x40, outline = 0xFF}, --sfa, sfa2, nwarr
	          ["throwable"] = {color = 0xF0F0F0, fill = 0x20, outline = 0xFF},
}

local globals = {
	axis_color           = 0xFFFFFFFF,
	blank_color          = 0xFFFFFFFF,
	axis_size            = 12,
	mini_axis_size       = 2,
	blank_screen         = false,
	draw_axis            = true,
	draw_mini_axis       = false,
	draw_pushboxes       = true,
	draw_throwable_boxes = false,
}

local function eval(list)
	for _, condition in ipairs(list) do
		if condition == true then return true end
	end
end

--------------------------------------------------------------------------------
-- game-specific modules

local profile = {
	{
		games = {"sfa"},
		number = {players = 3, projectiles = 8},
		address = {
			player           = 0xFF8400,
			projectile       = 0xFF9000,
			left_screen_edge = 0xFF8290,
		},
		offset = {
			projectile_space = 0x80,
			facing_dir       = 0x0B,
			hitbox_ptr       = {player = 0x50, projectile = 0x50},
		},
		box = {
			radius_read = memory.readbyte,
			offset_read = memory.readbytesigned,
			hval = 0x0, vval = 0x1, hrad = 0x2, vrad = 0x3,
		},
		box_list = {
			{anim_ptr = 0x20, addr_table_ptr = 0x08, p_addr_table_ptr = 0x4, id_ptr = 0x0C, id_shift = 0x2, type = "push"},
			{anim_ptr = 0x20, addr_table_ptr = 0x00, p_addr_table_ptr = 0x0, id_ptr = 0x08, id_shift = 0x2, type = "vulnerability"},
			{anim_ptr = 0x20, addr_table_ptr = 0x02, p_addr_table_ptr = 0x0, id_ptr = 0x09, id_shift = 0x2, type = "vulnerability"},
			{anim_ptr = 0x20, addr_table_ptr = 0x04, p_addr_table_ptr = 0x0, id_ptr = 0x0A, id_shift = 0x2, type = "vulnerability"},
			{anim_ptr = 0x20, addr_table_ptr = 0x08, p_addr_table_ptr = 0x4, id_ptr = 0x0C, id_shift = 0x2, type = "throwable"},
			{anim_ptr = 0x20, addr_table_ptr = 0x06, p_addr_table_ptr = 0x2, id_ptr = 0x0B, id_shift = 0x4, type = "attack"},
		},
		throw_box_list = {
			{dimensions = 0x88, clear = true, type = "axis throw"},
		},
		breakpoints = {
			{["sfa"] = 0x020F14, cmd = "maincpu.pw@(a6+88) = d0; maincpu.pw@(a6+8c) = d1"}, --ground throw
			{["sfa"] = 0x020FF2, cmd = 
			"maincpu.pw@(a6+88) = d0; maincpu.pw@(a6+8c) = d1; maincpu.pw@(a6+8a) = d2; maincpu.pw@(a6+8e) = d3"}, --airthrow
		},
		clones = {
			["sfar3"] = -0xB4, ["sfar2"] = -0x64, ["sfar1"] = -0x18, ["sfad"] = 0, ["sfau"] = -0x64, 
			["sfza"] = -0x64, ["sfzbr1"] = 0, ["sfzb"] = 0x5B4, ["sfzhr1"] = -0x64, ["sfzh"] = -0x18, 
			["sfzjr2"] = -0xB4, ["sfzjr1"] = -0x64, ["sfzj"] = 0, 
		},
		friends = {0x0D},
		active = function() return eval({
			(memory.readdword(0xFF8004) == 0x40000 and memory.readdword(0xFF8008) == 0x40000),
			(memory.readword(0xFF8008) == 0x2 and memory.readword(0xFF800A) > 0),
		}) end,
		invulnerable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x13B) > 0,
		}) end,
		unthrowable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x241) > 0,
			memory.readword(obj.base + 0x004) ~= 0x200,
			memory.readbyte(obj.base + 0x02F) > 0,
			bit.band(memory.readdword(memory.readdword(obj.base + 0x020) + 0x8), 0xFFFFFF00) == 0,
		}) end,
	},
	{
		games = {"sfa2","sfz2al"},
		number = {players = 3, projectiles = 26},
		address = {
			player           = 0xFF8400,
			projectile       = 0xFF9400,
			left_screen_edge = 0xFF8290,
		},
		offset = {
			projectile_space = 0x80,
			facing_dir       = 0x0B,
			hitbox_ptr       = {player = nil, projectile = 0x60},
		},
		box_list = {
			{anim_ptr = 0x1C, addr_table_ptr = 0x120, p_addr_table_ptr = 0x4, id_ptr = 0x0C, id_shift = 0x3, type = "push"},
			{anim_ptr = 0x1C, addr_table_ptr = 0x110, p_addr_table_ptr = 0x0, id_ptr = 0x08, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr = 0x1C, addr_table_ptr = 0x114, p_addr_table_ptr = 0x0, id_ptr = 0x09, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr = 0x1C, addr_table_ptr = 0x118, p_addr_table_ptr = 0x0, id_ptr = 0x0A, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr = 0x1C, addr_table_ptr = 0x120, p_addr_table_ptr = 0x4, id_ptr = 0x0C, id_shift = 0x3, type = "throwable"},
			{anim_ptr = 0x1C, addr_table_ptr = 0x11C, p_addr_table_ptr = 0x2, id_ptr = 0x0B, id_shift = 0x5, type = "attack"},
		},
		throw_box_list = {
			{dimensions = 0x88, clear = true, type = "axis throw"},
		},
		breakpoints = {
			{["sfa2"] = 0x025516, ["sfz2al"] = 0x025C8A, cmd = "maincpu.pw@(a6+88) = d0; maincpu.pw@(a6+8c) = d1"}, --ground throw
			{["sfa2"] = 0x02564A, ["sfz2al"] = 0x025DD6, cmd = "maincpu.pw@(a6+88) = d0; maincpu.pw@(a6+8c) = d1"}, --tripwire
			{["sfa2"] = 0x025786, ["sfz2al"] = 0x025F12, cmd = 
			"maincpu.pw@(a6+88) = d0; maincpu.pw@(a6+8c) = d1; maincpu.pw@(a6+8a) = d2; maincpu.pw@(a6+8e) = d3"}, --airthrow
		},
		clones = {
			["sfa2u"] = 0xBD2, ["sfa2ur1"] = 0xBC2, ["sfz2ad"] = 0xC0A, ["sfz2a"] = 0xC0A, 
			["sfz2br1"] = 0x48, ["sfz2b"] = 0x42, ["sfz2h"] = 0x48, ["sfz2jd"] = 0xC0A, ["sfz2j"] = 0xC0A, ["sfz2n"] = 0, 
			["sfz2al"] = 0, ["sfz2ald"] = 0, ["sfz2alb"] = 0, ["sfz2alh"] = 0, ["sfz2alj"] = -0x310, 
		},
		friends = {0x17},
		active = function() return eval({
			(memory.readdword(0xFF8004) == 0x40000 and
			(memory.readdword(0xFF8008) == 0x40000 or memory.readdword(0xFF8008) == 0xA0000)),
			memory.readword(0xFF8008) == 0x2 and memory.readword(0xFF800A) > 0,
		}) end,
		invulnerable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x25B) > 0,
			memory.readbyte(obj.base + 0x273) > 0,
			memory.readbyte(obj.base + 0x13B) > 0,
		}) end,
		unthrowable = function(obj, box)
			if eval({
					memory.readbyte(0xFF810E) > 0,
					memory.readbyte(obj.base + 0x273) > 0,
					bit.band(memory.readdword(memory.readdword(obj.base + 0x01C) + 0x8), 0xFFFFFF00) == 0,
				}) then
				return true
			elseif memory.readbyte(0xFF0000 + memory.readword(obj.base + 0x38) + 0x142) > 0 then --opponent in CC
				return eval({
					memory.readbyte(memory.readdword(obj.base + 0x01C) + 0xD) > 0,
				})
			else --not in CC
				return eval({
					memory.readbyte(obj.base + 0x241) > 0,
					memory.readword(obj.base + 0x004) ~= 0x200,
					memory.readbyte(obj.base + 0x031) > 0,
				})
			end
		end,
	},
	{
		games = {"sfa3"},
		number = {players = 4, projectiles = 24},
		address = {
			player           = 0xFF8400,
			projectile       = 0xFF9400,
			left_screen_edge = 0xFF8290,
		},
		offset = {
			projectile_space = 0x100,
			facing_dir       = 0x0B,
			hitbox_ptr       = nil,
		},
		box_list = {
			{anim_ptr =  nil, addr_table_ptr = 0x9C, id_ptr =  0xCB, id_shift = 0x3, type = "push"},
			{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr =  0xC8, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr =  nil, addr_table_ptr = 0x94, id_ptr =  0xC9, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr =  nil, addr_table_ptr = 0x98, id_ptr =  0xCA, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr =  nil, addr_table_ptr = 0x9C, id_ptr =  0xCB, id_shift = 0x3, type = "throwable"}, --identical to pushbox
			{anim_ptr = 0x1C, addr_table_ptr = 0xA0, id_ptr =   0x9, id_shift = 0x5, type = "attack"},
		},
		throw_box_list = {
			{anim_ptr =  nil, addr_table_ptr = 0xA0, id_ptr = 0x32F, id_shift = 0x5, type = "throw"},
			{anim_ptr =  nil, addr_table_ptr = 0xA0, id_ptr =  0x82, id_shift = 0x5, type = "tripwire"},
		},
		friends = {0x17, 0x22},
		active = function() return eval({
			(memory.readdword(0xFF8004) == 0x40000 and memory.readdword(0xFF8008) == 0x60000),
			(memory.readword(0xFF8008) == 0x2 and memory.readword(0xFF800A) > 0),
		}) end,
		invulnerable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x067) > 0,
			memory.readbyte(obj.base + 0x25D) > 0,
			memory.readbyte(obj.base + 0x0D6) > 0,
			memory.readbyte(obj.base + 0x2CE) > 0,
		}) end,
		unpushable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x67) > 0,
		}) end,
		unthrowable = function(obj, box)
			if eval({
					memory.readbyte(obj.base + 0x25D) > 0,
					memory.readbyte(obj.base + 0x23F) > 0,
					memory.readbyte(obj.base + 0x2CE) > 0,
					bit.band(memory.readdword(obj.base + 0x0C8), 0xFFFFFF00) == 0,
					memory.readbyte(obj.base + 0x067) > 0,
				}) then
				return true
			end
			local opp = { base = 0xFF0000 + memory.readword(obj.base + 0x38)}
			opp.air = memory.readbyte(opp.base + 0x31) > 0
			opp.VC  = memory.readbyte(opp.base + 0xB9) > 0
			local status = memory.readword(obj.base + 0x4)
			if opp.VC and memory.readbyte(obj.base + 0x24E) == 0 then --VC: 02E37C
				return
			elseif not opp.air then --ground: 02E3FE
				return eval({ --02E422
					status ~= 0x204 and status ~= 0x200 and memory.readbyte(obj.base + 0x24E) == 0 and 
					(status ~= 0x202 or memory.readbyte(obj.base + 0x54) ~= 0xC),
					memory.readbyte(obj.base + 0x031) > 0,
				})
			else --air: 02E636
				return eval({ --02E66E
					memory.readbyte(obj.base + 0x031) == 0,
					memory.readbyte(obj.base + 0x0D6) > 0,
					status ~= 0x204 and status ~= 0x200 and status ~= 0x202,
				})
			end
		end,
	},
	{
		games = {"dstlk"},
		number = {players = 2, projectiles = 4},
		address = {
			player           = 0xFF8388,
			projectile       = 0xFFAA2E,
			left_screen_edge = 0xFF9518,
		},
		offset = {
			projectile_space = 0xC0,
			facing_dir       = 0x09,
			hitbox_ptr       = {player = 0x5C, projectile = 0x5C},
			character        = 0x3A1, 
		},
		box = {hval = 0x0, vval = 0x4, hrad = 0x2, vrad = 0x6},
		box_list = {
			{anim_ptr = 0x1C, addr_table_ptr = 0x0A, id_ptr = 0x15, id_shift = 0x3, type = "push", no_projectile = true},
			{anim_ptr = 0x1C, addr_table_ptr = 0x00, id_ptr = 0x10, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr = 0x1C, addr_table_ptr = 0x02, id_ptr = 0x11, id_shift = 0x3, type = "vulnerability", no_projectile = true},
			{anim_ptr = 0x1C, addr_table_ptr = 0x04, id_ptr = 0x12, id_shift = 0x3, type = "vulnerability", no_projectile = true},
			{anim_ptr = 0x1C, addr_table_ptr = 0x06, id_ptr = 0x13, id_shift = 0x3, type = "negate"},
			{anim_ptr = 0x1C, addr_table_ptr = 0x08, id_ptr = 0x14, id_shift = 0x4, type = "attack"},
		},
		throw_box_list = {
			{x_base = 0x010, x_range = 0x1E8, y_range = 0x1E9, air_state = 0x4C, type = "throwable"},
			{ptr = 0x1EA, offset = 0x27, y_range = 0x1E9, throw_state = 0x114, anak_width = 0x20, type = "throw"}, --width @ 033C5C
			{ptr = 0x1EA, offset = 0x25, y_range = 0x1E9, clear = true, type = "throw"}, --airthrow
			{hard_x_range = 0x24, y_range = 0x1E9, pleasure_state = 0xB4, type = "throw"}, --hard range @ 03B938
		},
		breakpoints = {
			{["dstlk"] = 0x033BEC, cmd = "maincpu.pb@(a6+25) = d0"}, --air throws
			{["dstlk"] = 0x033CBE, cmd = "d3 = 0"}, --attempt ground throws out of range
		},
		clones = {
			["dstlka"] = 0, ["dstlkh"] = 0x2D6, ["dstlku1d"] = 0, ["dstlkur1"] = 0, ["dstlku"] = 0x2D6, 
			["vampjr1"] = 0x24A2, ["vampja"] = 0x24B6, ["vampj"] = 0x24B6, 
		},
		breakables = {start = 0xFFAD2E, space = 0x80, number = 8},
		active = function() return eval({
			(memory.readdword(0xFF8004) == 0x40000 and bit.band(memory.readdword(0xFF8008), 0x8FFFF) == 0x80000),
		}) end,
		invulnerable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x15D) > 0,
			(memory.readbyte(obj.base + 0x167) == 0 and 
			memory.readbytesigned(obj.base + 0x062) < 0 and 
			memory.readbyte(obj.base + 0x12A) == 0),
		}) end,
		unpushable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x1E6) > 0,
			memory.readbyte(obj.base + 0x04D) > 0,
		}) end,
		unthrowable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x062) > 0,
			memory.readword(obj.base + 0x15D) > 0,
		}) end,
	},
	{
		games = {"nwarr"},
		number = {players = 2, projectiles = 12},
		address = {
			player           = 0xFF8388,
			projectile       = 0xFFA86E,
			left_screen_edge = 0xFF8F18,
		},
		offset = {
			player_space     = 0x500,
			projectile_space = 0xC0,
			facing_dir       = 0x09,
			hitbox_ptr       = {player = 0x5C, projectile = 0x5C},
			character        = 0x4A1, 
		},
		box = {hval = 0x0, vval = 0x4, hrad = 0x2, vrad = 0x6},
		box_list = {
			{anim_ptr = 0x1C, addr_table_ptr = 0x0A, id_ptr = 0x15, id_shift = 0x3, type = "push", no_projectile = true},
			{anim_ptr = 0x1C, addr_table_ptr = 0x00, id_ptr = 0x10, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr = 0x1C, addr_table_ptr = 0x02, id_ptr = 0x11, id_shift = 0x3, type = "vulnerability", no_projectile = true},
			{anim_ptr = 0x1C, addr_table_ptr = 0x04, id_ptr = 0x12, id_shift = 0x3, type = "vulnerability", no_projectile = true},
			{anim_ptr = 0x1C, addr_table_ptr = 0x06, id_ptr = 0x13, id_shift = 0x3, type = "negate"},
			{anim_ptr = 0x1C, addr_table_ptr = 0x08, id_ptr = 0x14, id_shift = 0x4, type = "attack"},
		},
		throw_box_list = {
			{x_base = 0x010, x_range = 0x1E8, y_range = 0x1E9, air_state = 0x41, type = "throwable"},
			{x_base = 0x192, x_range = 0x196, y_range = 0x194, hard_y_base = 0, type = "axis throw"}, --"no_box" projectiles
			{ptr = 0x1EA, offset = 0x27, y_range = 0x1E9, throw_state = 0x114, type = "throw"},
			{ptr = 0x1EA, offset = 0x25, y_range = 0x1E9, clear = true, type = "throw"}, --airthrow
			{hard_x_range = 0x22, y_range = 0x1E9, pleasure_state = 0xAE, type = "throw"}, --hard range @ 036706
		},
		breakpoints = {
			{["nwarr"] = 0x029F5C, cmd = "maincpu.pb@(a6+25) = d0"}, --air throws
			{["nwarr"] = 0x02A002, cmd = "d3 = 0"}, --attempt ground throws out of range
		},
		clones = {
			["nwarra"] = -0x282, ["nwarrb"] = 0, ["nwarrh"] = 0, ["nwarrud"] = 0, ["nwarru"] = 0, 
			["vhuntjr2"] = -0x25E, ["vhuntjr1"] = 0x024, ["vhuntj"] = 0x024, 
		},
		special_projectiles = {start = 0xFF9A6E, space = 0x80, number = 28, whitelist = {
			0x56, --Demitri 263KK
			0x4C, --Gallon 41236KK
			0x5F, --Gallon 63214PP
			0x50, --Lei-Lei 623P
			0x54, --Lei-Lei 214P
			0x6E, --Lei-Lei LK,HK,MP,MP,8
			0x0C, --Morrigan LP,LP,6,LK,HP
			0x40, --Morrigan LP,LP,6,MP,HP
			0x44, --Felicia 41236KK
			0x52, --Aulbath 41236PP
			0x05, --Huitzil GC
			0x22, --Huitzil 63214KK
			0x5A, --Huitzil 623P
			0x70, --Pyron 41236PP/KK
		}, 
		no_box = { --special throws
			0x3F, --Rapter 623PP
			0x68, --Anakaris 236P
			0x3E, --Aulbath 623PP
		}},
		friends = {0x40, 0x4C, 0x50},
		breakables = {start = 0xFFB16E, space = 0x80, number = 8},
		active = function() return eval({
			(memory.readdword(0xFF8004) == 0x40000 and bit.band(memory.readdword(0xFF8008), 0x8FFFF) == 0x80000),
			(memory.readword(0xFF8000) >= 0x0E and memory.readdword(0xFF8004) == 0),
		}) end,
		invulnerable = function(obj, box) return eval({
			(memory.readbyte(obj.base + 0x49F) == 0 and memory.readbyte(obj.base + 0x2AF) > 0),
			memory.readbyte(obj.base + 0x15D) > 0,
			bit.band(memory.readbyte(obj.base + 0x167), 0x0E) > 0,
			(memory.readbyte(obj.base + 0x167) == 0 and 
			memory.readbytesigned(obj.base + 0x062) < 0 and 
			memory.readbyte(obj.base + 0x12A) == 0),
		}) end,
		unpushable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x1E6) > 0,
			memory.readbyte(obj.base + 0x04C) > 0,
		}) end,
		unthrowable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x062) > 0,
			memory.readbyte(obj.base + 0x15D) > 0,
		}) end,
	},
	{
		games = {"vsav","vhunt2","vsav2"},
		number = {players = 2, projectiles = 32},
		address = {
			player           = 0xFF8400,
			projectile       = 0xFF9400,
			left_screen_edge = 0xFF8290,
		},
		offset = {
			projectile_space = 0x100,
			facing_dir       = 0x0B,
			hitbox_ptr       = nil,
		},
		box_list = {
			{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr = 0x97, id_shift = 0x3, type = "push"},
			{anim_ptr =  nil, addr_table_ptr = 0x80, id_ptr = 0x94, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr =  nil, addr_table_ptr = 0x84, id_ptr = 0x95, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x96, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr =  nil, addr_table_ptr = 0x90, id_ptr = 0x97, id_shift = 0x3, type = "throwable"}, --identical to pushbox
			{anim_ptr = 0x1C, addr_table_ptr = 0x8C, id_ptr = 0x0A, id_shift = 0x5, type = "attack"},
		},
		throw_box_list = {
			{anim_ptr =  nil, addr_table_ptr = 0x8C, id_ptr = 0x98, id_shift = 0x5, pos_offset = 0x9A, type = "throw"},
		},
		breakpoints = {
			{["vsav"] = 0x029450, ["vhunt2"] = 0x028778, ["vsav2"] = 0x02874A, 
			cmd = "maincpu.pb@(a6+98) = d0"},
			{["vsav"] = 0x029638, ["vhunt2"] = 0x02896C, ["vsav2"] = 0x02893E, 
			cmd = "maincpu.pb@(a6+98) = d0; maincpu.pw@(a6+9a) = maincpu.pw@(a4+10); maincpu.pw@(a6+9c) = maincpu.pw@(a4+14)"},
		},
		clones = {
			["vsava"] = 0, ["vsavd"] = 0, ["vsavh"] = 0, ["vsavj"] = 0, ["vsavu"] = 0, 
			["vhunt2"] = 0, ["vhunt2r1"] = -0xB2, ["vhunt2d"] = 0, ["vsav2"] = 0, ["vsav2d"] = 0, 
		},
		friends = {0x08, 0x10, 0x11, 0x37},
		active = function() return eval({
			(memory.readdword(0xFF8004) == 0x40000 and memory.readdword(0xFF8008) == 0x40000),
			(memory.readword(0xFF8008) == 0x2 and memory.readword(0xFF800A) > 0),
		}) end,
		invulnerable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x134) > 0,
			memory.readbyte(obj.base + 0x147) > 0,
			memory.readbyte(obj.base + 0x11E) > 0,
			memory.readbyte(obj.base + 0x145) > 0 and memory.readbyte(obj.base + 0x1A4) == 0,
		}) end,
		unpushable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x134) > 0,
		}) end,
		unthrowable = function(obj, box) return eval({
			not (memory.readword(obj.base + 0x004) == 0x0200 or memory.readword(obj.base + 0x004) == 0x0204),
			memory.readbyte(obj.base + 0x143) > 0,
			memory.readbyte(obj.base + 0x147) > 0,
			memory.readbyte(obj.base + 0x11E) > 0,
			bit.band(memory.readdword(obj.base + 0x094), 0xFFFFFF00) == 0,
		}) end,
	},
	{
		games = {"ringdest"},
		number = {players = 2, projectiles = 28},
		ground_level = 23,
		address = {
			player           = 0xFF8000,
			projectile       = 0xFF9000,
			left_screen_edge = 0xFF72D2,
			top_screen_edge  = 0xFF72D4,
		},
		offset = {
			projectile_space = 0x100,
			facing_dir       = 0x38,
			id_ptr           = 0x4A,
		},
		box = {
			radscale = 2,
			hval = 0x0, vval = 0x4, hrad = 0x2, vrad = 0x6,
		},
		box_list = {
			{addr_table_ptr = 0x2D8, type = "push"},
			{addr_table_offset = 0xC956, id_shift = 0x2, type = "throwable"},
			{addr_table_offset = 0xC92E, id_shift = 0x2, type = "vulnerability"},
			{addr_table_offset = 0xC936, id_shift = 0x2, type = "vulnerability"},
			{addr_table_offset = 0xC93E, id_shift = 0x2, type = "vulnerability"},
			{addr_table_offset = 0xC946, id_shift = 0x2, type = "attack"},
		},
		throw_box_list = {
			{addr_table_offset = 0xC94E, id_shift = 0x2, type = "throw"},
		},
		active = function() return eval({
			memory.readwordsigned(0xFF72D2) > 0,
		}) end,
		projectile_active = function(obj, box) return eval({
			memory.readword(obj.base) > 0x0100 and memory.readbyte(obj.base + 0x02) == 0x01,
		}) end,
		unpushable = function(obj, box) return eval({
			obj.projectile ~= nil,
			memory.readword(obj.base + 0x70) > 0,
			memory.readword(obj.base + 0x98) > 0,
		}) end,
	},
	{
		games = {"cybots"},
		number = {players = 2, projectiles = 16},
		address = {
			player           = 0xFF81A0,
			projectile       = 0xFF92A0,
		},
		offset = {
			projectile_space = 0xC0,
			facing_dir       = 0x09,
			x_position       = 0x1A,
			hitbox_ptr       = {player = 0x32, projectile = 0x32},
		},
		box_list = {
			{anim_ptr = nil, addr_table_ptr = 0x08, id_ptr = 0x66, id_shift = 0x3, type = "push"},
			{anim_ptr = nil, addr_table_ptr = 0x02, id_ptr = 0x63, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr = nil, addr_table_ptr = 0x04, id_ptr = 0x64, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr = nil, addr_table_ptr = 0x06, id_ptr = 0x65, id_shift = 0x3, type = "vulnerability"},
			{dimensions = 0x19E, type = "throwable"},
			{anim_ptr = nil, addr_table_ptr = 0x00, id_ptr = 0x62, id_shift = 0x4, type = "attack"},
		},
		throw_box_list = {
			{dimensions = 0x3F8, clear = true, type = "throw"},
		},
		breakpoints = {
			{["cybots"] = 0x002D66, ["cybots"] = 0x002DF0, cmd = "maincpu.pd@(a6+3f8) = maincpu.pd@(a6+160); " .. 
			"maincpu.pw@(a6+3fc) = maincpu.pw@(a6+164); maincpu.pw@(a6+3fe) = maincpu.pw@(a6+164)"},
		},
		clones = {
			["cybotsj"] = 0, ["cybotsud"] = 0, ["cybotsu"] = 0, 
		},
		get_cam_ptr = function()
			local slot = {
				0x6CDA, 0x6C1A, 0x6CDA, 0x6C1A, 0x6CDA, 0x6CDA, 0x6D9A, 0x6CDA, 
				0x6CDA, 0x6D9A, 0x6D9A, 0x6D9A, 0x6C1A, 0x6D9A, 0x6CDA, 0x6CDA,
			}
			return 0xFF8000 + (slot[memory.readword(0xFFEA1A) + 1] or slot[1]) + 0x1A
		end,
		active = function() return eval({
			bit.band(memory.readdword(0xFF8008), 0x10FFFF) == 0x100000,
		}) end,
		projectile_active = function(obj) return eval({
			memory.readword(obj.base) > 0x0100 and bit.band(memory.readword(obj.base + 0x2), 0x8) == 0,
		}) end,
		invulnerable = function(obj, box) return eval({
			bit.band(memory.readword(obj.base + 0x126), 0x20) > 0,
		}) end,
		unpushable = function(obj, box) return eval({
			bit.band(memory.readword(obj.base + 0x126), 0x20) > 0,
		}) end,
		unthrowable = function(obj, box)
		local status = memory.readword(obj.base + 0x126)
		return eval({
			memory.readword(obj.base + 0x152) > 0,
			bit.band(status, 0x40) == 0 and bit.band(status, 0x30) > 0,
			memory.readbyte(obj.base + 0x1A6) > 0,
		}) end,
	},
	{
		games = {"sgemf"},
		number = {players = 2, projectiles = 14},
		address = {
			player           = 0xFF8400,
			projectile       = 0xFF8C00,
			left_screen_edge = 0xFF8290,
		},
		offset = {
			projectile_space = 0x100,
			facing_dir       = 0x0B,
			hitbox_ptr       = nil,
		},
		box = {radscale = 2},
		box_list = {
			{anim_ptr =  nil, addr_table_ptr = 0x8C, id_ptr = 0x93, id_shift = 0x3, type = "push"},
			{anim_ptr =  nil, addr_table_ptr = 0x80, id_ptr = 0x90, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr =  nil, addr_table_ptr = 0x84, id_ptr = 0x91, id_shift = 0x3, type = "vulnerability"},
			{anim_ptr = 0x1C, addr_table_ptr = 0x8C, id_ptr = 0x0B, id_shift = 0x3, type = "throwable"}, --same as pushbox?
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x92, id_shift = 0x5, type = "attack"},
		},
		throw_box_list = {
			{anim_ptr =  nil, addr_table_ptr = 0x88, id_ptr = 0x98, id_shift = 0x5, type = "throw"},
		},
		breakpoints = {
			{["sgemf"] = 0x012800, cmd = "maincpu.pb@(a6+98) = d0"},
		},
		clones = {
			["pfghtj"] = 0, ["sgemfd"] = 0, ["sgemfa"] = 0, ["sgemfh"] = 0, 
		},
		active = function() return eval({
			memory.readdword(0xFF8004) == 0x40000 and memory.readdword(0xFF8008) == 0x40000,
			memory.readword(0xFF8008) == 0x2 and memory.readword(0xFF800A) > 0,
		}) end,
		no_hit = function(obj, box) return eval({
			memory.readbyte(obj.base + 0xB1) > 0,
		}) end,
		invulnerable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x147) > 0,
			memory.readbyte(obj.base + 0x132) > 0,
			memory.readbyte(obj.base + 0x11B) > 0,
		}) end,
		unpushable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x1AA) > 0,
			memory.readbyte(obj.base + 0x093) == 0,
		}) end,
		unthrowable = function(obj, box) return eval({
			memory.readbyte(obj.base + 0x143) > 0,
			memory.readbyte(obj.base + 0x188) > 0,
			memory.readbyte(obj.base + 0x119) == 0 and memory.readword(obj.base + 0x04) == 0x0202,
			memory.readword(box.id_base + 0x08) == 0,
			memory.readbyte(obj.base + 0x105) > 0 and memory.readbyte(obj.base + 0x1BE) == 0 and memory.readbyte(box.id_base + 0x17) == 0,
		}) end,
	},
}

--------------------------------------------------------------------------------
-- post-process modules

local function get_throw_type(entry)
	if entry.throw_type then
		return entry.throw_type
	end
	for _, trait in ipairs({
		{entry.dimensions, "dimensions"},
		{entry.x_range, "range given"},
		{entry.ptr, "range pointer"},
		{entry.pleasure_state, "midnight pleasure"},
	}) do
		if trait[1] then
			return trait[2]
		end
	end
	return nil
end

for game in ipairs(profile) do
	local g = profile[game]
	g.box_number = #g.box_list + #g.throw_box_list
	g.box_type = g.offset.id_ptr and "id ptr" or "hitbox ptr"
	for _, entry in ipairs(g.box_list) do
		entry.throw_type = get_throw_type(entry)
	end
	for _, entry in ipairs(g.throw_box_list) do
		entry.throw_type = get_throw_type(entry)
	end

	g.ground_level = g.ground_level or -15
	g.address.top_screen_edge = g.address.top_screen_edge or (g.address.left_screen_edge and g.address.left_screen_edge + 0x4)
	g.offset.player_space = g.offset.player_space or 0x400
	g.offset.x_position   = g.offset.x_position   or 0x10
	g.offset.y_position   = g.offset.y_position   or g.offset.x_position + 0x4
	g.offset.hitbox_ptr   = g.offset.hitbox_ptr   or {}
	g.friends = g.friends or {}
	g.box     = g.box     or {}
	g.box.radius_read = g.box.radius_read or memory.readword
	g.box.offset_read = g.box.radius_read == memory.readword and memory.readwordsigned or memory.readbytesigned
	g.box.hval        = g.box.hval or 0x0
	g.box.vval        = g.box.vval or 0x2
	g.box.hrad        = g.box.hrad or 0x4
	g.box.vrad        = g.box.vrad or 0x6
	g.box.radscale    = g.box.radscale or 1
	g.no_hit       = g.no_hit       or function() end
	g.invulnerable = g.invulnerable or function() end
	g.unpushable   = g.unpushable   or function() end
	g.unthrowable  = g.unthrowable  or function() end
	g.projectile_active = g.projectile_active or function(obj)
		if memory.readword(obj.base) > 0x0100 and memory.readbyte(obj.base + 0x04) == 0x02 then
			return true
		end
	end
	g.special_projectiles = g.special_projectiles or {number = 0}
	g.breakables = g.breakables or {number = 0}
end

for _,box in pairs(boxes) do
	box.fill    = box.color * 0x100 + box.fill
	box.outline = box.color * 0x100 + box.outline
end

local game
local player, projectiles, frame_buffer = {}, {}, {}
local DRAW_DELAY = 1
if fba then
	DRAW_DELAY = DRAW_DELAY + 1
end


--------------------------------------------------------------------------------
-- prepare the hitboxes

local function update_globals()
	globals.left_screen_edge_ptr = game.address.left_screen_edge or game.get_cam_ptr()
	globals.top_screen_edge_ptr  = game.address.top_screen_edge or globals.left_screen_edge_ptr + 0x4
	globals.left_screen_edge = memory.readwordsigned(globals.left_screen_edge_ptr)
	globals.top_screen_edge  = memory.readwordsigned(globals.top_screen_edge_ptr)
	globals.match_active     = game.active()
end


local function get_x(x)
	return x - globals.left_screen_edge
end


local function get_y(y)
	return emu.screenheight() - (y + game.ground_level) + globals.top_screen_edge
end


local process_box_type = {
	["vulnerability"] = function(obj, box, box_entry)
		if game.invulnerable(obj, box) or obj.friends then
			return false
		end
	end,

	["attack"] = function(obj, box)
		if game.no_hit(obj, box) then
			return false
		end
	end,

	["push"] = function(obj, box)
		if game.unpushable(obj, box) or obj.friends then
			return false
		end
	end,

	["tripwire"] = function(obj, box, box_entry)
		box.id = bit.rshift(box.id, 1) + 0x3E
		obj.throw_pos_x = memory.readwordsigned(obj.base + 0x1E4)
		if obj.throw_pos_x == 0 or memory.readbyte(obj.base + 0x102) ~= 0x0E then
			return false
		end
		obj.throw_pos_x = obj.throw_pos_x + obj.pos_x
		if not (memory.readbyte(obj.base + 0x07) == 0x04 and memory.readbyte(obj.base + 0xAA) == 0x0C) then
			memory.writeword(obj.base + 0x1E4, 0) --bad
		end
	end,

	["negate"] = function(obj, box)
	end,

	["throw"] = function(obj, box, box_entry)
		if not box_entry.id_ptr then
			return
		end
		memory.writebyte(obj.base + box_entry.id_ptr, 0) --bad
		if box_entry.pos_offset and memory.readdword(obj.base + box_entry.pos_offset) ~= 0 then --ranged throws in vsav
			obj.throw_pos_x = get_x(memory.readword(obj.base + box_entry.pos_offset))
			obj.throw_pos_y = get_y(memory.readword(obj.base + box_entry.pos_offset + 2))
			memory.writedword(obj.base + box_entry.pos_offset, 0)
		end
	end,

	["axis throw"] = function(obj, box, box_entry)
	end,

	["throwable"] = function(obj, box)
		if game.unthrowable(obj, box) or obj.projectile then
			return false
		end
	end,
}


local function set_projectile_type(obj, box)
	if obj.projectile and not obj.friends then
		box.type = (box.type == "vulnerability" and "proj. vulnerability") or box.type
		box.type = (box.type == "attack" and "proj. attack") or box.type
	end
end


local define_box = {
	["hitbox ptr"] = function(obj, box_entry)
		if obj.projectile and box_entry.no_projectile then
			return nil
		end

		local box = {type = box_entry.type}

		box.id_base = (box_entry.anim_ptr and memory.readdword(obj.base + box_entry.anim_ptr)) or obj.base
		box.id = memory.readbyte(box.id_base + box_entry.id_ptr)

		if process_box_type[box.type](obj, box, box_entry) == false or box.id == 0 then
			return nil
		end

		local addr_table
		if not obj.hitbox_ptr then
			addr_table = memory.readdword(obj.base + box_entry.addr_table_ptr)
		else
			local table_offset = obj.projectile and box_entry.p_addr_table_ptr or box_entry.addr_table_ptr
			addr_table = obj.hitbox_ptr + memory.readwordsigned(obj.hitbox_ptr + table_offset)
		end
		box.address = addr_table + bit.lshift(box.id, box_entry.id_shift)

		box.hrad = game.box.radius_read(box.address + game.box.hrad)/game.box.radscale
		box.vrad = game.box.radius_read(box.address + game.box.vrad)/game.box.radscale
		box.hval = game.box.offset_read(box.address + game.box.hval)
		box.vval = game.box.offset_read(box.address + game.box.vval)
		if box.type == "push" then
			obj.vval, obj.vrad = box.vval, box.vrad
		end

		box.hval   = (obj.throw_pos_x or obj.pos_x) + box.hval * (obj.facing_dir == 1 and -1 or 1)
		box.vval   = (obj.throw_pos_y or obj.pos_y) - box.vval
		box.left   = box.hval - box.hrad
		box.right  = box.hval + box.hrad
		box.top    = box.vval - box.vrad
		box.bottom = box.vval + box.vrad

		set_projectile_type(obj, box)

		return box
	end,

	["id ptr"] = function(obj, box_entry) --for ringdest only
		local box = {type = box_entry.type}

		if process_box_type[box.type](obj, box, box_entry) == false then
			return nil
		end

		if box_entry.addr_table_offset then
			box.address = box_entry.addr_table_offset + bit.lshift(obj.id_offset, box_entry.id_shift)
		else
			box.address = memory.readdword(obj.base + box_entry.addr_table_ptr)
		end

		box.hrad = game.box.radius_read(box.address + game.box.hrad)/game.box.radscale
		box.vrad = game.box.radius_read(box.address + game.box.vrad)/game.box.radscale
		if box.hrad == 0 or box.vrad == 0 then
			return nil
		end
		box.hval = game.box.offset_read(box.address + game.box.hval)
		box.vval = game.box.offset_read(box.address + game.box.vval)

		box.hval   = obj.pos_x + (box.hrad + box.hval) * (obj.facing_dir > 0 and -1 or 1)
		box.vval   = obj.pos_y - (box.vrad + box.vval)
		box.left   = box.hval - box.hrad
		box.right  = box.hval + box.hrad
		box.top    = box.vval - box.vrad
		box.bottom = box.vval + box.vrad

		set_projectile_type(obj, box)

		return box
	end,

	["dimensions"] = function(obj, box_entry) --sfa/sfa2 & cybots
		local box = {type = box_entry.type}

		if process_box_type[box.type](obj, box, box_entry) == false or 
			memory.readdword(obj.base + box_entry.dimensions + 0x4) == 0 then
			return nil
		end
		box.hval = memory.readwordsigned(obj.base + box_entry.dimensions + 0x0)
		box.hrad = memory.readword(obj.base + box_entry.dimensions + 0x4)
		if memory.readword(obj.base + box_entry.dimensions + 0x6) == 0 then --sfa/sfa2 ground/tripwire
			box.vval = obj.vval or 0x28
			box.vrad = obj.vrad or 0x28
			box.type = "throw"
		else
			box.vval = memory.readwordsigned(obj.base + box_entry.dimensions + 0x2)
			box.vrad = memory.readword(obj.base + box_entry.dimensions + 0x6)
		end
		if box_entry.clear then
			memory.writedword(obj.base + box_entry.dimensions + 0x0, 0)
			memory.writedword(obj.base + box_entry.dimensions + 0x4, 0)
		end
		box.hval   = obj.pos_x + box.hval * (obj.facing_dir == 1 and -1 or 1)
		box.vval   = obj.pos_y - box.vval
		box.left   = box.hval - box.hrad
		box.right  = box.hval + box.hrad
		box.top    = box.vval - box.vrad
		box.bottom = box.vval + box.vrad

		return box
	end,

	["range given"] = function(obj, box_entry) --dstlk/nwarr throwable; nwarr ranged
		local box = {type = box_entry.type}

		local base  = memory.readword(obj.base + box_entry.x_base)
		local range = memory.readbyte(obj.base + box_entry.x_range) * (obj.facing_dir == 1 and -1 or 1)
		if process_box_type[box.type](obj, box, box_entry) == false or base == 0 or range == 0 then
			return nil
		end
		box.right = get_x(base) - range
		box.left  = get_x(base) + range
		if box_entry.hard_y_base and box_entry.y_range then --nwarr ranged
			box.bottom = get_y(box_entry.hard_y_base) --ground level is 0x28
			box.top    = get_y(memory.readword(obj.base + box_entry.y_range))
		else
			box.top    = obj.pos_y - memory.readbyte(obj.base + box_entry.y_range)
			if memory.readbyte(obj.base + box_entry.air_state) > 0 then
				box.bottom = box.top + 0xC --air throwable; verify range @ 033BE0 [dstlk] & 029F50 [nwarr]
			else
				box.bottom = obj.pos_y --ground throwable
			end
		end
		box.hval = (box.left + box.right)/2
		box.vval = (box.bottom + box.top)/2

		return box
	end,

	["range pointer"] = function(obj, box_entry) --dstlk/nwarr ground & air
		local box = {type = box_entry.type}

		local range_offset = memory.readbytesigned(obj.base + box_entry.offset)
		local range = memory.readword(memory.readdword(obj.base + box_entry.ptr) + range_offset)
		if bit.band(range_offset, 0xFE) == 0 or range == 0 or 
			(box_entry.throw_state and memory.readbyte(obj.base + box_entry.throw_state) == 0) then
			return nil
		end
		if box_entry.clear then --air: check if throw command was just input; verify @ 0451D6 [dstlk] & 0355CE [nwarr]
			memory.writebyte(obj.base + box_entry.offset, 0)
			local curr = memory.readword(obj.base + game.offset.player_space - 0x7A)
			local prev = memory.readword(obj.base + game.offset.player_space - 0x78)
			if bit.band(curr, 0x07) == 0 or bit.band(bit.band(bit.bnot(prev), curr), 0x60) == 0 then
				return nil
			end
		end

		box.left = obj.pos_x + range * (obj.facing_dir == 1 and -1 or 1)
		if box_entry.anak_width and memory.readbyte(obj.base + game.offset.character) == 0x06 then --dstlk Anakaris
			box.right = box.left - box_entry.anak_width * (obj.facing_dir == 1 and -1 or 1)
		else
			box.right = obj.pos_x
		end
		box.top = obj.pos_y - memory.readbyte(obj.base + box_entry.y_range)
		if box_entry.clear then --air: same vertical range as throwable
			box.bottom = box.top + 0xC
		else --ground
			box.bottom = obj.pos_y
		end
		box.hval = (box.left + box.right)/2
		box.vval = (box.bottom + box.top)/2
		
		return box
	end,

	["midnight pleasure"] = function(obj, box_entry) --dstlk/nwarr
		local box = {type = box_entry.type}

		if memory.readbyte(obj.base + game.offset.character) ~= 0x01 or --Demitri
			memory.readbyte(obj.base + 0x13C) ~= box_entry.pleasure_state or --MP in progress
			memory.readword(obj.base + 0x020) == 0x00 then --time remaining
			return nil
		end

		box.right  = obj.pos_x
		box.left   = obj.pos_x + box_entry.hard_x_range * (obj.facing_dir == 1 and -1 or 1)
		box.bottom = obj.pos_y
		box.top    = obj.pos_y - memory.readbyte(obj.base + box_entry.y_range)
		box.hval   = (box.left + box.right)/2
		box.vval   = (box.bottom + box.top)/2
		
		return box
	end,
}


local prepare_boxes = {
	["hitbox ptr"] = function(obj)
		obj.hitbox_ptr = obj.projectile and game.offset.hitbox_ptr.projectile or game.offset.hitbox_ptr.player
		obj.hitbox_ptr = obj.hitbox_ptr and memory.readdword(obj.base + obj.hitbox_ptr) or nil
		for _, box_entry in ipairs(game.box_list) do
			table.insert(obj, define_box[box_entry.throw_type or game.box_type](obj, box_entry))
		end
	end,

	["id ptr"] = function(obj) --for ringdest only
		obj.id_offset = memory.readword(obj.base + game.offset.id_ptr)
		for entry in ipairs(game.box_list) do
			table.insert(obj, define_box[game.box_type](obj, game.box_list[entry]))
		end
	end,
}


local function update_game_object(obj)
	obj.facing_dir = memory.readbyte(obj.base + game.offset.facing_dir)
	obj.pos_x      = get_x(memory.readwordsigned(obj.base + game.offset.x_position))
	obj.pos_y      = get_y(memory.readwordsigned(obj.base + game.offset.y_position))
	prepare_boxes[game.box_type](obj)
	return obj
end


local function friends_status(id)
	for _, friend in ipairs(game.friends) do
		if id == friend then
			return true
		end
	end
end


local function read_projectiles()
	local current_projectiles = {}

	for i = 1, game.number.projectiles do
		local obj = {base = game.address.projectile + (i-1) * game.offset.projectile_space}
		if game.projectile_active(obj) then
			obj.projectile = true
			obj.friends = friends_status(memory.readbyte(obj.base + 0x02))
			table.insert(current_projectiles, update_game_object(obj))
		end
	end

	for i = 1, game.special_projectiles.number do --for nwarr only
		local obj = {base = game.special_projectiles.start + (i-1) * game.special_projectiles.space}
		local id = memory.readbyte(obj.base + 0x02)
		for _, valid in ipairs(game.special_projectiles.no_box) do
			if id == valid then
				obj.pos_x = get_x(memory.readwordsigned(obj.base + game.offset.x_position))
				obj.pos_y = get_y(memory.readwordsigned(obj.base + game.offset.y_position))
				table.insert(current_projectiles, obj)
				break
			end
		end
		for _, valid in ipairs(game.special_projectiles.whitelist) do
			if id == valid then
				obj.projectile, obj.hit_only, obj.friends = true, true, friends_status(id)
				table.insert(current_projectiles, update_game_object(obj))
				break
			end
		end
	end
--[[
	for i = 1, game.breakables.number do --for dstlk, nwarr
		local obj = {base = game.breakables.start + (i-1) * game.breakables.space}
		local status = memory.readbyte(obj.base + 0x04)
		if status == 0x02 then
			obj.projectile = true
			obj.x_adjust = 0x1C*((globals.left_screen_edge-0x100)/0xC0-1)
			table.insert(current_projectiles, update_game_object(obj))
		end
	end
]]
	return current_projectiles
end


local function update_hitboxes()
	if not game then
		return
	end
	update_globals()

	for f = 1, DRAW_DELAY do
		frame_buffer[f].match_active = frame_buffer[f+1].match_active
		for p = 1, game.number.players do
			frame_buffer[f][player][p] = copytable(frame_buffer[f+1][player][p])
		end
		frame_buffer[f][projectiles] = copytable(frame_buffer[f+1][projectiles])
	end

	frame_buffer[DRAW_DELAY+1].match_active = globals.match_active
	for p = 1, game.number.players do
		player[p] = {base = game.address.player + (p-1) * game.offset.player_space}
		if memory.readword(player[p].base) > 0x0100 then
			update_game_object(player[p])
		else
			player[p] = {}
		end

		frame_buffer[DRAW_DELAY+1][player][p] = player[p]

		local prev_frame = frame_buffer[DRAW_DELAY][player][p]
		if prev_frame and prev_frame.pos_x then
			for _, box_entry in ipairs(game.throw_box_list) do
				table.insert(prev_frame, define_box[box_entry.throw_type or game.box_type](prev_frame, box_entry))
			end
		end

	end
	frame_buffer[DRAW_DELAY+1][projectiles] = read_projectiles()

end


emu.registerafter( function()
	update_hitboxes()
end)


--------------------------------------------------------------------------------
-- draw the hitboxes

local function draw_hitbox(obj, entry)
	local hb = obj[entry]
	if eval ({
		not globals.draw_pushboxes and hb.type == "push",
		not globals.draw_throwable_boxes and hb.type == "throwable",
	}) then return
	end

	if globals.draw_mini_axis then
		gui.drawline(hb.hval, hb.vval-globals.mini_axis_size, hb.hval, hb.vval+globals.mini_axis_size, boxes[hb.type].outline)
		gui.drawline(hb.hval-globals.mini_axis_size, hb.vval, hb.hval+globals.mini_axis_size, hb.vval, boxes[hb.type].outline)
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, boxes[hb.type].fill, boxes[hb.type].outline)
end


local function draw_axis(obj)
	if not obj or not obj.pos_x then
		return
	end
	
	gui.drawline(obj.pos_x, obj.pos_y-globals.axis_size, obj.pos_x, obj.pos_y+globals.axis_size, globals.axis_color)
	gui.drawline(obj.pos_x-globals.axis_size, obj.pos_y, obj.pos_x+globals.axis_size, obj.pos_y, globals.axis_color)
	--gui.text(obj.pos_x, obj.pos_y, string.format("%06X, %02X", obj.base, memory.readbyte(obj.base + 2))) --debug
end


local function render_hitboxes()
	gui.clearuncommitted()
	if not game or not frame_buffer[1].match_active then
		return
	end

	if globals.blank_screen then
		gui.box(0, 0, emu.screenwidth(), emu.screenheight(), globals.blank_color)
	end

	for entry = 1, game.box_number do
		for p = 1, #frame_buffer[1][projectiles] do
			local obj = frame_buffer[1][projectiles][p]
			if obj[entry] then
				draw_hitbox(obj, entry)
			end
		end

		for p = 1, game.number.players do
			local obj = frame_buffer[1][player][p]
			if obj and obj[entry] then
				draw_hitbox(obj, entry)
			end
		end
	end

	if globals.draw_axis then
		for p = 1, #frame_buffer[1][projectiles] do
			draw_axis(frame_buffer[1][projectiles][p])
		end

		for p = 1, game.number.players do
			draw_axis(frame_buffer[1][player][p])
		end
	end
end


gui.register( function()
	render_hitboxes()
end)


--------------------------------------------------------------------------------
-- hotkey functions

input.registerhotkey(1, function()
	globals.blank_screen = not globals.blank_screen
	render_hitboxes()
	print((globals.blank_screen and "activated" or "deactivated") .. " blank screen mode")
end)


input.registerhotkey(2, function()
	globals.draw_axis = not globals.draw_axis
	render_hitboxes()
	print((globals.draw_axis and "showing" or "hiding") .. " object axis")
end)


input.registerhotkey(3, function()
	globals.draw_mini_axis = not globals.draw_mini_axis
	render_hitboxes()
	print((globals.draw_mini_axis and "showing" or "hiding") .. " hitbox axis")
end)


input.registerhotkey(4, function()
	globals.draw_pushboxes = not globals.draw_pushboxes
	render_hitboxes()
	print((globals.draw_pushboxes and "showing" or "hiding") .. " pushboxes")
end)


input.registerhotkey(5, function()
	globals.draw_throwable_boxes = not globals.draw_throwable_boxes
	render_hitboxes()
	print((globals.draw_throwable_boxes and "showing" or "hiding") .. " throwable boxes")
end)


--------------------------------------------------------------------------------
-- initialize on game startup

local function initialize()
	globals.left_screen_edge = 0
	globals.top_screen_edge  = 0
	for p = 1, game.number.players do
		player[p] = {}
	end
	for f = 1, DRAW_DELAY + 1 do
		frame_buffer[f] = {}
		frame_buffer[f][player] = {}
		frame_buffer[f][projectiles] = {}
	end
end


local function whatgame()
	print()
	game = nil
	for _, module in ipairs(profile) do
		for _, shortname in ipairs(module.games) do
			if emu.romname() == shortname or emu.parentname() == shortname then
				print("drawing " .. emu.romname() .. " hitboxes")
				game = module
				initialize()
				if game.breakpoints then
					if mame then
						if (emu.parentname() == "0" or game.clones[emu.romname()]) then
							print("Copy this line into the MAME-rr debugger to show throwboxes:") print()
							local bpstring = ""
							for _, bp in ipairs(game.breakpoints) do
								local bpaddr = bp[emu.romname()] or bp[shortname] + game.clones[emu.romname()]
								bpstring = bpstring .. string.format("bp %06X, 1, {%s; g}; ", bpaddr, bp.cmd)
							end
							print(bpstring:sub(1, -3)) print()
						else
							print("MAME-rr can show throwboxes, but breakpoints are unknown for clone '" .. emu.romname() .. "'.") print()
						end
					else
						print() print("(MAME-rr can show throwboxes for this game.)") print()
					end
				end
				return
			end
		end
	end
	print("not prepared for " .. emu.romname() .. " hitboxes")
end


savestate.registerload(function()
	initialize()
end)


emu.registerstart(function()
	whatgame()
end)