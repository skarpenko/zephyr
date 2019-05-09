/*
 * Copyright (c) 2018-2019 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SOC_ULTISOC_H__
#define __SOC_ULTISOC_H__


/*
 * This header provides SoC specific macros for UltiSoC SoC.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#if !defined(_ASMLANGUAGE)

extern void _arch_irq_enable(unsigned int irq);
extern void _arch_irq_disable(unsigned int irq);

extern u32_t _timer_cycle_get_32(void);
#define _arch_k_cycle_get_32()	_timer_cycle_get_32()


#endif /* _ASMLANGUAGE */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SOC_ULTISOC_H__ */
