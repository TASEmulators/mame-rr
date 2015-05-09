


---

# Introduction #

Although the sprites on screen seem to interact directly with one another, 2D fighting games and other action games usually use unseen hitboxes to determine whether attacks connect or miss. These boxes can be made visible in MAME-rr and [FBA-rr](http://code.google.com/p/fbarr/) through the use of [Lua scripting](LuaScriptingFunctions.md). This information can help players improve their game and help people who don't play the games to appreciate how they work.

Also see [combovid](http://combovid.com/?category_name=technical) for in-depth discussions.

## Common box types ##

Some game engines have specialized kinds of boxes, but here are the types that are found in most games.

![http://img96.imageshack.us/img96/6730/hitboxdemo.png](http://img96.imageshack.us/img96/6730/hitboxdemo.png)

### Vulnerability and Attack ###
The attack box must touch the opponent's vulnerability for the attack to connect. Characters may have one or several vulnerability boxes for different areas of the sprite, but most fighting games only allow attacks to have one attack box at a time.

### Projectile Vulnerability and Attack ###
If an opposing projectile's attack touches a projectile vulnerability box, the projectiles negate one another. In some games, projectiles don't have designated vulnerability areas and negate by touching attack boxes.

### Push ###
A pushbox cannot occupy the same space as another pushbox. Attempting to press them together will cause characters to push one another. Jumping attacks that extend below and behind the pushbox allow for easier crossup attacks. Some moves cause pushboxes to disappear, allowing the character to pass through the opponent. In primitive games like Fatal Fury 1 & 2, vulnerability boxes also perform the function of pushboxes.

### Throw and Throwable ###
A throwbox must touch the opponent's throwable box for the throw attempt to succeed. Throws are generally active for only one frame. Some games don't have designated throwable boxes and reuse the pushbox for this purpose.


---

# Usage #

These scripts work with either parent and clone ROMs. Download the script file, load the appropriate ROM, launch a Lua window (ctrl-L by default), then browse for and run the .lua file. The ROM must be up to date for MAME-rr or FBA-rr, and may be out of date with respect to [mainline MAME](http://mamedev.org/).

**The current versions can be downloaded from the [SVN repository](http://mame-rr.googlecode.com/svn/lua/).** Changes are trackable on the [update page](http://code.google.com/p/mame-rr/updates/list) or by subscribing to the update feed.

The behavior can be modified by pressing Lua hotkeys. (The key bindings are assigned in the emulator settings.)
  1. Hide or reveal all the graphics on the screen to show only the boxes. Default off.
  1. Hide or reveal axis markers for characters and other objects. Default on.
  1. Hide or reveal the centers of individual boxes. Default off.
  1. Hide or reveal pushboxes. Default on.
  1. Hide or reveal throwability boxes. Default off.

The best way to get images is with the emulator's screenshot function (F12 by default).


---

## MAME-rr vs. FBA-rr ##
Both emulators can show boxes, but they differ in usability and accuracy.

MAME-rr advantages:
  * doesn't crash so much on loading savestates or loading new ROMs
  * doesn't demand so much CPU when displaying partially transparent boxes
  * more powerful cheat engine means a more reliable background removal procedure
  * no frame sync problems for CPS2 games
  * can use debugger breakpoints for games on any system, but this requires extra effort in coding and usage

FBA-rr advantages:
  * much better performance for CPS3 games
  * built-in Lua breakpoints for NeoGeo, CPS1 and CPS2 systems
    * This means only FBA can properly show throwboxes for the `sf2` and `cps2` scripts, and is required for the `garou` script.


---

## Background removal ##

The game backgrounds can make the boxes harder to see. This can be offset by increasing the fill opacity of the boxes, but this in turn makes the character sprites harder to see. The ideal solution is to remove the backgrounds entirely. One way to do this is with "cheat" codes that prevent the background and HUD from being displayed by the game.

![http://img84.imageshack.us/img84/3733/ssf2twithbg.png](http://img84.imageshack.us/img84/3733/ssf2twithbg.png)
![http://img508.imageshack.us/img508/8744/ssf2tblackbg.png](http://img508.imageshack.us/img508/8744/ssf2tblackbg.png)

Here is how to get the codes working in MAME:
  1. Download the [latest official cheat pack](http://www.mamecheat.co.uk/).
  1. Extract cheat.zip from the downloaded archive to the base MAME folder.
  1. Run the ROM in MAME, find _Cheats_ from the main menu, and set the code to _On_.

Other information:
  * The codes for some games need to be activated before the start of the match to remove the HUD.
  * The codes are designed to only affect the game while a match is in progress.
  * The effects of some codes are not fully reversible.
  * Most of the codes patch ROM data and will only work with parent sets, not clones.
  * The background color for NeoGeo games can be adjusted by editing the color value in the codes.
  * It possible to convert codes for some, not all, games to work in FBA.

![http://img703.imageshack.us/img703/2167/ssf2tnobg.png](http://img703.imageshack.us/img703/2167/ssf2tnobg.png)

Screenshots or videos captured with the backgrounds removed can easily be processed to make the background color transparent. In order for this to work with hitboxes, the fill opacity must be set to zero and the border opacity should be maximum. A shortcut to do this is to set `no_alpha` in the `globals` at the top of the script to `true`.

```
local globals = {
	axis_color           = 0xFFFFFFFF,
	blank_color          = 0xFFFFFFFF,
	axis_size            = 12,
	mini_axis_size       = 2,
	blank_screen         = false,
	draw_axis            = true,
	draw_mini_axis       = false,
	draw_pushboxes       = true,
	draw_throwable_boxes = false,
	no_alpha             = true, --fill = 0x00, outline = 0xFF for all box types
}
```

---

## Color customization ##

Colors can be changed by editing the `box` values at the top of the script.
```
	      ["vulnerability"] = {color = 0x7777FF, fill = 0x40, outline = 0xFF},
	             ["attack"] = {color = 0xFF0000, fill = 0x40, outline = 0xFF},
	["proj. vulnerability"] = {color = 0x00FFFF, fill = 0x40, outline = 0xFF},
	       ["proj. attack"] = {color = 0xFF66FF, fill = 0x40, outline = 0xFF},
	               ["push"] = {color = 0x00FF00, fill = 0x20, outline = 0xFF},
	              ["throw"] = {color = 0xFFFF00, fill = 0x40, outline = 0xFF},
	          ["throwable"] = {color = 0xF0F0F0, fill = 0x20, outline = 0xFF},
```

The `color` numbers are [RGB values](http://www.w3schools.com/html/html_colors.asp), and the `fill` and `outline` determine the opacity of the inside and outside of the box. `0xFF` is fully opaque and `0x00` is invisible.

The default colors are subject to change in future versions.


---

# Supported games #

## sf2-hitboxes.lua ##
All of the Street Fighter II games.
|Parent ROM|Game|
|:---------|:---|
|[sf2](http://www.progettoemma.net/gioco.php?game=sf2)|Street Fighter II: The World Warrior|
|[sf2ce](http://www.progettoemma.net/gioco.php?game=sf2ce)|Street Fighter II': Champion Edition|
|[sf2hf](http://www.progettoemma.net/gioco.php?game=sf2hf)|Street Fighter II': Hyper Fighting|
|[ssf2](http://www.progettoemma.net/gioco.php?game=ssf2)|Super Street Fighter II: The New Challengers|
|[ssf2t](http://www.progettoemma.net/gioco.php?game=ssf2t)|Super Street Fighter II Turbo|
|[hsf2](http://www.progettoemma.net/gioco.php?game=hsf2)|Hyper Street Fighter 2: The Anniversary Edition|

![http://img853.imageshack.us/img853/918/sf2sf2.png](http://img853.imageshack.us/img853/918/sf2sf2.png)
![http://img827.imageshack.us/img827/9961/sf2sf2ceuc.png](http://img827.imageshack.us/img827/9961/sf2sf2ceuc.png)
![http://img94.imageshack.us/img94/6789/sf2sf2hf.png](http://img94.imageshack.us/img94/6789/sf2sf2hf.png)
![http://img17.imageshack.us/img17/8926/sf2ssf2.png](http://img17.imageshack.us/img17/8926/sf2ssf2.png)
![http://img854.imageshack.us/img854/4783/sf2ssf2t.png](http://img854.imageshack.us/img854/4783/sf2ssf2t.png)
![http://img33.imageshack.us/img33/1657/sf2hsf2.png](http://img33.imageshack.us/img33/1657/sf2hsf2.png)

### Notes ###
![http://img51.imageshack.us/img51/1993/colorssf2.png](http://img51.imageshack.us/img51/1993/colorssf2.png)

#### `weak` box ####
Attacks that hit a weakbox deal double damage. Weak boxes only appear in _Street Fighter II: World Warrior_ and in WW mode of _Hyper Street Fighter 2_. They are colored the same as throwboxes by default.

#### `air throwable` box ####
This box is susceptible to air throws but not ground throws. The opposite is true for the normal throwable box.


---

## marvel-hitboxes.lua ##
All of the Marvel/Versus games on the CPS-2 system.
|Parent ROM|Game|
|:---------|:---|
|[xmcota](http://www.progettoemma.net/gioco.php?game=xmcota)|X-Men: Children of the Atom|
|[msh](http://www.progettoemma.net/gioco.php?game=msh)|Marvel Super Heroes|
|[xmvsf](http://www.progettoemma.net/gioco.php?game=xmvsf)|X-Men Vs. Street Fighter|
|[mshvsf](http://www.progettoemma.net/gioco.php?game=mshvsf)|Marvel Super Heroes Vs. Street Fighter|
|[mvsc](http://www.progettoemma.net/gioco.php?game=mvsc)|Marvel Vs. Capcom: Clash of Super Heroes|

![http://img30.imageshack.us/img30/4595/marvelxmcota.png](http://img30.imageshack.us/img30/4595/marvelxmcota.png)
![http://img844.imageshack.us/img844/5650/marvelmsh.png](http://img844.imageshack.us/img844/5650/marvelmsh.png)
![http://img217.imageshack.us/img217/3070/marvelxmvsf.png](http://img217.imageshack.us/img217/3070/marvelxmvsf.png)
![http://img707.imageshack.us/img707/5739/marvelmshvsf.png](http://img707.imageshack.us/img707/5739/marvelmshvsf.png)
![http://img153.imageshack.us/img153/3811/marvelmvsc.png](http://img153.imageshack.us/img153/3811/marvelmvsc.png)

### Notes ###
FBA is needed to show the exact frames when throws are active and when pushboxes are inactive. In MAME, only the potential throw range is viewable, and pushboxes tend to flicker or not disappear when inactive. However, boxes sometimes update a frame early or late in FBA.

![http://img23.imageshack.us/img23/4328/colorsmsh.png](http://img23.imageshack.us/img23/4328/colorsmsh.png)

#### `potential throw` box ####
It's possible to show where the box would appear if a throw were attempted. This box type is fully transparent (invisible) by default and can be shown by increasing its opacity value. This is unnecessary since FBA can show the real throw attempts.


---

## cps2-hitboxes.lua ##
All of the remaining CPS-2 fighting games, including the Street Fighter Alpha and Darkstalkers series.
|Parent ROM|Game|
|:---------|:---|
|[sfa](http://www.progettoemma.net/gioco.php?game=sfa)|Street Fighter Alpha: Warriors' Dreams|
|[sfa2](http://www.progettoemma.net/gioco.php?game=sfa2)|Street Fighter Alpha 2|
|[sfz2al](http://www.progettoemma.net/gioco.php?game=sfz2al)|Street Fighter Zero 2 Alpha|
|[sfa3](http://www.progettoemma.net/gioco.php?game=sfa3)|Street Fighter Alpha 3|
|[dstlk](http://www.progettoemma.net/gioco.php?game=dstlk)|Darkstalkers: The Night Warriors|
|[nwarr](http://www.progettoemma.net/gioco.php?game=nwarr)|Night Warriors: Darkstalkers' Revenge|
|[vsav](http://www.progettoemma.net/gioco.php?game=vsav)|Vampire Savior: The Lord of Vampire|
|[vhunt2](http://www.progettoemma.net/gioco.php?game=vhunt2)|Vampire Hunter 2: Darkstalkers Revenge|
|[vsav2](http://www.progettoemma.net/gioco.php?game=vsav2)|Vampire Savior 2: The Lord of Vampire|
|[ringdest](http://www.progettoemma.net/gioco.php?game=ringdest)|Ring of Destruction: Slammasters II|
|[cybots](http://www.progettoemma.net/gioco.php?game=cybots)|Cyberbots: Fullmetal Madness|
|[sgemf](http://www.progettoemma.net/gioco.php?game=sgemf)|Super Gem Fighter Mini Mix|

![http://img694.imageshack.us/img694/4024/cps2sfa.png](http://img694.imageshack.us/img694/4024/cps2sfa.png)
![http://img836.imageshack.us/img836/7163/cps2sfa2.png](http://img836.imageshack.us/img836/7163/cps2sfa2.png)
![http://img218.imageshack.us/img218/236/cps2sfz2al.png](http://img218.imageshack.us/img218/236/cps2sfz2al.png)
![http://img535.imageshack.us/img535/9103/cps2sfa3.png](http://img535.imageshack.us/img535/9103/cps2sfa3.png)
![http://img42.imageshack.us/img42/4465/cps2dstlk.png](http://img42.imageshack.us/img42/4465/cps2dstlk.png)
![http://img849.imageshack.us/img849/2909/cps2nwarr.png](http://img849.imageshack.us/img849/2909/cps2nwarr.png)
![http://img689.imageshack.us/img689/6391/cps2vsav.png](http://img689.imageshack.us/img689/6391/cps2vsav.png)
![http://img109.imageshack.us/img109/5154/cps2vhunt2.png](http://img109.imageshack.us/img109/5154/cps2vhunt2.png)
![http://img807.imageshack.us/img807/8093/cps2vsav2.png](http://img807.imageshack.us/img807/8093/cps2vsav2.png)
![http://img560.imageshack.us/img560/5288/cps2ringdest.png](http://img560.imageshack.us/img560/5288/cps2ringdest.png)
![http://img546.imageshack.us/img546/7540/cps2cybots.png](http://img546.imageshack.us/img546/7540/cps2cybots.png)
![http://img696.imageshack.us/img696/5448/cps2sgemf.png](http://img696.imageshack.us/img696/5448/cps2sgemf.png)

### Notes ###
FBA is required to show all throws in: sfa, sfa2, sfz2al, vsav, vhunt2, vsav2, cybots, and sgemf. It is also needed in dstlk and nwarr to show air throws and to make ground throws be attempted at long range. However, boxes sometimes update a frame early or late in FBA.

![http://img855.imageshack.us/img855/5071/colorssfa2.png](http://img855.imageshack.us/img855/5071/colorssfa2.png)

#### `axis throw` box ####
Air throws in sfa, sfa2 & sfz2al, and special ranged throws in nwarr grab at the opponent's axis instead of a box.


---

## beatemup-hitboxes.lua ##
Selected beat-em-up games on the CPS-2 system.
|Parent ROM|Game|
|:---------|:---|
|[ffight](http://www.progettoemma.net/gioco.php?game=ffight)|Final Fight|
|[dino](http://www.progettoemma.net/gioco.php?game=dino)|Cadillacs and Dinosaurs|
|[punisher](http://www.progettoemma.net/gioco.php?game=punisher)|The Punisher|
|[avsp](http://www.progettoemma.net/gioco.php?game=avsp)|Alien vs. Predator|
|[ddtod](http://www.progettoemma.net/gioco.php?game=ddtod)|Dungeons & Dragons: Tower of Doom|
|[ddsom](http://www.progettoemma.net/gioco.php?game=ddsom)|Dungeons & Dragons: Shadow over Mystara|
|[batcir](http://www.progettoemma.net/gioco.php?game=batcir)|Battle Circuit|

![http://img822.imageshack.us/img822/9829/beatemupffight.png](http://img822.imageshack.us/img822/9829/beatemupffight.png)
![http://img819.imageshack.us/img819/744/beatemupdino.png](http://img819.imageshack.us/img819/744/beatemupdino.png)
![http://img708.imageshack.us/img708/2287/beatemuppunisher.png](http://img708.imageshack.us/img708/2287/beatemuppunisher.png)
![http://img269.imageshack.us/img269/8210/beatemupavsp.png](http://img269.imageshack.us/img269/8210/beatemupavsp.png)
![http://img263.imageshack.us/img263/8329/beatemupddtod.png](http://img263.imageshack.us/img263/8329/beatemupddtod.png)
![http://img231.imageshack.us/img231/1513/beatemupddsom.png](http://img231.imageshack.us/img231/1513/beatemupddsom.png)
![http://img23.imageshack.us/img23/1985/beatemupbatcir.png](http://img23.imageshack.us/img23/1985/beatemupbatcir.png)

### Notes ###
Non-character objects are colored as projectiles. Pushboxes are not used.


---

## kof-hitboxes.lua ##
Most of the King of Fighters games on the NeoGeo MVS system.
|Parent ROM|Game|
|:---------|:---|
|[kof94](http://www.progettoemma.net/gioco.php?game=kof94)|The King of Fighters '94|
|[kof95](http://www.progettoemma.net/gioco.php?game=kof95)|The King of Fighters '95|
|[kof96](http://www.progettoemma.net/gioco.php?game=kof96)|The King of Fighters '96|
|[kof97](http://www.progettoemma.net/gioco.php?game=kof97)|The King of Fighters '97|
|[kof98](http://www.progettoemma.net/gioco.php?game=kof98)|The King of Fighters '98 - The Slugfest|
|[kof99](http://www.progettoemma.net/gioco.php?game=kof99)|The King of Fighters '99 - Millennium Battle|
|[kof2000](http://www.progettoemma.net/gioco.php?game=kof2000)|The King of Fighters 2000|
|[kof2001](http://www.progettoemma.net/gioco.php?game=kof2001)|The King of Fighters 2001|
|[kof2002](http://www.progettoemma.net/gioco.php?game=kof2002)|The King of Fighters 2002|

![http://img52.imageshack.us/img52/5269/neogeokof94.png](http://img52.imageshack.us/img52/5269/neogeokof94.png)
![http://img600.imageshack.us/img600/9328/neogeokof95.png](http://img600.imageshack.us/img600/9328/neogeokof95.png)
![http://img715.imageshack.us/img715/993/neogeokof96.png](http://img715.imageshack.us/img715/993/neogeokof96.png)
![http://img200.imageshack.us/img200/231/neogeokof97.png](http://img200.imageshack.us/img200/231/neogeokof97.png)
![http://img23.imageshack.us/img23/660/neogeokof98.png](http://img23.imageshack.us/img23/660/neogeokof98.png)
![http://img545.imageshack.us/img545/2005/neogeokof99a.png](http://img545.imageshack.us/img545/2005/neogeokof99a.png)
![http://img98.imageshack.us/img98/3276/neogeokof2000.png](http://img98.imageshack.us/img98/3276/neogeokof2000.png)
![http://img197.imageshack.us/img197/1767/neogeokof2001.png](http://img197.imageshack.us/img197/1767/neogeokof2001.png)
![http://img840.imageshack.us/img840/5589/neogeokof2002.png](http://img840.imageshack.us/img840/5589/neogeokof2002.png)

### Notes ###
All throwboxes in NeoGeo games show up one frame late. Not enough frames pass between the box computations and the graphics update to correct this.

FBA is needed to show throwboxes in `kof94` and `kof95`. The the only `axis throw` boxes for this script are the special throws in those two games.

![http://img231.imageshack.us/img231/78/colorskof98.png](http://img231.imageshack.us/img231/78/colorskof98.png)

#### `guard` box ####
Attacks that hit a guardbox will be blocked, either by manually blocking, by an autoguard move, or by a special counter move.


---

## garou-hitboxes.lua ##
The Garou Densetsu/Fatal Fury NeoGeo series.
|Parent ROM|Game|
|:---------|:---|
|[fatfury1](http://www.progettoemma.net/gioco.php?game=fatfury1)|Fatal Fury - King of Fighters|
|[fatfury2](http://www.progettoemma.net/gioco.php?game=fatfury2)|Fatal Fury 2|
|[fatfursp](http://www.progettoemma.net/gioco.php?game=fatfursp)|Fatal Fury Special|
|[fatfury3](http://www.progettoemma.net/gioco.php?game=fatfury3)|Fatal Fury 3 - Road to the Final Victory|
|[rbff1](http://www.progettoemma.net/gioco.php?game=rbff1)|Real Bout Fatal Fury|
|[rbffspec](http://www.progettoemma.net/gioco.php?game=rbffspec)|Real Bout Fatal Fury Special|
|[rbff2](http://www.progettoemma.net/gioco.php?game=rbff2)|Real Bout Fatal Fury 2 - The Newcomers|
|[garou](http://www.progettoemma.net/gioco.php?game=garou)|Garou - Mark of the Wolves|

![http://img215.imageshack.us/img215/2107/neogeofatfury1.png](http://img215.imageshack.us/img215/2107/neogeofatfury1.png)
![http://img841.imageshack.us/img841/8889/garoufatfury2.png](http://img841.imageshack.us/img841/8889/garoufatfury2.png)
![http://img291.imageshack.us/img291/2624/neogeofatfursa.png](http://img291.imageshack.us/img291/2624/neogeofatfursa.png)
![http://img405.imageshack.us/img405/1434/neogeofatfury3.png](http://img405.imageshack.us/img405/1434/neogeofatfury3.png)
![http://img815.imageshack.us/img815/7351/neogeorbff1.png](http://img815.imageshack.us/img815/7351/neogeorbff1.png)
![http://img854.imageshack.us/img854/8909/garourbffspec.png](http://img854.imageshack.us/img854/8909/garourbffspec.png)
![http://img694.imageshack.us/img694/2333/garourbff2.png](http://img694.imageshack.us/img694/2333/garourbff2.png)
![http://img9.imageshack.us/img9/3346/garougarou.png](http://img9.imageshack.us/img9/3346/garougarou.png)

### Notes ###
This script mostly doesn't work MAME due to extensive use of breakpoints. FBA is required.

The backgrounds can be removed using the script itself by setting `no_background` in the `globals` section to `true`. This must be done before the match start to work.

![http://img405.imageshack.us/img405/2128/colorsrbff2.png](http://img405.imageshack.us/img405/2128/colorsrbff2.png)

There are no throwability or projectile vulnerability boxes.


---

## cps3-hitboxes.lua ##
The CPS3 games, excluding the Jojo titles.
|[redearth](http://www.progettoemma.net/gioco.php?game=redearth)|Red Earth|
|:--------------------------------------------------------------|:--------|
|[sfiii](http://www.progettoemma.net/gioco.php?game=sfiii)|Street Fighter III: New Generation|
|[sfiii2](http://www.progettoemma.net/gioco.php?game=sfiii2)|Street Fighter III 2nd Impact: Giant Attack|
|[sfiii3](http://www.progettoemma.net/gioco.php?game=sfiii3)|Street Fighter III 3rd Strike: Fight for the Future|

![http://img62.imageshack.us/img62/5932/cps3redearth.png](http://img62.imageshack.us/img62/5932/cps3redearth.png)
![http://img715.imageshack.us/img715/9530/cps3sfiii3.png](http://img715.imageshack.us/img715/9530/cps3sfiii3.png)

### Notes ###
This script was originally translated to Lua from the [mameserver](http://code.google.com/p/mameserver/) project. The 3rd Strike code was also helped by the work of [crystal cube](http://ameblo.jp/3fv/)

For now, the only way to show normal throwboxes in redearth/warzard is with the MAME-rr debugger. Run MAME from the command line with the `-debug` switch:

```
> mame redearth -debug -lua cps3-hitboxes.lua
```

Or to avoid using the command line, edit mame-rr.ini so that `debug` is `1`:

```
#
# CORE DEBUGGING OPTIONS
#
log                       0
verbose                   0
update_in_pause           0
debug                     1
debugscript               
```

The debugger starts by halting execution of the game and the UI. Press F5 to release control. The script will print the BPs to the Lua console:

![http://img600.imageshack.us/img600/4006/consoleluabp.png](http://img600.imageshack.us/img600/4006/consoleluabp.png)

Copy and paste the line over to the debugger console and hit enter. Now all supported box types will appear correctly.

![http://img52.imageshack.us/img52/3715/consoledebuggerbpcrop.png](http://img52.imageshack.us/img52/3715/consoledebuggerbpcrop.png)

The game may glitch up when the command is entered but this only happens once. Before quitting or unloading the ROM, enter `bpc` into the debugger console to clear any installed breaks, or else MAME will crash.


---

Errors or inconsistencies can be reported by leaving a comment to this page.