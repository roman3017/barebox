/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2020 roman3017 <rbacik@hotmail.com>
 *
 * This file is part of barebox.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __MACH_VEXRISCV_DEBUG_LL__
#define __MACH_VEXRISCV_DEBUG_LL__

/** @file
 *  This File contains declaration for early output support
 */

#define DEBUG_LL_UART_ADDR 0x10010000

#ifndef __ASSEMBLY__
static inline void debug_ll_init(void)
{
}
static inline void PUTC_LL(char ch) {
#ifdef CONFIG_DEBUG_LL
	while (!((__raw_readl((u8 *)DEBUG_LL_UART_ADDR + 4)>>16) & 0xff));
	__raw_writel(ch, (u8 *)DEBUG_LL_UART_ADDR);
#endif
}
#else
.macro	debug_ll_init
.endm
#endif

#endif /* __MACH_VEXRISCV_DEBUG_LL__ */
