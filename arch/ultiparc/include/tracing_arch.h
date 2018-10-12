/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Kernel event logger support for Ultiparc
 */

#ifndef ZEPHYR_ARCH_ULTIPARC_INCLUDE_TRACING_ARCH_H_
#define ZEPHYR_ARCH_ULTIPARC_INCLUDE_TRACING_ARCH_H_

#include <arch/ultiparc/irq_controller.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the identification of the current interrupt.
 *
 * This routine obtain the key of the interrupt that is currently processed
 * if it is called from a ISR context.
 *
 * @return The key of the interrupt that is currently being processed.
 */
static inline int _sys_current_irq_key_get(void)
{
	return irq_controller_isr_vector_get();
}

#ifdef __cplusplus
}
#endif


#endif /* ZEPHYR_ARCH_ULTIPARC_INCLUDE_TRACING_ARCH_H_ */
