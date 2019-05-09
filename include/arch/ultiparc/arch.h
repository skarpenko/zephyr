/*
 * Copyright (c) 2018-2019 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Ultiparc specific kernel interface header
 * This header contains the Ultiparc specific kernel interface.  It is
 * included by the generic kernel interface header (include/arch/cpu.h)
 */

#ifndef ZEPHYR_INCLUDE_ARCH_ULTIPARC_ARCH_H_
#define ZEPHYR_INCLUDE_ARCH_ULTIPARC_ARCH_H_

#include <arch/ultiparc/asm_inline.h>

#ifdef __cplusplus
extern "C" {
#endif


#define STACK_ALIGN	4


#define _NANO_ERR_CPU_EXCEPTION		(0)	/* Any unhandled exception */
#define _NANO_ERR_STACK_CHK_FAIL	(2)	/* Stack corruption detected */
#define _NANO_ERR_ALLOCATION_FAIL	(3)	/* Kernel Allocation Failure */
#define _NANO_ERR_SPURIOUS_INT		(4)	/* Spurious interrupt */
#define _NANO_ERR_KERNEL_OOPS		(5)	/* Kernel oops (fatal to thread) */
#define _NANO_ERR_KERNEL_PANIC		(6)	/* Kernel panic (fatal to system) */

/* APIs need to support non-byte addressable architectures */

#define OCTET_TO_SIZEOFUNIT(X) (X)
#define SIZEOFUNIT_TO_OCTET(X) (X)

#ifndef _ASMLANGUAGE
#include <zephyr/types.h>
#include <irq.h>
#include <sw_isr_table.h>

/* physical/virtual address types required by the kernel */
typedef unsigned int paddr_t;
typedef unsigned int vaddr_t;

/**
 * Configure a static interrupt.
 *
 * All arguments must be computable by the compiler at build time.
 *
 * Internally this function does a few things:
 *
 * 1. The enum statement has no effect but forces the compiler to only
 * accept constant values for the irq_p parameter, very important as the
 * numerical IRQ line is used to create a named section.
 *
 * 2. An instance of struct _isr_table_entry is created containing the ISR and
 * its parameter. If you look at how _sw_isr_table is created, each entry in
 * the array is in its own section named by the IRQ line number. What we are
 * doing here is to override one of the default entries (which points to the
 * spurious IRQ handler) with what was supplied here.
 *
 * There is no notion of priority with the Ultiparc SoC interrupt
 * controller and no flags are currently supported.
 *
 * @param irq_p IRQ line number
 * @param priority_p Interrupt priority (ignored)
 * @param isr_p Interrupt service routine
 * @param isr_param_p ISR parameter
 * @param flags_p IRQ triggering options (currently unused)
 *
 * @return The vector assigned to this interrupt
 */
#define _ARCH_IRQ_CONNECT(irq_p, priority_p, isr_p, isr_param_p, flags_p) \
({ \
	_ISR_DECLARE(irq_p, 0, isr_p, isr_param_p); \
	irq_p; \
})

extern void _irq_spurious(void *unused);

static ALWAYS_INLINE unsigned int _arch_irq_lock(void)
{
	u32_t key;
	__asm__ __volatile__ (
		".set push         ;"
		".set noreorder    ;"
		"mfc0 %0, $12      ;"
		"li $t0, ~1        ;"
		"and $t0, %0, $t0  ;"
		"mtc0 $t0, $12     ;"
		"nop               ;"
		".set pop          ;"
		: "=r" (key)
		:
		: "$t0"
	);
	return key & 0x01;
}

static ALWAYS_INLINE void _arch_irq_unlock(unsigned int key)
{
	__asm__ __volatile__ (
		".set push             ;"
		".set noreorder        ;"
		"mfc0 $t0, $12         ;"
		"li $t1, ~1            ;"
		"and $t0, $t0, $t1     ;"
		"or $t0, $t0, %0       ;"
		"mtc0 $t0, $12         ;"
		"nop                   ;"
		"nop                   ;"
		".set pop              ;"
		:
		: "r" (key)
		: "$t0", "$t1"
	);
}

/**
 * @brief Explicitly nop operation.
 */
static ALWAYS_INLINE void arch_nop(void)
{
	__asm__ __volatile__("nop;");
}


struct __esf {
	u32_t vec;	/* Vector number */
	u32_t psr;	/* Previous status */
	u32_t sr;	/* Status register */
	u32_t cause;	/* Cause register */
	u32_t epc;	/* EPC */
	u32_t lo;	/* LO special register */
	u32_t hi;	/* HI special register */
	/* General purpose registers follow */
	u32_t ra;
	u32_t t9;
	u32_t t8;
	u32_t t7;
	u32_t t6;
	u32_t t5;
	u32_t t4;
	u32_t t3;
	u32_t t2;
	u32_t t1;
	u32_t t0;
	u32_t a3;
	u32_t a2;
	u32_t a1;
	u32_t a0;
	u32_t v1;
	u32_t v0;
	u32_t at;
};

typedef struct __esf NANO_ESF;
extern const NANO_ESF _default_esf;

FUNC_NORETURN void _SysFatalErrorHandler(unsigned int reason,
					 const NANO_ESF *esf);

FUNC_NORETURN void _NanoFatalErrorHandler(unsigned int reason,
					  const NANO_ESF *esf);

enum ultiparc_exception_cause {
	ULTIPARC_EXCEPTION_UNKNOWN		= -1,
	ULTIPARC_EXCEPTION_RESET		= 0,
	ULTIPARC_EXCEPTION_BUS_ERROR		= 1,
	ULTIPARC_EXCEPTION_INTEGER_OVERFLOW	= 2,
	ULTIPARC_EXCEPTION_ADDRESS_ERROR	= 3,
	ULTIPARC_EXCEPTION_RESERVED_INSTRUCTION	= 4,
	ULTIPARC_EXCEPTION_BREAKPOINT		= 5,
	ULTIPARC_EXCEPTION_SYSTEM_CALL		= 6,
	ULTIPARC_EXCEPTION_HARDWARE_INTERRUPT	= 7
};


#endif /* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#include "ultiparc.h"

#if defined(CONFIG_SOC_ULTIPARC_RTL)
# include "rtl/ultiparc_rtl.h"
#endif

#if defined(CONFIG_SOC_ULTISOC)
# include "soc/ultisoc.h"
#endif

#endif /* ZEPHYR_INCLUDE_ARCH_ULTIPARC_ARCH_H_ */
