/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Ultiparc C-domain spurious interrupt management code
 */


#include <kernel.h>
#include <kernel_structs.h>
#include <arch/cpu.h>
#include <misc/printk.h>


void _irq_spurious(void *unused)
{
	ARG_UNUSED(unused);
	printk("Spurious interrupt detected!\n");
	_NanoFatalErrorHandler(_NANO_ERR_SPURIOUS_INT, &_default_esf);
}
