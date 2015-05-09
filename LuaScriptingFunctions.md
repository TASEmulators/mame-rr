See also [Lua manual](http://www.lua.org/manual/5.1/).

**Important: nil is not 0. nil and 0 are two different entities.**

# Global #

### `print` ###

Prints any value or values, mainly to help you debug your script. Unlike the default implementation, this can even print the contents of tables. Also, the printed values will go to the script's output window instead of stdout. Note that if you want to print a memory address you should use print(string.format("0x%X",address)) instead of print(address).

### `tostring` ###

Returns a string that represents the argument. You can use this if you want to get the same string that print would print, but use it for some purpose other than immediate printing. This function is actually what gives print its ability to print tables and other non-string values. Note that there is currently a limit of 65536 characters per result, after which only a "..." is appended, but in typical use you shouldn't ever run into this limit.

### `addressof` ###

Returns the pointer address of a reference-type value. In particular, this can be used on tables and functions to see what their addresses are. There's not much worth doing with a pointer address besides printing it to look at it and see that it's different from the address of something else. Please do not store the address to use for hashing or logical comparison, that is completely unnecessary in Lua because you can simply use the actual object instead of its address for those purposes. If the argument is not a reference type then this function will return 0.

### `copytable` ###

Returns a shallow copy of the given table. In other words, it gives you a different table that contains all of the same values as the original. This is unlike simple assignment of a table, which only copies a reference to the original table. You could write a Lua function that does what this function does, but it's such a common operation that it seems worth having a pre-defined function available to do it.

### `AND,OR,XOR,SHIFT,BIT` ###

Old bit operation functions, use [bit.\*](http://bitop.luajit.org/) functions instead.


---

# emu #

### `emu.speedmode(string mode)` ###

Changes the speed of emulation depending on mode. If "normal", emulator runs at normal speed. If "turbo", emulator runs at max speed and drops some frames. If "nothrottle", emulator runs at max speed without dropping frames.

### `emu.frameadvance()` ###

Pauses script until a frame is emulated. Cannot be called by a coroutine or registered function.

### `emu.pause()` ###

Pauses emulator when the current frame has finished emulating.

### `emu.unpause()` ###

Unpauses emulator.

### `int emu.framecount()` ###

Returns the frame count for the movie, or the number of frames since the start otherwise.

### `string emu.romname()` ###

The name of the ROM currently loaded. (ex: "mutantf")

### `string emu.gamename()` ###

The full name of the game currently loaded. (ex: "Mutant Fighter (World ver EM-5)")

### `string emu.parentname()` ###

The ROM name of the parent of the currently loaded game. This returns 0 if the game IS the parent of the set.

### `string emu.sourcename()` ###

The name of the source file for the currently running driver. (ex: for Mutant Fighter, this would return "cninja.c", since it uses the same driver of Caveman Ninja.)

### `int emu.screenwidth()` ###

Returns the width of the internal resolution that the game uses.

### `int emu.screenheight()` ###

Returns the height of the internal resolution that the game uses.

### `emu.registerbefore(function func)` ###

Registers a callback function to run immediately before each frame gets emulated. This runs after the next frame's input is known but before it's used, so this is your only chance to set the next frame's input using the next frame's would-be input. For example, if you want to make a script that filters or modifies ongoing user input, such as making the game think "left" is pressed whenever you press "right", you can do it easily with this.

Note that this is not quite the same as code that's placed before a call to emu.frameadvance. This callback runs a little later than that. Also, you cannot safely assume that this will only be called once per frame. Depending on the emulator's options, every frame may be simulated multiple times and your callback will be called once per simulation. If for some reason you need to use this callback to keep track of a stateful linear progression of things across frames then you may need to key your calculations to the results of emu.framecount.

Like other callback-registering functions provided by MAME-RR, there is only one registered callback at a time per registering function per script. If you register two callbacks, the second one will replace the first, and the call to emu.registerbefore will return the old callback. You may register nil instead of a function to clear a previously-registered callback. If a script returns while it still has registered callbacks, MAME-RR will keep it alive to call those callbacks when appropriate, until either the script is stopped by the user or all of the callbacks are de-registered.

### `emu.registerafter(function func)` ###

Registers a callback function to run immediately after each frame gets emulated. It runs at a similar time as (and slightly before) gui.register callbacks, except unlike with gui.register it doesn't also get called again whenever the screen gets redrawn. Similar caveats as those mentioned in emu.registerbefore apply.

### `emu.registerstart(function func)` ###

Registers a function that runs once immediately if emulation has already started, and also runs again whenever the game is reset. A soft reset or movie playback will not cause the entire script to restart, so if you have some code that needs to run when the game starts in addition to when the script starts, then register it with this.

### `emu.registerexit(function func)` ###

Registers a callback function that runs when the script stops. Whether the script stops on its own or the user tells it to stop, or even if the script crashes or the user tries to close the emulator, MAME-RR will try to run whatever Lua code you put in here first. So if you want to make sure some code runs that cleans up some external resources or saves your progress to a file or just says some last words, you could put it here. (Of course, a forceful termination of the application or a crash from inside the registered exit function will still prevent the code from running.)

Suppose you write a script that registers an exit function and then enters an infinite loop. If the user clicks "Stop" your script will be forcefully stopped, but then it will start running its exit function. If your exit function enters an infinite loop too, then the user will have to click "Stop" a second time to really stop your script. That would be annoying. So try to avoid doing too much inside the exit function.

Note that restarting a script counts as stopping it and then starting it again, so doing so (either by clicking "Restart" or by editing the script while it is running) will trigger the callback. Note also that returning from a script generally does NOT count as stopping (because your script is still running or waiting to run its callback functions and thus does not stop... see here for more information), even if the exit callback is the only one you have registered.

### `emu.message(string msg)` ###

Displays the message on the screen.


---

# memory #

### `int memory.readbyte(int addr)` ###
### `int memory.readbytesigned(int addr)` ###
### `int memory.readword(int addr)` ###
### `int memory.readwordsigned(int addr)` ###
### `int memory.readdword(int addr)` ###
### `int memory.readdwordsigned(int addr)` ###

Reads value from memory address. Word=2 bytes, Dword=4 bytes.

### `string memory.readbyterange(int startaddr, int length)` ###

Returns a chunk of memory from the given address with the given length as a string. To access, use _string.byte(str,offset)_.

### `memory.writebyte(int addr)` ###
### `memory.writeword(int addr)` ###
### `memory.writedword(int addr)` ###

Writes value to memory address.


---

# joypad #

Before the next frame is emulated, one may set keys to be pressed. The buffer is cleared each frame.

### `table joypad.get()` ###

Returns a table of every game button, where each entry is true if that button is currently held (as of the last time the emulation checked), or false if it is not held. If a movie is playing, this will read the input actually being received from the movie instead of what the user is pressing. Other controllers (such as analogs or dipswitches) are currently not supported.

The keys for joypad table can be found in each game's input configuration dialog. Keys are case-sensitive.

### `table joypad.getdown()` ###

Returns a table of only the game buttons that are currently held. Each entry is true if that button is currently held (as of the last time the emulation checked), or nil if it is not held. If a movie is playing, this will read the input actually being received from the movie instead of what the user is pressing.

The keys for joypad table can be found in each game's input configuration dialog. Keys are case-sensitive.

### `table joypad.getup()` ###

Returns a table of only the game buttons that are not currently held. Each entry is nil if that button is currently held (as of the last time the emulation checked), or false if it is not held. If a movie is playing, this will read the input actually being received from the movie instead of what the user is pressing.

The keys for joypad table can be found in each game's input configuration dialog. Keys are case-sensitive.

### `joypad.set(table buttons)` ###

Sets the buttons to be pressed next frame. true for pressed, nil or false for not pressed.


---

# savestate #

### `object savestate.create(string filename=nil)` ###

Creates a savestate object.

If any argument is given, it will be saved in the sta subfolder of the game with that filename. If, for example, filename is 1 or "1" (int are automatically translated to strings) this savestate can then be loaded by the emulator by using the "Load State 1" hotkey.

If no argument is given, the savestate can only be accessed by Lua.

### `savestate.save(savestate)` ###

Saves the current state to the savestate object. You are allowed to use slot numbers outside of the normal 0 to 9 range (including negative numbers) and you may pass a number (or any other string) directly into this function without calling savestate.create first.

### `savestate.load(savestate)` ###

Loads the state of the savestate object as the current state. For non-anonymous savestates it is allowed to be a save slot number (or any other string) instead.

### `savestate.registersave(function func)` ###

Registers a function to get called whenever the emulation state is saved to any save slot (either by the user or by a script). In addition, whatever your callback function returns will get saved alongside the savestate file, so you can use this to effectively save extra data of your choosing with each savestate the user saves.

Currently the following types are allowed to be saved (returned by your callback): boolean, number, string, table, nil. If your callback returns multiple values they will all be saved, although you don't technically need to use this because you could also put them all inside a table and return that as a single value.

The callback function func will receive the save slot number as an argument when it is called, although you can make the callback take no arguments if you don't care what number slot it is.

### `savestate.registerload(function func)` ###

Registers a function to get called whenever the emulation state is loaded from any save slot (either by the user or by a script).

The callback function func will receive the save slot number as its first argument when it is called. It will also receive as additional arguments whatever data was returned by a callback registered by savestate.registersave when the state being loaded was last saved. If you don't need to use that data now then don't define your load callback with any additional arguments.

### `savestate.savescriptdata(string location)` ###

Calls any registered save callbacks and saves the return values alongside the savestate that's at the given location. location should be a save slot.

### `savestate.loadscriptdata(string location)` ###

Returns the data associated with the given savestate (data that was earlier returned by a registered save callback) without actually loading the rest of that savestate or calling any callbacks. location should be a save slot.


---

# movie #

### `string movie.mode()` ###

Returns "record" if movie is recording, "playback" if movie is replaying input, or nil if there is no movie.

### `movie.rerecordcounting(boolean skipcounting)` ###

If set to true, no rerecords done by Lua are counted in the rerecord total. If set to false, rerecords done by Lua count. By default, rerecords count.

### `movie.stop()` ###

Stops the movie. Cannot be used if there is no movie.


---

# gui #

Color can be given as "0xrrggbbaa" or as a name (e.g. "red").

### `function gui.register(function func)` ###

Registers a function to be called when the screen is updated. Function can be nil. The previous function is returned, possibly nil.

All drawing process should be done in the callback of this function.

### `gui.pixel(int x, int y [, type color])` ###

Draws a pixel at (x,y) with the given color.

### `gui.line(int x1, int y1, int x2, int y2 [, type color [, skipfirst]])` ###

Draws a line from (x1,y1) to (x2,y2) with the given color.

### `gui.box(int x1, int y1, int x2, int y2 [, type fillcolor [, outlinecolor]])` ###

Draws a box with (x1,y1) and (x2,y2) as opposite corners with the given color.

### `string gui.gdscreenshot()` ###

Takes a screenshot and returns it as a string that can be used by the [gd library](http://lua-gd.luaforge.net/).

For direct access, use _string.byte(str,offset)_. The gd image consists of a 11-byte header and each pixel is alpha,red,green,blue (1 byte each, alpha is 0 in this case) left to right then top to bottom.

### `gui.opacity(float alpha)` ###

Sets the opacity of drawings depending on alpha. 0.0 is invisible, 1.0 is drawn over. Values less than 0.0 or greater than 1.0 work by extrapolation.

### `gui.transparency(float strength)` ###

4.0 is invisible, 0.0 is drawn over. Values less than 0.0 or greater than 4.0 work by extrapolation.

### `gui.text(int x, int y, string msg)` ###

Draws the given text at (x,y). Not to be confused with _emu.message(string msg)_.

### `gui.getpixel(x, y)` ###

Returns the RGB color at the given onscreen pixel location. You can say local r,g,b = gui.getpixel(x,y). r,g,b are the red/green/blue color components of that pixel, each ranging from 0 to 255. If the coordinate you give is offscreen, you will receive the color values of the nearest onscreen pixel instead.

Note that this function can return colors that have already been written to the screen by GUI drawing functions. If for some reason you want to make sure that you only get the clean untampered-with colors the emulation drew onscreen, then you'll have to call this function before any GUI drawing functions have written to the screen for the current frame. Probably the most reliable way to do that is to call gui.getpixel inside of a callback function that you register with emu.registerafter.

### `gui.parsecolor(color)` ###

Returns the separate RGBA components of the given color.

For example, you can say local r,g,b,a = gui.parsecolor('orange') to retrieve the red/green/blue values of the preset color orange. (You could also omit the a in cases like this.) This uses the same conversion method that MAME-RR uses internally to support the different representations of colors that the GUI library uses. Overriding this function will not change how MAME-RR interprets color values, however.

### `gui.gdoverlay([int dx=0, int dy=0,] string str [, sx=0, sy=0, sw, sh] [, float alphamul=1.0])` ###

Overlays the given gd image with top-left corner at (dx,dy) and given opacity.

### `gui.clearuncommitted()` ###

Clears drawing buffer. Only works if it hasn't been drawn on screen yet.

### `string gui.popup(msg [, type [, icon]])` ###

Brings up a modal popup dialog box (everything stops until the user dismisses it). The box displays the message tostring(msg). This function returns the name of the button the user clicked on (as a string).

type determines which buttons are on the dialog box, and it can be one of the following: 'ok', 'yesno', 'yesnocancel', 'okcancel', 'abortretryignore'.
type defaults to 'ok' for gui.popup, or to 'yesno' for input.popup.

icon indicates the purpose of the dialog box (or more specifically it dictates which title and icon is displayed in the box), and it can be one of the following: 'message', 'question', 'warning', 'error'.
icon defaults to 'message' for gui.popup, or to 'question' for input.popup.

Try to avoid using this function much if at all, because modal dialog boxes can be irritating.


---

# input #

### `string input.popup(msg [, type [, icon]])` ###

See gui.popup.

### `table input.get()` ###

Returns a table of which keyboard buttons are pressed as well as mouse status. Key values for keyboard buttons and mouse clicks are true for pressed, nil for not pressed. Mouse position is returned in terms of game screen pixel coordinates. Keys for mouse are (xmouse, ymouse, leftclick, rightclick, middleclick). Keys for keyboard buttons:
(backspace, tab, enter, shift, control, alt, pause, capslock, escape, space, pageup, pagedown, end, home, left, up, right, down, insert, delete,
0, 1, ..., 9, A, B, ..., Z, numpad0, numpad1, ..., numpad9, numpad`*`, numpad+, numpad-, numpad., numpad/, F1, F2, ..., F24,
numlock, scrolllock, semicolon, plus, comma, minus, period, slash, tilde, leftbracket, backslash, rightbracket, quote)

Keys are case-sensitive. Keys for keyboard buttons are for buttons, **not** ASCII characters, so there is no need to hold down shift. Key names may differ depending on keyboard layout. On US keyboard layouts, "slash" is /?, "tilde" is `````~, "leftbracket" is `[``{`, "backslash" is \|, "rightbracket" is `]``}`, "quote" is '".

### `input.registerhotkey(int which, function func)` ###

Registers a callback function to run when a certain hotkey is pressed. which is the hotkey number, and currently must be a number between 1 and 5. This range corresponds with the user-configurable hotkeys "Lua Custom Hotkey 1" through "Lua Custom Hotkey 5", which will do nothing when pressed except trigger the aforementioned callback. These hotkeys must be configured by the user in the input settings beforehand.

The only real advantage this has over other methods of checking input is that it works even if MAME-RR is paused. There is no alternative way of allowing user-controlled activation of script code during a pause, except for using gui.register or other similarly hacky ways.