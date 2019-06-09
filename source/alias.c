#include <stdio.h>
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

t_alias *alias_look(const char *name, int type)
{

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
	t_symbol *alias, *sym = stlook(0);
	memcpy(symbol, lablptr->name, SBOLSZ);
	if (sym) {
		sym->refcnt--;					/* don't actually want to reference the symbol */
		if (sym->overridable != 2)
		{
			switch (sym->type) {
			case MDEF:		/* todo: symbols may be changed internally */
			case DEFABS:	/* aliases should reflect that */
			case PC:
				alias = stinstall(symhash(lablptr->name), 0);
				alias->type = DEFABS;
				alias->value = sym->value;
				break;
			}
		}
	}


}