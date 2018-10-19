/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <toolchain.h>
#include <linker/sections.h>
#include <arch/ultiparc/asm.h>
#include <arch/ultiparc/arch.h>
#include <kernel_structs.h>
#include <offsets_short.h>


/* exports */
GTEXT(__exception_entry)

/* import */
GTEXT(_Fault)
GTEXT(__swap)
#ifdef CONFIG_IRQ_OFFLOAD
GTEXT(_irq_do_offload)
GTEXT(_offload_routine)
#endif


.set noreorder
/* Allows use of r1/at register, otherwise reserved for assembler use */
.set noat

/* Common exceptions entry point */
SECTION_FUNC(TEXT, __exception_entry)
	/* Registers $k0 and $k1 are ours! */
	/* $k0 contains exception vector number (see reset.S) */

	/* Reserve thread stack space for saving context */
	addiu $sp, $sp, -__NANO_ESF_SIZEOF

	/* Preserve all caller-saved registers onto the thread's stack */
	sw $ra, __NANO_ESF_ra_OFFSET($sp)
	sw $t9, __NANO_ESF_t9_OFFSET($sp)
	sw $t8, __NANO_ESF_t8_OFFSET($sp)
	sw $t7, __NANO_ESF_t7_OFFSET($sp)
	sw $t6, __NANO_ESF_t6_OFFSET($sp)
	sw $t5, __NANO_ESF_t5_OFFSET($sp)
	sw $t4, __NANO_ESF_t4_OFFSET($sp)
	sw $t3, __NANO_ESF_t3_OFFSET($sp)
	sw $t2, __NANO_ESF_t2_OFFSET($sp)
	sw $t1, __NANO_ESF_t1_OFFSET($sp)
	sw $t0, __NANO_ESF_t0_OFFSET($sp)
	sw $a3, __NANO_ESF_a3_OFFSET($sp)
	sw $a2, __NANO_ESF_a2_OFFSET($sp)
	sw $a1, __NANO_ESF_a1_OFFSET($sp)
	sw $a0, __NANO_ESF_a0_OFFSET($sp)
	sw $v1, __NANO_ESF_v1_OFFSET($sp)
	sw $v0, __NANO_ESF_v0_OFFSET($sp)
	sw $at, __NANO_ESF_at_OFFSET($sp)

	/* store vector number */
	sw $k0, __NANO_ESF_vec_OFFSET($sp)

	/* store Previous Status register (PSR) */
	mfc0 $k1, $PSR
	sw $k1, __NANO_ESF_psr_OFFSET($sp)

	/* store Status register (SR) */
	mfc0 $k1, $SR
	sw $k1, __NANO_ESF_sr_OFFSET($sp)

	/* store Cause register */
	mfc0 $k1, $CAUSE
	sw $k1, __NANO_ESF_cause_OFFSET($sp)

	/* store exception return address */
	mfc0 $k1, $EPC
	sw $k1, __NANO_ESF_epc_OFFSET($sp)

	/* store HI/LO pair */
	mflo $k1
	sw $k1, __NANO_ESF_lo_OFFSET($sp)
	mfhi $k1
	sw $k1, __NANO_ESF_hi_OFFSET($sp)

	/* Figure out whether we are here because of an interrupt or an
	 * exception. If an interrupt, switch stacks and enter IRQ handling
	 * code. If an exception, remain on current stack and enter exception
	 * handing code.
	 */

	addiu $k1, $zero, 7	/* vector 7 is hardware interrupt request */
	bne $k0, $k1, not_interrupt
	nop

is_interrupt:
	/* If we get here, this is an interrupt */

	/* Grab a reference to _kernel in $t0 so we can determine the
	 * current irq stack pointer
	 */
	lui $t0, %hi(_kernel)
	addiu $t0, %lo(_kernel)

	/* Stash a copy of thread's $sp in $t1 so that we can put it on the IRQ
	 * stack
	 */
	move $t1, $sp

	/* Switch to interrupt stack */
	lw $sp, _kernel_offset_to_irq_stack($t0)
	nop

	/* Store thread stack pointer onto IRQ stack */
	addiu $sp, $sp, -4
	sw $t1, 0($sp)

on_irq_stack:

	/* Enter C interrupt handling code. */
	jal _soc_enter_irq
	nop

	/* Interrupt handler finished and the interrupt should be serviced */

	/* Get a reference to _kernel again in $t0 */
	lui $t0, %hi(_kernel)
	addiu $t0, %lo(_kernel)

#ifdef CONFIG_PREEMPT_ENABLED
	lw $t1, _kernel_offset_to_current($t0)
	nop
	/* Determine whether the exception of the ISR requires context
	 * switch
	 */

	/* Call into the kernel to see if a scheduling decision is necessary */
	lw $t2, _kernel_offset_to_ready_q_cache($t0)
	nop
	beq $t2, $t1, no_reschedule
	nop

	/*
	 * A context reschedule is required: keep the volatile registers of
	 * the interrupted thread on the context's stack.  Utilize
	 * the existing __swap() primitive to save the remaining
	 * thread's registers (including floating point) and perform
	 * a switch to the new thread.
	 */

	/* We put the thread stack pointer on top of the IRQ stack before
	 * we switched stacks. Restore it to go back to thread stack
	 */
	lw $sp, 0($sp)
	nop

	/* Argument to Swap() is IRQ status since that's the state of the
	 * status register before the exception happened. When coming
	 * out of the context switch we need this info to restore
	 * IRQ lock state.
	 */
	jal __swap
	mfc0 $a0, $PSR	/* Copy previous IRQ status */

	j _exception_exit
	nop
#else
	j no_reschedule
	nop
#endif /* CONFIG_PREEMPT_ENABLED */

not_interrupt:

	/* Since this wasn't an interrupt we're not going to restart the
	 * faulting instruction.
	 */

#ifdef CONFIG_IRQ_OFFLOAD
	/* Check the contents of _offload_routine. If non-NULL, jump into
	 * the interrupt code anyway.
	 */
	lui $t0, %hi(_offload_routine)
	addiu $t0, %lo(_offload_routine)
	lw $t1, 0($t0)
	nop
	bne $t1, $zero, is_interrupt
	nop
#endif

_exception_enter_fault:
	/* If we get here, the exception wasn't in interrupt or an
	 * invocation of irq_oflload(). Let _Fault() handle it in
	 * C domain
	 */
	jal _Fault
	move $a0, $sp /* arg = stack frame */

	j _exception_exit
	nop

no_reschedule:

	/* We put the thread stack pointer on top of the IRQ stack before
	 * we switched stacks. Restore it to go back to thread stack
	 */
	lw $sp, 0($sp)
	nop

	/* Fall through */

_exception_exit:
	/* We are on the thread stack. Restore all saved registers
	 * and return to the interrupted context */

	/* Disable interrupts */
	mtc0 $zero, $SR
	nop
	nop

	/* restore HI/LO pair */
	lw $k1, __NANO_ESF_lo_OFFSET($sp)
	nop
	mtlo $k1
	lw $k1, __NANO_ESF_hi_OFFSET($sp)
	nop
	mthi $k1

	/* copy exception return address to $k0 */
	lw $k0, __NANO_ESF_epc_OFFSET($sp)
	nop

	/* restore Cause register */
	lw $k1, __NANO_ESF_cause_OFFSET($sp)
	nop
	mtc0 $k1, $CAUSE

	/* restore Status register (SR) */
	lw $k1, __NANO_ESF_sr_OFFSET($sp)
	nop
	mtc0 $k1, $SR

	/* restore Previous Status register (PSR) */
	lw $k1, __NANO_ESF_psr_OFFSET($sp)
	nop
	mtc0 $k1, $PSR

	/* Restore caller-saved registers */
	lw $ra, __NANO_ESF_ra_OFFSET($sp)
	lw $t9, __NANO_ESF_t9_OFFSET($sp)
	lw $t8, __NANO_ESF_t8_OFFSET($sp)
	lw $t7, __NANO_ESF_t7_OFFSET($sp)
	lw $t6, __NANO_ESF_t6_OFFSET($sp)
	lw $t5, __NANO_ESF_t5_OFFSET($sp)
	lw $t4, __NANO_ESF_t4_OFFSET($sp)
	lw $t3, __NANO_ESF_t3_OFFSET($sp)
	lw $t2, __NANO_ESF_t2_OFFSET($sp)
	lw $t1, __NANO_ESF_t1_OFFSET($sp)
	lw $t0, __NANO_ESF_t0_OFFSET($sp)
	lw $a3, __NANO_ESF_a3_OFFSET($sp)
	lw $a2, __NANO_ESF_a2_OFFSET($sp)
	lw $a1, __NANO_ESF_a1_OFFSET($sp)
	lw $a0, __NANO_ESF_a0_OFFSET($sp)
	lw $v1, __NANO_ESF_v1_OFFSET($sp)
	lw $v0, __NANO_ESF_v0_OFFSET($sp)
	lw $at, __NANO_ESF_at_OFFSET($sp)

	/* Put the stack pointer back where it was when we entered
	 * exception state
	 */
	addiu $sp, $sp, __NANO_ESF_SIZEOF

	/* All done, return. */
	jr $k0
	rfe


/* SoC level code must override this */
.weak _soc_enter_irq
_soc_enter_irq:
	jal _irq_spurious
	nop
