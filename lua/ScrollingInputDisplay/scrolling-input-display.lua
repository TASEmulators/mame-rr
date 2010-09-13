local version = "9/6/2010"

--[[
Scrolling input display Lua script
uses the Lua gd library (http://luaforge.net/projects/lua-gd/)
written by Dammit (dammit9x at hotmail dot com)

Works with MAME, FBA, pcsx, snes9x and Gens:
http://code.google.com/p/mame-rr/downloads/list
http://code.google.com/p/fbarr/downloads/list
http://code.google.com/p/pcsxrr/downloads/list
http://code.google.com/p/snes9x-rr/downloads/list
http://code.google.com/p/gens-rerecording/downloads/list

You may tweak the parameters above the dashed line.

Key bindings below only apply if Lua hotkeys are not supported.
]]

local playerswitch = "Q"                 --key pressed to toggle players on/off
local clearkey     = "tilde"             --key pressed to clear screen
local sizekey      = "semicolon"         --key pressed to change icon size
local scalekey     = "quote"             --key pressed to toggle icon stretching
local recordkey    = "numpad/"           --key pressed to start/stop recording video
local recordpath   = "framedump"         --folder to place recordings (relative to this lua file)

local buffersize   = 10                  --how many lines to show
local margin_left  = 1                   --space from the left of the screen, in tiles, for player 1
local margin_right = 3                   --space from the right of the screen, in tiles, for player 2
local margin_top   = 2                   --space from the top of the screen, in tiles
local timeout      = 240                 --how many idle frames until old lines are cleared
local opacity      = 1.0                 -- 1 = fully opaque; 0 = fully transparent

local sourcefile   = "scrolling-input-icons.png"
local bgfile       = "scrolling-input-bg.png"

----------------------------------------------------------------------------------------------------
print("Scrolling input display Lua script, " .. version)
print("Press " .. (input.registerhotkey and "Lua hotkey 1" or playerswitch) .. " to toggle players.")
print("Press " .. (input.registerhotkey and "Lua hotkey 2" or clearkey) .. " to clear the screen.")
print("Press " .. (input.registerhotkey and "Lua hotkey 3" or sizekey) .. " to resize the icons.")
print("Press " .. (input.registerhotkey and "Lua hotkey 4" or scalekey) .. " to toggle icon stretching.")
print("Press " .. (input.registerhotkey and "Lua hotkey 5" or recordkey) .. " to start/stop recording to '" .. recordpath .. "' folder")
print()

require "gd"
local minimum_tile_size = 16
local thisframe, lastframe, keyset, changed = {}, {}
local margin, rescale_icons, recording, display, start, effective_width = {}, true, false
local icon_size, image_icon_size = minimum_tile_size
local draw = { [1] = true, [2] = true }
local inp  = { [1] =   {}, [2] =   {} }
local idle = { [1] =    0, [2] =    0 }

local gamekeys = {
	emulator = 
	{         snes9x,    gens,       pcsx,            fba,       mame },
	{  "l",   "left",  "left",     "left",         "Left",     "Left" }, 
	{  "r",  "right", "right",    "right",        "Right",    "Right" }, 
	{  "u",     "up",    "up",       "up",           "Up",       "Up" }, 
	{  "d",   "down",  "down",     "down",         "Down",     "Down" }, 
	{ "ul"},
	{ "ur"},
	{ "dl"},
	{ "dr"},
	{ "LP",      "Y",     "X",   "square",   "Weak Punch", "Button 1" }, 
	{ "MP",      "X",     "Y", "triangle", "Medium Punch", "Button 2" }, 
	{ "HP",      "L",     "Z",       "r1", "Strong Punch", "Button 3" }, 
	{ "LK",      "B",     "A",        "x",    "Weak Kick", "Button 4" }, 
	{ "MK",      "A",     "B",   "circle",  "Medium Kick", "Button 5" }, 
	{ "HK",      "R",     "C",       "r2",  "Strong Kick", "Button 6" }, 
	{  "S", "select",  "none",    "start",        "Start",    "Start" },
}

for k, emu in pairs(gamekeys.emulator) do --Detect what emulator this is.
	if emu then
		keyset = k+1
		break
	end
	error("This script doesn't work on this emulator yet.", 0)
end
emu = emu or gens

----------------------------------------------------------------------------------------------------
-- display functions

local function text(x, y, row)
	gui.text(x, y, gamekeys[row][1])
end

local function image(x, y, row)
	gui.gdoverlay(x, y, gamekeys[row].img:gdStr(), opacity)
end

display = image
if not io.open(sourcefile, "rb") then
	print("Image file not found: " .. sourcefile)
	display = text
end
if not io.open(bgfile, "rb") then
	print("Image file not found: " .. bgfile)
	display = text
end
if display == text then
	print("Falling back on text mode.")
end

local function readimages()
	local scaled_width = icon_size
	if rescale_icons and emu.screenwidth and emu.screenheight then
		scaled_width = icon_size * emu.screenwidth()/emu.screenheight() / (4/3)
	end
	if display == image then
		local sourceimg = gd.createFromPng(sourcefile)
		image_icon_size = sourceimg:sizeX()/2
		for n, key in ipairs(gamekeys) do
			key.img = gd.createFromPng(bgfile)
			gd.copyResampled(key.img, sourceimg, 0,0, 0,(n-1)*image_icon_size, scaled_width,icon_size, image_icon_size,image_icon_size)
		end
	end
	effective_width = scaled_width
end
readimages()

gui.register(function()
	gui.text(0,0,"")
	for player = 1, 2 do
		if draw[player] then
			for line in pairs(inp[player]) do
				for index,row in pairs(inp[player][line]) do
					display(margin[player] + (index-1)*effective_width, margin[3] + (line-1)*icon_size, row)
				end
			end
		end
	end
end)

----------------------------------------------------------------------------------------------------
-- update functions

local function filterinput(p, frame)
	for pressed, state in pairs(joypad.getdown(p)) do --Check current controller state >
		for row, name in pairs(gamekeys) do               --but ignore non-gameplay buttons.
			if pressed == name[keyset]
		--Arcade does not distinguish joypads, so inputs must be filtered by "P1" and "P2".
			or pressed == "P" .. p .. " " .. tostring(name[keyset])
		--MAME also has unusual names for the start buttons.
			or pressed == p .. (p == 1 and " Player " or " Players ") .. tostring(name[keyset]) then
				--frame[name[1]] = state
				frame[row] = state
				break
			end
		end
	end
end

local function compositeinput(frame)          --Convert individual directions to diagonals.
	for _,v in pairs({ {1,3,5},{2,3,6},{1,4,7},{2,4,8} }) do --ul, ur, dl, dr
		if frame[v[1]] and frame[v[2]] then
			frame[v[1]], frame[v[2]] = nil, nil
			frame[v[3]] = true
			break
		end
	end
end

local function detectchanges(lastframe, thisframe)
	changed = false
	for key, state in pairs(thisframe) do       --If a key is pressed >
		if lastframe and not lastframe[key] then  --that wasn't pressed last frame >
			changed = true                          --then changes were made.
			break
		end
	end
end

local function updaterecords(player, frame, input)
	if changed then                         --If changes were made >
		if idle[player] < timeout then        --and the player hasn't been idle too long >
			for record = buffersize, 2, -1 do
				input[record] = input[record-1]   --then shift every old record by 1 >
			end
		else
			for record = buffersize, 2, -1 do
				input[record] = nil               --otherwise wipe out the old records.
			end
		end
		idle[player] = 0                      --Reset the idle count >
		input[1] = {}                         --and set current input as record 1 >
		local index = 1
		for row, name in ipairs(gamekeys) do  --but the order must not deviate from gamekeys.
			for key, state in pairs(frame) do
				if key == row then
					input[1][index] = row
					index = index+1
					break
				end
			end
		end
	else
		idle[player] = idle[player]+1         --Increment the idle count if nothing changed.
	end
end

emu.registerafter(function()
	margin[1] = margin_left*effective_width
	margin[2] = (emu.screenwidth and emu.screenwidth() or 256) - margin_right*effective_width
	margin[3] = margin_top*icon_size
	for player = 1, 2 do
		thisframe = {}
		filterinput(player, thisframe)
		compositeinput(thisframe)
		detectchanges(lastframe[player], thisframe)
		updaterecords(player, thisframe, inp[player])
		lastframe[player] = thisframe
	end
	if recording then
		gd.createFromGdStr(gui.gdscreenshot()):png(recordpath .. "/moviedump-"..string.format("%06d",movie.framecount())..".png")
	end
end)

----------------------------------------------------------------------------------------------------
-- savestate functions

if savestate.registersave and savestate.registerload then --registersave/registerload are unavailable in some emus
	savestate.registersave(function(slot)
		return draw, inp, idle
	end)

	savestate.registerload(function(slot)
		draw, inp, idle = savestate.loadscriptdata(slot)
		if type(draw) ~= "table" then draw = { [1] = true, [2] = true } end
		if type(inp)  ~= "table" then inp  = { [1] =   {}, [2] =   {} } end
		if type(idle) ~= "table" then idle = { [1] =    0, [2] =    0 } end
	end)
end

----------------------------------------------------------------------------------------------------
-- hotkey functions

local function toggleplayer()
	if draw[1] and draw[2] then
		draw[1] = false
		emu.message("Player 1 off.")
	elseif not draw[1] and draw[2] then
		draw[1] = true
		draw[2] = false
		emu.message("Player 2 off.")
	elseif draw[1] and not draw[2] then
		draw[2] = true
		emu.message("Both players on.")
	end
end

local function clear()
	inp = { [1] = {}, [2] = {} }
	emu.message("Cleared screen.")
end

local function resize(image_icon_size)
	if not image_icon_size then return end

	if icon_size < image_icon_size then
		icon_size = icon_size + minimum_tile_size/4
	else
		icon_size = minimum_tile_size
	end
	emu.message("Icon size: " .. icon_size)
	readimages()
end

local function togglescaling()
	rescale_icons = not rescale_icons
	if emu.screenwidth and emu.screenheight then
		emu.message("Icon stretching " .. (rescale_icons and "on." or "off."))
		readimages()
	else
		emu.message("This emulator does not support icon scaling.")
	end
end

local function togglerecording()
	recording = not recording
	if recording then
		start = movie.framecount()
		print("Started recording.")
	else
		local stop = movie.framecount()
		print("Stopped recording. (" .. stop - start .. " frames)")
		if stop > start then
			print("'moviedump-"..string.format("%06d",start)..".png' to 'moviedump-"..string.format("%06d",stop-1)..".png'")
		end
		print()
		start = nil
	end
end

if input.registerhotkey then
	input.registerhotkey(1, function()
		toggleplayer()
	end)

	input.registerhotkey(2, function()
		clear()
	end)

	input.registerhotkey(3, function()
		resize(image_icon_size)
	end)

	input.registerhotkey(4, function()
		togglescaling()
	end)

	input.registerhotkey(5, function()
		togglerecording()
	end)
end

local oldswitch, oldclearkey, oldsizekey, oldscalekey, oldrecordkey
emu.registerbefore( function()
	if not input.registerhotkey then        --use input.get if registerhotkey is unavailable
		local nowswitch = input.get()[playerswitch]
		if nowswitch and not oldswitch then
				toggleplayer()
		end
		oldswitch = nowswitch

		local nowclearkey = input.get()[clearkey]
		if nowclearkey and not oldclearkey then
			clear()
		end
		oldclearkey = nowclearkey

		local nowsizekey = input.get()[sizekey]
		if nowsizekey and not oldsizekey then
			resize(image_icon_size)
		end
		oldsizekey = nowsizekey

		local nowscalekey = input.get()[scalekey]
		if nowscalekey and not oldscalekey then
			togglescaling()
		end
		oldscalekey = nowscalekey

		local nowrecordkey = input.get()[recordkey]
		if nowrecordkey and not oldrecordkey then
			togglerecording()
		end
		oldrecordkey = nowrecordkey
	end
end)