/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __ULTIPARC_H__
#define __ULTIPARC_H__


/*
 * This header provides processor specific macros for Ultiparc.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* Size in bits of registers */
#define SYSTEM_BUS_WIDTH 32


/* Processor Id */
#define ULTIPARC_CPU_ID		0x001A8100


/* Coprocessor 0 registers */
#define TSCLO	8	/* Low half of timestamp counter */
#define TSCHI	9	/* Upper half of timestamp counter */
#define IVTB	10	/* IVT Base */
#define PSR	11	/* Prev. Status Register  */
#define SR	12	/* Status Register */
#define CAUSE	13	/* Cause Register */
#define EPC	14	/* Exception PC */
#define PRID	15	/* Processor Id */


#define SR_IE_MASK	0x01		/* Interrupt enable mask */
#define CAUSE_BD_MASK	0x80000000	/* Interrupt enable mask */


#if !defined(_ASMLANGUAGE)

#include <zephyr/types.h>
#include <arch/cpu.h>
#include <sys_io.h>


/* Wait for interrupt */
static inline void waiti(void)
{
	__asm__ __volatile__ (
		".set push        ;"
		".set noreorder   ;"
		".long 0x42000020 ;"
		"nop              ;"
		"nop              ;"
		".set pop         ;"
		:
		:
		:
	);
}


/* Enable interrupts and wait for interrupt */
static inline void waiti_safe(void)
{
	__asm__ __volatile__ (
		".set push        ;"
		".set noreorder   ;"
		"mfc0 $t0, $12    ;"
		"ori $t0, $t0, 1  ;"
		"mtc0 $t0, $12    ;"
		".long 0x42000020 ;"
		"nop              ;"
		"nop              ;"
		".set pop         ;"
		:
		:
		: "$t0"
	);
}


/* Read lower half of timestamp counter */
static inline u32_t rdtsc_lo(void)
{
	u32_t v;
	__asm__ __volatile__ (
		".set push       ;"
		".set noreorder  ;"
		"mfc0 %0, $8     ;"
		".set pop        ;"
		: "=r" (v)
		:
		:
	);

	return v;
}


/* Read upper half of timestamp counter */
static inline u32_t rdtsc_hi(void)
{
	u32_t v;
	__asm__ __volatile__ (
		".set push       ;"
		".set noreorder  ;"
		"mfc0 %0, $9     ;"
		".set pop        ;"
		: "=r" (v)
		:
		:
	);

	return v;
}


/* Read timestamp counter */
static inline u64_t rdtsc(void)
{
	u32_t lo = rdtsc_lo();	/* Read lower half first to latch upper half */
	u32_t hi = rdtsc_hi();
	return ((u64_t)hi << 32) | lo;
}


/* Read timestamp counter (interrupt safe) */
static inline u64_t rdtsc_safe(void)
{
	unsigned int intstate = _arch_irq_lock();
	u32_t lo = rdtsc_lo();
	u32_t hi = rdtsc_hi();
	_arch_irq_unlock(intstate);
	return ((u64_t)hi << 32) | lo;
}

#elif defined(_ASMLANGUAGE)


/* WAIT instruction code */
#define WAITI	.long 0x42000020


#endif /* _ASMLANGUAGE */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ULTIPARC_H__ */
