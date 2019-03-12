/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_KASAN_H
#define __ASM_KASAN_H

#ifdef CONFIG_KASAN
#define _GLOBAL_KASAN(fn)	_GLOBAL(__##fn)
#define _GLOBAL_TOC_KASAN(fn)	_GLOBAL_TOC(__##fn)
#define EXPORT_SYMBOL_KASAN(fn)	EXPORT_SYMBOL(__##fn)
#else
#define _GLOBAL_KASAN(fn)	_GLOBAL(fn)
#define _GLOBAL_TOC_KASAN(fn)	_GLOBAL_TOC(fn)
#define EXPORT_SYMBOL_KASAN(fn)
#endif

#ifndef __ASSEMBLY__

#include <asm/page.h>

#define KASAN_SHADOW_SCALE_SHIFT	3

#define KASAN_SHADOW_START	(KASAN_SHADOW_OFFSET + \
				 (PAGE_OFFSET >> KASAN_SHADOW_SCALE_SHIFT))

#ifdef CONFIG_PPC32
#define KASAN_SHADOW_OFFSET	ASM_CONST(CONFIG_KASAN_SHADOW_OFFSET)

#define KASAN_SHADOW_END	0UL

#define KASAN_SHADOW_SIZE	(KASAN_SHADOW_END - KASAN_SHADOW_START)

#endif /* CONFIG_PPC32 */

#ifdef CONFIG_KASAN
void kasan_early_init(void);
void kasan_mmu_init(void);
void kasan_init(void);
#else
static inline void kasan_init(void) { }
static inline void kasan_mmu_init(void) { }
#endif

#ifdef CONFIG_PPC_BOOK3E_64
#include <asm/pgtable.h>
#include <linux/jump_label.h>

/*
 * We don't put this in Kconfig as we only support KASAN_MINIMAL, and
 * that will be disabled if the symbol is available in Kconfig
 */
#define KASAN_SHADOW_OFFSET	ASM_CONST(0x6800040000000000)

#define KASAN_SHADOW_SIZE	(KERN_VIRT_SIZE >> KASAN_SHADOW_SCALE_SHIFT)

extern struct static_key_false powerpc_kasan_enabled_key;
extern unsigned char kasan_early_shadow_page[];

static inline bool kasan_arch_is_ready_book3e(void)
{
	if (static_branch_likely(&powerpc_kasan_enabled_key))
		return true;
	return false;
}
#define kasan_arch_is_ready kasan_arch_is_ready_book3e

static inline void *kasan_mem_to_shadow_book3e(const void *ptr)
{
	unsigned long addr = (unsigned long)ptr;

	if (addr >= KERN_VIRT_START && addr < KERN_VIRT_START + KERN_VIRT_SIZE)
		return kasan_early_shadow_page;

	return (void *)(addr >> KASAN_SHADOW_SCALE_SHIFT) + KASAN_SHADOW_OFFSET;
}
#define kasan_mem_to_shadow kasan_mem_to_shadow_book3e

static inline void *kasan_shadow_to_mem_book3e(const void *shadow_addr)
{
	/*
	 * We map the entire non-linear virtual mapping onto the zero page so if
	 * we are asked to map the zero page back just pick the beginning of that
	 * area.
	 */
	if (shadow_addr >= (void *)kasan_early_shadow_page &&
	    shadow_addr < (void *)(kasan_early_shadow_page + PAGE_SIZE))
		return (void *)KERN_VIRT_START;

	return (void *)(((unsigned long)shadow_addr - KASAN_SHADOW_OFFSET) <<
			KASAN_SHADOW_SCALE_SHIFT);
}
#define kasan_shadow_to_mem kasan_shadow_to_mem_book3e

static inline bool kasan_addr_has_shadow_book3e(const void *ptr)
{
	unsigned long addr = (unsigned long)ptr;

	/*
	 * We want to specifically assert that the addresses in the 0x8000...
	 * region have a shadow, otherwise they are considered by the kasan
	 * core to be wild pointers
	 */
	if (addr >= KERN_VIRT_START && addr < (KERN_VIRT_START + KERN_VIRT_SIZE))
		return true;

	return (ptr >= kasan_shadow_to_mem((void *)KASAN_SHADOW_START));
}
#define kasan_addr_has_shadow kasan_addr_has_shadow_book3e

#endif /* CONFIG_PPC_BOOK3E_64 */

#endif /* __ASSEMBLY */
#endif
