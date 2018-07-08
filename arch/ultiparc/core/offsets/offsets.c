/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Ultiparc kernel structure member offset definition file
 *
 * This module is responsible for the generation of the absolute symbols whose
 * value represents the member offsets for various Ultiparc kernel
 * structures.
 */

#include <gen_offset.h>
#include <kernel_structs.h>
#include <kernel_offsets.h>


/* thread_arch_t member offsets */
GEN_OFFSET_SYM(_thread_arch_t, irq_lock_state);
GEN_OFFSET_SYM(_thread_arch_t, swap_return_value);


/* struct coop member offsets */
GEN_OFFSET_SYM(_callee_saved_t, sp);
GEN_OFFSET_SYM(_callee_saved_t, fp);
GEN_OFFSET_SYM(_callee_saved_t, gp);
GEN_OFFSET_SYM(_callee_saved_t, ra);
GEN_OFFSET_SYM(_callee_saved_t, s0);
GEN_OFFSET_SYM(_callee_saved_t, s1);
GEN_OFFSET_SYM(_callee_saved_t, s2);
GEN_OFFSET_SYM(_callee_saved_t, s3);
GEN_OFFSET_SYM(_callee_saved_t, s4);
GEN_OFFSET_SYM(_callee_saved_t, s5);
GEN_OFFSET_SYM(_callee_saved_t, s6);
GEN_OFFSET_SYM(_callee_saved_t, s7);


/* esf member offsets */
GEN_OFFSET_SYM(NANO_ESF, vec);
GEN_OFFSET_SYM(NANO_ESF, psr);
GEN_OFFSET_SYM(NANO_ESF, sr);
GEN_OFFSET_SYM(NANO_ESF, cause);
GEN_OFFSET_SYM(NANO_ESF, epc);
GEN_OFFSET_SYM(NANO_ESF, lo);
GEN_OFFSET_SYM(NANO_ESF, hi);
GEN_OFFSET_SYM(NANO_ESF, ra);
GEN_OFFSET_SYM(NANO_ESF, t9);
GEN_OFFSET_SYM(NANO_ESF, t8);
GEN_OFFSET_SYM(NANO_ESF, t7);
GEN_OFFSET_SYM(NANO_ESF, t6);
GEN_OFFSET_SYM(NANO_ESF, t5);
GEN_OFFSET_SYM(NANO_ESF, t4);
GEN_OFFSET_SYM(NANO_ESF, t3);
GEN_OFFSET_SYM(NANO_ESF, t2);
GEN_OFFSET_SYM(NANO_ESF, t1);
GEN_OFFSET_SYM(NANO_ESF, t0);
GEN_OFFSET_SYM(NANO_ESF, a3);
GEN_OFFSET_SYM(NANO_ESF, a2);
GEN_OFFSET_SYM(NANO_ESF, a1);
GEN_OFFSET_SYM(NANO_ESF, a0);
GEN_OFFSET_SYM(NANO_ESF, v1);
GEN_OFFSET_SYM(NANO_ESF, v0);
GEN_OFFSET_SYM(NANO_ESF, at);


/* size of the entire NANO_ESF structure */
GEN_ABSOLUTE_SYM(__NANO_ESF_SIZEOF, sizeof(NANO_ESF));


GEN_ABS_SYM_END
