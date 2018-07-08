/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Abstraction layer for Ultiparc interrupt controller(s)
 */

#ifndef IRQ_CONTROLLER_H
#define IRQ_CONTROLLER_H


#ifndef _ASMLANGUAGE
#include <zephyr/types.h>


#if defined(CONFIG_ULTIPARC_ICTL)

/**
 * @brief Return the vector of the currently in-service ISR
 *
 * This function should be called in interrupt context.
 * It is not expected for this function to reveal the identity of
 * vectors triggered by a CPU exception or 'int' instruction.
 *
 * @return the vector of the interrupt that is currently being processed, or
 * -1 if this can't be determined
 */
extern int irq_controller_isr_vector_get(void);


/**
 * @brief Acknowledge IRQ
 *
 * This function should be called in interrupt context.
 */
extern void irq_controller_ack(unsigned int irq);

#else

static inline int irq_controller_isr_vector_get(void) { return -1; }
static inline void irq_controller_ack(unsigned int irq) {}

#endif

#endif /* _ASMLANGUAGE */

#endif /* IRQ_CONTROLLER_H */
