/*************************************************************************

    Driver for Atari/Midway Phoenix/Seattle/Flagstaff hardware games

    driver by Aaron Giles

    Games supported:
        * Wayne Gretzky's 3d Hockey    [Phoenix, Atari, ~100MHz, 4MB RAM, 1xTMU]

        * Bio Freaks                   [Seattle, Midway, ???MHz, 8MB RAM, 1xTMU]
        * CarnEvil                     [Seattle, Midway, 150MHz, 8MB RAM, 1xTMU]
        * NFL Blitz                    [Seattle, Midway, 150MHz, 8MB RAM, 1xTMU]
        * NFL Blitz 99                 [Seattle, Midway, 150MHz, 8MB RAM, 1xTMU]
        * NFL Blitz 2000               [Seattle, Midway, 150MHz, 8MB RAM, 1xTMU]
        * Mace: The Dark Age           [Seattle, Atari,  200MHz, 8MB RAM, 1xTMU]

        * California Speed             [Seattle + Widget, Atari, 150MHz, 8MB RAM, 1xTMU]
        * Vapor TRX                    [Seattle + Widget, Atari, 192MHz, 8MB RAM, 1xTMU]
        * Hyperdrive                   [Seattle + Widget, Midway, 200MHz, 8MB RAM, 1xTMU]

        * San Francisco Rush           [Flagstaff, Atari, 192MHz, 2xTMU]
        * San Francisco Rush: The Rock [Flagstaff, Atari, 192MHz, 8MB RAM, 2xTMU]

    Known bugs:
        * Carnevil: lets you set the flash brightness; need to emulate that

***************************************************************************

    Phoenix hardware main board:

        * 100MHz R4700 main CPU (50MHz system clock)
        * Galileo GT64010 system controller
        * National Semiconductor PC87415 IDE controller
        * 3dfx FBI with 2MB frame buffer
        * 3dfx TMU with 4MB txture memory
        * Midway I/O ASIC
        * 4MB DRAM for main CPU
        * 512KB boot ROM
        * 16MHz ADSP 2115 audio CPU
        * 4MB DRAM for audio CPU
        * 32KB boot ROM

    Seattle hardware main board:

        * 144MHz/150MHz/192MHz/200MHz R5000 main CPU (system clock 48MHz/50MHz)
        * Galileo GT64010 system controller
        * National Semiconductor PC87415 IDE controller
        * 3dfx FBI with 2MB frame buffer
        * 3dfx TMU with 4MB txture memory
        * Midway I/O ASIC
        * 8MB DRAM for main CPU
        * 512KB boot ROM
        * 16MHz ADSP 2115 audio CPU
        * 4MB DRAM for audio CPU
        * 32KB boot ROM

    Flagstaff hardware main board:

        * 200MHz R5000 main CPU (system clock 50MHz)
        * Galileo GT64010 system controller
        * National Semiconductor PC87415 IDE controller
        * SMC91C94 ethernet controller
        * ADC0848 8 x A-to-D converters
        * 3dfx FBI with 2MB frame buffer
        * 2 x 3dfx TMU with 4MB txture memory
        * Midway I/O ASIC
        * 8MB DRAM for main CPU
        * 512KB boot ROM
        * 33MHz TMS32C031 audio CPU
        * 8MB ROM space for audio CPU
        * 512KB boot ROM

    Widget board:

        * SMC91C94 ethernet controller
        * ADC0848 8 x A-to-D converters

***************************************************************************

    Blitz '99 board:

    Seattle 5770-15206-08
        1x R5000 CPU (heatsink covering part numbers)
        1x Midway 5410-14589-00 IO chip
        1x Midway 5410-14590-00 ??
        1x Midway 5410-15349-00 Orbit 61142A ??
        1x ADSP-2115
        1x Midway security PIC Blitz 99 25" 481xxxxxx (U96)
        1x mid sized QFP (Galileo?) has heatsink (u86)
        1x mid sized QFP (PixelFX?) heakstink (u17)
        1x mid sized QFP, smaller (TexelFX?) heatsink (u87)
        12x v53c16258hk40 256Kx16 RAM (near Voodoo section)
        1x PC87415VCG IDE controller
        1x lh52256cn-70ll RAM  (near Galileo)
        1x 7can2 4k (unknown Texas Instruments SOIC)
        2x IDT 7201 LA 35J  (512x9 Async FIFO)
        1x DS232AS serial port chip
        1x Altera PLCC Seattle A-21859 (u50)
        1x Altera PLCC PAD_C1 (u60)
        3x IS61C256AH-15 (32Kx8 SRAM) near ADSP
        1x TMS418160ADZ (1Meg x 16 RAM) near ADSP
        4x TMS418160ADZ (1Meg x 16 RAM) near CPU
        1x TVP3409-17OCFN (3dfx DAC?)
        1xAD1866 audio DAC
        1x Maxim max693acwe (watchdog) near 2325 battery and Midway IO chip
        4MHz crystal attached to security PIC
        14.31818MHz crystal near 3dfx DAC
        16MHz crystal attached to ADSP
        16.6667MHz crystal near Midway IO chip
        33.3333MHz crystal near IDE chip and Galileo(PCI bus I assume)
        50MHz crystal near CPU

    Boot ROM-1.3
    Sound ROM-1.02

    Connectors:
        P2 and P6 look like PCI slots, but with no connectors soldered in, near
            3dfx/Galileo/IDE section.
        P19 is for the Daisy Dukes widget board(used by Cal Speed), and maybe
            the Carnevil gun board.
        P28 is a large 120 pin connector that is not populated, right next to
            the CPU.
        P20 is a 10 pin connector labeled "factory test".
        P1 is a 5 pin unpopulated connector marked "snd in" no idea what it
            would be for.
        P11 is a 5 pin connector marked "snd out" for line level stereo output.
        P25 is a standard IDE connector marked "Disk Drive" P15 is a laptop
            sized IDE connection right next to it.
        P9 and P10 are 14 pin connectors marked "Player 3 I/O" and player 4
            respectively.
        P16 is a 6 pin marked "Aux in"
        P3 is a 6 pin marked "Bill in"
        P8 is a 14 pin marked "Aux Latched Outputs"
        P22 is a 9 pin marked "serial port"

***************************************************************************

    Interrupt summary:

                        __________
    UART clear-to-send |          |                     __________
    -------(0x2000)--->|          |   Ethernet/Widget  |          |
                       |          |   ----(IRQ3/4/5)-->|          |
    UART data ready    |          |                    |          |
    -------(0x1000)--->|          |                    |          |
                       |          |   VSYNC            |          |
    Main-to-sound empty|  IOASIC  |   ----(IRQ3/4/5)-->|          |
    -------(0x0080)--->|          |                    |          |
                       |          |                    |          |
    Sound-to-main full |          |   IDE Controller   |   CPU    |
    -------(0x0040)--->|          |   -------(IRQ2)--->|          |
                       |          |                    |          |
    Sound FIFO empty   |          |                    |          |
    -------(0x0008)--->|          |   IOASIC Summary   |          |
                       |__________|----------(IRQ1)--->|          |
                                                       |          |
                        __________                     |          |
    Timer 3            |          |   Galileo Summary  |          |
    -------(0x0800)--->|          |----------(IRQ0)--->|          |
                       |          |                    |__________|
    Timer 2            |          |
    -------(0x0400)--->|          |
                       |          |
    Timer 1            |          |
    -------(0x0200)--->|          |
                       |          |
    Timer 0            |          |
    -------(0x0100)--->|          |
                       | Galileo  |
    DMA channel 3      |          |
    -------(0x0080)--->|          |
                       |          |
    DMA channel 2      |          |
    -------(0x0040)--->|          |
                       |          |
    DMA channel 1      |          |
    -------(0x0020)--->|          |
                       |          |
    DMA channel 0      |          |
    -------(0x0010)--->|          |
                       |__________|

**************************************************************************/

#include "emu.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/mips/mips3.h"
#include "audio/dcs.h"
#include "audio/cage.h"
#include "machine/idectrl.h"
#include "machine/midwayic.h"
#include "machine/smc91c9x.h"
#include "video/voodoo.h"



/*************************************
 *
 *  Debugging constants
 *
 *************************************/

#define LOG_GALILEO			(0)
#define LOG_TIMERS			(0)
#define LOG_DMA				(0)
#define LOG_PCI				(0)
#define LOG_WIDGET			(0)



/*************************************
 *
 *  Core constants
 *
 *************************************/

#define SYSTEM_CLOCK			50000000
#define TIMER_PERIOD			ATTOTIME_IN_HZ(SYSTEM_CLOCK)

/* various board configurations */
#define PHOENIX_CONFIG			(0)
#define SEATTLE_CONFIG			(1)
#define SEATTLE_WIDGET_CONFIG	(2)
#define FLAGSTAFF_CONFIG		(3)

/* static interrupts */
#define GALILEO_IRQ_NUM			(0)
#define IOASIC_IRQ_NUM			(1)
#define IDE_IRQ_NUM				(2)

/* configurable interrupts */
#define ETHERNET_IRQ_SHIFT		(1)
#define WIDGET_IRQ_SHIFT		(1)
#define VBLANK_IRQ_SHIFT		(7)



/*************************************
 *
 *  Galileo constants
 *
 *************************************/

/* Galileo registers - 0x000-0x3ff */
#define GREG_CPU_CONFIG		(0x000/4)
#define GREG_RAS_1_0_LO		(0x008/4)
#define GREG_RAS_1_0_HI		(0x010/4)
#define GREG_RAS_3_2_LO		(0x018/4)
#define GREG_RAS_3_2_HI		(0x020/4)
#define GREG_CS_2_0_LO		(0x028/4)
#define GREG_CS_2_0_HI		(0x030/4)
#define GREG_CS_3_BOOT_LO	(0x038/4)
#define GREG_CS_3_BOOT_HI	(0x040/4)
#define GREG_PCI_IO_LO		(0x048/4)
#define GREG_PCI_IO_HI		(0x050/4)
#define GREG_PCI_MEM_LO		(0x058/4)
#define GREG_PCI_MEM_HI		(0x060/4)
#define GREG_INTERNAL_SPACE	(0x068/4)
#define GREG_BUSERR_LO		(0x070/4)
#define GREG_BUSERR_HI		(0x078/4)

/* Galileo registers - 0x400-0x7ff */
#define GREG_RAS0_LO		(0x400/4)
#define GREG_RAS0_HI		(0x404/4)
#define GREG_RAS1_LO		(0x408/4)
#define GREG_RAS1_HI		(0x40c/4)
#define GREG_RAS2_LO		(0x410/4)
#define GREG_RAS2_HI		(0x414/4)
#define GREG_RAS3_LO		(0x418/4)
#define GREG_RAS3_HI		(0x41c/4)
#define GREG_CS0_LO			(0x420/4)
#define GREG_CS0_HI			(0x424/4)
#define GREG_CS1_LO			(0x428/4)
#define GREG_CS1_HI			(0x42c/4)
#define GREG_CS2_LO			(0x430/4)
#define GREG_CS2_HI			(0x434/4)
#define GREG_CS3_LO			(0x438/4)
#define GREG_CS3_HI			(0x43c/4)
#define GREG_CSBOOT_LO		(0x440/4)
#define GREG_CSBOOT_HI		(0x444/4)
#define GREG_DRAM_CONFIG	(0x448/4)
#define GREG_DRAM_BANK0		(0x44c/4)
#define GREG_DRAM_BANK1		(0x450/4)
#define GREG_DRAM_BANK2		(0x454/4)
#define GREG_DRAM_BANK3		(0x458/4)
#define GREG_DEVICE_BANK0	(0x45c/4)
#define GREG_DEVICE_BANK1	(0x460/4)
#define GREG_DEVICE_BANK2	(0x464/4)
#define GREG_DEVICE_BANK3	(0x468/4)
#define GREG_DEVICE_BOOT	(0x46c/4)
#define GREG_ADDRESS_ERROR	(0x470/4)

/* Galileo registers - 0x800-0xbff */
#define GREG_DMA0_COUNT		(0x800/4)
#define GREG_DMA1_COUNT		(0x804/4)
#define GREG_DMA2_COUNT		(0x808/4)
#define GREG_DMA3_COUNT		(0x80c/4)
#define GREG_DMA0_SOURCE	(0x810/4)
#define GREG_DMA1_SOURCE	(0x814/4)
#define GREG_DMA2_SOURCE	(0x818/4)
#define GREG_DMA3_SOURCE	(0x81c/4)
#define GREG_DMA0_DEST		(0x820/4)
#define GREG_DMA1_DEST		(0x824/4)
#define GREG_DMA2_DEST		(0x828/4)
#define GREG_DMA3_DEST		(0x82c/4)
#define GREG_DMA0_NEXT		(0x830/4)
#define GREG_DMA1_NEXT		(0x834/4)
#define GREG_DMA2_NEXT		(0x838/4)
#define GREG_DMA3_NEXT		(0x83c/4)
#define GREG_DMA0_CONTROL	(0x840/4)
#define GREG_DMA1_CONTROL	(0x844/4)
#define GREG_DMA2_CONTROL	(0x848/4)
#define GREG_DMA3_CONTROL	(0x84c/4)
#define GREG_TIMER0_COUNT	(0x850/4)
#define GREG_TIMER1_COUNT	(0x854/4)
#define GREG_TIMER2_COUNT	(0x858/4)
#define GREG_TIMER3_COUNT	(0x85c/4)
#define GREG_DMA_ARBITER	(0x860/4)
#define GREG_TIMER_CONTROL	(0x864/4)

/* Galileo registers - 0xc00-0xfff */
#define GREG_PCI_COMMAND	(0xc00/4)
#define GREG_PCI_TIMEOUT	(0xc04/4)
#define GREG_PCI_RAS_1_0	(0xc08/4)
#define GREG_PCI_RAS_3_2	(0xc0c/4)
#define GREG_PCI_CS_2_0		(0xc10/4)
#define GREG_PCI_CS_3_BOOT	(0xc14/4)
#define GREG_INT_STATE		(0xc18/4)
#define GREG_INT_MASK		(0xc1c/4)
#define GREG_PCI_INT_MASK	(0xc24/4)
#define GREG_CONFIG_ADDRESS	(0xcf8/4)
#define GREG_CONFIG_DATA	(0xcfc/4)

/* Galileo interrupts */
#define GINT_SUMMARY_SHIFT	(0)
#define GINT_MEMOUT_SHIFT	(1)
#define GINT_DMAOUT_SHIFT	(2)
#define GINT_CPUOUT_SHIFT	(3)
#define GINT_DMA0COMP_SHIFT	(4)
#define GINT_DMA1COMP_SHIFT	(5)
#define GINT_DMA2COMP_SHIFT	(6)
#define GINT_DMA3COMP_SHIFT	(7)
#define GINT_T0EXP_SHIFT	(8)
#define GINT_T1EXP_SHIFT	(9)
#define GINT_T2EXP_SHIFT	(10)
#define GINT_T3EXP_SHIFT	(11)
#define GINT_MASRDERR_SHIFT	(12)
#define GINT_SLVWRERR_SHIFT	(13)
#define GINT_MASWRERR_SHIFT	(14)
#define GINT_SLVRDERR_SHIFT	(15)
#define GINT_ADDRERR_SHIFT	(16)
#define GINT_MEMERR_SHIFT	(17)
#define GINT_MASABORT_SHIFT	(18)
#define GINT_TARABORT_SHIFT	(19)
#define GINT_RETRYCTR_SHIFT	(20)



/*************************************
 *
 *  Widget board constants
 *
 *************************************/

/* Widget registers */
#define WREG_ETHER_ADDR		(0x00/4)
#define WREG_INTERRUPT		(0x04/4)
#define WREG_ANALOG			(0x10/4)
#define WREG_ETHER_DATA		(0x14/4)

/* Widget interrupts */
#define WINT_ETHERNET_SHIFT	(2)



/*************************************
 *
 *  Structures
 *
 *************************************/

typedef struct _galileo_timer galileo_timer;
struct _galileo_timer
{
	emu_timer *		timer;
	UINT32			count;
	UINT8			active;
};


typedef struct _galileo_data galileo_data;
struct _galileo_data
{
	/* raw register data */
	UINT32			reg[0x1000/4];

	/* timer info */
	galileo_timer	timer[4];

	/* DMA info */
	INT8			dma_active;
	UINT8			dma_stalled_on_voodoo[4];

	/* PCI info */
	UINT32			pci_bridge_regs[0x40];
	UINT32			pci_3dfx_regs[0x40];
	UINT32			pci_ide_regs[0x40];
};


typedef struct _widget_data widget_data;
struct _widget_data
{
	/* ethernet register address */
	UINT8			ethernet_addr;

	/* IRQ information */
	UINT8			irq_num;
	UINT8			irq_mask;
};



/*************************************
 *
 *  Local variables
 *
 *************************************/

static UINT32 *rambase;
static UINT32 *rombase;

static galileo_data galileo;
static widget_data widget;

static running_device *voodoo;
static UINT8 voodoo_stalled;
static UINT8 cpu_stalled_on_voodoo;
static UINT32 cpu_stalled_offset;
static UINT32 cpu_stalled_data;
static UINT32 cpu_stalled_mem_mask;

static UINT8 board_config;

static UINT8 ethernet_irq_num;
static UINT8 ethernet_irq_state;

static UINT8 vblank_irq_num;
static UINT8 vblank_latch;
static UINT8 vblank_state;
static UINT32 *interrupt_config;
static UINT32 *interrupt_enable;

static UINT32 *asic_reset;

static UINT8 pending_analog_read;
static UINT8 status_leds;

static UINT32 cmos_write_enabled;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void vblank_assert(running_device *device, int state);
static void update_vblank_irq(running_machine *machine);
static void galileo_reset(void);
static TIMER_CALLBACK( galileo_timer_callback );
static void galileo_perform_dma(const address_space *space, int which);
static void voodoo_stall(running_device *device, int stall);
static void widget_reset(running_machine *machine);
static void update_widget_irq(running_machine *machine);



/*************************************
 *
 *  Video start and update
 *
 *************************************/

static VIDEO_UPDATE( seattle )
{
	return voodoo_update(voodoo, bitmap, cliprect) ? 0 : UPDATE_HAS_NOT_CHANGED;
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

static MACHINE_START( seattle )
{
	int index;

	voodoo = machine->device("voodoo");

	/* allocate timers for the galileo */
	galileo.timer[0].timer = timer_alloc(machine, galileo_timer_callback, NULL);
	galileo.timer[1].timer = timer_alloc(machine, galileo_timer_callback, NULL);
	galileo.timer[2].timer = timer_alloc(machine, galileo_timer_callback, NULL);
	galileo.timer[3].timer = timer_alloc(machine, galileo_timer_callback, NULL);

	/* set the fastest DRC options, but strict verification */
	mips3drc_set_options(machine->device("maincpu"), MIPS3DRC_FASTEST_OPTIONS + MIPS3DRC_STRICT_VERIFY);

	/* configure fast RAM regions for DRC */
	mips3drc_add_fastram(machine->device("maincpu"), 0x00000000, 0x007fffff, FALSE, rambase);
	mips3drc_add_fastram(machine->device("maincpu"), 0x1fc00000, 0x1fc7ffff, TRUE,  rombase);

	/* register for save states */
	state_save_register_global_array(machine, galileo.reg);
	state_save_register_global(machine, galileo.dma_active);
	state_save_register_global_array(machine, galileo.dma_stalled_on_voodoo);
	state_save_register_global_array(machine, galileo.pci_bridge_regs);
	state_save_register_global_array(machine, galileo.pci_3dfx_regs);
	state_save_register_global_array(machine, galileo.pci_ide_regs);
	for (index = 0; index < ARRAY_LENGTH(galileo.timer); index++)
	{
		state_save_register_item(machine, "galileo", NULL, index, galileo.timer[index].count);
		state_save_register_item(machine, "galileo", NULL, index, galileo.timer[index].active);
	}
	state_save_register_global(machine, widget.ethernet_addr);
	state_save_register_global(machine, widget.irq_num);
	state_save_register_global(machine, widget.irq_mask);
	state_save_register_global(machine, voodoo_stalled);
	state_save_register_global(machine, cpu_stalled_on_voodoo);
	state_save_register_global(machine, cpu_stalled_offset);
	state_save_register_global(machine, cpu_stalled_data);
	state_save_register_global(machine, cpu_stalled_mem_mask);
	state_save_register_global(machine, board_config);
	state_save_register_global(machine, ethernet_irq_num);
	state_save_register_global(machine, ethernet_irq_state);
	state_save_register_global(machine, vblank_irq_num);
	state_save_register_global(machine, vblank_latch);
	state_save_register_global(machine, vblank_state);
	state_save_register_global(machine, pending_analog_read);
	state_save_register_global(machine, status_leds);
	state_save_register_global(machine, cmos_write_enabled);
}


static MACHINE_RESET( seattle )
{
	galileo.dma_active = -1;

	vblank_irq_num = 0;
	voodoo_stalled = FALSE;
	cpu_stalled_on_voodoo = FALSE;

	/* reset either the DCS2 board or the CAGE board */
	if (machine->device("dcs2") != NULL)
	{
		dcs_reset_w(1);
		dcs_reset_w(0);
	}
	else if (machine->device("cage") != NULL)
	{
		cage_control_w(machine, 0);
		cage_control_w(machine, 3);
	}

	/* reset the other devices */
	galileo_reset();
	if (board_config == SEATTLE_WIDGET_CONFIG)
		widget_reset(machine);
}



/*************************************
 *
 *  IDE interrupts
 *
 *************************************/

static void ide_interrupt(running_device *device, int state)
{
	cputag_set_input_line(device->machine, "maincpu", IDE_IRQ_NUM, state);
}



/*************************************
 *
 *  Ethernet interrupts
 *
 *************************************/

static void ethernet_interrupt_machine(running_machine *machine, int state)
{
	ethernet_irq_state = state;
	if (board_config == FLAGSTAFF_CONFIG)
	{
		UINT8 assert = ethernet_irq_state && (*interrupt_enable & (1 << ETHERNET_IRQ_SHIFT));
		if (ethernet_irq_num != 0)
			cputag_set_input_line(machine, "maincpu", ethernet_irq_num, assert ? ASSERT_LINE : CLEAR_LINE);
	}
	else if (board_config == SEATTLE_WIDGET_CONFIG)
		update_widget_irq(machine);
}

static void ethernet_interrupt(running_device *device, int state)
{
	ethernet_interrupt_machine(device->machine, state);
}



/*************************************
 *
 *  I/O ASIC interrupts
 *
 *************************************/

static void ioasic_irq(running_machine *machine, int state)
{
	cputag_set_input_line(machine, "maincpu", IOASIC_IRQ_NUM, state);
}



/*************************************
 *
 *  Configurable interrupts
 *
 *************************************/

static READ32_HANDLER( interrupt_state_r )
{
	UINT32 result = 0;
	result |= ethernet_irq_state << ETHERNET_IRQ_SHIFT;
	result |= vblank_latch << VBLANK_IRQ_SHIFT;
	return result;
}


static READ32_HANDLER( interrupt_state2_r )
{
	UINT32 result = interrupt_state_r(space, offset, mem_mask);
	result |= vblank_state << 8;
	return result;
}


static WRITE32_HANDLER( interrupt_config_w )
{
	int irq;
	COMBINE_DATA(interrupt_config);

	/* VBLANK: clear anything pending on the old IRQ */
	if (vblank_irq_num != 0)
		cputag_set_input_line(space->machine, "maincpu", vblank_irq_num, CLEAR_LINE);

	/* VBLANK: compute the new IRQ vector */
	irq = (*interrupt_config >> (2*VBLANK_IRQ_SHIFT)) & 3;
	vblank_irq_num = (irq != 0) ? (2 + irq) : 0;

	/* Widget board case */
	if (board_config == SEATTLE_WIDGET_CONFIG)
	{
		/* Widget: clear anything pending on the old IRQ */
		if (widget.irq_num != 0)
			cputag_set_input_line(space->machine, "maincpu", widget.irq_num, CLEAR_LINE);

		/* Widget: compute the new IRQ vector */
		irq = (*interrupt_config >> (2*WIDGET_IRQ_SHIFT)) & 3;
		widget.irq_num = (irq != 0) ? (2 + irq) : 0;
	}

	/* Flagstaff board case */
	if (board_config == FLAGSTAFF_CONFIG)
	{
		/* Ethernet: clear anything pending on the old IRQ */
		if (ethernet_irq_num != 0)
			cputag_set_input_line(space->machine, "maincpu", ethernet_irq_num, CLEAR_LINE);

		/* Ethernet: compute the new IRQ vector */
		irq = (*interrupt_config >> (2*ETHERNET_IRQ_SHIFT)) & 3;
		ethernet_irq_num = (irq != 0) ? (2 + irq) : 0;
	}

	/* update the states */
	update_vblank_irq(space->machine);
	ethernet_interrupt_machine(space->machine, ethernet_irq_state);
}


static WRITE32_HANDLER( seattle_interrupt_enable_w )
{
	UINT32 old = *interrupt_enable;
	COMBINE_DATA(interrupt_enable);
	if (old != *interrupt_enable)
	{
		if (vblank_latch)
			update_vblank_irq(space->machine);
		if (ethernet_irq_state)
			ethernet_interrupt_machine(space->machine, ethernet_irq_state);
	}
}



/*************************************
 *
 *  VBLANK interrupts
 *
 *************************************/

static void update_vblank_irq(running_machine *machine)
{
	int state = CLEAR_LINE;

	/* skip if no interrupt configured */
	if (vblank_irq_num == 0)
		return;

	/* if the VBLANK has been latched, and the interrupt is enabled, assert */
	if (vblank_latch && (*interrupt_enable & (1 << VBLANK_IRQ_SHIFT)))
		state = ASSERT_LINE;
	cputag_set_input_line(machine, "maincpu", vblank_irq_num, state);
}


static WRITE32_HANDLER( vblank_clear_w )
{
	/* clear the latch and update the IRQ */
	vblank_latch = 0;
	update_vblank_irq(space->machine);
}


static void vblank_assert(running_device *device, int state)
{
	/* cache the raw state */
	vblank_state = state;

	/* latch on the correct polarity transition */
	if ((state && !(*interrupt_enable & 0x100)) || (!state && (*interrupt_enable & 0x100)))
	{
		vblank_latch = 1;
		update_vblank_irq(device->machine);
	}
}



/*************************************
 *
 *  PCI bridge accesses
 *
 *************************************/

static UINT32 pci_bridge_r(const address_space *space, UINT8 reg, UINT8 type)
{
	UINT32 result = galileo.pci_bridge_regs[reg];

	switch (reg)
	{
		case 0x00:		/* ID register: 0x0146 = GT64010, 0x11ab = Galileo */
			result = 0x014611ab;
			break;

		case 0x02:		/* Base Class:Sub Class:Reserved:Revision */
			result = 0x06000003;
			break;
	}

	if (LOG_PCI)
		logerror("%08X:PCI bridge read: reg %d type %d = %08X\n", cpu_get_pc(space->cpu), reg, type, result);
	return result;
}


static void pci_bridge_w(const address_space *space, UINT8 reg, UINT8 type, UINT32 data)
{
	galileo.pci_bridge_regs[reg] = data;
	if (LOG_PCI)
		logerror("%08X:PCI bridge write: reg %d type %d = %08X\n", cpu_get_pc(space->cpu), reg, type, data);
}



/*************************************
 *
 *  PCI 3dfx accesses
 *
 *************************************/

static UINT32 pci_3dfx_r(const address_space *space, UINT8 reg, UINT8 type)
{
	UINT32 result = galileo.pci_3dfx_regs[reg];

	switch (reg)
	{
		case 0x00:		/* ID register: 0x0001 = SST-1, 0x121a = 3dfx */
			result = 0x0001121a;
			break;

		case 0x02:		/* Base Class:Sub Class:Reserved:Revision */
			result = 0x00000001;
			break;
	}

	if (LOG_PCI)
		logerror("%08X:PCI 3dfx read: reg %d type %d = %08X\n", cpu_get_pc(space->cpu), reg, type, result);
	return result;
}


static void pci_3dfx_w(const address_space *space, UINT8 reg, UINT8 type, UINT32 data)
{
	galileo.pci_3dfx_regs[reg] = data;

	switch (reg)
	{
		case 0x04:		/* address register */
			galileo.pci_3dfx_regs[reg] &= 0xff000000;
			if (data != 0x08000000)
				logerror("3dfx not mapped where we expect it! (%08X)\n", data);
			break;

		case 0x10:		/* initEnable register */
			voodoo_set_init_enable(voodoo, data);
			break;
	}
	if (LOG_PCI)
		logerror("%08X:PCI 3dfx write: reg %d type %d = %08X\n", cpu_get_pc(space->cpu), reg, type, data);
}



/*************************************
 *
 *  PCI IDE accesses
 *
 *************************************/

static UINT32 pci_ide_r(const address_space *space, UINT8 reg, UINT8 type)
{
	UINT32 result = galileo.pci_ide_regs[reg];

	switch (reg)
	{
		case 0x00:		/* ID register: 0x0002 = PC87415, 0x100b = National Semiconductor */
			result = 0x0002100b;
			break;

		case 0x02:		/* Base Class:Sub Class:Reserved:Revision */
			result = 0x01010001;
			break;
	}

	if (LOG_PCI)
		logerror("%08X:PCI IDE read: reg %d type %d = %08X\n", cpu_get_pc(space->cpu), reg, type, result);
	return result;
}


static void pci_ide_w(const address_space *space, UINT8 reg, UINT8 type, UINT32 data)
{
	galileo.pci_ide_regs[reg] = data;
	if (LOG_PCI)
		logerror("%08X:PCI bridge write: reg %d type %d = %08X\n", cpu_get_pc(space->cpu), reg, type, data);
}



/*************************************
 *
 *  Galileo timers & interrupts
 *
 *************************************/

static void update_galileo_irqs(running_machine *machine)
{
	int state = CLEAR_LINE;

	/* if any unmasked interrupts are live, we generate */
	if (galileo.reg[GREG_INT_STATE] & galileo.reg[GREG_INT_MASK])
		state = ASSERT_LINE;
	cputag_set_input_line(machine, "maincpu", GALILEO_IRQ_NUM, state);

	if (LOG_GALILEO)
		logerror("Galileo IRQ %s\n", (state == ASSERT_LINE) ? "asserted" : "cleared");
}


static TIMER_CALLBACK( galileo_timer_callback )
{
	int which = param;
	galileo_timer *timer = &galileo.timer[which];

	if (LOG_TIMERS)
		logerror("timer %d fired\n", which);

	/* copy the start value from the registers */
	timer->count = galileo.reg[GREG_TIMER0_COUNT + which];
	if (which != 0)
		timer->count &= 0xffffff;

	/* if we're a timer, adjust the timer to fire again */
	if (galileo.reg[GREG_TIMER_CONTROL] & (2 << (2 * which)))
		timer_adjust_oneshot(timer->timer, attotime_mul(TIMER_PERIOD, timer->count), which);
	else
		timer->active = timer->count = 0;

	/* trigger the interrupt */
	galileo.reg[GREG_INT_STATE] |= 1 << (GINT_T0EXP_SHIFT + which);
	update_galileo_irqs(machine);
}



/*************************************
 *
 *  Galileo DMA handler
 *
 *************************************/

static int galileo_dma_fetch_next(const address_space *space, int which)
{
	offs_t address = 0;
	UINT32 data;

	/* no-op for unchained mode */
	if (!(galileo.reg[GREG_DMA0_CONTROL + which] & 0x200))
		address = galileo.reg[GREG_DMA0_NEXT + which];

	/* if we hit the end address, signal an interrupt */
	if (address == 0)
	{
		if (galileo.reg[GREG_DMA0_CONTROL + which] & 0x400)
		{
			galileo.reg[GREG_INT_STATE] |= 1 << (GINT_DMA0COMP_SHIFT + which);
			update_galileo_irqs(space->machine);
		}
		galileo.reg[GREG_DMA0_CONTROL + which] &= ~0x5000;
		return 0;
	}

	/* fetch the byte count */
	data = memory_read_dword(space, address); address += 4;
	galileo.reg[GREG_DMA0_COUNT + which] = data;

	/* fetch the source address */
	data = memory_read_dword(space, address); address += 4;
	galileo.reg[GREG_DMA0_SOURCE + which] = data;

	/* fetch the dest address */
	data = memory_read_dword(space, address); address += 4;
	galileo.reg[GREG_DMA0_DEST + which] = data;

	/* fetch the next record address */
	data = memory_read_dword(space, address); address += 4;
	galileo.reg[GREG_DMA0_NEXT + which] = data;
	return 1;
}


static void galileo_perform_dma(const address_space *space, int which)
{
	do
	{
		offs_t srcaddr = galileo.reg[GREG_DMA0_SOURCE + which];
		offs_t dstaddr = galileo.reg[GREG_DMA0_DEST + which];
		UINT32 bytesleft = galileo.reg[GREG_DMA0_COUNT + which] & 0xffff;
		int srcinc, dstinc;

		galileo.dma_active = which;
		galileo.reg[GREG_DMA0_CONTROL + which] |= 0x5000;

		/* determine src/dst inc */
		switch ((galileo.reg[GREG_DMA0_CONTROL + which] >> 2) & 3)
		{
			default:
			case 0:		srcinc = 1;		break;
			case 1:		srcinc = -1;	break;
			case 2:		srcinc = 0;		break;
		}
		switch ((galileo.reg[GREG_DMA0_CONTROL + which] >> 4) & 3)
		{
			default:
			case 0:		dstinc = 1;		break;
			case 1:		dstinc = -1;	break;
			case 2:		dstinc = 0;		break;
		}

		if (LOG_DMA)
			logerror("Performing DMA%d: src=%08X dst=%08X bytes=%04X sinc=%d dinc=%d\n", which, srcaddr, dstaddr, bytesleft, srcinc, dstinc);

		/* special case: transfer to voodoo */
		if (dstaddr >= 0x08000000 && dstaddr < 0x09000000)
		{
			if (bytesleft % 4 != 0)
				fatalerror("Galileo DMA to voodoo: bytesleft = %d", bytesleft);
			srcinc *= 4;
			dstinc *= 4;

			/* transfer data */
			while (bytesleft >= 4)
			{
				/* if the voodoo is stalled, stop early */
				if (voodoo_stalled)
				{
					if (LOG_DMA)
						logerror("Stalled on voodoo with %d bytes left\n", bytesleft);
					break;
				}

				/* write the data and advance */
				voodoo_w(voodoo, (dstaddr & 0xffffff) / 4, memory_read_dword(space, srcaddr), 0xffffffff);
				srcaddr += srcinc;
				dstaddr += dstinc;
				bytesleft -= 4;
			}
		}

		/* standard transfer */
		else
		{
			while (bytesleft > 0)
			{
				memory_write_byte(space, dstaddr, memory_read_byte(space, srcaddr));
				srcaddr += srcinc;
				dstaddr += dstinc;
				bytesleft--;
			}
		}

		/* not verified, but seems logical these should be updated byte the end */
		galileo.reg[GREG_DMA0_SOURCE + which] = srcaddr;
		galileo.reg[GREG_DMA0_DEST + which] = dstaddr;
		galileo.reg[GREG_DMA0_COUNT + which] = (galileo.reg[GREG_DMA0_COUNT + which] & ~0xffff) | bytesleft;
		galileo.dma_active = -1;

		/* if we did not hit zero, punt and return later */
		if (bytesleft != 0)
			return;

		/* interrupt? */
		if (!(galileo.reg[GREG_DMA0_CONTROL + which] & 0x400))
		{
			galileo.reg[GREG_INT_STATE] |= 1 << (GINT_DMA0COMP_SHIFT + which);
			update_galileo_irqs(space->machine);
		}
	} while (galileo_dma_fetch_next(space, which));

	galileo.reg[GREG_DMA0_CONTROL + which] &= ~0x5000;
}



/*************************************
 *
 *  Galileo system controller
 *
 *************************************/

static void galileo_reset(void)
{
	memset(&galileo.reg, 0, sizeof(galileo.reg));
}


static READ32_HANDLER( galileo_r )
{
	UINT32 result = galileo.reg[offset];

	/* switch off the offset for special cases */
	switch (offset)
	{
		case GREG_TIMER0_COUNT:
		case GREG_TIMER1_COUNT:
		case GREG_TIMER2_COUNT:
		case GREG_TIMER3_COUNT:
		{
			int which = offset % 4;
			galileo_timer *timer = &galileo.timer[which];

			result = timer->count;
			if (timer->active)
			{
				UINT32 elapsed = attotime_to_double(attotime_mul(timer_timeelapsed(timer->timer), SYSTEM_CLOCK));
				result = (result > elapsed) ? (result - elapsed) : 0;
			}

			/* eat some time for those which poll this register */
			cpu_eat_cycles(space->cpu, 100);

			if (LOG_TIMERS)
				logerror("%08X:hires_timer_r = %08X\n", cpu_get_pc(space->cpu), result);
			break;
		}

		case GREG_PCI_COMMAND:
			// code at 40188 loops until this returns non-zero in bit 0
			result = 0x0001;
			break;

		case GREG_CONFIG_DATA:
		{
			int bus = (galileo.reg[GREG_CONFIG_ADDRESS] >> 16) & 0xff;
			int unit = (galileo.reg[GREG_CONFIG_ADDRESS] >> 11) & 0x1f;
			int func = (galileo.reg[GREG_CONFIG_ADDRESS] >> 8) & 7;
			int reg = (galileo.reg[GREG_CONFIG_ADDRESS] >> 2) & 0x3f;
			int type = galileo.reg[GREG_CONFIG_ADDRESS] & 3;

			/* unit 0 is the PCI bridge */
			if (unit == 0 && func == 0)
				result = pci_bridge_r(space, reg, type);

			/* unit 8 is the 3dfx card */
			else if (unit == 8 && func == 0)
				result = pci_3dfx_r(space, reg, type);

			/* unit 9 is the IDE controller */
			else if (unit == 9 && func == 0)
				result = pci_ide_r(space, reg, type);

			/* anything else, just log */
			else
			{
				result = ~0;
				logerror("%08X:PCIBus read: bus %d unit %d func %d reg %d type %d = %08X\n", cpu_get_pc(space->cpu), bus, unit, func, reg, type, result);
			}
			break;
		}

		case GREG_CONFIG_ADDRESS:
		case GREG_INT_STATE:
		case GREG_INT_MASK:
		case GREG_TIMER_CONTROL:
//          if (LOG_GALILEO)
//              logerror("%08X:Galileo read from offset %03X = %08X\n", cpu_get_pc(space->cpu), offset*4, result);
			break;

		default:
			logerror("%08X:Galileo read from offset %03X = %08X\n", cpu_get_pc(space->cpu), offset*4, result);
			break;
	}

	return result;
}


static WRITE32_HANDLER( galileo_w )
{
	UINT32 oldata = galileo.reg[offset];
	COMBINE_DATA(&galileo.reg[offset]);

	/* switch off the offset for special cases */
	switch (offset)
	{
		case GREG_DMA0_CONTROL:
		case GREG_DMA1_CONTROL:
		case GREG_DMA2_CONTROL:
		case GREG_DMA3_CONTROL:
		{
			int which = offset % 4;

			if (LOG_DMA)
				logerror("%08X:Galileo write to offset %03X = %08X & %08X\n", cpu_get_pc(space->cpu), offset*4, data, mem_mask);

			/* keep the read only activity bit */
			galileo.reg[offset] &= ~0x4000;
			galileo.reg[offset] |= (oldata & 0x4000);

			/* fetch next record */
			if (data & 0x2000)
				galileo_dma_fetch_next(space, which);
			galileo.reg[offset] &= ~0x2000;

			/* if enabling, start the DMA */
			if (!(oldata & 0x1000) && (data & 0x1000))
				galileo_perform_dma(space, which);
			break;
		}

		case GREG_TIMER0_COUNT:
		case GREG_TIMER1_COUNT:
		case GREG_TIMER2_COUNT:
		case GREG_TIMER3_COUNT:
		{
			int which = offset % 4;
			galileo_timer *timer = &galileo.timer[which];

			if (which != 0)
				data &= 0xffffff;
			if (!timer->active)
				timer->count = data;
			if (LOG_TIMERS)
				logerror("%08X:timer/counter %d count = %08X [start=%08X]\n", cpu_get_pc(space->cpu), offset % 4, data, timer->count);
			break;
		}

		case GREG_TIMER_CONTROL:
		{
			int which, mask;

			if (LOG_TIMERS)
				logerror("%08X:timer/counter control = %08X\n", cpu_get_pc(space->cpu), data);
			for (which = 0, mask = 0x01; which < 4; which++, mask <<= 2)
			{
				galileo_timer *timer = &galileo.timer[which];
				if (!timer->active && (data & mask))
				{
					timer->active = 1;
					if (timer->count == 0)
					{
						timer->count = galileo.reg[GREG_TIMER0_COUNT + which];
						if (which != 0)
							timer->count &= 0xffffff;
					}
					timer_adjust_oneshot(timer->timer, attotime_mul(TIMER_PERIOD, timer->count), which);
					if (LOG_TIMERS)
						logerror("Adjusted timer to fire in %f secs\n", attotime_to_double(attotime_mul(TIMER_PERIOD, timer->count)));
				}
				else if (timer->active && !(data & mask))
				{
					UINT32 elapsed = attotime_to_double(attotime_mul(timer_timeelapsed(timer->timer), SYSTEM_CLOCK));
					timer->active = 0;
					timer->count = (timer->count > elapsed) ? (timer->count - elapsed) : 0;
					timer_adjust_oneshot(timer->timer, attotime_never, which);
					if (LOG_TIMERS)
						logerror("Disabled timer\n");
				}
			}
			break;
		}

		case GREG_INT_STATE:
			if (LOG_GALILEO)
				logerror("%08X:Galileo write to IRQ clear = %08X & %08X\n", offset*4, data, mem_mask);
			galileo.reg[offset] = oldata & data;
			update_galileo_irqs(space->machine);
			break;

		case GREG_CONFIG_DATA:
		{
			int bus = (galileo.reg[GREG_CONFIG_ADDRESS] >> 16) & 0xff;
			int unit = (galileo.reg[GREG_CONFIG_ADDRESS] >> 11) & 0x1f;
			int func = (galileo.reg[GREG_CONFIG_ADDRESS] >> 8) & 7;
			int reg = (galileo.reg[GREG_CONFIG_ADDRESS] >> 2) & 0x3f;
			int type = galileo.reg[GREG_CONFIG_ADDRESS] & 3;

			/* unit 0 is the PCI bridge */
			if (unit == 0 && func == 0)
				pci_bridge_w(space, reg, type, data);

			/* unit 8 is the 3dfx card */
			else if (unit == 8 && func == 0)
				pci_3dfx_w(space, reg, type, data);

			/* unit 9 is the IDE controller */
			else if (unit == 9 && func == 0)
				pci_ide_w(space, reg, type, data);

			/* anything else, just log */
			else
				logerror("%08X:PCIBus write: bus %d unit %d func %d reg %d type %d = %08X\n", cpu_get_pc(space->cpu), bus, unit, func, reg, type, data);
			break;
		}

		case GREG_DMA0_COUNT:	case GREG_DMA1_COUNT:	case GREG_DMA2_COUNT:	case GREG_DMA3_COUNT:
		case GREG_DMA0_SOURCE:	case GREG_DMA1_SOURCE:	case GREG_DMA2_SOURCE:	case GREG_DMA3_SOURCE:
		case GREG_DMA0_DEST:	case GREG_DMA1_DEST:	case GREG_DMA2_DEST:	case GREG_DMA3_DEST:
		case GREG_DMA0_NEXT:	case GREG_DMA1_NEXT:	case GREG_DMA2_NEXT:	case GREG_DMA3_NEXT:
		case GREG_CONFIG_ADDRESS:
		case GREG_INT_MASK:
			if (LOG_GALILEO)
				logerror("%08X:Galileo write to offset %03X = %08X & %08X\n", cpu_get_pc(space->cpu), offset*4, data, mem_mask);
			break;

		default:
			logerror("%08X:Galileo write to offset %03X = %08X & %08X\n", cpu_get_pc(space->cpu), offset*4, data, mem_mask);
			break;
	}
}



/*************************************
 *
 *  Voodoo handling
 *
 *************************************/

static WRITE32_HANDLER( seattle_voodoo_w )
{
	/* if we're not stalled, just write and get out */
	if (!voodoo_stalled)
	{
		voodoo_w(voodoo, offset, data, mem_mask);
		return;
	}

	/* shouldn't get here if the CPU is already stalled */
	if (cpu_stalled_on_voodoo)
		fatalerror("seattle_voodoo_w while CPU is stalled");

	/* remember all the info about this access for later */
	cpu_stalled_on_voodoo = TRUE;
	cpu_stalled_offset = offset;
	cpu_stalled_data = data;
	cpu_stalled_mem_mask = mem_mask;

	/* spin until we send the magic trigger */
	cpu_spinuntil_trigger(space->cpu, 45678);
	if (LOG_DMA) logerror("%08X:Stalling CPU on voodoo (already stalled)\n", cpu_get_pc(space->cpu));
}


static void voodoo_stall(running_device *device, int stall)
{
	/* set the new state */
	voodoo_stalled = stall;

	/* if we're stalling and DMA is active, take note */
	if (stall)
	{
		if (galileo.dma_active != -1)
		{
			if (LOG_DMA) logerror("Stalling DMA%d on voodoo\n", galileo.dma_active);
			galileo.dma_stalled_on_voodoo[galileo.dma_active] = TRUE;
		}
		else
		{
			if (LOG_DMA) logerror("%08X:Stalling CPU on voodoo\n", cpu_get_pc(device->machine->device("maincpu")));
			cpu_spinuntil_trigger(device->machine->device("maincpu"), 45678);
		}
	}

	/* if we're unstalling, resume DMA or allow the CPU to proceed */
	else
	{
		int which;

		/* loop over any active DMAs and resume them */
		for (which = 0; which < 4; which++)
			if (galileo.dma_stalled_on_voodoo[which])
			{
				const address_space *space = cputag_get_address_space(device->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
				if (LOG_DMA) logerror("Resuming DMA%d on voodoo\n", which);

				/* mark this DMA as no longer stalled */
				galileo.dma_stalled_on_voodoo[which] = FALSE;

				/* resume execution */
				galileo_perform_dma(space, which);
				break;
			}

		/* if we finished all our pending DMAs, then we can resume CPU operations */
		if (!voodoo_stalled)
		{
			/* if the CPU had a pending write, do it now */
			if (cpu_stalled_on_voodoo)
				voodoo_w(device, cpu_stalled_offset, cpu_stalled_data, cpu_stalled_mem_mask);
			cpu_stalled_on_voodoo = FALSE;

			/* resume CPU execution */
			if (LOG_DMA) logerror("Resuming CPU on voodoo\n");
			cpuexec_trigger(device->machine, 45678);
		}
	}
}



/*************************************
 *
 *  Analog input handling (ADC0848)
 *
 *************************************/

static READ32_HANDLER( analog_port_r )
{
	return pending_analog_read;
}


static WRITE32_HANDLER( analog_port_w )
{
	static const char *const portnames[] = { "AN0", "AN1", "AN2", "AN3", "AN4", "AN5", "AN6", "AN7" };

	if (data < 8 || data > 15)
		logerror("%08X:Unexpected analog port select = %08X\n", cpu_get_pc(space->cpu), data);
	pending_analog_read = input_port_read(space->machine, portnames[data & 7]);
}



/*************************************
 *
 *  CarnEvil gun handling
 *
 *************************************/

static READ32_HANDLER( carnevil_gun_r )
{
	UINT32 result = 0;

	switch (offset)
	{
		case 0:		/* low 8 bits of X */
			result = (input_port_read(space->machine, "LIGHT0_X") << 4) & 0xff;
			break;

		case 1:		/* upper 4 bits of X */
			result = (input_port_read(space->machine, "LIGHT0_X") >> 4) & 0x0f;
			result |= (input_port_read(space->machine, "FAKE") & 0x03) << 4;
			result |= 0x40;
			break;

		case 2:		/* low 8 bits of Y */
			result = (input_port_read(space->machine, "LIGHT0_Y") << 2) & 0xff;
			break;

		case 3:		/* upper 4 bits of Y */
			result = (input_port_read(space->machine, "LIGHT0_Y") >> 6) & 0x03;
			break;

		case 4:		/* low 8 bits of X */
			result = (input_port_read(space->machine, "LIGHT1_X") << 4) & 0xff;
			break;

		case 5:		/* upper 4 bits of X */
			result = (input_port_read(space->machine, "LIGHT1_X") >> 4) & 0x0f;
			result |= (input_port_read(space->machine, "FAKE") & 0x30);
			result |= 0x40;
			break;

		case 6:		/* low 8 bits of Y */
			result = (input_port_read(space->machine, "LIGHT1_Y") << 2) & 0xff;
			break;

		case 7:		/* upper 4 bits of Y */
			result = (input_port_read(space->machine, "LIGHT1_Y") >> 6) & 0x03;
			break;
	}
	return result;
}


static WRITE32_HANDLER( carnevil_gun_w )
{
	logerror("carnevil_gun_w(%d) = %02X\n", offset, data);
}



/*************************************
 *
 *  Ethernet access
 *
 *************************************/

static READ32_DEVICE_HANDLER( ethernet_r )
{
	if (!(offset & 8))
		return smc91c9x_r(device, offset & 7, mem_mask & 0xffff);
	else
		return smc91c9x_r(device, offset & 7, mem_mask & 0x00ff);
}


static WRITE32_DEVICE_HANDLER( ethernet_w )
{
	if (!(offset & 8))
		smc91c9x_w(device, offset & 7, data & 0xffff, mem_mask | 0xffff);
	else
		smc91c9x_w(device, offset & 7, data & 0x00ff, mem_mask | 0x00ff);
}



/*************************************
 *
 *  Widget board access
 *
 *************************************/

static void widget_reset(running_machine *machine)
{
	UINT8 saved_irq = widget.irq_num;
	memset(&widget, 0, sizeof(widget));
	widget.irq_num = saved_irq;
}


static void update_widget_irq(running_machine *machine)
{
	UINT8 state = ethernet_irq_state << WINT_ETHERNET_SHIFT;
	UINT8 mask = widget.irq_mask;
	UINT8 assert = ((mask & state) != 0) && (*interrupt_enable & (1 << WIDGET_IRQ_SHIFT));

	/* update the IRQ state */
	if (widget.irq_num != 0)
		cputag_set_input_line(machine, "maincpu", widget.irq_num, assert ? ASSERT_LINE : CLEAR_LINE);
}


static READ32_DEVICE_HANDLER( widget_r )
{
	UINT32 result = ~0;

	switch (offset)
	{
		case WREG_ETHER_ADDR:
			result = widget.ethernet_addr;
			break;

		case WREG_INTERRUPT:
			result = ethernet_irq_state << WINT_ETHERNET_SHIFT;
			result = ~result;
			break;

		case WREG_ANALOG:
			result = analog_port_r(cputag_get_address_space(device->machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0, mem_mask);
			break;

		case WREG_ETHER_DATA:
			result = smc91c9x_r(device, widget.ethernet_addr & 7, mem_mask & 0xffff);
			break;
	}

	if (LOG_WIDGET)
		logerror("Widget read (%02X) = %08X & %08X\n", offset*4, result, mem_mask);
	return result;
}


static WRITE32_DEVICE_HANDLER( widget_w )
{
	if (LOG_WIDGET)
		logerror("Widget write (%02X) = %08X & %08X\n", offset*4, data, mem_mask);

	switch (offset)
	{
		case WREG_ETHER_ADDR:
			widget.ethernet_addr = data;
			break;

		case WREG_INTERRUPT:
			widget.irq_mask = data;
			update_widget_irq(device->machine);
			break;

		case WREG_ANALOG:
			analog_port_w(cputag_get_address_space(device->machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0, data, mem_mask);
			break;

		case WREG_ETHER_DATA:
			smc91c9x_w(device, widget.ethernet_addr & 7, data & 0xffff, mem_mask & 0xffff);
			break;
	}
}



/*************************************
 *
 *  CMOS access
 *
 *************************************/

static WRITE32_HANDLER( cmos_w )
{
	if (cmos_write_enabled)
		COMBINE_DATA(space->machine->generic.nvram.u32 + offset);
	cmos_write_enabled = FALSE;
}


static READ32_HANDLER( cmos_r )
{
	return space->machine->generic.nvram.u32[offset];
}


static WRITE32_HANDLER( cmos_protect_w )
{
	cmos_write_enabled = TRUE;
}


static READ32_HANDLER( cmos_protect_r )
{
	return cmos_write_enabled;
}



/*************************************
 *
 *  Misc accesses
 *
 *************************************/

static WRITE32_HANDLER( seattle_watchdog_w )
{
	cpu_eat_cycles(space->cpu, 100);
}


static WRITE32_HANDLER( asic_reset_w )
{
	COMBINE_DATA(asic_reset);
	if (!(*asic_reset & 0x0002))
		midway_ioasic_reset(space->machine);
}


static WRITE32_HANDLER( asic_fifo_w )
{
	midway_ioasic_fifo_w(space->machine, data);
}


static READ32_HANDLER( status_leds_r )
{
	return status_leds | 0xffffff00;
}


static WRITE32_HANDLER( status_leds_w )
{
	if (ACCESSING_BITS_0_7)
		status_leds = data;
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

/*

    WG3DH config:

RAS[1:0] = 00000000-007FFFFF
  RAS[0] = 00000000-001FFFFF
  RAS[1] = 00200000-003FFFFF

RAS[3:2] = 04000000-07FFFFFF
  RAS[2] = 04000000-05FFFFFF
  RAS[3] = 06000000-07FFFFFF

PCI I/O  = 0A000000-0BFFFFFF
PCI Mem  = 08000000-09FFFFFF

 CS[2:0] = 10000000-15FFFFFF
   CS[0] = 10000000-11FFFFFF
   CS[1] = 12000000-13FFFFFF
   CS[2] = 14000000-15FFFFFF

 CS[3]/B = 16000000-1FFFFFFF
   CS[3] = 16000000-17FFFFFF



    Carnevil config:

RAS[1:0] = 00000000-03FFFFFF
  RAS[0] = 00000000-00BFFFFF
  RAS[1] = 00100000-03FFFFFF

RAS[3:2] = 04000000-07FFFFFF
  RAS[2] = 04000000-05FFFFFF
  RAS[3] = 06000000-07FFFFFF

PCI I/O  = 0A000000-0BFFFFFF
PCI Mem  = 08000000-09FFFFFF

 CS[2:0] = 10000000-15FFFFFF
   CS[0] = 10000000-11FFFFFF
   CS[1] = 12000000-13FFFFFF
   CS[2] = 14000000-15FFFFFF

 CS[3]/B = 16000000-1FFFFFFF
   CS[3] = 16000000-17FFFFFF



    SFRush config:

RAS[1:0] = 00000000-007FFFFF
  RAS[0] = 00000000-007FFFFF
  RAS[1] = 00080000-00AFFFFF

RAS[3:2] = 04000000-07FFFFFF
  RAS[2] = 04000000-05FFFFFF
  RAS[3] = 06000000-07FFFFFF

PCI I/O  = 0A000000-0BFFFFFF
PCI Mem  = 08000000-09FFFFFF

 CS[2:0] = 10000000-15FFFFFF
   CS[0] = 10000000-11FFFFFF
   CS[1] = 12000000-13FFFFFF
   CS[2] = 14000000-15FFFFFF

 CS[3]/B = 16000000-1FFFFFFF
   CS[3] = 16000000-17FFFFFF

*/

static READ32_DEVICE_HANDLER( seattle_ide_r )
{
	/* note that blitz times out if we don't have this cycle stealing */
	if (offset == 0x3f6/4)
		cpu_eat_cycles(device->machine->device("maincpu"), 100);
	return ide_controller32_r(device, offset, mem_mask);
}

static ADDRESS_MAP_START( seattle_map, ADDRESS_SPACE_PROGRAM, 32 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x007fffff) AM_RAM AM_BASE(&rambase)	// wg3dh only has 4MB; sfrush, blitz99 8MB
	AM_RANGE(0x08000000, 0x08ffffff) AM_DEVREAD("voodoo", voodoo_r) AM_WRITE(seattle_voodoo_w)
	AM_RANGE(0x0a000000, 0x0a0003ff) AM_DEVREADWRITE("ide", seattle_ide_r, ide_controller32_w)
	AM_RANGE(0x0a00040c, 0x0a00040f) AM_NOP						// IDE-related, but annoying
	AM_RANGE(0x0a000f00, 0x0a000f07) AM_DEVREADWRITE("ide", ide_bus_master32_r, ide_bus_master32_w)
	AM_RANGE(0x0c000000, 0x0c000fff) AM_READWRITE(galileo_r, galileo_w)
	AM_RANGE(0x13000000, 0x13000003) AM_WRITE(asic_fifo_w)
	AM_RANGE(0x16000000, 0x1600003f) AM_READWRITE(midway_ioasic_r, midway_ioasic_w)
	AM_RANGE(0x16100000, 0x1611ffff) AM_READWRITE(cmos_r, cmos_w) AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x17000000, 0x17000003) AM_READWRITE(cmos_protect_r, cmos_protect_w)
	AM_RANGE(0x17100000, 0x17100003) AM_WRITE(seattle_watchdog_w)
	AM_RANGE(0x17300000, 0x17300003) AM_RAM_WRITE(seattle_interrupt_enable_w) AM_BASE(&interrupt_enable)
	AM_RANGE(0x17400000, 0x17400003) AM_RAM_WRITE(interrupt_config_w) AM_BASE(&interrupt_config)
	AM_RANGE(0x17500000, 0x17500003) AM_READ(interrupt_state_r)
	AM_RANGE(0x17600000, 0x17600003) AM_READ(interrupt_state2_r)
	AM_RANGE(0x17700000, 0x17700003) AM_WRITE(vblank_clear_w)
	AM_RANGE(0x17800000, 0x17800003) AM_NOP
	AM_RANGE(0x17900000, 0x17900003) AM_READWRITE(status_leds_r, status_leds_w)
	AM_RANGE(0x17f00000, 0x17f00003) AM_RAM_WRITE(asic_reset_w) AM_BASE(&asic_reset)
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_REGION("user1", 0) AM_BASE(&rombase)
ADDRESS_MAP_END



/*************************************
 *
 *  Common input ports
 *
 *************************************/

static INPUT_PORTS_START( seattle_common )
	PORT_START("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, "Unknown0001" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Unknown0002" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Unknown0004" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Unknown0008" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Unknown0010" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Unknown0020" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown0040" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unknown0080" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Unknown0100" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Unknown0200" )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Unknown0400" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown0800" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Unknown1000" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Unknown2000" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Unknown4000" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown8000" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)	/* 3d cam */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( wg3dh )
	PORT_INCLUDE(seattle_common)

	PORT_MODIFY("DIPS")
	PORT_SERVICE_NO_TOGGLE( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0002, 0x0002, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mace )
	PORT_INCLUDE(seattle_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_NO_TOGGLE( 0x0080, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x8000, 0x0000, "Resolution" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Medium ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sfrush )
	PORT_INCLUDE(seattle_common)

	PORT_MODIFY("DIPS")
	PORT_SERVICE_NO_TOGGLE( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0002, 0x0002, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Abort") PORT_PLAYER(1) /* Abort */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME(DEF_STR( Reverse )) PORT_PLAYER(1)	/* reverse */
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START4 )  /* There are no start buttons on a Rush! */
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("View 1") PORT_PLAYER(1)	/* view 1 */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("View 2") PORT_PLAYER(1)	/* view 2 */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("View 3") PORT_PLAYER(1)	/* view 3 */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("Music") PORT_PLAYER(1)	/* music */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME("Track 1") PORT_PLAYER(1)	/* track 1 */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("Track 2") PORT_PLAYER(1)	/* track 2 */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_NAME("Track 3") PORT_PLAYER(1)	/* track 3 */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON16 ) PORT_NAME("Track 4") PORT_PLAYER(1)	/* track 4 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("1st Gear") PORT_PLAYER(1)	/* 1st gear */
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("2nd Gear") PORT_PLAYER(1)	/* 2nd gear */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("3rd Gear") PORT_PLAYER(1)	/* 3rd gear */
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("4th Gear") PORT_PLAYER(1)	/* 4th gear */
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN2")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN3")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN4")	/* Accel */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("AN5")	/* Brake */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(100) PORT_PLAYER(1)

	PORT_START("AN6")	/* Clutch */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL3 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(100) PORT_PLAYER(1)

	PORT_START("AN7")	/* Steer */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(5)
INPUT_PORTS_END


static INPUT_PORTS_START( sfrushrk )
	PORT_INCLUDE(sfrush)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, "Calibrate at startup" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Unknown0002" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_NO_TOGGLE( 0x0080, IP_ACTIVE_LOW )
INPUT_PORTS_END


static INPUT_PORTS_START( calspeed )
	PORT_INCLUDE(seattle_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_NO_TOGGLE( 0x0080, IP_ACTIVE_LOW )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2 )	/* test */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Radio") PORT_PLAYER(1)	/* radio */
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("View 1") PORT_PLAYER(1)	/* road cam */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("View 2") PORT_PLAYER(1)	/* tailgate cam */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("View 3") PORT_PLAYER(1)	/* sky cam */
	PORT_BIT( 0x0f80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("1st Gear") PORT_PLAYER(1)	/* 1st gear */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("2nd Gear") PORT_PLAYER(1)	/* 2nd gear */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("3rd Gear") PORT_PLAYER(1)	/* 3rd gear */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("4th Gear") PORT_PLAYER(1)	/* 4th gear */

	PORT_MODIFY("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")	/* Steer */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(5)

	PORT_START("AN1")	/* Accel */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("AN2")	/* Brake */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(100) PORT_PLAYER(1)

	PORT_START("AN3")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN4")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN5")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN6")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN7")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )
INPUT_PORTS_END


static INPUT_PORTS_START( vaportrx )
	PORT_INCLUDE(seattle_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_NO_TOGGLE( 0x0080, IP_ACTIVE_LOW )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)	/* left trigger */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2 )					/* test */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_A) PORT_PLAYER(1)	/* right trigger */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)						/* left thumb */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_S) PORT_PLAYER(1)	/* right thumb */
	PORT_BIT( 0x0180, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)						/* left view */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_Q) PORT_PLAYER(1)	/* right view */
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN3")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN4")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN5")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN6")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN7")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )
INPUT_PORTS_END


static INPUT_PORTS_START( biofreak )
	PORT_INCLUDE(seattle_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, "Hilink download??" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)	/* LP = P1 left punch */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)	/* F  = P1 ??? */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)	/* RP = P1 right punch */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	/* LP = P1 left punch */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)	/* F  = P1 ??? */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	/* RP = P1 right punch */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)	/* LK = P1 left kick */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)	/* RK = P1 right kick */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)	/* T  = P1 ??? */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)	/* LK = P2 left kick */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)	/* RK = P2 right kick */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)	/* T  = P2 ??? */
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( blitz )
	PORT_INCLUDE(seattle_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0000, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x000e, "Mode 1" )
	PORT_DIPSETTING(      0x0008, "Mode 2" )
	PORT_DIPSETTING(      0x0009, "Mode 3" )
	PORT_DIPSETTING(      0x0002, "Mode 4" )
	PORT_DIPSETTING(      0x000c, "Mode ECA" )
//  PORT_DIPSETTING(      0x0004, "Not Used 1" )        /* Marked as Unused in the manual */
//  PORT_DIPSETTING(      0x0008, "Not Used 2" )        /* Marked as Unused in the manual */
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0030, 0x0030, "Curency Type" )
	PORT_DIPSETTING(      0x0030, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( French ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( German ) )
//  PORT_DIPSETTING(      0x0000, "Not Used" )      /* Marked as Unused in the manual */
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ))	/* Marked as Unused in the manual */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Power Up Test Loop" )
	PORT_DIPSETTING(      0x0080, "One Time" )
	PORT_DIPSETTING(      0x0000, "Continuous" )
	PORT_DIPNAME( 0x0100, 0x0100, "Joysticks" )
	PORT_DIPSETTING(      0x0100, "8-Way" )
	PORT_DIPSETTING(      0x0000, "49-Way" )
	PORT_DIPNAME( 0x0600, 0x0200, "Graphics Mode" )
	PORT_DIPSETTING(      0x0200, "512x385 @ 25KHz" )
	PORT_DIPSETTING(      0x0400, "512x256 @ 15KHz" )
//  PORT_DIPSETTING(      0x0600, "0" )         /* Marked as Unused in the manual */
//  PORT_DIPSETTING(      0x0000, "3" )         /* Marked as Unused in the manual */
	PORT_DIPNAME( 0x1800, 0x1800, "Graphics Speed" )
	PORT_DIPSETTING(      0x0000, "45 MHz" )
	PORT_DIPSETTING(      0x0800, "47 MHz" )
	PORT_DIPSETTING(      0x1000, "49 MHz" )
	PORT_DIPSETTING(      0x1800, "51 MHz" )
	PORT_DIPNAME( 0x2000, 0x0000, "Bill Validator" )
	PORT_DIPSETTING(      0x2000, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0000, "One" )
	PORT_DIPNAME( 0x4000, 0x0000, "Power On Self Test" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ))
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ))
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( blitz99 )
	PORT_INCLUDE(blitz)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x003e, 0x003e, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x003e, "USA 1" )
	PORT_DIPSETTING(      0x003c, "USA 2" )
	PORT_DIPSETTING(      0x003a, "USA 3" )
	PORT_DIPSETTING(      0x0038, "USA 4" )
	PORT_DIPSETTING(      0x0036, "USA 5" )
	PORT_DIPSETTING(      0x0034, "USA 6" )
	PORT_DIPSETTING(      0x0032, "USA 7" )
	PORT_DIPSETTING(      0x0030, "USA ECA" )
	PORT_DIPSETTING(      0x002e, "France 1" )
	PORT_DIPSETTING(      0x002c, "France 2" )
	PORT_DIPSETTING(      0x002a, "France 3" )
	PORT_DIPSETTING(      0x0028, "France 4" )
	PORT_DIPSETTING(      0x0026, "France 5" )
	PORT_DIPSETTING(      0x0024, "France 6" )
	PORT_DIPSETTING(      0x0022, "France 7" )
	PORT_DIPSETTING(      0x0020, "France ECA" )
	PORT_DIPSETTING(      0x001e, "German 1" )
	PORT_DIPSETTING(      0x001c, "German 2" )
	PORT_DIPSETTING(      0x001a, "German 3" )
	PORT_DIPSETTING(      0x0018, "German 4" )
	PORT_DIPSETTING(      0x0016, "German 5" )
//  PORT_DIPSETTING(      0x0014, "German 5" )
//  PORT_DIPSETTING(      0x0012, "German 5" )
	PORT_DIPSETTING(      0x0010, "German ECA" )
	PORT_DIPSETTING(      0x000e, "U.K. 1 ECA" )
	PORT_DIPSETTING(      0x000c, "U.K. 2 ECA" )
	PORT_DIPSETTING(      0x000a, "U.K. 3 ECA" )
	PORT_DIPSETTING(      0x0008, "U.K. 4" )
	PORT_DIPSETTING(      0x0006, "U.K. 5" )
	PORT_DIPSETTING(      0x0004, "U.K. 6 ECA" )
	PORT_DIPSETTING(      0x0002, "U.K. 7 ECA" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Players ) )
	PORT_DIPSETTING(      0x2000, "2" )
	PORT_DIPSETTING(      0x0000, "4" )
INPUT_PORTS_END


static INPUT_PORTS_START( carnevil )
	PORT_INCLUDE( seattle_common )

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0000, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x003e, 0x003e, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x003e, "USA 1" )
	PORT_DIPSETTING(      0x003c, "USA 2" )
	PORT_DIPSETTING(      0x003a, "USA 3" )
	PORT_DIPSETTING(      0x0038, "USA 4" )
	PORT_DIPSETTING(      0x0036, "USA 5" )
	PORT_DIPSETTING(      0x0034, "USA 6" )
	PORT_DIPSETTING(      0x0032, "USA 7" )
	PORT_DIPSETTING(      0x0030, "USA ECA" )
	PORT_DIPSETTING(      0x002e, "France 1" )
	PORT_DIPSETTING(      0x002c, "France 2" )
	PORT_DIPSETTING(      0x002a, "France 3" )
	PORT_DIPSETTING(      0x0028, "France 4" )
	PORT_DIPSETTING(      0x0026, "France 5" )
	PORT_DIPSETTING(      0x0024, "France 6" )
	PORT_DIPSETTING(      0x0022, "France 7" )
	PORT_DIPSETTING(      0x0020, "France ECA" )
	PORT_DIPSETTING(      0x001e, "German 1" )
	PORT_DIPSETTING(      0x001c, "German 2" )
	PORT_DIPSETTING(      0x001a, "German 3" )
	PORT_DIPSETTING(      0x0018, "German 4" )
	PORT_DIPSETTING(      0x0016, "German 5" )
//  PORT_DIPSETTING(      0x0014, "German 5" )
//  PORT_DIPSETTING(      0x0012, "German 5" )
	PORT_DIPSETTING(      0x0010, "German ECA" )
	PORT_DIPSETTING(      0x000e, "U.K. 1" )
	PORT_DIPSETTING(      0x000c, "U.K. 2" )
	PORT_DIPSETTING(      0x000a, "U.K. 3" )
	PORT_DIPSETTING(      0x0008, "U.K. 4" )
	PORT_DIPSETTING(      0x0006, "U.K. 5" )
	PORT_DIPSETTING(      0x0004, "U.K. 6" )
	PORT_DIPSETTING(      0x0002, "U.K. 7 ECA" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Power Up Test Loop" )
	PORT_DIPSETTING(      0x0080, DEF_STR( No ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ))
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0100, "0" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x0600, 0x0400, "Resolution" )
//  PORT_DIPSETTING(      0x0600, "0" )
//  PORT_DIPSETTING(      0x0200, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Low ) )
//  PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x1800, 0x1800, "Graphics Speed" )
	PORT_DIPSETTING(      0x0000, "45 MHz" )
	PORT_DIPSETTING(      0x0800, "47 MHz" )
	PORT_DIPSETTING(      0x1000, "49 MHz" )
	PORT_DIPSETTING(      0x1800, "51 MHz" )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x2000, "0" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x4000, 0x0000, "Power On Self Test" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ))
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ))
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0780, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHT0_X")				/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("LIGHT0_Y")				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("LIGHT1_X")				/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("FAKE")					/* fake switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( hyprdriv )
	PORT_INCLUDE( seattle_common )

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0000, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x003e, 0x0034, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x003e, "USA 10" )
	PORT_DIPSETTING(      0x003c, "USA 11" )
	PORT_DIPSETTING(      0x003a, "USA 12" )
	PORT_DIPSETTING(      0x0038, "USA 13" )
	PORT_DIPSETTING(      0x0036, "USA 9" )
	PORT_DIPSETTING(      0x0034, "USA 1" )
	PORT_DIPSETTING(      0x0032, "USA 2" )
	PORT_DIPSETTING(      0x0030, "USA ECA" )
	PORT_DIPSETTING(      0x002e, "France 1" )
	PORT_DIPSETTING(      0x002c, "France 2" )
	PORT_DIPSETTING(      0x002a, "France 3" )
	PORT_DIPSETTING(      0x0028, "France 4" )
	PORT_DIPSETTING(      0x0026, "France 5" )
	PORT_DIPSETTING(      0x0024, "France 6" )
	PORT_DIPSETTING(      0x0022, "France 7" )
	PORT_DIPSETTING(      0x0020, "France ECA" )
	PORT_DIPSETTING(      0x001e, "German 1" )
	PORT_DIPSETTING(      0x001c, "German 2" )
	PORT_DIPSETTING(      0x001a, "German 3" )
	PORT_DIPSETTING(      0x0018, "German 4" )
	PORT_DIPSETTING(      0x0016, "German 5" )
	PORT_DIPSETTING(      0x0014, "German 5" )
	PORT_DIPSETTING(      0x0012, "German 5" )
	PORT_DIPSETTING(      0x0010, "German ECA" )
	PORT_DIPSETTING(      0x000e, "U.K. 1 ECA" )
	PORT_DIPSETTING(      0x000c, "U.K. 2 ECA" )
	PORT_DIPSETTING(      0x000a, "U.K. 3 ECA" )
	PORT_DIPSETTING(      0x0008, "U.K. 4" )
	PORT_DIPSETTING(      0x0006, "U.K. 5" )
	PORT_DIPSETTING(      0x0004, "U.K. 6 ECA" )
	PORT_DIPSETTING(      0x0002, "U.K. 7 ECA" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Power Up Test Loop" )
	PORT_DIPSETTING(      0x0080, DEF_STR( No ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ))
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ))
	PORT_DIPSETTING(      0x0100, "0" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x0600, 0x0200, "Resolution" )
	PORT_DIPSETTING(      0x0600, "0" )
	PORT_DIPSETTING(      0x0200, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x1800, 0x0000, "Graphics Speed" )
	PORT_DIPSETTING(      0x0000, "45 MHz" )
	PORT_DIPSETTING(      0x0800, "47 MHz" )
	PORT_DIPSETTING(      0x1000, "49 MHz" )
	PORT_DIPSETTING(      0x1800, "51 MHz" )
	PORT_DIPNAME( 0x2000, 0x2000, "Brake" )
	PORT_DIPSETTING(      0x2000, "Enabled" )
	PORT_DIPSETTING(      0x0000, "Disabled" )
	PORT_DIPNAME( 0x4000, 0x0000, "Power On Self Test" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ))
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ))
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")
	PORT_BIT( 0x00ff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(25)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN1")	/* Accel */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("AN2")	/* Brake */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(100) PORT_PLAYER(1)

	PORT_START("AN3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(25)

	PORT_START("AN4")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN5")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN6")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN7")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	/* 2008-06 FP: is this ever read?? */
	PORT_START("AN8")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static const mips3_config config =
{
	16384,		/* code cache size */
	16384,		/* data cache size */
	SYSTEM_CLOCK	/* system clock rate */
};

static MACHINE_DRIVER_START( seattle_common )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", R5000LE, SYSTEM_CLOCK*3)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(seattle_map)

	MDRV_MACHINE_START(seattle)
	MDRV_MACHINE_RESET(seattle)
	MDRV_NVRAM_HANDLER(generic_1fill)

	MDRV_IDE_CONTROLLER_ADD("ide", ide_interrupt)
	MDRV_IDE_BUS_MASTER_SPACE("maincpu", PROGRAM)

	MDRV_3DFX_VOODOO_1_ADD("voodoo", STD_VOODOO_1_CLOCK, 2, "screen")
	MDRV_3DFX_VOODOO_CPU("maincpu")
	MDRV_3DFX_VOODOO_TMU_MEMORY(0, 4)
	MDRV_3DFX_VOODOO_VBLANK(vblank_assert)
	MDRV_3DFX_VOODOO_STALL(voodoo_stall)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 639, 0, 479)

	MDRV_VIDEO_UPDATE(seattle)

	/* sound hardware */
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( phoenixsa )
	MDRV_IMPORT_FROM(seattle_common)
	MDRV_IMPORT_FROM(dcs2_audio_2115)

	MDRV_CPU_REPLACE("maincpu", R4700LE, SYSTEM_CLOCK*2)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(seattle_map)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( seattle150 )
	MDRV_IMPORT_FROM(seattle_common)
	MDRV_IMPORT_FROM(dcs2_audio_2115)

	MDRV_CPU_REPLACE("maincpu", R5000LE, SYSTEM_CLOCK*3)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(seattle_map)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( seattle150_widget )
	MDRV_IMPORT_FROM(seattle150)
	MDRV_SMC91C94_ADD("ethernet", ethernet_interrupt)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( seattle200 )
	MDRV_IMPORT_FROM(seattle_common)
	MDRV_IMPORT_FROM(dcs2_audio_2115)

	MDRV_CPU_REPLACE("maincpu", R5000LE, SYSTEM_CLOCK*4)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(seattle_map)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( seattle200_widget )
	MDRV_IMPORT_FROM(seattle200)
	MDRV_SMC91C94_ADD("ethernet", ethernet_interrupt)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( flagstaff )
	MDRV_IMPORT_FROM(seattle_common)
	MDRV_IMPORT_FROM(cage_seattle)

	MDRV_CPU_REPLACE("maincpu", R5000LE, SYSTEM_CLOCK*4)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(seattle_map)

	MDRV_SMC91C94_ADD("ethernet", ethernet_interrupt)

	MDRV_3DFX_VOODOO_MODIFY("voodoo")
	MDRV_3DFX_VOODOO_TMU_MEMORY(1, 4)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( wg3dh )
	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* Boot Code Version L1.2 (10/8/96) */
	ROM_LOAD( "wg3dh_12.u32", 0x000000, 0x80000, CRC(15e4cea2) SHA1(72c0db7dc53ce645ba27a5311b5ce803ad39f131) )

	DISK_REGION( "ide" )	/* Hard Drive Version 1.3 (Guts 10/15/96, Main 10/15/96) */
	DISK_IMAGE( "wg3dh", 0, SHA1(4fc6f25d7f043d9bcf8743aa8df1d9be3cbc375b) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version L1.1 */
	ROM_LOAD16_BYTE( "soundl11.u95", 0x000000, 0x8000, CRC(c589458c) SHA1(0cf970a35910a74cdcf3bd8119bfc0c693e19b00) )
ROM_END


ROM_START( mace )
	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* Boot Code Version 1.0ce 7/2/97 */
	ROM_LOAD( "mace10ce.u32", 0x000000, 0x80000, CRC(7a50b37e) SHA1(33788835f84a9443566c80bee9f20a1691490c6d) )

	DISK_REGION( "ide" )	/* Hard Drive Version 1.0B 6/10/97 (Guts 7/2/97, Main 7/2/97) */
	DISK_IMAGE( "mace", 0, SHA1(96ec8d3ff5dd894e21aa81403bcdbeba44bb97ea) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version L1.1, Labeled as Version 1.0 */
	ROM_LOAD16_BYTE( "soundl11.u95", 0x000000, 0x8000, CRC(c589458c) SHA1(0cf970a35910a74cdcf3bd8119bfc0c693e19b00) )
ROM_END


ROM_START( macea )
	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* Boot Code Version ??? 5/7/97 */
	ROM_LOAD( "maceboot.u32", 0x000000, 0x80000, CRC(effe3ebc) SHA1(7af3ca3580d6276ffa7ab8b4c57274e15ee6bcbb) )

	DISK_REGION( "ide" )	/* Hard Drive Version 1.0a (Guts 6/9/97, Main 5/12/97) */
	DISK_IMAGE( "macea", 0, BAD_DUMP SHA1(9bd4a60627915d71932cab24f89c48ea21f4c1cb) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version L1.1 */
	ROM_LOAD16_BYTE( "soundl11.u95", 0x000000, 0x8000, CRC(c589458c) SHA1(0cf970a35910a74cdcf3bd8119bfc0c693e19b00) )
ROM_END


ROM_START( sfrush )
	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* Boot Code Version L1.0 */
	ROM_LOAD( "hdboot.u32", 0x000000, 0x80000, CRC(39a35f1b) SHA1(c46d83448399205d38e6e41dd56abbc362254254) )

	ROM_REGION32_LE( 0x200000, "cageboot", 0 )	/* TMS320C31 boot ROM  Version L1.0 */
	ROM_LOAD32_BYTE( "sndboot.u69", 0x000000, 0x080000, CRC(7e52cdc7) SHA1(f735063e19d2ca672cef6d761a2a47df272e8c59) )

	ROM_REGION32_LE( 0x1000000, "cage", 0 )	/* TMS320C31 sound ROMs */
	ROM_LOAD32_WORD( "sfrush.u62",  0x400000, 0x200000, CRC(5d66490e) SHA1(bd39ea3b45d44cae6ca5890f365653326bbecd2d) )
	ROM_LOAD32_WORD( "sfrush.u61",  0x400002, 0x200000, CRC(f3a00ee8) SHA1(c1ac780efc32b2e30522d7cc3e6d92e7daaadddd) )
	ROM_LOAD32_WORD( "sfrush.u53",  0x800000, 0x200000, CRC(71f8ddb0) SHA1(c24bef801f43bae68fda043c4356e8cf1298ca97) )
	ROM_LOAD32_WORD( "sfrush.u49",  0x800002, 0x200000, CRC(dfb0a54c) SHA1(ed34f9485f7a7e5bb73bf5c6428b27548e12db12) )

	DISK_REGION( "ide" )	/* Hard Drive Version L1.06 */
	DISK_IMAGE( "sfrush", 0, SHA1(e2db0270a707fb2115207f988d5751081d6b4994) )
ROM_END


ROM_START( sfrushrk )
	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* Boot Code */
	ROM_LOAD( "boot.bin",   0x000000, 0x080000, CRC(0555b3cf) SHA1(a48abd6d06a26f4f9b6c52d8c0af6095b6be57fd) )

	ROM_REGION32_LE( 0x200000, "cageboot", 0 )	/* TMS320C31 boot ROM */
	ROM_LOAD32_BYTE( "audboot.bin",    0x000000, 0x080000, CRC(c70c060d) SHA1(dd014bd13efdf5adc5450836bd4650351abefc46) )

	ROM_REGION32_LE( 0x1000000, "cage", 0 )	/* TMS320C31 sound ROMs */
	ROM_LOAD32_WORD( "audio.u62",  0x400000, 0x200000, CRC(cacf09e3) SHA1(349af1767cb0ee2a0eb9d7c2ab078fcae5fec8e7) )
	ROM_LOAD32_WORD( "audio.u61",  0x400002, 0x200000, CRC(ea895d29) SHA1(1edde0497f2abd1636c5d7bcfbc03bcff321261c) )
	ROM_LOAD32_WORD( "audio.u53",  0x800000, 0x200000, CRC(51c89a14) SHA1(6bc62bcda224040a4596d795132874828011a038) )
	ROM_LOAD32_WORD( "audio.u49",  0x800002, 0x200000, CRC(e6b684d3) SHA1(1f5bab7fae974cecc8756dd23e3c7aa2cf6e7dc7) )

	DISK_REGION( "ide" )	/* Hard Drive Version 1.2 */
	DISK_IMAGE( "sfrushrk", 0, SHA1(e763f26aca67ebc17fe8b8df4fba91d492cf7837) )
ROM_END


ROM_START( calspeed )
	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* Boot Code Version 1.2 (2/18/98) */
	ROM_LOAD( "caspd1_2.u32", 0x000000, 0x80000, CRC(0a235e4e) SHA1(b352f10fad786260b58bd344b5002b6ea7aaf76d) )

	DISK_REGION( "ide" )	/* Release version 2.1a (4/17/98) (Guts 1.25 4/17/98, Main 4/17/98) */
	DISK_IMAGE( "calspeed", 0, SHA1(08d411c591d4b8bbdd6437ea80d01c4cec8516f8) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version 1.02 */
	ROM_LOAD16_BYTE( "sound102.u95", 0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )
ROM_END


ROM_START( calspeeda )
	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* Boot Code Version 1.2 (2/18/98) */
	ROM_LOAD( "caspd1_2.u32", 0x000000, 0x80000, CRC(0a235e4e) SHA1(b352f10fad786260b58bd344b5002b6ea7aaf76d) )

	DISK_REGION( "ide" )	/* Release version 1.0r7a (3/4/98) (Guts 3/3/98, Main 1/19/98) */
	DISK_IMAGE( "calspeda", 0, SHA1(6b1c3a7530195ef7309b06a651b01c8b3ece92c6) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version 1.02 */
	ROM_LOAD16_BYTE( "sound102.u95", 0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )
ROM_END


ROM_START( vaportrx )
	ROM_REGION32_LE( 0x80000, "user1", 0 )
	ROM_LOAD( "vtrxboot.bin", 0x000000, 0x80000, CRC(ee487a6c) SHA1(fb9efda85047cf615f24f7276a9af9fd542f3354) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "vaportrx", 0, SHA1(fe53ca7643d2ed2745086abb7f2243c69678cab1) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version 1.02 */
	ROM_LOAD16_BYTE( "vaportrx.snd", 0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )
ROM_END


ROM_START( vaportrxp )
	ROM_REGION32_LE( 0x80000, "user1", 0 )
	ROM_LOAD( "vtrxboot.bin", 0x000000, 0x80000, CRC(ee487a6c) SHA1(fb9efda85047cf615f24f7276a9af9fd542f3354) )

	DISK_REGION( "ide" ) /* Guts: Apr 10 1998 11:03:14  Main: Apr 10 1998 11:27:44 */
	DISK_IMAGE( "vaportrp", 0, SHA1(6c86637c442ebd6994eee8c0ae0dce343c35dbe9) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version 1.02 */
	ROM_LOAD16_BYTE( "vaportrx.snd", 0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )
ROM_END


ROM_START( biofreak )
	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version 1.02 */
	ROM_LOAD16_BYTE( "sound102.u95", 0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) /* Seattle System Boot ROM Version 0.1i Apr 14 1997  14:52:53 */
	ROM_LOAD( "biofreak.u32", 0x000000, 0x80000, CRC(cefa00bb) SHA1(7e171610ede1e8a448fb8d175f9cb9e7d549de28) )

	DISK_REGION( "ide" ) /* Build Date 12/11/97 */
	DISK_IMAGE( "biofreak", 0, SHA1(711241642f92ded8eaf20c418ea748989183fe10) )
ROM_END


ROM_START( blitz )
	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version 1.02 */
	ROM_LOAD16_BYTE( "sound102.u95", 0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* Boot Code Version 1.2 */
	ROM_LOAD( "blitz1_2.u32", 0x000000, 0x80000, CRC(38dbecf5) SHA1(7dd5a5b3baf83a7f8f877ff4cd3f5e8b5201b36f) )

	DISK_REGION( "ide" )	/* Hard Drive Version 1.21 */
	DISK_IMAGE( "blitz", 0, SHA1(9131c7888e89b3c172780156ed3fe1fe46f78b0a) )
ROM_END


ROM_START( blitz11 )
	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version 1.02 */
	ROM_LOAD16_BYTE( "sound102.u95", 0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* Boot Code Version 1.1 */
	ROM_LOAD( "blitz1_1.u32", 0x000000, 0x80000, CRC(8163ce02) SHA1(89b432d8879052f6c5534ee49599f667f50a010f) )

	DISK_REGION( "ide" )	/* Hard Drive Version 1.21 */
	DISK_IMAGE( "blitz", 0, SHA1(9131c7888e89b3c172780156ed3fe1fe46f78b0a) )
ROM_END


ROM_START( blitz99 )
	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version 1.02 */
	ROM_LOAD16_BYTE( "sound102.u95", 0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* Boot Code Version 1.0 */
	ROM_LOAD( "bltz9910.u32", 0x000000, 0x80000, CRC(777119b2) SHA1(40d255181c2f3a787919c339e83593fd506779a5) )

	DISK_REGION( "ide" )	/* Hard Drive Version 1.30 */
	DISK_IMAGE( "blitz99", 0, SHA1(19877e26ffce81dd525031e9e2b4f83ff982e2d9) )
ROM_END


ROM_START( blitz2k )
	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version 1.02 */
	ROM_LOAD16_BYTE( "sound102.u95", 0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x80000, "user1", 0 )	/* Boot Code Version 1.4 */
	ROM_LOAD( "bltz2k14.u32", 0x000000, 0x80000, CRC(ac4f0051) SHA1(b8125c17370db7bfd9b783230b4ef3d5b22a2025) )

	DISK_REGION( "ide" )	/* Hard Drive Version 1.5 */
	DISK_IMAGE( "blitz2k", 0, SHA1(e89b7fbd4b4a9854d47ae97493e0afffbd1f69e7) )
ROM_END


ROM_START( carnevil )
	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version 1.02 */
	ROM_LOAD16_BYTE( "sound102.u95", 0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) /* Boot Rom Version 1.9 */
	ROM_LOAD( "carnevil1_9.u32", 0x000000, 0x80000, CRC(82c07f2e) SHA1(fa51c58022ce251c53bad12fc6ffadb35adb8162) )

	DISK_REGION( "ide" )	/* Hard Drive v1.0.3  Diagnostics v3.4 / Feb 1 1999 16:00:07 */
	DISK_IMAGE( "carnevil", 0, SHA1(5cffb0de63ad36eb01c5951bab04d3f8a9e23e16) )
ROM_END


ROM_START( carnevil1 )
	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version 1.02 */
	ROM_LOAD16_BYTE( "sound102.u95", 0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) /* Boot Rom Version 1.9 */
	ROM_LOAD( "carnevil1_9.u32", 0x000000, 0x80000, CRC(82c07f2e) SHA1(fa51c58022ce251c53bad12fc6ffadb35adb8162) )

	DISK_REGION( "ide" )	/* Hard Drive v1.0.1  Diagnostics v3.3 / Oct 20 1998 11:44:41 */
	DISK_IMAGE( "carnevi1", 0, BAD_DUMP SHA1(94532727512280930a100fe473bf3a938fe2d44f) )
ROM_END


ROM_START( hyprdriv )
	ROM_REGION16_LE( 0x10000, "dcs", 0 )	/* ADSP-2115 data Version 1.02 */
	ROM_LOAD16_BYTE( "seattle.snd", 0x000000, 0x8000, BAD_DUMP CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x80000, "user1", 0 ) /* Boot Rom Version 9. */
	ROM_LOAD( "hyprdrve.u32", 0x000000, 0x80000, CRC(3e18cb80) SHA1(b18cc4253090ee1d65d72a7ec0c426ed08c4f238) )

	DISK_REGION( "ide" )	/* Version 1.40  Oct 23 1998  15:16:00 */
	DISK_IMAGE( "hyprdriv", 0, SHA1(8cfa343797575b32f46cc24150024be48963a03e) )
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

static void init_common(running_machine *machine, int ioasic, int serialnum, int yearoffs, int config)
{
	running_device *device;

	/* initialize the subsystems */
	midway_ioasic_init(machine, ioasic, serialnum, yearoffs, ioasic_irq);

	/* switch off the configuration */
	board_config = config;
	switch (config)
	{
		case PHOENIX_CONFIG:
			/* original Phoenix board only has 4MB of RAM */
			memory_unmap_readwrite(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x00400000, 0x007fffff, 0, 0);
			break;

		case SEATTLE_WIDGET_CONFIG:
			/* set up the widget board */
			device = machine->device("ethernet");
			memory_install_readwrite32_device_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), device, 0x16c00000, 0x16c0001f, 0, 0, widget_r, widget_w);
			break;

		case FLAGSTAFF_CONFIG:
			/* set up the analog inputs */
			memory_install_readwrite32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x14000000, 0x14000003, 0, 0, analog_port_r, analog_port_w);

			/* set up the ethernet controller */
			device = machine->device("ethernet");
			memory_install_readwrite32_device_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), device, 0x16c00000, 0x16c0003f, 0, 0, ethernet_r, ethernet_w);
			break;
	}
}


static DRIVER_INIT( wg3dh )
{
	dcs2_init(machine, 2, 0x3839);
	init_common(machine, MIDWAY_IOASIC_STANDARD, 310/* others? */, 80, PHOENIX_CONFIG);

	/* speedups */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x8004413C, 0x0C0054B4, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x80094930, 0x00A2102B, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x80092984, 0x3C028011, 250);		/* confirmed */
}


static DRIVER_INIT( mace )
{
	dcs2_init(machine, 2, 0x3839);
	init_common(machine, MIDWAY_IOASIC_MACE, 319/* others? */, 80, SEATTLE_CONFIG);

	/* speedups */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x800108F8, 0x8C420000, 250);		/* confirmed */
}


static DRIVER_INIT( sfrush )
{
	cage_init(machine, 0x5236);
	init_common(machine, MIDWAY_IOASIC_STANDARD, 315/* no alternates */, 100, FLAGSTAFF_CONFIG);

	/* speedups */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x80059F34, 0x3C028012, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x800A5AF4, 0x8E300010, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x8004C260, 0x3C028012, 250);		/* confirmed */
}


static DRIVER_INIT( sfrushrk )
{
	cage_init(machine, 0x5329);
	init_common(machine, MIDWAY_IOASIC_SFRUSHRK, 331/* unknown */, 100, FLAGSTAFF_CONFIG);

	/* speedups */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x800343E8, 0x3C028012, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x8008F4F0, 0x3C028012, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x800A365C, 0x8E300014, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x80051DAC, 0x3C028012, 250);		/* confirmed */
}


static DRIVER_INIT( calspeed )
{
	dcs2_init(machine, 2, 0x39c0);
	init_common(machine, MIDWAY_IOASIC_CALSPEED, 328/* others? */, 100, SEATTLE_WIDGET_CONFIG);
	midway_ioasic_set_auto_ack(1);

	/* speedups */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x80032534, 0x02221024, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x800B1BE4, 0x8E110014, 250);		/* confirmed */
}


static DRIVER_INIT( vaportrx )
{
	dcs2_init(machine, 2, 0x39c2);
	init_common(machine, MIDWAY_IOASIC_VAPORTRX, 324/* 334? unknown */, 100, SEATTLE_WIDGET_CONFIG);

	/* speedups */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x80049F14, 0x3C028020, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x8004859C, 0x3C028020, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x8005922C, 0x8E020014, 250);		/* confirmed */
}


static DRIVER_INIT( biofreak )
{
	dcs2_init(machine, 2, 0x3835);
	init_common(machine, MIDWAY_IOASIC_STANDARD, 231/* no alternates */, 80, SEATTLE_CONFIG);

	/* speedups */
}


static DRIVER_INIT( blitz )
{
	dcs2_init(machine, 2, 0x39c2);
	init_common(machine, MIDWAY_IOASIC_BLITZ99, 444/* or 528 */, 80, SEATTLE_CONFIG);

	/* for some reason, the code in the ROM appears buggy; this is a small patch to fix it */
	rombase[0x934/4] += 4;

	/* main CPU speedups */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x80135510, 0x3C028024, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x800087DC, 0x8E820010, 250);		/* confirmed */
}


static DRIVER_INIT( blitz99 )
{
	dcs2_init(machine, 2, 0x0afb);
	init_common(machine, MIDWAY_IOASIC_BLITZ99, 481/* or 484 or 520 */, 80, SEATTLE_CONFIG);

	/* speedups */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x8014E41C, 0x3C038025, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x80011F10, 0x8E020018, 250);		/* confirmed */
}


static DRIVER_INIT( blitz2k )
{
	dcs2_init(machine, 2, 0x0b5d);
	init_common(machine, MIDWAY_IOASIC_BLITZ99, 494/* or 498 */, 80, SEATTLE_CONFIG);

	/* speedups */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x8015773C, 0x3C038025, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x80012CA8, 0x8E020018, 250);		/* confirmed */
}


static DRIVER_INIT( carnevil )
{
	dcs2_init(machine, 2, 0x0af7);
	init_common(machine, MIDWAY_IOASIC_CARNEVIL, 469/* 469 or 486 or 528 */, 80, SEATTLE_CONFIG);

	/* set up the gun */
	memory_install_readwrite32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x16800000, 0x1680001f, 0, 0, carnevil_gun_r, carnevil_gun_w);

	/* speedups */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x8015176C, 0x3C03801A, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x80011FBC, 0x8E020018, 250);		/* confirmed */
}


static DRIVER_INIT( hyprdriv )
{
	dcs2_init(machine, 2, 0x0af7);
	init_common(machine, MIDWAY_IOASIC_HYPRDRIV, 469/* unknown */, 80, SEATTLE_WIDGET_CONFIG);

	/* speedups */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x801643BC, 0x3C03801B, 250);		/* confirmed */
	mips3drc_add_hotspot(machine->device("maincpu"), 0x80011FB8, 0x8E020018, 250);		/* confirmed */
	//mips3drc_add_hotspot(machine->device("maincpu"), 0x80136A80, 0x3C02801D, 250);      /* potential */
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

/* Atari */
GAME( 1996, wg3dh,    0,        phoenixsa,         wg3dh,    wg3dh,    ROT0, "Atari Games",  "Wayne Gretzky's 3D Hockey", GAME_SUPPORTS_SAVE )
GAME( 1996, mace,     0,        seattle150,        mace,     mace,     ROT0, "Atari Games",  "Mace: The Dark Age (boot ROM 1.0ce, HDD 1.0b)", GAME_SUPPORTS_SAVE )
GAME( 1997, macea,    mace,     seattle150,        mace,     mace,     ROT0, "Atari Games",  "Mace: The Dark Age (HDD 1.0a)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfrush,   0,        flagstaff,         sfrush,   sfrush,   ROT0, "Atari Games",  "San Francisco Rush", GAME_SUPPORTS_SAVE )
GAME( 1996, sfrushrk, 0,        flagstaff,         sfrushrk, sfrushrk, ROT0, "Atari Games",  "San Francisco Rush: The Rock", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1998, calspeed, 0,        seattle150_widget, calspeed, calspeed, ROT0, "Atari Games",  "California Speed (Version 2.1a, 4/17/98)", GAME_SUPPORTS_SAVE )
GAME( 1998, calspeeda,calspeed, seattle150_widget, calspeed, calspeed, ROT0, "Atari Games",  "California Speed (Version 1.0r7a 3/4/98)", GAME_SUPPORTS_SAVE )
GAME( 1998, vaportrx, 0,        seattle200_widget, vaportrx, vaportrx, ROT0, "Atari Games",  "Vapor TRX", GAME_SUPPORTS_SAVE )
GAME( 1998, vaportrxp,vaportrx, seattle200_widget, vaportrx, vaportrx, ROT0, "Atari Games",  "Vapor TRX (prototype)", GAME_SUPPORTS_SAVE )

/* Midway */
GAME( 1997, biofreak, 0,        seattle150,        biofreak, biofreak, ROT0, "Midway Games", "BioFreaks (prototype)", GAME_SUPPORTS_SAVE )
GAME( 1997, blitz,    0,        seattle150,        blitz,    blitz,    ROT0, "Midway Games", "NFL Blitz (boot ROM 1.2)", GAME_SUPPORTS_SAVE )
GAME( 1997, blitz11,  blitz,    seattle150,        blitz,    blitz,    ROT0, "Midway Games", "NFL Blitz (boot ROM 1.1)", GAME_SUPPORTS_SAVE )
GAME( 1998, blitz99,  0,        seattle150,        blitz99,  blitz99,  ROT0, "Midway Games", "NFL Blitz '99", GAME_SUPPORTS_SAVE )
GAME( 1999, blitz2k,  0,        seattle150,        blitz99,  blitz2k,  ROT0, "Midway Games", "NFL Blitz 2000 Gold Edition", GAME_SUPPORTS_SAVE )
GAME( 1998, carnevil, 0,        seattle150,        carnevil, carnevil, ROT0, "Midway Games", "CarnEvil (v1.0.3)", GAME_SUPPORTS_SAVE )
GAME( 1998, carnevil1,carnevil, seattle150,        carnevil, carnevil, ROT0, "Midway Games", "CarnEvil (v1.0.1)", GAME_SUPPORTS_SAVE )
GAME( 1998, hyprdriv, 0,        seattle200_widget, hyprdriv, hyprdriv, ROT0, "Midway Games", "Hyperdrive", GAME_SUPPORTS_SAVE )
