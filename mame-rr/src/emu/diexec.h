/***************************************************************************

    diexec.h

    Device execution interfaces.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DIEXEC_H__
#define __DIEXEC_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// suspension reasons for executing devices
const UINT32 SUSPEND_REASON_HALT		= 0x0001;	// HALT line set (or equivalent)
const UINT32 SUSPEND_REASON_RESET		= 0x0002;	// RESET line set (or equivalent)
const UINT32 SUSPEND_REASON_SPIN		= 0x0004;	// currently spinning
const UINT32 SUSPEND_REASON_TRIGGER 	= 0x0008;	// waiting for a trigger
const UINT32 SUSPEND_REASON_DISABLE 	= 0x0010;	// disabled (due to disable flag)
const UINT32 SUSPEND_REASON_TIMESLICE	= 0x0020;	// waiting for the next timeslice
const UINT32 SUSPEND_ANY_REASON 		= ~0;		// all of the above


// I/O line states
enum line_state
{
	CLEAR_LINE = 0,				// clear (a fired or held) line
	ASSERT_LINE,				// assert an interrupt immediately
	HOLD_LINE,					// hold interrupt line until acknowledged
	PULSE_LINE					// pulse interrupt line instantaneously (only for NMI, RESET)
};


// I/O line definitions
enum
{
	// input lines
	MAX_INPUT_LINES = 32+3,
	INPUT_LINE_IRQ0 = 0,
	INPUT_LINE_IRQ1 = 1,
	INPUT_LINE_IRQ2 = 2,
	INPUT_LINE_IRQ3 = 3,
	INPUT_LINE_IRQ4 = 4,
	INPUT_LINE_IRQ5 = 5,
	INPUT_LINE_IRQ6 = 6,
	INPUT_LINE_IRQ7 = 7,
	INPUT_LINE_IRQ8 = 8,
	INPUT_LINE_IRQ9 = 9,
	INPUT_LINE_NMI = MAX_INPUT_LINES - 3,

	// special input lines that are implemented in the core
	INPUT_LINE_RESET = MAX_INPUT_LINES - 2,
	INPUT_LINE_HALT = MAX_INPUT_LINES - 1
};



//**************************************************************************
//  MACROS
//**************************************************************************

// IRQ callback to be called by device implementations when an IRQ is actually taken
#define IRQ_CALLBACK(func)				int func(device_t *device, int irqline)



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_DEVICE_DISABLE() \
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_DIEXEC_DISABLE, 8), \

#define MDRV_DEVICE_VBLANK_INT(_tag, _func) \
	TOKEN_UINT32_PACK2(MCONFIG_TOKEN_DIEXEC_VBLANK_INT, 8, 0, 24), \
	TOKEN_PTR(device_interrupt, _func), \
	TOKEN_PTR(stringptr, _tag), \

#define MDRV_DEVICE_PERIODIC_INT(_func, _rate)	\
	TOKEN_UINT32_PACK1(MCONFIG_TOKEN_DIEXEC_PERIODIC_INT, 8), \
	TOKEN_PTR(device_interrupt, _func), \
	TOKEN_UINT64(UINT64_ATTOTIME_IN_HZ(_rate)), \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class emu_timer;
class screen_device;


// interrupt callback for VBLANK and timed interrupts
typedef void (*device_interrupt_func)(device_t *device);

// IRQ callback to be called by executing devices when an IRQ is actually taken
typedef int (*device_irq_callback)(device_t *device, int irqnum);



// ======================> device_config_execute_interface

// class representing interface-specific configuration state
class device_config_execute_interface : public device_config_interface
{
	friend class device_execute_interface;

public:
	// construction/destruction
	device_config_execute_interface(const machine_config &mconfig, device_config &devconfig);
	virtual ~device_config_execute_interface();

	// basic information getters
	bool disabled() const { return m_disabled; }

	// clock and cycle information getters
	UINT64 clocks_to_cycles(UINT64 clocks) const { return execute_clocks_to_cycles(clocks); }
	UINT64 cycles_to_clocks(UINT64 cycles) const { return execute_cycles_to_clocks(cycles); }
	UINT32 min_cycles() const { return execute_min_cycles(); }
	UINT32 max_cycles() const { return execute_max_cycles(); }

	// input line information getters
	UINT32 input_lines() const { return execute_input_lines(); }
	UINT32 default_irq_vector() const { return execute_default_irq_vector(); }

protected:
	// clock and cycle information getters
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const;
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const;
	virtual UINT32 execute_min_cycles() const;
	virtual UINT32 execute_max_cycles() const;

	// input line information getters
	virtual UINT32 execute_input_lines() const;
	virtual UINT32 execute_default_irq_vector() const;

	// optional operation overrides
	virtual bool interface_process_token(UINT32 entrytype, const machine_config_token *&tokens);
	virtual bool interface_validity_check(const game_driver &driver) const;

	bool					m_disabled;
	device_interrupt_func	m_vblank_interrupt;				// for interrupts tied to VBLANK
	int 					m_vblank_interrupts_per_frame;	// usually 1
	const char *			m_vblank_interrupt_screen;		// the screen that causes the VBLANK interrupt
	device_interrupt_func	m_timed_interrupt;				// for interrupts not tied to VBLANK
	UINT64					m_timed_interrupt_period;		// period for periodic interrupts
};



// ======================> device_execute_interface

class device_execute_interface : public device_interface
{
	friend class device_scheduler;

public:
	// construction/destruction
	device_execute_interface(running_machine &machine, const device_config &config, device_t &device);
	virtual ~device_execute_interface();

	// configuration access
	const device_config_execute_interface &execute_config() const { return m_execute_config; }

	// basic information getters
	bool disabled() const { return m_execute_config.disabled(); }

	// execution management
	bool executing() const;
	INT32 cycles_remaining() const;
	void eat_cycles(int cycles);
	void adjust_icount(int delta);
	void abort_timeslice();

	// input and interrupt management
	void set_input_line(int linenum, int state) { m_input[linenum].set_state_synced(state); }
	void set_input_line_vector(int linenum, int vector) { m_input[linenum].set_vector(vector); }
	void set_input_line_and_vector(int linenum, int state, int vector) { m_input[linenum].set_state_synced(state, vector); }
	int input_state(int linenum) { return m_input[linenum].m_curstate; }
	void set_irq_callback(device_irq_callback callback);

	// deprecated, but still needed for older drivers
	int iloops() const { return m_iloops; }

	// suspend/resume
	void suspend(UINT32 reason, bool eatcycles);
	void resume(UINT32 reason);
	bool suspended(UINT32 reason = SUSPEND_ANY_REASON) { return (m_nextsuspend & reason) != 0; }
	void yield() { suspend(SUSPEND_REASON_TIMESLICE, false); }
	void spin() { suspend(SUSPEND_REASON_TIMESLICE, true); }
	void spin_until_trigger(int trigid) { suspend_until_trigger(trigid, true); }
	void spin_until_time(attotime duration);
	void spin_until_interrupt() { spin_until_trigger(m_inttrigger); }

	// triggers
	void suspend_until_trigger(int trigid, bool eatcycles);
	void trigger(int trigid);
	void signal_interrupt_trigger() { trigger(m_inttrigger); }

	// time and cycle accounting
	attotime local_time() const;
	UINT64 total_cycles() const;

	// clock and cycle information getters ... pass through to underlying config
	UINT64 clocks_to_cycles(UINT64 clocks) const { return m_execute_config.clocks_to_cycles(clocks); }
	UINT64 cycles_to_clocks(UINT64 cycles) const { return m_execute_config.cycles_to_clocks(cycles); }
	UINT32 min_cycles() const { return m_execute_config.min_cycles(); }
	UINT32 max_cycles() const { return m_execute_config.max_cycles(); }
	attotime cycles_to_attotime(UINT64 cycles) const { return device().clocks_to_attotime(cycles_to_clocks(cycles)); }
	UINT64 attotime_to_cycles(attotime duration) const { return clocks_to_cycles(device().attotime_to_clocks(duration)); }

	// input line information getters
	UINT32 input_lines() const { return m_execute_config.input_lines(); }
	UINT32 default_irq_vector() const { return m_execute_config.default_irq_vector(); }

	// required operation overrides
	void run() { execute_run(); }

protected:
	// optional operation overrides
	virtual void execute_run() = 0;
	virtual void execute_burn(INT32 cycles);
	virtual void execute_set_input(int linenum, int state);

	// interface-level overrides
	virtual void interface_pre_start();
	virtual void interface_post_start();
	virtual void interface_pre_reset();
	virtual void interface_post_reset();
	virtual void interface_clock_changed();

	// for use by devcpu for now...
	static IRQ_CALLBACK( static_standard_irq_callback );
	int standard_irq_callback(int irqline);

	// internal information about the state of inputs
	class device_input
	{
		static const int USE_STORED_VECTOR = 0xff000000;

	public:
		device_input();

		void start(device_execute_interface *execute, int linenum);
		void reset();

		void set_state_synced(int state, int vector = USE_STORED_VECTOR);
		void set_vector(int vector) { m_stored_vector = vector; }
		int default_irq_callback();

		device_execute_interface *m_execute;// pointer to the execute interface
		device_t *		m_device;			// pointer to our device
		int				m_linenum;			// which input line we are

		INT32			m_stored_vector;	// most recently written vector
		INT32			m_curvector;		// most recently processed vector
		UINT8			m_curstate;			// most recently processed state
		INT32			m_queue[32];		// queue of pending events
		int				m_qindex;			// index within the queue

	private:
		static void static_empty_event_queue(running_machine *machine, void *ptr, int param);
		void empty_event_queue();
	};

	// configuration
	running_machine &		m_machine;					// reference to owning machine
	const device_config_execute_interface &m_execute_config;	// reference to our device_config_execute_interface

	// execution lists
	device_execute_interface *m_nextexec;				// pointer to the next device to execute, in order

	// input states and IRQ callbacks
	device_irq_callback		m_driver_irq;				// driver-specific IRQ callback
	device_input			m_input[MAX_INPUT_LINES];	// data about inputs
	emu_timer *				m_timedint_timer;			// reference to this device's periodic interrupt timer

	// these below are hacks to support multiple interrupts per frame
	INT32					m_iloops;					// number of interrupts remaining this frame
	emu_timer *				m_partial_frame_timer;		// the timer that triggers partial frame interrupts
	attotime				m_partial_frame_period;		// the length of one partial frame for interrupt purposes

	// cycle counting and executing
	int						m_profiler;					// profiler tag
	int *					m_icount;					// pointer to the icount
	int 					m_cycles_running;			// number of cycles we are executing
	int						m_cycles_stolen;			// number of cycles we artificially stole

	// suspend states
	UINT32					m_suspend;					// suspend reason mask (0 = not suspended)
	UINT32					m_nextsuspend;				// pending suspend reason mask
	UINT8					m_eatcycles;				// true if we eat cycles while suspended
	UINT8					m_nexteatcycles;			// pending value
	INT32					m_trigger;					// pending trigger to release a trigger suspension
	INT32					m_inttrigger;				// interrupt trigger index

	// clock and timing information
	UINT64					m_totalcycles;				// total device cycles executed
	attotime				m_localtime;				// local time, relative to the timer system's global time
	INT32					m_divisor;					// 32-bit attoseconds_per_cycle divisor
	UINT8					m_divshift;					// right shift amount to fit the divisor into 32 bits
	UINT32					m_cycles_per_second;		// cycles per second, adjusted for multipliers
	attoseconds_t			m_attoseconds_per_cycle;	// attoseconds per adjusted clock cycle

private:
	// callbacks
	static void static_timed_trigger_callback(running_machine *machine, void *ptr, int param);

	static void static_on_vblank(screen_device &screen, void *param, bool vblank_state);
	void on_vblank_start(screen_device &screen);

	static void static_trigger_partial_frame_interrupt(running_machine *machine, void *ptr, int param);
	void trigger_partial_frame_interrupt();

	static void static_trigger_periodic_interrupt(running_machine *machine, void *ptr, int param);
	void trigger_periodic_interrupt();

	attoseconds_t minimum_quantum() const;
};



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  device_execute - return a pointer to the device
//  execute interface for this device
//-------------------------------------------------

inline device_execute_interface *device_execute(device_t *device)
{
	device_execute_interface *intf;
	if (!device->interface(intf))
		throw emu_fatalerror("Device '%s' does not have execute interface", device->tag());
	return intf;
}



// ======================> device scheduling

// suspend the given device for a specific reason
inline void device_suspend(device_t *device, int reason, bool eatcycles)
{
	device_execute(device)->suspend(reason, eatcycles);
}

// resume the given device for a specific reason
inline void device_resume(device_t *device, int reason)
{
	device_execute(device)->resume(reason);
}



// ======================> synchronization helpers

// yield the given device until the end of the current timeslice
inline void device_yield(device_t *device)
{
	device_execute(device)->yield();
}

// burn device cycles until the end of the current timeslice
inline void device_spin(device_t *device)
{
	device_execute(device)->spin();
}

// burn specified device cycles until a trigger
inline void device_spin_until_trigger(device_t *device, int trigger)
{
	device_execute(device)->spin_until_trigger(trigger);
}

// burn device cycles for a specific period of time
inline void device_spin_until_time(device_t *device, attotime duration)
{
	device_execute(device)->spin_until_time(duration);
}



// ======================> device timing

// returns the current local time for a device
inline attotime device_get_local_time(device_t *device)
{
	return device_execute(device)->local_time();
}

// safely eats cycles so we don't cross a timeslice boundary
inline void device_eat_cycles(device_t *device, int cycles)
{
	device_execute(device)->eat_cycles(cycles);
}

// apply a +/- to the current icount
inline void device_adjust_icount(device_t *device, int delta)
{
	device_execute(device)->adjust_icount(delta);
}

// abort execution for the current timeslice, allowing other devices to run before we run again
inline void device_abort_timeslice(device_t *device)
{
	device_execute(device)->abort_timeslice();
}



// ======================> triggers

// generate a trigger corresponding to an interrupt on the given device
inline void device_triggerint(device_t *device)
{
	device_execute(device)->signal_interrupt_trigger();
}



// ======================> interrupts

// set the logical state (ASSERT_LINE/CLEAR_LINE) of the an input line on a device
inline void device_set_input_line(device_t *device, int line, int state)
{
	device_execute(device)->set_input_line(line, state);
}

// set the vector to be returned during a device's interrupt acknowledge cycle
inline void device_set_input_line_vector(device_t *device, int line, int vector)
{
	device_execute(device)->set_input_line_vector(line, vector);
}

// set the logical state (ASSERT_LINE/CLEAR_LINE) of the an input line on a device and its associated vector
inline void device_set_input_line_and_vector(device_t *device, int line, int state, int vector)
{
	device_execute(device)->set_input_line_and_vector(line, state, vector);
}

// install a driver-specific callback for IRQ acknowledge
inline void device_set_irq_callback(device_t *device, device_irq_callback callback)
{
	device_execute(device)->set_irq_callback(callback);
}



// ======================> additional helpers

// burn device cycles until the next interrupt
inline void device_spin_until_interrupt(device_t *device)
{
	device_execute(device)->spin_until_interrupt();
}



#endif	/* __DIEXEC_H__ */
