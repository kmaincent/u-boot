/*
 * am335x_evm.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - https://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CONFIG_AM335X_EVM_H
#define __CONFIG_AM335X_EVM_H

#include <configs/ti_am335x_common.h>
#include <linux/sizes.h>

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

/* NS16550 Configuration */
#define CFG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */
#define CFG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CFG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CFG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CFG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CFG_SYS_NS16550_COM6		0x481aa000	/* UART5 */

#ifdef CONFIG_MTD_RAW_NAND
/* NAND: device related configs */
/* NAND: driver related configs */
#define CFG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

#define CFG_SYS_NAND_ECCSIZE		512
#define CFG_SYS_NAND_ECCBYTES	14
#endif /* !CONFIG_MTD_RAW_NAND */

/*
 * NOR Size = 16 MiB
 * Number of Sectors/Blocks = 128
 * Sector Size = 128 KiB
 * Word length = 16 bits
 * Default layout:
 * 0x000000 - 0x07FFFF : U-Boot (512 KiB)
 * 0x080000 - 0x09FFFF : First copy of U-Boot Environment (128 KiB)
 * 0x0A0000 - 0x0BFFFF : Second copy of U-Boot Environment (128 KiB)
 * 0x0C0000 - 0x4BFFFF : Linux Kernel (4 MiB)
 * 0x4C0000 - 0xFFFFFF : Userland (11 MiB + 256 KiB)
 */
#if defined(CONFIG_NOR)
#define CFG_SYS_FLASH_BASE		(0x08000000)
#define CFG_SYS_FLASH_SIZE		0x01000000
#endif  /* NOR support */

#endif	/* ! __CONFIG_AM335X_EVM_H */
