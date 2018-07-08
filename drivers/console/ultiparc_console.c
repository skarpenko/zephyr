/*
 * Copyright (c) 2018 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys_io.h>
#include <device.h>
#include <init.h>


#if defined(CONFIG_PRINTK) || defined(CONFIG_STDOUT_CONSOLE)
/**
 * @brief Output one character to SIMULATOR console
 * @param c Character to output
 * @return The character passed as input.
 */
static int console_out(int c)
{
	sys_write32(c, CONFIG_ULTIPARC_CONSOLE_BASE_ADDRESS);
	return c;
}
#endif

#if defined(CONFIG_STDOUT_CONSOLE)
extern void __stdout_hook_install(int (*hook)(int));
#else
#define __stdout_hook_install(x)		\
	do {/* nothing */			\
	} while ((0))
#endif

#if defined(CONFIG_PRINTK)
extern void __printk_hook_install(int (*fn)(int));
#else
#define __printk_hook_install(x)		\
	do {/* nothing */			\
	} while ((0))
#endif


/**
 *
 * @brief Install printk/stdout hook for Ultiparc console output
 * @return N/A
 */

void ultiparc_console_hook_install(void)
{
	__stdout_hook_install(console_out);
	__printk_hook_install(console_out);
}

/**
 *
 * @brief Initialize the console/debug port
 * @return 0 if successful, otherwise failed.
 */
static int ultiparc_console_init(struct device *arg)
{
	ARG_UNUSED(arg);
	ultiparc_console_hook_install();
	return 0;
}

SYS_INIT(ultiparc_console_init,
#if defined(CONFIG_EARLY_CONSOLE)
			PRE_KERNEL_1,
#else
			POST_KERNEL,
#endif
			CONFIG_ULTIPARC_CONSOLE_INIT_PRIORITY);
