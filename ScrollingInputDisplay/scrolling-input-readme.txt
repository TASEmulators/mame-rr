This is a Lua script to show the player's inputs, "Training Mode" style.
The icons scroll down the screen as the keys are pressed.
It works with 6-button games, NeoGeo games and Tekken games, and more can easily be added.
It works with MAME-rr, FBA-rr, PCSX-rr and any other emulator with Lua.

Instructions:
Extract the contents of the archive to your emulator folder.
Pick the icon set you want and put the filename as "iconfile" in scrolling-input-display.lua.
Open the emulator, run your game, and find Lua scripting in the menu.
Browse to the script and click "Run".

This script depends on the Lua gd library. For more information see:
http://luaforge.net/projects/lua-gd/

The gd dlls are copied from:
http://luaforge.net/frs/download.php/1594/lua-gd-2.0.33r2-win32.zip


New to v006:
* Modularized the icon selection. Now you can easily switch between multiple icon/control modules and add new modules.
* Renamed icon sets:
  * scrolling-input-icons-16.png to icons-capcom-16.png
  * scrolling-input-icons-32.png to icons-capcom-32.png
* Added icon sets:
  * icons-neogeo-16.png
  * icons-neogeo-32.png
  * icons-tekken-32.png
* The actual code has been separated into another file named scrolling-input-code.lua.
* The scrolling-input-display.lua file contains only the modules and user settings and may be edited by the user.
* The code file, the icon files, and the framedump folder have been moved into the scrolling-input folder. This was done to reduce file clutter.
* Added a user parameter, screenwidth, which lets you specify the size of the screen for determining where P2's icons go. This setting only applies if the emu doesn't have emu.screenwidth(). It's useful for PSX games because they have various screen dimensions.


New to v005:
* Added the 16x16 pixel icon set from v002. You can change icon sets by editing the "sourcefile" line.
* The blank background png file is no longer necessary and has been removed.
* The dlls that were removed turned out to be needed and have been readded.


New to v004:
* Added icon resize key.
* Icons keep away from the edge of the screen by a fixed amount of tiles.
* This margin is adjusted dynamically when the icons are stretched or resized or when the game resolution changes.
* User options are easier to understand.


New to v003:
* Support for MAME-rr.
* Added lua51.dll for MAME-rr and removed unneeded dlls.
* All files are at the root to accommodate MAME-rr.
* All icons are merged into a single image file.
* Toggleable rescale icons option, which resizes icons to appear normal when the screen is scaled to 4:3.
* The display state is saved to savestate for emulators that support it.
* Uses Lua hotkeys instead of predefined keys for emulators that support it.

* The current version of MAME-rr (0139-test2) does not support framedump mode, but regular screenshots work.


New to v002:
* Included gd.dll which was left out before for some reason.
* Clear screen key.
* Works fine with pcsx.
* Frame by frame recording mode. Slow, but it works. Use in conjunction with audio dump and convert to avi with VirtualDub.