--[[
Scrolling input display Lua script
requires the Lua gd library (http://luaforge.net/projects/lua-gd/)
written by Dammit (dammit9x at hotmail dot com)

Works with MAME, FBA, pcsx, snes9x and Gens:
http://code.google.com/p/mame-rr/downloads/list
http://code.google.com/p/fbarr/downloads/list
http://code.google.com/p/pcsxrr/downloads/list
http://code.google.com/p/snes9x-rr/downloads/list
http://code.google.com/p/gens-rerecording/downloads/list
]]

version      = "11/10/2010"

iconfile     = "icons-capcom-32.png"  --file containing the icons to be shown

buffersize   = 10     --how many lines to show
margin_left  = 1      --space from the left of the screen, in tiles, for player 1
margin_right = 3      --space from the right of the screen, in tiles, for player 2
margin_top   = 2      --space from the top of the screen, in tiles
timeout      = 240    --how many idle frames until old lines are cleared on the next input
screenwidth  = 256    --pixel width of the screen for spacing calculations (only applies if emu.screenwidth() is unavailable)

--Key bindings below only apply if the emulator does not support Lua hotkeys.
playerswitch = "Q"         --key pressed to toggle players on/off
clearkey     = "tilde"     --key pressed to clear screen
sizekey      = "semicolon" --key pressed to change icon size
scalekey     = "quote"     --key pressed to toggle icon stretching
recordkey    = "numpad/"   --key pressed to start/stop recording video

----------------------------------------------------------------------------------------------------

gamekeys = {
	{ set =
		{ "capcom",   snes9x,    gens,       pcsx,            fba,       mame },
		{      "l",   "left",  "left",     "left",         "Left",     "Left" },
		{      "r",  "right", "right",    "right",        "Right",    "Right" },
		{      "u",     "up",    "up",       "up",           "Up",       "Up" },
		{      "d",   "down",  "down",     "down",         "Down",     "Down" },
		{     "ul"},
		{     "ur"},
		{     "dl"},
		{     "dr"},
		{     "LP",      "Y",     "X",   "square",   "Weak Punch", "Button 1" },
		{     "MP",      "X",     "Y", "triangle", "Medium Punch", "Button 2" },
		{     "HP",      "L",     "Z",       "r1", "Strong Punch", "Button 3" },
		{     "LK",      "B",     "A",        "x",    "Weak Kick", "Button 4" },
		{     "MK",      "A",     "B",   "circle",  "Medium Kick", "Button 5" },
		{     "HK",      "R",     "C",       "r2",  "Strong Kick", "Button 6" },
		{      "S", "select",  "none",    "start",        "Start",    "Start" },
	},
	{ set =
		{ "neogeo",       pcsx,        fba,       mame },
		{      "l",     "left",     "Left",     "Left" },
		{      "r",    "right",    "Right",    "Right" },
		{      "u",       "up",       "Up",       "Up" },
		{      "d",     "down",     "Down",     "Down" },
		{     "ul"},
		{     "ur"},
		{     "dl"},
		{     "dr"},
		{      "A",   "square", "Button A", "Button 1" },
		{      "B", "triangle", "Button B", "Button 2" },
		{      "C",        "x", "Button C", "Button 3" },
		{      "D",   "circle", "Button D", "Button 4" },
		{      "S",    "start",    "Start",    "Start" },
	},
	{ set =
		{ "tekken",       pcsx,       mame },
		{      "l",     "left",     "Left" },
		{      "r",    "right",    "Right" },
		{      "u",       "up",       "Up" },
		{      "d",     "down",     "Down" },
		{     "ul"},
		{     "ur"},
		{     "dl"},
		{     "dr"},
		{      "1",   "square", "Button 1" },
		{      "2", "triangle", "Button 2" },
		{      "T",        nil, "Button 3" },
		{      "3",        "x", "Button 4" },
		{      "4",   "circle", "Button 5" },
		{      "S",    "start",    "Start" },
	},
}

--folder with scrolling-input-code.lua, icon files, & frame dump folder (relative to this lua file)
resourcepath = "scrolling-input"

dofile(resourcepath .. "/scrolling-input-code.lua")
