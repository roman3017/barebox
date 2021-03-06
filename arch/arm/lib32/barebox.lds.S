/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 */

#include <asm-generic/barebox.lds.h>
#include <asm/secure.h>

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(start)
SECTIONS
{
#ifdef CONFIG_RELOCATABLE
	. = 0x0;
#else
	. = TEXT_BASE;
#endif
	.image_start : { *(.__image_start) }

#ifndef CONFIG_PBL_IMAGE
	PRE_IMAGE
#endif
	. = ALIGN(4);

	._text : { *(._text) }
	.text      :
	{
		_stext = .;
		*(.text_entry*)
		__bare_init_start = .;
		*(.text_bare_init*)
		__bare_init_end = .;
		. = ALIGN(0x20);
		__exceptions_start = .;
		KEEP(*(.text_exceptions*))
		__exceptions_stop = .;
		*(.text*)
	}
	BAREBOX_BARE_INIT_SIZE

	. = ALIGN(4);
	.rodata : {
		*(.rodata*)
		RO_DATA_SECTION
	}

#ifdef CONFIG_ARM_UNWIND
	/*
	 * Stack unwinding tables
	 */
	. = ALIGN(8);
	.ARM.unwind_idx : {
		__start_unwind_idx = .;
		*(.ARM.exidx*)
		__stop_unwind_idx = .;
	}
	.ARM.unwind_tab : {
		__start_unwind_tab = .;
		*(.ARM.extab*)
		__stop_unwind_tab = .;
	}
#endif
	_etext = .;			/* End of text and rodata section */
	_sdata = .;

	. = ALIGN(4);
	.data : { *(.data*) }

	.barebox_imd : { BAREBOX_IMD }

	. = .;

	.rel_dyn_start : { *(.__rel_dyn_start) }
	.rel.dyn : { *(.rel*) }
	.rel_dyn_end : { *(.__rel_dyn_end) }

	.__dynsym_start :  { *(.__dynsym_start) }
	.dynsym : { *(.dynsym) }
	.__dynsym_end : { *(.__dynsym_end) }

	_edata = .;
	.image_end : { *(.__image_end) }

	. = ALIGN(4);
	.__bss_start :  { *(.__bss_start) }
	.bss : { *(.bss*) }
	.__bss_stop :  { *(.__bss_stop) }

#ifdef CONFIG_ARM_SECURE_MONITOR
	. = ALIGN(16);
	__secure_stack_start = .;
	. = . + (ARM_SECURE_MAX_CPU << ARM_SECURE_STACK_SHIFT);
	__secure_stack_end = .;
	__secure_end = .;
#endif

	_end = .;
	_barebox_image_size = __bss_start - TEXT_BASE;
}
