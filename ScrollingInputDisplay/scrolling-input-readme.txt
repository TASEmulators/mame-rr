This is a Lua script to show the player's inputs, "Training Mode" style.
The icons scroll down the screen as the keys are pressed.
It's configured for 6-button fighting games.

Instructions:
Extract the contents of the archive to your emulator folder.
Open the emulator, run your game, and find Lua scripting in the menu.
Browse to the script and click "Run".

This script depends on the Lua gd library. For more information see:
http://luaforge.net/projects/lua-gd/

The gd dlls are copied from:
http://luaforge.net/frs/download.php/1594/lua-gd-2.0.33r2-win32.zip


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