

# Introduction #

This is a Lua script for fighting games that displays gameplay-relevant data on the screen in numerical form. It can use this data to draw colored bars for further illustration. This information is useful for studying game mechanics.

Pasky has a more detailed script specializing in [SSF2T](http://code.google.com/p/ssf2thud/).

# Usage #

[Download fighting-OSD.lua](http://mame-rr.googlecode.com/svn/lua/), load the appropriate ROM, launch a Lua window (ctrl-L by default), then browse for and run the .lua file. This script works with either parent and clone ROMs, and with either MAME-rr or [code.google.com/p/fbarr/ FBA-rr].

The behavior can be modified by pressing Lua hotkeys. (The key bindings are assigned in the emulator settings.)
  1. Hide or reveal text data. Default on.
  1. Hide or reveal bars. Default on.

Colors for text and bars can be customized by editing the values at the top of the script.

# Data types #

All supported games show numerical counters for vitality. The values for super and guard meters are also shown when applicable. All values are given in the form of "current/maximum".

Stun bars come in pairs: The top (red) gives the amount of stun damage relative to the maximum amount that can be taken. The bottom bar depends on the state of the character: If not dizzy, it shows the time that must pass before the damage is cleared (yellow). If dizzy, it shows the time before the stun wears off (blue), and the "STUN" graphic from [Street Fighter 3: Third Strike](http://maws.mameworld.info/maws/romset/sfiii3) flickers over the top bar. If dizzy state has recently ended, it shows the grace period during which no stun damage can be taken (green). Both bars are accompanied by the numerical values.

Some games display additional information, for example:
  * The character Balrog/Vega in the SF2 games can take a certain number of hits before his claw is prone to fall off. This durability counter is shown. In SFA3 he is also liable to lose his mask.
  * Data for all characters is shown during three-player modes of SFA, SFZ2AL, SFA3, and FF1.
  * In SFA3, a "!" is shown next to a player's vitality when an air recovery can be performed.
  * In SFA3, a "`*`" is shown next to the combo meter if one or more hits of the combo landed when the opponent could have performed an air recovery.
  * Rage meters, complete with a pair of bars, are shown for Slammasters II.

# Supported games #

|Parent<br>ROM<table><thead><th>Game</th><th>Stun<br>bars</th><th>Super<br>meter</th><th>Guard<br>meter</th><th>Other</th></thead><tbody>
<tr><td><a href='http://maws.mameworld.info/maws/romset/sf2'>sf2</a></td><td>Street Fighter II: The World Warrior</td><td>✓</td><td>  </td><td>  </td><td>claw meter</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/sf2ce'>sf2ce</a></td><td>Street Fighter II': Champion Edition</td><td>✓</td><td>  </td><td>  </td><td>claw meter</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/sf2hf'>sf2hf</a></td><td>Street Fighter II': Hyper Fighting</td><td>✓</td><td>  </td><td>  </td><td>claw meter</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/ssf2'>ssf2</a></td><td>Super Street Fighter II: The New Challengers</td><td>✓</td><td>  </td><td>  </td><td>claw meter</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/ssf2t'>ssf2t</a></td><td>Super Street Fighter II Turbo</td><td>✓</td><td>✓</td><td>  </td><td>claw meter</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/hsf2'>hsf2</a></td><td>Hyper Street Fighter 2: The Anniversary Edition</td><td>✓</td><td>✓</td><td>  </td><td>claw meter</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/sfa'>sfa</a></td><td>Street Fighter Alpha: Warriors' Dreams</td><td>✓</td><td>✓</td><td>  </td><td>three-player mode</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/sfa2'>sfa2</a></td><td>Street Fighter Alpha 2</td><td>✓</td><td>✓</td><td>  </td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/sfz2al'>sfz2al</a></td><td>Street Fighter Zero 2 Alpha</td><td>✓</td><td>✓</td><td>  </td><td>three-player mode</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/sfa3'>sfa3</a></td><td>Street Fighter Alpha 3</td><td>✓</td><td>✓</td><td>✓</td><td>three-player mode<br>claw & mask meters<br>flip indicator<br>pseudocombo indicator</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/dstlk'>dstlk</a></td><td>Darkstalkers: The Night Warriors</td><td>  </td><td>✓</td><td>  </td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/nwarr'>nwarr</a></td><td>Night Warriors: Darkstalkers' Revenge</td><td>  </td><td>✓</td><td>  </td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/vsav'>vsav</a></td><td>Vampire Savior: The Lord of Vampire</td><td>  </td><td>✓</td><td>  </td><td>Dark Force timer</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/vhunt2'>vhunt2</a></td><td>Vampire Hunter 2: Darkstalkers Revenge</td><td>  </td><td>✓</td><td>  </td><td>Dark Force timers</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/vsav2'>vsav2</a></td><td>Vampire Savior 2: The Lord of Vampire</td><td>  </td><td>✓</td><td>  </td><td>Dark Force timers</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/ringdest'>ringdest</a></td><td>Ring of Destruction: Slammasters II</td><td>✓</td><td>  </td><td>  </td><td>heat-up meters</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/cybots'>cybots</a></td><td>Cyberbots: Fullmetal Madness</td><td>  </td><td>✓</td><td>  </td><td>Gun & Boost timers</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/sgemf'>sgemf</a></td><td>Super Gem Fighter Mini Mix</td><td>✓</td><td>✓</td><td>  </td><td>gem power counters</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/xmcota'>xmcota</a></td><td>X-Men: Children of the Atom</td><td>✓</td><td>✓</td><td>  </td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/msh'>msh</a></td><td>Marvel Super Heroes</td><td>✓</td><td>✓</td><td>  </td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/xmvsf'>xmvsf</a></td><td>X-Men Vs. Street Fighter</td><td>✓</td><td>✓</td><td>  </td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/mshvsf'>mshvsf</a></td><td>Marvel Super Heroes Vs. Street Fighter</td><td>✓</td><td>✓</td><td>  </td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/mvsc'>mvsc</a></td><td>Marvel Vs. Capcom: Clash of Super Heroes</td><td>✓</td><td>✓</td><td>  </td><td>  </td></tr>
<tr><td>[maws.mameworld.info/maws/romset/sfiii sfiii]</td><td>Street Fighter III: New Generation</td><td>✓</td><td>✓</td><td>  </td><td>charge meters<br>stun recovery</td></tr>
<tr><td>[maws.mameworld.info/maws/romset/sfiii2 sfiii2]</td><td>Street Fighter III 2nd Impact: Giant Attack</td><td>✓</td><td>✓</td><td>  </td><td>charge meters<br>juggle counter<br>PA bonuses & stun recovery</td></tr>
<tr><td>[maws.mameworld.info/maws/romset/sfiii3 sfiii3]</td><td>Street Fighter III 3rd Strike: Fight for the Future</td><td>✓</td><td>✓</td><td>  </td><td>charge meters<br>juggle counter<br>PA bonuses & stun recovery</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/kof94'>kof94</a></td><td>The King of Fighters '94</td><td>✓</td><td>✓</td><td>  </td><td>Maxmode timer</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/kof95'>kof95</a></td><td>The King of Fighters '95</td><td>✓</td><td>✓</td><td>  </td><td>Maxmode timer</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/kof96'>kof96</a></td><td>The King of Fighters '96</td><td>✓</td><td>✓</td><td>✓</td><td>Maxmode timer</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/kof97'>kof97</a></td><td>The King of Fighters '97</td><td>✓</td><td>✓</td><td>✓</td><td>Maxmode timer</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/kof98'>kof98</a></td><td>The King of Fighters '98 - The Slugfest</td><td>✓</td><td>✓</td><td>✓</td><td>Maxmode timer</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/kof99'>kof99</a></td><td>The King of Fighters '99 - Millennium Battle</td><td>✓</td><td>✓</td><td>✓</td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/kof2000'>kof2000</a></td><td>The King of Fighters 2000</td><td>✓</td><td>✓</td><td>✓</td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/kof2001'>kof2001</a></td><td>The King of Fighters 2001</td><td>  </td><td>✓</td><td>✓</td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/kof2002'>kof2002</a></td><td>The King of Fighters 2002</td><td>  </td><td>✓</td><td>✓</td><td>Maxmode timer</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/fatfury1'>fatfury1</a></td><td>Fatal Fury - King of Fighters</td><td>  </td><td>  </td><td>  </td><td>three-player mode</td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/fatfury2'>fatfury2</a></td><td>Fatal Fury 2</td><td>✓</td><td>  </td><td>  </td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/fatfursp'>fatfursp</a></td><td>Fatal Fury Special</td><td>✓</td><td>  </td><td>  </td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/fatfury3'>fatfury3</a></td><td>Fatal Fury 3 - Road to the Final Victory</td><td>✓</td><td>  </td><td>  </td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/rbff1'>rbff1</a></td><td>Real Bout Fatal Fury</td><td>✓</td><td>✓</td><td>  </td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/rbffspec'>rbffspec</a></td><td>Real Bout Fatal Fury Special</td><td>  </td><td>✓</td><td>  </td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/rbff2'>rbff2</a></td><td>Real Bout Fatal Fury 2 - The Newcomers</td><td>  </td><td>✓</td><td>  </td><td>  </td></tr>
<tr><td><a href='http://maws.mameworld.info/maws/romset/garou'>garou</a></td><td>Garou - Mark of the Wolves</td><td>  </td><td>✓</td><td>✓</td><td>  </td></tr></tbody></table>

Corrections or suggestions can be submitted as comments to this page.