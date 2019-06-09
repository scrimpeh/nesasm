#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"

t_alias *alias_labels[256];
t_alias *alias_macros[256];
t_alias *alias_funcs[256];
t_alias *alias_directives[256];
t_alias *alias_instructions[256];
t_alias *alias_inbuilts[256];

t_alias *alias_look_table(t_alias **table, int hash, const char *name)
{
	t_alias *alias = table[hash];
	while (alias) {
		if (!strcmp(name, alias->name))
			break;
		alias = alias->next;
	}

	if (alias)
		alias->refcnt++;

	return alias;
}

t_alias *alias_look(const char *name, unsigned int type)
{
	const int hash = symhash(name);
	t_alias* alias;

	if (type & ALIAS_SYMBOL) {
		if (alias = alias_look_table(alias_labels, hash, name))
			return alias;
	}
	if (type & ALIAS_MACRO) {
		if (alias = alias_look_table(alias_macros, hash, name))
			return alias;
	}
	if (type & ALIAS_FUNC) {
		if (alias = alias_look_table(alias_funcs, hash, name))
			return alias;
	}
	if (type & ALIAS_DIRECTIVE) {
		if (alias = alias_look_table(alias_directives, hash, name))
			return alias;
	}
	if (type & ALIAS_INST) {
		if (alias = alias_look_table(alias_instructions, hash, name))
			return alias;
	}
	if (type & ALIAS_INBUILT) {
		if (alias = alias_look_table(alias_inbuilts, hash, name))
			return alias;
	}

	return NULL;
}

t_alias *alias_install(const char *name, void *shadowed, int type)
{
	t_alias **alias_table;
	if (!name || !shadowed)
		return NULL;
	int hash = symhash(name);

	switch (type) {
	default: return NULL;
	case ALIAS_SYMBOL:    alias_table = alias_labels;       break;
	case ALIAS_MACRO:     alias_table = alias_macros;       break;
	case ALIAS_FUNC:      alias_table = alias_funcs;        break;
	case ALIAS_DIRECTIVE: alias_table = alias_directives;   break;
	case ALIAS_INST:      alias_table = alias_instructions; break;
	case ALIAS_INBUILT:   alias_table = alias_inbuilts;     break;
	}

	t_alias *alias = malloc(sizeof(t_alias));
	if (!alias) {
		fatal_error("Out of memory!");
		return NULL;
	}

	alias->type = type;
	strcpy(alias->name, name);
	alias->refcnt = 0;

	switch (alias->type) {
	case ALIAS_SYMBOL:    memcpy(&alias->sym, shadowed, sizeof(t_symbol));  break;
	case ALIAS_MACRO:     memcpy(&alias->macro, shadowed, sizeof(t_macro)); break;
	case ALIAS_FUNC:      memcpy(&alias->func, shadowed, sizeof(t_func));   break;
	case ALIAS_DIRECTIVE: memcpy(&alias->op, shadowed, sizeof(t_opcode));   break;
	case ALIAS_INST:      memcpy(&alias->op, shadowed, sizeof(t_opcode));   break;
	case ALIAS_INBUILT:   memcpy(&alias->ib, shadowed, sizeof(t_inbuilt));  break;
	}

	t_alias *next = alias_table[hash];
	alias->next = next;
	alias_table[hash] = alias;
}

/* functionally, an alias is evaluated in the first pass and maps
   a symbol/directive to a new (case-sensitive) symbol */
   
void do_alias(int *ip)
{
	if (pass == LAST_PASS) {
		println();
		return;
	}

	if (!lablptr) {
		fatal_error("No name for this alias!");
		return;
	}

	while (isspace(prlnbuf[*ip]))
		(*ip)++;
	int ip_old = *ip;
	char target[SBOLSZ];

	if (!get_identifier(target, ip)) {
		fatal_error("No symbol in alias!");
		return;
	}
	if (target[0] == 1) {
		const char reg = toupper(target[1]);
		if (reg == 'A' || reg == 'X' || reg == 'Y') {
			fatal_error("Cannot alias register!");
			return;
		}
	}

	if (strchr(&symbol[1], '.')) {
		fatal_error("Invalid alias name!");
		return;
	}

	if (!check_eol(ip))
		return;

	/* now try to find out what we're aliasing */
	/* first, symbols are checked, then instructions/directives */
	

	memcpy(symbol, target, SBOLSZ); 	/* gotta love that global state */
	
	t_symbol *sym = stlook(0);
	if (sym) {
		sym->refcnt--;					/* don't actually want to reference the symbol */
		if (sym == lablptr) {
			error("Symbol cannot alias itself!");
			goto end;
		}
		switch (sym->type) {
		case MDEF:		/* todo: symbols may be changed internally */
		case DEFABS:	/* aliases should reflect that (although really any symbol that can be changed should be a func anyway */
		case PC:
			alias_install(lablptr->name, sym, ALIAS_SYMBOL);
			goto end;
		}
	}

	t_func *func = func_look();
	if (func) {
		alias_install(lablptr->name, func, ALIAS_FUNC);
		goto end;
	}

	t_macro *macro = macro_look(&ip_old);
	if (macro) {
		alias_install(lablptr->name, macro, ALIAS_MACRO);
		goto end;
	}

	error("Undefined alias!");
end:
	memcpy(symbol, lablptr->name, SBOLSZ);
	lablptr->type = ALIAS;
}