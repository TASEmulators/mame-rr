###########################################################################
#
#   tiny.mak
#
#   Small driver-specific example makefile
#	Use make SUBTARGET=tiny to build
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit  http://mamedev.org for licensing and usage restrictions.
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
# Specify all the CPU cores necessary for the
# drivers referenced in tiny.c.
#-------------------------------------------------

CPUS += Z80
CPUS += M6502
CPUS += MCS48
CPUS += MCS51
CPUS += M6800
CPUS += M6809
CPUS += M680X0
CPUS += M68000
CPUS += H6280



#-------------------------------------------------
# Specify all the sound cores necessary for the
# drivers referenced in tiny.c.
#-------------------------------------------------

SOUNDS += CUSTOM
SOUNDS += SAMPLES
SOUNDS += DAC
SOUNDS += DISCRETE
SOUNDS += AY8910
SOUNDS += YM2151
SOUNDS += ASTROCADE
SOUNDS += TMS5220
SOUNDS += OKIM6295
SOUNDS += HC55516
SOUNDS += YM3812
SOUNDS += CEM3394
SOUNDS += YM2203
SOUNDS += YM2608
SOUNDS += YM2610
SOUNDS += YM2610B



#-------------------------------------------------
# This is the list of files that are necessary
# for building all of the drivers referenced
# in tiny.c
#-------------------------------------------------

DRVLIBS = \
	$(MAMEOBJ)/tiny.o \
	$(MACHINE)/ticket.o \
	$(DRIVERS)/carpolo.o $(MACHINE)/carpolo.o $(VIDEO)/carpolo.o \
	$(DRIVERS)/circus.o $(AUDIO)/circus.o $(VIDEO)/circus.o \
	$(DRIVERS)/exidy.o $(AUDIO)/exidy.o $(VIDEO)/exidy.o \
	$(DRIVERS)/starfire.o $(VIDEO)/starfire.o \
	$(DRIVERS)/victory.o $(VIDEO)/victory.o \
	$(AUDIO)/targ.o \
	$(DRIVERS)/astrocde.o $(VIDEO)/astrocde.o \
	$(DRIVERS)/gridlee.o $(AUDIO)/gridlee.o $(VIDEO)/gridlee.o \
	$(DRIVERS)/williams.o $(MACHINE)/williams.o $(AUDIO)/williams.o $(VIDEO)/williams.o \
	$(AUDIO)/gorf.o \
	$(AUDIO)/wow.o \
	$(DRIVERS)/gaelco.o $(VIDEO)/gaelco.o $(MACHINE)/gaelcrpt.o \
	$(DRIVERS)/wrally.o $(MACHINE)/wrally.o $(VIDEO)/wrally.o \
	$(DRIVERS)/cninja.o $(VIDEO)/cninja.o \
	$(MACHINE)/deco102.o \
	$(MACHINE)/decocrpt.o \
	$(MACHINE)/decoprot.o \
	$(VIDEO)/deco16ic.o \



#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

$(DRIVERS)/astrocde.o:	$(LAYOUT)/gorf.lh \
						$(LAYOUT)/tenpindx.lh
$(DRIVERS)/circus.o:	$(LAYOUT)/circus.lh \
						$(LAYOUT)/crash.lh
