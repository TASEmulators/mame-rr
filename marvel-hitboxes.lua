print("CPS-2 Marvel series hitbox viewer")
print("August 30, 2011")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis")
print("Lua hotkey 4: toggle pushboxes")
print("Lua hotkey 5: toggle throwable boxes")
if not mame then
	print() print("(MAME-rr can show more accurate throwboxes and pushboxes with this script.)")
end

local boxes = {
	      ["vulnerability"] = {color = 0x7777FF, fill = 0x20, outline = 0xFF},
	             ["attack"] = {color = 0xFF0000, fill = 0x40, outline = 0xFF},
	["proj. vulnerability"] = {color = 0x00FFFF, fill = 0x40, outline = 0xFF},
	       ["proj. attack"] = {color = 0xFF66FF, fill = 0x40, outline = 0xFF},
	               ["push"] = {color = 0x00FF00, fill = 0x20, outline = 0xFF},
	    ["potential throw"] = {color = 0xFFFF00, fill = 0x00, outline = 0x00}, --not visible by default
	       ["active throw"] = {color = 0xFFFF00, fill = 0x40, outline = 0xFF},
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
	no_alpha             = false, --fill = 0x00, outline = 0xFF for all box types
}


--------------------------------------------------------------------------------
-- game-specific modules

local function any_true(condition)
	for n in ipairs(condition) do
		if condition[n] == true then return true end
	end
end

local profile = {
	{
		game = "xmcota",
		number_players = 2,
		address = {
			player         = 0xFF4000,
			match_status   = 0xFF4800,
			projectile_ptr = 0xFFD6C4,
			stage          = 0xFF488F,
		},
		offset = {
			facing_dir           = 0x4D,
			character_id         = 0x50,
			addr_table_ptr       = 0x88, 
			projectile_ptr_space = 0x1C8,
			stage_base = {
				-0x3680, -0x3600, -0x3600, -0x3600, -0x3600, -0x3600, -0x3600, -0x3600, 
				-0x3600, -0x3600, -0x3600, -0x3680, -0x3680, -0x3680, -0x3680, -0x3600,
			},
		},
		stage_lag = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		box_list = {
			{id_ptr = 0xA2, type = "push"},
			{id_ptr = 0x74, type = "vulnerability"},
			{id_ptr = 0x76, type = "vulnerability"},
			{id_ptr = 0x78, type = "vulnerability"},
			{id_ptr = 0x7A, type = "vulnerability"},
			{id_ptr = 0x7C, type = "throwable"},
			{id_ptr = 0x70, type = "attack"},
			{id_ptr = 0x72, type = "attack"},
		},
		pushbox_data = { base = 0x0C15E2, 
			["xmcotajr"] = -0x9FA0, --941208
			["xmcotaj3"] = -0xCEC, ["xmcotaar1"] = -0xCEC, --941217
			["xmcotaj2"] = -0xB52, --941219
			["xmcotaj1"] = -0xB24, --941222
			["xmcota"] = 0, ["xmcotaa"] = 0, ["xmcotad"] = 0, ["xmcotahr1"] = 0, ["xmcotaj"] = 0, ["xmcotau"] = 0, --950105
			["xmcotah"] = 0x36, --950331
		},
		push_check = { base = 0x00A010,
			cmd = "maincpu.pw@ff43fb = maincpu.pw@ff410a; maincpu.pw@ff47fb = maincpu.pw@ff450a",
			["xmcotajr"] = -0x614, --941208
			["xmcotaj3"] = -0x56, ["xmcotaar1"] = -0x56, --941217
			["xmcotaj2"] = -0x34, --941219
			["xmcotaj1"] = -0x32, --941222
			["xmcota"] = 0, ["xmcotaa"] = 0, ["xmcotad"] = 0, ["xmcotahr1"] = 0, ["xmcotaj"] = 0, ["xmcotau"] = 0, --950105
			["xmcotah"] = 0, --950331
		},
		throw_check = { base  = 0x01544A,
			["xmcotajr"] = -0x2E9A, --941208
			["xmcotaj3"] = -0x5CA, ["xmcotaar1"] = -0x5CA, --941217
			["xmcotaj2"] = -0x544, --941219
			["xmcotaj1"] = -0x520, --941222
			["xmcota"] = 0, ["xmcotaa"] = 0, ["xmcotad"] = 0, ["xmcotahr1"] = 0, ["xmcotaj"] = 0, ["xmcotau"] = 0, --950105
			["xmcotah"] = 0, --950331
		},
		match_active = function(p1_base, stage)
			return stage < 0xC
		end,
		no_hit = function(obj, box) return any_true({
			memory.readbyte(obj.base + 0x083) == 0,
		}) end,
		invulnerable = function(obj, box) return any_true({
			memory.readbyte(obj.base + 0x084) == 0,
		}) end,
		unpushable = function(obj, box) return any_true({
			obj.projectile ~= nil,
			memory.readword(obj.base + 0x10A) > 0,
			memory.readword(obj.base + 0x006) == 0x04,
			memory.readword(obj.base + 0x006) == 0x08,
		}) end,
		unthrowable = function(obj, box) return any_true({
			bit.band(memory.readbyte(0xFF480F), 0x08) > 0,
			memory.readbyte(obj.base + 0x125) > 0,
			memory.readbyte(0xFF4800) ~= 0x04,
			memory.readbyte(obj.base + 0x11E) >= 0x02,
			memory.readbyte(obj.base + 0x084) == 0 and memory.readbyte(obj.base + 0x10C) == 0,
			memory.readbyte(obj.base + 0x137) > 0,
			memory.readword(obj.base + 0x006) == 0 and 
			(memory.readbyte(obj.base + 0x0A0) == 0x1C or memory.readbyte(obj.base + 0x0A0) == 0x1E),
		}) end,
	},
	{
		game = "msh",
		number_players = 2,
		address = {
			player         = 0xFF4000,
			match_status   = 0xFF4800,
			projectile_ptr = 0xFFE400,
			stage          = 0xFF4893,
		},
		offset = {
			facing_dir           = 0x4D,
			character_id         = 0x50,
			addr_table_ptr       = 0x90,
			projectile_ptr_space = 0x148,
			stage_base = {
				-0x3600, -0x3600, -0x3600, -0x3600, -0x3680, -0x3600, -0x3500, -0x3600, 
				-0x3600, -0x3600, -0x3600, -0x3600, -0x3600, -0x3600, -0x3600, -0x3600,
			},
		},
		stage_lag = {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		box_list = {
			{id_ptr = 0xA2, type = "push"},
			{id_ptr = 0x78, type = "vulnerability"},
			{id_ptr = 0x7A, type = "vulnerability"},
			{id_ptr = 0x7C, type = "vulnerability"},
			{id_ptr = 0x7E, type = "vulnerability"},
			{id_ptr = 0x80, type = "throwable"},
			{id_ptr = 0x74, type = "attack"},
			{id_ptr = 0x76, type = "attack"},
		},
		pushbox_data = { base = 0x09E82C,
			["msh"] = 0, ["msha"] = 0, ["mshjr1"] = 0, ["mshud"] = 0, ["mshu"] = 0, --951024
			["mshb"] = 0x132, ["mshh"] = 0x132, ["mshj"] = 0x132, --951117
		},
		push_check = { base = 0x00DD0A,
			cmd = "maincpu.pb@ff43fc = maincpu.pb@ff410a; maincpu.pb@ff47fc = maincpu.pb@ff450a",
			["msh"] = 0, ["msha"] = 0, ["mshjr1"] = 0, ["mshud"] = 0, ["mshu"] = 0, --951024
			["mshb"] = 0x8, ["mshh"] = 0x8, ["mshj"] = 0x8, --951117
		},
		throw_check = { base  = 0x091882, 
			["msh"] = 0, ["msha"] = 0, ["mshjr1"] = 0, ["mshud"] = 0, ["mshu"] = 0, --951024
			["mshb"] = 0x18, ["mshh"] = 0x18, ["mshj"] = 0x18, --951117
		},
		match_active = function(p1_base, stage)
			return memory.readdword(0xFF8FA2) == p1_base
		end,
		no_hit = function(obj, box) return any_true({
			memory.readbyte(obj.base + 0x086) == 0,
		}) end,
		invulnerable = function(obj, box) return any_true({
			memory.readbyte(obj.base + 0x087) == 0,
		}) end,
		unpushable = function(obj, box) return any_true({
			obj.projectile ~= nil,
			memory.readbyte(obj.base + 0x10A) > 0,
			memory.readword(obj.base + 0x006) == 0x04,
		}) end,
		unthrowable = function(obj, box) return any_true({
			bit.band(memory.readbyte(0xFF480F), 0x08) > 0,
			memory.readbyte(obj.base + 0x125) > 0,
			memory.readbyte(0xFF4800) ~= 0x08,
			memory.readbyte(obj.base + 0x11E) >= 0x02,
			memory.readbyte(obj.base + 0x087) == 0 and memory.readbyte(obj.base + 0x10C) == 0,
			memory.readbyte(obj.base + 0x137) > 0,
			memory.readword(obj.base + 0x006) == 0 and 
			(memory.readbyte(obj.base + 0x0A0) == 0x24 or memory.readbyte(obj.base + 0x0A0) == 0x26),
		}) end,
	},
	{
		game = "xmvsf",
		number_players = 4,
		address = {
			player         = 0xFF4000,
			match_status   = 0xFF5000,
			projectile_ptr = 0xFFE3D8,
			stage          = 0xFF5113,
		},
		offset = {
			facing_dir           = 0x4B,
			character_id         = 0x52,
			addr_table_ptr       = 0x6C,
			projectile_ptr_space = 0x150,
			stage_base = {
				-0x2CC0, -0x2DC0, -0x2DC0, -0x2D40, -0x2DC0, -0x2DC0, -0x2CC0, -0x2DC0, 
				-0x2D40, -0x2D40, -0x2DC0, -0x2DC0, -0x2DC0, -0x2DC0, -0x2DC0, -0x2DC0,
			},
		},
		stage_lag = {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		box_list = {
			{id_ptr = 0xA4, projectile_id_ptr = 0xB2, type = "push"},
			{id_ptr = 0x74, type = "vulnerability"},
			{id_ptr = 0x76, type = "vulnerability"},
			{id_ptr = 0x78, type = "vulnerability"},
			{id_ptr = 0x7A, type = "vulnerability"},
			{id_ptr = 0x7C, type = "throwable"},
			{id_ptr = 0x70, type = "attack"},
			{id_ptr = 0x72, type = "attack"},
		},
		pushbox_data = { base = 0x08B022, 
			["xmvsfjr2"] = -0x16E, --960909
			["xmvsfar2"] = -0x134, ["xmvsfr1"] = -0x134, ["xmvsfjr1"] = -0x134, --960910
			["xmvsfar1"] = 0, --960919
			["xmvsf"] = 0, ["xmvsfh"] = 0, ["xmvsfj"] = 0, ["xmvsfu1d"] = 0, ["xmvsfur1"] = 0, --961004
			["xmvsfa"] = 0x2E, ["xmvsfb"] = 0x2E, ["xmvsfu"] = 0x2E, --961023
		},
		push_check = { base = 0x0104D4,
			cmd = "maincpu.pb@ff43fc = maincpu.pb@ff4105; maincpu.pb@ff47fc = maincpu.pb@ff4505",
			["xmvsfjr2"] = -0x28, --960909
			["xmvsfar2"] = -0x22, ["xmvsfr1"] = -0x22, ["xmvsfjr1"] = -0x22, --960910
			["xmvsfar1"] = 0, --960919
			["xmvsf"] = 0, ["xmvsfh"] = 0, ["xmvsfj"] = 0, ["xmvsfu1d"] = 0, ["xmvsfur1"] = 0, --961004
			["xmvsfa"] = 0xE, ["xmvsfb"] = 0xE, ["xmvsfu"] = 0xE, --961023
		},
		throw_check = { base  = 0x07CC62, 
			["xmvsfjr2"] = -0x74, --960909
			["xmvsfar2"] = -0x4A, ["xmvsfr1"] = -0x4A, ["xmvsfjr1"] = -0x4A, --960910
			["xmvsfar1"] = 0x8, --960919
			["xmvsf"] = 0, ["xmvsfh"] = 0, ["xmvsfj"] = 0, ["xmvsfu1d"] = 0, ["xmvsfur1"] = 0, --961004
			["xmvsfa"] = 0x2E, ["xmvsfb"] = 0x2E, ["xmvsfu"] = 0x2E, --961023
		},
		match_active = function(p1_base, stage)
			return memory.readdword(0xFFA014) == p1_base
		end,
		no_hit = function(obj, box) return any_true({
			memory.readbyte(obj.base + 0x082) == 0,
		}) end,
		invulnerable = function(obj, box) return any_true({
			memory.readbyte(obj.base + 0x083) == 0,
		}) end,
		unpushable = function(obj, box) return any_true({
			obj.projectile ~= nil and memory.readword(obj.base + 0x24) ~= 0x94, --Apocalypse fist
			not obj.projectile and obj.base > 0xFF4400, --only the two point chars can push
			not obj.projectile and memory.readbyte(obj.base + 0x105) > 0,
			not obj.projectile and memory.readword(obj.base + 0x006) == 0x04,
		}) end,
		unthrowable = function(obj, box) return any_true({
			bit.band(memory.readbyte(0xFF500F), 0x08) > 0,
			memory.readbyte(obj.base + 0x221) > 0,
			memory.readbyte(0xFF5031) > 0,
			memory.readbyte(obj.base + 0x120) > 0,
			memory.readbyte(0xFF5000) ~= 0x08,
			memory.readbyte(obj.base + 0x10A) >= 0x02,
			memory.readbyte(obj.base + 0x083) == 0,
			memory.readword(obj.base + 0x006) == 0 and 
			(memory.readbyte(obj.base + 0x0A0) == 0x24 or memory.readbyte(obj.base + 0x0A0) == 0x26),
			memory.readword(obj.base + 0x006) == 0x0C,
		}) end,
	},
	{
		game = "mshvsf",
		number_players = 4,
		address = {
			player         = 0xFF3800,
			match_status   = 0xFF4800,
			projectile_ptr = 0xFFE32E,
			stage          = 0xFF4913,
		},
		offset = {
			facing_dir           = 0x4B,
			character_id         = 0x52,
			addr_table_ptr       = 0x6C,
			projectile_ptr_space = 0x150,
			stage_base = {
				-0x34C0, -0x35C0, -0x35C0, -0x3540, -0x35C0, -0x35C0, -0x34C0, -0x35C0, 
				-0x3540, -0x3540, -0x35C0, -0x35C0, -0x35C0, -0x35C0, -0x35C0, -0x35C0,
			},
		},
		stage_lag = {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		box_list = {
			{id_ptr = 0xA4, projectile_id_ptr = 0xB2, type = "push"},
			{id_ptr = 0x74, type = "vulnerability"},
			{id_ptr = 0x76, type = "vulnerability"},
			{id_ptr = 0x78, type = "vulnerability"},
			{id_ptr = 0x7A, type = "vulnerability"},
			{id_ptr = 0x7C, type = "throwable"},
			{id_ptr = 0x70, type = "attack"},
			{id_ptr = 0x72, type = "attack"},
		},
		pushbox_data = { base = 0x137EE2, 
			["mshvsfa1"] = -0x52, --970620
			["mshvsf"] = 0, ["mshvsfa"] = 0, ["mshvsfb1"] = 0, ["mshvsfh"] = 0, ["mshvsfj2"] = 0, ["mshvsfu1"] = 0, ["mshvsfu1d"] = 0, --970625
			["mshvsfj1"] = 0x276, --970702
			["mshvsfj"] = 0x302, --970707
			["mshvsfu"] = 0x2E4, ["mshvsfb"] = 0x2E4, --970827
		},
		push_check = { base = 0x011922,
			cmd = "maincpu.pb@(a2+3fc) = maincpu.pb@(a2+105); maincpu.pb@(a3+3fc) = maincpu.pb@(a3+105)",
			["mshvsfa1"] = -0x34, --970620
			["mshvsf"] = 0, ["mshvsfa"] = 0, ["mshvsfb1"] = 0, ["mshvsfh"] = 0, ["mshvsfj2"] = 0, ["mshvsfu1"] = 0, ["mshvsfu1d"] = 0, --970625
			["mshvsfj1"] = -0x4, --970702
			["mshvsfj"] = -0x4, --970707
			["mshvsfu"] = -0x4, ["mshvsfb"] = -0x4, --970827
		},
		throw_check = { base  = 0x0B0A58, 
			["mshvsfa1"] = -0x3C, --970620
			["mshvsf"] = 0, ["mshvsfa"] = 0, ["mshvsfb1"] = 0, ["mshvsfh"] = 0, ["mshvsfj2"] = 0, ["mshvsfu1"] = 0, ["mshvsfu1d"] = 0, --970625
			["mshvsfj1"] = 0x276, --970702
			["mshvsfj"] = 0x302, --970707
			["mshvsfu"] = 0x2E4, ["mshvsfb"] = 0x2E4, --970827
		},
		match_active = function(p1_base, stage)
			return memory.readdword(0xFF9F2A) == p1_base
		end,
		no_hit = function(obj, box) return any_true({
			memory.readbyte(obj.base + 0x082) == 0,
		}) end,
		invulnerable = function(obj, box) return any_true({
			memory.readbyte(obj.base + 0x083) == 0,
		}) end,
		unpushable = function(obj, box) return any_true({
			obj.projectile ~= nil and memory.readword(obj.base + 0x24) ~= 0x94, --Apocalypse fist
			not obj.projectile and memory.readbyte(obj.base) == 0,
			not obj.projectile and memory.readword(obj.base + 0x006) == 0x04,
			not obj.projectile and memory.readbyte(obj.base + 0x105) > 0,
		}) end,
		unthrowable = function(obj, box) return any_true({
			bit.band(memory.readbyte(0xFF480F), 0x08) > 0,
			memory.readbyte(0xFF4831) > 0,
			memory.readbyte(obj.base + 0x261) > 0,
			memory.readbyte(obj.base + 0x120) > 0,
			memory.readbyte(0xFF4800) ~= 0x08,
			memory.readbyte(obj.base + 0x10A) >= 0x02,
			memory.readbyte(obj.base + 0x083) == 0,
			memory.readword(obj.base + 0x006) == 0 and 
			(memory.readbyte(obj.base + 0x0A0) == 0x24 or memory.readbyte(obj.base + 0x0A0) == 0x26),
			memory.readword(obj.base + 0x006) == 0x0C,
			memory.readword(obj.base + 0x006) == 0x08,
		}) end,
	},
	{
		game = "mvsc",
		number_players = 4,
		address = {
			player         = 0xFF3000,
			match_status   = 0xFF4000,
			projectile_ptr = 0xFFDF1A,
			stage          = 0xFF4113,
		},
		offset = {
			facing_dir           = 0x4B,
			character_id         = 0x52,
			addr_table_ptr       = 0x6C,
			projectile_ptr_space = 0x158,
			stage_base = {
				-0x3DA0, -0x3DA0, -0x3DA0, -0x3D20, -0x3D20, -0x3DA0, -0x3DA0, -0x3E20, 
				-0x3E20, -0x3CA0, -0x3DA0, -0x3DA0, -0x3DA0, -0x3DA0, -0x3DA0, -0x3DA0,
			},
		},
		stage_lag = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
		box_list = {
			{id_ptr = 0xB4, type = "push"},
			{id_ptr = 0x74, type = "vulnerability"},
			{id_ptr = 0x76, type = "vulnerability"},
			{id_ptr = 0x78, type = "vulnerability"},
			{id_ptr = 0x7A, type = "vulnerability"},
			{id_ptr = 0x7C, type = "throwable"},
			{id_ptr = 0x70, type = "attack"},
			{id_ptr = 0x72, type = "attack"},
		},
		pushbox_data = { base = 0x0E6FEE,
			["mvscur1"] = -0x1248, --971222
			["mvscar1"] = -0x10FA, ["mvscr1"] = -0x10FA, ["mvscjr1"] = -0x10FA, --980112
			["mvsc"] = 0, ["mvsca"] = 0, ["mvscb"] = 0, ["mvsch"] = 0, ["mvscj"] = 0, ["mvscud"] = 0, ["mvscu"] = 0, --980123
		},
		push_check = { base = 0x0137CE, 
			cmd = "maincpu.pb@(a2+3fc) = maincpu.pb@(a2+115); maincpu.pb@(a3+3fc) = maincpu.pb@(a3+115)",
			["mvscur1"] = -0x28, --971222
			["mvscar1"] = -0x34, ["mvscr1"] = -0x34, ["mvscjr1"] = -0x34, --980112
			["mvsc"] = 0, ["mvsca"] = 0, ["mvscb"] = 0, ["mvsch"] = 0, ["mvscj"] = 0, ["mvscud"] = 0, ["mvscu"] = 0, --980123
		},
		throw_check = { base = 0x0D75E8, 
			["mvscur1"] = -0x1156, --971222
			["mvscar1"] = -0x10FA, ["mvscr1"] = -0x10FA, ["mvscjr1"] = -0x10FA, --980112
			["mvsc"] = 0, ["mvsca"] = 0, ["mvscb"] = 0, ["mvsch"] = 0, ["mvscj"] = 0, ["mvscud"] = 0, ["mvscu"] = 0, --980123
		},
		match_active = function(p1_base, stage)
			return memory.readdword(0xFF963E) == p1_base
		end,
		no_hit = function(obj, box) return any_true({
			memory.readbyte(obj.base + 0x082) == 0,
		}) end,
		invulnerable = function(obj, box) return any_true({
			memory.readbyte(obj.base + 0x083) == 0,
		}) end,
		unpushable = function(obj, box) return any_true({
			obj.projectile ~= nil,
			memory.readbyte(obj.base) == 0,
			memory.readword(obj.base + 0x006) == 0x04,
			memory.readbyte(obj.base + 0x115) > 0,
		}) end,
		unthrowable = function(obj, box) return any_true({
			bit.band(memory.readbyte(0xFF400F), 0x08) > 0,
			memory.readbyte(0xFF4031) > 0,
			memory.readbyte(obj.base) == 0,
			memory.readword(obj.base + 0x270) == 0,
			memory.readbyte(obj.base + 0x130) > 0,
			memory.readbyte(0xFF4000) ~= 0x08,
			memory.readbyte(obj.base + 0x11A) >= 0x02,
			memory.readbyte(obj.base + 0x083) == 0,
			memory.readword(obj.base + 0x006) > 0,
			memory.readbyte(obj.base + 0x0B0) == 0x24,
			memory.readbyte(obj.base + 0x0B0) == 0x26,
			memory.readword(obj.base + 0x006) == 0x0C,
			memory.readword(obj.base + 0x006) == 0x08,
		}) end,
	},
}

--------------------------------------------------------------------------------
-- post-process modules

for game in ipairs(profile) do
	local g = profile[game]
	g.offset.player_space = 0x400
	g.offset.x_position   = 0x0C
	g.offset.y_position   = 0x10
	g.box = {
		radius_read = memory.readword,
		offset_read = memory.readwordsigned,
		hval = 0x0, hrad = 0x2, vval = 0x4, vrad = 0x6,
	}
	g.address.projectile_limit = g.address.player + g.offset.player_space * g.number_players
	g.match_over = g.match_over or 0x0E
end

for _,box in pairs(boxes) do
	box.fill    = bit.lshift(box.color, 8) + (globals.no_alpha and 0x00 or box.fill)
	box.outline = bit.lshift(box.color, 8) + (globals.no_alpha and (box.outline == 0x00 and 0x00 or 0xFF) or box.outline)
end

local game
local frame_buffer = {}
local DRAW_DELAY = 1
if fba then
	DRAW_DELAY = DRAW_DELAY + 1
end


--------------------------------------------------------------------------------
-- prepare the hitboxes

local get_address = function(obj, box, box_entry)
	box.id = memory.readword(obj.base + box_entry.id_ptr)
	if box.id == 0 then
		return false
	end

	box.address = memory.readdword(obj.base + game.offset.addr_table_ptr) + bit.lshift(bit.band(box.id, 0x1FFF), 3)
end


local process_box_type = {
	["vulnerability"] = function(obj, box, box_entry)
		if get_address(obj, box, box_entry) == false or game.invulnerable(obj, box) then
			return false
		elseif obj.projectile then
			box.type = "proj. vulnerability"
		end
	end,

	["attack"] = function(obj, box, box_entry)
		if get_address(obj, box, box_entry) == false or game.no_hit(obj, box) then
			return false
		elseif bit.band(box.id, 0x8000) > 0 then
			box.type = "potential throw"
			if memory.readword(obj.base + 0x06) > 0 then --hurt
				return false
			end
		elseif obj.projectile then
			box.type = "proj. attack"
		end
	end,

	["active throw"] = function(obj, box, box_entry)
		if get_address(obj, box, box_entry) == false then
			return false
		end
	end,

	["throwable"] = function(obj, box, box_entry)
		if get_address(obj, box, box_entry) == false or game.unthrowable(obj, box) then
			return false
		end
	end,

	["push"] = function(obj, box, box_entry)
		if not globals.pushbox_base or game.unpushable(obj, box) then
			return false
		end
		local count = memory.readbytesigned(obj.base + 0x3FC)
		if not obj.projectile and count > 0 then
			memory.writebyte(obj.base + 0x3FC, count - 1)
			return false
		end
		box.id = memory.readbyte(obj.base + (obj.projectile and box_entry.projectile_id_ptr or box_entry.id_ptr))
		box.address = memory.readdword(globals.pushbox_base + box.id * 2)
		box.address = box.address + bit.lshift(memory.readword(obj.base + game.offset.character_id), 2)
	end,
}


local function define_box(obj, box_entry)
	local box = {type = box_entry.type}

	if process_box_type[box.type](obj, box, box_entry) == false then
		return nil
	end

	box.hrad = game.box.radius_read(box.address + game.box.hrad)
	box.vrad = game.box.radius_read(box.address + game.box.vrad)
	box.hval = game.box.offset_read(box.address + game.box.hval)
	box.vval = game.box.offset_read(box.address + game.box.vval)

	box.hval   = obj.pos_x + box.hval * (bit.band(obj.facing_dir, 1) > 0 and -1 or 1)
	box.vval   = obj.pos_y + box.vval * (bit.band(obj.facing_dir, 2) > 0 and box.type ~= "push" and -1 or 1)
	box.left   = box.hval - box.hrad
	box.right  = box.hval + box.hrad
	box.top    = box.vval - box.vrad
	box.bottom = box.vval + box.vrad

	return box
end


local function update_object(obj)
	obj.facing_dir   = memory.readbyte(obj.base + game.offset.facing_dir)
	obj.pos_x        = memory.readwordsigned(obj.base + game.offset.x_position) - globals.left_screen_edge
	obj.pos_y        = memory.readwordsigned(obj.base + game.offset.y_position) - globals.top_screen_edge - 0x0F

	for entry = 1, #game.box_list do
		table.insert(obj, define_box(obj, game.box_list[entry]))
	end
	return obj
end


local function read_projectiles(object_list)
	for player = 1, 2 do
		local i = 1
		while i do
			local obj = {base = memory.readdword(game.address.projectile_ptr + (player-1) * game.offset.projectile_ptr_space - i * 4)}
			if obj.base < game.address.projectile_limit then
				i = nil
			else
				obj.projectile = true
				table.insert(object_list, update_object(obj))
				i = i + 1
			end
		end
	end
end


local function update_hitboxes()
	if not game then
		return
	end
	globals.stage            = bit.band(memory.readbyte(game.address.stage), 0xF)
	globals.stage_base       = 0xFF8000 + game.offset.stage_base[globals.stage + 1]
	globals.match_active     = memory.readbyte(game.address.match_status)
	globals.match_active     = globals.match_active > 0 and globals.match_active <= game.match_over and 
		game.match_active(game.address.player, globals.stage)
	globals.left_screen_edge = memory.readwordsigned(globals.stage_base + game.offset.x_position) + 0x40
	globals.top_screen_edge  = memory.readwordsigned(globals.stage_base + game.offset.y_position)

	local effective_delay = DRAW_DELAY + (not mame and 0 or game.stage_lag[globals.stage + 1])

	for f = 1, effective_delay do
		frame_buffer[f] = copytable(frame_buffer[f+1])
	end

	frame_buffer[effective_delay+1] = {match_active = globals.match_active}

	for p = 1, game.number_players do
		local player = {base = game.address.player + (p-1) * game.offset.player_space}
		if game.number_players <= 2 or memory.readbyte(player.base) > 0 then
			table.insert(frame_buffer[effective_delay+1], update_object(player))
		end
	end
	read_projectiles(frame_buffer[effective_delay+1])

	for _, prev_frame in ipairs(frame_buffer[effective_delay] or {}) do
		if prev_frame.projectile then
			break
		elseif memory.readbyte(prev_frame.base + 0x3FD) > 0 then
			table.insert(prev_frame, define_box(prev_frame, {id_ptr = 0x3FE, type = "active throw"}))
			memory.writebyte(prev_frame.base + 0x3FD, memory.readbyte(prev_frame.base + 0x3FD) - 1)
			--memory.writeword(prev_frame.base + 0x3FE, 0)
		end
	end
end


emu.registerafter(function()
	update_hitboxes()
end)


--------------------------------------------------------------------------------
-- draw the hitboxes

local function draw_hitbox(hb)
	if not hb or any_true({
		not globals.draw_pushboxes and hb.type == "push",
		not globals.draw_throwable_boxes and (hb.type == "potential throw" or hb.type == "throwable"),
	}) then return
	end

	if globals.draw_mini_axis then
		gui.drawline(hb.hval, hb.vval-globals.mini_axis_size, hb.hval, hb.vval+globals.mini_axis_size, boxes[hb.type].outline)
		gui.drawline(hb.hval-globals.mini_axis_size, hb.vval, hb.hval+globals.mini_axis_size, hb.vval, boxes[hb.type].outline)
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, boxes[hb.type].fill, boxes[hb.type].outline)
end


local function draw_axis(obj)
	gui.drawline(obj.pos_x, obj.pos_y-globals.axis_size, obj.pos_x, obj.pos_y+globals.axis_size, globals.axis_color)
	gui.drawline(obj.pos_x-globals.axis_size, obj.pos_y, obj.pos_x+globals.axis_size, obj.pos_y, globals.axis_color)
	--gui.text(obj.pos_x, obj.pos_y + 8, string.format("%06X", obj.base)) --debug
end


local function render_hitboxes()
	gui.clearuncommitted()
	if not game or not frame_buffer[1].match_active then
		return
	end

	if globals.blank_screen then
		gui.box(0, 0, emu.screenwidth(), emu.screenheight(), globals.blank_color)
	end

	for entry = 1, #game.box_list do
		for _, obj in ipairs(frame_buffer[1]) do
			draw_hitbox(obj[entry])
		end
	end

	if globals.draw_axis then
		for _, obj in ipairs(frame_buffer[1]) do
			draw_axis(obj)
		end
	end
end


gui.register(function()
	render_hitboxes()
end)


--------------------------------------------------------------------------------
-- hotkey functions

input.registerhotkey(1, function()
	globals.blank_screen = not globals.blank_screen
	render_hitboxes()
	emu.message((globals.blank_screen and "activated" or "deactivated") .. " blank screen mode")
end)


input.registerhotkey(2, function()
	globals.draw_axis = not globals.draw_axis
	render_hitboxes()
	emu.message((globals.draw_axis and "showing" or "hiding") .. " object axis")
end)


input.registerhotkey(3, function()
	globals.draw_mini_axis = not globals.draw_mini_axis
	render_hitboxes()
	emu.message((globals.draw_mini_axis and "showing" or "hiding") .. " hitbox axis")
end)


input.registerhotkey(4, function()
	globals.draw_pushboxes = not globals.draw_pushboxes
	render_hitboxes()
	emu.message((globals.draw_pushboxes and "showing" or "hiding") .. " pushboxes")
end)


input.registerhotkey(5, function()
	globals.draw_throwable_boxes = not globals.draw_throwable_boxes
	render_hitboxes()
	emu.message((globals.draw_throwable_boxes and "showing" or "hiding") .. " throwable boxes")
end)


--------------------------------------------------------------------------------
-- initialize on game startup

local function initialize_fb()
	for f = 1, DRAW_DELAY + 2 do
		frame_buffer[f] = {}
	end
end


local function whatgame()
	print()
	game = nil
	initialize_fb()
	for _, module in ipairs(profile) do
		if emu.romname() == module.game or emu.parentname() == module.game then
			print("drawing " .. emu.romname() .. " hitboxes") print()
			game = module
			if game.pushbox_data[emu.romname()] then
				globals.pushbox_base = game.pushbox_data.base + game.pushbox_data[emu.romname()]
			else
				globals.pushbox_base = nil
				print("Unrecognized version [" .. emu.romname() .. "]: cannot draw pushboxes")
			end
			if not mame then
				return
			end
			local bpstring, instruction = "", ""
			if game.push_check[emu.romname()] and globals.pushbox_base then
				bpstring = string.format("bp %06X, 1, {%s; g}; ", 
					game.push_check.base + game.push_check[emu.romname()], game.push_check.cmd)
				instruction = "pushboxes"
			elseif globals.pushbox_base then
				print("Unrecognized version [" .. emu.romname() .. "]: cannot accurately draw pushboxes")
			end
			if game.throw_check[emu.romname()] then
				bpstring = string.format("%sbp %06X, 1, {maincpu.pw@(a6+3fe) = d0; maincpu.pb@(a6+3fd) = maincpu.pb@(a6+3fd)+1; g}; ", 
					bpstring, game.throw_check.base + game.throw_check[emu.romname()])
				instruction = instruction .. (instruction:len() > 0 and " and " or "") .. "throwboxes"
			else
				print("Unrecognized version [" .. emu.romname() .. "]: cannot draw active throwboxes")
			end
			if bpstring:len() > 0 then
				print("Copy this line into the MAME-rr debugger for more accurate " .. instruction .. ":") print()
				print(bpstring:sub(1, -3))
			end
			return
		end
	end
	print("not prepared for " .. emu.romname() .. " hitboxes")
end


savestate.registerload(function()
	initialize_fb()
end)


emu.registerstart(function()
	whatgame()
end)