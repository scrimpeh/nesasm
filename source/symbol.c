#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"


/* ----
 * symcasehash()
 * ----
 * calculate the case-insensitive hash value of a symbol
 */
int symcasehash(const char *buf)
{
	int hash = 0;
	char c;

	for (int i = 1; i <= buf[0]; i++) {
		c = toupper(buf[i]);
		hash += c;
		hash = (hash << 3) + (hash >> 5) + c;
	}

	return hash & 0xFF;
}

/* ----
 * symhash()
 * ----
 * calculate the hash value of a symbol, must be 
 * a length-indexed string
 */
int symhash(const char *buf)
{
	int hash = 0;
	char c;

	/* hash value */
	for (int i = 1; i <= buf[0]; i++) {
		c = buf[i];
		hash += c;
		hash = (hash << 3) + (hash >> 5) + c;
	}

	/* ok */
	return hash & 0xFF;
}

/* target must hold at least SBOLSZ entries */
int get_identifier(char *target, int *ip)
{
	int  err = 0;
	int	 i = 0;
	char c;

	/* get the symbol */
	for (;;) {
		c = prlnbuf[*ip];
		if (isdigit(c) && i == 0)
			break;
		if (!isalnum(c) && c != '_' && c != '.')
			break;
		if (i < (SBOLSZ - 1))
			target[++i] = c;
		(*ip)++;
	}

	target[0] = i;
	target[i + 1] = '\0';
}

/* ----
 * colsym()
 * ----
 * collect a symbol from prlnbuf into symbol[],
 * leaves prlnbuf pointer at first invalid symbol character,
 * returns 0 if no symbol collected
 */
int colsym(int *ip)
{
	int err = 0;
	get_identifier(symbol, ip);

	/* check if it's a reserved symbol */
	if (symbol[0] == 1) {
		switch (toupper(symbol[1])) {
		case 'A':
		case 'X':
		case 'Y':
			err = 1;
			break;
		}
	}

	t_inbuilt* ib = iblook(symbol);
	if (ib && ib->overridable == 0)	/* not overridable */
		err = 1;

	if (err) {
		fatal_error("Reserved symbol!");
		return 0;
	}	

	return symbol[0];
}


/* ----
 * stlook()
 * ----
 * symbol table lookup
 * if found, return pointer to symbol
 * else, install symbol as undefined and return pointer
 */

t_symbol *stlook(int create)
{
	t_symbol *sym;

	int sym_flag = 0;
	int hash;

	/* local symbol */
	if (symbol[1] == '.') {
		if (glablptr) {
			/* search the symbol in the local list */
			sym = glablptr->local;

			while (sym) {
				if (!strcmp(symbol, sym->name))
					break;			
				sym = sym->next;
			}

			/* new symbol */
			if (!sym) {
				if (create) {
					sym = stinstall(0, 1);
					sym_flag = 1;
				}
			}
		}
		else {
			error("Local symbol not allowed here!");
			return NULL;
		}
	}

	/* global symbol */
	else {
		hash = symhash(symbol);
		/* check aliases first */
		t_alias *alias = alias_look(symbol, ALIAS_SYMBOL | ALIAS_FUNC | ALIAS_MACRO);
		if (alias) 
			return &alias->sym;

		/* search symbol */
		sym = hash_tbl[hash];
		while (sym) {
			if (!strcmp(symbol, sym->name))
				break;			
			sym = sym->next;
		}

		/* new symbol */
		if (!sym) {
			if (create) {
				sym = stinstall(hash, 0);
				sym_flag = 1;
			}
		}
	}

	/* incremente symbol reference counter */
	if (sym_flag == 0) {
		if (sym)
			sym->refcnt++;
	}

	return sym;
}

const char *st_get_name(int type, int uppercase)
{
	if (type == MACRO)
		return uppercase ? "Macro" : "macro";
	else if (type == FUNC)
		return uppercase ? "Function" : "function";
	else
		return uppercase ? "Label" : "label";
}

int st_available(t_symbol *label, int type)
{
	if (!label) {
		error("No name for this %s!", st_get_name(type, 0));
		return 0;
	}

	if (label->overridable == 1) {
		if (lablptr->refcnt > 1) {
			/* reserved word has already been used */
			fatal_error("Cannot hide reserved symbol %s, has already been used!", &label->name[1]);
			return 0;
		}
		lablptr->overridable = 2;
	}
	else if (label->refcnt) {
		if (label->type == type)
			fatal_error("%s already defined!", st_get_name(label->type, 1));
		else
			fatal_error("Symbol already used by a %s!", st_get_name(label->type, 0));
		return 0;
	}

	/* now test for an inbuilt */
	t_inbuilt* ib = iblook(label->name);
	if (ib) {
		if (ib->overridable == 0) {
			fatal_error("Symbol already used by a function!");
			return 0;
		}
		if (ib->refcnt > 2) {	/* 1 while searching the label earlier, 2 now */
			fatal_error("Cannot hide reserved function %s, has already been used!", &ib->name[1]);
			return 0;
		}
		ib->overridable = 2;
	}
	
	/* ok */
	return 1;
}

/* ----
 * stinstall()
 * ----
 * install symbol into symbol hash table
 */
t_symbol *stinstall(int hash, int type)
{
	t_symbol *sym;

	/* allocate symbol structure */
	if (!(sym = malloc(sizeof(t_symbol)))) {
		fatal_error("Out of memory!");
		return (NULL);
	}

	/* init the symbol struct */
	sym->type  = if_expr ? IFUNDEF : UNDEF;
	sym->value = 0;
	sym->local = NULL;
	sym->proc  = NULL;
	sym->bank  = RESERVED_BANK;
	sym->nb    = 0;
	sym->size  = 0;
	sym->page  = -1;
	sym->vram  = -1;
	sym->pal   = -1;
	sym->refcnt = 0;
	sym->reserved = 0;
	sym->data_type = -1;
	sym->data_size = 0;
	sym->overridable = 0;
	strcpy(sym->name, symbol);

	/* add the symbol to the hash table */
	if (type) {
		/* local */
		sym->next = glablptr->local;
		glablptr->local = sym;
	}
	else {
		/* global */
		sym->next = hash_tbl[hash];
		hash_tbl[hash] = sym;
	}

	/* ok */
	return (sym);
}


/* ----
 * labldef()
 * ----
 * assign <lval> to label pointed to by lablptr,
 * checking for valid definition, etc.
 */

int labldef(int lval, int flag)
{
	char c;
	t_inbuilt *ib;

	/* check for NULL ptr */
	if (!lablptr)
		return 0;

	/* adjust symbol address */	
	if (flag)
		lval = (lval & 0x1FFF) | (page << 13);

	/* first pass */
	if (pass == FIRST_PASS) {
		/* newly added inbuilts need a compatibility shim */
		
		if (ib = iblook(lablptr->name)) {
			if (ib->overridable == 0) {
				error("Symbol already used by a function!");
				return -1;
			}
			if (ib->refcnt > 2) {
				/* inbuilt has already been used - for normal symbols, this wouldn't be a problem */
				/* here it is, though, because inbuilts are functions and this is a symbol */
				fatal_error("Cannot hide reserved function %s, has already been used!", &ib->name[1]);
				return -1;
			}
			ib->overridable = 2;			
		}

		/* as do reserved labels */
		if (lablptr->overridable == 1) {
			if ((lablptr->type == MACRO || lablptr->type == FUNC) && lablptr->refcnt > 1) {
				/* label has already been used -- as long as there's no forward definitions of funcs and labels */
				/* this oughta be sufficient */
				/* TODO: add some overrridable inbuilt macros and funcs for testing */
				fatal_error("Cannot hide reserved %s %s, has already been used!", st_get_name(lablptr->type, 0), &lablptr->name[1]);
				return -1;
			}
			lablptr->overridable = 0;
			lablptr->type = DEFABS;
			lablptr->value = lval;
		}
		else {
			switch (lablptr->type) {
				/* undefined */
			case UNDEF:
				lablptr->type = DEFABS;
				lablptr->value = lval;
				break;

				/* already defined - error */
			case IFUNDEF:
				error("Can not define this label, declared as undefined in an IF expression!");
				return -1;

			case MACRO:
				error("Symbol already used by a macro!");
				return -1;

			case FUNC:
				error("Symbol already used by a function!");
				return -1;

			default:
				/* reserved label */
				if (lablptr->reserved) {
					fatal_error("Reserved symbol!");
					return -1;
				}

				/* compare the values */
				if (lablptr->value == lval)
					break;

				/* normal label */
				lablptr->type = MDEF;
				lablptr->value = 0;
				error("Label multiply defined!");
				return (-1);
			}
		}
	}

	/* second pass */
	else {
		if ((lablptr->value != lval) || (flag && bank < bank_limit && lablptr->bank != bank_base + bank))
		{
			if (lablptr->type == UNDEF)
				fatal_error("Label redefinition (Was undefined, is $%04X)", lval);
			else 
				fatal_error("Label redefinition (Was $%04X, is $%04X)", lablptr->value, lval);
			return -1;
		}
	}

	/* update symbol data */
	if (flag) {
		if (section == S_CODE)
			lablptr->proc = proc_ptr;
		lablptr->bank = bank_base + bank;
		lablptr->page = page;

		/* check if it's a local or global symbol */
		c = lablptr->name[1];
		if (c == '.')
			/* local */
			lastlabl = NULL;
		else {
			/* global */
			glablptr = lablptr;
			lastlabl = lablptr;
		}
	}

	/* ok */
	return 0;
}


/* ----
 * hlablset()
 * ----
 * create/update a hidable symbol
 */

void
hlablset(char *name, int val)
{
	lablset(name, val);
	if (lablptr) {
		lablptr->reserved = 0;
		lablptr->overridable = 1;
	}
}


/* ----
 * lablset()
 * ----
 * create/update a reserved symbol
 */

void
lablset(char *name, int val)
{
	int len = strlen(name);
	lablptr = NULL;

	if (len) {
		symbol[0] = len;
		strcpy(&symbol[1], name);
		lablptr = stlook(1);

		if (lablptr) {
			lablptr->type = DEFABS;
			lablptr->value = val;
			lablptr->reserved = 1;
		}
	}
}


/* ----
 * lablremap()
 * ----
 * remap all the labels
 */

void
lablremap(void)
{
	struct t_symbol *sym;
	int i;

	/* browse the symbol table */
	for (i = 0; i < 256; i++) {
		sym = hash_tbl[i];
		while (sym) {
			/* remap the bank */
			if (sym->bank <= bank_limit)
				sym->bank += bank_base;
			sym = sym->next;
		}
	}
}


void funcdump(const char *name, const char *in_fname)
{
    int i;
    FILE *fns;

    fns = fopen(name, "w");
    if (!fns) {
        printf("Cannot open function file '%s'!\n", name);
        exit(1);
    }

    fprintf(fns, "; %s\n", in_fname);

    for (i = 0; i < 256; i++) {
        struct t_symbol *sym;
        for (sym = hash_tbl[i]; sym; sym = sym->next) {
            if (sym->name  &&  sym->bank < RESERVED_BANK) {
                fprintf(fns, "%-32s = $%04X\n", sym->name+1, sym->value);
            }
        }
    }

    fclose(fns);
}
