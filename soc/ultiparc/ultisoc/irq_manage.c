/*
 * Copyright (c) 2019 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Ultiparc C-domain interrupt management code for use with
 * UltiSoC SoC Interrupt Controller
 */


#include <kernel.h>
#include <kernel_structs.h>
#include <arch/cpu.h>
#include <arch/ultiparc/irq_controller.h>
#include <irq.h>
#include <sw_isr_table.h>
#include <kswap.h>
#include <tracing.h>


/**
 * @brief Interrupt demux function
 *
 */
void _soc_enter_irq(void)
{
	int index;

	_kernel.nested++;


#ifdef CONFIG_IRQ_OFFLOAD
	_irq_do_offload();
#endif

	while((index = irq_controller_isr_vector_get()) != -1) {
		struct _isr_table_entry *ite = &_sw_isr_table[index];

		z_sys_trace_isr_enter();

		ite->isr(ite->arg);
		irq_controller_ack(index);

		sys_trace_isr_exit();
	}

	_kernel.nested--;

#ifdef CONFIG_STACK_SENTINEL
	_check_stack_sentinel();
#endif
}
