// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * This file contains the routines for initializing kernel userspace protection
 */

#include <linux/export.h>
#include <linux/init.h>
#include <linux/jump_label.h>
#include <linux/printk.h>
#include <linux/smp.h>

#include <asm/kup.h>
#include <asm/smp.h>

#ifdef CONFIG_PPC_KUAP
void setup_kuap(bool disabled)
{
	WARN_ON(disabled);

	pr_info("Activating Kernel Userspace Access Protection\n");

	__prevent_user_access(KUAP_READ_WRITE);
}
#endif
