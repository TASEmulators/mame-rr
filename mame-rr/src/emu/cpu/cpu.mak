###########################################################################
#
#   cpu.mak
#
#   Rules for building CPU cores
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


CPUSRC = $(EMUSRC)/cpu
CPUOBJ = $(EMUOBJ)/cpu


#-------------------------------------------------
# Shared code
#-------------------------------------------------

OBJDIRS += $(CPUOBJ)
CPUOBJS += $(CPUOBJ)/vtlb.o



#-------------------------------------------------
# Dynamic recompiler objects
#-------------------------------------------------

DRCOBJ = \
	$(CPUOBJ)/drcbec.o \
	$(CPUOBJ)/drcbeut.o \
	$(CPUOBJ)/drccache.o \
	$(CPUOBJ)/drcfe.o \
	$(CPUOBJ)/drcuml.o \

DRCDEPS = \
	$(CPUSRC)/drcbeut.h \
	$(CPUSRC)/drccache.h \
	$(CPUSRC)/drcfe.h \
	$(CPUSRC)/drcuml.h \
	$(CPUSRC)/drcumlsh.h \

# fixme - need to make this work for other target architectures (PPC)

ifndef FORCE_DRC_C_BACKEND
ifeq ($(PTR64),1)

DRCOBJ += \
	$(CPUOBJ)/drcbex64.o \
	$(CPUOBJ)/x86log.o $(CPUOBJ)/i386/i386dasm.o

DRCDEPS += \
	$(CPUSRC)/x86emit.h

DEFS += -DNATIVE_DRC=drcbe_x64_be_interface

else

DRCOBJ += \
	$(CPUOBJ)/drcbex86.o \
	$(CPUOBJ)/x86log.o $(CPUOBJ)/i386/i386dasm.o

DRCDEPS += \
	$(CPUSRC)/x86emit.h

DEFS += -DNATIVE_DRC=drcbe_x86_be_interface

endif
endif


$(DRCOBJ): $(DRCDEPS)



#-------------------------------------------------
# Acorn ARM series
#-------------------------------------------------

ifneq ($(filter ARM,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/arm
CPUOBJS += $(CPUOBJ)/arm/arm.o
DASMOBJS += $(CPUOBJ)/arm/armdasm.o
endif

ifneq ($(filter ARM7,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/arm7
CPUOBJS += $(CPUOBJ)/arm7/arm7.o
DASMOBJS += $(CPUOBJ)/arm7/arm7dasm.o
endif

$(CPUOBJ)/arm/arm.o:	$(CPUSRC)/arm/arm.c \
						$(CPUSRC)/arm/arm.h

$(CPUOBJ)/arm7/arm7.o:	$(CPUSRC)/arm7/arm7.c \
						$(CPUSRC)/arm7/arm7.h \
						$(CPUSRC)/arm7/arm7exec.c \
						$(CPUSRC)/arm7/arm7core.c



#-------------------------------------------------
# Advanced Digital Chips SE3208
#-------------------------------------------------

ifneq ($(filter SE3208,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/se3208
CPUOBJS += $(CPUOBJ)/se3208/se3208.o
DASMOBJS += $(CPUOBJ)/se3208/se3208dis.o
endif

$(CPUOBJ)/se3208/se3208.o:	$(CPUSRC)/se3208/se3208.c \
							$(CPUSRC)/se3208/se3208.h



#-------------------------------------------------
# Alpha 8201
#-------------------------------------------------

ifneq ($(filter ALPHA8201,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/alph8201
CPUOBJS += $(CPUOBJ)/alph8201/alph8201.o
DASMOBJS += $(CPUOBJ)/alph8201/8201dasm.o
endif

$(CPUOBJ)/alph8201/alph8201.o:	$(CPUSRC)/alph8201/alph8201.c \
								$(CPUSRC)/alph8201/alph8201.h



#-------------------------------------------------
# Analog Devices ADSP21xx series
#-------------------------------------------------

ifneq ($(filter ADSP21XX,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/adsp2100
CPUOBJS += $(CPUOBJ)/adsp2100/adsp2100.o
DASMOBJS += $(CPUOBJ)/adsp2100/2100dasm.o
endif

$(CPUOBJ)/adsp2100/adsp2100.o:	$(CPUSRC)/adsp2100/adsp2100.c \
								$(CPUSRC)/adsp2100/adsp2100.h \
								$(CPUSRC)/adsp2100/2100ops.c



#-------------------------------------------------
# Analog Devices "Sharc" ADSP21062
#-------------------------------------------------

ifneq ($(filter ADSP21062,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/sharc
CPUOBJS += $(CPUOBJ)/sharc/sharc.o
DASMOBJS += $(CPUOBJ)/sharc/sharcdsm.o
endif

$(CPUOBJ)/sharc/sharc.o:	$(CPUSRC)/sharc/sharc.c \
							$(CPUSRC)/sharc/sharc.h \
							$(CPUSRC)/sharc/sharcops.c \
							$(CPUSRC)/sharc/sharcops.h \
							$(CPUSRC)/sharc/sharcdsm.c \
							$(CPUSRC)/sharc/sharcdsm.h \
							$(CPUSRC)/sharc/compute.c \
							$(CPUSRC)/sharc/sharcdma.c \
							$(CPUSRC)/sharc/sharcmem.c



#-------------------------------------------------
# APEXC
#-------------------------------------------------

ifneq ($(filter APEXC,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/apexc
CPUOBJS += $(CPUOBJ)/apexc/apexc.o
DASMOBJS += $(CPUOBJ)/apexc/apexcdsm.o
endif

$(CPUOBJ)/apexc/apexc.o:	$(CPUSRC)/apexc/apexc.c \
							$(CPUSRC)/apexc/apexc.h



#-------------------------------------------------
# AT&T DSP32C
#-------------------------------------------------

ifneq ($(filter DSP32C,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/dsp32
CPUOBJS += $(CPUOBJ)/dsp32/dsp32.o
DASMOBJS += $(CPUOBJ)/dsp32/dsp32dis.o
endif

$(CPUOBJ)/dsp32/dsp32.o:	$(CPUSRC)/dsp32/dsp32.c \
							$(CPUSRC)/dsp32/dsp32.h \
							$(CPUSRC)/dsp32/dsp32ops.c



#-------------------------------------------------
# Atari custom RISC processor
#-------------------------------------------------

ifneq ($(filter ASAP,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/asap
CPUOBJS += $(CPUOBJ)/asap/asap.o
DASMOBJS += $(CPUOBJ)/asap/asapdasm.o
endif

$(CPUOBJ)/asap/asap.o:	$(CPUSRC)/asap/asap.c \
						$(CPUSRC)/asap/asap.h



#-------------------------------------------------
# AMD Am29000
#-------------------------------------------------

ifneq ($(filter AM29000,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/am29000
CPUOBJS += $(CPUOBJ)/am29000/am29000.o
DASMOBJS += $(CPUOBJ)/am29000/am29dasm.o
endif

$(CPUOBJ)/am29000/am29000.o:	$(CPUSRC)/am29000/am29000.c \
								$(CPUSRC)/am29000/am29000.h \
								$(CPUSRC)/am29000/am29ops.h \



#-------------------------------------------------
# Atari Jaguar custom DSPs
#-------------------------------------------------

ifneq ($(filter JAGUAR,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/jaguar
CPUOBJS += $(CPUOBJ)/jaguar/jaguar.o
DASMOBJS += $(CPUOBJ)/jaguar/jagdasm.o
endif

$(CPUOBJ)/jaguar/jaguar.o:	$(CPUSRC)/jaguar/jaguar.c \
							$(CPUSRC)/jaguar/jaguar.h



#-------------------------------------------------
# Simutrek Cube Quest bit-sliced CPUs
#-------------------------------------------------

ifneq ($(filter CUBEQCPU,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/cubeqcpu
CPUOBJS += $(CPUOBJ)/cubeqcpu/cubeqcpu.o
DASMOBJS += $(CPUOBJ)/cubeqcpu/cubedasm.o
endif

$(CPUOBJ)/cubeqcpu/cubeqcpu.o:	$(CPUSRC)/cubeqcpu/cubeqcpu.c \
								$(CPUSRC)/cubeqcpu/cubeqcpu.h



#-------------------------------------------------
# Entertainment Sciences AM29116-based RIP
#-------------------------------------------------

ifneq ($(filter ESRIP,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/esrip
CPUOBJS += $(CPUOBJ)/esrip/esrip.o
DASMOBJS += $(CPUOBJ)/esrip/esripdsm.o
endif

$(CPUOBJ)/esrip/esrip.o:	$(CPUSRC)/esrip/esrip.c \
							$(CPUSRC)/esrip/esrip.h



#-------------------------------------------------
# RCA CDP1802
#-------------------------------------------------

ifneq ($(filter CDP1802,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/cdp1802
CPUOBJS += $(CPUOBJ)/cdp1802/cdp1802.o
DASMOBJS += $(CPUOBJ)/cdp1802/1802dasm.o
endif

$(CPUOBJ)/cdp1802/cdp1802.o:	$(CPUSRC)/cdp1802/cdp1802.c \
								$(CPUSRC)/cdp1802/cdp1802.h



#-------------------------------------------------
# National Semiconductor COP400 family
#-------------------------------------------------

ifneq ($(filter COP400,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/cop400
CPUOBJS += $(CPUOBJ)/cop400/cop400.o
DASMOBJS += $(CPUOBJ)/cop400/cop410ds.o
DASMOBJS += $(CPUOBJ)/cop400/cop420ds.o
DASMOBJS += $(CPUOBJ)/cop400/cop440ds.o
endif

$(CPUOBJ)/cop400/cop400.o:	$(CPUSRC)/cop400/cop400.c \
							$(CPUSRC)/cop400/cop400.h \
							$(CPUSRC)/cop400/cop400op.c



#-------------------------------------------------
# CP1610
#-------------------------------------------------

ifneq ($(filter CP1610,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/cp1610
CPUOBJS += $(CPUOBJ)/cp1610/cp1610.o
DASMOBJS += $(CPUOBJ)/cp1610/1610dasm.o
endif

$(CPUOBJ)/cp1610/cp1610.o:	$(CPUSRC)/cp1610/cp1610.c \
							$(CPUSRC)/cp1610/cp1610.h



#-------------------------------------------------
# Cinematronics vector "CPU"
#-------------------------------------------------

ifneq ($(filter CCPU,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/ccpu
CPUOBJS += $(CPUOBJ)/ccpu/ccpu.o
DASMOBJS += $(CPUOBJ)/ccpu/ccpudasm.o
endif

$(CPUOBJ)/ccpu/ccpu.o:	$(CPUSRC)/ccpu/ccpu.c \
						$(CPUSRC)/ccpu/ccpu.h



#-------------------------------------------------
# DEC T-11
#-------------------------------------------------

ifneq ($(filter T11,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/t11
CPUOBJS += $(CPUOBJ)/t11/t11.o
DASMOBJS += $(CPUOBJ)/t11/t11dasm.o
endif

$(CPUOBJ)/t11/t11.o:	$(CPUSRC)/t11/t11.c \
						$(CPUSRC)/t11/t11.h \
						$(CPUSRC)/t11/t11ops.c \
						$(CPUSRC)/t11/t11table.c



#-------------------------------------------------
# F8
#-------------------------------------------------

ifneq ($(filter F8,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/f8
CPUOBJS += $(CPUOBJ)/f8/f8.o
DASMOBJS += $(CPUOBJ)/f8/f8dasm.o
endif

$(CPUOBJ)/f8/f8.o:	$(CPUSRC)/f8/f8.c \
					$(CPUSRC)/f8/f8.h



#-------------------------------------------------
# G65816
#-------------------------------------------------

ifneq ($(filter G65816,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/g65816
CPUOBJS += \
	$(CPUOBJ)/g65816/g65816.o \
	$(CPUOBJ)/g65816/g65816o0.o \
	$(CPUOBJ)/g65816/g65816o1.o \
	$(CPUOBJ)/g65816/g65816o2.o \
	$(CPUOBJ)/g65816/g65816o3.o \
	$(CPUOBJ)/g65816/g65816o4.o
DASMOBJS += $(CPUOBJ)/g65816/g65816ds.o
endif

G65816DEPS = \
	$(CPUSRC)/g65816/g65816.h \
	$(CPUSRC)/g65816/g65816cm.h \
	$(CPUSRC)/g65816/g65816op.h

$(CPUOBJ)/g65816/g65816.o:		$(CPUSRC)/g65816/g65816.c \
								$(G65816DEPS)

$(CPUOBJ)/g65816/g65816o0.o:	$(CPUSRC)/g65816/g65816o0.c \
								$(G65816DEPS)

$(CPUOBJ)/g65816/g65816o1.o:	$(CPUSRC)/g65816/g65816o1.c \
								$(G65816DEPS)

$(CPUOBJ)/g65816/g65816o2.o:	$(CPUSRC)/g65816/g65816o2.c \
								$(G65816DEPS)

$(CPUOBJ)/g65816/g65816o3.o:	$(CPUSRC)/g65816/g65816o3.c \
								$(G65816DEPS)

$(CPUOBJ)/g65816/g65816o4.o:	$(CPUSRC)/g65816/g65816o4.c \
								$(G65816DEPS)



#-------------------------------------------------
# Hitachi 6309
#-------------------------------------------------

ifneq ($(filter HD6309,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/hd6309
CPUOBJS += $(CPUOBJ)/hd6309/hd6309.o
DASMOBJS += $(CPUOBJ)/hd6309/6309dasm.o
endif

$(CPUOBJ)/hd6309/hd6309.o:	$(CPUSRC)/hd6309/hd6309.c \
							$(CPUSRC)/hd6309/hd6309.h \
							$(CPUSRC)/hd6309/6309ops.c \
							$(CPUSRC)/hd6309/6309tbl.c


#-------------------------------------------------
# Hitachi H8/30xx (16/32-bit H8/3xx series)
#-------------------------------------------------

ifneq ($(filter H83002,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/h83002
CPUOBJS += $(CPUOBJ)/h83002/h8_16.o $(CPUOBJ)/h83002/h8periph.o
DASMOBJS += $(CPUOBJ)/h83002/h8disasm.o
endif

$(CPUOBJ)/h83002/h8_16.o:		$(CPUSRC)/h83002/h8_16.c \
								$(CPUSRC)/h83002/h8.h \
								$(CPUSRC)/h83002/h8ops.h \
								$(CPUSRC)/h83002/h8priv.h

$(CPUOBJ)/h83002/h8disasm.o:	$(CPUSRC)/h83002/h8disasm.c

$(CPUOBJ)/h83002/h8periph.o:	$(CPUSRC)/h83002/h8periph.c \
								$(CPUSRC)/h83002/h8priv.h


#-------------------------------------------------
# Hitachi H8/3334 (8/16-bit H8/3xx series)
#-------------------------------------------------

ifneq ($(filter H83334,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/h83002
CPUOBJS += $(CPUOBJ)/h83002/h8_8.o $(CPUOBJ)/h83002/h8periph.o
DASMOBJS += $(CPUOBJ)/h83002/h8disasm.o
endif

$(CPUOBJ)/h83002/h8_8.o:		$(CPUSRC)/h83002/h8_8.c \
								$(CPUSRC)/h83002/h8.h \
								$(CPUSRC)/h83002/h8ops.h \
								$(CPUSRC)/h83002/h8priv.h

$(CPUOBJ)/h83002/h8disasm.o:	$(CPUSRC)/h83002/h8disasm.c

$(CPUOBJ)/h83002/h8periph.o:	$(CPUSRC)/h83002/h8periph.c \
								$(CPUSRC)/h83002/h8priv.h

#-------------------------------------------------
# Hitachi SH1/SH2
#-------------------------------------------------

ifneq ($(filter SH2,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/sh2
CPUOBJS += $(CPUOBJ)/sh2/sh2.o $(CPUOBJ)/sh2/sh2comn.o $(CPUOBJ)/sh2/sh2drc.o $(CPUOBJ)/sh2/sh2fe.o $(DRCOBJ)
DASMOBJS += $(CPUOBJ)/sh2/sh2dasm.o
endif

$(CPUOBJ)/sh2/sh2.o:	$(CPUSRC)/sh2/sh2.c \
			$(CPUSRC)/sh2/sh2.h \
			$(CPUSRC)/sh2/sh2comn.h

$(CPUOBJ)/sh2/sh2comn.o:  $(CPUSRC)/sh2/sh2comn.c \
			$(CPUSRC)/sh2/sh2comn.h \
			$(CPUSRC)/sh2/sh2.h

$(CPUOBJ)/sh2/sh2drc.o:	$(CPUSRC)/sh2/sh2drc.c \
			$(CPUSRC)/sh2/sh2.h \
			$(CPUSRC)/sh2/sh2comn.h

$(CPUOBJ)/sh2/sh2fe.o:	$(CPUSRC)/sh2/sh2fe.c \
			$(CPUSRC)/sh2/sh2.h \
			$(CPUSRC)/sh2/sh2comn.h

#-------------------------------------------------
# Hitachi SH4
#-------------------------------------------------

ifneq ($(filter SH4,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/sh4
CPUOBJS += $(CPUOBJ)/sh4/sh4.o $(CPUOBJ)/sh4/sh4comn.o
DASMOBJS += $(CPUOBJ)/sh4/sh4dasm.o
endif

$(CPUOBJ)/sh4/sh4.o:	$(CPUSRC)/sh4/sh4.c \
			$(CPUSRC)/sh4/sh4.h \
			$(CPUSRC)/sh4/sh4regs.h \
			$(CPUSRC)/sh4/sh4comn.h

$(CPUOBJ)/sh4/sh4comn.o:  $(CPUSRC)/sh4/sh4comn.c \
			$(CPUSRC)/sh4/sh4comn.h \
			$(CPUSRC)/sh4/sh4regs.h \
			$(CPUSRC)/sh4/sh4.h

#-------------------------------------------------
# Hudsonsoft 6280
#-------------------------------------------------

ifneq ($(filter H6280,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/h6280
CPUOBJS += $(CPUOBJ)/h6280/h6280.o
DASMOBJS += $(CPUOBJ)/h6280/6280dasm.o
endif

$(CPUOBJ)/h6280/h6280.o:	$(CPUSRC)/h6280/h6280.c \
							$(CPUSRC)/h6280/h6280.h \
							$(CPUSRC)/h6280/h6280ops.h \
							$(CPUSRC)/h6280/tblh6280.c



#-------------------------------------------------
# Hyperstone E1 series
#-------------------------------------------------

ifneq ($(filter E1,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/e132xs
CPUOBJS += $(CPUOBJ)/e132xs/e132xs.o
DASMOBJS += $(CPUOBJ)/e132xs/32xsdasm.o
endif

$(CPUOBJ)/e132xs/e132xs.o:	$(CPUSRC)/e132xs/e132xs.c \
							$(CPUSRC)/e132xs/e132xs.h \
							$(CPUSRC)/e132xs/e132xsop.c



#-------------------------------------------------
# Intel 4004
#-------------------------------------------------

ifneq ($(filter I4004,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i4004
CPUOBJS += $(CPUOBJ)/i4004/i4004.o
DASMOBJS += $(CPUOBJ)/i4004/4004dasm.o
endif

$(CPUOBJ)/i4004/i4004.o:	$(CPUSRC)/i4004/i4004.c \
							$(CPUSRC)/i4004/i4004.h


#-------------------------------------------------
# Intel 8008
#-------------------------------------------------

ifneq ($(filter I8008,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i8008
CPUOBJS += $(CPUOBJ)/i8008/i8008.o
DASMOBJS += $(CPUOBJ)/i8008/8008dasm.o
endif

$(CPUOBJ)/i8008/i8008.o:	$(CPUSRC)/i8008/i8008.c \
							$(CPUSRC)/i8008/i8008.h

#-------------------------------------------------
#  National Semiconductor SC/MP
#-------------------------------------------------

ifneq ($(filter SCMP,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/scmp
CPUOBJS += $(CPUOBJ)/scmp/scmp.o
DASMOBJS += $(CPUOBJ)/scmp/scmpdasm.o
endif

$(CPUOBJ)/scmp/scmp.o:		$(CPUSRC)/scmp/scmp.c \
							$(CPUSRC)/scmp/scmp.h


#-------------------------------------------------
# Intel 8080/8085A
#-------------------------------------------------

ifneq ($(filter I8085,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i8085
CPUOBJS += $(CPUOBJ)/i8085/i8085.o
DASMOBJS += $(CPUOBJ)/i8085/8085dasm.o
endif

$(CPUOBJ)/i8085/i8085.o:	$(CPUSRC)/i8085/i8085.c \
							$(CPUSRC)/i8085/i8085.h \
							$(CPUSRC)/i8085/i8085cpu.h



#-------------------------------------------------
# Intel MCS-48 (8039 and derivatives)
#-------------------------------------------------

ifneq ($(filter MCS48,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/mcs48
CPUOBJS += $(CPUOBJ)/mcs48/mcs48.o
DASMOBJS += $(CPUOBJ)/mcs48/mcs48dsm.o
endif

$(CPUOBJ)/mcs48/mcs48.o:	$(CPUSRC)/mcs48/mcs48.c \
							$(CPUSRC)/mcs48/mcs48.h



#-------------------------------------------------
# Intel 8051 and derivatives
#-------------------------------------------------

ifneq ($(filter MCS51,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/mcs51
CPUOBJS += $(CPUOBJ)/mcs51/mcs51.o
DASMOBJS += $(CPUOBJ)/mcs51/mcs51dasm.o
endif

$(CPUOBJ)/mcs51/mcs51.o:	$(CPUSRC)/mcs51/mcs51.c \
							$(CPUSRC)/mcs51/mcs51.h \
							$(CPUSRC)/mcs51/mcs51ops.c

#-------------------------------------------------
# Intel 80x86 series
#-------------------------------------------------

ifneq ($(filter I86,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i86 $(CPUOBJ)/i386
CPUOBJS += $(CPUOBJ)/i86/i86.o
CPUOBJS += $(CPUOBJ)/i86/i286.o
DASMOBJS += $(CPUOBJ)/i386/i386dasm.o
endif

ifneq ($(filter I386,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i386
CPUOBJS += $(CPUOBJ)/i386/i386.o
DASMOBJS += $(CPUOBJ)/i386/i386dasm.o
endif

I86DEPS = \
	$(CPUSRC)/i86/i86priv.h \
	$(CPUSRC)/i86/ea.h \
	$(CPUSRC)/i86/host.h \
	$(CPUSRC)/i86/modrm.h

$(CPUOBJ)/i86/i86.o:	$(CPUSRC)/i86/i86.c \
						$(CPUSRC)/i86/i86.h \
						$(CPUSRC)/i86/i86mem.c \
						$(CPUSRC)/i86/i86time.c \
						$(CPUSRC)/i86/instr86.c \
						$(CPUSRC)/i86/instr186.c \
						$(I86DEPS)

$(CPUOBJ)/i86/i286.o:	$(CPUSRC)/i86/i286.c \
						$(CPUSRC)/i86/i286.h \
						$(CPUSRC)/i86/i86mem.c \
						$(CPUSRC)/i86/i86time.c \
						$(CPUSRC)/i86/instr86.c \
						$(CPUSRC)/i86/instr186.c \
						$(CPUSRC)/i86/instr286.c \
						$(I86DEPS)

$(CPUOBJ)/i386/i386.o:	$(CPUSRC)/i386/i386.c \
						$(CPUSRC)/i386/i386.h \
						$(CPUSRC)/i386/i386priv.h \
						$(CPUSRC)/i386/i386op16.c \
						$(CPUSRC)/i386/i386op32.c \
						$(CPUSRC)/i386/i386ops.c \
						$(CPUSRC)/i386/i486ops.c \
						$(CPUSRC)/i386/pentops.c \
						$(CPUSRC)/i386/x87ops.c \
						$(CPUSRC)/i386/i386ops.h \
						$(CPUSRC)/i386/cycles.h



#-------------------------------------------------
# Intel i860
#-------------------------------------------------

ifneq ($(filter I860,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i860
CPUOBJS += $(CPUOBJ)/i860/i860.o
DASMOBJS += $(CPUOBJ)/i860/i860dis.o
endif

$(CPUOBJ)/i860/i860.o:  $(CPUSRC)/i860/i860.c \
                                               $(CPUSRC)/i860/i860.h \
                                               $(CPUSRC)/i860/i860dec.c

#-------------------------------------------------
# Intel i960
#-------------------------------------------------

ifneq ($(filter I960,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/i960
CPUOBJS += $(CPUOBJ)/i960/i960.o
DASMOBJS += $(CPUOBJ)/i960/i960dis.o
endif

$(CPUOBJ)/i960/i960.o:	$(CPUSRC)/i960/i960.c \
						$(CPUSRC)/i960/i960.h



#-------------------------------------------------
# Konami custom CPU (6809-based)
#-------------------------------------------------

ifneq ($(filter KONAMI,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/konami
CPUOBJS += $(CPUOBJ)/konami/konami.o
DASMOBJS += $(CPUOBJ)/konami/knmidasm.o
endif

$(CPUOBJ)/konami/konami.o:	$(CPUSRC)/konami/konami.c \
							$(CPUSRC)/konami/konami.h \
							$(CPUSRC)/konami/konamops.c \
							$(CPUSRC)/konami/konamtbl.c



#-------------------------------------------------
# LH5801
#-------------------------------------------------

ifneq ($(filter LH5801,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/lh5801
CPUOBJS += $(CPUOBJ)/lh5801/lh5801.o
DASMOBJS += $(CPUOBJ)/lh5801/5801dasm.o
endif

$(CPUOBJ)/lh5801/lh5801.o:	$(CPUSRC)/lh5801/lh5801.c \
							$(CPUSRC)/lh5801/5801tbl.c \
							$(CPUSRC)/lh5801/lh5801.h



#-------------------------------------------------
# Manchester Small-Scale Experimental Machine
#-------------------------------------------------

ifneq ($(filter SSEM,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/ssem
CPUOBJS += $(CPUOBJ)/ssem/ssem.o
DASMOBJS += $(CPUOBJ)/ssem/ssemdasm.o
endif

$(CPUOBJ)/ssem/ssem.o:	$(CPUSRC)/ssem/ssem.c \
			$(CPUSRC)/ssem/ssem.h



#-------------------------------------------------
# Fujitsu MB88xx
#-------------------------------------------------

ifneq ($(filter MB88XX,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/mb88xx
CPUOBJS += $(CPUOBJ)/mb88xx/mb88xx.o
DASMOBJS += $(CPUOBJ)/mb88xx/mb88dasm.o
endif

$(CPUOBJ)/mb88xx/mb88xx.o:	$(CPUSRC)/mb88xx/mb88xx.c \
							$(CPUSRC)/mb88xx/mb88xx.h



#-------------------------------------------------
# Fujitsu MB86233
#-------------------------------------------------

ifneq ($(filter MB86233,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/mb86233
CPUOBJS += $(CPUOBJ)/mb86233/mb86233.o
DASMOBJS += $(CPUOBJ)/mb86233/mb86233d.o
endif

$(CPUOBJ)/mb86233/mb86233.o:	$(CPUSRC)/mb86233/mb86233.c \
								$(CPUSRC)/mb86233/mb86233.h



#-------------------------------------------------
# Microchip PIC16C5x
#-------------------------------------------------

ifneq ($(filter PIC16C5X,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/pic16c5x
CPUOBJS += $(CPUOBJ)/pic16c5x/pic16c5x.o
DASMOBJS += $(CPUOBJ)/pic16c5x/16c5xdsm.o
endif

$(CPUOBJ)/pic16c5x/pic16c5x.o:	$(CPUSRC)/pic16c5x/pic16c5x.c \
								$(CPUSRC)/pic16c5x/pic16c5x.h



#-------------------------------------------------
# Microchip PIC16C62x
#-------------------------------------------------

ifneq ($(filter PIC16C62X,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/pic16c62x
CPUOBJS += $(CPUOBJ)/pic16c62x/pic16c62x.o
DASMOBJS += $(CPUOBJ)/pic16c62x/16c62xdsm.o
endif

$(CPUOBJ)/pic16c62x/pic16c62x.o:	$(CPUSRC)/pic16c62x/pic16c62x.c \
								$(CPUSRC)/pic16c62x/pic16c62x.h



#-------------------------------------------------
# MIPS R3000 (MIPS I/II) series
# MIPS R4000 (MIPS III/IV) series
# Sony PlayStation CPU (R3000-based + GTE)
#-------------------------------------------------

ifneq ($(filter MIPS,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/mips
CPUOBJS += $(CPUOBJ)/mips/r3000.o
CPUOBJS += $(CPUOBJ)/mips/mips3com.o $(CPUOBJ)/mips/mips3fe.o $(CPUOBJ)/mips/mips3drc.o $(DRCOBJ)
CPUOBJS += $(CPUOBJ)/mips/psx.o
DASMOBJS += $(CPUOBJ)/mips/r3kdasm.o
DASMOBJS += $(CPUOBJ)/mips/mips3dsm.o
DASMOBJS += $(CPUOBJ)/mips/psxdasm.o
endif

$(CPUOBJ)/mips/r3000.o:	$(CPUSRC)/mips/r3000.c \
			$(CPUSRC)/mips/r3000.h

$(CPUOBJ)/mips/mips3com.o:	$(CPUSRC)/mips/mips3.h \
				$(CPUSRC)/mips/mips3com.h

$(CPUOBJ)/mips/mips3fe.o:	$(CPUSRC)/mips/mips3.h \
				$(CPUSRC)/mips/mips3com.h \
				$(CPUSRC)/mips/mips3fe.h

$(CPUOBJ)/mips/mips3drc.o:	$(CPUSRC)/mips/mips3drc.c \
				$(CPUSRC)/mips/mips3.h \
				$(CPUSRC)/mips/mips3com.h \
				$(CPUSRC)/mips/mips3fe.h \
				$(DRCDEPS)

$(CPUOBJ)/mips/psx.o:	$(CPUSRC)/mips/psx.c \
			$(CPUSRC)/mips/psx.h



#-------------------------------------------------
# Mitsubishi M37702 and M37710 (based on 65C816)
#-------------------------------------------------

ifneq ($(filter M37710,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m37710
CPUOBJS += \
	$(CPUOBJ)/m37710/m37710.o \
	$(CPUOBJ)/m37710/m37710o0.o \
	$(CPUOBJ)/m37710/m37710o1.o \
	$(CPUOBJ)/m37710/m37710o2.o \
	$(CPUOBJ)/m37710/m37710o3.o
DASMOBJS += $(CPUOBJ)/m37710/m7700ds.o
endif

M37710DEPS = \
	$(CPUSRC)/m37710/m37710.h \
	$(CPUSRC)/m37710/m37710op.h \
	$(CPUSRC)/m37710/m7700ds.h

$(CPUOBJ)/m37710/m37710.o:		$(CPUSRC)/m37710/m37710.c \
								$(M37710DEPS)

$(CPUOBJ)/m37710/m37710o0.o:	$(CPUSRC)/m37710/m37710o0.c \
								$(M37710DEPS)

$(CPUOBJ)/m37710/m37710o1.o:	$(CPUSRC)/m37710/m37710o1.c \
								$(M37710DEPS)

$(CPUOBJ)/m37710/m37710o2.o:	$(CPUSRC)/m37710/m37710o2.c \
								$(M37710DEPS)

$(CPUOBJ)/m37710/m37710o3.o:	$(CPUSRC)/m37710/m37710o3.c \
								$(M37710DEPS)

$(CPUOBJ)/m37710/m7700ds.o:		$(CPUSRC)/m37710/m7700ds.c \
								$(CPUSRC)/m37710/m7700ds.h



#-------------------------------------------------
# Mostek 6502 and its many derivatives
#-------------------------------------------------

ifneq ($(filter M6502,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m6502
CPUOBJS += $(CPUOBJ)/m6502/m6502.o
CPUOBJS += $(CPUOBJ)/m6502/m6509.o
CPUOBJS += $(CPUOBJ)/m6502/m65ce02.o
CPUOBJS += $(CPUOBJ)/m6502/m4510.o
DASMOBJS += $(CPUOBJ)/m6502/6502dasm.o
endif

$(CPUOBJ)/m6502/m4510.o:	$(CPUSRC)/m6502/m4510.c \
							$(CPUSRC)/m6502/t65ce02.c

$(CPUOBJ)/m6502/m6502.o:	$(CPUSRC)/m6502/m6502.c \
							$(CPUSRC)/m6502/m6502.h \
							$(CPUSRC)/m6502/ops02.h \
							$(CPUSRC)/m6502/t6502.c \
							$(CPUSRC)/m6502/t65c02.c \
							$(CPUSRC)/m6502/t65sc02.c \
							$(CPUSRC)/m6502/t6510.c \
							$(CPUSRC)/m6502/tn2a03.c \
							$(CPUSRC)/m6502/tdeco16.c

$(CPUOBJ)/m6502/m65ce02.o:	$(CPUSRC)/m6502/m65ce02.c \
							$(CPUSRC)/m6502/m65ce02.h \
							$(CPUSRC)/m6502/opsce02.h \
							$(CPUSRC)/m6502/t65ce02.c

$(CPUOBJ)/m6502/m6509.o:	$(CPUSRC)/m6502/m6509.c \
							$(CPUSRC)/m6502/m6509.h \
							$(CPUSRC)/m6502/ops09.h \
							$(CPUSRC)/m6502/t6509.c



#-------------------------------------------------
# Motorola 680x
#-------------------------------------------------

ifneq ($(filter M6800,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m6800
CPUOBJS += $(CPUOBJ)/m6800/m6800.o
DASMOBJS += $(CPUOBJ)/m6800/6800dasm.o
endif

$(CPUOBJ)/m6800/m6800.o:	$(CPUSRC)/m6800/m6800.c \
							$(CPUSRC)/m6800/m6800.h \
							$(CPUSRC)/m6800/6800ops.c \
							$(CPUSRC)/m6800/6800tbl.c



#-------------------------------------------------
# Motorola 6805
#-------------------------------------------------

ifneq ($(filter M6805,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m6805
CPUOBJS += $(CPUOBJ)/m6805/m6805.o
DASMOBJS += $(CPUOBJ)/m6805/6805dasm.o
endif

$(CPUOBJ)/m6805/m6805.o:	$(CPUSRC)/m6805/m6805.c \
							$(CPUSRC)/m6805/m6805.h \
							$(CPUSRC)/m6805/6805ops.c



#-------------------------------------------------
# Motorola 6809
#-------------------------------------------------

ifneq ($(filter M6809,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m6809
CPUOBJS += $(CPUOBJ)/m6809/m6809.o
DASMOBJS += $(CPUOBJ)/m6809/6809dasm.o
endif

$(CPUOBJ)/m6809/m6809.o:	$(CPUSRC)/m6809/m6809.c \
							$(CPUSRC)/m6809/m6809.h \
							$(CPUSRC)/m6809/6809ops.c \
							$(CPUSRC)/m6809/6809tbl.c



#-------------------------------------------------
# Motorola 68HC11
#-------------------------------------------------

ifneq ($(filter MC68HC11,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/mc68hc11
CPUOBJS += $(CPUOBJ)/mc68hc11/mc68hc11.o
DASMOBJS += $(CPUOBJ)/mc68hc11/hc11dasm.o
endif

$(CPUOBJ)/mc68hc11/mc68hc11.o:	$(CPUSRC)/mc68hc11/mc68hc11.c \
								$(CPUSRC)/mc68hc11/hc11ops.c



#-------------------------------------------------
# Motorola 68000 series
#-------------------------------------------------

ifneq ($(filter M680X0,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/m68000
CPUOBJS += $(CPUOBJ)/m68000/m68kcpu.o $(CPUOBJ)/m68000/m68kops.o
DASMOBJS += $(CPUOBJ)/m68000/m68kdasm.o
M68KMAKE = $(BUILDOUT)/m68kmake$(BUILD_EXE)
endif

# when we compile source files we need to include generated files from the OBJ directory
$(CPUOBJ)/m68000/%.o: $(CPUSRC)/m68000/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -I$(CPUOBJ)/m68000 -c $< -o $@

# when we compile generated files we need to include stuff from the src directory
$(CPUOBJ)/m68000/%.o: $(CPUOBJ)/m68000/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -I$(CPUSRC)/m68000 -c $< -o $@

# rule to generate the C files
$(CPUOBJ)/m68000/m68kops.c: $(M68KMAKE) $(CPUSRC)/m68000/m68k_in.c
	@echo Generating M68K source files...
	$(M68KMAKE) $(CPUOBJ)/m68000 $(CPUSRC)/m68000/m68k_in.c

# rule to build the generator
ifneq ($(CROSS_BUILD),1)

BUILD += $(M68KMAKE)

$(M68KMAKE): $(CPUOBJ)/m68000/m68kmake.o $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $^ $(LIBS) -o $@
endif

# rule to ensure we build the header before building the core CPU file
$(CPUOBJ)/m68000/m68kcpu.o: 	$(CPUOBJ)/m68000/m68kops.c \
								$(CPUSRC)/m68000/m68kcpu.h $(CPUSRC)/m68000/m68kfpu.c $(CPUSRC)/m68000/m68kmmu.h



#-------------------------------------------------
# Motorola/Freescale dsp56k
#-------------------------------------------------

ifneq ($(filter DSP56156,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/dsp56k
CPUOBJS += $(CPUOBJ)/dsp56k/dsp56k.o
DASMOBJS += $(CPUOBJ)/dsp56k/dsp56dsm.o
endif

$(CPUOBJ)/dsp56k/dsp56k.o:	$(CPUSRC)/dsp56k/dsp56k.c \
							$(CPUSRC)/dsp56k/dsp56ops.c \
							$(CPUSRC)/dsp56k/dsp56mem.c \
							$(CPUSRC)/dsp56k/dsp56pcu.c \
							$(CPUSRC)/dsp56k/dsp56k.h



#-------------------------------------------------
# PDP-1
# TX0
#-------------------------------------------------

ifneq ($(filter PDP1,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/pdp1
CPUOBJS += $(CPUOBJ)/pdp1/pdp1.o
CPUOBJS += $(CPUOBJ)/pdp1/tx0.o
DASMOBJS += $(CPUOBJ)/pdp1/pdp1dasm.o
DASMOBJS += $(CPUOBJ)/pdp1/tx0dasm.o
endif

$(CPUOBJ)/pdp1/pdp1.o:	$(CPUSRC)/pdp1/pdp1.c \
						$(CPUSRC)/pdp1/pdp1.h

$(CPUOBJ)/pdp1/tx0.o:		$(CPUSRC)/pdp1/tx0.h \
							$(CPUSRC)/pdp1/tx0.c

$(CPUOBJ)/pdp1/tx0dasm.o:	$(CPUSRC)/pdp1/tx0.h \
							$(CPUSRC)/pdp1/tx0dasm.c


#-------------------------------------------------
# Motorola PowerPC series
#-------------------------------------------------

ifneq ($(filter POWERPC,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/powerpc
CPUOBJS += $(CPUOBJ)/powerpc/ppccom.o $(CPUOBJ)/powerpc/ppcfe.o $(CPUOBJ)/powerpc/ppcdrc.o $(DRCOBJ)
DASMOBJS += $(CPUOBJ)/powerpc/ppc_dasm.o
endif

$(CPUOBJ)/powerpc/ppccom.o:	$(CPUSRC)/powerpc/ppc.h \
							$(CPUSRC)/powerpc/ppccom.h

$(CPUOBJ)/powerpc/ppcfe.o:	$(CPUSRC)/powerpc/ppc.h \
							$(CPUSRC)/powerpc/ppccom.h \
							$(CPUSRC)/powerpc/ppcfe.h

$(CPUOBJ)/powerpc/ppcdrc.o:	$(CPUSRC)/powerpc/ppcdrc.c \
							$(CPUSRC)/powerpc/ppc.h \
							$(CPUSRC)/powerpc/ppccom.h \
							$(CPUSRC)/powerpc/ppcfe.h \
							$(DRCDEPS)



#-------------------------------------------------
# NEC V-series Intel-compatible
#-------------------------------------------------

ifneq ($(filter NEC,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/nec
CPUOBJS += $(CPUOBJ)/nec/nec.o
DASMOBJS += $(CPUOBJ)/nec/necdasm.o
endif

ifneq ($(filter V30MZ,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/v30mz $(CPUOBJ)/nec
CPUOBJS += $(CPUOBJ)/v30mz/v30mz.o
DASMOBJS += $(CPUOBJ)/nec/necdasm.o
endif

$(CPUOBJ)/nec/nec.o:	$(CPUSRC)/nec/nec.c \
						$(CPUSRC)/nec/nec.h \
						$(CPUSRC)/nec/necea.h \
						$(CPUSRC)/nec/nechost.h \
						$(CPUSRC)/nec/necinstr.h \
						$(CPUSRC)/nec/necmodrm.h \
						$(CPUSRC)/nec/necpriv.h

$(CPUOBJ)/v30mz/v30mz.o:	$(CPUSRC)/v30mz/v30mz.c \
							$(CPUSRC)/v30mz/v30mz.h \
							$(CPUSRC)/v30mz/necmodrm.h \
							$(CPUSRC)/v30mz/necinstr.h \
							$(CPUSRC)/v30mz/necea.h \
							$(CPUSRC)/v30mz/nechost.h \
							$(CPUSRC)/v30mz/nec.h



#-------------------------------------------------
# NEC V60/V70
#-------------------------------------------------

ifneq ($(filter V60,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/v60
CPUOBJS += $(CPUOBJ)/v60/v60.o
DASMOBJS += $(CPUOBJ)/v60/v60d.o
endif

$(CPUOBJ)/v60/v60.o:	$(CPUSRC)/v60/am.c \
						$(CPUSRC)/v60/am1.c \
						$(CPUSRC)/v60/am2.c \
						$(CPUSRC)/v60/am3.c \
						$(CPUSRC)/v60/op12.c \
						$(CPUSRC)/v60/op2.c \
						$(CPUSRC)/v60/op3.c \
						$(CPUSRC)/v60/op4.c \
						$(CPUSRC)/v60/op5.c \
						$(CPUSRC)/v60/op6.c \
						$(CPUSRC)/v60/op7a.c \
						$(CPUSRC)/v60/optable.c \
						$(CPUSRC)/v60/v60.c \
						$(CPUSRC)/v60/v60.h \
						$(CPUSRC)/v60/v60mem.c \
						$(CPUSRC)/v60/v60d.c



#-------------------------------------------------
# NEC V810 (uPD70732)
#-------------------------------------------------

ifneq ($(filter V810,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/v810
CPUOBJS += $(CPUOBJ)/v810/v810.o
DASMOBJS += $(CPUOBJ)/v810/v810dasm.o
endif

$(CPUOBJ)/v810/v810.o:	$(CPUSRC)/v810/v810.c \
						$(CPUSRC)/v810/v810.h



#-------------------------------------------------
# NEC uPD7810 series
#-------------------------------------------------

ifneq ($(filter UPD7810,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/upd7810
CPUOBJS += $(CPUOBJ)/upd7810/upd7810.o
DASMOBJS += $(CPUOBJ)/upd7810/7810dasm.o
endif

$(CPUOBJ)/upd7810/upd7810.o:	$(CPUSRC)/upd7810/upd7810.c \
								$(CPUSRC)/upd7810/7810tbl.c \
								$(CPUSRC)/upd7810/7810ops.c \
								$(CPUSRC)/upd7810/upd7810.h



#-------------------------------------------------
# Nintendo Minx
#-------------------------------------------------

ifneq ($(filter MINX,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/minx
CPUOBJS += $(CPUOBJ)/minx/minx.o
DASMOBJS += $(CPUOBJ)/minx/minxd.o
endif

$(CPUOBJ)/minx/minx.o:		$(CPUSRC)/minx/minx.c \
							$(CPUSRC)/minx/minx.h \
							$(CPUSRC)/minx/minxd.c \
							$(CPUSRC)/minx/minxopce.h \
							$(CPUSRC)/minx/minxopcf.h \
							$(CPUSRC)/minx/minxops.h \
							$(CPUSRC)/minx/minxfunc.h


#-------------------------------------------------
# Nintendo/SGI RSP (R3000-based + vector processing)
#-------------------------------------------------

ifneq ($(filter RSP,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/rsp
CPUOBJS += $(CPUOBJ)/rsp/rsp.o $(CPUOBJ)/rsp/rspdrc.o $(CPUOBJ)/rsp/rspfe.o $(DRCOBJ)
DASMOBJS += $(CPUOBJ)/rsp/rsp_dasm.o
endif

$(CPUOBJ)/rsp/rsp.o:	$(CPUSRC)/rsp/rsp.c \
				$(CPUSRC)/rsp/rsp.h

$(CPUOBJ)/rsp/rspdrc.o:	$(CPUSRC)/rsp/rspdrc.c \
			$(CPUSRC)/rsp/rsp.h \
			$(CPUSRC)/rsp/rspfe.h \
			$(DRCDEPS)

$(CPUOBJ)/rsp/rspfe.o:	$(CPUSRC)/rsp/rspfe.c \
			$(CPUSRC)/rsp/rspfe.h


#-------------------------------------------------
# Panasonic MN10200
#-------------------------------------------------

ifneq ($(filter MN10200,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/mn10200
CPUOBJS += $(CPUOBJ)/mn10200/mn10200.o
DASMOBJS += $(CPUOBJ)/mn10200/mn102dis.o
endif

$(CPUOBJ)/mn10200/mn10200.o:	$(CPUSRC)/mn10200/mn10200.c \
								$(CPUSRC)/mn10200/mn10200.h


#-------------------------------------------------
# Saturn
#-------------------------------------------------

ifneq ($(filter SATURN,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/saturn
CPUOBJS += $(CPUOBJ)/saturn/saturn.o
DASMOBJS += $(CPUOBJ)/saturn/saturnds.o
endif

$(CPUOBJ)/saturn/saturn.o:	$(CPUSRC)/saturn/saturn.c \
							$(CPUSRC)/saturn/sattable.c \
							$(CPUSRC)/saturn/satops.c \
							$(CPUSRC)/saturn/saturn.h



#-------------------------------------------------
# Signetics 2650
#-------------------------------------------------

ifneq ($(filter S2650,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/s2650
CPUOBJS += $(CPUOBJ)/s2650/s2650.o
DASMOBJS += $(CPUOBJ)/s2650/2650dasm.o
endif

$(CPUOBJ)/s2650/s2650.o:	$(CPUSRC)/s2650/s2650.c \
							$(CPUSRC)/s2650/s2650.h \
							$(CPUSRC)/s2650/s2650cpu.h



#-------------------------------------------------
# SC61860
#-------------------------------------------------

ifneq ($(filter SC61860,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/sc61860
CPUOBJS += $(CPUOBJ)/sc61860/sc61860.o
DASMOBJS += $(CPUOBJ)/sc61860/scdasm.o
endif

$(CPUOBJ)/sc61860/sc61860.o:	$(CPUSRC)/sc61860/sc61860.h \
								$(CPUSRC)/sc61860/sc.h \
								$(CPUSRC)/sc61860/scops.c \
								$(CPUSRC)/sc61860/sctable.c



#-------------------------------------------------
# SM8500
#-------------------------------------------------

ifneq ($(filter SM8500,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/sm8500
CPUOBJS += $(CPUOBJ)/sm8500/sm8500.o
DASMOBJS += $(CPUOBJ)/sm8500/sm8500d.o
endif

$(CPUOBJ)/sm8500/sm8500.o:	$(CPUSRC)/sm8500/sm8500.c \
							$(CPUSRC)/sm8500/sm8500.h \
							$(CPUSRC)/sm8500/sm85ops.h



#-------------------------------------------------
# Sony/Nintendo SPC700
#-------------------------------------------------

ifneq ($(filter SPC700,$(CPUS)),)
SPCD = cpu/spc700
OBJDIRS += $(CPUOBJ)/spc700
CPUOBJS += $(CPUOBJ)/spc700/spc700.o
DASMOBJS += $(CPUOBJ)/spc700/spc700ds.o
endif

$(CPUOBJ)/spc700/spc700.o:	$(CPUSRC)/spc700/spc700.c \
							$(CPUSRC)/spc700/spc700.h



#-------------------------------------------------
# SSP1601
#-------------------------------------------------

ifneq ($(filter SSP1601,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/ssp1601
CPUOBJS += $(CPUOBJ)/ssp1601/ssp1601.o
DASMOBJS += $(CPUOBJ)/ssp1601/ssp1601d.o
endif

$(CPUOBJ)/ssp1610/ssp1601.o:	$(CPUSRC)/ssp1601/ssp1601.c \
								$(CPUSRC)/ssp1610/ssp1601.h



#-------------------------------------------------
# SunPlus u'nSP
#-------------------------------------------------

ifneq ($(filter UNSP,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/unsp
CPUOBJS += $(CPUOBJ)/unsp/unsp.o
DASMOBJS += $(CPUOBJ)/unsp/unspdasm.o
endif

$(CPUOBJ)/unsp/unsp.o:	$(CPUSRC)/unsp/unsp.c \
			$(CPUSRC)/unsp/unsp.h



#-------------------------------------------------
# Atmel 8-bit AVR
#-------------------------------------------------

ifneq ($(filter AVR8,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/avr8
CPUOBJS += $(CPUOBJ)/avr8/avr8.o
DASMOBJS += $(CPUOBJ)/avr8/avr8dasm.o
endif

$(CPUOBJ)/avr8/avr8.o:	$(CPUSRC)/avr8/avr8.c \
			$(CPUSRC)/avr8/avr8.h



#-------------------------------------------------
# Texas Instruments TMS0980
#-------------------------------------------------

ifneq ($(filter TMS0980,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms0980
CPUOBJS += $(CPUOBJ)/tms0980/tms0980.o
DASMOBJS += $(CPUOBJ)/tms0980/tms0980d.o
endif

$(CPUOBJ)/tms0980/tms0980.o:	$(CPUSRC)/tms0980/tms0980.h \
								$(CPUSRC)/tms0980/tms0980.c

$(CPUOBJ)/tms0980/tms0980d.o:	$(CPUSRC)/tms0980/tms0980.h \
								$(CPUSRC)/tms0980/tms0980d.c



#-------------------------------------------------
# Texas Instruments TMS7000 series
#-------------------------------------------------

ifneq ($(filter TMS7000,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms7000
CPUOBJS += $(CPUOBJ)/tms7000/tms7000.o
DASMOBJS += $(CPUOBJ)/tms7000/7000dasm.o
endif

$(CPUOBJ)/tms7000/tms7000.o:	$(CPUSRC)/tms7000/tms7000.h \
								$(CPUSRC)/tms7000/tms7000.c \
								$(CPUSRC)/tms7000/tms70op.c \
								$(CPUSRC)/tms7000/tms70tb.c

$(CPUOBJ)/tms7000/7000dasm.o:	$(CPUSRC)/tms7000/tms7000.h \
								$(CPUSRC)/tms7000/7000dasm.c



#-------------------------------------------------
# Texas Instruments TMS99xx series
#-------------------------------------------------

ifneq ($(filter TMS9900,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms9900
CPUOBJS += $(CPUOBJ)/tms9900/tms9900.o
CPUOBJS += $(CPUOBJ)/tms9900/tms9980a.o
CPUOBJS += $(CPUOBJ)/tms9900/tms9995.o
CPUOBJS += $(CPUOBJ)/tms9900/ti990_10.o
DASMOBJS += $(CPUOBJ)/tms9900/9900dasm.o
endif

$(CPUOBJ)/tms9900/tms9900.o:	$(CPUSRC)/tms9900/tms9900.c \
								$(CPUSRC)/tms9900/tms9900.h \
								$(CPUSRC)/tms9900/99xxcore.h \
								$(CPUSRC)/tms9900/99xxstat.h

$(CPUOBJ)/tms9900/tms9980a.o:	$(CPUSRC)/tms9900/tms9980a.c \
								$(CPUSRC)/tms9900/tms9900.h \
								$(CPUSRC)/tms9900/99xxcore.h \
								$(CPUSRC)/tms9900/99xxstat.h

$(CPUOBJ)/tms9900/tms9995.o:	$(CPUSRC)/tms9900/tms9995.c \
								$(CPUSRC)/tms9900/tms9900.h \
								$(CPUSRC)/tms9900/99xxcore.h \
								$(CPUSRC)/tms9900/99xxstat.h

$(CPUOBJ)/tms9900/ti990_10.o:	$(CPUSRC)/tms9900/ti990_10.c \
								$(CPUSRC)/tms9900/tms9900.h \
								$(CPUSRC)/tms9900/99xxcore.h \
								$(CPUSRC)/tms9900/99xxstat.h



#-------------------------------------------------
# Texas Instruments TMS340x0 graphics controllers
#-------------------------------------------------

ifneq ($(filter TMS340X0,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms34010
CPUOBJS += $(CPUOBJ)/tms34010/tms34010.o
DASMOBJS += $(CPUOBJ)/tms34010/34010dsm.o
endif

$(CPUOBJ)/tms34010/tms34010.o:	$(CPUSRC)/tms34010/tms34010.c \
								$(CPUSRC)/tms34010/tms34010.h \
								$(CPUSRC)/tms34010/34010ops.c \
								$(CPUSRC)/tms34010/34010gfx.c \
								$(CPUSRC)/tms34010/34010fld.c \
								$(CPUSRC)/tms34010/34010tbl.c



#-------------------------------------------------
# Texas Instruments TMS3201x DSP
#-------------------------------------------------

ifneq ($(filter TMS32010,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms32010
CPUOBJS += $(CPUOBJ)/tms32010/tms32010.o
DASMOBJS += $(CPUOBJ)/tms32010/32010dsm.o
endif

$(CPUOBJ)/tms32010/tms32010.o:	$(CPUSRC)/tms32010/tms32010.c \
								$(CPUSRC)/tms32010/tms32010.h



#-------------------------------------------------
# Texas Instruments TMS3202x DSP
#-------------------------------------------------

ifneq ($(filter TMS32025,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms32025
CPUOBJS += $(CPUOBJ)/tms32025/tms32025.o
DASMOBJS += $(CPUOBJ)/tms32025/32025dsm.o
endif

$(CPUOBJ)/tms32025/tms32025.o:	$(CPUSRC)/tms32025/tms32025.c \
								$(CPUSRC)/tms32025/tms32025.h



#-------------------------------------------------
# Texas Instruments TMS3203x DSP
#-------------------------------------------------

ifneq ($(filter TMS32031,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms32031
CPUOBJS += $(CPUOBJ)/tms32031/tms32031.o
DASMOBJS += $(CPUOBJ)/tms32031/dis32031.o
endif

$(CPUOBJ)/tms32031/tms32031.o:	$(CPUSRC)/tms32031/tms32031.c \
								$(CPUSRC)/tms32031/tms32031.h \
								$(CPUSRC)/tms32031/32031ops.c



#-------------------------------------------------
# Texas Instruments TMS3205x DSP
#-------------------------------------------------

ifneq ($(filter TMS32051,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms32051
CPUOBJS += $(CPUOBJ)/tms32051/tms32051.o
DASMOBJS += $(CPUOBJ)/tms32051/dis32051.o
endif

$(CPUOBJ)/tms32051/tms32051.o:	$(CPUSRC)/tms32051/tms32051.c \
								$(CPUSRC)/tms32051/tms32051.h \
								$(CPUSRC)/tms32051/32051ops.c



#-------------------------------------------------
# Texas Instruments TMS57002 DSP
#-------------------------------------------------

ifneq ($(filter TMS57002,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tms57002
CPUOBJS += $(CPUOBJ)/tms57002/tms57002.o
DASMOBJS += $(CPUOBJ)/tms57002/57002dsm.o
TMSMAKE += $(BUILDOUT)/tmsmake$(BUILD_EXE)
endif

$(CPUOBJ)/tms57002/tms57002.o:	$(CPUSRC)/tms57002/tms57002.c \
								$(CPUSRC)/tms57002/tms57002.h \
								$(CPUOBJ)/tms57002/tms57002.inc

$(CPUOBJ)/tms57002/57002dsm.o:	$(CPUSRC)/tms57002/57002dsm.c \
								$(CPUOBJ)/tms57002/tms57002.inc

# rule to generate the C file
$(CPUOBJ)/tms57002/tms57002.inc: $(TMSMAKE) $(CPUSRC)/tms57002/tmsinstr.lst
	@echo Generating TMS57002 source file...
	$(TMSMAKE) $(CPUSRC)/tms57002/tmsinstr.lst $@

# rule to build the generator
ifneq ($(CROSS_BUILD),1)

BUILD += $(TMSMAKE)

$(TMSMAKE): $(CPUOBJ)/tms57002/tmsmake.o $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $^ $(LIBS) -o $@

endif


#-------------------------------------------------
# Toshiba TLCS-90 Series
#-------------------------------------------------

ifneq ($(filter TLCS90,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tlcs90
CPUOBJS += $(CPUOBJ)/tlcs90/tlcs90.o
#DASMOBJS += $(CPUOBJ)/tlcs90/tlcs90.o
endif

$(CPUOBJ)/tlcs90/tlcs90.o:	$(CPUSRC)/tlcs90/tlcs90.c \
							$(CPUSRC)/tlcs90/tlcs90.h



#-------------------------------------------------
# Toshiba TLCS-900 Series
#-------------------------------------------------

ifneq ($(filter TLCS900,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/tlcs900
CPUOBJS += $(CPUOBJ)/tlcs900/tlcs900.o
DASMOBJS += $(CPUOBJ)/tlcs900/dasm900.o
endif

$(CPUOBJ)/tlcs900/tlcs900.o:	$(CPUSRC)/tlcs900/tlcs900.c \
								$(CPUSRC)/tlcs900/900tbl.c \
								$(CPUSRC)/tlcs900/tlcs900.h

$(CPUOBJ)/tlcs900/dasm900.o:	$(CPUSRC)/tlcs900/dasm900.c



#-------------------------------------------------
# Zilog Z80
#-------------------------------------------------

ifneq ($(filter Z80,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/z80
CPUOBJS += $(CPUOBJ)/z80/z80.o $(CPUOBJ)/z80/z80daisy.o
DASMOBJS += $(CPUOBJ)/z80/z80dasm.o
endif

$(CPUOBJ)/z80/z80.o:	$(CPUSRC)/z80/z80.c \
						$(CPUSRC)/z80/z80.h



#-------------------------------------------------
# Sharp LR35902 (Game Boy CPU)
#-------------------------------------------------

ifneq ($(filter LR35902,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/lr35902
CPUOBJS += $(CPUOBJ)/lr35902/lr35902.o
DASMOBJS += $(CPUOBJ)/lr35902/lr35902d.o
endif

$(CPUOBJ)/lr35902/lr35902.o:	$(CPUSRC)/lr35902/lr35902.c \
								$(CPUSRC)/lr35902/lr35902.h \
								$(CPUSRC)/lr35902/opc_cb.h \
								$(CPUSRC)/lr35902/opc_main.h



#-------------------------------------------------
# Zilog Z180
#-------------------------------------------------

ifneq ($(filter Z180,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/z180 $(CPUOBJ)/z80
CPUOBJS += $(CPUOBJ)/z180/z180.o $(CPUOBJ)/z80/z80daisy.o
DASMOBJS += $(CPUOBJ)/z180/z180dasm.o
endif

$(CPUOBJ)/z180/z180.o:	$(CPUSRC)/z180/z180.c \
						$(CPUSRC)/z180/z180.h \
						$(CPUSRC)/z180/z180op.c \
						$(CPUSRC)/z180/z180ops.h \
						$(CPUSRC)/z180/z180tbl.h \
						$(CPUSRC)/z180/z180cb.c \
						$(CPUSRC)/z180/z180dd.c \
						$(CPUSRC)/z180/z180ed.c \
						$(CPUSRC)/z180/z180fd.c \
						$(CPUSRC)/z180/z180xy.c



#-------------------------------------------------
# Zilog Z8000
#-------------------------------------------------

ifneq ($(filter Z8000,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/z8000
CPUOBJS += $(CPUOBJ)/z8000/z8000.o
DASMOBJS += $(CPUOBJ)/z8000/8000dasm.o
endif

$(CPUOBJ)/z8000/z8000.o:	$(CPUSRC)/z8000/z8000.c \
							$(CPUSRC)/z8000/z8000.h \
							$(CPUSRC)/z8000/z8000cpu.h \
							$(CPUSRC)/z8000/z8000dab.h \
							$(CPUSRC)/z8000/z8000ops.c \
							$(CPUSRC)/z8000/z8000tbl.c



#-------------------------------------------------
# Zilog Z8
#-------------------------------------------------

ifneq ($(filter Z8,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/z8
CPUOBJS += $(CPUOBJ)/z8/z8.o
DASMOBJS += $(CPUOBJ)/z8/z8dasm.o
endif

$(CPUOBJ)/z8/z8.o:	$(CPUSRC)/z8/z8.c \
					$(CPUSRC)/z8/z8ops.c \
					$(CPUSRC)/z8/z8.h



#-------------------------------------------------
# Argonaut SuperFX
#-------------------------------------------------

ifneq ($(filter SUPERFX,$(CPUS)),)
OBJDIRS += $(CPUOBJ)/superfx
CPUOBJS += $(CPUOBJ)/superfx/superfx.o
DASMOBJS += $(CPUOBJ)/superfx/sfx_dasm.o
endif

$(CPUOBJ)/superfx/superfx.o:$(CPUSRC)/superfx/superfx.c \
							$(CPUSRC)/superfx/superfx.h
