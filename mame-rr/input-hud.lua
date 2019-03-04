-- feos, 2019
-- this shows all the inputs current machine has mapped, snes9x style, and also current frame


-------------------------------------------------------------------------------------------
-- Ordered table iterator, allows to iterate on the natural order of the keys of a table
-- http://lua-users.org/wiki/SortedIteration
-------------------------------------------------------------------------------------------
function __genOrderedIndex( t )
    local orderedIndex = {}
    for key in pairs(t) do
        table.insert( orderedIndex, key )
    end
    table.sort( orderedIndex )
    return orderedIndex
end
function orderedNext(t, state)
    -- Equivalent of the next function, but returns the keys in the alphabetic
    -- order. We use a temporary ordered key table that is stored in the
    -- table being iterated.

    local key = nil
    --print("orderedNext: state = "..tostring(state) )
    if state == nil then
        -- the first time, generate the index
        t.__orderedIndex = __genOrderedIndex( t )
        key = t.__orderedIndex[1]
    else
        -- fetch the next value
        for i = 1,table.getn(t.__orderedIndex) do
            if t.__orderedIndex[i] == state then
                key = t.__orderedIndex[i+1]
            end
        end
    end

    if key then
        return key, t[key]
    end

    -- no more value to return, cleanup
    t.__orderedIndex = nil
    return
end
function orderedPairs(t)
    -- Equivalent of the pairs() function on tables. Allows to iterate
    -- in order
    return orderedNext, t, nil
end


local players = {}
local other = {}

function ParseInput()
	local input = joypad.get()	
	local buttons = string.format("%d\n", movie.framecount())
	
	for k, v in orderedPairs(input) do
		if k:find("^P") then
			local spacePos = k:find(" ")
			local playerNum = tonumber(k:sub(2, spacePos))
			local key = k:sub(spacePos + 1)
				:gsub("Button ", "")
				:gsub("Up",     "U")
				:gsub("Down",   "D")
				:gsub("Left",   "L")
				:gsub("Right",  "R")
			
			if players[playerNum] == nil then
			   players[playerNum] = {}
			end
			
			players[playerNum][key] = v
		else
			local key = k
				:gsub("Coin ", "C")
				:gsub("Service Mode", "SM")
			
			if k:find("Start") then
				local spacePos = k:find(" ")
				local playerNum = k:sub(1, spacePos - 1)
				key = "S"..playerNum
			end
			
			other[key] = v
		end
	end
	
	for k, v in pairs(players) do
		buttons = buttons .. "P" .. k ..": "
		for kk, vv in orderedPairs(v) do
			local value = " "
			if vv then
				value = kk
			end
			buttons = buttons .. value
		end
		buttons = buttons .. "\n"
	end
	
	for k, v in orderedPairs(other) do
		local value = " "
		if v then
			value = k
		end
		buttons = buttons .. value
	end
	
	gui.text(0, 0, buttons)
end

gui.register(ParseInput)