/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Private kernel definitions
 *
 * This file contains private kernel function/macro definitions and various
 * other definitions for the Ultiparc processor architecture.
 *
 * This file is also included by assembly language files which must #define
 * _ASMLANGUAGE before including this header file.  Note that kernel
 * assembly source files obtains structure offset values via "absolute
 * symbols" in the offsets.o module.
 */

#ifndef _kernel_arch_func__h_
#define _kernel_arch_func__h_

#ifdef __cplusplus
extern "C" {
#endif


#ifndef _ASMLANGUAGE

void k_cpu_idle(void);
void k_cpu_atomic_idle(unsigned int key);


static ALWAYS_INLINE void kernel_arch_init(void)
{
	_kernel.nested = 0;
	_kernel.irq_stack =
		K_THREAD_STACK_BUFFER(_interrupt_stack) + CONFIG_ISR_STACK_SIZE;
}

static ALWAYS_INLINE void
_set_thread_return_value(struct k_thread *thread, unsigned int value)
{
	thread->arch.swap_return_value = value;
}

static inline void _IntLibInit(void)
{
	/* No special initialization of the interrupt subsystem required */
}

#define _is_in_isr() (_kernel.nested != 0)

#ifdef CONFIG_IRQ_OFFLOAD
void _irq_do_offload(void);
#endif

#endif /* _ASMLANGUAGE */


#ifdef __cplusplus
}
#endif

#endif /* _kernel_arch_func__h_ */
