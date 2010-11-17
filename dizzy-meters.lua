print("Dizzy/Stun meter viewer")
print("written by Dammit")
print("November 17, 2010")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle numbers") print()

local color = {
	back          = 0x00000060,
	border        = 0x000000FF,
	level         = 0xFF0000FF,
	long_level    = 0xFFAAAAFF,
	timeout       = 0xFFFF00FF,
	long_timeout  = 0xFFA000FF,
	duration      = 0x00C0FFFF,
	long_duration = 0xA0FFFFFF,
}
local STUN = {
	fill   = 0xF8B000FF,
	shade  = 0xB06000FF,
	border = 0x500000FF,
}

local HARD_LIMIT        = 1
local PTR_LIMIT         = 2
local PTR_LIMIT_BASE    = 3
local PTR_LIMIT_NO_BASE = 4
local HSF2_LIMIT        = 5
local DIRECT            = 1
local POINTER           = 2
local NORMAL            = 1
local SF2               = 2
local SFA               = 3

local profile = {
	{
		games        = {"hsf2"},
		active       = 0xFF836D,
		player       = 0xFF833C,
		level        = 0x05E,
		limit        = 0x2B1,
		timeout      = 0x05C,
		duration     = 0x060,
		dizzy        = 0x1F0,
		countdown    = 0x1F0, --dummy
		char_mode    = 0x32A,
		read_func    = memory.readword,
		limit_func   = HSF2_LIMIT,
		limit_base_array = {
			[0x06D830] = {"hsf2", "hsf2a", "hsf2d"}, --040202
			[0x06D828] = {"hsf2j"}, --031222
		},
		limit_read   = memory.readbyte,
		bar_X        = 0x18,
		bar_Y        = 0x20,
		bar_length   = 0x40,
		bar_height   = 0x04,
		max_timeout  = 180,
		max_duration = 180,
	},
	{
		games        = {"ssf2t"},
		active       = 0xFF847F,
		player       = 0xFF844E,
		limit_func   = PTR_LIMIT_BASE,
		limit_base_array = {
			[0x07F1C6] = {"ssf2t", "ssf2ta", "ssf2tur1", "ssf2xjd", "ssf2xj", "ssf2tu"}, --940223, 940323
		},
		limit_read   = memory.readbyte,
	},
	{
		games        = {"ssf2"},
		active       = 0xFF83FF,
		player       = 0xFF83CE,
		limit_func   = HARD_LIMIT,
		limit        = 0x1F,
	},
	{
		games        = {"sf2ce","sf2hf"},
		active       = 0xFF83EF,
		player       = 0xFF83BE,
		space        = 0x300,
		dizzy        = 0x123,
		countdown    = 0x124,
		countdown_check = SF2,
	},
	{
		games        = {"sf2"},
		active       = 0xFF83F7,
		player       = 0xFF83C6,
		space        = 0x300,
		limit        = 0x1E,
		countdown_check = SF2,
	},
	{
		games        = {"sfa"},
		active       = 0xFF8280,
		player       = 0xFF8400,
		level        = 0x137,
		limit        = 0x13A,
		timeout      = 0x136,
		duration     = 0x02D,
		countdown    = 0x006,
		dizzy        = 0x13B,
		limit_func   = PTR_LIMIT,
		read_func    = memory.readbyte,
		duration_func = memory.readbyte,
		countdown_check = SFA,
		bar_X        = 0x10,
		bar_Y        = 0x22,
		max_timeout  = 210,
	},
	{
		games        = {"sfa2","sfz2al"},
		active       = 0xFF812D,
		player       = 0xFF8400,
		duration     = 0x03B,
		countdown    = 0x006,
		duration_func = memory.readbyte,
		countdown_check = SFA,
		bar_X        = 0x60,
		bar_Y        = 0x1C,
	},
	{
		games        = {"sfa3"},
		active       = 0xFF812D,
		player       = 0xFF8400,
		level        = 0x2CC,
		limit        = 0x2CD,
		timeout      = 0x2CB,
		duration     = 0x03B,
		dizzy        = 0x2CF,
		countdown    = 0x2CF, --dummy
		duration_func = memory.readbyte,
		bar_X        = 0x48,
		bar_Y        = 0x06,
		max_timeout  = 180,
	},
	{
		games        = {"xmcota"},
		active       = 0xFF4BA4,
		player       = 0xFF4000,
		level        = 0x0B9,
		limit        = 0x050,
		timeout      = 0x0BA,
		duration     = 0x0FC,
		dizzy        = 0x13A,
		countdown    = 0x0BB,
		limit_func   = PTR_LIMIT_BASE,
		limit_base_array = {
			[0x0B7DF4] = {"xmcotajr"}, --941208
			[0x0C10C2] = {"xmcotaa", "xmcotaj3"}, --941217
			[0x0C125C] = {"xmcotaj2"}, --941219
			[0x0C128A] = {"xmcotaj1"}, --941222
			[0x0C1DAE] = {"xmcota", "xmcotad", "xmcotahr1", "xmcotaj", "xmcotau"}, --950105
			[0x0C1DE4] = {"xmcotah"}, --950331
		},
		limit_read   = memory.readword,
		bar_X        = 0x60,
		bar_Y        = 0x28,
	},
	{
		games        = {"msh"},
		active       = 0xFF8EC3,
		player       = 0xFF4000,
		limit_func   = PTR_LIMIT_BASE,
		limit_base_array = {
			[0x09F34A] = {"msh", "msha", "mshjr1", "mshud", "mshu"}, --951024
			[0x09F47C] = {"mshb", "mshh", "mshj"}, --951117
		},
		limit_read   = memory.readword,
		bar_X        = 0x10,
		bar_Y        = 0x24,
	},
	{
		games        = {"xmvsf"},
		active       = 0xFF5400,
		player       = 0xFF4000,
		limit        = 0x052,
		limit_func   = PTR_LIMIT_BASE,
		limit_base_array = {
			[0x08BAFE] = {"xmvsfjr2"}, --960909
			[0x08BB38] = {"xmvsfar2", "xmvsfr1", "xmvsfjr1"}, --960910
			[0x08BC6C] = {"xmvsf", "xmvsfar1", "xmvsfh", "xmvsfj", "xmvsfu1d", "xmvsfur1"}, --960919, 961004
			[0x08BC9A] = {"xmvsfa", "xmvsfb", "xmvsfu"}, --961023
		},
		limit_read   = memory.readword,
		duration     = 0x138,
		dizzy        = 0x135,
		bar_X        = 0x14,
		bar_Y        = 0x0A,
	},
	{
		games        = {"mshvsf"},
		active       = 0xFF4C00,
		player_ptr   = 0xFF48C8,
		space        = 0x8,
		limit        = 0x052,
		limit_func   = PTR_LIMIT_BASE,
		limit_base_array = {
			[0x138C3E] = {"mshvsfa1"}, --970620
			[0x138C90] = {"mshvsf", "mshvsfa", "mshvsfb1", "mshvsfh", "mshvsfj2", "mshvsfu1", "mshvsfu1d"}, --970625
			[0x138F06] = {"mshvsfj1"}, --970702
			[0x138F92] = {"mshvsfj"}, --970707
			[0x138F74] = {"mshvsfu", "mshvsfb"}, --970827
		},
		limit_read   = memory.readword,
		bar_Y        = 0x20,
	},
	{
		games        = {"mvsc"},
		active       = 0xFF62B7,
		player_ptr   = 0xFF40C8,
		space        = 0x8,
		level        = 0x0C9,
		timeout      = 0x0CA,
		duration     = 0x146, --dummy
		dizzy        = 0x145,
		countdown    = 0x146, --dummy
		limit_func   = PTR_LIMIT_BASE,
		limit_base_array = {
			[0x0E6A8E] = {"mvscur1"}, --971222
			[0x0E6BDC] = {"mvscar1", "mvscr1", "mvscjr1"}, --980112
			[0x0E7CD6] = {"mvsc", "mvsca", "mvscb", "mvsch", "mvscj", "mvscud", "mvscu"}, --980123
		},
		limit_read   = memory.readword,
		bar_Y        = 0x2C,
		max_timeout  = 60,
	},
	{
		games        = {"sgemf"},
		active       = 0xFFCBBC,
		player       = 0xFF8400,
		level        = 0x17F,
		limit        = 0x19E,
		timeout      = 0x19F,
		duration     = 0x146,
		dizzy        = 0x146, --dummy
		countdown    = 0x146, --dummy
		limit_func   = PTR_LIMIT,
		duration_func = memory.readbyte,
		bar_X        = 0x20,
		bar_Y        = 0x08,
		max_timeout  = 180,
	},
	{
		games        = {"ringdest"},
		active       = 0xFF72D2,
		player       = 0xFF8000,
		level        = 0x0AD,
		limit        = 0x0CD,
		timeout      = 0x0AF,
		duration     = 0x0CE,
		dizzy        = 0x0AB,
		countdown    = 0x0AB, --dummy
		limit_func   = PTR_LIMIT,
		bar_X        = 0x18,
		bar_Y        = 0x0C,
		max_timeout  = 80,
	},
}

for n, g in ipairs(profile) do
	local last = profile[n-1]
	g.space           = g.space           or 0x400
	g.duration_func   = g.duration_func   or memory.readword
	g.countdown_check = g.countdown_check or NORMAL
	g.base            = g.player_ptr and POINTER or DIRECT
	g.level           = g.level           or last.level
	g.limit           = g.limit           or last.limit
	g.timeout         = g.timeout         or last.timeout
	g.duration        = g.duration        or last.duration
	g.dizzy           = g.dizzy           or last.dizzy
	g.countdown       = g.countdown       or last.countdown
	g.limit_func      = g.limit_func      or last.limit_func
	g.read_func       = g.read_func       or last.read_func
	g.max_timeout     = g.max_timeout     or last.max_timeout
	g.max_duration    = g.max_duration    or last.max_duration
	g.bar_X           = g.bar_X           or last.bar_X
	g.bar_Y           = g.bar_Y           or last.bar_Y
	g.bar_length      = g.bar_length      or last.bar_length
	g.bar_height      = g.bar_height      or last.bar_height
end

--------------------------------------------------------------------------------

local show_numbers = true
input.registerhotkey(1, function()
	show_numbers = not show_numbers
	print((show_numbers and "showing" or "hiding") .. " numbers")
end)


local game, limit_func, center, level, timeout, duration
local player = {}

local get_player_base = {
	[DIRECT] = function(p)
		return game.player + (p-1)*game.space
	end,

	[POINTER] = function(p)
		return memory.readdword(game.player_ptr + (p-1)*game.space)
	end,
}


local get_limit = {
	[HARD_LIMIT] = function(p)
		return game.limit
	end,

	[PTR_LIMIT] = function(p)
		return memory.readbyte(player[p].base + game.limit)
	end,

	[PTR_LIMIT_BASE] = function(p)
		return memory.readword(game.limit_base + game.limit_read(player[p].base + game.limit))
	end,

	[PTR_LIMIT_NO_BASE] = function(p)
		return 64
	end,

	[HSF2_LIMIT] = function(p)
		local opponent = memory.readbyte(game.player + (p == 1 and 1 or 0)*game.space + game.char_mode)
		if opponent == 4 then --WW
			return 0x1E
		elseif opponent == 6 or opponent == 8 then --CE or HF
			return 0x1F
		else --Super or ST
			return memory.readword(game.limit_base + game.limit_read(player[p].base + game.limit))
		end
	end,
}


local get_countdown_status = {
	[NORMAL] = function(p)
		return player[p].dizzy and memory.readbyte(player[p].base + game.countdown) ~= 0
	end,

	[SF2] = function(p)
		return memory.readbyte(player[p].base + game.countdown) ~= 0
	end,

	[SFA] = function(p)
		return memory.readbyte(player[p].base + game.countdown) == 0x12
	end,
}

--------------------------------------------------------------------------------

local function whatversion(game)
	for base,version_set in pairs(game.limit_base_array) do
		for _,version in ipairs(version_set) do
			if emu.romname() == version then
				return base
			end
		end
	end
	print("unrecognized version (" .. emu.romname() .. "): limits will be wrong")
	limit_func = get_limit[PTR_LIMIT_NO_BASE]
	return nil
end


local function whatgame()
	game = nil
	for _, module in ipairs(profile) do
		for _, shortname in ipairs(module.games) do
			if emu.romname() == shortname or emu.parentname() == shortname then
				print("drawing " .. emu.romname() .. " dizzy meters")
				game = module
				center = emu.screenwidth and emu.screenwidth()/2 or 128
				limit_func = get_limit[game.limit_func]
				if game.limit_base_array then
					game.limit_base = whatversion(game)
				end
				for p = 1, 2 do
					player[p] = {timeout = {}, level = {}, duration = {}, side = p%2 == 1 and -1 or 1, bg = {}}
					player[p].inner = center + game.bar_X * player[p].side
					for n = 1, 2 do
						player[p].bg[n] = {
							inner  = center + (game.bar_X - 1) * player[p].side,
							top    = game.bar_Y + game.bar_height*(n-1),
							outer  = center + (game.bar_X + game.bar_length + 1) * player[p].side,
							bottom = game.bar_Y + game.bar_height*n,
							border = color.border,
						}
					end
					player[p].stun_X = player[p].inner + game.bar_length/2 * player[p].side - 13
					player[p].stun_Y = player[p].bg[1].top - 1
					player[p].text_X = center + (game.bar_X + game.bar_length + 8) * player[p].side
				end
				level = {
					offset = game.level,
					func = game.read_func,
					position = 1,
					normal_color = color.level,
					long_color = color.long_level,
				}
				timeout = {
					max = game.max_timeout,
					offset = game.timeout,
					func = game.read_func,
					position = 2,
					normal_color = color.timeout,
					long_color = color.long_timeout,
				}
				duration = {
					max = game.max_duration,
					offset = game.duration,
					func = game.duration_func,
					position = 2,
					normal_color = color.duration,
					long_color = color.long_duration,
				}
				return
			end
		end
	end
	print("not prepared for " .. emu.romname() .. " dizzy bars")
end


emu.registerstart( function()
	whatgame()
end)

--------------------------------------------------------------------------------

local function load_bar(p, ref)
	local b = {}
	b.val = ref.func(player[p].base + ref.offset)
	b.max = ref.max
	local outer = b.val%b.max
	outer = game.bar_X + (outer == 0 and b.max or outer)/b.max*game.bar_length
	outer = center + outer * player[p].side
	b.outer  = outer
	b.inner  = player[p].inner
	b.top    = player[p].bg[ref.position].top + 1
	b.bottom = player[p].bg[ref.position].bottom - 1
	if b.val > b.max then
		b.color = ref.long_color
		player[p].bg[ref.position].color = ref.normal_color
	else
		b.color = ref.normal_color
		player[p].bg[ref.position].color = color.back
	end
	return b
end


local function set_text_X(p, str)
	return player[p].text_X - (p%2 == 1 and 4 * string.len(str) or 0)
end


local function update_dizzy()
	if not game then return end
	active = memory.readword(game.active) > 0
	for p = 1, 2 do
		player[p].base = get_player_base[game.base](p)
	end
	for p = 1, 2 do
		player[p].dizzy = memory.readbyte(player[p].base + game.dizzy) ~= 0
		player[p].countdown = get_countdown_status[game.countdown_check](p)

		if not player[p].countdown then
			player[p].timeout = load_bar(p, timeout)
		else
			player[p].timeout = load_bar(p, duration)
		end
		level.max = limit_func(p)
		player[p].level = load_bar(p, level)

		if player[p].dizzy or player[p].countdown then
			player[p].level.outer = center + (game.bar_X + game.bar_length) * player[p].side
			player[p].level.val = "-"
		end

		player[p].level.text_X = set_text_X(p, player[p].level.val .. "/" .. player[p].level.max)
		player[p].timeout.text_X = set_text_X(p, player[p].timeout.val)
	end
end


emu.registerafter( function()
	update_dizzy()
end)

--------------------------------------------------------------------------------

local function pixel(x1, y1, color, dx, dy)
	gui.pixel(x1 + dx, y1 + dy, color)
end

local function line(x1, y1, x2, y2, color, dx, dy)
	gui.line(x1 + dx, y1 + dy, x2 + dx, y2 + dy, color)
end

local function box(x1, y1, x2, y2, color, dx, dy)
	gui.box(x1 + dx, y1 + dy, x2 + dx, y2 + dy, color)
end

local function drawstun(x, y)
	box(0,1,6,6, STUN.border, x, y)
	line(7,3,7,5, STUN.border, x, y)
	box(1,0,28,2, STUN.border, x, y)
	box(9,3,12,6, STUN.border, x, y)
	box(14,3,28,6, STUN.border, x, y)
	box(1,1,6,5, STUN.fill, x, y)
	line(3,2,6,2, STUN.border, x, y)
	line(1,4,4,4, STUN.border, x, y)
	pixel(1,1, STUN.shade, x, y)
	pixel(1,3, STUN.shade, x, y)
	pixel(6,3, STUN.shade, x, y)
	pixel(6,5, STUN.shade, x, y)
	line(8,1,13,1, STUN.fill, x, y)
	box(10,2,11,5, STUN.fill, x, y)
	box(15,1,20,5, STUN.fill, x, y)
	box(17,1,18,4, STUN.border, x, y)
	pixel(15,5, STUN.shade, x, y)
	pixel(20,5, STUN.shade, x, y)
	box(22,1,23,5, STUN.fill, x, y)
	box(26,1,27,5, STUN.fill, x, y)
	line(24,2,25,3, STUN.fill, x, y)
	line(24,3,25,4, STUN.fill, x, y)
end


local function draw_bar(bar)
	if bar.val == 0 or not bar.inner then
		return
	end
	gui.box(bar.inner, bar.top, bar.outer, bar.bottom, bar.color, bar.border)
end


local function draw_dizzy()
	gui.clearuncommitted()
	if not game or not active then return end

	for p = 1, 2 do
		draw_bar(player[p].bg[1])
		draw_bar(player[p].bg[2])
		draw_bar(player[p].level)
		draw_bar(player[p].timeout)
		if (player[p].dizzy or player[p].countdown) and bit.band(emu.framecount(), 2) > 0 then
			drawstun(player[p].stun_X, player[p].stun_Y)
		end

		if show_numbers and player[p].level.text_X then
			gui.text(player[p].level.text_X, game.bar_Y - 2, player[p].level.val .. "/" .. player[p].level.max)
			gui.text(player[p].timeout.text_X, game.bar_Y + 6, player[p].timeout.val)
		end
	end
end


gui.register(function()
	draw_dizzy()
end)
