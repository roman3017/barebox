// SPDX-License-Identifier: GPL-2.0
/*
 *  Uart module for vexriscv serial ports.
 *
 *  Copyright (C) 2020 roman3017 <rbacik@hotmail.com>
 */

#include <common.h>
#include <init.h>
#include <malloc.h>
#include <errno.h>

#define UART_ADDR 0x10010000
#define STATUS_TX 16
#define STATUS_RX 24

struct vexriscv_uart_regs {
	u32 data;
	u32 status;
	u32 div;
	u32 frame;
};

static void serial_vexriscv_putc(struct console_device *cdev, char c)
{
	struct vexriscv_uart_regs *regs;
	regs = (struct vexriscv_uart_regs *)cdev->dev->priv;
	while (0 == ((readl(&regs->status)>>STATUS_TX) & 0xff));
	writel(c, &regs->data);
}

static int serial_vexriscv_tstc(struct console_device *cdev)
{
	struct vexriscv_uart_regs *regs;
	regs = (struct vexriscv_uart_regs *)cdev->dev->priv;
	return ((readl(&regs->status)>>STATUS_RX) & 0xff);
}

static int serial_vexriscv_getc(struct console_device *cdev)
{
	struct vexriscv_uart_regs *regs;
	regs = (struct vexriscv_uart_regs *)cdev->dev->priv;
	while (0 == ((readl(&regs->status)>>STATUS_RX) & 0xff));
	return readl(&regs->data);
}

static int serial_vexriscv_probe(struct device_d *dev)
{
	struct console_device *cdev;
	cdev = xzalloc(sizeof(struct console_device));

	dev->priv = dev_get_mem_region(dev, 0);
	if (IS_ERR(dev->priv))
		return PTR_ERR(dev->priv);

	cdev->dev = dev;
	cdev->tstc = serial_vexriscv_tstc;
	cdev->putc = serial_vexriscv_putc;
	cdev->getc = serial_vexriscv_getc;

	dev_info(dev, "serial_vexriscv_probe %x\n", dev->priv);
	return console_register(cdev);
}

static __maybe_unused struct of_device_id serial_dt_ids[] = {
	{
		.compatible = "spinal,serial",
	},
	{
		/* sentinel */
	}
};

static struct driver_d serial_vexriscv_driver = {
	.name = "serial_vexriscv",
	.probe = serial_vexriscv_probe,
	.of_compatible = DRV_OF_COMPAT(serial_dt_ids),
};
console_platform_driver(serial_vexriscv_driver);
