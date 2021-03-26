/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_CMDLINE_H
#define _LINUX_CMDLINE_H

#include <linux/string.h>
#include <linux/printk.h>
#include <asm/setup.h>

/* Allow users to override strlcat/strlcpy, powerpc can't use strings so early*/
#ifndef cmdline_strlcat
#define cmdline_strlcat strlcat
#define cmdline_strlcpy strlcpy
#endif

/*
 * This function will append or prepend a builtin command line to the command
 * line provided by the bootloader. Kconfig options can be used to alter
 * the behavior of this builtin command line.
 * @dst: The destination of the final command line (Min. size COMMAND_LINE_SIZE)
 * @src: The starting string or NULL if there isn't one. Must not equal dst.
 * Returns: false if the string was truncated, true otherwise
 */
static __always_inline bool __cmdline_build(char *dst, const char *src)
{
	int len;
	char *ptr, *last_space;
	bool in_quote = false;

	if (IS_ENABLED(CONFIG_CMDLINE_FORCE))
		src = NULL;

	dst[0] = 0;

	if (IS_ENABLED(CONFIG_CMDLINE_PREPEND))
		len = cmdline_strlcat(dst, CONFIG_CMDLINE " ", COMMAND_LINE_SIZE);

	len = cmdline_strlcat(dst, src, COMMAND_LINE_SIZE);

	if (IS_ENABLED(CONFIG_CMDLINE_APPEND))
		len = cmdline_strlcat(dst, " " CONFIG_CMDLINE, COMMAND_LINE_SIZE);

	if (len < COMMAND_LINE_SIZE - 1)
		return true;

	for (ptr = dst; *ptr; ptr++) {
		if (*ptr == '"')
			in_quote = !in_quote;
		if (*ptr == ' ' && !in_quote)
			last_space = ptr;
	}
	if (last_space)
		*last_space = 0;

	return false;
}

/*
 * This function will append or prepend a builtin command line to the command
 * line provided by the bootloader. Kconfig options can be used to alter
 * the behavior of this builtin command line.
 * @dst: The destination of the final command line (Min. size COMMAND_LINE_SIZE)
 * @src: The starting string or NULL if there isn't one.
 */
static __always_inline void cmdline_build(char *dst, const char *src)
{
	static char tmp[COMMAND_LINE_SIZE] __initdata;

	if (src == dst) {
		cmdline_strlcpy(tmp, src, COMMAND_LINE_SIZE);
		src = tmp;
	}
	if (!__cmdline_build(dst, src))
		pr_warn("Command line is too long, it has been truncated\n");

	if (IS_ENABLED(CONFIG_CMDLINE_FORCE))
		pr_info("Forcing kernel command line to: %s\n", dst);
}

#endif /* _LINUX_CMDLINE_H */
