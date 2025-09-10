// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/cacheflush.h>
#include <linux/kprobes.h>

/**
 * flush_coherent_icache() - if a CPU has a coherent icache, flush it
 * Return true if the cache was flushed, false otherwise
 */
static inline bool flush_coherent_icache(void)
{
	/*
	 * For a snooping icache, we still need a dummy icbi to purge all the
	 * prefetched instructions from the ifetch buffers. We also need a sync
	 * before the icbi to order the actual stores to memory that might
	 * have modified instructions with the icbi.
	 */
	if (cpu_has_feature(CPU_FTR_COHERENT_ICACHE)) {
		mb(); /* sync */
		icbi((void *)PAGE_OFFSET);
		mb(); /* sync */
		isync();
		return true;
	}

	return false;
}

/**
 * invalidate_icache_range() - Flush the icache by issuing icbi across an address range
 * @start: the start address
 * @stop: the stop address (exclusive)
 */
static void invalidate_icache_range(unsigned long start, unsigned long stop)
{
	unsigned long shift = l1_icache_shift();
	unsigned long bytes = l1_icache_bytes();
	char *addr = (char *)(start & ~(bytes - 1));
	unsigned long size = stop - (unsigned long)addr + (bytes - 1);
	unsigned long i;

	for (i = 0; i < size >> shift; i++, addr += bytes)
		icbi(addr);

	mb(); /* sync */
	isync();
}

/**
 * flush_icache_range: Write any modified data cache blocks out to memory
 * and invalidate the corresponding blocks in the instruction cache
 *
 * Generic code will call this after writing memory, before executing from it.
 *
 * @start: the start address
 * @stop: the stop address (exclusive)
 */
void flush_icache_range(unsigned long start, unsigned long stop)
{
	if (flush_coherent_icache())
		return;

	clean_dcache_range(start, stop);

	if (IS_ENABLED(CONFIG_44x)) {
		/*
		 * Flash invalidate on 44x because we are passed kmapped
		 * addresses and this doesn't work for userspace pages due to
		 * the virtually tagged icache.
		 */
		iccci((void *)start);
		mb(); /* sync */
		isync();
	} else
		invalidate_icache_range(start, stop);
}
EXPORT_SYMBOL(flush_icache_range);

/**
 * __flush_dcache_icache(): Flush a particular page from the data cache to RAM.
 * Note: this is necessary because the instruction cache does *not*
 * snoop from the data cache.
 *
 * @p: the address of the page to flush
 */
static void __flush_dcache_icache(void *p)
{
	unsigned long addr = (unsigned long)p & PAGE_MASK;

	clean_dcache_range(addr, addr + PAGE_SIZE);

	/*
	 * We don't flush the icache on 44x. Those have a virtual icache and we
	 * don't have access to the virtual address here (it's not the page
	 * vaddr but where it's mapped in user space). The flushing of the
	 * icache on these is handled elsewhere, when a change in the address
	 * space occurs, before returning to user space.
	 */

	if (mmu_has_feature(MMU_FTR_TYPE_44x))
		return;

	invalidate_icache_range(addr, addr + PAGE_SIZE);
}

void flush_dcache_icache_folio(struct folio *folio)
{
	unsigned int i, nr = folio_nr_pages(folio);
	void *addr = folio_address(folio);

	if (flush_coherent_icache())
		return;

	for (i = 0; i < nr; i++)
		__flush_dcache_icache(addr + i * PAGE_SIZE);
}
EXPORT_SYMBOL(flush_dcache_icache_folio);

void clear_user_page(void *page, unsigned long vaddr, struct page *pg)
{
	clear_page(page);

	/*
	 * We shouldn't have to do this, but some versions of glibc
	 * require it (ld.so assumes zero filled pages are icache clean)
	 * - Anton
	 */
	flush_dcache_page(pg);
}
EXPORT_SYMBOL(clear_user_page);

void copy_user_page(void *vto, void *vfrom, unsigned long vaddr,
		    struct page *pg)
{
	copy_page(vto, vfrom);

	/*
	 * We should be able to use the following optimisation, however
	 * there are two problems.
	 * Firstly a bug in some versions of binutils meant PLT sections
	 * were not marked executable.
	 * Secondly the first word in the GOT section is blrl, used
	 * to establish the GOT address. Until recently the GOT was
	 * not marked executable.
	 * - Anton
	 */
#if 0
	if (!vma->vm_file && ((vma->vm_flags & VM_EXEC) == 0))
		return;
#endif

	flush_dcache_page(pg);
}

void flush_icache_user_page(struct vm_area_struct *vma, struct page *page,
			     unsigned long addr, int len)
{
	void *maddr;

	maddr = page_address(page) + (addr & ~PAGE_MASK);
	flush_icache_range((unsigned long)maddr, (unsigned long)maddr + len);
}
