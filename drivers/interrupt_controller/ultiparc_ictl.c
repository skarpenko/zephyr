/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Ultiparc Interrupt Controller
 */

/* includes */

#include <kernel.h>
#include <arch/cpu.h>
#include <misc/__assert.h>
#include <misc/util.h>
#include <sys_io.h>
#include <init.h>
#include <inttypes.h>
#include <device.h>


/* Interrupt controller registers */
#define ICTL_STATUS_REG		(CONFIG_ULTIPARC_ICTL_BASE_ADDRESS + 0x00)
#define ICTL_INTMASK_REG	(CONFIG_ULTIPARC_ICTL_BASE_ADDRESS + 0x04)
#define ICTL_RAWINT_REG		(CONFIG_ULTIPARC_ICTL_BASE_ADDRESS + 0x08)


/**
 *
 * @brief initialize the interrupt controller of Ultiparc SoC.
 *
 * @returns: N/A
 */
static int _ultiparc_ictl_init(struct device *unused)
{
	ARG_UNUSED(unused);

	/* Mask all interrupt lines */
	sys_write32(0, ICTL_INTMASK_REG);

	/* Discard pending interrupts if any */
	sys_write32(0xFFFFFFFF, ICTL_STATUS_REG);

	return 0;

}
SYS_INIT(_ultiparc_ictl_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);


void _arch_irq_enable(unsigned int irq)
{
	unsigned int irqstate;
	u32_t mask;

	irqstate = irq_lock();

	mask = sys_read32(ICTL_INTMASK_REG);
	mask |= (1 << irq);
	sys_write32(mask, ICTL_INTMASK_REG);

	irq_unlock(irqstate);
}


void _arch_irq_disable(unsigned int irq)
{
	unsigned int irqstate;
	u32_t mask;

	irqstate = irq_lock();

	mask = sys_read32(ICTL_INTMASK_REG);
	mask &= ~(1 << irq);
	sys_write32(mask, ICTL_INTMASK_REG);

	irq_unlock(irqstate);
}


/**
 * @brief Find the currently executing interrupt vector, if any
 *
 * This routine finds the vector of the interrupt that is being processed.
 * The IC Status register contain the vectors of the interrupts
 * in service. And the lower vector is the identification of the interrupt
 * being currently processed.
 *
 * @return The vector of the interrupt that is currently being processed, or
 * -1 if this can't be determined
 */
int irq_controller_isr_vector_get(void)
{
	/* In-service register value */
	u32_t isr;

	isr = sys_read32(ICTL_STATUS_REG);
	if (unlikely(!isr))
		return -1;

	return find_lsb_set(isr) - 1;
}


/**
 * @brief Acknowledge IRQ
 */
void irq_controller_ack(unsigned int irq)
{
	sys_write32(1 << irq, ICTL_STATUS_REG);
}
