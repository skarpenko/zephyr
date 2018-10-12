/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <arch/ultiparc/asm.h>
#include <arch/ultiparc/arch.h>
#include <kernel_structs.h>
#include <offsets_short.h>


.set noreorder


/* exports */
GTEXT(__swap)
GTEXT(_thread_entry_wrapper)

/* imports */
GTEXT(z_sys_trace_thread_switched_in)
GTEXT(_k_neg_eagain)


/* unsigned int __swap(unsigned int key)
 *
 * Always called with interrupts locked
 */
SECTION_FUNC(TEXT, __swap)
	/* Get a reference to _kernel in $t0 */
	la $t0, _kernel

	/* Get the pointer to kernel->current */
	lw  $t1, _kernel_offset_to_current($t0)
	nop

	/* Store all the callee saved registers. We either got here via
	 * an exception or from a cooperative invocation of __swap() from C
	 * domain, so all the caller-saved registers have already been
	 * saved by the exception asm or the calling C code already.
	 */
	sw $s0, _thread_offset_to_s0($t1)
	sw $s1, _thread_offset_to_s1($t1)
	sw $s2, _thread_offset_to_s2($t1)
	sw $s3, _thread_offset_to_s3($t1)
	sw $s4, _thread_offset_to_s4($t1)
	sw $s5, _thread_offset_to_s5($t1)
	sw $s6, _thread_offset_to_s6($t1)
	sw $s7, _thread_offset_to_s7($t1)
	sw $sp, _thread_offset_to_sp($t1)
	sw $fp, _thread_offset_to_fp($t1)
	sw $gp, _thread_offset_to_gp($t1)
	sw $ra, _thread_offset_to_ra($t1)

	/* Populate default return value */
	la $t2, _k_neg_eagain
	lw $t3, 0($t2)
	nop
	sw $t3, _thread_offset_to_swap_return_value($t1)

	/* Save irq state passed in key */
	sw $a0, _thread_offset_to_irq_lock_state($t1)


#if CONFIG_TRACING
	jal z_sys_trace_thread_switched_in
	nop
	/* restore caller-saved $t0 */
	la $t0, _kernel
#endif

	/* get cached thread to run */
	lw $t2, _kernel_offset_to_ready_q_cache($t0)
	nop

	/* At this point $t2 points to the next thread to be swapped in */

	/* the thread to be swapped in is now the current thread */
	sw $t2, _kernel_offset_to_current($t0)

	/* Restore callee-saved registers and switch to the incoming
	 * thread's stack
	 */
	lw $s0, _thread_offset_to_s0($t2)
	lw $s1, _thread_offset_to_s1($t2)
	lw $s2, _thread_offset_to_s2($t2)
	lw $s3, _thread_offset_to_s3($t2)
	lw $s4, _thread_offset_to_s4($t2)
	lw $s5, _thread_offset_to_s5($t2)
	lw $s6, _thread_offset_to_s6($t2)
	lw $s7, _thread_offset_to_s7($t2)
	lw $sp, _thread_offset_to_sp($t2)
	lw $fp, _thread_offset_to_fp($t2)
	lw $gp, _thread_offset_to_gp($t2)
	lw $ra, _thread_offset_to_ra($t2)

	/*
	 * Load return value into $t2 (return value register). -EAGAIN unless
	 * someone previously called _set_thread_return_value(). Do this before
	 * we potentially unlock interrupts.
	 */
	lw $v0, _thread_offset_to_swap_return_value($t2)

	/* load irq lock state */
	lw $a0, _thread_offset_to_irq_lock_state($t2)
	nop

	/*
	 * Restore interrupts
	 * $a0 = key
	 */
	mtc0 $a0, $SR
	nop
	nop

	jr $ra
	nop


/* void _thread_entry_wrapper(void)
 */
SECTION_FUNC(TEXT, _thread_entry_wrapper)
	/* This all corresponds to struct init_stack_frame defined in
	 * thread.c. We need to take this stuff off the stack and put
	 * it in the apporpriate registers
	 */

	/* Can't return from here, just put NULL in $ra */
	addu $ra, $zero, $zero

	/* Calling convention has first 4 arguments in registers $a0-$a3. */

	lw  $a0, 0($sp)
	lw  $a1, 4($sp)
	lw  $a2, 8($sp)
	lw  $a3, 12($sp)

	/* pop all the stuff that we just loaded into registers */
	addiu $sp, $sp, 16

	j _thread_entry
	nop
