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
#define VAL_FUNC	11	 /* Function pointer*/