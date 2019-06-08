#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"

struct t_func *func_tbl[256];
struct t_func *func_ptr;
char func_line[FUNC_LENGTH];
char func_arg[8][10][FUNC_ARG_LENGTH];
int  func_argnum[8];
int  func_argtype[8][10];
int  func_idx;
int  fcounter, fcntmax;
int  fcntstack[8];


/* ----
 * do_func()
 * ----
 * .func pseudo
 */
void do_func(int *ip)
{
	if (pass == LAST_PASS)
		println();
	else {
		if (!st_available(lablptr, FUNC))
			return;
		func_install(*ip);
	} 
}

/* search a function */
int func_look(void)
{
	int hash;

	/* search the function in the hash table */
	hash = symhash();
	func_ptr = func_tbl[hash];
	while (func_ptr) {
		if (!strcmp(&symbol[1], func_ptr->name))
			break;			
		func_ptr = func_ptr->next;
	}

	/* ok */
	if (func_ptr)
		return 1;

	/* didn't find a function with this name */
	return 0;
}

/* install a function in the hash table */
int func_install(int ip)
{
	int hash;
	t_inbuilt *ib = iblook(lablptr->name);
	if (ib) {
		if (ib->overridable == 0) {
			error("Symbol already used by a function!");
			return -1;
		}
		ib->overridable = 2;
	}

	/* mark the function name as reserved */
	lablptr->type = FUNC;

	/* check function name syntax */
	if (strchr(&symbol[1], '.')) {
		error("Invalid function name!");
		return 0;
	}

	/* extract function body */
	if (func_extract(ip) == -1)
		return 0;

	/* allocate a new func struct */
	if ((func_ptr = (void *)malloc(sizeof(struct t_func))) == NULL) {
		error("Out of memory!");
		return 0;
	}

	/* initialize it */
	strcpy(func_ptr->name, &symbol[1]);
	strcpy(func_ptr->line, func_line);
	hash = symhash();
	func_ptr->next = func_tbl[hash];
	func_tbl[hash] = func_ptr;

	/* ok */
	return 1;
}

/* extract function body */
int func_extract(int ip)
{
	char *ptr;
	char  c;
	int   i, arg, max_arg, end, arg_valid;

	/* skip spaces */
	while (isspace(prlnbuf[ip]))
		ip++;

	/* get function body */
	ptr = func_line;
	max_arg = 0;
	end = 0;
	i = 0;

	while (!end) {
		c = prlnbuf[ip++];
		switch (c) {
		/* end of line */	
		case ';':
		case '\0':
		   *ptr++ = '\0';
			end = 1;
			break;

		/* function arg */
		case '\\':
			arg_valid = 0;
		   *ptr++ = c;
		    i++;
			c = prlnbuf[ip++];

			if (c == '#' || c == '@')
				arg_valid = 1;
			else if (c >= '1' && c <= '9') {
				arg_valid = 1;
				arg = c - '1';
				if (max_arg < arg)
					max_arg = arg;
			}
			else if (c == '?') {
				*ptr++ = c;
				i++;
				c = prlnbuf[ip++];
				if (c >= '1' && c <= '9') {
					arg_valid = 1;
					arg = c - '1';
					if (max_arg < arg)
						max_arg = arg;
				}
			}

			if (!arg_valid) {
				error("Invalid function argument!");
				return (-1);
			}

		/* other */
		default:
		   *ptr++ = c;
		    i++;
			if (i == FUNC_LENGTH - 1) {
				error("Function line too long!");
				return (-1);
			}
			break;
		}
	}

	/* return the number of args */
	return max_arg;
}

/* extract function args */
int func_getargs(void)
{
	char c, *cur_arg, *line;
	int arg, level, space, read_cur_func_arg, empty_args, last_arg, arg_type;
	int i, x;

	/* can not nest too much macros */
	if (func_idx == 7) {
		error("Too many nested function calls!");
		return 0;
	}

	/* skip spaces */
	while (isspace(*expr))
		expr++;

	/* function args must be enclosed in parenthesis */
	if (*expr++ != '(')
		return 0;

	/* initialize args */
    line = NULL;
	cur_arg = func_arg[func_idx][0];

	for (i = 0; i < 9; i++) {
		func_arg[func_idx][i][0] = '\0';
		func_argtype[func_idx][i] = NO_ARG;
	}
	func_argnum[func_idx] = 0;

	fcntstack[func_idx] = fcntmax;
	arg = 0;
	/* imitate macro behaviour, which counts empty args
	   only if they precede non-empty args */
	empty_args = 0;
	last_arg = 0;

	/* get args one by one */
	for (;;) {
		/* skip spaces */
		while (isspace(*expr))
			expr++;

		c = *expr++;
		switch (c) {
		/* empty arg */
		case ',':
			if (!last_arg)
				empty_args++;
			last_arg = 0;

			arg++;
			cur_arg = func_arg[func_idx][arg];
			if (arg == 9) {
				error("Too many arguments for a function!");
				return 0;
			}
			break;

		/* end of line */	
		case ';':
		case '\0':
			error("Syntax error in function call!");
			return (0);

		/* end of function */
		case ')':
			return 1;

		/* arg */
		default:
			space = 0;
			level = 0;
			read_cur_func_arg = 0;
			i = 0;
			x = 0;

			/* update arg number */
			func_argnum[func_idx]++;
			func_argnum[func_idx] += empty_args;
			last_arg = 1;
			empty_args = 0;

			/* try to determine argument type, imitates macro_getargtype(void), even though
			 only NONE, ABSOLUTE or LABEL really make sense here*/
			arg_type = NO_ARG;
			switch (toupper(c)) {
			case '[': arg_type = ARG_INDIRECT; break;
			case '#': arg_type = ARG_IMM;      break;
			case '"': arg_type = ARG_STRING;   break;
			case 'A':
			case 'X':
			case 'Y':
				if (!isalnum(*expr) && *expr != '_') {	/* look ahead one*/
					arg_type = ARG_REG;
					break;
				}
			default:	/* label or string */
				arg_type = (isdigit(c) || c == '$' || c == '%') ? ARG_ABS : ARG_LABEL; break;
			}
			func_argtype[func_idx][arg] = arg_type;

			for (;;) {
				/* function end? */
				if (c == '\0') {
					if (!read_cur_func_arg)
						break;
					else {
						read_cur_func_arg = 0;
						c = *expr++;
						continue;
					}
				}
				else if (c == ';') {
					break;
				}
				else if (c == ',') {
					if (level == 0)
						break;
				}
				else if (c == '\\') {
					/* read an argument from the current function */
					if (func_idx == 0) {
						error("Syntax error!");
						return 0;
					}	
					c = *expr++;
					if (c == '#' || c == '@') {
						const int src = c == '#' ? func_argnum[func_idx - 1] : fcntstack[func_idx - 1];
						i += snprintf(cur_arg + i, FUNC_ARG_LENGTH - i, "%d", src);
						if (i >= FUNC_ARG_LENGTH) {
							error("Function argument too long!");
							return 0;
						}
						c = *expr++;
					}
					else if (c == '?') {
						c = *expr++;
						if (c < '1' || c > '9') {
							error("Invalid function argument!");
							return 0;
						}
						i += snprintf(cur_arg + i, FUNC_ARG_LENGTH - i, "%d", func_argtype[func_idx - 1][c - '1']);
						if (i >= FUNC_ARG_LENGTH) {
							error("Function argument too long!");
							return 0;
						}
						c = *expr++;
					}
					else if (c >= '1' && c <= '9') {
						line = func_arg[func_idx - 1][c - '1'];
						read_cur_func_arg = 1;
						c = *line++;
					}
					else {
						error("Invalid function argument!");
						return 0;
					}

					continue;
				}
				else if (c == '(') {
					level++;
				}
				else if (c == ')') {
					if (level == 0)
						break;
					level--;
				}
				if (space) {
					if (c != ' ') {
						while (i < x)
							cur_arg[i++] = ' ';
						cur_arg[i++] = c;
						space = 0;
					}
				}
				else if (c == ' ') {
					space = 1;
				}
				else {
					cur_arg[i++] = c;
				}
				if (i == FUNC_ARG_LENGTH) {
					error("Invalid function argument length!");
					return 0;
				}
				x++;
				if (read_cur_func_arg)
					c = *line++;
				else
					c = *expr++;
			}
			cur_arg[i] = '\0';
			expr--;
			break;
		}
	}
}

