#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"


/* ----
 * symcasehash()
 * ----
 * calculate the case-insensitive hash value of a symbol
 */

int
symcasehash(void)
{
	int i;
	char c;
	int hash = 0;

	for (i = 1; i <= symbol[0]; i++) {
		c = toupper(symbol[i]);
		hash += c;
		hash = (hash << 3) + (hash >> 5) + c;
	}

	return hash & 0xFF;
}

/* ----
 * iblook()
 * ----
 * search for an inbuilt in the table
 */

struct t_inbuilt *iblook(void)
{
	/* search symbol */
	int hash = symcasehash();
	struct t_inbuilt* ib;

	ib = inbuilt_tbl[hash];
	while (ib) {
		if (!strcasecmp(symbol, ib->name))
			break;
		ib = ib->next;
	}

	return ib;
}

/* ----
 * ibregister()
 * ----
 * register a new inbuilt with the name "name", and the operation type "op"
 */

void ibregister(char *name, int op, int overridable)
{
	int hash, len = strlen(name);
	t_inbuilt *ib;

	/* allocate symbol structure */
	if (!(ib = malloc(sizeof(*ib)))) {
		fatal_error("Out of memory!");
		return NULL;
	}

	ib->op_type = op;
	ib->name[0] = strlen(name);
	strcpy(&ib->name[1], name);
	ib->overridable = overridable;

	strcpy(symbol, ib->name);
	hash = symcasehash();
	ib->next = inbuilt_tbl[hash];
	inbuilt_tbl[hash] = ib;

	return ib;
}