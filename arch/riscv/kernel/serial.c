/*
 *  Registration of RISC-V UART platform device.
 *
 *  Copyright (C) 2015  Darius Rad <darius@bluespec.com>
 *
 *  Based on Cobalt UART platform device, which is:
 *
 *  Copyright (C) 2007  Yoichi Yuasa <yuasa@linux-mips.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>

#include <asm/irq.h>

static struct resource riscv_uart_resource[] __initdata = {
	{
		.start	= 0xc0000000,
		.end	= 0xc0000020,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= IRQ_EXTERNAL,
		.end	= IRQ_EXTERNAL,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct plat_serial8250_port riscv_serial8250_port[] = {
	{
		.irq		= IRQ_EXTERNAL,
		.uartclk	= 10000000,
		.iotype		= UPIO_MEM,
		.flags		= UPF_IOREMAP | UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.mapbase	= 0xc0000000,
		.regshift       = 2,
	},
	{},
};

static __init int riscv_uart_add(void)
{
	struct platform_device *pdev;
	int retval;

	pdev = platform_device_alloc("serial8250", -1);
	if (!pdev)
		return -ENOMEM;

	pdev->id = PLAT8250_DEV_PLATFORM;
	pdev->dev.platform_data = riscv_serial8250_port;

	retval = platform_device_add_resources(pdev, riscv_uart_resource, ARRAY_SIZE(riscv_uart_resource));
	if (retval)
		goto err_free_device;

	retval = platform_device_add(pdev);
	if (retval)
		goto err_free_device;

	return 0;

err_free_device:
	platform_device_put(pdev);

	return retval;
}
device_initcall(riscv_uart_add);
