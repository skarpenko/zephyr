/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <arch/cpu.h>
#include <kernel_structs.h>
#include <wait_q.h>
#include <string.h>


extern void _thread_entry_wrapper(k_thread_entry_t thread,
	void *arg1, void *arg2, void *arg3);


struct init_stack_frame {
	/* top of the stack / most recently pushed */

	/* Used by _thread_entry_wrapper. pulls these off the stack and
	 * into argument registers before calling _thread_entry()
	 */
	k_thread_entry_t entry_point;
	void *arg1;
	void *arg2;
	void *arg3;

	/* least recently pushed */
};


void _new_thread(struct k_thread *thread, k_thread_stack_t *stack,
		 size_t stack_size, k_thread_entry_t thread_func,
		 void *arg1, void *arg2, void *arg3,
		 int priority, unsigned int options)
{
	char *stack_memory = K_THREAD_STACK_BUFFER(stack);
	_ASSERT_VALID_PRIO(priority, thread_func);

	struct init_stack_frame *iframe;

	_new_thread_init(thread, stack_memory, stack_size, priority, options);

	/* Initial stack frame data, stored at the base of the stack */
	iframe = (struct init_stack_frame *)
		STACK_ROUND_DOWN(stack_memory + stack_size - sizeof(*iframe));

	/* Setup the initial stack frame */
	iframe->entry_point = thread_func;
	iframe->arg1 = arg1;
	iframe->arg2 = arg2;
	iframe->arg3 = arg3;

	extern u32_t _gp;
	thread->callee_saved.sp = (u32_t)iframe;
	thread->callee_saved.gp = (u32_t)&_gp;
	thread->callee_saved.ra = (u32_t)_thread_entry_wrapper;

	/* Leave the rest of thread->callee_saved junk */

	thread->arch.irq_lock_state = SR_IE_MASK;
}
