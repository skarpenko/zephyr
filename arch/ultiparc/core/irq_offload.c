/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <kernel_structs.h>
#include <irq_offload.h>


volatile irq_offload_routine_t _offload_routine;
static volatile void *offload_param;


/* Called by _enter_irq if it was passed 0 for ipending.
 * Just in case the offload routine itself generates an unhandled
 * exception, clear the offload_routine global before executing.
 */
void _irq_do_offload(void)
{
	irq_offload_routine_t tmp;

	if (!_offload_routine) {
		return;
	}

	tmp = _offload_routine;
	_offload_routine = NULL;

	tmp((void *)offload_param);
}


void irq_offload(irq_offload_routine_t routine, void *parameter)
{
	int key;

	key = irq_lock();
	_offload_routine = routine;
	offload_param = parameter;

	__asm__ __volatile__ ("syscall 0x0");

	irq_unlock(key);
}
