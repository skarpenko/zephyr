/*
 * Copyright (c) 2018-2019 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <arch/cpu.h>
#include <kernel_structs.h>
#include <misc/printk.h>
#include <inttypes.h>


const NANO_ESF _default_esf = {
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad,
	0xdeadbaad
};


#if defined(CONFIG_PRINTK)
static const char *cause_str(u32_t vec)
{
	switch (vec) {
	case 0:
		return "Reset";
	case 1:
		return "Bus Error";
	case 2:
		return "Integer Overflow";
	case 3:
		return "Address Error";
	case 4:
		return "Unimplemented Instruction";
	case 5:
		return "Breakpoint Trap";
	case 6:
		return "System Call Trap";
	case 7:
		return "Hardware Interrupt";
	default:
		return "Unknown";
	}
}
#endif


/**
 *
 * @brief Kernel fatal error handler
 *
 * This routine is called when a fatal error condition is detected by either
 * hardware or software.
 *
 * The caller is expected to always provide a usable ESF.  In the event that the
 * fatal error does not have a hardware generated ESF, the caller should either
 * create its own or call _Fault instead.
 *
 * @param reason the reason that the handler was called
 * @param pEsf pointer to the exception stack frame
 *
 * @return This function does not return.
 */
FUNC_NORETURN void _NanoFatalErrorHandler(unsigned int reason,
					const NANO_ESF *esf)
{
#ifdef CONFIG_PRINTK
	switch (reason) {
	case _NANO_ERR_CPU_EXCEPTION:
	case _NANO_ERR_SPURIOUS_INT:
		break;

	case _NANO_ERR_ALLOCATION_FAIL:
		printk("**** Kernel Allocation Failure! ****\n");
		break;

	case _NANO_ERR_KERNEL_OOPS:
		printk("***** Kernel OOPS! *****\n");
		break;

	case _NANO_ERR_KERNEL_PANIC:
		printk("***** Kernel Panic! *****\n");
		break;

#ifdef CONFIG_STACK_SENTINEL
	case _NANO_ERR_STACK_CHK_FAIL:
		printk("***** Stack overflow *****\n");
		break;
#endif
	default:
		printk("**** Unknown Fatal Error %u! ****\n", reason);
		break;
	}

	printk("Current thread ID: %p\n"
		"Exception: %s\n"
		"Faulting instruction: 0x%08x\n"
		"   at: 0x%08x     v0: 0x%08x     v1: 0x%08x     a0: 0x%08x\n"
		"   a1: 0x%08x     a2: 0x%08x     a3: 0x%08x     t0: 0x%08x\n"
		"   t1: 0x%08x     t2: 0x%08x     t3: 0x%08x     t4: 0x%08x\n"
		"   t5: 0x%08x     t6: 0x%08x     t7: 0x%08x     t8: 0x%08x\n"
		"   t9: 0x%08x     ra: 0x%08x     hi: 0x%08x     lo: 0x%08x\n"
		"  epc: 0x%08x  cause: 0x%08x     sr: 0x%08x    psr: 0x%08x\n",
		k_current_get(), cause_str(esf->vec),
		esf->epc + (esf->cause & CAUSE_BD_MASK ? 4 : 0),
		esf->at, esf->v0, esf->v1, esf->a0, esf->a1, esf->a2,
		esf->a3, esf->t0, esf->t1, esf->t2, esf->t3, esf->t4,
		esf->t5, esf->t6, esf->t7, esf->t8, esf->t9, esf->ra,
		esf->hi, esf->lo, esf->epc, esf->cause, esf->sr, esf->psr);
#endif

	_SysFatalErrorHandler(reason, esf);
}

FUNC_NORETURN void _Fault(const NANO_ESF *esf)
{
	_NanoFatalErrorHandler(_NANO_ERR_CPU_EXCEPTION, esf);
}


/**
 *
 * @brief Fatal error handler
 *
 * This routine implements the corrective action to be taken when the system
 * detects a fatal error.
 *
 * This sample implementation attempts to abort the current thread and allow
 * the system to continue executing, which may permit the system to continue
 * functioning with degraded capabilities.
 *
 * System designers may wish to enhance or substitute this sample
 * implementation to take other actions, such as logging error (or debug)
 * information to a persistent repository and/or rebooting the system.
 *
 * @param reason the fatal error reason
 * @param pEsf pointer to exception stack frame
 *
 * @return N/A
 */
FUNC_NORETURN __weak void _SysFatalErrorHandler(unsigned int reason,
						const NANO_ESF *pEsf)
{
	ARG_UNUSED(pEsf);

#if !defined(CONFIG_SIMPLE_FATAL_ERROR_HANDLER)
#ifdef CONFIG_STACK_SENTINEL
	if (reason == _NANO_ERR_STACK_CHK_FAIL) {
		goto hang_system;
	}
#endif
	if (reason == _NANO_ERR_KERNEL_PANIC) {
		goto hang_system;
	}
	if (k_is_in_isr() || _is_thread_essential()) {
		printk("Fatal fault in %s! Spinning...\n",
			k_is_in_isr() ? "ISR" : "essential thread");
		goto hang_system;
	}
	printk("Fatal fault in thread %p! Aborting.\n", _current);
	k_thread_abort(_current);

hang_system:
#else
	ARG_UNUSED(reason);
#endif

	irq_lock();
	waiti();

	CODE_UNREACHABLE;
}
