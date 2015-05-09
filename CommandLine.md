MAME is a command-line program. This page gives basic instructions on how to give it various commands.




---

# Getting started #

Open a command prompt. One way to do this is by clicking the windows _Start_ button, then _Run..._ Type `cmd` in the box and click _OK_.

Change the directory to the location where MAME-rr is set up. For example:
```
cd /d D:\mame-rr\
```

Before any games can be run by MAME you must first designate where the ROMs are located in _mame-rr.ini_, the configuration file. If the file doesn't exist, create one with this command:
```
mame -cc
```

Open _mame-rr.ini_ in a text editor like Notepad and find the `rompath` line. Add the path to your ROMs there. You may have multiple paths separated by semicolons. For example:
```
#
# CORE SEARCH PATH OPTIONS
#
rompath                   roms;E:\MAME\roms
```

This file also controls many other options which are discussed in [docs\config.txt](http://mame-rr.googlecode.com/svn/trunk/mame-rr/docs/config.txt).


---

# Launch commands #

To run the ROM _raiden_:
```
mame raiden
```

To run the game and immediately load savestate 3:
```
mame raiden -state 3
```

To run the game and a [Lua script](LuaScriptingFunctions.md):
```
mame raiden -lua myscript.lua
```

Options can be combined:
```
mame raiden -state 3 -lua myscript.lua
```

See [docs\config.txt](http://mame-rr.googlecode.com/svn/trunk/mame-rr/docs/config.txt) for more.

---

# Making an AVI #

To run the game and record the audio & video to an AVI file, which will be put in the _snap_ folder:
```
mame raiden -aviwrite moviefile.avi
```

Unlike [FBA](http://code.google.com/p/fbarr/wiki/MakingAVI) and other emulators, there is no choice of codec available. The AVI is uncompressed RGB32 and will be very large. This may be unacceptable for long movies. You can get a much smaller result with this command:
```
mame raiden -mngwrite mngfile.mng -wavwrite wavfile.wav
```
The MNG is a lossless video file, like PNG but for movies instead of images. It doesn't contain audio, so the audio is dumped separately with `-wavwrite`. Then you must combine the audio and video and convert to AVI.

[Not many programs can handle MNG.](http://en.wikipedia.org/wiki/MNG#Application_support) One method is the command-line program [mplayer/mencoder](http://oss.netfarm.it/mplayer-win32.php). Download and extract the package. Change to its directory in a command prompt and tell it to convert the MNG and WAV to AVI. For example:
```
mencoder -fps 59.600 -ovc lavc -lavcopts vcodec=ffv1:vstrict=-2 -oac copy -o mng-to-avi.avi 
D:\mame-rr\snap\mngfile.mng -audiofile D:\mame-rr\snap\wavfile.wav
```
(That's a single line.)

This converts _mngfile.mng_ and _wavfile.wav_ to _mng-to-avi.avi_, encoded with FFmpeg’s lossless video codec. (You may use other codecs.) Enter the framerate of the resultant video after `-fps`. The authentic framerate can be found by running the game in MAME and selecting the _Game Information_ item from the menu.

Further processing is up to your encoding and editing software.