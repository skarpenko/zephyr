/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <arch/ultiparc/asm.h>
#include <arch/ultiparc/arch.h>
#include <kernel_structs.h>


.set noreorder


/* exports */
GTEXT(__start)
GTEXT(__reset)

/* imports */
GTEXT(_PrepC)
GTEXT(_interrupt_stack)
GTEXT(__exception_entry)


SECTION_FUNC(reset, __reset)
__reset:
__exception_vectors:
	j __start		/* Jump to __start */
	mtc0 $zero, $SR		/* Disable interrupts */
/* exception entries */
__bus_error_exception:
	j __exception_entry
	addiu $k0, $zero, 1
__overflow_exception:
	j __exception_entry
	addiu $k0, $zero, 2
__address_error_exception:
	j __exception_entry
	addiu $k0, $zero, 3
__resvd_instr_exception:
	j __exception_entry
	addiu $k0, $zero, 4
__break_exception:
	j __exception_entry
	addiu $k0, $zero, 5
__syscall_exception:
	j __exception_entry
	addiu $k0, $zero, 6
__hardware_interrupt:
	j __exception_entry
	addiu $k0, $zero, 7


/* use ABI name of registers for the sake of simplicity */

/*
 * Remainder of asm-land initialization code before we can jump into
 * the C domain
 */
SECTION_FUNC(TEXT, __start)
	/*
	 * Update address of exception vectors table in case if we
	 * started by bootloader. __reset matches exception vectors table
	 * base address and it must be 1KB aligned.
	 */
	la $t0, __exception_vectors
	mtc0 $t0, $IVTB
	nop
	nop

	/* set Global Pointer */
	la $gp, _gp

#ifdef CONFIG_INIT_STACKS
	/* Pre-populate all bytes in _interrupt_stack with 0xAA */
	la $t0, _interrupt_stack
	li $t1, CONFIG_ISR_STACK_SIZE
	addu $t1, $t1, $t0

	/* Populate _interrupt_stack with 0xaaaaaaaa */
	li $t2, 0xaaaaaaaa
aa_loop:
	sw $t2, 0x00($t0)
	addiu $t0, $t0, 4
	blt $t0, $t1, aa_loop
	nop
#endif

	/*
	 * Initially, setup stack pointer to
	 * _interrupt_stack + CONFIG_ISR_STACK_SIZE
	 */
	la $sp, _interrupt_stack
	li $t0, CONFIG_ISR_STACK_SIZE
	addu $sp, $sp, $t0

	/*
	 * Jump into C domain. _PrepC zeroes BSS, copies rw data into RAM,
	 * and then enters kernel _Cstart
	 */
	j _PrepC
	nop
