// SPDX-License-Identifier: GPL-2.0-only
/*
 *  PowerPC version derived from arch/arm/mm/consistent.c
 *    Copyright (C) 2001 Dan Malek (dmalek@jlc.net)
 *
 *  Copyright (C) 2000 Russell King
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/cacheflush.h>
#include <linux/dma-direct.h>
#include <linux/dma-map-ops.h>

#include <asm/tlbflush.h>
#include <asm/dma.h>

/*
 * make an area consistent.
 */
static void __dma_sync(void *vaddr, size_t size, int direction)
{
	unsigned long start = (unsigned long)vaddr;
	unsigned long end   = start + size;

	switch (direction) {
	case DMA_NONE:
		BUG();
	case DMA_FROM_DEVICE:
		/*
		 * invalidate only when cache-line aligned otherwise there is
		 * the potential for discarding uncommitted data from the cache
		 */
		if ((start | end) & (L1_CACHE_BYTES - 1))
			flush_dcache_range(start, end);
		else
			invalidate_dcache_range(start, end);
		break;
	case DMA_TO_DEVICE:		/* writeback only */
		clean_dcache_range(start, end);
		break;
	case DMA_BIDIRECTIONAL:	/* writeback and invalidate */
		flush_dcache_range(start, end);
		break;
	}
}

/*
 * __dma_sync_page makes memory consistent. identical to __dma_sync, but
 * takes a struct page instead of a virtual address
 */
static void __dma_sync_page(phys_addr_t paddr, size_t size, int dir)
{
	struct page *page = pfn_to_page(paddr >> PAGE_SHIFT);
	unsigned offset = paddr & ~PAGE_MASK;
	unsigned long start = (unsigned long)page_address(page) + offset;
	__dma_sync((void *)start, size, dir);
}

void arch_sync_dma_for_device(phys_addr_t paddr, size_t size,
		enum dma_data_direction dir)
{
	__dma_sync_page(paddr, size, dir);
}

void arch_sync_dma_for_cpu(phys_addr_t paddr, size_t size,
		enum dma_data_direction dir)
{
	__dma_sync_page(paddr, size, dir);
}

void arch_dma_prep_coherent(struct page *page, size_t size)
{
	unsigned long kaddr = (unsigned long)page_address(page);

	flush_dcache_range(kaddr, kaddr + size);
}
