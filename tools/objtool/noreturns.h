/* SPDX-License-Identifier: GPL-2.0 */

#include <arch/noreturns.h>

/*
 * This is a (sorted!) list of all known __noreturn functions in the kernel.
 * It's needed for objtool to properly reverse-engineer the control flow graph.
 *
 * Yes, this is unfortunate.  A better solution is in the works.
 */
NORETURN(__invalid_creds)
NORETURN(__kunit_abort)
NORETURN(__module_put_and_kthread_exit)
NORETURN(__reiserfs_panic)
NORETURN(__stack_chk_fail)
NORETURN(__ubsan_handle_builtin_unreachable)
NORETURN(arch_call_rest_init)
NORETURN(arch_cpu_idle_dead)
NORETURN(cpu_startup_entry)
NORETURN(do_exit)
NORETURN(do_group_exit)
NORETURN(do_task_dead)
NORETURN(fortify_panic)
NORETURN(kthread_complete_and_exit)
NORETURN(kthread_exit)
NORETURN(kunit_try_catch_throw)
NORETURN(make_task_dead)
NORETURN(mpt_halt_firmware)
NORETURN(nmi_panic_self_stop)
NORETURN(panic)
NORETURN(panic_smp_self_stop)
NORETURN(rest_init)
NORETURN(start_kernel)
NORETURN(stop_this_cpu)
NORETURN(usercopy_abort)
