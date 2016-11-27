###########################################################################
#
#   mame.mak
#
#   MAME target makefile
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


MAMESRC = $(SRC)/mame
MAMEOBJ = $(OBJ)/mame

AUDIO = $(MAMEOBJ)/audio
DRIVERS = $(MAMEOBJ)/drivers
LAYOUT = $(MAMEOBJ)/layout
MACHINE = $(MAMEOBJ)/machine
VIDEO = $(MAMEOBJ)/video

OBJDIRS += \
	$(AUDIO) \
	$(DRIVERS) \
	$(LAYOUT) \
	$(MACHINE) \
	$(VIDEO) \



#-------------------------------------------------
# specify available CPU cores; some of these are
# only for MESS, but are included so that they get
# updated with any MAME core changes
#-------------------------------------------------

CPUS += Z80
CPUS += Z180
CPUS += I8085
CPUS += M6502
CPUS += H6280
CPUS += I86
CPUS += I386
CPUS += NEC
CPUS += V30MZ
CPUS += V60
CPUS += MCS48
CPUS += MCS51
CPUS += M6800
CPUS += M6805
CPUS += HD6309
CPUS += M6809
CPUS += KONAMI
CPUS += M680X0
CPUS += T11
CPUS += S2650
CPUS += TMS340X0
CPUS += TMS9900
CPUS += Z8000
CPUS += TMS32010
CPUS += TMS32025
CPUS += TMS32031
CPUS += TMS32051
CPUS += TMS57002
CPUS += CCPU
CPUS += ADSP21XX
CPUS += ASAP
CPUS += AM29000
CPUS += UPD7810
CPUS += ARM
CPUS += ARM7
CPUS += JAGUAR
CPUS += CUBEQCPU
CPUS += ESRIP
CPUS += MIPS
CPUS += SH2
CPUS += SH4
CPUS += DSP32C
CPUS += PIC16C5X
CPUS += PIC16C62X
CPUS += G65816
CPUS += SPC700
CPUS += E1
CPUS += I860
CPUS += I960
CPUS += H83002
CPUS += H83334
CPUS += V810
CPUS += M37710
CPUS += POWERPC
CPUS += SE3208
CPUS += MC68HC11
CPUS += ADSP21062
CPUS += DSP56156
CPUS += RSP
CPUS += ALPHA8201
CPUS += CDP1802
CPUS += COP400
CPUS += TLCS90
CPUS += TLCS900
CPUS += MB88XX
CPUS += MB86233
CPUS += SSP1601
CPUS += APEXC
CPUS += CP1610
CPUS += F8
CPUS += LH5801
CPUS += PDP1
CPUS += SATURN
CPUS += SC61860
CPUS += LR35902
CPUS += TMS7000
CPUS += SM8500
CPUS += MINX
CPUS += SSEM
CPUS += AVR8
CPUS += TMS0980
CPUS += I4004
CPUS += SUPERFX
CPUS += Z8
CPUS += I8008
CPUS += SCMP
CPUS += MN10200

#-------------------------------------------------
# specify available sound cores; some of these are
# only for MESS and so aren't included
#-------------------------------------------------

SOUNDS += CUSTOM
SOUNDS += SAMPLES
SOUNDS += DAC
SOUNDS += DMADAC
SOUNDS += SPEAKER
SOUNDS += BEEP
SOUNDS += DISCRETE
SOUNDS += AY8910
SOUNDS += YM2151
SOUNDS += YM2203
SOUNDS += YM2413
SOUNDS += YM2608
SOUNDS += YM2610
SOUNDS += YM2610B
SOUNDS += YM2612
SOUNDS += YM3438
SOUNDS += YM3812
SOUNDS += YM3526
SOUNDS += Y8950
SOUNDS += YMF262
SOUNDS += YMF271
SOUNDS += YMF278B
SOUNDS += YMZ280B
SOUNDS += SN76477
SOUNDS += SN76496
SOUNDS += POKEY
SOUNDS += TIA
SOUNDS += NES
SOUNDS += ASTROCADE
SOUNDS += NAMCO
SOUNDS += NAMCO_15XX
SOUNDS += NAMCO_CUS30
SOUNDS += NAMCO_52XX
SOUNDS += NAMCO_63701X
SOUNDS += T6W28
SOUNDS += SNKWAVE
SOUNDS += C140
SOUNDS += C352
SOUNDS += TMS36XX
SOUNDS += TMS3615
SOUNDS += TMS5110
SOUNDS += TMS5220
SOUNDS += VLM5030
SOUNDS += ADPCM
SOUNDS += MSM5205
SOUNDS += MSM5232
SOUNDS += OKIM6258
SOUNDS += OKIM6295
SOUNDS += OKIM6376
SOUNDS += UPD7759
SOUNDS += HC55516
SOUNDS += K005289
SOUNDS += K007232
SOUNDS += K051649
SOUNDS += K053260
SOUNDS += K054539
SOUNDS += K056800
SOUNDS += SEGAPCM
SOUNDS += MULTIPCM
SOUNDS += SCSP
SOUNDS += AICA
SOUNDS += RF5C68
SOUNDS += RF5C400
SOUNDS += CEM3394
SOUNDS += QSOUND
SOUNDS += SAA1099
SOUNDS += IREMGA20
SOUNDS += ES5503
SOUNDS += ES5505
SOUNDS += ES5506
SOUNDS += BSMT2000
SOUNDS += GAELCO_CG1V
SOUNDS += GAELCO_GAE1
SOUNDS += C6280
SOUNDS += SP0250
SOUNDS += PSXSPU
SOUNDS += CDDA
SOUNDS += ICS2115
SOUNDS += ST0016
SOUNDS += NILE
SOUNDS += X1_010
SOUNDS += VRENDER0
#SOUNDS += VOTRAX
SOUNDS += ES8712
SOUNDS += CDP1869
SOUNDS += S14001A
SOUNDS += WAVE
#SOUNDS += SID6581
#SOUNDS += SID8580
SOUNDS += SP0256
SOUNDS += DIGITALKER
SOUNDS += CDP1863
SOUNDS += CDP1864
SOUNDS += ZSG2
SOUNDS += MOS656X

#-------------------------------------------------
# this is the list of driver libraries that
# comprise MAME plus mamedriv.o which contains
# the list of drivers
#-------------------------------------------------

DRVLIBS = \
	$(MAMEOBJ)/mamedriv.o \
	$(MAMEOBJ)/alba.a \
	$(MAMEOBJ)/alliedl.a \
	$(MAMEOBJ)/alpha.a \
	$(MAMEOBJ)/amiga.a \
	$(MAMEOBJ)/aristocr.a \
	$(MAMEOBJ)/atari.a \
	$(MAMEOBJ)/atlus.a \
	$(MAMEOBJ)/barcrest.a \
	$(MAMEOBJ)/bfm.a \
	$(MAMEOBJ)/bmc.a \
	$(MAMEOBJ)/capcom.a \
	$(MAMEOBJ)/cinemat.a \
	$(MAMEOBJ)/comad.a \
	$(MAMEOBJ)/cvs.a \
	$(MAMEOBJ)/dataeast.a \
	$(MAMEOBJ)/dooyong.a \
	$(MAMEOBJ)/dynax.a \
	$(MAMEOBJ)/edevices.a \
	$(MAMEOBJ)/eolith.a \
	$(MAMEOBJ)/excelent.a \
	$(MAMEOBJ)/exidy.a \
	$(MAMEOBJ)/f32.a \
	$(MAMEOBJ)/fuuki.a \
	$(MAMEOBJ)/gaelco.a \
	$(MAMEOBJ)/gameplan.a \
	$(MAMEOBJ)/gametron.a \
	$(MAMEOBJ)/gottlieb.a \
	$(MAMEOBJ)/igs.a \
	$(MAMEOBJ)/irem.a \
	$(MAMEOBJ)/itech.a \
	$(MAMEOBJ)/jaleco.a \
	$(MAMEOBJ)/jpm.a \
	$(MAMEOBJ)/kaneko.a \
	$(MAMEOBJ)/konami.a \
	$(MAMEOBJ)/maygay.a \
	$(MAMEOBJ)/meadows.a \
	$(MAMEOBJ)/merit.a \
	$(MAMEOBJ)/metro.a \
	$(MAMEOBJ)/midcoin.a \
	$(MAMEOBJ)/midw8080.a \
	$(MAMEOBJ)/midway.a \
	$(MAMEOBJ)/msx.a \
	$(MAMEOBJ)/namco.a \
	$(MAMEOBJ)/nasco.a \
	$(MAMEOBJ)/neogeo.a \
	$(MAMEOBJ)/nichibut.a \
	$(MAMEOBJ)/nintendo.a \
	$(MAMEOBJ)/nix.a \
	$(MAMEOBJ)/nmk.a \
	$(MAMEOBJ)/omori.a \
	$(MAMEOBJ)/olympia.a \
	$(MAMEOBJ)/orca.a \
	$(MAMEOBJ)/pacific.a \
	$(MAMEOBJ)/pacman.a \
	$(MAMEOBJ)/pce.a \
	$(MAMEOBJ)/phoenix.a \
	$(MAMEOBJ)/playmark.a \
	$(MAMEOBJ)/psikyo.a \
	$(MAMEOBJ)/ramtek.a \
	$(MAMEOBJ)/rare.a \
	$(MAMEOBJ)/sanritsu.a \
	$(MAMEOBJ)/sega.a \
	$(MAMEOBJ)/seibu.a \
	$(MAMEOBJ)/seta.a \
	$(MAMEOBJ)/sigma.a \
	$(MAMEOBJ)/snk.a \
	$(MAMEOBJ)/stern.a \
	$(MAMEOBJ)/subsino.a \
	$(MAMEOBJ)/sun.a \
	$(MAMEOBJ)/suna.a \
	$(MAMEOBJ)/taito.a \
	$(MAMEOBJ)/tatsumi.a \
	$(MAMEOBJ)/tch.a \
	$(MAMEOBJ)/tecfri.a \
	$(MAMEOBJ)/technos.a \
	$(MAMEOBJ)/tehkan.a \
	$(MAMEOBJ)/thepit.a \
	$(MAMEOBJ)/toaplan.a \
	$(MAMEOBJ)/tong.a \
	$(MAMEOBJ)/unico.a \
	$(MAMEOBJ)/univers.a \
	$(MAMEOBJ)/upl.a \
	$(MAMEOBJ)/valadon.a \
	$(MAMEOBJ)/veltmjr.a \
	$(MAMEOBJ)/venture.a \
	$(MAMEOBJ)/vsystem.a \
	$(MAMEOBJ)/yunsung.a \
	$(MAMEOBJ)/zaccaria.a \
	$(MAMEOBJ)/misc.a \
	$(MAMEOBJ)/shared.a \



#-------------------------------------------------
# the following files are general components and
# shared across a number of drivers
#-------------------------------------------------

$(MAMEOBJ)/shared.a: \
	$(MACHINE)/nmk112.o \
	$(MACHINE)/pckeybrd.o \
	$(MACHINE)/pcshare.o \
	$(MACHINE)/segacrpt.o \
	$(MACHINE)/segacrp2.o \
	$(MACHINE)/ticket.o \
	$(VIDEO)/avgdvg.o \



#-------------------------------------------------
# manufacturer-specific groupings for drivers
#-------------------------------------------------

$(MAMEOBJ)/alba.a: \
	$(DRIVERS)/albazc.o \
	$(DRIVERS)/albazg.o \
	$(DRIVERS)/rmhaihai.o \

$(MAMEOBJ)/alliedl.a: \
	$(DRIVERS)/ace.o \
	$(DRIVERS)/clayshoo.o \

$(MAMEOBJ)/alpha.a: \
	$(DRIVERS)/alpha68k.o $(VIDEO)/alpha68k.o \
	$(DRIVERS)/champbas.o $(VIDEO)/champbas.o \
	$(DRIVERS)/equites.o $(VIDEO)/equites.o \
	$(DRIVERS)/meijinsn.o \
	$(DRIVERS)/shougi.o \

$(MAMEOBJ)/amiga.a: \
	$(DRIVERS)/alg.o \
	$(MACHINE)/amiga.o $(AUDIO)/amiga.o $(VIDEO)/amiga.o $(VIDEO)/amigaaga.o\
	$(DRIVERS)/arcadia.o \
	$(DRIVERS)/cubocd32.o $(MACHINE)/cubocd32.o \
	$(DRIVERS)/mquake.o \
	$(DRIVERS)/upscope.o \

$(MAMEOBJ)/aristocr.a: \
	$(DRIVERS)/86lions.o \
	$(DRIVERS)/caswin.o \
	$(DRIVERS)/aristmk4.o \
	$(DRIVERS)/aristmk5.o \
	$(MACHINE)/archimds.o \

$(MAMEOBJ)/atari.a: \
	$(DRIVERS)/atarigx2.o $(VIDEO)/atarigx2.o \
	$(DRIVERS)/arcadecl.o $(VIDEO)/arcadecl.o \
	$(DRIVERS)/asteroid.o $(MACHINE)/asteroid.o $(AUDIO)/asteroid.o $(AUDIO)/llander.o \
	$(DRIVERS)/atarifb.o $(MACHINE)/atarifb.o $(AUDIO)/atarifb.o $(VIDEO)/atarifb.o \
	$(DRIVERS)/atarig1.o $(VIDEO)/atarig1.o \
	$(DRIVERS)/atarig42.o $(VIDEO)/atarig42.o \
	$(DRIVERS)/atarigt.o $(VIDEO)/atarigt.o \
	$(DRIVERS)/atarisy1.o $(VIDEO)/atarisy1.o \
	$(DRIVERS)/atarisy2.o $(VIDEO)/atarisy2.o \
	$(DRIVERS)/atetris.o $(VIDEO)/atetris.o \
	$(DRIVERS)/avalnche.o $(AUDIO)/avalnche.o \
	$(DRIVERS)/badlands.o $(VIDEO)/badlands.o \
	$(DRIVERS)/batman.o $(VIDEO)/batman.o \
	$(DRIVERS)/beathead.o $(VIDEO)/beathead.o \
	$(DRIVERS)/blstroid.o $(VIDEO)/blstroid.o \
	$(DRIVERS)/boxer.o \
	$(DRIVERS)/bsktball.o $(MACHINE)/bsktball.o $(AUDIO)/bsktball.o $(VIDEO)/bsktball.o \
	$(DRIVERS)/bwidow.o \
	$(DRIVERS)/bzone.o $(AUDIO)/bzone.o \
	$(DRIVERS)/canyon.o $(AUDIO)/canyon.o $(VIDEO)/canyon.o \
	$(DRIVERS)/cball.o \
	$(DRIVERS)/ccastles.o $(VIDEO)/ccastles.o \
	$(DRIVERS)/centiped.o $(VIDEO)/centiped.o \
	$(DRIVERS)/cloak.o $(VIDEO)/cloak.o \
	$(DRIVERS)/cloud9.o $(VIDEO)/cloud9.o \
	$(DRIVERS)/cmmb.o \
	$(DRIVERS)/cojag.o $(AUDIO)/jaguar.o $(VIDEO)/jaguar.o \
	$(DRIVERS)/copsnrob.o $(MACHINE)/copsnrob.o $(VIDEO)/copsnrob.o \
	$(DRIVERS)/cyberbal.o $(AUDIO)/cyberbal.o $(VIDEO)/cyberbal.o \
	$(DRIVERS)/destroyr.o \
	$(DRIVERS)/dragrace.o $(AUDIO)/dragrace.o $(VIDEO)/dragrace.o \
	$(DRIVERS)/eprom.o $(VIDEO)/eprom.o \
	$(DRIVERS)/firefox.o \
	$(DRIVERS)/firetrk.o $(AUDIO)/firetrk.o $(VIDEO)/firetrk.o \
	$(DRIVERS)/flyball.o \
	$(DRIVERS)/foodf.o $(VIDEO)/foodf.o \
	$(DRIVERS)/gauntlet.o $(VIDEO)/gauntlet.o \
	$(DRIVERS)/harddriv.o $(MACHINE)/harddriv.o $(AUDIO)/harddriv.o $(VIDEO)/harddriv.o \
	$(DRIVERS)/irobot.o $(MACHINE)/irobot.o $(VIDEO)/irobot.o \
	$(DRIVERS)/jedi.o $(AUDIO)/jedi.o $(VIDEO)/jedi.o \
	$(DRIVERS)/klax.o $(VIDEO)/klax.o \
	$(DRIVERS)/liberatr.o $(VIDEO)/liberatr.o \
	$(DRIVERS)/mediagx.o \
	$(DRIVERS)/metalmx.o \
	$(DRIVERS)/mgolf.o \
	$(DRIVERS)/mhavoc.o $(MACHINE)/mhavoc.o \
	$(DRIVERS)/missile.o \
	$(DRIVERS)/nitedrvr.o $(MACHINE)/nitedrvr.o $(AUDIO)/nitedrvr.o $(VIDEO)/nitedrvr.o \
	$(DRIVERS)/offtwall.o $(VIDEO)/offtwall.o \
	$(DRIVERS)/orbit.o $(AUDIO)/orbit.o $(VIDEO)/orbit.o \
	$(DRIVERS)/poolshrk.o $(AUDIO)/poolshrk.o $(VIDEO)/poolshrk.o \
	$(DRIVERS)/quantum.o \
	$(DRIVERS)/rampart.o $(VIDEO)/rampart.o \
	$(DRIVERS)/relief.o $(VIDEO)/relief.o \
	$(DRIVERS)/runaway.o $(VIDEO)/runaway.o \
	$(DRIVERS)/sbrkout.o \
	$(DRIVERS)/shuuz.o $(VIDEO)/shuuz.o \
	$(DRIVERS)/skullxbo.o $(VIDEO)/skullxbo.o \
	$(DRIVERS)/skydiver.o $(AUDIO)/skydiver.o $(VIDEO)/skydiver.o \
	$(DRIVERS)/skyraid.o $(AUDIO)/skyraid.o $(VIDEO)/skyraid.o \
	$(DRIVERS)/sprint2.o $(AUDIO)/sprint2.o $(VIDEO)/sprint2.o \
	$(DRIVERS)/sprint4.o $(VIDEO)/sprint4.o $(AUDIO)/sprint4.o \
	$(DRIVERS)/sprint8.o $(AUDIO)/sprint8.o $(VIDEO)/sprint8.o \
	$(DRIVERS)/starshp1.o $(AUDIO)/starshp1.o $(VIDEO)/starshp1.o \
	$(DRIVERS)/starwars.o $(MACHINE)/starwars.o $(AUDIO)/starwars.o \
	$(DRIVERS)/subs.o $(MACHINE)/subs.o $(AUDIO)/subs.o $(VIDEO)/subs.o \
	$(DRIVERS)/tank8.o $(AUDIO)/tank8.o $(VIDEO)/tank8.o \
	$(DRIVERS)/tempest.o \
	$(DRIVERS)/thunderj.o $(VIDEO)/thunderj.o \
	$(DRIVERS)/tomcat.o \
	$(DRIVERS)/toobin.o $(VIDEO)/toobin.o \
	$(DRIVERS)/tourtabl.o $(VIDEO)/tia.o \
	$(DRIVERS)/triplhnt.o $(AUDIO)/triplhnt.o $(VIDEO)/triplhnt.o \
	$(DRIVERS)/tunhunt.o $(VIDEO)/tunhunt.o \
	$(DRIVERS)/ultratnk.o $(VIDEO)/ultratnk.o \
	$(DRIVERS)/videopin.o $(AUDIO)/videopin.o $(VIDEO)/videopin.o \
	$(DRIVERS)/vindictr.o $(VIDEO)/vindictr.o \
	$(DRIVERS)/wolfpack.o $(VIDEO)/wolfpack.o \
	$(DRIVERS)/xybots.o $(VIDEO)/xybots.o \
	$(MACHINE)/asic65.o \
	$(MACHINE)/atari_vg.o \
	$(MACHINE)/atarigen.o \
	$(MACHINE)/mathbox.o \
	$(MACHINE)/slapstic.o \
	$(AUDIO)/atarijsa.o \
	$(AUDIO)/cage.o \
	$(AUDIO)/redbaron.o \
	$(VIDEO)/atarimo.o \
	$(VIDEO)/atarirle.o \

$(MAMEOBJ)/atlus.a: \
	$(DRIVERS)/blmbycar.o $(VIDEO)/blmbycar.o \
	$(DRIVERS)/ohmygod.o $(VIDEO)/ohmygod.o \
	$(DRIVERS)/powerins.o $(VIDEO)/powerins.o \

$(MAMEOBJ)/barcrest.a: \
	$(VIDEO)/awpvid.o \
	$(MACHINE)/meters.o \
	$(DRIVERS)/mpu4.o \
	$(DRIVERS)/mpu5.o \
	$(MACHINE)/steppers.o \

$(MAMEOBJ)/bfm.a: \
	$(DRIVERS)/bfm_sc1.o \
	$(DRIVERS)/bfmsys85.o \
	$(DRIVERS)/bfcobra.o \
	$(DRIVERS)/bfm_sc2.o $(VIDEO)/bfm_adr2.o \
	$(MACHINE)/bfm_bd1.o $(VIDEO)/bfm_dm01.o \

$(MAMEOBJ)/bmc.a: \
	$(DRIVERS)/bmcbowl.o \
	$(DRIVERS)/koftball.o \

$(MAMEOBJ)/capcom.a: \
	$(DRIVERS)/1942.o $(VIDEO)/1942.o \
	$(DRIVERS)/1943.o $(VIDEO)/1943.o \
	$(DRIVERS)/bionicc.o $(VIDEO)/bionicc.o \
	$(DRIVERS)/blktiger.o $(VIDEO)/blktiger.o \
	$(DRIVERS)/cbasebal.o $(VIDEO)/cbasebal.o \
	$(DRIVERS)/commando.o $(VIDEO)/commando.o \
	$(DRIVERS)/cps1.o $(VIDEO)/cps1.o \
	$(DRIVERS)/cps2.o \
	$(DRIVERS)/cps3.o $(AUDIO)/cps3.o \
	$(DRIVERS)/egghunt.o \
	$(DRIVERS)/fcrash.o \
	$(DRIVERS)/gng.o $(VIDEO)/gng.o \
	$(DRIVERS)/gunsmoke.o $(VIDEO)/gunsmoke.o \
	$(DRIVERS)/exedexes.o $(VIDEO)/exedexes.o \
	$(DRIVERS)/higemaru.o $(VIDEO)/higemaru.o \
	$(DRIVERS)/lastduel.o $(VIDEO)/lastduel.o \
	$(DRIVERS)/lwings.o $(VIDEO)/lwings.o \
	$(DRIVERS)/mitchell.o $(VIDEO)/mitchell.o \
	$(DRIVERS)/sf.o $(VIDEO)/sf.o \
	$(DRIVERS)/sidearms.o $(VIDEO)/sidearms.o \
	$(DRIVERS)/sonson.o $(VIDEO)/sonson.o \
	$(DRIVERS)/srumbler.o $(VIDEO)/srumbler.o \
	$(DRIVERS)/vulgus.o $(VIDEO)/vulgus.o \
	$(DRIVERS)/tigeroad.o $(VIDEO)/tigeroad.o \
	$(DRIVERS)/zn.o $(MACHINE)/znsec.o \
	$(DRIVERS)/taitogn.o \
	$(MACHINE)/cps2crpt.o \
	$(MACHINE)/kabuki.o \

$(MAMEOBJ)/cinemat.a: \
	$(DRIVERS)/ataxx.o \
	$(DRIVERS)/cinemat.o $(AUDIO)/cinemat.o $(VIDEO)/cinemat.o \
	$(DRIVERS)/cchasm.o $(MACHINE)/cchasm.o $(AUDIO)/cchasm.o $(VIDEO)/cchasm.o \
	$(DRIVERS)/dlair.o \
	$(DRIVERS)/embargo.o \
	$(DRIVERS)/jack.o $(VIDEO)/jack.o \
	$(DRIVERS)/leland.o $(MACHINE)/leland.o $(AUDIO)/leland.o $(VIDEO)/leland.o \

$(MAMEOBJ)/comad.a: \
	$(DRIVERS)/funybubl.o $(VIDEO)/funybubl.o \
	$(DRIVERS)/galspnbl.o $(VIDEO)/galspnbl.o \
	$(DRIVERS)/pushman.o $(VIDEO)/pushman.o \
	$(DRIVERS)/zerozone.o $(VIDEO)/zerozone.o \

$(MAMEOBJ)/cvs.a: \
	$(DRIVERS)/cvs.o $(VIDEO)/cvs.o \
	$(DRIVERS)/quasar.o $(VIDEO)/quasar.o \

$(MAMEOBJ)/dataeast.a: \
	$(DRIVERS)/actfancr.o $(VIDEO)/actfancr.o \
	$(DRIVERS)/astrof.o $(AUDIO)/astrof.o \
	$(DRIVERS)/backfire.o \
	$(DRIVERS)/battlera.o $(VIDEO)/battlera.o \
	$(DRIVERS)/boogwing.o $(VIDEO)/boogwing.o \
	$(DRIVERS)/brkthru.o $(VIDEO)/brkthru.o \
	$(DRIVERS)/btime.o $(MACHINE)/btime.o $(VIDEO)/btime.o \
	$(DRIVERS)/chanbara.o \
	$(DRIVERS)/bwing.o $(VIDEO)/bwing.o \
	$(DRIVERS)/deshoros.o \
	$(DRIVERS)/cbuster.o $(VIDEO)/cbuster.o \
	$(DRIVERS)/cninja.o $(VIDEO)/cninja.o \
	$(DRIVERS)/cntsteer.o \
	$(DRIVERS)/compgolf.o $(VIDEO)/compgolf.o \
	$(DRIVERS)/darkseal.o $(VIDEO)/darkseal.o \
	$(DRIVERS)/dassault.o $(VIDEO)/dassault.o \
	$(DRIVERS)/dblewing.o \
	$(DRIVERS)/dec0.o $(MACHINE)/dec0.o $(VIDEO)/dec0.o \
	$(DRIVERS)/dec8.o $(VIDEO)/dec8.o \
	$(DRIVERS)/deco_ld.o \
	$(DRIVERS)/deco_mlc.o $(VIDEO)/deco_mlc.o \
	$(DRIVERS)/deco156.o $(MACHINE)/deco156.o \
	$(DRIVERS)/deco32.o $(VIDEO)/deco32.o \
	$(DRIVERS)/decocass.o $(MACHINE)/decocass.o $(VIDEO)/decocass.o \
	$(DRIVERS)/dietgo.o $(VIDEO)/dietgo.o \
	$(DRIVERS)/exprraid.o $(VIDEO)/exprraid.o \
	$(DRIVERS)/firetrap.o $(VIDEO)/firetrap.o \
	$(DRIVERS)/funkyjet.o $(VIDEO)/funkyjet.o \
	$(DRIVERS)/karnov.o $(VIDEO)/karnov.o \
	$(DRIVERS)/kchamp.o $(VIDEO)/kchamp.o \
	$(DRIVERS)/kingobox.o $(VIDEO)/kingobox.o \
	$(DRIVERS)/lemmings.o $(VIDEO)/lemmings.o \
	$(DRIVERS)/liberate.o $(VIDEO)/liberate.o \
	$(DRIVERS)/madalien.o $(AUDIO)/madalien.o $(VIDEO)/madalien.o \
	$(DRIVERS)/madmotor.o $(VIDEO)/madmotor.o \
	$(DRIVERS)/metlclsh.o $(VIDEO)/metlclsh.o \
	$(DRIVERS)/pcktgal.o $(VIDEO)/pcktgal.o \
	$(DRIVERS)/pktgaldx.o $(VIDEO)/pktgaldx.o \
	$(DRIVERS)/pokechmp.o $(VIDEO)/pokechmp.o \
	$(DRIVERS)/progolf.o \
	$(DRIVERS)/rohga.o $(VIDEO)/rohga.o \
	$(DRIVERS)/shootout.o $(VIDEO)/shootout.o \
	$(DRIVERS)/sidepckt.o $(VIDEO)/sidepckt.o \
	$(DRIVERS)/simpl156.o $(VIDEO)/simpl156.o \
	$(DRIVERS)/sshangha.o $(VIDEO)/sshangha.o \
	$(DRIVERS)/stadhero.o $(VIDEO)/stadhero.o \
	$(DRIVERS)/supbtime.o $(VIDEO)/supbtime.o \
	$(DRIVERS)/tryout.o $(VIDEO)/tryout.o \
	$(DRIVERS)/tumbleb.o $(VIDEO)/tumbleb.o \
	$(DRIVERS)/tumblep.o $(VIDEO)/tumblep.o \
	$(DRIVERS)/vaportra.o $(VIDEO)/vaportra.o \
	$(MACHINE)/deco102.o \
	$(MACHINE)/decocrpt.o \
	$(MACHINE)/decoprot.o \
	$(VIDEO)/deco16ic.o \

$(MAMEOBJ)/dooyong.a: \
	$(DRIVERS)/dooyong.o $(VIDEO)/dooyong.o \
	$(DRIVERS)/gundealr.o $(VIDEO)/gundealr.o \

$(MAMEOBJ)/dynax.a: \
	$(DRIVERS)/ddenlovr.o \
	$(DRIVERS)/dynax.o $(VIDEO)/dynax.o \
	$(DRIVERS)/hnayayoi.o $(VIDEO)/hnayayoi.o \
	$(DRIVERS)/realbrk.o $(VIDEO)/realbrk.o \
	$(DRIVERS)/royalmah.o \

$(MAMEOBJ)/edevices.a: \
	$(DRIVERS)/diverboy.o \
	$(DRIVERS)/fantland.o $(VIDEO)/fantland.o \
	$(DRIVERS)/mwarr.o \
	$(DRIVERS)/mugsmash.o $(VIDEO)/mugsmash.o \
	$(DRIVERS)/stlforce.o $(VIDEO)/stlforce.o \
	$(DRIVERS)/ppmast93.o \
	$(DRIVERS)/twins.o \
	$(DRIVERS)/pzletime.o \

$(MAMEOBJ)/eolith.a: \
	$(DRIVERS)/eolith.o $(VIDEO)/eolith.o \
	$(DRIVERS)/eolith16.o \
	$(DRIVERS)/eolithsp.o \
	$(DRIVERS)/ghosteo.o \
	$(DRIVERS)/vegaeo.o \

$(MAMEOBJ)/excelent.a: \
	$(DRIVERS)/aquarium.o $(VIDEO)/aquarium.o \
	$(DRIVERS)/d9final.o \
	$(DRIVERS)/gcpinbal.o $(VIDEO)/gcpinbal.o \
	$(DRIVERS)/vmetal.o \

$(MAMEOBJ)/exidy.a: \
	$(DRIVERS)/carpolo.o $(MACHINE)/carpolo.o $(VIDEO)/carpolo.o \
	$(DRIVERS)/circus.o $(AUDIO)/circus.o $(VIDEO)/circus.o \
	$(DRIVERS)/exidy.o $(AUDIO)/exidy.o $(VIDEO)/exidy.o \
	$(DRIVERS)/exidy440.o $(AUDIO)/exidy440.o $(VIDEO)/exidy440.o \
	$(DRIVERS)/maxaflex.o $(MACHINE)/atari.o $(VIDEO)/atari.o $(VIDEO)/antic.o $(VIDEO)/gtia.o \
	$(DRIVERS)/starfire.o $(VIDEO)/starfire.o \
	$(DRIVERS)/vertigo.o $(MACHINE)/vertigo.o $(VIDEO)/vertigo.o \
	$(DRIVERS)/victory.o $(VIDEO)/victory.o \
	$(AUDIO)/targ.o \

$(MAMEOBJ)/f32.a: \
	$(DRIVERS)/crospang.o $(VIDEO)/crospang.o \
	$(DRIVERS)/f-32.o \

$(MAMEOBJ)/fuuki.a: \
	$(DRIVERS)/fuukifg2.o $(VIDEO)/fuukifg2.o \
	$(DRIVERS)/fuukifg3.o $(VIDEO)/fuukifg3.o \

$(MAMEOBJ)/gaelco.a: \
	$(DRIVERS)/gaelco.o $(VIDEO)/gaelco.o $(MACHINE)/gaelcrpt.o \
	$(DRIVERS)/gaelco2.o $(MACHINE)/gaelco2.o $(VIDEO)/gaelco2.o \
	$(DRIVERS)/gaelco3d.o $(VIDEO)/gaelco3d.o \
	$(DRIVERS)/glass.o $(VIDEO)/glass.o \
	$(DRIVERS)/mastboy.o \
	$(DRIVERS)/splash.o $(VIDEO)/splash.o \
	$(DRIVERS)/targeth.o $(VIDEO)/targeth.o \
	$(DRIVERS)/thoop2.o $(VIDEO)/thoop2.o \
	$(DRIVERS)/xorworld.o $(VIDEO)/xorworld.o \
	$(DRIVERS)/wrally.o $(MACHINE)/wrally.o $(VIDEO)/wrally.o \
	$(DRIVERS)/tokyocop.o \

$(MAMEOBJ)/gameplan.a: \
	$(DRIVERS)/enigma2.o \
	$(DRIVERS)/gameplan.o $(VIDEO)/gameplan.o \
	$(DRIVERS)/toratora.o \

$(MAMEOBJ)/gametron.a: \
	$(DRIVERS)/gotya.o $(AUDIO)/gotya.o $(VIDEO)/gotya.o \
	$(DRIVERS)/sbugger.o $(VIDEO)/sbugger.o \
	$(DRIVERS)/gatron.o $(VIDEO)/gatron.o \

$(MAMEOBJ)/gottlieb.a: \
	$(DRIVERS)/exterm.o $(VIDEO)/exterm.o \
	$(DRIVERS)/gottlieb.o $(AUDIO)/gottlieb.o $(VIDEO)/gottlieb.o \

$(MAMEOBJ)/igs.a: \
	$(DRIVERS)/cabaret.o \
	$(DRIVERS)/ddz.o \
	$(DRIVERS)/dunhuang.o \
	$(DRIVERS)/goldstar.o $(VIDEO)/goldstar.o \
	$(DRIVERS)/jackie.o \
	$(DRIVERS)/igspoker.o \
	$(DRIVERS)/igs009.o \
	$(DRIVERS)/igs011.o \
	$(DRIVERS)/igs017.o \
	$(DRIVERS)/igs_m027.o \
	$(DRIVERS)/spoker.o \
	$(DRIVERS)/iqblock.o $(VIDEO)/iqblock.o \
	$(DRIVERS)/lordgun.o $(VIDEO)/lordgun.o \
	$(DRIVERS)/pgm.o $(VIDEO)/pgm.o \
	$(MACHINE)/pgmcrypt.o \
	$(MACHINE)/pgmprot.o \

$(MAMEOBJ)/irem.a: \
	$(DRIVERS)/m10.o $(VIDEO)/m10.o \
	$(DRIVERS)/m14.o \
	$(DRIVERS)/m52.o $(VIDEO)/m52.o \
	$(DRIVERS)/m57.o $(VIDEO)/m57.o \
	$(DRIVERS)/m58.o $(VIDEO)/m58.o \
	$(DRIVERS)/m62.o $(VIDEO)/m62.o \
	$(DRIVERS)/m63.o \
	$(DRIVERS)/m72.o $(AUDIO)/m72.o $(VIDEO)/m72.o \
	$(DRIVERS)/m90.o $(VIDEO)/m90.o \
	$(DRIVERS)/m92.o $(VIDEO)/m92.o \
	$(DRIVERS)/m107.o $(VIDEO)/m107.o \
	$(DRIVERS)/olibochu.o \
	$(DRIVERS)/redalert.o $(AUDIO)/redalert.o $(VIDEO)/redalert.o \
	$(DRIVERS)/shisen.o $(VIDEO)/shisen.o \
	$(DRIVERS)/travrusa.o $(VIDEO)/travrusa.o \
	$(DRIVERS)/vigilant.o $(VIDEO)/vigilant.o \
	$(MACHINE)/irem_cpu.o \
	$(AUDIO)/irem.o \

$(MAMEOBJ)/itech.a: \
	$(DRIVERS)/capbowl.o $(VIDEO)/capbowl.o \
	$(DRIVERS)/itech8.o $(MACHINE)/slikshot.o $(VIDEO)/itech8.o \
	$(DRIVERS)/itech32.o $(VIDEO)/itech32.o \

$(MAMEOBJ)/jaleco.a: \
	$(DRIVERS)/aeroboto.o $(VIDEO)/aeroboto.o \
	$(DRIVERS)/argus.o $(VIDEO)/argus.o \
	$(VIDEO)/jalblend.o \
	$(DRIVERS)/bestleag.o \
	$(DRIVERS)/bigstrkb.o $(VIDEO)/bigstrkb.o \
	$(DRIVERS)/blueprnt.o $(VIDEO)/blueprnt.o \
	$(DRIVERS)/bnstars.o \
	$(DRIVERS)/cischeat.o $(VIDEO)/cischeat.o \
	$(DRIVERS)/citycon.o $(VIDEO)/citycon.o \
	$(DRIVERS)/ddayjlc.o \
	$(DRIVERS)/exerion.o $(VIDEO)/exerion.o \
	$(DRIVERS)/fcombat.o $(VIDEO)/fcombat.o \
	$(DRIVERS)/ginganin.o $(VIDEO)/ginganin.o \
	$(DRIVERS)/homerun.o $(VIDEO)/homerun.o \
	$(DRIVERS)/megasys1.o $(VIDEO)/megasys1.o \
	$(DRIVERS)/momoko.o $(VIDEO)/momoko.o \
	$(DRIVERS)/ms32.o $(VIDEO)/ms32.o \
	$(DRIVERS)/psychic5.o $(VIDEO)/psychic5.o \
	$(DRIVERS)/pturn.o \
	$(DRIVERS)/skyfox.o $(VIDEO)/skyfox.o \
	$(DRIVERS)/stepstag.o \
	$(DRIVERS)/tetrisp2.o $(VIDEO)/tetrisp2.o \

$(MAMEOBJ)/jpm.a: \
	$(DRIVERS)/guab.o \
	$(DRIVERS)/jpmsys5.o \
	$(DRIVERS)/jpmimpct.o $(VIDEO)/jpmimpct.o \

$(MAMEOBJ)/kaneko.a: \
	$(DRIVERS)/airbustr.o $(VIDEO)/airbustr.o \
	$(DRIVERS)/djboy.o $(VIDEO)/djboy.o \
	$(DRIVERS)/galpanic.o $(VIDEO)/galpanic.o \
	$(DRIVERS)/galpani2.o $(VIDEO)/galpani2.o \
	$(DRIVERS)/galpani3.o \
	$(DRIVERS)/hvyunit.o \
	$(DRIVERS)/jchan.o \
	$(DRIVERS)/kaneko16.o $(MACHINE)/kaneko16.o $(VIDEO)/kaneko16.o \
	$(DRIVERS)/expro02.o \
	$(DRIVERS)/sandscrp.o \
	$(DRIVERS)/suprnova.o $(VIDEO)/suprnova.o \

$(MAMEOBJ)/konami.a: \
	$(DRIVERS)/88games.o $(VIDEO)/88games.o \
	$(DRIVERS)/ajax.o $(MACHINE)/ajax.o $(VIDEO)/ajax.o \
	$(DRIVERS)/aliens.o $(VIDEO)/aliens.o \
	$(DRIVERS)/asterix.o $(VIDEO)/asterix.o \
	$(DRIVERS)/battlnts.o $(VIDEO)/battlnts.o \
	$(DRIVERS)/bishi.o $(VIDEO)/bishi.o \
	$(DRIVERS)/bladestl.o $(VIDEO)/bladestl.o \
	$(DRIVERS)/blockhl.o $(VIDEO)/blockhl.o \
	$(DRIVERS)/bottom9.o $(VIDEO)/bottom9.o \
	$(DRIVERS)/chqflag.o $(VIDEO)/chqflag.o \
	$(DRIVERS)/circusc.o $(VIDEO)/circusc.o \
	$(DRIVERS)/combatsc.o $(VIDEO)/combatsc.o \
	$(DRIVERS)/contra.o $(VIDEO)/contra.o \
	$(DRIVERS)/crimfght.o $(VIDEO)/crimfght.o \
	$(DRIVERS)/dbz.o $(VIDEO)/dbz.o \
	$(DRIVERS)/ddribble.o $(VIDEO)/ddribble.o \
	$(DRIVERS)/djmain.o $(VIDEO)/djmain.o \
	$(DRIVERS)/fastfred.o $(VIDEO)/fastfred.o \
	$(DRIVERS)/fastlane.o $(VIDEO)/fastlane.o \
	$(DRIVERS)/finalizr.o $(VIDEO)/finalizr.o \
	$(DRIVERS)/firebeat.o \
	$(DRIVERS)/flkatck.o $(VIDEO)/flkatck.o \
	$(DRIVERS)/gberet.o $(VIDEO)/gberet.o \
	$(DRIVERS)/gbusters.o $(VIDEO)/gbusters.o \
	$(DRIVERS)/gijoe.o $(VIDEO)/gijoe.o \
	$(DRIVERS)/gradius3.o $(VIDEO)/gradius3.o \
	$(DRIVERS)/gticlub.o $(VIDEO)/gticlub.o \
	$(DRIVERS)/gyruss.o $(VIDEO)/gyruss.o \
	$(DRIVERS)/hcastle.o $(VIDEO)/hcastle.o \
	$(DRIVERS)/hexion.o $(VIDEO)/hexion.o \
	$(DRIVERS)/hornet.o $(MACHINE)/konppc.o \
	$(DRIVERS)/hyperspt.o $(VIDEO)/hyperspt.o \
	$(DRIVERS)/ironhors.o $(VIDEO)/ironhors.o \
	$(DRIVERS)/jackal.o $(VIDEO)/jackal.o \
	$(DRIVERS)/jailbrek.o $(VIDEO)/jailbrek.o \
	$(DRIVERS)/junofrst.o \
	$(DRIVERS)/konamigq.o \
	$(DRIVERS)/konamigv.o \
	$(DRIVERS)/konamigx.o $(MACHINE)/konamigx.o $(VIDEO)/konamigx.o \
	$(DRIVERS)/konamim2.o \
	$(DRIVERS)/kongambl.o \
	$(DRIVERS)/ksys573.o $(MACHINE)/zs01.o \
	$(DRIVERS)/twinkle.o \
	$(DRIVERS)/labyrunr.o $(VIDEO)/labyrunr.o \
	$(DRIVERS)/lethal.o $(VIDEO)/lethal.o \
	$(DRIVERS)/mainevt.o $(VIDEO)/mainevt.o \
	$(DRIVERS)/megazone.o $(VIDEO)/megazone.o \
	$(DRIVERS)/mikie.o $(VIDEO)/mikie.o \
	$(DRIVERS)/mogura.o \
	$(DRIVERS)/moo.o $(VIDEO)/moo.o \
	$(DRIVERS)/mystwarr.o $(VIDEO)/mystwarr.o \
	$(DRIVERS)/nemesis.o $(VIDEO)/nemesis.o \
	$(DRIVERS)/nwk-tr.o \
	$(DRIVERS)/overdriv.o $(VIDEO)/overdriv.o \
	$(DRIVERS)/pandoras.o $(VIDEO)/pandoras.o \
	$(DRIVERS)/parodius.o $(VIDEO)/parodius.o \
	$(DRIVERS)/pingpong.o $(VIDEO)/pingpong.o \
	$(DRIVERS)/plygonet.o $(VIDEO)/plygonet.o \
	$(DRIVERS)/pooyan.o $(VIDEO)/pooyan.o \
	$(DRIVERS)/qdrmfgp.o $(VIDEO)/qdrmfgp.o \
	$(DRIVERS)/rockrage.o $(VIDEO)/rockrage.o \
	$(DRIVERS)/rocnrope.o $(VIDEO)/rocnrope.o \
	$(DRIVERS)/rollerg.o $(VIDEO)/rollerg.o \
	$(DRIVERS)/rungun.o $(VIDEO)/rungun.o \
	$(DRIVERS)/sbasketb.o $(VIDEO)/sbasketb.o \
	$(DRIVERS)/scobra.o \
	$(DRIVERS)/scotrsht.o $(VIDEO)/scotrsht.o \
	$(DRIVERS)/scramble.o $(MACHINE)/scramble.o $(AUDIO)/scramble.o \
	$(DRIVERS)/shaolins.o $(VIDEO)/shaolins.o \
	$(DRIVERS)/simpsons.o $(MACHINE)/simpsons.o $(VIDEO)/simpsons.o \
	$(DRIVERS)/spy.o $(VIDEO)/spy.o \
	$(DRIVERS)/surpratk.o $(VIDEO)/surpratk.o \
	$(DRIVERS)/tgtpanic.o \
	$(DRIVERS)/thunderx.o $(VIDEO)/thunderx.o \
	$(DRIVERS)/timeplt.o $(AUDIO)/timeplt.o $(VIDEO)/timeplt.o \
	$(DRIVERS)/tmnt.o $(VIDEO)/tmnt.o \
	$(DRIVERS)/tp84.o $(VIDEO)/tp84.o \
	$(DRIVERS)/trackfld.o $(MACHINE)/konami1.o $(AUDIO)/trackfld.o $(VIDEO)/trackfld.o \
	$(DRIVERS)/tutankhm.o $(VIDEO)/tutankhm.o \
	$(DRIVERS)/twin16.o $(VIDEO)/twin16.o \
	$(DRIVERS)/ultrsprt.o \
	$(DRIVERS)/ultraman.o $(VIDEO)/ultraman.o \
	$(DRIVERS)/vendetta.o $(VIDEO)/vendetta.o \
	$(DRIVERS)/viper.o \
	$(DRIVERS)/wecleman.o $(VIDEO)/wecleman.o \
	$(DRIVERS)/xexex.o $(VIDEO)/xexex.o \
	$(DRIVERS)/xmen.o $(VIDEO)/xmen.o \
	$(DRIVERS)/yiear.o $(VIDEO)/yiear.o \
	$(DRIVERS)/zr107.o \
	$(VIDEO)/konamiic.o $(VIDEO)/konicdev.o \

$(MAMEOBJ)/maygay.a: \
	$(DRIVERS)/maygay1b.o \
	$(DRIVERS)/maygayv1.o \

$(MAMEOBJ)/meadows.a: \
	$(DRIVERS)/lazercmd.o $(VIDEO)/lazercmd.o \
	$(DRIVERS)/meadows.o $(AUDIO)/meadows.o $(VIDEO)/meadows.o \

$(MAMEOBJ)/merit.a: \
	$(DRIVERS)/mpoker.o \
	$(DRIVERS)/merit.o \
	$(DRIVERS)/meritm.o \

$(MAMEOBJ)/metro.a: \
	$(DRIVERS)/hyprduel.o $(VIDEO)/hyprduel.o \
	$(DRIVERS)/metro.o $(VIDEO)/metro.o \
	$(DRIVERS)/rabbit.o \
	$(DRIVERS)/tmmjprd.o \

$(MAMEOBJ)/midcoin.a: \
	$(DRIVERS)/wallc.o \
	$(DRIVERS)/wink.o \

$(MAMEOBJ)/midw8080.a: \
	$(DRIVERS)/8080bw.o $(AUDIO)/8080bw.o $(VIDEO)/8080bw.o \
	$(DRIVERS)/m79amb.o $(AUDIO)/m79amb.o \
	$(DRIVERS)/mw8080bw.o $(MACHINE)/mw8080bw.o $(AUDIO)/mw8080bw.o $(VIDEO)/mw8080bw.o \
	$(DRIVERS)/rotaryf.o \
	$(DRIVERS)/sspeedr.o $(VIDEO)/sspeedr.o \

$(MAMEOBJ)/midway.a: \
	$(DRIVERS)/astrocde.o $(VIDEO)/astrocde.o \
	$(DRIVERS)/balsente.o $(MACHINE)/balsente.o $(VIDEO)/balsente.o \
	$(DRIVERS)/gridlee.o $(AUDIO)/gridlee.o $(VIDEO)/gridlee.o \
	$(DRIVERS)/mcr.o $(MACHINE)/mcr.o $(AUDIO)/mcr.o $(VIDEO)/mcr.o \
	$(DRIVERS)/mcr3.o $(VIDEO)/mcr3.o \
	$(DRIVERS)/mcr68.o $(VIDEO)/mcr68.o \
	$(DRIVERS)/midtunit.o $(MACHINE)/midtunit.o $(VIDEO)/midtunit.o \
	$(DRIVERS)/midvunit.o $(VIDEO)/midvunit.o \
	$(DRIVERS)/midwunit.o $(MACHINE)/midwunit.o \
	$(DRIVERS)/midxunit.o \
	$(DRIVERS)/midyunit.o $(MACHINE)/midyunit.o $(VIDEO)/midyunit.o \
	$(DRIVERS)/midzeus.o $(VIDEO)/midzeus.o $(VIDEO)/midzeus2.o \
	$(DRIVERS)/omegrace.o \
	$(DRIVERS)/seattle.o \
	$(DRIVERS)/tmaster.o \
	$(DRIVERS)/vegas.o \
	$(DRIVERS)/williams.o $(MACHINE)/williams.o $(AUDIO)/williams.o $(VIDEO)/williams.o \
	$(MACHINE)/midwayic.o \
	$(AUDIO)/dcs.o \
	$(AUDIO)/gorf.o \
	$(AUDIO)/wow.o \

$(MAMEOBJ)/msx.a: \
	$(DRIVERS)/big10.o \
	$(DRIVERS)/forte2.o \
	$(DRIVERS)/pengadvb.o \
	$(DRIVERS)/sangho.o \
	$(DRIVERS)/sfkick.o \

$(MAMEOBJ)/namco.a: \
	$(DRIVERS)/20pacgal.o $(VIDEO)/20pacgal.o \
	$(DRIVERS)/baraduke.o $(VIDEO)/baraduke.o \
	$(DRIVERS)/dambustr.o \
	$(DRIVERS)/gal3.o \
	$(DRIVERS)/galaga.o $(AUDIO)/galaga.o $(VIDEO)/galaga.o \
	$(DRIVERS)/galaxian.o $(AUDIO)/galaxian.o $(VIDEO)/galaxian.o \
	$(DRIVERS)/galaxold.o $(MACHINE)/galaxold.o $(VIDEO)/galaxold.o \
	$(DRIVERS)/gaplus.o $(MACHINE)/gaplus.o $(VIDEO)/gaplus.o \
	$(DRIVERS)/kungfur.o \
	$(DRIVERS)/mappy.o $(VIDEO)/mappy.o \
	$(DRIVERS)/namcofl.o $(VIDEO)/namcofl.o \
	$(DRIVERS)/namcoic.o \
	$(DRIVERS)/namcona1.o $(VIDEO)/namcona1.o \
	$(DRIVERS)/namconb1.o $(VIDEO)/namconb1.o \
	$(DRIVERS)/namcond1.o $(MACHINE)/namcond1.o $(VIDEO)/ygv608.o \
	$(DRIVERS)/namcos1.o $(MACHINE)/namcos1.o $(VIDEO)/namcos1.o \
	$(DRIVERS)/namcos10.o \
	$(DRIVERS)/namcos11.o \
	$(DRIVERS)/namcos12.o \
	$(DRIVERS)/namcos2.o $(MACHINE)/namcos2.o $(VIDEO)/namcos2.o \
	$(DRIVERS)/namcos21.o $(VIDEO)/namcos21.o \
	$(DRIVERS)/namcos22.o $(VIDEO)/namcos22.o \
	$(DRIVERS)/namcos23.o \
	$(DRIVERS)/namcos86.o $(VIDEO)/namcos86.o \
	$(DRIVERS)/pacland.o $(VIDEO)/pacland.o \
	$(DRIVERS)/polepos.o $(AUDIO)/polepos.o $(VIDEO)/polepos.o \
	$(DRIVERS)/rallyx.o $(VIDEO)/rallyx.o \
	$(DRIVERS)/skykid.o $(VIDEO)/skykid.o \
	$(DRIVERS)/tankbatt.o $(VIDEO)/tankbatt.o \
	$(DRIVERS)/tceptor.o $(VIDEO)/tceptor.o \
	$(DRIVERS)/toypop.o $(VIDEO)/toypop.o \
	$(DRIVERS)/turrett.o \
	$(DRIVERS)/warpwarp.o $(AUDIO)/warpwarp.o $(VIDEO)/warpwarp.o \
	$(MACHINE)/namcoio.o \
	$(AUDIO)/geebee.o \
	$(MACHINE)/namco06.o \
	$(MACHINE)/namco50.o \
	$(MACHINE)/namco51.o \
	$(MACHINE)/namco53.o \
	$(AUDIO)/namco52.o \
	$(AUDIO)/namco54.o \
	$(AUDIO)/namcoc7x.o \
	$(VIDEO)/bosco.o \
	$(VIDEO)/digdug.o \
	$(MACHINE)/psx.o $(VIDEO)/psx.o \
	$(MACHINE)/xevious.o $(VIDEO)/xevious.o \
	$(DRIVERS)/namcops2.o \

$(MAMEOBJ)/nasco.a: \
	$(DRIVERS)/crgolf.o $(VIDEO)/crgolf.o \
	$(DRIVERS)/suprgolf.o \

$(MAMEOBJ)/neogeo.a: \
	$(DRIVERS)/neogeo.o $(VIDEO)/neogeo.o \
	$(MACHINE)/neoboot.o \
	$(MACHINE)/neocrypt.o \
	$(MACHINE)/neoprot.o \

$(MAMEOBJ)/nichibut.a: \
	$(DRIVERS)/armedf.o $(VIDEO)/armedf.o \
	$(DRIVERS)/bigfghtr.o \
	$(DRIVERS)/cclimber.o $(MACHINE)/cclimber.o $(AUDIO)/cclimber.o $(VIDEO)/cclimber.o \
	$(DRIVERS)/clshroad.o $(VIDEO)/clshroad.o \
	$(DRIVERS)/cop01.o $(VIDEO)/cop01.o \
	$(DRIVERS)/dacholer.o \
	$(DRIVERS)/galivan.o $(VIDEO)/galivan.o \
	$(DRIVERS)/gomoku.o $(AUDIO)/gomoku.o $(VIDEO)/gomoku.o \
	$(DRIVERS)/hyhoo.o $(VIDEO)/hyhoo.o \
	$(DRIVERS)/jangou.o \
	$(DRIVERS)/magmax.o $(VIDEO)/magmax.o \
	$(DRIVERS)/nbmj8688.o $(VIDEO)/nbmj8688.o \
	$(DRIVERS)/nbmj8891.o $(VIDEO)/nbmj8891.o \
	$(DRIVERS)/nbmj8900.o $(VIDEO)/nbmj8900.o \
	$(DRIVERS)/nbmj8991.o $(VIDEO)/nbmj8991.o \
	$(DRIVERS)/nbmj9195.o $(VIDEO)/nbmj9195.o \
	$(DRIVERS)/nightgal.o \
	$(DRIVERS)/niyanpai.o $(MACHINE)/m68kfmly.o $(VIDEO)/niyanpai.o \
	$(DRIVERS)/pastelg.o $(VIDEO)/pastelg.o \
	$(DRIVERS)/seicross.o $(VIDEO)/seicross.o \
	$(DRIVERS)/terracre.o $(VIDEO)/terracre.o \
	$(DRIVERS)/tubep.o $(VIDEO)/tubep.o \
	$(DRIVERS)/wiping.o $(AUDIO)/wiping.o $(VIDEO)/wiping.o \
	$(MACHINE)/nb1413m3.o \

$(MAMEOBJ)/nintendo.a: \
	$(DRIVERS)/dkong.o $(AUDIO)/dkong.o $(VIDEO)/dkong.o \
	$(DRIVERS)/cham24.o \
	$(DRIVERS)/snesb.o \
	$(DRIVERS)/mario.o $(AUDIO)/mario.o $(VIDEO)/mario.o \
	$(DRIVERS)/multigam.o \
	$(DRIVERS)/n8080.o $(AUDIO)/n8080.o $(VIDEO)/n8080.o \
	$(DRIVERS)/nss.o $(MACHINE)/snes.o $(AUDIO)/snes_snd.o $(VIDEO)/snes.o \
	$(DRIVERS)/playch10.o $(MACHINE)/playch10.o $(VIDEO)/playch10.o \
	$(DRIVERS)/popeye.o $(VIDEO)/popeye.o \
	$(DRIVERS)/punchout.o $(VIDEO)/punchout.o \
	$(DRIVERS)/sfcbox.o \
	$(DRIVERS)/spacefb.o $(AUDIO)/spacefb.o  $(VIDEO)/spacefb.o \
	$(DRIVERS)/vsnes.o $(MACHINE)/vsnes.o $(VIDEO)/vsnes.o \
	$(VIDEO)/ppu2c0x.o \

$(MAMEOBJ)/nix.a: \
	$(DRIVERS)/fitfight.o $(VIDEO)/fitfight.o \
	$(DRIVERS)/pirates.o $(VIDEO)/pirates.o \

$(MAMEOBJ)/nmk.a: \
	$(DRIVERS)/acommand.o \
	$(DRIVERS)/cultures.o \
	$(DRIVERS)/ddealer.o \
	$(DRIVERS)/jalmah.o \
	$(DRIVERS)/macrossp.o $(VIDEO)/macrossp.o \
	$(DRIVERS)/nmk16.o $(MACHINE)/nmk004.o $(VIDEO)/nmk16.o \
	$(DRIVERS)/quizdna.o $(VIDEO)/quizdna.o \
	$(DRIVERS)/quizpani.o $(VIDEO)/quizpani.o \

$(MAMEOBJ)/olympia.a: \
	$(DRIVERS)/dday.o $(VIDEO)/dday.o \
	$(DRIVERS)/monzagp.o \
	$(DRIVERS)/portrait.o $(VIDEO)/portrait.o \
	$(DRIVERS)/vega.o \

$(MAMEOBJ)/omori.a: \
	$(DRIVERS)/battlex.o $(VIDEO)/battlex.o \
	$(DRIVERS)/carjmbre.o $(VIDEO)/carjmbre.o \
	$(DRIVERS)/popper.o $(VIDEO)/popper.o \
	$(DRIVERS)/spaceg.o \

$(MAMEOBJ)/orca.a: \
	$(DRIVERS)/espial.o $(VIDEO)/espial.o \
	$(DRIVERS)/funkybee.o $(VIDEO)/funkybee.o \
	$(DRIVERS)/marineb.o $(VIDEO)/marineb.o \
	$(DRIVERS)/vastar.o $(VIDEO)/vastar.o \
	$(DRIVERS)/zodiack.o $(VIDEO)/zodiack.o \

$(MAMEOBJ)/pacific.a: \
	$(DRIVERS)/mrflea.o $(VIDEO)/mrflea.o \
	$(DRIVERS)/thief.o $(VIDEO)/thief.o \

$(MAMEOBJ)/pacman.a: \
	$(DRIVERS)/jrpacman.o \
	$(DRIVERS)/pacman.o $(VIDEO)/pacman.o \
	$(DRIVERS)/pengo.o \
	$(MACHINE)/acitya.o \
	$(MACHINE)/jumpshot.o \
	$(MACHINE)/pacplus.o \
	$(MACHINE)/theglobp.o \

$(MAMEOBJ)/pce.a: \
	$(DRIVERS)/uapce.o \
	$(DRIVERS)/paranoia.o \
	$(DRIVERS)/ggconnie.o \
	$(MACHINE)/pcecommn.o $(VIDEO)/vdc.o \

$(MAMEOBJ)/phoenix.a: \
	$(DRIVERS)/naughtyb.o $(VIDEO)/naughtyb.o \
	$(DRIVERS)/phoenix.o $(AUDIO)/phoenix.o $(VIDEO)/phoenix.o \
	$(DRIVERS)/safarir.o \
	$(AUDIO)/pleiads.o \

$(MAMEOBJ)/playmark.a: \
	$(DRIVERS)/drtomy.o \
	$(DRIVERS)/playmark.o $(VIDEO)/playmark.o \
	$(DRIVERS)/powerbal.o \
	$(DRIVERS)/sderby.o $(VIDEO)/sderby.o \
	$(DRIVERS)/sslam.o $(VIDEO)/sslam.o \

$(MAMEOBJ)/psikyo.a: \
	$(DRIVERS)/psikyo.o $(VIDEO)/psikyo.o \
	$(DRIVERS)/psikyo4.o $(VIDEO)/psikyo4.o \
	$(DRIVERS)/psikyosh.o $(VIDEO)/psikyosh.o \

$(MAMEOBJ)/ramtek.a: \
	$(DRIVERS)/hitme.o $(AUDIO)/hitme.o \
	$(DRIVERS)/starcrus.o $(VIDEO)/starcrus.o \

$(MAMEOBJ)/rare.a: \
	$(DRIVERS)/btoads.o $(VIDEO)/btoads.o \
	$(DRIVERS)/kinst.o \
	$(DRIVERS)/xtheball.o \

$(MAMEOBJ)/sanritsu.a: \
	$(DRIVERS)/appoooh.o $(VIDEO)/appoooh.o \
	$(DRIVERS)/bankp.o $(VIDEO)/bankp.o \
	$(DRIVERS)/chinsan.o \
	$(DRIVERS)/drmicro.o $(VIDEO)/drmicro.o \
	$(DRIVERS)/jantotsu.o \
	$(DRIVERS)/mayumi.o \
	$(DRIVERS)/mermaid.o $(VIDEO)/mermaid.o \
	$(DRIVERS)/mjkjidai.o $(VIDEO)/mjkjidai.o \

$(MAMEOBJ)/sega.a: \
	$(DRIVERS)/aladbl.o \
	$(DRIVERS)/angelkds.o $(VIDEO)/angelkds.o \
	$(DRIVERS)/bingoc.o \
	$(DRIVERS)/blockade.o $(AUDIO)/blockade.o $(VIDEO)/blockade.o \
	$(DRIVERS)/calorie.o \
	$(DRIVERS)/coolridr.o \
	$(DRIVERS)/deniam.o $(VIDEO)/deniam.o \
	$(DRIVERS)/dotrikun.o \
	$(VIDEO)/genesis.o \
	$(DRIVERS)/gpworld.o \
	$(DRIVERS)/hikaru.o \
	$(DRIVERS)/hshavoc.o \
	$(DRIVERS)/kopunch.o $(VIDEO)/kopunch.o \
	$(DRIVERS)/megadriv.o \
	$(DRIVERS)/megaplay.o \
	$(DRIVERS)/megatech.o \
	$(DRIVERS)/model1.o $(MACHINE)/model1.o $(VIDEO)/model1.o \
	$(DRIVERS)/model2.o $(VIDEO)/model2.o \
	$(DRIVERS)/model3.o $(VIDEO)/model3.o $(MACHINE)/model3.o \
	$(DRIVERS)/naomi.o $(MACHINE)/dc.o $(VIDEO)/dc.o $(MACHINE)/gdcrypt.o $(MACHINE)/naomibd.o $(MACHINE)/naomi.o \
	$(DRIVERS)/triforce.o $(DRIVERS)/chihiro.o \
	$(DRIVERS)/puckpkmn.o \
	$(DRIVERS)/segac2.o \
	$(DRIVERS)/segag80r.o $(MACHINE)/segag80.o $(AUDIO)/segag80r.o $(VIDEO)/segag80r.o \
	$(DRIVERS)/segag80v.o $(AUDIO)/segag80v.o $(VIDEO)/segag80v.o \
	$(DRIVERS)/segahang.o $(VIDEO)/segahang.o \
	$(DRIVERS)/segald.o \
	$(DRIVERS)/segaorun.o $(VIDEO)/segaorun.o \
	$(DRIVERS)/segas16a.o $(VIDEO)/segas16a.o \
	$(DRIVERS)/segas16b.o $(VIDEO)/segas16b.o \
	$(DRIVERS)/segas18.o $(VIDEO)/segas18.o \
	$(DRIVERS)/segas24.o $(MACHINE)/segas24.o $(VIDEO)/segas24.o \
	$(DRIVERS)/segas32.o $(MACHINE)/segas32.o $(VIDEO)/segas32.o \
	$(DRIVERS)/segamsys.o $(DRIVERS)/segae.o \
	$(DRIVERS)/segaxbd.o $(VIDEO)/segaxbd.o \
	$(DRIVERS)/segaybd.o $(VIDEO)/segaybd.o \
	$(DRIVERS)/ssf2md.o \
	$(DRIVERS)/ssgbl.o \
	$(DRIVERS)/sg1000a.o \
	$(DRIVERS)/stactics.o $(VIDEO)/stactics.o \
	$(DRIVERS)/stv.o $(DRIVERS)/stvinit.o $(MACHINE)/stvprot.o $(MACHINE)/stvcd.o \
	$(DRIVERS)/suprloco.o $(VIDEO)/suprloco.o \
	$(DRIVERS)/system1.o $(VIDEO)/system1.o \
	$(DRIVERS)/system16.o $(VIDEO)/system16.o \
	$(DRIVERS)/timetrv.o \
	$(DRIVERS)/topshoot.o \
	$(DRIVERS)/turbo.o $(AUDIO)/turbo.o $(VIDEO)/turbo.o \
	$(DRIVERS)/vicdual.o $(AUDIO)/vicdual.o $(VIDEO)/vicdual.o \
	$(DRIVERS)/zaxxon.o $(AUDIO)/zaxxon.o $(VIDEO)/zaxxon.o \
	$(MACHINE)/fd1089.o \
	$(MACHINE)/fd1094.o \
	$(MACHINE)/fddebug.o \
	$(MACHINE)/mc8123.o \
	$(MACHINE)/s16fd.o \
	$(MACHINE)/s24fd.o \
	$(MACHINE)/scudsp.o \
	$(MACHINE)/segaic16.o \
	$(AUDIO)/carnival.o \
	$(AUDIO)/depthch.o \
	$(AUDIO)/invinco.o \
	$(AUDIO)/pulsar.o \
	$(AUDIO)/segasnd.o \
	$(VIDEO)/segaic16.o \
	$(VIDEO)/sega16sp.o \
	$(VIDEO)/segaic24.o \
	$(VIDEO)/stvvdp1.o $(VIDEO)/stvvdp2.o \

$(MAMEOBJ)/seibu.a: \
	$(DRIVERS)/cshooter.o \
	$(DRIVERS)/dcon.o $(VIDEO)/dcon.o \
	$(DRIVERS)/deadang.o $(VIDEO)/deadang.o \
	$(DRIVERS)/dynduke.o $(VIDEO)/dynduke.o \
	$(DRIVERS)/kncljoe.o $(VIDEO)/kncljoe.o \
	$(DRIVERS)/mustache.o $(VIDEO)/mustache.o \
	$(DRIVERS)/panicr.o \
	$(DRIVERS)/raiden.o $(VIDEO)/raiden.o \
	$(DRIVERS)/raiden2.o $(MACHINE)/r2crypt.o \
	$(DRIVERS)/seibuspi.o $(MACHINE)/seibuspi.o $(VIDEO)/seibuspi.o \
	$(VIDEO)/sei_crtc.o \
	$(DRIVERS)/sengokmj.o \
	$(DRIVERS)/goodejan.o \
	$(DRIVERS)/stfight.o $(MACHINE)/stfight.o $(VIDEO)/stfight.o \
	$(DRIVERS)/wiz.o $(VIDEO)/wiz.o \
	$(MACHINE)/spisprit.o \
	$(AUDIO)/seibu.o \
	$(DRIVERS)/bloodbro.o $(VIDEO)/bloodbro.o \
	$(DRIVERS)/cabal.o $(VIDEO)/cabal.o \
	$(DRIVERS)/goal92.o $(VIDEO)/goal92.o \
	$(DRIVERS)/legionna.o $(VIDEO)/legionna.o \
	$(MACHINE)/seicop.o \
	$(DRIVERS)/toki.o $(VIDEO)/toki.o \
	$(DRIVERS)/feversoc.o \

$(MAMEOBJ)/seta.a: \
	$(DRIVERS)/aleck64.o $(MACHINE)/n64.o $(VIDEO)/n64.o $(VIDEO)/rdpblend.o $(VIDEO)/rdpfb.o $(VIDEO)/rdpspn16.o $(VIDEO)/rdptrect.o $(VIDEO)/rdpfrect.o $(VIDEO)/rdpfetch.o $(VIDEO)/rdptpipe.o \
	$(DRIVERS)/darkhors.o \
	$(DRIVERS)/hanaawas.o $(VIDEO)/hanaawas.o \
	$(DRIVERS)/macs.o \
	$(DRIVERS)/seta.o $(VIDEO)/seta.o \
	$(DRIVERS)/seta2.o $(VIDEO)/seta2.o \
	$(DRIVERS)/speglsht.o \
	$(DRIVERS)/speedatk.o $(VIDEO)/speedatk.o \
	$(DRIVERS)/srmp2.o $(VIDEO)/srmp2.o \
	$(DRIVERS)/srmp5.o \
	$(DRIVERS)/srmp6.o \
	$(DRIVERS)/ssv.o $(VIDEO)/ssv.o \
	$(DRIVERS)/st0016.o $(VIDEO)/st0016.o \

$(MAMEOBJ)/sigma.a: \
	$(DRIVERS)/nyny.o \
	$(DRIVERS)/r2dtank.o \
	$(DRIVERS)/sigmab52.o \
	$(DRIVERS)/sigmab98.o \
	$(DRIVERS)/spiders.o $(AUDIO)/spiders.o \
	$(DRIVERS)/sub.o \

$(MAMEOBJ)/snk.a: \
	$(DRIVERS)/bbusters.o $(VIDEO)/bbusters.o \
	$(DRIVERS)/dmndrby.o \
	$(DRIVERS)/hng64.o $(VIDEO)/hng64.o \
	$(DRIVERS)/lasso.o $(VIDEO)/lasso.o \
	$(DRIVERS)/mainsnk.o $(VIDEO)/mainsnk.o \
	$(DRIVERS)/munchmo.o $(VIDEO)/munchmo.o \
	$(DRIVERS)/prehisle.o $(VIDEO)/prehisle.o \
	$(DRIVERS)/snk6502.o $(AUDIO)/snk6502.o $(VIDEO)/snk6502.o \
	$(DRIVERS)/snk.o $(VIDEO)/snk.o \
	$(DRIVERS)/snk68.o $(VIDEO)/snk68.o \

$(MAMEOBJ)/stern.a: \
	$(DRIVERS)/astinvad.o \
	$(DRIVERS)/berzerk.o \
	$(DRIVERS)/cliffhgr.o $(AUDIO)/cliffhgr.o \
	$(DRIVERS)/mazerbla.o \
	$(DRIVERS)/supdrapo.o \

$(MAMEOBJ)/subsino.a: \
	$(DRIVERS)/bishjan.o \
	$(DRIVERS)/lastfght.o \
	$(DRIVERS)/subsino.o \

$(MAMEOBJ)/sun.a: \
	$(DRIVERS)/arabian.o $(VIDEO)/arabian.o \
	$(DRIVERS)/ikki.o $(VIDEO)/ikki.o \
	$(DRIVERS)/kangaroo.o $(VIDEO)/kangaroo.o \
	$(DRIVERS)/markham.o $(VIDEO)/markham.o \
	$(DRIVERS)/route16.o $(VIDEO)/route16.o \
	$(DRIVERS)/shanghai.o \
	$(DRIVERS)/shangha3.o $(VIDEO)/shangha3.o \
	$(DRIVERS)/strnskil.o $(VIDEO)/strnskil.o \

$(MAMEOBJ)/suna.a: \
	$(DRIVERS)/goindol.o $(VIDEO)/goindol.o \
	$(DRIVERS)/suna8.o $(AUDIO)/suna8.o $(VIDEO)/suna8.o \
	$(DRIVERS)/suna16.o $(VIDEO)/suna16.o \
	$(DRIVERS)/go2000.o \

$(MAMEOBJ)/taito.a: \
	$(DRIVERS)/40love.o $(VIDEO)/40love.o \
	$(DRIVERS)/arkanoid.o $(MACHINE)/arkanoid.o $(VIDEO)/arkanoid.o\
	$(DRIVERS)/ashnojoe.o $(VIDEO)/ashnojoe.o \
	$(DRIVERS)/asuka.o $(MACHINE)/bonzeadv.o $(VIDEO)/asuka.o \
	$(DRIVERS)/bigevglf.o $(MACHINE)/bigevglf.o $(VIDEO)/bigevglf.o \
	$(DRIVERS)/bking.o $(VIDEO)/bking.o \
	$(DRIVERS)/bublbobl.o $(MACHINE)/bublbobl.o $(VIDEO)/bublbobl.o \
	$(DRIVERS)/buggychl.o $(MACHINE)/buggychl.o $(VIDEO)/buggychl.o \
	$(DRIVERS)/cchance.o \
	$(DRIVERS)/chaknpop.o $(MACHINE)/chaknpop.o $(VIDEO)/chaknpop.o \
	$(DRIVERS)/champbwl.o \
	$(DRIVERS)/changela.o $(VIDEO)/changela.o \
	$(DRIVERS)/crbaloon.o $(VIDEO)/crbaloon.o $(AUDIO)/crbaloon.o \
	$(DRIVERS)/cyclemb.o \
	$(DRIVERS)/darius.o $(VIDEO)/darius.o \
	$(DRIVERS)/darkmist.o $(VIDEO)/darkmist.o \
	$(DRIVERS)/exzisus.o $(VIDEO)/exzisus.o \
	$(DRIVERS)/fgoal.o $(VIDEO)/fgoal.o \
	$(DRIVERS)/flstory.o $(MACHINE)/flstory.o $(VIDEO)/flstory.o \
	$(DRIVERS)/galastrm.o $(VIDEO)/galastrm.o \
	$(DRIVERS)/gladiatr.o $(VIDEO)/gladiatr.o \
	$(DRIVERS)/grchamp.o $(AUDIO)/grchamp.o $(VIDEO)/grchamp.o \
	$(DRIVERS)/groundfx.o $(VIDEO)/groundfx.o \
	$(DRIVERS)/gsword.o $(MACHINE)/tait8741.o $(VIDEO)/gsword.o \
	$(DRIVERS)/gunbustr.o $(VIDEO)/gunbustr.o \
	$(DRIVERS)/halleys.o \
	$(DRIVERS)/jollyjgr.o \
	$(DRIVERS)/ksayakyu.o $(VIDEO)/ksayakyu.o \
	$(DRIVERS)/lgp.o \
	$(DRIVERS)/lkage.o $(MACHINE)/lkage.o $(VIDEO)/lkage.o \
	$(DRIVERS)/lsasquad.o $(MACHINE)/lsasquad.o $(VIDEO)/lsasquad.o \
	$(DRIVERS)/marinedt.o \
	$(DRIVERS)/mexico86.o $(MACHINE)/mexico86.o $(VIDEO)/mexico86.o \
	$(DRIVERS)/minivadr.o \
	$(DRIVERS)/missb2.o \
	$(DRIVERS)/mlanding.o \
	$(DRIVERS)/msisaac.o $(VIDEO)/msisaac.o \
	$(DRIVERS)/ninjaw.o $(VIDEO)/ninjaw.o \
	$(DRIVERS)/nycaptor.o $(MACHINE)/nycaptor.o $(VIDEO)/nycaptor.o \
	$(DRIVERS)/opwolf.o $(MACHINE)/opwolf.o \
	$(DRIVERS)/othunder.o $(VIDEO)/othunder.o \
	$(DRIVERS)/pitnrun.o $(MACHINE)/pitnrun.o $(VIDEO)/pitnrun.o \
	$(DRIVERS)/qix.o $(MACHINE)/qix.o $(AUDIO)/qix.o $(VIDEO)/qix.o \
	$(DRIVERS)/rainbow.o $(MACHINE)/rainbow.o \
	$(DRIVERS)/rastan.o $(VIDEO)/rastan.o \
	$(DRIVERS)/retofinv.o $(MACHINE)/retofinv.o $(VIDEO)/retofinv.o \
	$(DRIVERS)/rollrace.o $(VIDEO)/rollrace.o \
	$(DRIVERS)/sbowling.o \
	$(DRIVERS)/slapshot.o $(VIDEO)/slapshot.o \
	$(DRIVERS)/ssrj.o $(VIDEO)/ssrj.o \
	$(DRIVERS)/superchs.o $(VIDEO)/superchs.o \
	$(DRIVERS)/superqix.o $(VIDEO)/superqix.o \
	$(DRIVERS)/taito_b.o $(VIDEO)/taito_b.o \
	$(DRIVERS)/taito_f2.o $(VIDEO)/taito_f2.o \
	$(DRIVERS)/taito_f3.o $(VIDEO)/taito_f3.o $(AUDIO)/taito_en.o \
	$(DRIVERS)/taito_h.o $(VIDEO)/taito_h.o \
	$(DRIVERS)/taito_l.o $(VIDEO)/taito_l.o \
	$(DRIVERS)/taito_x.o $(MACHINE)/cchip.o \
	$(DRIVERS)/taito_z.o $(VIDEO)/taito_z.o \
	$(DRIVERS)/taito_o.o $(VIDEO)/taito_o.o \
	$(DRIVERS)/taitoair.o $(VIDEO)/taitoair.o \
	$(DRIVERS)/taitojc.o $(VIDEO)/taitojc.o \
	$(DRIVERS)/taitopjc.o $\
	$(DRIVERS)/taitosj.o $(MACHINE)/taitosj.o $(VIDEO)/taitosj.o \
	$(DRIVERS)/taitotz.o \
	$(DRIVERS)/taitowlf.o \
	$(DRIVERS)/tnzs.o $(MACHINE)/tnzs.o $(VIDEO)/tnzs.o \
	$(DRIVERS)/topspeed.o $(VIDEO)/topspeed.o \
	$(DRIVERS)/tsamurai.o $(VIDEO)/tsamurai.o \
	$(DRIVERS)/undrfire.o $(VIDEO)/undrfire.o \
	$(DRIVERS)/volfied.o $(MACHINE)/volfied.o $(VIDEO)/volfied.o \
	$(DRIVERS)/warriorb.o $(VIDEO)/warriorb.o \
	$(DRIVERS)/wgp.o $(VIDEO)/wgp.o \
	$(AUDIO)/taitosnd.o \
	$(AUDIO)/t5182.o \
	$(MACHINE)/taitoio.o \
	$(VIDEO)/taitoic.o \
	$(AUDIO)/taito_zm.o \

$(MAMEOBJ)/tatsumi.a: \
	$(DRIVERS)/kingdrby.o \
	$(DRIVERS)/tx1.o $(MACHINE)/tx1.o $(AUDIO)/tx1.o $(VIDEO)/tx1.o \
	$(DRIVERS)/lockon.o $(VIDEO)/lockon.o \
	$(DRIVERS)/tatsumi.o $(MACHINE)/tatsumi.o $(VIDEO)/tatsumi.o \

$(MAMEOBJ)/tch.a: \
	$(DRIVERS)/kickgoal.o $(VIDEO)/kickgoal.o \
	$(DRIVERS)/littlerb.o \
	$(DRIVERS)/speedspn.o $(VIDEO)/speedspn.o \
	$(DRIVERS)/wheelfir.o \

$(MAMEOBJ)/tecfri.a: \
	$(DRIVERS)/holeland.o $(VIDEO)/holeland.o \
	$(DRIVERS)/sauro.o $(VIDEO)/sauro.o \
	$(DRIVERS)/speedbal.o $(VIDEO)/speedbal.o \

$(MAMEOBJ)/technos.a: \
	$(DRIVERS)/battlane.o $(VIDEO)/battlane.o \
	$(DRIVERS)/blockout.o $(VIDEO)/blockout.o \
	$(DRIVERS)/bogeyman.o $(VIDEO)/bogeyman.o \
	$(DRIVERS)/chinagat.o \
	$(DRIVERS)/ddragon.o $(VIDEO)/ddragon.o \
	$(DRIVERS)/ddragon3.o $(VIDEO)/ddragon3.o \
	$(DRIVERS)/dogfgt.o $(VIDEO)/dogfgt.o \
	$(DRIVERS)/matmania.o $(MACHINE)/maniach.o $(VIDEO)/matmania.o \
	$(DRIVERS)/mystston.o $(VIDEO)/mystston.o \
	$(DRIVERS)/renegade.o $(VIDEO)/renegade.o \
	$(DRIVERS)/scregg.o \
	$(DRIVERS)/shadfrce.o $(VIDEO)/shadfrce.o \
	$(DRIVERS)/spdodgeb.o $(VIDEO)/spdodgeb.o \
	$(DRIVERS)/ssozumo.o $(VIDEO)/ssozumo.o \
	$(DRIVERS)/tagteam.o $(VIDEO)/tagteam.o \
	$(DRIVERS)/vball.o $(VIDEO)/vball.o \
	$(DRIVERS)/wwfsstar.o $(VIDEO)/wwfsstar.o \
	$(DRIVERS)/wwfwfest.o $(VIDEO)/wwfwfest.o \
	$(DRIVERS)/xain.o $(VIDEO)/xain.o \

$(MAMEOBJ)/tehkan.a: \
	$(DRIVERS)/bombjack.o $(VIDEO)/bombjack.o \
	$(DRIVERS)/gaiden.o $(VIDEO)/gaiden.o \
	$(DRIVERS)/lvcards.o $(VIDEO)/lvcards.o \
	$(DRIVERS)/pbaction.o $(VIDEO)/pbaction.o \
	$(DRIVERS)/senjyo.o $(AUDIO)/senjyo.o $(VIDEO)/senjyo.o \
	$(DRIVERS)/solomon.o $(VIDEO)/solomon.o \
	$(DRIVERS)/spbactn.o $(VIDEO)/spbactn.o \
	$(DRIVERS)/tbowl.o $(VIDEO)/tbowl.o \
	$(DRIVERS)/tecmo.o $(VIDEO)/tecmo.o \
	$(DRIVERS)/tecmo16.o $(VIDEO)/tecmo16.o \
	$(DRIVERS)/tecmosys.o $(MACHINE)/tecmosys.o \
	$(DRIVERS)/tehkanwc.o $(VIDEO)/tehkanwc.o \
	$(DRIVERS)/wc90.o $(VIDEO)/wc90.o \
	$(DRIVERS)/wc90b.o $(VIDEO)/wc90b.o \

$(MAMEOBJ)/thepit.a: \
	$(DRIVERS)/thepit.o $(VIDEO)/thepit.o \
	$(DRIVERS)/timelimt.o $(VIDEO)/timelimt.o \

$(MAMEOBJ)/toaplan.a: \
	$(DRIVERS)/mjsister.o \
	$(DRIVERS)/slapfght.o $(MACHINE)/slapfght.o $(VIDEO)/slapfght.o \
	$(DRIVERS)/snowbros.o $(VIDEO)/kan_pand.o $(VIDEO)/kan_panb.o \
	$(DRIVERS)/toaplan1.o $(MACHINE)/toaplan1.o $(VIDEO)/toaplan1.o \
	$(DRIVERS)/toaplan2.o $(AUDIO)/toaplan2.o $(VIDEO)/toaplan2.o \
	$(DRIVERS)/twincobr.o $(MACHINE)/twincobr.o $(VIDEO)/twincobr.o \
	$(DRIVERS)/wardner.o \

$(MAMEOBJ)/tong.a: \
	$(DRIVERS)/beezer.o $(MACHINE)/beezer.o $(VIDEO)/beezer.o \

$(MAMEOBJ)/unico.a: \
	$(DRIVERS)/drgnmst.o $(VIDEO)/drgnmst.o \
	$(DRIVERS)/silkroad.o $(VIDEO)/silkroad.o \
	$(DRIVERS)/unico.o $(VIDEO)/unico.o \

$(MAMEOBJ)/univers.a: \
	$(DRIVERS)/cheekyms.o $(VIDEO)/cheekyms.o \
	$(DRIVERS)/cosmic.o $(VIDEO)/cosmic.o \
	$(DRIVERS)/docastle.o $(MACHINE)/docastle.o $(VIDEO)/docastle.o \
	$(DRIVERS)/ladybug.o $(VIDEO)/ladybug.o \
	$(DRIVERS)/mrdo.o $(VIDEO)/mrdo.o \
	$(DRIVERS)/redclash.o $(VIDEO)/redclash.o \
	$(DRIVERS)/superdq.o

$(MAMEOBJ)/upl.a: \
	$(DRIVERS)/mouser.o $(VIDEO)/mouser.o \
	$(DRIVERS)/ninjakd2.o $(VIDEO)/ninjakd2.o \
	$(DRIVERS)/nova2001.o $(VIDEO)/nova2001.o \
	$(DRIVERS)/xxmissio.o $(VIDEO)/xxmissio.o \

$(MAMEOBJ)/valadon.a: \
	$(DRIVERS)/bagman.o $(MACHINE)/bagman.o $(VIDEO)/bagman.o \
	$(DRIVERS)/tankbust.o $(VIDEO)/tankbust.o \

$(MAMEOBJ)/veltmjr.a: \
	$(DRIVERS)/cardline.o \
	$(DRIVERS)/witch.o \

$(MAMEOBJ)/venture.a: \
	$(DRIVERS)/looping.o \
	$(DRIVERS)/spcforce.o $(VIDEO)/spcforce.o \
	$(DRIVERS)/suprridr.o $(VIDEO)/suprridr.o \

$(MAMEOBJ)/vsystem.a: \
	$(DRIVERS)/aerofgt.o $(VIDEO)/aerofgt.o \
	$(DRIVERS)/crshrace.o $(VIDEO)/crshrace.o \
	$(DRIVERS)/f1gp.o $(VIDEO)/f1gp.o \
	$(DRIVERS)/fromance.o $(VIDEO)/fromance.o \
	$(DRIVERS)/fromanc2.o $(VIDEO)/fromanc2.o \
	$(DRIVERS)/gstriker.o $(VIDEO)/gstriker.o \
	$(DRIVERS)/inufuku.o $(VIDEO)/inufuku.o \
	$(DRIVERS)/ojankohs.o $(VIDEO)/ojankohs.o \
	$(DRIVERS)/pipedrm.o \
	$(DRIVERS)/rpunch.o $(VIDEO)/rpunch.o \
	$(DRIVERS)/suprslam.o $(VIDEO)/suprslam.o \
	$(DRIVERS)/tail2nos.o $(VIDEO)/tail2nos.o \
	$(DRIVERS)/taotaido.o $(VIDEO)/taotaido.o \
	$(DRIVERS)/welltris.o $(VIDEO)/welltris.o \

$(MAMEOBJ)/yunsung.a: \
	$(DRIVERS)/nmg5.o \
	$(DRIVERS)/paradise.o $(VIDEO)/paradise.o \
	$(DRIVERS)/yunsung8.o $(VIDEO)/yunsung8.o \
	$(DRIVERS)/yunsun16.o $(VIDEO)/yunsun16.o \

$(MAMEOBJ)/zaccaria.a: \
	$(DRIVERS)/galaxia.o \
	$(DRIVERS)/laserbat.o $(AUDIO)/laserbat.o \
	$(DRIVERS)/zac2650.o $(VIDEO)/zac2650.o \
	$(DRIVERS)/zaccaria.o $(VIDEO)/zaccaria.o \


#-------------------------------------------------
# remaining drivers
#-------------------------------------------------

$(MAMEOBJ)/misc.a: \
	$(DRIVERS)/39in1.o \
	$(DRIVERS)/1945kiii.o \
	$(DRIVERS)/2mindril.o \
	$(DRIVERS)/3super8.o \
	$(DRIVERS)/4enraya.o $(VIDEO)/4enraya.o \
	$(DRIVERS)/4roses.o \
	$(DRIVERS)/5clown.o \
	$(DRIVERS)/acefruit.o \
	$(DRIVERS)/adp.o \
	$(DRIVERS)/amaticmg.o \
	$(DRIVERS)/ambush.o $(VIDEO)/ambush.o \
	$(DRIVERS)/ampoker2.o $(VIDEO)/ampoker2.o \
	$(DRIVERS)/amspdwy.o $(VIDEO)/amspdwy.o \
	$(DRIVERS)/artmagic.o $(VIDEO)/artmagic.o \
	$(DRIVERS)/astrocorp.o \
	$(DRIVERS)/attckufo.o \
	$(DRIVERS)/aztarac.o $(AUDIO)/aztarac.o $(VIDEO)/aztarac.o \
	$(DRIVERS)/beaminv.o \
	$(DRIVERS)/bingor.o \
	$(DRIVERS)/blackt96.o \
	$(DRIVERS)/buster.o \
	$(DRIVERS)/calchase.o \
	$(DRIVERS)/calomega.o $(VIDEO)/calomega.o \
	$(DRIVERS)/carrera.o \
	$(DRIVERS)/cave.o $(VIDEO)/cave.o \
	$(DRIVERS)/cb2001.o \
	$(DRIVERS)/cdi.o \
	$(DRIVERS)/chsuper.o \
	$(DRIVERS)/cidelsa.o $(VIDEO)/cidelsa.o \
	$(DRIVERS)/coinmstr.o \
	$(DRIVERS)/coinmvga.o \
	$(DRIVERS)/comebaby.o \
	$(DRIVERS)/coolpool.o \
	$(DRIVERS)/crystal.o $(DRIVERS)/psattack.o $(VIDEO)/vrender0.o \
	$(DRIVERS)/cubeqst.o \
	$(DRIVERS)/cybertnk.o \
	$(DRIVERS)/dcheese.o $(VIDEO)/dcheese.o \
	$(DRIVERS)/dgpix.o \
	$(DRIVERS)/discoboy.o \
	$(DRIVERS)/dominob.o \
	$(DRIVERS)/dorachan.o \
	$(DRIVERS)/dreamwld.o \
	$(DRIVERS)/dribling.o $(VIDEO)/dribling.o \
	$(DRIVERS)/drw80pkr.o \
	$(DRIVERS)/dwarfd.o \
	$(DRIVERS)/dynadice.o \
	$(DRIVERS)/epos.o $(VIDEO)/epos.o \
	$(DRIVERS)/ertictac.o \
	$(DRIVERS)/esd16.o $(VIDEO)/esd16.o \
	$(DRIVERS)/esh.o \
	$(DRIVERS)/esripsys.o $(VIDEO)/esripsys.o \
	$(DRIVERS)/ettrivia.o \
	$(DRIVERS)/flower.o $(AUDIO)/flower.o $(VIDEO)/flower.o \
	$(DRIVERS)/fortecar.o \
	$(DRIVERS)/freekick.o $(VIDEO)/freekick.o \
	$(DRIVERS)/funworld.o $(VIDEO)/funworld.o \
	$(DRIVERS)/galaxi.o \
	$(DRIVERS)/gamecstl.o \
	$(DRIVERS)/gei.o \
	$(DRIVERS)/good.o \
	$(DRIVERS)/goldnpkr.o \
	$(DRIVERS)/gotcha.o $(VIDEO)/gotcha.o \
	$(DRIVERS)/gstream.o \
	$(DRIVERS)/gumbo.o $(VIDEO)/gumbo.o \
	$(DRIVERS)/gunpey.o \
	$(DRIVERS)/highvdeo.o \
	$(DRIVERS)/himesiki.o $(VIDEO)/himesiki.o \
	$(DRIVERS)/hitpoker.o \
	$(DRIVERS)/homedata.o $(VIDEO)/homedata.o \
	$(DRIVERS)/hotblock.o \
	$(DRIVERS)/ilpag.o \
	$(DRIVERS)/imolagp.o \
	$(DRIVERS)/intrscti.o \
	$(DRIVERS)/istellar.o \
	$(DRIVERS)/itgambl2.o \
	$(DRIVERS)/itgambl3.o \
	$(DRIVERS)/itgamble.o \
	$(DRIVERS)/jackpool.o \
	$(DRIVERS)/jokrwild.o \
	$(DRIVERS)/jongkyo.o \
	$(DRIVERS)/kingpin.o \
	$(DRIVERS)/koikoi.o \
	$(DRIVERS)/kyugo.o $(VIDEO)/kyugo.o \
	$(DRIVERS)/ladyfrog.o $(VIDEO)/ladyfrog.o \
	$(DRIVERS)/laserbas.o \
	$(DRIVERS)/lethalj.o $(VIDEO)/lethalj.o \
	$(DRIVERS)/limenko.o \
	$(DRIVERS)/ltcasino.o \
	$(DRIVERS)/lucky74.o $(VIDEO)/lucky74.o \
	$(DRIVERS)/luckgrln.o \
	$(DRIVERS)/magic10.o \
	$(DRIVERS)/magicard.o \
	$(DRIVERS)/magicfly.o \
	$(DRIVERS)/magictg.o \
	$(DRIVERS)/malzak.o $(VIDEO)/malzak.o \
	$(DRIVERS)/mcatadv.o $(VIDEO)/mcatadv.o \
	$(DRIVERS)/micro3d.o $(MACHINE)/micro3d.o $(VIDEO)/micro3d.o $(AUDIO)/micro3d.o \
	$(DRIVERS)/midas.o \
	$(DRIVERS)/mil4000.o \
	$(DRIVERS)/miniboy7.o \
	$(DRIVERS)/mirage.o \
	$(DRIVERS)/mirax.o \
	$(DRIVERS)/mole.o \
	$(DRIVERS)/mosaic.o $(VIDEO)/mosaic.o \
	$(DRIVERS)/mrjong.o $(VIDEO)/mrjong.o \
	$(DRIVERS)/multfish.o \
	$(DRIVERS)/murogem.o \
	$(DRIVERS)/murogmbl.o \
	$(DRIVERS)/neptunp2.o \
	$(DRIVERS)/news.o $(VIDEO)/news.o \
	$(DRIVERS)/norautp.o  $(AUDIO)/norautp.o \
	$(DRIVERS)/oneshot.o $(VIDEO)/oneshot.o \
	$(DRIVERS)/onetwo.o \
	$(DRIVERS)/othello.o \
	$(DRIVERS)/othldrby.o $(VIDEO)/othldrby.o \
	$(DRIVERS)/pachifev.o \
	$(DRIVERS)/pangofun.o \
	$(DRIVERS)/pasha2.o \
	$(DRIVERS)/pass.o $(VIDEO)/pass.o \
	$(DRIVERS)/pcat_dyn.o \
	$(DRIVERS)/pcat_nit.o \
	$(DRIVERS)/pcxt.o \
	$(DRIVERS)/peplus.o \
	$(DRIVERS)/photon.o $(VIDEO)/pk8000.o \
	$(DRIVERS)/photon2.o \
	$(DRIVERS)/photoply.o \
	$(DRIVERS)/pinkiri8.o \
	$(DRIVERS)/pipeline.o \
	$(DRIVERS)/pkscram.o \
	$(DRIVERS)/pntnpuzl.o \
	$(DRIVERS)/policetr.o $(VIDEO)/policetr.o \
	$(DRIVERS)/polyplay.o $(AUDIO)/polyplay.o $(VIDEO)/polyplay.o \
	$(DRIVERS)/poker72.o \
	$(DRIVERS)/poo.o \
	$(DRIVERS)/quakeat.o \
	$(DRIVERS)/queen.o \
	$(DRIVERS)/quizo.o \
	$(DRIVERS)/quizpun2.o \
	$(DRIVERS)/rbmk.o \
	$(DRIVERS)/rcorsair.o \
	$(DRIVERS)/re900.o \
	$(DRIVERS)/rgum.o \
	$(DRIVERS)/roul.o \
	$(DRIVERS)/sfbonus.o \
	$(DRIVERS)/shangkid.o $(VIDEO)/shangkid.o \
	$(DRIVERS)/skeetsht.o \
	$(DRIVERS)/skimaxx.o \
	$(DRIVERS)/skyarmy.o \
	$(DRIVERS)/skylncr.o \
	$(DRIVERS)/sliver.o \
	$(DRIVERS)/slotcarn.o \
	$(DRIVERS)/sms.o \
	$(DRIVERS)/snookr10.o $(VIDEO)/snookr10.o \
	$(DRIVERS)/sothello.o \
	$(DRIVERS)/spool99.o \
	$(DRIVERS)/sprcros2.o $(VIDEO)/sprcros2.o \
	$(DRIVERS)/ssfindo.o \
	$(DRIVERS)/ssingles.o \
	$(DRIVERS)/sstrangr.o \
	$(DRIVERS)/statriv2.o \
	$(DRIVERS)/supertnk.o \
	$(DRIVERS)/tattack.o \
	$(DRIVERS)/taxidrvr.o $(VIDEO)/taxidrvr.o \
	$(DRIVERS)/tcl.o \
	$(DRIVERS)/thayers.o \
	$(DRIVERS)/thedeep.o $(VIDEO)/thedeep.o \
	$(DRIVERS)/tiamc1.o $(VIDEO)/tiamc1.o $(AUDIO)/tiamc1.o \
	$(DRIVERS)/tickee.o \
	$(DRIVERS)/truco.o $(VIDEO)/truco.o \
	$(DRIVERS)/trucocl.o $(VIDEO)/trucocl.o \
	$(DRIVERS)/trvmadns.o \
	$(DRIVERS)/trvquest.o \
	$(DRIVERS)/ttchamp.o \
	$(DRIVERS)/tugboat.o \
	$(DRIVERS)/usgames.o $(VIDEO)/usgames.o \
	$(DRIVERS)/vamphalf.o \
	$(DRIVERS)/vcombat.o \
	$(DRIVERS)/videopkr.o \
	$(DRIVERS)/vp101.o \
	$(DRIVERS)/vpoker.o \
	$(DRIVERS)/vroulet.o \
	$(DRIVERS)/wldarrow.o \
	$(DRIVERS)/xyonix.o $(VIDEO)/xyonix.o \

#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

$(DRIVERS)/8080bw.o:	$(LAYOUT)/invrvnge.lh \
						$(LAYOUT)/shuttlei.lh

$(DRIVERS)/acefruit.o:	$(LAYOUT)/sidewndr.lh

$(DRIVERS)/ampoker2.o:	$(LAYOUT)/ampoker2.lh \
						$(LAYOUT)/sigmapkr.lh \

$(DRIVERS)/aristmk4.o:	$(LAYOUT)/aristmk4.lh \

$(DRIVERS)/astrocde.o:	$(LAYOUT)/gorf.lh \
						$(LAYOUT)/tenpindx.lh

$(DRIVERS)/atarifb.o:	$(LAYOUT)/atarifb.lh \
						$(LAYOUT)/atarifb4.lh \
						$(LAYOUT)/abaseb.lh

$(DRIVERS)/avalnche.o:	$(LAYOUT)/avalnche.lh

$(DRIVERS)/balsente.o:	$(LAYOUT)/stocker.lh

$(DRIVERS)/bfm_sc1.o:	$(LAYOUT)/bfm_sc1.lh

$(DRIVERS)/bfm_sc2.o:	$(LAYOUT)/bfm_sc2.lh \
						$(LAYOUT)/awpdmd.lh \
						$(LAYOUT)/awpvid14.lh \
						$(LAYOUT)/awpvid16.lh \
						$(LAYOUT)/drwho.lh \
						$(LAYOUT)/gldncrwn.lh \
						$(LAYOUT)/quintoon.lh \
						$(LAYOUT)/paradice.lh \
						$(LAYOUT)/pyramid.lh \
						$(LAYOUT)/pokio.lh \
						$(LAYOUT)/slots.lh \
						$(LAYOUT)/sltblgpo.lh \
						$(LAYOUT)/sltblgtk.lh

$(DRIVERS)/buggychl.o:	$(LAYOUT)/buggychl.lh

$(DRIVERS)/bzone.o:		$(LAYOUT)/bzone.lh

$(DRIVERS)/cardline.o:	$(LAYOUT)/cardline.lh

$(DRIVERS)/cdi.o:	$(LAYOUT)/cdi.lh

$(DRIVERS)/changela.o:	$(LAYOUT)/changela.lh

$(DRIVERS)/chqflag.o:	$(LAYOUT)/chqflag.lh

$(DRIVERS)/cinemat.o:	$(LAYOUT)/armora.lh \
						$(LAYOUT)/solarq.lh \
						$(LAYOUT)/starcas.lh

$(DRIVERS)/circus.o:	$(LAYOUT)/circus.lh \
						$(LAYOUT)/crash.lh

$(DRIVERS)/copsnrob.o:	$(LAYOUT)/copsnrob.lh

$(DRIVERS)/darius.o:	$(LAYOUT)/darius.lh

$(DRIVERS)/dlair.o:		$(LAYOUT)/dlair.lh

$(DRIVERS)/firebeat.o:	$(LAYOUT)/firebeat.lh

$(DRIVERS)/funworld.o:	$(LAYOUT)/jollycrd.lh \
						$(LAYOUT)/bigdeal.lh \
						$(LAYOUT)/royalcrd.lh

$(DRIVERS)/galaxi.o:	$(LAYOUT)/galaxi.lh

$(DRIVERS)/gatron.o:	$(LAYOUT)/poker41.lh \
						$(LAYOUT)/pulltabs.lh

$(DRIVERS)/goldnpkr.o:	$(LAYOUT)/goldnpkr.lh \
						$(LAYOUT)/pmpoker.lh \

$(DRIVERS)/goldstar.o:	$(LAYOUT)/lucky8.lh

$(DRIVERS)/grchamp.o:	$(LAYOUT)/grchamp.lh

$(DRIVERS)/highvdeo.o:	$(LAYOUT)/fashion.lh

$(DRIVERS)/igspoker.o:	$(LAYOUT)/igspoker.lh

$(DRIVERS)/lazercmd.o:	$(LAYOUT)/lazercmd.lh

$(DRIVERS)/luckgrln.o:	$(LAYOUT)/luckgrln.lh

$(DRIVERS)/lucky74.o:	$(LAYOUT)/lucky74.lh

$(DRIVERS)/magic10.o:	$(LAYOUT)/sgsafari.lh

$(DRIVERS)/maxaflex.o:	$(LAYOUT)/maxaflex.lh

$(DRIVERS)/mcr3.o:		$(LAYOUT)/turbotag.lh

$(DRIVERS)/mpoker.o:	$(LAYOUT)/mpoker.lh

$(DRIVERS)/mpu4.o:		$(LAYOUT)/mpu4.lh \
						$(LAYOUT)/connect4.lh \
						$(LAYOUT)/gamball.lh

$(DRIVERS)/mw8080bw.o:	$(LAYOUT)/280zzzap.lh \
						$(LAYOUT)/clowns.lh \
						$(LAYOUT)/invaders.lh \
						$(LAYOUT)/invad2ct.lh \
						$(LAYOUT)/lagunar.lh \
						$(LAYOUT)/spacwalk.lh

$(DRIVERS)/meadows.o:	$(LAYOUT)/deadeye.lh \
						$(LAYOUT)/gypsyjug.lh

$(DRIVERS)/midzeus.o:	$(LAYOUT)/crusnexo.lh

$(DRIVERS)/mil4000.o:	$(LAYOUT)/mil4000.lh

$(DRIVERS)/namcofl.o:    $(LAYOUT)/namcofl.lh

$(DRIVERS)/nbmj8688.o:	$(LAYOUT)/nbmj8688.lh

$(DRIVERS)/namcos2.o:	$(LAYOUT)/finallap.lh

$(DRIVERS)/neogeo.o:	$(LAYOUT)/neogeo.lh

$(DRIVERS)/norautp.o:	$(LAYOUT)/noraut11.lh \
						$(LAYOUT)/noraut12.lh

$(DRIVERS)/overdriv.o:	$(LAYOUT)/overdriv.lh

$(DRIVERS)/peplus.o:	$(LAYOUT)/peplus.lh \
						$(LAYOUT)/pe_schip.lh \
						$(LAYOUT)/pe_poker.lh \
						$(LAYOUT)/pe_bjack.lh \
						$(LAYOUT)/pe_keno.lh \
						$(LAYOUT)/pe_slots.lh

$(DRIVERS)/polepos.o:	$(LAYOUT)/polepos.lh \
						$(LAYOUT)/topracer.lh

$(DRIVERS)/qix.o:		$(LAYOUT)/elecyoyo.lh

$(DRIVERS)/re900.o:		$(LAYOUT)/re900.lh

$(DRIVERS)/roul.o:		$(LAYOUT)/roul.lh

$(DRIVERS)/sbrkout.o:	$(LAYOUT)/sbrkout.lh

$(DRIVERS)/sderby.o:	$(LAYOUT)/sderby.lh \
						$(LAYOUT)/spacewin.lh \
						$(LAYOUT)/pmroulet.lh \

$(DRIVERS)/segaorun.o:	$(LAYOUT)/outrun.lh

$(DRIVERS)/segas32.o:	$(LAYOUT)/radr.lh

$(DRIVERS)/segaybd.o:	$(LAYOUT)/pdrift.lh

$(DRIVERS)/snookr10.o:	$(LAYOUT)/snookr10.lh

$(DRIVERS)/sspeedr.o:	$(LAYOUT)/sspeedr.lh

$(DRIVERS)/stactics.o:	$(LAYOUT)/stactics.lh

$(DRIVERS)/stepstag.o:	$(LAYOUT)/stepstag.lh

$(DRIVERS)/sstrangr.o:	$(LAYOUT)/sstrangr.lh

$(DRIVERS)/subsino.o:	$(LAYOUT)/victor5.lh \
						$(LAYOUT)/victor21.lh \
						$(LAYOUT)/tisub.lh \
						$(LAYOUT)/stisub.lh \
						$(LAYOUT)/crsbingo.lh \
						$(LAYOUT)/sharkpy.lh \
						$(LAYOUT)/sharkpye.lh \
						$(LAYOUT)/smoto.lh

$(DRIVERS)/superchs.o:	$(LAYOUT)/superchs.lh

$(DRIVERS)/sfbonus.o:	$(LAYOUT)/pirpok2.lh

$(DRIVERS)/taito_z.o:	$(LAYOUT)/contcirc.lh \
						$(LAYOUT)/dblaxle.lh

$(DRIVERS)/tatsumi.o:	$(LAYOUT)/roundup5.lh

$(DRIVERS)/tceptor.o:	$(LAYOUT)/tceptor2.lh

$(DRIVERS)/tehkanwc.o:	$(LAYOUT)/gridiron.lh

$(DRIVERS)/tetrisp2.o:	$(LAYOUT)/rocknms.lh

$(DRIVERS)/thayers.o:	$(LAYOUT)/dlair.lh

$(DRIVERS)/topspeed.o:	$(LAYOUT)/topspeed.lh

$(DRIVERS)/turbo.o:		$(LAYOUT)/turbo.lh \
						$(LAYOUT)/subroc3d.lh \
						$(LAYOUT)/buckrog.lh

$(DRIVERS)/tx1.o:		$(LAYOUT)/buggybjr.lh \
						$(LAYOUT)/buggyboy.lh \
						$(LAYOUT)/tx1.lh

$(DRIVERS)/undrfire.o:	$(LAYOUT)/cbombers.lh

$(DRIVERS)/vicdual.o:	$(LAYOUT)/depthch.lh

$(DRIVERS)/videopin.o:	$(LAYOUT)/videopin.lh

$(DRIVERS)/videopkr.o:	$(LAYOUT)/videopkr.lh \
						$(LAYOUT)/blckjack.lh \
						$(LAYOUT)/videodad.lh \
						$(LAYOUT)/videocba.lh \
						$(LAYOUT)/babypkr.lh \
						$(LAYOUT)/babydad.lh

$(DRIVERS)/warpwarp.o:	$(LAYOUT)/geebee.lh \
						$(LAYOUT)/sos.lh

$(DRIVERS)/wecleman.o:	$(LAYOUT)/wecleman.lh

$(DRIVERS)/zac2650.o:	$(LAYOUT)/tinv2650.lh


#-------------------------------------------------
# misc dependencies
#-------------------------------------------------

$(DRIVERS)/galaxian.o:	$(MAMESRC)/drivers/galdrvr.c
$(DRIVERS)/mpu4.o:		$(MAMESRC)/drivers/mpu4drvr.c
$(DRIVERS)/neogeo.o:	$(MAMESRC)/drivers/neodrvr.c
$(MACHINE)/snes.o:		$(MAMESRC)/machine/snesdsp1.c \
				$(MAMESRC)/machine/snesdsp2.c \
				$(MAMESRC)/machine/snesdsp3.c \
				$(MAMESRC)/machine/snesdsp4.c \
				$(MAMESRC)/machine/snesobc1.c \
				$(MAMESRC)/machine/snescx4.c \
				$(MAMESRC)/machine/cx4ops.c \
				$(MAMESRC)/machine/cx4oam.c \
				$(MAMESRC)/machine/cx4fn.c \
				$(MAMESRC)/machine/cx4data.c \
				$(MAMESRC)/machine/snesrtc.c \
				$(MAMESRC)/machine/snessdd1.c \
				$(MAMESRC)/machine/snesst10.c \
				$(MAMESRC)/machine/snes7110.c \
				$(MAMESRC)/machine/snesbsx.c
$(VIDEO)/jaguar.o:		$(MAMESRC)/video/jagobj.c \
				$(MAMESRC)/video/jagblit.c
$(VIDEO)/model2.o:		$(MAMESRC)/video/model2rd.c
$(VIDEO)/model3.o:		$(MAMESRC)/video/m3raster.c
$(VIDEO)/n64.o:			$(MAMESRC)/video/rdpfiltr.c
