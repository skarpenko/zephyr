/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Per-arch thread definition
 *
 * This file contains definitions for
 *
 *  struct _thread_arch
 *  struct _callee_saved
 *  struct _caller_saved
 *
 * necessary to instantiate instances of struct k_thread.
 */

#ifndef _kernel_arch_thread__h_
#define _kernel_arch_thread__h_


#ifndef _ASMLANGUAGE
#include <zephyr/types.h>

/*
 * The following structure defines the list of registers that need to be
 * saved/restored when a cooperative context switch occurs.
 */
struct _callee_saved {
	u32_t sp;	/* Stack pointer */
	u32_t fp;	/* Frame pointer */
	u32_t gp;	/* Global pointer */
	u32_t ra;	/* Return address */
	u32_t s0;	/* Saved register */
	u32_t s1;	/* Saved register */
	u32_t s2;	/* Saved register */
	u32_t s3;	/* Saved register */
	u32_t s4;	/* Saved register */
	u32_t s5;	/* Saved register */
	u32_t s6;	/* Saved register */
	u32_t s7;	/* Saved register */
};
typedef struct _callee_saved _callee_saved_t;

struct _caller_saved {
	/*
	 * Nothing here, the exception code puts all the caller-saved
	 * registers onto the stack.
	 */
};

typedef struct _caller_saved _caller_saved_t;

struct _thread_arch {
	u32_t irq_lock_state;		/* IRQs state */
	u32_t swap_return_value;	/* Return value of _Swap() */
};

typedef struct _thread_arch _thread_arch_t;

#endif /* _ASMLANGUAGE */


#endif /* _kernel_arch_thread__h_ */
