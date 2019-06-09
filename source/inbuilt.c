#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"


/* ----
 * iblook()
 * ----
 * search for an inbuilt in the table
 */

t_inbuilt *iblook(const char *buf)
{
	t_alias *alias = alias_look(buf, ALIAS_INBUILT);
	if (alias) {
		return &alias->ib;
	}

	/* search symbol */
	int hash = symcasehash(buf);
	t_inbuilt* ib;

	ib = inbuilt_tbl[hash];
	while (ib) {
		if (!strcasecmp(buf, ib->name))
			break;
		ib = ib->next;
	}

	if (ib)
		ib->refcnt++;

	return ib;
}

/* ----
 * ibregister()
 * ----
 * register a new inbuilt with the name "name", and the operation type "op"
 */

void ibregister(char *name, int (*op)(void), int overridable)
{
	int hash, len = strlen(name);
	t_inbuilt *ib;

	/* allocate symbol structure */
	if (!(ib = malloc(sizeof(*ib)))) {
		fatal_error("Out of memory!");
		return NULL;
	}

	ib->op = op;
	ib->name[0] = strlen(name);
	strcpy(&ib->name[1], name);
	ib->overridable = overridable;
	ib->refcnt = 0;

	hash = symcasehash(ib->name);
	ib->next = inbuilt_tbl[hash];
	inbuilt_tbl[hash] = ib;

	return ib;
}