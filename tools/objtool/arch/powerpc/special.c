// SPDX-License-Identifier: GPL-2.0-or-later
#include <string.h>
#include <stdlib.h>
#include <objtool/special.h>
#include <objtool/builtin.h>
#include <objtool/endianness.h>

bool arch_support_alt_relocation(struct special_alt *special_alt,
				 struct instruction *insn,
				 struct reloc *reloc)
{
	exit(-1);
}

struct reloc *arch_find_switch_table(struct objtool_file *file,
				     struct instruction *insn,
				     struct instruction *orig_insn)
{
	struct reloc *text_reloc;
	struct section *table_sec;
	unsigned long table_offset;
	u32 ins;

	/* look for a relocation which references .rodata */
	text_reloc = find_reloc_by_dest_range(file->elf, insn->sec,
					      insn->offset, insn->len);
	if (!text_reloc || reloc_type(text_reloc) != R_PPC_ADDR16_LO ||
	    text_reloc->sym->type != STT_SECTION || !text_reloc->sym->sec->rodata)
		return NULL;

	ins = bswap_if_needed(file->elf, *(u32 *)(insn->sec->data->d_buf + insn->offset));
	if (orig_insn && ((ins >> 21) & 0x1f) != orig_insn->gpr)
		return NULL;

	table_offset = reloc_addend(text_reloc);
	table_sec = text_reloc->sym->sec;

	/*
	 * Make sure the .rodata address isn't associated with a
	 * symbol.  GCC jump tables are anonymous data.
	 *
	 * Also support C jump tables which are in the same format as
	 * switch jump tables.  For objtool to recognize them, they
	 * need to be placed in the C_JUMP_TABLE_SECTION section.  They
	 * have symbols associated with them.
	 */
	if (find_symbol_containing(table_sec, table_offset) &&
	    strcmp(table_sec->name, C_JUMP_TABLE_SECTION))
		return NULL;

	return find_reloc_by_dest(file->elf, table_sec, table_offset);
}
