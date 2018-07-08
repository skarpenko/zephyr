/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file SoC configuration macros for the pulpino core
 */

#ifndef __ULTIPARC_SOC_H_
#define __ULTIPARC_SOC_H_


/* IRQ numbers */
#define ULTIPARC_RTL_TIMER_IRQ		0	/* Timer interrupt line */


/* lib-c hooks required RAM defined variables */
#define ULTIPARC_RAM_BASE	(CONFIG_RAM_BASE)
#define ULTIPARC_RAM_SIZE	(CONFIG_RAM_SIZE)


#endif /* __ULTIPARC_SOC_H_ */
