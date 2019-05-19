/*
 * Copyright (c) 2019 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief UART driver for the UltiSoC UART module
 */

#include <kernel.h>
#include <arch/cpu.h>
#include <uart.h>


/* UART registers */
#define UART_CTRL(iobase)	((iobase) + 0x00)	/* Control register */
#define UART_DIVD(iobase)	((iobase) + 0x04)	/* Divider register */
#define UART_DATA(iobase)	((iobase) + 0x08)	/* Data register */
#define UART_FIFO(iobase)	((iobase) + 0x0C)	/* FIFO state register */
/* UART flags */
#define UART_CTRL_TX_IM		(1<<0)			/* TX interrupt mask */
#define UART_CTRL_TX_FF		(1<<1)			/* TX FIFO full */
#define UART_CTRL_TX_FE		(1<<2)			/* TX FIFO empty */
#define UART_CTRL_RX_IM		(1<<3)			/* RX interrupt mask */
#define UART_CTRL_RX_FF		(1<<4)			/* RX FIFO full */
#define UART_CTRL_RX_FE		(1<<5)			/* RX FIFO empty */
#define UART_FIFO_TX_COUNT(a)	((a) & 0xFFFF)		/* TX FIFO bytes count */
#define UART_FIFO_RX_COUNT(a)	(((a) >> 16) & 0xFFFF)	/* RX FIFO bytes count */


#ifdef CONFIG_UART_INTERRUPT_DRIVEN
typedef void (*irq_cfg_func_t)(void);
#endif


struct uart_ultisoc_device_config {
	u32_t		port;		/* Port number */
	mem_addr_t	base;		/* I/O base address */
	u32_t		irq;		/* IRQ number */
	u32_t		clk_freq;	/* UART clock freq. */
	u32_t		baud_rate;	/* Baud rate */
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	irq_cfg_func_t	cfg_func;
#endif
};


struct uart_ultisoc_data {
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	uart_irq_callback_user_data_t	callback;
	void				*cb_data;
#endif
};


#define DEV_CFG(dev)						\
	((const struct uart_ultisoc_device_config * const)	\
	 (dev)->config->config_info)

#define DEV_BASE(dev)						\
	((mem_addr_t)(DEV_CFG(dev))->base)

#define DEV_DATA(dev)						\
	((struct uart_ultisoc_data * const)(dev)->driver_data)


/**
 * @brief Output a character in polled mode.
 *
 * Writes data to tx register if transmitter is not full.
 *
 * @param dev UART device struct
 * @param c Character to send
 */
static void uart_ultisoc_poll_out(struct device *dev, unsigned char c)
{
	mem_addr_t uart_base = DEV_BASE(dev);

	/* Wait while TX FIFO is full */
	while(sys_read32(UART_CTRL(uart_base)) & UART_CTRL_TX_FF)
		;

	sys_write32((u32_t)c, UART_DATA(uart_base));
}


/**
 * @brief Poll the device for input.
 *
 * @param dev UART device struct
 * @param c Pointer to character
 *
 * @return 0 if a character arrived, -1 if the input buffer if empty.
 */
static int uart_ultisoc_poll_in(struct device *dev, unsigned char *c)
{
	mem_addr_t uart_base = DEV_BASE(dev);

	/* Check if FIFO is empty */
	if(sys_read32(UART_CTRL(uart_base)) & UART_CTRL_RX_FE)
		return -1;

	*c = (unsigned char)(sys_read32(UART_DATA(uart_base)) & 0xFF);

	return 0;
}


#ifdef CONFIG_UART_INTERRUPT_DRIVEN

/**
 * @brief Fill FIFO with data
 *
 * @param dev UART device struct
 * @param tx_data Data to transmit
 * @param size Number of bytes to send
 *
 * @return Number of bytes sent
 */
static int uart_ultisoc_fifo_fill(struct device *dev, const u8_t *tx_data, int size)
{
	mem_addr_t uart_base = DEV_BASE(dev);
	int i;

	for (i = 0; i < size && !(sys_read32(UART_CTRL(uart_base)) & UART_CTRL_TX_FF); ++i)
		sys_write32((u32_t)tx_data[i], UART_DATA(uart_base));

	return i;
}


/**
 * @brief Read data from FIFO
 *
 * @param dev UART device struct
 * @param rxData Data container
 * @param size Container size
 *
 * @return Number of bytes read
 */
static int uart_ultisoc_fifo_read(struct device *dev, u8_t *rx_data, const int size)
{
	mem_addr_t uart_base = DEV_BASE(dev);
	int i;

	for (i = 0; i < size; ++i) {
		if(sys_read32(UART_CTRL(uart_base)) & UART_CTRL_RX_FE)
			break;

		rx_data[i] = (u8_t)(sys_read32(UART_DATA(uart_base)) & 0xFF);
	}

	return i;
}


/**
 * @brief Enable TX interrupt in CTRL register
 *
 * @param dev UART device struct
 *
 * @return N/A
 */
static void uart_ultisoc_irq_tx_enable(struct device *dev)
{
	mem_addr_t uart_base = DEV_BASE(dev);
	u32_t val = sys_read32(UART_CTRL(uart_base));
	sys_write32(val & (~UART_CTRL_TX_IM), UART_CTRL(uart_base));
}


/**
 * @brief Disable TX interrupt in CTRL register
 *
 * @param dev UART device struct
 *
 * @return N/A
 */
static void uart_ultisoc_irq_tx_disable(struct device *dev)
{
	mem_addr_t uart_base = DEV_BASE(dev);
	u32_t val = sys_read32(UART_CTRL(uart_base));
	sys_write32(val | UART_CTRL_TX_IM, UART_CTRL(uart_base));
}


/**
 * @brief Check if Tx IRQ has been raised
 *
 * @param dev UART device struct
 *
 * @return 1 if an IRQ is ready, 0 otherwise
 */
static int uart_ultisoc_irq_tx_ready(struct device *dev)
{
	mem_addr_t uart_base = DEV_BASE(dev);
	u32_t val = sys_read32(UART_CTRL(uart_base));
	return !!(val & UART_CTRL_TX_FE);
}


/**
 * @brief Check if nothing remains to be transmitted
 *
 * @param dev UART device struct
 *
 * @return 1 if nothing remains to be transmitted, 0 otherwise
 */
static int uart_ultisoc_irq_tx_complete(struct device *dev)
{
	return uart_ultisoc_irq_tx_ready(dev);
}


/**
 * @brief Enable RX interrupt in CTRL register
 *
 * @param dev UART device struct
 *
 * @return N/A
 */
static void uart_ultisoc_irq_rx_enable(struct device *dev)
{
	mem_addr_t uart_base = DEV_BASE(dev);
	u32_t val = sys_read32(UART_CTRL(uart_base));
	sys_write32(val & (~UART_CTRL_RX_IM), UART_CTRL(uart_base));
}


/**
 * @brief Disable RX interrupt in CTRL register
 *
 * @param dev UART device struct
 *
 * @return N/A
 */
static void uart_ultisoc_irq_rx_disable(struct device *dev)
{
	mem_addr_t uart_base = DEV_BASE(dev);
	u32_t val = sys_read32(UART_CTRL(uart_base));
	sys_write32(val | UART_CTRL_RX_IM, UART_CTRL(uart_base));
}


/**
 * @brief Check if Rx IRQ has been raised
 *
 * @param dev UART device struct
 *
 * @return 1 if an IRQ is ready, 0 otherwise
 */
static int uart_ultisoc_irq_rx_ready(struct device *dev)
{
	mem_addr_t uart_base = DEV_BASE(dev);
	u32_t val = sys_read32(UART_CTRL(uart_base));
	return !(val & UART_CTRL_RX_FE);
}


/* No error interrupt for this controller */
static void uart_ultisoc_irq_err_enable(struct device *dev)
{
	ARG_UNUSED(dev);
}


static void uart_ultisoc_irq_err_disable(struct device *dev)
{
	ARG_UNUSED(dev);
}


/**
 * @brief Check if any IRQ is pending
 *
 * @param dev UART device struct
 *
 * @return 1 if an IRQ is pending, 0 otherwise
 */
static int uart_ultisoc_irq_is_pending(struct device *dev)
{
	return uart_ultisoc_irq_tx_ready(dev) | uart_ultisoc_irq_rx_ready(dev);
}


static int uart_ultisoc_irq_update(struct device *dev)
{
	return 1;
}


/**
 * @brief Set the callback function pointer for IRQ.
 *
 * @param dev UART device struct
 * @param cb Callback function pointer.
 *
 * @return N/A
 */
static void uart_ultisoc_irq_callback_set(struct device *dev,
	uart_irq_callback_user_data_t cb, void *cb_data)
{
	struct uart_ultisoc_data *data = DEV_DATA(dev);

	data->callback = cb;
	data->cb_data = cb_data;
}


static void uart_ultisoc_irq_handler(void *arg)
{
	struct device *dev = (struct device *)arg;
	struct uart_ultisoc_data *data = DEV_DATA(dev);

	if (data->callback)
		data->callback(data->cb_data);
}

#endif /* CONFIG_UART_INTERRUPT_DRIVEN */


static int uart_ultisoc_init(struct device *dev)
{
	const struct uart_ultisoc_device_config * const cfg = DEV_CFG(dev);
	mem_addr_t uart_base = DEV_BASE(dev);
	u32_t divq, divr;

	/* Set UART control reg */
	sys_write32((UART_CTRL_TX_IM | UART_CTRL_RX_IM), UART_CTRL(uart_base));

	/* Calculate divider for configured baud rate */
	divq = cfg->clk_freq / (16 * cfg->baud_rate);
	divr = cfg->clk_freq % (16 * cfg->baud_rate);
	if(divr > (16 * cfg->baud_rate / 2)) ++divq;

	/* Set divider */
	sys_write32(divq, UART_DIVD(uart_base));

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	/* Setup IRQ handler */
	cfg->cfg_func();
#endif

	return 0;
}


/* UART driver API */
static const struct uart_driver_api uart_ultisoc_driver_api = {
	.poll_in		= uart_ultisoc_poll_in,
	.poll_out		= uart_ultisoc_poll_out,
	.err_check		= NULL,
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	.fifo_fill		= uart_ultisoc_fifo_fill,
	.fifo_read		= uart_ultisoc_fifo_read,
	.irq_tx_enable		= uart_ultisoc_irq_tx_enable,
	.irq_tx_disable		= uart_ultisoc_irq_tx_disable,
	.irq_tx_ready		= uart_ultisoc_irq_tx_ready,
	.irq_tx_complete	= uart_ultisoc_irq_tx_complete,
	.irq_rx_enable		= uart_ultisoc_irq_rx_enable,
	.irq_rx_disable		= uart_ultisoc_irq_rx_disable,
	.irq_rx_ready		= uart_ultisoc_irq_rx_ready,
	.irq_err_enable		= uart_ultisoc_irq_err_enable,
	.irq_err_disable	= uart_ultisoc_irq_err_disable,
	.irq_is_pending		= uart_ultisoc_irq_is_pending,
	.irq_update		= uart_ultisoc_irq_update,
	.irq_callback_set	= uart_ultisoc_irq_callback_set,
#endif
};


/* Configure port 0 */
#ifdef CONFIG_UART_ULTISOC_PORT_0

static struct uart_ultisoc_data uart_ultisoc_data_0;

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static void uart_ultisoc_irq_cfg_func_0(void);
#endif

static const struct uart_ultisoc_device_config uart_ultisoc_dev_cfg_0 = {
	.port		= 0,
	.base		= CONFIG_UART_ULTISOC_PORT_0_BASE_ADDRESS,
	.irq		= CONFIG_UART_ULTISOC_PORT_0_IRQ,
	.clk_freq	= CONFIG_UART_ULTISOC_PORT_0_CLK_FREQ,
	.baud_rate	= CONFIG_UART_ULTISOC_PORT_0_BAUD_RATE,
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	.cfg_func	= uart_ultisoc_irq_cfg_func_0,
#endif
};

DEVICE_AND_API_INIT(uart_ultisoc_ultisoc_0, "UART_0",
			uart_ultisoc_init,
			&uart_ultisoc_data_0, &uart_ultisoc_dev_cfg_0,
			PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
			(void *)&uart_ultisoc_driver_api);

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static void uart_ultisoc_irq_cfg_func_0(void)
{
	IRQ_CONNECT(CONFIG_UART_ULTISOC_PORT_0_IRQ,
			CONFIG_UART_ULTISOC_PORT_0_IRQ_PRIORITY,
			uart_ultisoc_irq_handler, DEVICE_GET(uart_ultisoc_ultisoc_0),
			0);

	irq_enable(CONFIG_UART_ULTISOC_PORT_0_IRQ);
}
#endif

#endif /* CONFIG_UART_ULTISOC_PORT_0 */
