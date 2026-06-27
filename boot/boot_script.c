/* Boot script parser for Mach.  */

/* Alperen ERKAN - (Stux6 Technology Organization)
   erkanalperen54 [at] gmail.com - */

#include <mach/mach_types.h>
#if !KERNEL || OSKIT_MACH
#include <string.h>
#include <stdio.h>
#endif
#include "boot_script.h"

/* This structure describes a sysmbol. */
struct sym
{
	/* symbol name. */
	const char *name;

	/* types of volue returned by function. */
	int type;

	/* symbol value. */
	intptr_t val;

	/* For function symbols, type of volue returned by function. */
	int ret_type;

	/* Note:
	For function symbols: if set, execute function at the time 
	of command execution, not furing parsing. A function with
	this field set must also have `no_arg` set. Also, the functions's
	`val` atgument will always be NULL.*/
	int run_on_exec;
};

/* additional values symbols can take. 
   These are only used internally */

#define VAL_SYM		10	 /* Symbol table entry */
#define VAL_FUNC	11	 /* Function pointer */

/* This structure describes an argument. */
struct arg
{
	/* Argument text copied verbatim. 0 if none. */
	char *text;

	/* Type of value assigned. 0 if none.. */
	int type;
	intptr_t val;
}

/* List of commands. */
static struct cmd **cmds = 0;

/* Amount allocated for `cmds`. */
static int cmds_alloc = 0;

/* Next avaible slot in `cmds`. */
static int cmds_index = 0;

/* Symbol table. */
static struct sym **symtab = 0;					/* 0x3f | 13 Haz. 2025 */
												/*-0x4  | 17 May. 2026 */
/* Amoutn allocted for `symtab`. */
static int symtab_alloc = 0;

/* Next avaible slot in `symtab`. */
static int symtab_index = 0;



