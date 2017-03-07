### Repository

https://github.com/TASVideos/mame-rr

### Usage
* You can replay/record movies by pressing the corresponding hotkeys (Control+R and Control+N by default). There are also hotkeys for stopping a movie (Control+T by default) and for playing the active movie from the beginning (Shift+R by default).
* You can toggle movie read-only/read+write mode (Control+Q by default).
* You can use RAM Search and RAM Watch by pressing the corresponding hotkeys (Control+F and Control+W by default).
* You can open the Lua console by pressing the corresponding hotkeys (Control+L by default) or with the command line parameter "-lua" (example: mame.exe cninja -lua example.lua).
* AVI (uncomressed) can be recorded from the movie using the command: mame-rr romname -pb moviename.mar -aviwrite aviname.avi. To stop recording, just close the emulator.
* If movie recording was started while paused, the replay dialog should also be opened while paused.
* If the state was saved while paused, it should also be loaded while paused.

### Compiling
* Download mingw-mame-20100102.exe:

    http://www.mameworld.info/ubbthreads/showthreaded.php?Cat=&Number=207730    
    https://github.com/TASVideos/mame-rr/releases
    
* Add path to mingw64-w32\bin to your system path variable:

	    set path=mingw\mingw64-w32\bin
	    
* Open command line in the folder with mame-rr makefile
* To build with gcc, release configuration (mame-rr.exe):

	    make
	    
* To buld with gcc, debug configuration and symbols (mamed-rr.exe):

	    make DEBUG=1 SYMBOLS=1
	    
* To build with Microsoft compiler and linker, release configuration (vmame-rr.exe):

	    call "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall" x86
	    make MSVC_BUILD=1
	    
* To build with Microsoft compiler and linker, debug configuration and symbols (vmamed-rr.exe):

	    call "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall" x86
	    make MSVC_BUILD=1 DEBUG=1 SYMBOLS=1
		
### What's new in 0.139-v0.1-beta
* Split AVI at 2GB
* Report intended aspect ratio for games
* Statically link Lua 5.1.4
		
### What's new in 0.139-v0.1-alpha
* Read-only switch
* Movie header now stores framerate (as it should be parsed too for tasvideos submissions to provide proper movie time), the header changed a bit
* Movie replay dialog shows new contents (rom name, emulator version, framerate)
* HUD shows more info relevant to tasing (recording/paused/fastforwarding, number of frames, total frames)
