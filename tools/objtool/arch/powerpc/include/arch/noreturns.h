/* SPDX-License-Identifier: GPL-2.0 */

/*
 * This is a (sorted!) list of all known __noreturn functions in arch/powerpc.
 * It's needed for objtool to properly reverse-engineer the control flow graph.
 *
 * Yes, this is unfortunate.  A better solution is in the works.
 */
NORETURN(longjmp)
NORETURN(start_secondary_resume)
NORETURN(unrecoverable_exception)
