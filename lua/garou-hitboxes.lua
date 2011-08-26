print("Garou Densetsu/Fatal Fury hitbox viewer")
print("August 25, 2011")
print("http://code.google.com/p/mame-rr/")
print("Lua hotkey 1: toggle blank screen")
print("Lua hotkey 2: toggle object axis")
print("Lua hotkey 3: toggle hitbox axis")
print("Lua hotkey 4: toggle pushboxes")
if fba then
	print("Warning: This doesn't work with FBA-rr.")
end

local boxes = {
	["vulnerability"] = {color = 0x7777FF, fill = 0x40, outline = 0xFF},
	       ["attack"] = {color = 0xFF0000, fill = 0x40, outline = 0xFF},
	 ["proj. attack"] = {color = 0xFF66FF, fill = 0x40, outline = 0xFF},
	         ["push"] = {color = 0x00FF00, fill = 0x20, outline = 0xFF},
	        ["guard"] = {color = 0xCCCCFF, fill = 0x40, outline = 0xFF},
	        ["throw"] = {color = 0xFFFF00, fill = 0x40, outline = 0xFF},
	   ["axis throw"] = {color = 0xFFAA00, fill = 0x40, outline = 0xFF},
}

local globals = {
	axis_color      = 0xFFFFFFFF,
	blank_color     = 0xFFFFFFFF,
	axis_size       = 12,
	mini_axis_size  = 2,
	blank_screen    = false,
	draw_axis       = true,
	draw_mini_axis  = false,
	draw_pushboxes  = true,
	no_alpha        = false, --fill = 0x00, outline = 0xFF for all box types
	throwbox_height = 0x50, --default for ground throws
}

--------------------------------------------------------------------------------
-- game-specific modules

local offset = {
	player_space = 0x100,
	x_position   = 0x20,
	z_position   = 0x24,
	y_position   = 0x28,
}

local a,v,p,g,t,x = "attack","vulnerability","push","guard","throw","undefined"

bit.btst = function(val, n)
	return bit.band(bit.rshift(val, n), 1)
end

local profile = {
	{
		games = {"fatfury1"},
		number_players = 3,
		no_combos = true,
		match_active = 0x1042CC,
		stage_base   = 0x104100,
		player_base  = 0x100300,
		box_types = {
			v,p,g,g
		},
		box = {
			top = 0x6, bottom = 0x8, left = 0x2, right = 0x4, space = 0xA, scale = 1, header = 2, 
			read = memory.readwordsigned,
			get_id = function(address)
				return memory.readword(address)
			end
		},
		adjust_y = function(y)
			return y - globals.top_screen_edge
		end,
		update_object = function(obj)
			obj.facing_dir = bit.band(memory.readbyte(obj.base + 0x47), 0x01) > 0 and 1 or -1
			obj.hitbox_ptr = memory.readdword(obj.base + 0xB2)
			obj.num_boxes = (bit.band(obj.hitbox_ptr, 0xFFFFFF) > 0 and memory.readword(obj.hitbox_ptr)) or 0
			obj.scale = memory.readbyte(obj.base + 0x4B) + 1
			obj.char_id = memory.readword(obj.base + 0x30)
		end,
		no_hit = function(obj)
			return bit.btst(memory.readbyte(obj.base + 0x36), 6) == 0
		end,
		invulnerable = function(obj)
			local vuln = memory.readbyte(obj.base + 0xFD)
			if memory.readbyte(obj.base + 0xB6) == 0xFF then --impact freeze
				return false
			elseif vuln > 0 then
				memory.writebyte(obj.base + 0xFD, vuln-1)
				return false
			end
			return true
		end,
		throw = function(obj)
			local range = memory.readword(obj.base + 0xFE)
			if range > 0 then
				memory.writeword(obj.base + 0xFE, 0)
				local range_ptr = memory.readbyte(obj.base + 0xD2) == 0 and 0x04F9AE or 0x04FA2E
				range = memory.readword(range_ptr + range + 0x02)
				local box = {
					type   = "axis throw",
					left   = obj.pos_x,
					right  = obj.pos_x + range * obj.facing_dir,
					top    = obj.pos_y - globals.throwbox_height,
					bottom = obj.pos_y,
				}
				return box
			end
		end,
		breakpoints = {
			{
			{base = 0x003ABE, cmd = "maincpu.pb@(a4+fd)=2"}, --vulnerability
			{base = 0x00755E, cmd = "maincpu.pw@(a4+fe)=d2"}, --throws
			},
		},
	},
	{
		games = {"fatfury2"},
		no_combos = true,
		match_active   = 0x100B89,
		stage_base     = 0x100B00,
		player_base    = 0x100300,
		obj_ptr_offset = 0xBA,
		box_types = {
			p,v,v,g,g,a,a,a,a,g,x,x,x,x,x,a,
			v
		},
		box = {
			top = 0x2, bottom = 0x3, left = 0x4, right = 0x5, space = 0x6, scale = 4, header = 0, 
			read = memory.readbytesigned,
			get_id = function(address)
				return bit.band(memory.readword(address), 0x000F)
			end
		},
		update_object = function(obj)
			obj.facing_dir = bit.rol(memory.readword(obj.base + 0x7A), 16+1)
			obj.facing_dir = bit.bxor(obj.facing_dir, memory.readbyte(obj.base + 0x67))
			obj.facing_dir = bit.band(obj.facing_dir, 1) > 0 and 1 or -1
			local hitbox_ptr_offset = memory.readword(obj.base + 0x88)
			obj.hitbox_ptr = memory.readdword(obj.base + 0x8A) + bit.band(hitbox_ptr_offset, 0x0FFF) * 6
			obj.num_boxes = bit.rshift(bit.band(hitbox_ptr_offset, 0xF000), 12)
			obj.scale = memory.readbyte(obj.base + 0x63) + 1
			obj.char_id = memory.readword(obj.base + 0x5C)
		end,
		no_hit = function(obj)
			return bit.btst(memory.readbyte(obj.base + 0x7A), 5) == 0
		end,
		invulnerable = function(obj)
			if memory.readbyte(obj.base - 0x111) > 0 then
				memory.writebyte(obj.base - 0x111, 0)
				return false
			end
			return true
		end,
		throw = function(obj)
			obj.opp_base = memory.readdword(obj.base + 0xBE)
			obj.opp_id = memory.readword(obj.opp_base + 0x5C)
			local throw = memory.readdword(obj.base - 0x108)
			for _, throw_type in ipairs({
				{ptr = 0x026320, level = 0}, --HP throw
				{ptr = 0x0274DC, level = 0}, --HK throw
				{ptr = 0x01F368, level = 0}, --kim DM
				{ptr = 0x01D154, level = 0}, --jubei _B,F+LK
				{ptr = 0x01D416, level = 0}, --jubei _B,F+HK
				{ptr = 0x01D6E0, level = 2}, --jubei _D,U+LP
				{ptr = 0x01DA7A, level = 2}, --jubei _D,U+HP
				{ptr = 0x01DC10, level = 2}, --jubei DM
				{ptr = 0x0263BC}, --jubei/laurence airthrow
			}) do
				if throw == throw_type.ptr then
					memory.writedword(obj.base - 0x108, 0)
					local box = {type = "axis throw"}
					if not throw_type.level then --airthrow
						if memory.readword(obj.base + offset.y_position) == 0 then --not jumping
							return
						end
						local range_ptr = memory.readdword(memory.readdword(obj.base - 0x104)) + (obj.opp_id - 1) * 0x18
						box.top    = obj.pos_y - memory.readwordsigned(range_ptr + 0)
						box.bottom = obj.pos_y - memory.readwordsigned(range_ptr + 2)
						box.left   = obj.pos_x - memory.readwordsigned(range_ptr + 4) * obj.facing_dir
						box.right  = obj.pos_x -- - memory.readwordsigned(range_ptr + 6) * obj.facing_dir
					else
						local range = memory.readdword(memory.readdword(obj.base - 0x104))
						range = memory.readwordsigned(range + (obj.opp_id - 1) * 0xA + throw_type.level)
						box.left   = obj.pos_x
						box.right  = obj.pos_x + range * obj.facing_dir
						box.top    = obj.pos_y - globals.throwbox_height
						box.bottom = obj.pos_y
					end
					return box
				end
			end
		end,
		breakpoints = {
			{
			{base = 0x028904, cmd = "maincpu.pb@(a4-111)=1"}, --vulnerability
			{base = 0x013E14, cmd = "maincpu.pd@(a4-108)=maincpu.pd@(a7+8); maincpu.pd@(a4-104)=a1", 
				cond = "(maincpu.pd@(a7+10) band FF00) == 0 or maincpu.pd@(a7+10) == 1184"}, --ground throws (ugly but better than 9 BPs)
			{base = 0x0263BC, cmd = "maincpu.pd@(a4-108)=pc; maincpu.pd@(a4-104)=a1"}, --air throws
			},
		},
	},
	{
		games = {"fatfursp"},
		match_active   = 0x100A62,
		player_base    = 0x100400,
		stage_base     = 0x100B00,
		obj_ptr_offset = 0xAC,
		box_types = {
			p,v,v,g,g,a,a,a,a,g,x,x,x,x,x,a,
			v
		},
		update_object = function(obj)
			obj.facing_dir = memory.readbytesigned(obj.base + 0x62) < 0 and -1 or 1
			obj.hitbox_ptr = memory.readdword(obj.base + 0x8A)
			obj.num_boxes  = bit.rshift(obj.hitbox_ptr, 24)
			obj.scale = memory.readbyte(obj.base + 0x65) + 1
			obj.char_id = memory.readword(obj.base + 0x5C)
		end,
		no_hit = function(obj)
			return bit.btst(memory.readbyte(obj.base + 0x7C), 5) == 0
		end,
		invulnerable = function(obj)
			if memory.readbyte(obj.base + 0xB4) > 0x07 then
				return true
			end
			local vuln_1 = {base = 0x029088, ["fatfursa"] = 0}
			local vuln_2 = {base = 0x0248CC, ["fatfursa"] = 0}
			vuln_1, vuln_2 = vuln_1.base + (vuln_1[emu.romname()] or 0), vuln_2.base + (vuln_2[emu.romname()] or 0)
			local ptr = obj.ptr
			repeat
				local inst = memory.readword(ptr)
				if inst == 0x4EB9 and memory.readdword(ptr + 2) == vuln_1 then --jsr $29088.l
					return false
				elseif inst == 0x4EF9 and memory.readdword(ptr + 2) == vuln_2 then --jmp $248cc.l
					return false --opposite planes
				end
				ptr = ptr + 2
			until inst == 0x4E75 --rts
			return true --returned without checking vuln
		end,
		throw = function(obj)
			obj.opp_base = memory.readdword(obj.base + 0xA8)
			obj.opp_id = memory.readword(obj.opp_base + 0x5C)
			obj.side = memory.readbytesigned(obj.base + 0x62) < 0 and 1 or -1
			local ptr = memory.readbyte(obj.base - 0x101)
			if ptr > 0 then --normal throws
				memory.writebyte(obj.base - 0x101, 0)
				ptr = memory.readdword(0x027268 + obj.char_id * 4) + bit.lshift(ptr, 3) + memory.readbyte(obj.base + 0x92)
				ptr = memory.readdword(0x0271DC + memory.readbyte(ptr) * 4)
				for _, throw_type in ipairs({
					0x02C066, --F/B+HP
					0x02C0BA, --B+HK jubei
					0x02C10E, --DF+HP jubei
					0x02C162, --DF+HP bear
					0x02D71A, --F/B+HK
					0x02D760, --F+HK jubei
					0x02D7A6, --B+HK bear
				}) do
					if ptr == throw_type then
						local range = memory.readdword(0x064892 + obj.char_id * 4)
						range = memory.readwordsigned(range + (obj.opp_id - 1) * 0xA)
						return {
							type   = "axis throw",
							left   = obj.pos_x,
							right  = obj.pos_x + range * obj.facing_dir,
							top    = obj.pos_y - (obj.height or globals.throwbox_height),
							bottom = obj.pos_y,
						}
					end
				end
			end
			ptr = memory.readbyte(obj.base - 0x102)
			if ptr > 0 then --special throws
				local range = memory.readdword(0x064892 + obj.char_id * 4)
				range = memory.readwordsigned(range + (obj.opp_id - 1) * 0xA + (ptr - 1) * 2)
				local height = memory.readword(obj.base - 0x104)
				memory.writedword(obj.base - 0x104, 0)
				return {
					type   = "axis throw",
					left   = obj.pos_x,
					right  = obj.pos_x + range * obj.facing_dir,
					top    = height > 0 and emu.screenheight() + globals.top_screen_edge - obj.pos_z - height or 
						obj.pos_y - (obj.height or globals.throwbox_height),
					bottom = height > 0 and emu.screenheight() + globals.top_screen_edge - obj.pos_z or obj.pos_y,
				}
			end
			ptr = memory.readword(obj.base - 0x116)
			if ptr > 0 then --air throws
				memory.writeword(obj.base - 0x116, 0)
				ptr = memory.readdword(0x027F70 + obj.char_id * 4) + ptr
				ptr = memory.readbyte(ptr) * 4 + memory.readdword(obj.base - 0x114)
				ptr = memory.readdword(ptr)
				for _, throw_type in ipairs({
					0x02C210, --jubei
					0x02C354, --mai
					0x02C4C4, --duck
					0x02C624, --laurence
				}) do
					if ptr == throw_type then
						ptr = memory.readdword(0x06491A + obj.char_id * 4)
						ptr = ptr + obj.opp_id * 0x18
						return {
							type   = "axis throw",
							top    = obj.pos_y - memory.readwordsigned(ptr - 6),
							bottom = obj.pos_y - memory.readwordsigned(ptr - 8),
							left   = obj.pos_x - memory.readwordsigned(ptr - 2) * obj.side,
							right  = obj.pos_x -- - memory.readwordsigned(ptr - 4) * obj.side,
						}
					end
				end
			end
		end,
		breakpoints = {
			{
			{base = 0x0249F4, cmd = "pc=pc+4", ["fatfursa"] = 0}, --allow throwboxes at long range
			{base = 0x0249BA, cmd = "maincpu.pb@(a4-101)=d0", ["fatfursa"] = 0}, --ground throw param
			{base = 0x024210, cmd = "maincpu.pb@(a4-102)=d0+1", ["fatfursa"] = 0}, --special throw param
			{base = 0x025AD0, cmd = "maincpu.pw@(a4-116)=d0; maincpu.pd@(a4-114)=a1", ["fatfursa"] = 0}, --airthrow params
			}, {
			{base = 0x030BF0, cmd = "maincpu.pb@(a4-102)=maincpu.pw@(pc+2)", ["fatfursa"] = 0}, --jubei _B,F+LK
			{base = 0x030C98, cmd = "maincpu.pb@(a4-102)=maincpu.pw@(pc+2)", ["fatfursa"] = 0}, --jubei _B,F+HK
			{base = 0x036BE6, cmd = "maincpu.pb@(a4-102)=maincpu.pw@(pc+2)+1; maincpu.pw@(a4-104)=maincpu.pw@(pc+c)", 
				["fatfursa"] = 0}, --ryo DM
			},
		},
	},
	{
		games = {"fatfury3"},
		match_active = 0x100B24,
		player_base  = 0x100400,
		stage_base   = 0x100B00,
		obj_ptr_list = 0x10088A,
		box_types = {
			v,v,v,v,g,g,v,a,a,a,a,a,a,a,a,a,
			a,a,t,g,g
		},
		push = {box_data = 0x0648EE, unpushable = 0xEA, box_offset = function(obj)
			if memory.readdword(obj.base + 0x28) > 0 then --off ground
				if bit.band(memory.readdword(obj.base + 0xC6), 0xFFC000) > 0 then
					return 0x1C0 --air special moves
				else
					return 0x0E0 --jumping or juggled
				end
			else --on ground
				local be = memory.readdword(obj.base + 0xBE)
				if bit.band(be, 0x1C000044) > 0 then
					return 0x070 --crouching or knocked down
				elseif bit.band(be, 0xE000) > 0 then
					return 0x150 --???
				else
					return 0x000 --standing
				end
			end
		end},
		no_hit = function(obj)
			return bit.btst(memory.readbyte(obj.base + 0x6A), 4) == 0 or memory.readbyte(obj.base + 0xA8) > 0
		end,
		invulnerable = function(obj)
			if memory.readbyte(obj.base + 0xAF) > 0 then
				return true
			end
			local ptr = obj.ptr
			repeat
				local inst = memory.readword(ptr)
				if inst == 0x197C and memory.readdword(ptr + 2) == 0x000100B7 then --move.b #$1, ($b7,A4)
					return false
				end
				ptr = ptr + 2
			until inst == 0x4E75 --rts
			return true --returned without checking vuln
		end,
		throw = function(obj)
			for _, throw_type in ipairs({
				{ptr = 0x047460, active = 4, left =-0x20, right = 0x20, top = 0x00, bottom = 0x20}, --andy spider throw
				{ptr = 0x052B04, active = 3, left = 0x00, right = 0x40, top = 0x20, bottom = 0x40}, --mary spider throw
				{ptr = 0x053B2E, active = 3, left = 0x00, right = 0x30, top = 0x18, bottom = 0x48}, --mary DM
				{ptr = 0x0540E6, active = 3, left = 0x00, right = 0x30, top = 0x18, bottom = 0x48}, --mary SDM
			}) do
				if obj.ptr == throw_type.ptr and bit.btst(memory.readbyte(obj.base + 0x6A), throw_type.active) > 0 then
					return {
						type = "axis throw",
						left   = obj.pos_x + throw_type.left  * obj.facing_dir,
						right  = obj.pos_x + throw_type.right * obj.facing_dir,
						top    = obj.pos_y + throw_type.top,
						bottom = obj.pos_y + throw_type.bottom,
					}
				end
			end
			for _, throw_type in ipairs({
				{char = 0x9, special = 0x7, range = 0x38}, --mary 360; see 045558
				{char = 0x6, special = 0x5, range = 0x4A}, --sokaku 360; see 044ED0
			}) do
				if obj.char_id == throw_type.char and memory.readbyte(obj.base - 0x109) == throw_type.special then
					memory.writebyte(obj.base - 0x109, 0)
					return {
						type = "axis throw",
						left   = obj.pos_x,
						right  = obj.pos_x + throw_type.range * obj.facing_dir,
						top    = obj.pos_y - (obj.height or globals.throwbox_height),
						bottom = obj.pos_y,
					}
				end
			end
			if memory.readdword(obj.base - 0x108) > 0 then --air throws
				local box = {type = "axis throw"}
				box.address = obj.base - 0x108
				box.left   = obj.pos_x + memory.readwordsigned(box.address + 6) * obj.facing_dir
				box.right  = obj.pos_x + memory.readwordsigned(box.address + 4) * obj.facing_dir
				box.top    = obj.pos_y - memory.readwordsigned(box.address + 2)
				box.bottom = obj.pos_y - memory.readwordsigned(box.address + 0)
				memory.writedword(box.address + 0, 0)
				memory.writedword(box.address + 4, 0)
				return box
			end
		end,
		breakpoints = {
			{
			{base = 0x0447C6, cmd = "maincpu.pb@(a4-109)=d1"}, --special move ID
			{base = 0x0681EC, cmd = "maincpu.pq@(a4-108)=maincpu.pq@(a0-8)"}, --airthrow data
			},
		},
	},
	{
		games = {"rbff1"},
		match_active = 0x106D75,
		player_base  = 0x100400,
		stage_base   = 0x100B00,
		obj_ptr_list = 0x100890,
		box_types = {
			v,v,v,v,g,g,v,a,a,a,a,a,a,a,a,a,
			a,a,a,g,g
		},
		push = {box_data = 0x06C244, ["rbff1a"] = 0x1A, unpushable = 0xEA, box_offset = function(obj)
			if memory.readdword(obj.base + 0x28) > 0 then --off ground
				if bit.band(memory.readdword(obj.base + 0xC6), 0xFFFF0000) == 0 then
					return 0x110 --air special moves
				else
					return 0x220 --jumping or juggled
				end
			else --on ground
				if bit.band(memory.readdword(obj.base + 0xBE), 0x14000044) > 0 then
					return 0x088 --crouching or knocked down
				else
					return 0x000 --standing
				end
			end
		end},
		no_hit = function(obj)
			return bit.btst(memory.readbyte(obj.base + 0x6A), 4) == 0 or memory.readbyte(obj.base + 0xA8) > 0
		end,
		invulnerable = function(obj)
			if memory.readbyte(obj.base + 0xAF) > 0 then
				return true
			end
			local vuln = {base = 0x0285B8, ["rbff1a"] = 0x1A}
			vuln = vuln.base + (vuln[emu.romname()] or 0)
			local ptr = obj.ptr
			repeat
				local inst = memory.readword(ptr)
				if inst == 0x4EB9 and memory.readdword(ptr + 2) == vuln then --jsr $285b8.l
					return false
				end
				ptr = ptr + 2
			until inst == 0x4E75 --rts
			return true --returned without checking vuln
		end,
		throw = function(obj)
			obj.opp_base = memory.readdword(obj.base + 0x94)
			obj.opp_id = memory.readword(obj.opp_base + 0x10)
			obj.side = memory.readbytesigned(obj.base + 0x58) < 0 and -1 or 1
			for _, throw_type in ipairs({
				{ptr = 0x043B2C, ["rbff1a"] = 0x1A, active = 4, left =-0x20, right = 0x20, top = 0x00, bottom = 0x20}, --andy spider throw
				{ptr = 0x04EBF0, ["rbff1a"] = 0x1A, active = 3, left = 0x00, right = 0x40, top = 0x20, bottom = 0x40}, --mary spider throw
			}) do
				if obj.ptr == throw_type.ptr + (throw_type[emu.romname()] or 0) and 
					bit.btst(memory.readbyte(obj.base + 0x6A), throw_type.active) > 0 then
					return {
						type = "axis throw",
						left   = obj.pos_x + throw_type.left  * obj.side,
						right  = obj.pos_x + throw_type.right * obj.side,
						top    = obj.pos_y + throw_type.top,
						bottom = obj.pos_y + throw_type.bottom,
					}
				end
			end
			if memory.readdword(obj.base - 0x108) > 0 then --air throws
				local box = {type = "axis throw"}
				box.address = obj.base - 0x108
				box.left   = obj.pos_x + memory.readwordsigned(box.address + 6) * obj.facing_dir
				box.right  = obj.pos_x + memory.readwordsigned(box.address + 4) * obj.facing_dir
				box.top    = obj.pos_y - memory.readwordsigned(box.address + 2)
				box.bottom = obj.pos_y - memory.readwordsigned(box.address + 0)
				memory.writedword(box.address + 0, 0)
				memory.writedword(box.address + 4, 0)
				return box
			elseif memory.readword(obj.base - 0x102) > 0 then --ground throws
				local box = {type = "axis throw"}
				box.address = {base = 0x06CBB0, ["rbff1a"] = 0x1A}
				box.address = memory.readdword(box.address.base + (box.address[emu.romname()] or 0) + obj.char_id * 4)
				local range = (obj.opp_id - 1) * 0xC
				range = range + bit.band(memory.readword(obj.base - 0x102) - 0x60, 0x07) * 2
				range = memory.readwordsigned(box.address + range)
				memory.writeword(obj.base - 0x102, 0)
				box.left   = obj.pos_x
				box.right  = obj.pos_x + range * obj.side
				box.top    = obj.pos_y - (obj.height or globals.throwbox_height)
				box.bottom = obj.pos_y
				return box
			elseif memory.readword(obj.base - 0x104) > 0 then --special throws
				local range = memory.readword(obj.base - 0x104)
				memory.writeword(obj.base - 0x104, 0)
				return {
					type = "axis throw",
					left   = obj.pos_x,
					right  = obj.pos_x + range * obj.side,
					top    = obj.pos_y - (obj.height or globals.throwbox_height),
					bottom = obj.pos_y,
				}
			end
		end,
		breakpoints = {
			{
			{base = 0x06CAFA, cmd = "maincpu.pw@(a4-102)=d7", ["rbff1a"] = 0x1A}, --ground throw range level
			{base = 0x040ED2, cmd = "maincpu.pw@(a4-104)=d7", ["rbff1a"] = 0x1A}, --sokaku 360; duck dm/sdm; yamazaki sdm
			{base = 0x0500EC, cmd = "maincpu.pw@(a4-104)=maincpu.pw@(pc+64)", ["rbff1a"] = 0x1A}, --mary 360
			}, {
			{base = 0x050C0C, cmd = "maincpu.pw@(a4-104)=maincpu.pw@(pc+34)", ["rbff1a"] = 0x1A}, --mary dm
			{base = 0x051136, cmd = "maincpu.pw@(a4-104)=maincpu.pw@(pc+3c)", ["rbff1a"] = 0x1A}, --mary sdm
			{base = 0x0709AE, cmd = "maincpu.pq@(a4-108)=maincpu.pq@(a0-8)", ["rbff1a"] = 0x1A}, --airthrow data
			},
		},
	},
	{
		games = {"rbffspec"},
		match_active = 0x1096FA,
		player_base  = 0x100400,
		stage_base   = 0x100E00,
		obj_ptr_list = 0x100C92,
		box_types = {
			v,v,v,v,v,v,x,x,x,x,x,x,x,x,x,x,
			g,g,g,g,g,g,g,g,g,g,g,g,g,g,g,g,
		},
		push = {box_data = 0x072F7A, ["rbffspeck"] = -0x11C, unpushable = 0xEC, box_offset = function(obj)
			if memory.readdword(obj.base + 0x28) > 0 then --off ground
				local d2 = memory.readdword(obj.base + 0xC8)
				if (obj.char_id == 7 and bit.band(d2, 0x2000000) > 0 or bit.band(d2, 0xFFFF0000) == 0) or bit.band(d2, 0xFFFF0000) == 0 then
					return 0x150 --air special moves
				else
					return 0x1F8 --jumping or juggled
				end
			else --on ground
				if bit.band(memory.readdword(obj.base + 0xC0), 0x14000046) > 0 then
					return 0x0A8 --crouching or knocked down
				else
					return 0x000 --standing
				end
			end
		end},
		bank = {
			get = function()
				return memory.readword(0x2FFFE0)
			end,
			set = function(obj)
				local ptr = {base = 0x00CA3E, ["rbffspeck"] = -0x5E}
				ptr = ptr.base + (ptr[emu.romname()] or 0)
				memory.writeword(0x2FFFF0, memory.readword(ptr + obj.char_id * 0x12))
			end,
			restore = function(val)
				memory.writeword(0x2FFFF0, val)
			end,
		},
		no_hit = function(obj)
			return bit.btst(memory.readbyte(obj.base + 0x6A), 3) == 0 or memory.readbyte(obj.base + 0xAA) > 0 or
				(obj.projectile and memory.readbyte(obj.base + 0xE7) > 0) or 
				(not obj.projectile and memory.readbyte(obj.base + 0xB6) == 0)
		end,
		invulnerable = function(obj)
			if memory.readbyte(obj.base + 0xB1) > 0 then
				return true
			end
			local vuln_1 = {base = 0x02A096, ["rbffspeck"] = -0x128}
			local vuln_2 = {base = 0x045D88, ["rbffspeck"] = -0x11C}
			vuln_1, vuln_2 = vuln_1.base + (vuln_1[emu.romname()] or 0), vuln_2.base + (vuln_2[emu.romname()] or 0)
			local ptr = obj.ptr
			repeat
				local inst = memory.readword(ptr)
				if inst == 0x4EB9 and memory.readdword(ptr + 2) == vuln_1 then --jsr $2a096.l
					return false
				elseif inst == 0x4EB9 and memory.readdword(ptr + 2) == vuln_1 + 0x12
					and memory.readbytesigned(obj.base + 0xF6) <= 0 then --jsr $2a0a8.l
					return false
				elseif inst == 0x4EF9 and memory.readdword(ptr + 2) == vuln_2 then --jmp $45d88.l
					return false --yamazaki snake
				end
				ptr = ptr + 2
			until inst == 0x4E75 --rts
			return true --returned without checking vuln
		end,
		throw = function(obj)
			obj.opp_base = memory.readdword(obj.base + 0x96)
			obj.opp_id = memory.readword(obj.opp_base + 0x10)
			obj.side = memory.readbytesigned(obj.base + 0x58) < 0 and -1 or 1
			if memory.readdword(obj.base - 0x114) > 0 then --air throws
				local box = {type = "axis throw"}
				box.address = obj.base - 0x114
				box.left   = obj.pos_x
				box.right  = obj.pos_x + memory.readwordsigned(box.address) * obj.facing_dir
				box.top    = obj.pos_y - memory.readwordsigned(box.address + 2)
				box.bottom = obj.pos_y + memory.readwordsigned(box.address + 2)
				memory.writedword(box.address, 0)
				return box
			end
			local ptr = memory.readdword(obj.base - 0x108)
			local d6 = memory.readword(obj.base - 0x104)
			local d7 = memory.readword(obj.base - 0x102)
			if ptr == 0 then
				return
			end
			memory.writedword(obj.base - 0x108, 0)
			memory.writedword(obj.base - 0x104, 0)
			local clone = {["rbffspec"] = 0, ["rbffspeck"] = -0x11C}
			clone = clone[emu.romname()]
			local box = {type = "axis throw"}
			if ptr == 0x073DEE + clone then --normal ground throw
				local range = (memory.readbyte(obj.base + 0x58) == memory.readbyte(obj.opp_base + 0x58) and 0x4) or 0x3
				range = math.abs(memory.readbytesigned(0x072F7A + clone + bit.lshift(obj.opp_id, 3) + range) * 4)
				range = range + memory.readbytesigned(0x072F7A + clone + bit.lshift(obj.char_id, 3) + 0x3) * -4
				d7 = (d7 == 0x65 and 0x3) or bit.band(d7 - 0x60, 0x7)
				range = range + memory.readbytesigned(0x073EC8 + clone + obj.char_id * 4 + d7)
				box.left   = obj.pos_x
				box.right  = obj.pos_x + range * obj.side
				box.top    = obj.pos_y - (obj.height or globals.throwbox_height)
				box.bottom = obj.pos_y
			elseif memory.readwordsigned(obj.base + offset.y_position) > 0 then --special aerial throw
				box.left   = obj.pos_x
				box.right  = obj.pos_x + d7 * obj.side
				if d6 > 0 then --EX andy spider; EX mary spider
					box.top    = obj.pos_y
					box.bottom = obj.pos_y + d6
				else --mai SDM
					box.top    = obj.pos_y + 0x20
					box.bottom = obj.pos_y + 0x40
				end
			else --special ground throw
				box.left   = obj.pos_x
				box.right  = obj.pos_x + d7 * obj.side
				box.top    = obj.pos_y - (obj.height or globals.throwbox_height)
				box.bottom = obj.pos_y
			end
			return box
		end,
		breakpoints = {
			{
			{base = 0x0456D2, cmd = "maincpu.pd@(a4-108)=maincpu.pd@a7; maincpu.pw@(a4-104)=d6; maincpu.pw@(a4-102)=d7", 
				cond = "d7>0", ["rbffspeck"] = -0x11C}, --ground throw params
			{base = 0x054F86, cmd = "maincpu.pd@(a4-108)=pc; maincpu.pw@(a4-102)=maincpu.pw@(pc+12)", 
				["rbffspeck"] = -0x11C}, --ex mary dm
			}, {
			{base = 0x076CFE, cmd = "maincpu.pd@(a4-114)=maincpu.pd@a0", ["rbffspeck"] = -0x11C}, --air throw ranges
			{base = 0x0544C6, cmd = "maincpu.pw@(a4-114)=maincpu.pw@(pc+12); maincpu.pw@(a4-112)=maincpu.pw@(pc+28)", 
				["rbffspeck"] = -0x11C}, --ex mary snatcher
			},
		},
	},
	{
		games = {"rbff2"},
		match_active = 0x10B1A4,
		player_base  = 0x100400,
		stage_base   = 0x100E00,
		obj_ptr_list = 0x100C92,
		box_types = {
			v,v,v,v,v,v,x,x,x,x,x,x,x,x,x,x,
			g,g,g,g,g,g,g,g,g,g,g,g,g,g,g,g,
		},
		push = {box_data = 0x05C99C, ["rbff2k"] = -0x104, ["rbff2h"] = 0x20, unpushable = 0xEC, box_offset = function(obj)
			local d1, d2 = memory.readdword(obj.base + 0xC0), memory.readdword(obj.base + 0xC8)
			if obj.char_id == 5 and bit.band(d2, bit.lshift(1, 0xF)) > 0 then
				return 0x000 --standing?
			elseif bit.band(d1, bit.lshift(1, 0x1)) > 0 then
				return 0x0C0 --crouching or knocked down?
			elseif memory.readdword(obj.base + 0x28) > 0 then --off ground
				local char_table_lookup = memory.readdword(globals.pushbox_base - 0x294 + obj.char_id * 4)
				if memory.readdword(char_table_lookup) == 0 and bit.band(d2, 0xFFFF0000) > 0 then
					return 0x240 --air special moves
				else
					return 0x180 --jumping or juggled
				end
			else --on ground
				if bit.band(d1, 0x14000046) > 0 then
					return 0x0C0 --crouching or knocked down
				else
					return 0x000 --standing
				end
			end
		end},
		bank = {
			get = function()
				return memory.readword(0x2FFFE0)
			end,
			set = function(obj)
				memory.writeword(0x2FFFF0, bit.rshift(bit.rol(memory.readbyte(obj.base + 0x67), 2), 8))
			end,
			restore = function(val)
				memory.writeword(0x2FFFF0, val)
			end,
		},
		no_hit = function(obj)
			return bit.btst(memory.readbyte(obj.base + 0x6A), 3) == 0 or memory.readbyte(obj.base + 0xAA) > 0 or
				(obj.projectile and memory.readbyte(obj.base + 0xE7) > 0) or 
				(not obj.projectile and memory.readbyte(obj.base + 0xB6) == 0)
		end,
		invulnerable = function(obj)
			if memory.readbyte(obj.base + 0xB1) > 0 then
				return true
			end
			local vuln_1 = {base = 0x02327A, ["rbff2k"] = 0x28, ["rbff2h"] = 0}
			local vuln_2 = {base = 0x0396F4, ["rbff2k"] = 0xC, ["rbff2h"] = 0x20}
			vuln_1, vuln_2 = vuln_1.base + (vuln_1[emu.romname()] or 0), vuln_2.base + (vuln_2[emu.romname()] or 0)
			local ptr = obj.ptr
			repeat
				local inst = memory.readword(ptr)
				if inst == 0x4EB9 and memory.readdword(ptr + 2) == vuln_1 then --jsr $2327a.l
					return false
				elseif inst == 0x4EB9 and memory.readdword(ptr + 2) == vuln_1 + 0x12 
					and memory.readbytesigned(obj.base + 0xF6) <= 0 then --jsr $2328c.l
					return false
				elseif inst == 0x4EF9 and memory.readdword(ptr + 2) == vuln_2 then --jmp $396f4.l
					return false --heavy attacks; back row
				end
				ptr = ptr + 2
			until inst == 0x4E75 --rts
			return true --returned without checking vuln
		end,
		throw = function(obj)
			obj.opp_base = memory.readdword(obj.base + 0x96)
			obj.opp_id = memory.readword(obj.opp_base + 0x10)
			obj.side = memory.readbytesigned(obj.base + 0x58) < 0 and -1 or 1
			local ptr = memory.readdword(obj.base - 0x108)
			local h_range = memory.readwordsigned(obj.base - 0x104)
			local v_range = memory.readwordsigned(obj.base - 0x102)
			memory.writedword(obj.base - 0x108, 0)
			memory.writedword(obj.base - 0x104, 0)
			if v_range > 0 then --air throws
				local box = {type = "axis throw"}
				box.left   = obj.pos_x
				box.right  = obj.pos_x + h_range * obj.facing_dir
				box.top    = obj.pos_y - v_range
				box.bottom = obj.pos_y + v_range
				return box
			elseif ptr > 0 then --normal ground throw
				local box = {type = "axis throw"}
				local range = (memory.readbyte(obj.base + 0x58) == memory.readbyte(obj.opp_base + 0x58) and 0x4) or 0x3
				range = math.abs(memory.readbytesigned(memory.readdword(ptr + 0x02) + bit.lshift(obj.opp_id, 3) + range) * 4)
				range = range + memory.readbytesigned(memory.readdword(ptr + 0x02) + bit.lshift(obj.char_id, 3) + 0x3) * -4
				h_range = (h_range == 0x65 and 0x3) or bit.band(h_range - 0x60, 0x7)
				range = range + memory.readbytesigned(ptr + 0xD2 + obj.char_id * 4 + h_range)
				box.left   = obj.pos_x
				box.right  = obj.pos_x + range * obj.side
				box.top    = obj.pos_y - (obj.height or globals.throwbox_height)
				box.bottom = obj.pos_y
				return box
			end
			ptr = memory.readdword(obj.base - 0x10C)
			if ptr > 0 then
				memory.writedword(obj.base - 0x10C, 0)
				local box = {type = "axis throw"}
				box.left   = obj.pos_x
				box.right  = obj.pos_x + memory.readwordsigned(ptr) * obj.side
				if memory.readdword(ptr + 2) == 0 then --special ground throw
					box.top    = obj.pos_y - (obj.height or globals.throwbox_height)
					box.bottom = obj.pos_y
					return (memory.readword(obj.base + offset.y_position) == 0 and box) or nil
				else --special air throw
					box.top    = obj.pos_y - memory.readwordsigned(ptr + 2)
					box.bottom = obj.pos_y + memory.readwordsigned(ptr + 4)
					return box
				end
			end
		end,
		breakpoints = {
			{
			{base = 0x05D782, cmd = "maincpu.pd@(a4-108)=pc; maincpu.pw@(a4-104)=d7", 
				["rbff2k"] = -0x104, ["rbff2h"] = 0x20}, --ground throws
			{base = 0x060428, cmd = "maincpu.pd@(a4-104)=maincpu.pd@a0", ["rbff2k"] = -0x104, ["rbff2h"] = 0x20}, --air throw ranges
			{base = 0x039F2A, cmd = "maincpu.pd@(a4-10c)=a0", ["rbff2k"] = 0xC, ["rbff2h"] = 0x20}, --special throw range ptr
			},
		},
	},
	{
		games = {"garou"},
		match_active = 0x10748A,
		player_base  = 0x100400,
		stage_base   = 0x100E00,
		obj_ptr_list = 0x100C88,
		box_types = {
			v,v,v,v,v,v,x,x,x,x,x,x,x,x,x,x,
			g,g,g,g,g,g,g,g,g,g,g,g,g,g,g,g,
		},
		push = {box_data = 0x0358B0, ["garouo"] = 0, ["garoup"] = -0x1F2, ["garoubl"] = -0x1F2, unpushable = 0xEE, 
		box_offset = function(obj)
			if bit.band(memory.readdword(obj.base + 0xC2), 0x2) > 0 then
				return 0x100
			end
			if memory.readdword(obj.base + 0x28) > 0 then --off ground
				local c6, ca, bb = memory.readdword(obj.base + 0xC6), memory.readdword(obj.base + 0xCA), memory.readbyte(obj.base + 0xBB)
				if (bit.band(c6, 0xFFFFFFFF) > 0 or bit.band(ca, 0xFFFFFFDE) > 0)
					and bit.band(bb, bit.lshift(1, 0x4)) == 0 and bit.band(ca, 0xFFFFE01E) > 0 then
					return 0x300 --air special moves
				else
					return 0x200 --jumping or juggled
				end
			else --on ground
				if bit.band(memory.readdword(obj.base + 0xC2), 0x14000046) > 0 then
					return 0x100 --crouching or knocked down
				else
					return 0x000 --standing
				end
			end
		end},
		adjust_y = function(y)
			return y + globals.top_screen_edge - 0x4
		end,
		bank = {
			get = function()
				local target = {base = 0x2FFFC0, ["garouo"] = 0, ["garoup"] = 0x30, ["garoubl"] = 0x30}
				target = target.base + (target[emu.romname()] or 0)
				return memory.readword(target)
			end,
			set = function(obj)
				local target = {base = 0x2FFFC0, ["garouo"] = 0, ["garoup"] = 0x30, ["garoubl"] = 0x30}
				local ptr = {base = 0x011A44, ["garouo"] = 0, ["garoup"] = -0x2E6, ["garoubl"] = -0x2E6}
				target, ptr = target.base + (target[emu.romname()] or 0), ptr.base + (ptr[emu.romname()] or 0)
				memory.writeword(target, memory.readword(ptr + obj.char_id * 4))
			end,
			restore = function(val)
				local target = {base = 0x2FFFC0, ["garouo"] = 0, ["garoup"] = 0x30, ["garoubl"] = 0x30}
				target = target.base + (target[emu.romname()] or 0)
				memory.writeword(target, val)
			end,
		},
		no_hit = function(obj)
			return bit.btst(memory.readbyte(obj.base + 0x6A), 3) == 0 or memory.readbyte(obj.base + 0xAC) > 0 or 
				(obj.projectile and memory.readbyte(obj.base + 0xE9) > 0) or 
				(not obj.projectile and memory.readbyte(obj.base + 0xB8) == 0)
		end,
		invulnerable = function(obj)
			if memory.readbyte(obj.base + 0xB3) > 0 then
				return true
			end
			local vuln_1 = {base = 0x020684, ["garouo"] = 0, ["garoup"] = 0x5BE, ["garoubl"] = 0x5BE}
			local vuln_2 = {base = 0x0519EA, ["garouo"] = 0, ["garoup"] = 0xDCC, ["garoubl"] = 0xDCC}
			vuln_1, vuln_2 = vuln_1.base + (vuln_1[emu.romname()] or 0), vuln_2.base + (vuln_2[emu.romname()] or 0)
			local ptr = obj.ptr
			repeat
				local inst = memory.readword(ptr)
				if inst == 0x4EB9 and memory.readdword(ptr + 2) == vuln_1 then --jsr $20684.l
					return false
				elseif inst == 0x4EB9 and memory.readdword(ptr + 2) == vuln_1 + 0x12 
					and memory.readbytesigned(obj.base + 0xFE) <= 0 then --jsr $20696.l
					return false
				elseif inst == 0x4EB9 and memory.readdword(ptr + 2) == vuln_2 then --jsr $519ea.l
					return false --hokutomaru moves
				end
				ptr = ptr + 2
			until inst == 0x4E75 --rts
			return true --returned without checking vuln
		end,
		throw = function(obj)
			obj.side = memory.readbyte(obj.base - 0x102) == 0 and 0x59 or 0x58
			obj.side = memory.readbytesigned(obj.base + obj.side) < 0 and -1 or 1
			local h_range = memory.readbytesigned(obj.base - 0x101)
			if h_range == 0 then
				return
			end
			local t_range, b_range = memory.readbytesigned(obj.base - 0x104), memory.readbytesigned(obj.base - 0x103)
			local box = {type = "axis throw"}
			box.left   = obj.pos_x
			box.right  = obj.pos_x + h_range * obj.side
			box.top    = obj.pos_y - t_range
			box.bottom = obj.pos_y + b_range
			if memory.readbyte(obj.base - 0x102) == 1 then --normal airthrow
				local ptr = {base = 0x03332E, ["garouo"] = 0, ["garoup"] = 0x82, ["garoubl"] = 0x82}
				ptr = ptr.base + (ptr[emu.romname()] or 0)
				box.right  = obj.pos_x + math.min(h_range, memory.readwordsigned(ptr)) * obj.side
				box.top    = obj.pos_y - math.min(t_range, memory.readwordsigned(ptr + 2))
				box.bottom = obj.pos_y + math.min(b_range, memory.readwordsigned(ptr + 2))
			elseif memory.readbyte(obj.base - 0x102) == 3 then --air to ground throw
				box.top    = obj.pos_y
				box.bottom = emu.screenheight() + globals.top_screen_edge - obj.pos_z
			elseif memory.readword(obj.base - 0x104) == 0 then --undefined height; ground throw
				box.top    = obj.pos_y - (obj.height or globals.throwbox_height)
				box.bottom = obj.pos_y
			end
			memory.writedword(obj.base - 0x104, 0)
			return box
		end,
		breakpoints = {
			{{base = 0x02893C, cmd = "maincpu.pd@(a4-104)=maincpu.pd@(a1+4); maincpu.pb@(a4-101)=d2", 
				["garouo"] = 0, ["garoup"] = 0x1EC, ["garoubl"] = 0x1EC},},
		},
	},
}


--------------------------------------------------------------------------------
-- post-process modules

for game in ipairs(profile) do
	local g = profile[game]
	g.number_players = g.number_players or 2
	g.obj_engine = (g.obj_ptr_list and "garou") or (g.obj_ptr_offset and "fatal fury 2") or "fatal fury 1"
	g.box = g.box or {
		top = 0x1, bottom = 0x2, left = 0x3, right = 0x4, space = 0x5, scale = 4, header = 0, read = memory.readbytesigned,
	}
	g.box.get_id = g.box.get_id or function(address)
		return memory.readbyte(address)
	end
	g.adjust_y = g.adjust_y or function(y)
		return y + globals.top_screen_edge
	end
	g.update_object = g.update_object or function(obj)
		obj.facing_dir = memory.readwordsigned(obj.base + 0x6A) < 0 and 1 or 0
		obj.facing_dir = bit.bxor(obj.facing_dir, bit.band(memory.readbyte(obj.base + 0x71), 1))
		obj.facing_dir = obj.facing_dir > 0 and 1 or -1
		obj.hitbox_ptr = memory.readdword(obj.base + 0x7A)
		obj.num_boxes  = bit.rshift(obj.hitbox_ptr, 24)
		obj.scale      = memory.readbyte(obj.base + 0x73) + 1
		obj.char_id    = memory.readword(obj.base + 0x10)
	end
	g.bank = g.bank or {get = function() end, set = function() end, restore = function() end}
end

for _,box in pairs(boxes) do
	box.fill    = bit.lshift(box.color, 8) + (globals.no_alpha and 0x00 or box.fill)
	box.outline = bit.lshift(box.color, 8) + (globals.no_alpha and 0xFF or box.outline)
end
boxes["undefined"] = {}

local game, frame_buffer
emu.update_func = fba and emu.registerafter or emu.registerbefore


--------------------------------------------------------------------------------
-- prepare the hitboxes

local type_check = {
	["push"] = function(obj, box)
		if obj.unpushable then
			return true
		end
	end,

	["vulnerability"] = function(obj, box)
		if obj.invulnerable then
			if not game.no_combos then
				return true
			end
			box.type = "push"
		end
	end,

	["guard"] = function(obj, box)
	end,

	["attack"] = function(obj, box)
		if obj.no_hit then
			return true
		elseif obj.projectile then
			box.type = "proj. attack"
		end
	end,

	["throw"] = function(obj, box)
		if obj.no_hit then
			return true
		end
	end,

	["undefined"] = function(obj, box)
		emu.message(string.format("%x, unk box id: %02x", obj.base, box.id))
	end,
}


local plane_scale = function(obj, box, offset)
	return bit.arshift(game.box.read(box.address + offset) * (obj.scale or 0x100), 8)
end


local define_box = function(obj, box)
	if type_check[box.type](obj, box) then
		return nil
	end

	box.top    = obj.pos_y - plane_scale(obj, box, game.box.top)    * game.box.scale
	box.bottom = obj.pos_y - plane_scale(obj, box, game.box.bottom) * game.box.scale
	box.left   = obj.pos_x - plane_scale(obj, box, game.box.left)   * game.box.scale * obj.facing_dir
	box.right  = obj.pos_x - plane_scale(obj, box, game.box.right)  * game.box.scale * obj.facing_dir
	if box.top == box.bottom and box.left == box.right then
		return nil
	elseif box.type == "push" then
		obj.height = box.bottom - box.top --used for ground throw height
	end

	return box
end


local update_object = function(obj)
	obj.pos_x = memory.readwordsigned(obj.base + offset.x_position) - globals.left_screen_edge
	obj.pos_z = memory.readwordsigned(obj.base + offset.z_position)
	obj.pos_y = emu.screenheight() - memory.readwordsigned(obj.base + offset.y_position) - obj.pos_z
	obj.pos_y = game.adjust_y(obj.pos_y)
	obj.ptr   = memory.readdword(obj.base)
	game.update_object(obj)
	obj.no_hit = game.no_hit(obj)
	obj.invulnerable = not obj.projectile and game.invulnerable(obj)

	if globals.pushbox_base and not obj.projectile then
		obj.unpushable = memory.readbyte(obj.base + game.push.unpushable) > 0
		local box = {address = globals.pushbox_base + game.push.box_offset(obj) + bit.lshift(obj.char_id, 3)}
		box.id = memory.readbyte(box.address)
		box.type = "push"
		table.insert(obj, define_box(obj, box))
	end
	game.bank.set(obj)
	for n = obj.num_boxes, 1, -1 do
		local box = {address = obj.hitbox_ptr + (n-1)*game.box.space + game.box.header}
		box.id = game.box.get_id(box.address)
		box.type = box.id + 1 > #game.box_types and "attack" or game.box_types[box.id + 1] or "undefined"
		--box.type = "undefined" --debug
		table.insert(obj, define_box(obj, box))
	end
	table.insert(obj, not obj.projectile and game.throw(obj) or nil)
	return obj
end


local read_projectiles = {
	["fatal fury 1"] = function(object_list)
		local prev_address = game.player_base + offset.player_space * (game.number_players-1)
		while true do
			local obj = {base = 0x100100 + memory.readword(prev_address + 0x4)}
			obj.projectile = true
			if obj.base == game.player_base or obj.base == 0x100100 then
				return
			end
			prev_address = obj.base
			local hitbox_ptr = bit.band(memory.readdword(obj.base + 0xB2), 0xFFFFFF)
			if hitbox_ptr > 0 and memory.readword(hitbox_ptr + game.box.header) ~= 0x0006 then --back plane obstacle
				table.insert(object_list, update_object(obj))
			end
		end
	end,

	["fatal fury 2"] = function(object_list)
		for p = 1, game.number_players do
			local obj = {base = memory.readdword(game.player_base + offset.player_space * (p-1) + game.obj_ptr_offset)}
			obj.projectile = true
			local ptr = memory.readdword(obj.base)
			while obj.base > 0 do
				local inst = memory.readword(ptr)
				if inst == 0x4E75 then --rts
					break
				elseif inst == 0x4EB9 or inst == 0x6100 then --jsr or bsr to some routine
					table.insert(object_list, update_object(obj))
					break
				end
				ptr = ptr + 2
			end
		end
	end,

	["garou"] = function(object_list)
		local offset = 0
		while true do
			local obj = {base = memory.readdword(game.obj_ptr_list + offset)}
			obj.projectile = true
			if obj.base == 0 or memory.readword(memory.readdword(obj.base)) == 0x4E75 then --rts instruction
				return
			end
			for _, old_obj in ipairs(object_list) do
				if obj.base == old_obj.base then
					return
				end
			end
			table.insert(object_list, update_object(obj))
			offset = offset + 4
		end
	end,
}


local function match_active(address)
	local ram_value = memory.readword(address)
	for _, test_value in ipairs({0x5555, 0xAAAA, bit.band(0xFFFF, address)}) do
		if ram_value == test_value then
			return false
		end
	end
	if memory.readbyte(game.match_active) > 0 then
		return true
	end
	return false
end


local function update_hitboxes()
	frame_buffer = {}

	globals.match_active = game and match_active(game.player_base) or false
	if not game or not globals.match_active then
		return
	end

	globals.left_screen_edge = memory.readwordsigned(game.stage_base + offset.x_position) + globals.margin
	globals.top_screen_edge  = memory.readwordsigned(game.stage_base + offset.y_position)

	globals.bank = game.bank.get()
	for p = 1, game.number_players do
		local player = {base = game.player_base + offset.player_space * (p-1)}
		table.insert(frame_buffer, update_object(player))
	end
	read_projectiles[game.obj_engine](frame_buffer)
	game.bank.restore(globals.bank)

	globals.same_plane = game.number_players > 2 or frame_buffer[1].pos_z == frame_buffer[2].pos_z
end


emu.update_func( function()
	globals.register_count = (globals.register_count or 0) + 1
	globals.last_frame = globals.last_frame or emu.framecount()
	if globals.register_count == 1 then
		update_hitboxes()
	end
	if globals.last_frame < emu.framecount() then
		globals.register_count = 0
	end
	globals.last_frame = emu.framecount()
end)


--------------------------------------------------------------------------------
-- draw the hitboxes

local function draw_hitbox(hb)
	if not hb or (hb.type == "push" and (not globals.draw_pushboxes or not globals.same_plane)) then
		return
	end

	if globals.draw_mini_axis then
		hb.hval = (hb.right + hb.left)/2
		hb.vval = (hb.bottom + hb.top)/2
		gui.drawline(hb.hval, hb.vval-globals.mini_axis_size, hb.hval, hb.vval+globals.mini_axis_size, boxes[hb.type].outline)
		gui.drawline(hb.hval-globals.mini_axis_size, hb.vval, hb.hval+globals.mini_axis_size, hb.vval, boxes[hb.type].outline)
		--gui.text(hb.hval, hb.vval, string.format("%02X", hb.id or 0xFF)) --debug
	end

	gui.box(hb.left, hb.top, hb.right, hb.bottom, boxes[hb.type].fill, boxes[hb.type].outline)
end


local function draw_axis(obj)
	gui.drawline(obj.pos_x, obj.pos_y-globals.axis_size, obj.pos_x, obj.pos_y+globals.axis_size, globals.axis_color)
	gui.drawline(obj.pos_x-globals.axis_size, obj.pos_y, obj.pos_x+globals.axis_size, obj.pos_y, globals.axis_color)
	--gui.text(obj.pos_x, obj.pos_y -0x10, string.format("%06X", obj.base)) --debug
	--gui.text(obj.pos_x, obj.pos_y -0x08, string.format("%06X", obj.ptr)) --debug
	--gui.text(obj.pos_x, obj.pos_y -0x00, string.format("%08X", obj.hitbox_ptr)) --debug
end


local get_number_entries = function()
	local max_entries = 0
	for _, obj in ipairs(frame_buffer) do
		max_entries = math.max(max_entries, #obj)
	end
	return max_entries
end


local function render_hitboxes()
	gui.clearuncommitted()
	if not game or not globals.match_active then
		return
	end

	if globals.blank_screen then
		gui.box(0, 0, emu.screenwidth(), emu.screenheight(), globals.blank_color)
	end

	for entry = 1, get_number_entries() do
		for _, obj in ipairs(frame_buffer) do
			draw_hitbox(obj[entry])
		end
	end

	if globals.draw_axis then
		for _, obj in ipairs(frame_buffer) do
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


--------------------------------------------------------------------------------
-- initialize on game startup

local function whatgame()
	print()
	game = nil
	for _, module in ipairs(profile) do
		for _, shortname in ipairs(module.games) do
			if emu.romname() == shortname or emu.parentname() == shortname then
				print("drawing " .. emu.romname() .. " hitboxes")
				game = module
				frame_buffer = {}
				globals.game = shortname
				globals.margin = (320 - emu.screenwidth()) / 2 --fba removes the side margins for some games
				globals.pushbox_base = game.push and game.push.box_data + (game.push[emu.romname()] or 0)
				if fba then
					return
				end
				print("Copy " .. (#game.breakpoints > 1 and "these " .. #game.breakpoints .. " lines" or "this line") .. 
					" into the MAME-rr debugger to show throwboxes" .. (game.no_combos and " and vulnerability:" or ":"))
				for n = 1, #game.breakpoints do
					print()
					local bpstring = ""
					for _, bp in ipairs(game.breakpoints[n]) do
						bpstring = string.format("%sbp %06X, %s, {%s; g}; ", 
							bpstring, bp.base + (bp[emu.romname()] or 0), bp.cond or "1", bp.cmd)
					end
					print(bpstring:sub(1, -3))
				end
				return
			end
		end
	end
	print("not prepared for " .. emu.romname() .. " hitboxes")
end


savestate.registerload(function()
	frame_buffer = {}
end)


emu.registerstart(function()
	whatgame()
end)