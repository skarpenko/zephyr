/*
 * Copyright (c) 2019 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys_io.h>
#include <device.h>
#include <init.h>


/* UP-UART registers */
#define USOC_UART_IOBASE	(CONFIG_ULTISOC_CONSOLE_BASE_ADDRESS)	/* UART I/O base */
#define USOC_UART_CTRL		(USOC_UART_IOBASE + 0x00)		/* Control register */
#define USOC_UART_DIVD		(USOC_UART_IOBASE + 0x04)		/* Divider register */
#define USOC_UART_DATA		(USOC_UART_IOBASE + 0x08)		/* Data register */
#define USOC_UART_FIFO		(USOC_UART_IOBASE + 0x0C)		/* FIFO state register */
/***/
#define USOC_UART_CTRL_TX_IM		(1<<0)			/* TX interrupt mask */
#define USOC_UART_CTRL_TX_FF		(1<<1)			/* TX FIFO full */
#define USOC_UART_CTRL_TX_FE		(1<<2)			/* TX FIFO empty */
#define USOC_UART_CTRL_RX_IM		(1<<3)			/* RX interrupt mask */
#define USOC_UART_CTRL_RX_FF		(1<<4)			/* RX FIFO full */
#define USOC_UART_CTRL_RX_FE		(1<<5)			/* RX FIFO empty */
#define USOC_UART_FIFO_TX_COUNT(a)	((a) & 0xFFFF)		/* TX FIFO bytes count */
#define USOC_UART_FIFO_RX_COUNT(a)	(((a) >> 16) & 0xFFFF)	/* RX FIFO bytes count */

/* UP-UART baud rate */
#define BAUDRATE		(CONFIG_ULTISOC_CONSOLE_BAUD_RATE)


#if defined(CONFIG_PRINTK) || defined(CONFIG_STDOUT_CONSOLE)
/**
 * @brief Output one character to serial console
 * @param c Character to output
 * @return The character passed as input.
 */
static int console_out(int c)
{
	while(sys_read32(USOC_UART_CTRL) & USOC_UART_CTRL_TX_FF)
		;
	sys_write32(c, USOC_UART_DATA);
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
 * @brief Install printk/stdout hook for UltiSoC console output
 * @return N/A
 */

void ultisoc_console_hook_install(void)
{
	__stdout_hook_install(console_out);
	__printk_hook_install(console_out);
}

/**
 *
 * @brief Initialize the console/debug port
 * @return 0 if successful, otherwise failed.
 */
static int ultisoc_console_init(struct device *arg)
{
	u32_t sys_freq = CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;	/* UART frequency */
	u32_t divq, divr;

	/* Set UART control reg */
	sys_write32((USOC_UART_CTRL_TX_IM | USOC_UART_CTRL_RX_IM), USOC_UART_CTRL);

	/* Calculate divider for configured baud rate */
	divq = sys_freq / (16 * BAUDRATE);
	divr = sys_freq % (16 * BAUDRATE);
	if(divr > (16 * BAUDRATE / 2)) ++divq;

	/* Set divider */
	sys_write32(divq, USOC_UART_DIVD);

	/* Set hooks */
	ultisoc_console_hook_install();

	return 0;
}

SYS_INIT(ultisoc_console_init,
#if defined(CONFIG_EARLY_CONSOLE)
			PRE_KERNEL_1,
#else
			POST_KERNEL,
#endif
			CONFIG_ULTISOC_CONSOLE_INIT_PRIORITY);
