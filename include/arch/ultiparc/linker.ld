/*
 * Copyright (c) 2018-2019 Stepan Karpenko <stepan.karpenko@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Linker script
 */

/**
 * @file
 * @brief Linker command/script file
 *
 * Linker script for the Ultiparc platform
 */

#define _LINKER
#define _ASMLANGUAGE

#include <autoconf.h>
#include <soc.h>
#include <linker/sections.h>
#include <linker/linker-defs.h>
#include <linker/linker-tool.h>


#define RAMABLE_REGION	RAM
#define ROMABLE_REGION	RAM


ENTRY(__reset)


MEMORY
{
	RAM(rwx): ORIGIN = ULTIPARC_RAM_BASE, LENGTH = ULTIPARC_RAM_SIZE
	/* Used by and documented in include/linker/intlist.ld */
	IDT_LIST(wx) : ORIGIN = 0xFFFFF7FF, LENGTH = 2K
}


SECTIONS
{

/* #include <linker/rel-sections.ld> */

	GROUP_START(ROMABLE_REGION)
	_image_rom_start = ULTIPARC_RAM_BASE;

	SECTION_PROLOGUE(reset,,) {
		KEEP(*(.reset.*))
	} GROUP_LINK_IN(ROMABLE_REGION)


	SECTION_PROLOGUE(_TEXT_SECTION_NAME,,) {
		_image_text_start = .;
		*(.text)
		*(".text.*")
		*(.gnu.linkonce.t.*)
		KEEP(*(.openocd_dbg))
		KEEP(*(".openocd_dbg.*"))
	} GROUP_LINK_IN(ROMABLE_REGION)

	_image_text_end = .;

#include <linker/common-rom.ld>

	SECTION_PROLOGUE(_RODATA_SECTION_NAME,,) {
		*(.rodata)
		*(".rodata.*")
		*(.gnu.linkonce.r.*)

#ifdef CONFIG_SOC_RODATA_LD
#include <soc-rodata.ld>
#endif

#ifdef CONFIG_CUSTOM_RODATA_LD
/* Located in project source directory */
#include <custom-rodata.ld>
#endif

	} GROUP_LINK_IN(ROMABLE_REGION)

	_image_rom_end = .;

	__data_rom_start = ALIGN(4);
	GROUP_END(ROMABLE_REGION)

	_image_ram_start = .;

#include <linker/common-ram.ld>

	SECTION_DATA_PROLOGUE(_DATA_SECTION_NAME,,) {
#ifdef CONFIG_GEN_SW_ISR_TABLE
	KEEP(*(SW_ISR_TABLE))
#endif
		*(.data)
		*(.data.*)

#ifdef CONFIG_SOC_RWDATA_LD
#include <soc-rwdata.ld>
#endif

#ifdef CONFIG_CUSTOM_RWDATA_LD
/* Located in project source directory */
#include <custom-rwdata.ld>
#endif

	/* The MIPS-I architecture only has 16-bit signed immediate offsets in
	 * the instructions, so accessing a general address requires typically
	 * three instructions - basically, two for the two halves of the 32-bit
	 * address, and one to merge them - but if we can put the most commonly
	 * accessed globals in a special 64K span of memory addressed by the GP
	 * register, then we can access those values in a single instruction,
	 * saving both codespace and runtime.
	 *
	 * Since these immediate offsets are signed, place gp 0x8000 past the
	 * beginning of .sdata so that we can use both positive and negative
	 * offsets.
	 */
		HIDDEN(_gp = ALIGN(16) + 0x7ff0);
		PROVIDE(gp = _gp);

		KEEP(*(.sdata .sdata.* .gnu.linkonce.s.*))
		KEEP(*(.sdata2 .sdata2.* .gnu.linkonce.s2.*))
	} GROUP_DATA_LINK_IN(RAMABLE_REGION, RAMABLE_REGION)

	__data_ram_end = .;

	SECTION_DATA_PROLOGUE(_BSS_SECTION_NAME,(NOLOAD),) {
		/*
		 * For performance, BSS section is assumed to be 4 byte aligned and
		 * a multiple of 4 bytes
		 */
		. = ALIGN(4);
		__bss_start = .;
		KEEP(*(.sbss))
		KEEP(*(.sbss.*))
		*(.bss)
		*(.bss.*)
		COMMON_SYMBOLS
		/*
		 * As memory is cleared in words only, it is simpler to ensure the BSS
		 * section ends on a 4 byte boundary. This wastes a maximum of 3 bytes.
		 */
		__bss_end = ALIGN(4);
	} GROUP_DATA_LINK_IN(RAMABLE_REGION, RAMABLE_REGION)

	SECTION_PROLOGUE(_NOINIT_SECTION_NAME,(NOLOAD),) {
		/*
		 * This section is used for non-initialized objects that
		 * will not be cleared during the boot process.
		 */
		*(.noinit)
		*(.noinit.*)

#ifdef CONFIG_SOC_NOINIT_LD
#include <soc-noinit.ld>
#endif

	} GROUP_LINK_IN(RAMABLE_REGION)

	/* Define linker symbols */
	_image_ram_end = .;
	_end = .; /* end of image */

	GROUP_END(RAMABLE_REGION)

#ifdef CONFIG_CUSTOM_SECTIONS_LD
/* Located in project source directory */
#include <custom-sections.ld>
#endif

#ifdef CONFIG_GEN_ISR_TABLES
#include <linker/intlist.ld>
#endif

#include <linker/debug-sections.ld>

}
