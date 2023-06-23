/* SPDX-License-Identifier: GPL-2.0 */

/*
 * This is a (sorted!) list of all known __noreturn functions in arch/x86.
 * It's needed for objtool to properly reverse-engineer the control flow graph.
 *
 * Yes, this is unfortunate.  A better solution is in the works.
 */
NORETURN(cpu_bringup_and_idle)
NORETURN(ex_handler_msr_mce)
NORETURN(hlt_play_dead)
NORETURN(hv_ghcb_terminate)
NORETURN(machine_real_restart)
NORETURN(rewind_stack_and_make_dead)
NORETURN(sev_es_terminate)
NORETURN(snp_abort)
NORETURN(x86_64_start_kernel)
NORETURN(x86_64_start_reservations)
NORETURN(xen_cpu_bringup_again)
NORETURN(xen_start_kernel)
