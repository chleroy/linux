// SPDX-License-Identifier: GPL-2.0

#define DISABLE_BRANCH_PROFILING

#include <linux/kasan.h>
#include <linux/printk.h>
#include <linux/memblock.h>
#include <linux/sched/task.h>
#include <asm/pgalloc.h>

DEFINE_STATIC_KEY_FALSE(powerpc_kasan_enabled_key);

static void __init kasan_init_region(struct memblock_region *reg)
{
	void *start = __va(reg->base);
	void *end = __va(reg->base + reg->size);
	unsigned long k_start, k_end, k_cur;

	if (start >= end)
		return;

	k_start = (unsigned long)kasan_mem_to_shadow(start);
	k_end = (unsigned long)kasan_mem_to_shadow(end);

	for (k_cur = k_start; k_cur < k_end; k_cur += PAGE_SIZE) {
		void *va = memblock_alloc(PAGE_SIZE, PAGE_SIZE);

		map_kernel_page(k_cur, __pa(va), PAGE_KERNEL);
	}
	flush_tlb_kernel_range(k_start, k_end);
}

void __init kasan_init(void)
{
	struct memblock_region *reg;

	for_each_memblock(memory, reg)
		kasan_init_region(reg);

	/* map the zero page RO */
	map_kernel_page((unsigned long)kasan_early_shadow_page,
			__pa(kasan_early_shadow_page), PAGE_KERNEL_RO);

	/* Turn on checking */
	static_branch_inc(&powerpc_kasan_enabled_key);

	/* Enable error messages */
	init_task.kasan_depth = 0;
	pr_info("KASAN init done (64-bit Book3E)\n");
}
