// SPDX-License-Identifier: GPL-2.0-or-later
#include <string.h>
#include <stdlib.h>
#include <objtool/special.h>
#include <objtool/builtin.h>


bool arch_support_alt_relocation(struct special_alt *special_alt,
				 struct instruction *insn,
				 struct reloc *reloc)
{
	exit(-1);
}

struct reloc *arch_find_switch_table(struct objtool_file *file,
				    struct instruction *insn, bool *is_rel)
{
	struct reloc  *text_reloc, *rodata_reloc;
	struct section *table_sec;
	unsigned long table_offset;

	/* look for a relocation which references .rodata */
	text_reloc = find_reloc_by_dest_range(file->elf, insn->sec,
					      insn->offset, insn->len);
	if (!text_reloc || text_reloc->sym->type != STT_SECTION ||
	    !text_reloc->sym->sec->rodata)
		return NULL;

	table_offset = text_reloc->addend;
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
	if (find_symbol_containing(table_sec, table_offset)) {
		*is_rel = false;
		if (strcmp(table_sec->name, C_JUMP_TABLE_SECTION))
			return NULL;
	} else {
		*is_rel = true;
	}

	/*
	 * Each table entry has a rela associated with it.  The rela
	 * should reference text in the same function as the original
	 * instruction.
	 */
	rodata_reloc = find_reloc_by_dest(file->elf, table_sec, table_offset);
	if (!rodata_reloc)
		return NULL;

	return rodata_reloc;
}
