/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <arch/cpu.h>
#include <misc/util.h>
#include <sys_io.h>
#include <device.h>
#include <init.h>
#include <inttypes.h>
#include <system_timer.h>
#include <soc.h>


/* Timer registers */
#define TIMER_CTRL_REG		(CONFIG_ULTIPARC_TIMER_BASE_ADDRESS + 0x00)
#define TIMER_INTVL_REG		(CONFIG_ULTIPARC_TIMER_BASE_ADDRESS + 0x04)
#define TIMER_CURRV_REG		(CONFIG_ULTIPARC_TIMER_BASE_ADDRESS + 0x08)

/* Control register bits */
#define TIMER_EN	0x01	/* Timer enable */
#define TIMER_IE	0x02	/* Interrupt enable */
#define TIMER_RLD	0x04	/* Reload counter */


static u32_t accumulated_cycle_count;


static void ultiparc_timer_irq_handler(void *unused)
{
	ARG_UNUSED(unused);

	accumulated_cycle_count += sys_clock_hw_cycles_per_tick();

	z_clock_announce(1);
}


#ifdef CONFIG_TICKLESS_IDLE
#error "Tickless idle not yet implemented for pulpino timer"
#endif


int z_clock_driver_init(struct device *device)
{
	ARG_UNUSED(device);

	/* Setup timer ISR */
	IRQ_CONNECT(ULTIPARC_RTL_TIMER_IRQ, 0,
			ultiparc_timer_irq_handler, NULL, 0);
	irq_enable(ULTIPARC_RTL_TIMER_IRQ);


	/*
	 * Reset counter and set timer to generate interrupt
	 * every sys_clock_hw_cycles_per_tick()
	 */
	sys_write32(sys_clock_hw_cycles_per_tick(), TIMER_INTVL_REG);
	sys_write32(TIMER_EN | TIMER_IE | TIMER_RLD, TIMER_CTRL_REG);


	return 0;
}


/**
 *
 * @brief Read the platform's timer hardware
 *
 * This routine returns the current time in terms of timer hardware clock
 * cycles.
 *
 * @return up counter of elapsed clock cycles
 */
u32_t _timer_cycle_get_32(void)
{
	u32_t curv = sys_clock_hw_cycles_per_tick() - sys_read32(TIMER_CURRV_REG);
	return accumulated_cycle_count + curv;
}


u32_t z_clock_elapsed(void)
{
	return 0;
}
