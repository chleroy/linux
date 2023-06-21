// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdio.h>
#include <stdlib.h>
#include <objtool/check.h>
#include <objtool/elf.h>
#include <objtool/arch.h>
#include <objtool/warn.h>
#include <objtool/builtin.h>
#include <objtool/endianness.h>

int arch_ftrace_match(char *name)
{
	return !strcmp(name, "_mcount");
}

unsigned long arch_dest_reloc_offset(int addend)
{
	return addend;
}

bool arch_callee_saved_reg(unsigned char reg)
{
	return false;
}

int arch_decode_hint_reg(u8 sp_reg, int *base)
{
	exit(-1);
}

const char *arch_nop_insn(int len)
{
	exit(-1);
}

const char *arch_ret_insn(int len)
{
	exit(-1);
}

int arch_decode_instruction(struct objtool_file *file, const struct section *sec,
			    unsigned long offset, unsigned int maxlen,
			    struct instruction *insn)
{
	unsigned int opcode, xop;
	unsigned int rs, ra, rb, bo, bi, to, uimm, simm, lk, aa;
	enum insn_type typ;
	unsigned long imm;
	u32 ins;

	if (file->elf->ehdr.e_flags & EF_PPC_RELOCATABLE_LIB) {
		struct reloc *reloc;

		reloc = find_reloc_by_dest_range(file->elf, insn->sec, insn->offset, 4);

		if (reloc && reloc->type == R_PPC_REL32 &&
		    !strncmp(reloc->sym->sec->name, ".got2", 5)) {
			insn->type = INSN_OTHER;
			insn->ignore = true;
			insn->len = 4;

			return 0;
		}
	}

	ins = bswap_if_needed(file->elf, *(u32 *)(sec->data->d_buf + offset));
	opcode = ins >> 26;
	xop = (ins >> 1) & 0x3ff;
	rs = bo = to = (ins >> 21) & 0x1f;
	ra = bi = (ins >> 16) & 0x1f;
	rb = (ins >> 11) & 0x1f;
	uimm = simm = (ins >> 0) & 0xffff;
	aa = ins & 2;
	lk = ins & 1;

	switch (opcode) {
	case 3:
		if (to == 31 && ra == 0 && simm == 0) /* twi 31, r0, 0 */
			typ = INSN_BUG;
		else
			typ = INSN_OTHER;
		break;
	case 16: /* bc[l][a] */
		if (lk)	/* bcl[a] */
			typ = INSN_OTHER;
		else		/* bc[a] */
			typ = INSN_JUMP_CONDITIONAL;

		imm = ins & 0xfffc;
		if (imm & 0x8000)
			imm -= 0x10000;
		insn->immediate = imm | aa;
		break;
	case 18: /* b[l][a] */
		if (lk)	/* bl[a] */
			typ = INSN_CALL;
		else		/* b[a] */
			typ = INSN_JUMP_UNCONDITIONAL;

		imm = ins & 0x3fffffc;
		if (imm & 0x2000000)
			imm -= 0x4000000;
		insn->immediate = imm | aa;
		break;
	case 19:
		if (xop == 16 && bo == 20 && bi == 0)	/* blr */
			typ = INSN_RETURN;
		else if (xop == 16)	/* bclr */
			typ = INSN_RETURN_CONDITIONAL;
		else if (xop == 50)	/* rfi */
			typ = INSN_JUMP_DYNAMIC;
		else if (xop == 528 && bo == 20 && bi == 0 && !lk)	/* bctr */
			typ = INSN_JUMP_DYNAMIC;
		else if (xop == 528 && bo == 20 && bi == 0 && lk)	/* bctrl */
			typ = INSN_CALL_DYNAMIC;
		else
			typ = INSN_OTHER;
		break;
	case 24:
		if (rs == 0 && ra == 0 && uimm == 0)
			typ = INSN_NOP;
		else
			typ = INSN_OTHER;
		break;
	case 31:
		if (xop == 4 && to == 31 && ra == 0 && rb == 0) /* trap */
			typ = INSN_BUG;
		else
			typ = INSN_OTHER;
		break;
	default:
		typ = INSN_OTHER;
		break;
	}

	if (opcode == 1)
		insn->len = 8;
	else
		insn->len = 4;

	insn->type = typ;

	return 0;
}

unsigned long arch_jump_destination(struct instruction *insn)
{
	if (insn->immediate & 2)
		return insn->immediate & ~2;

	return insn->offset + insn->immediate;
}

bool arch_pc_relative_reloc(struct reloc *reloc)
{
	/*
	 * The powerpc build only allows certain relocation types, see
	 * relocs_check.sh, and none of those accepted are PC relative.
	 */
	return false;
}

void arch_initial_func_cfi_state(struct cfi_init_state *state)
{
	int i;

	for (i = 0; i < CFI_NUM_REGS; i++) {
		state->regs[i].base = CFI_UNDEFINED;
		state->regs[i].offset = 0;
	}

	/* initial CFA (call frame address) */
	state->cfa.base = CFI_SP;
	state->cfa.offset = 0;

	/* initial LR (return address) */
	state->regs[CFI_RA].base = CFI_CFA;
	state->regs[CFI_RA].offset = 0;
}
