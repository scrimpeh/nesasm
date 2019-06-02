#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"
#include "expr.h"
#include "util/strcasecmp.h"

/* ----
 * evaluate()
 * ----
 * evaluate an expression
 */
int evaluate(int *ip, char last_char)
{
	int end, level;
	int op, type;
	int arg;
	int i;
	unsigned char c;

	end = 0;
	level = 0;
	undef = 0;
	op_idx = 0;
	val_idx = 0;
	value = 0;
	val_stack[0] = 0;
	need_operator = 0;
	expr_lablptr = NULL;
	expr_lablcnt = 0;
	op = OP_START;
	func_idx = 0;
	inbuilt_arg[0] = 0;

	/* array index to pointer */
	expr = &prlnbuf[*ip];

	/* skip spaces */
cont:
	while (isspace(*expr))
		expr++;

	/* search for a continuation char */
	if (*expr == '\\') {
		/* skip spaces */
		i = 1;
		while (isspace(expr[i]))
			i++;

		/* check if end of line */
		if (expr[i] == ';' || expr[i] == '\0') {
			/* output */
			if (!continued_line) {
				/* replace '\' with three dots */
				strcpy(expr, "...");

				/* store the current line */
				strcpy(tmplnbuf, prlnbuf);
			}

			/* ok */
			continued_line++;

			/* read a new line */
			if (readline() == -1)
				return (0);

			/* rewind line pointer and continue */
			expr = &prlnbuf[SFIELD];
			goto cont;
		}
	}

	/* parser main loop */
	while (!end) {

		if (expr_inbuilt[func_idx]) {
			int status = expr_inbuilt[func_idx]->op();
			if (status == -1)
				return 0;
			else if (status == 0) {
				/* inbuilt done */
				expr_inbuilt[func_idx--] = NULL;	/* forget inbuilt */
				need_operator = 1;
			}
			else if (status <= 10) {
				/* need new arg */
				push_op(OP_START);
				expr_stack[func_idx++] = expr;
				expr = func_arg[func_idx - 2][status - 1];
				continue;
			}
		}

		c = *expr;

		/* number */
		if (isdigit(c)) {
			if (need_operator)
				goto error;
			if (!push_val(T_DECIMAL))
				return (0);
		} 

		/* symbol */
		else
		if (isalpha(c) || c == '_' || c == '.') {
			if (need_operator)
				goto error;
			if (!push_val(T_SYMBOL))
				return (0);
		} 

		/* operators */
		else {
			switch (c) {
			/* function arg */
			case '\\':
				if (func_idx == 0) {
					error("Syntax error in expression!");
					return (0);
				}
				expr++;
				c = *expr++;
				if (c == '#') {
					/* get number of arguments */
					expr_stack[func_idx++] = expr;
					expr = func_argnumbuf[func_idx - 2];
				}
				else if (c == '@') {
					/* get unique number for each function invocation */
					expr_stack[func_idx++] = expr;
					expr = func_fcntbuf[func_idx - 2];
				} 
				else if (c == '?') {
					c = *expr++;
					if (c < '1' || c > '9') {
						error("Invalid function argument index!");
						return (0);
					}

					arg = c - '1';
					expr_stack[func_idx++] = expr;
					expr = func_argtypebuf[func_idx - 2][arg];
				}
				else {
					if (c < '1' || c > '9') {
						error("Invalid function argument index!");
						return 0;
					}
					arg = c - '1';
					expr_stack[func_idx++] = expr;
					expr = func_arg[func_idx - 2][arg];
				}
				break;

			/* hexa prefix */
 			case '$':
				if (need_operator)
					goto error;
				if (!push_val(T_HEXA))
					return 0;
				break;

			/* character prefix */
			case '\'':
				if (need_operator)
					goto error;
				if (!push_val(T_CHAR))
					return (0);
				break;

			/* round brackets */
			case '(':
				if (need_operator)
					goto error;
				if (!push_op(OP_OPEN))
					return (0);
				level++;
				expr++;
				break;
			case ')':
				if (!need_operator)
					goto error;
				if (level == 0)
					goto error;
				while (op_stack[op_idx] != OP_OPEN) {
					if (!do_op())
						return (0);
				}
				op_idx--;
				level--;
				expr++;
				break;

			/* not equal, left shift, lower, lower or equal */
			case '<':
				if (!need_operator)
					goto error;
				expr++;
				switch (*expr) {
				case '>':
					op = OP_NOT_EQUAL;
					expr++;
					break;
				case '<':
					op = OP_SHL;
					expr++;
					break;
				case '=':
					op = OP_LOWER_EQUAL;
					expr++;
					break;
				default:
					op = OP_LOWER;
					break;
				}
				if (!push_op(op))
					return (0);
				break;

			/* right shift, higher, higher or equal */
			case '>':
				if (!need_operator)
					goto error;
				expr++;
				switch (*expr) {
				case '>':
					op = OP_SHR;
					expr++;
					break;
				case '=':
					op = OP_HIGHER_EQUAL;
					expr++;
					break;
				default:
					op = OP_HIGHER;
					break;
				}
				if (!push_op(op))
					return (0);
				break;

			/* equal */
			case '=':
				if (!need_operator)
					goto error;
				if (!push_op(OP_EQUAL))
					return (0);
				expr++;
				break;

			/* one complement */
			case '~':
				if (need_operator)
					goto error;
				if (!push_op(OP_COM))
					return (0);
				expr++;
				break;

			/* sub, neg */
			case '-':
				if (need_operator)
					op = OP_SUB;
				else
					op = OP_NEG;
				if (!push_op(op))
					return (0);
				expr++;
				break;

			/* not, not equal */
			case '!':
				if (!need_operator)
					op = OP_NOT;
				else {
					op = OP_NOT_EQUAL;
					expr++;
					if (*expr != '=')
						goto error;
				}
				if (!push_op(op))
					return (0);
				expr++;
				break;

			/* binary prefix, current PC */
			case '%':
			case '*':
				if (!need_operator) {
					if (c == '%')
						type = T_BINARY;
					else
						type = T_PC;
					if (!push_val(type))
						return (0);
					break;
				}

			/* modulo, mul, add, div, and, xor, or */
			case '+':
			case '/':
			case '&':
			case '^':
			case '|':
				if (!need_operator)
					goto error;
				switch (c) {
				case '%': op = OP_MOD; break;
				case '*': op = OP_MUL; break;
				case '+': op = OP_ADD; break;
				case '/': op = OP_DIV; break;
				case '&': op = OP_AND; break;
				case '^': op = OP_XOR; break;
				case '|': op = OP_OR;  break;
				}
				if (!push_op(op))
					return (0);
				expr++;
				break;

			/* skip immediate operand prefix if in macro */
			case '#':
				if (expand_macro)
					expr++;
				else
					end = 3;
				break;

			/* space or tab */
			case ' ':
			case '\t':
				expr++;
				break;

			/* end of line */
			case '\0':
				if (func_idx) {
					func_idx--;
					expr = expr_stack[func_idx];
					if (expr_inbuilt[func_idx]) {
						/* still currently gathering args */
						while (op_stack[op_idx] != OP_START) {
							if (!do_op())
								return 0;
						}
						op_idx--;
					}
					break;
				}
			case ';':
				end = 1;
				break;
			case ',':
				end = 2;
				break;
			default:
				end = 3;
				break;
			}
		}
	}

	if (!need_operator)
		goto error;
	if (level != 0)
		goto error;
	while (op_stack[op_idx] != OP_START) {
		if (!do_op())
			return (0);
	}

	/* get the expression value */
	value = val_stack[val_idx];

	/* any undefined symbols? trap that if in the last pass */
	if (undef) {
		if (pass == LAST_PASS)
			error("Undefined symbol in operand field!");
	}

	/* check if the last char is what the user asked for */
	switch (last_char) {
	case ';':
		if (end != 1)
			goto error;
		expr++;
		break;		
	case ',':
		if (end != 2) {
			error("Argument missing!");
			return 0;
		}
		expr++;
		break;		
	}

	/* convert back the pointer to an array index */
   *ip = (int)expr - (int)prlnbuf;

	/* ok */
	return (1);

	/* syntax error */
error:
	error("Syntax error in expression!");
	return (0);
}


/* ----
 * push_val()
 * ----
 * extract a number and push it on the value stack
 */
int push_val(int type)
{
	unsigned int mul, val;
	int op;
	char c, *la;

	val = 0;
	c = *expr;

	switch (type) {

	/* char ascii value */
	case T_CHAR:
		expr++;
		val = *expr++;
		if ((*expr != c) || (val == 0)) {
			error("Syntax Error!");
			return (0);
		}
		expr++;
		break;

	/* symbol or pc */
	case T_PC:
	case T_SYMBOL:
		/* extract it */
		if (!getsym())
			return 0;

		/* look-ahead to next character -- if we find parentheses, it must be a function */
		/* otherwise, it's a symbol */

		la = expr;
		while (isspace(*la))
			la++;
		if (*la == '(') {
			/* function or inbuilt */
			if (func_look()) {
				if(!func_getargs())
					return 0;

				expr_stack[func_idx++] = expr;
				expr_inbuilt[func_idx] = NULL;
				fcntmax++;
				fcounter = fcntmax;
				expr = func_ptr->line;
				return 1;
			}
			else {
				t_inbuilt *ib = iblook(symbol);
				if (!ib) {
					error("Unknown function specified!");
					return 0;
				}
				else {
					if (ib->overridable == 2) {
						char errbuf[256];
						sprintf(errbuf, "Function %s has been hidden by another symbol!", &ib->name[1]);
						error(errbuf);
						return 0;
					}

					if (!func_getargs())
						return 0;

					expr_stack[func_idx++] = expr;
					expr_inbuilt[func_idx] = ib;

					inbuilt_arg[func_idx] = 0;

					expr_lablptr = NULL;
					expr_lablcnt = 0;

					return 1;
				}
			}
		}

		/* symbol */
		/* search the symbol */
		expr_lablptr = stlook(1);

		/* check if undefined, if not get its value */
		if (!expr_lablptr)
			return 0;
		else if (expr_lablptr->type == UNDEF)
			undef++;
		else if (expr_lablptr->type == IFUNDEF)
			undef++;
		else
			val = expr_lablptr->value;

		/* remember we have seen a symbol in the expression */
		expr_lablcnt++;
		break;

	/* binary number %1100_0011 */
	case T_BINARY:
		mul = 2;
		goto extract;

	/* hexa number $15AF */
	case T_HEXA:
		mul = 16;
		goto extract;

	/* decimal number 48 (or hexa 0x5F) */
	case T_DECIMAL:
		if((c == '0') && (toupper(expr[1]) == 'X')) {
			mul = 16;
			expr++;
		}
		else {
			mul = 10;
			val = c - '0';
		}
		/* extract a number */
	extract:
		for (;;) {
			expr++;
			c = *expr;
			
			if (isdigit(c))
				c -= '0';
			else if (isalpha(c)) {
				c = toupper(c);
				if (c < 'A' && c > 'F')
					break;
				else {
					c -= 'A';
					c += 10;
				}
			}
			else if (c == '_' && mul == 2)
				continue;
			else
				break;
			if (c >= mul)
				break;
			val = (val * mul) + c;
		}
		break;
	}

	/* check for too big expression */
	if (val_idx == 63) {
		error("Expression too complex!");
		return (0);
	}

	/* push the result on the value stack */
	val_idx++;
	val_stack[val_idx] = val;

	/* next must be an operator */
	need_operator = 1;

	/* ok */
	return (1);
}


/* ----
 * getsym()
 * ----
 * extract a symbol name from the input string
 */

int
getsym(void)
{
	int	valid;
	int	i;
	char c;

	valid = 1;
	i = 0;

	/* special case for the PC */
	if (*expr == '*') {
		symbol[1] = *expr++;
		i = 1;
		valid = 0;
	}

	/* get the symbol, stop to the first 'non symbol' char */
	while (valid) {
		c = *expr;
		if (isalpha(c) || c == '_' || c == '.' || (isdigit(c) && i >= 1)) {
			if (i < SBOLSZ - 1)
				symbol[++i] = c;
			expr++;
		}
		else {
			valid = 0;
		}
	}

	/* is it a reserved symbol? */
	if (i == 1) {
		switch (toupper(symbol[1])) {
		case 'A':
		case 'X':
		case 'Y':
			error("Symbol is reserved (A, X or Y)!");
			i = 0;
		}
	}

	/* store symbol length */	
	symbol[0] = i;
	symbol[i+1] = '\0';
	return i;
}


/* ----
 * push_op()
 * ----
 * push an operator on the stack
 */
int push_op(int op)
{
	if (op != OP_OPEN && op != OP_START) {
		while (op_pri[op_stack[op_idx]] >= op_pri[op]) {
			if (!do_op())
				return 0;
		}
	}
	if (op_idx == 63) {
		error("Expression too complex!");
		return 0;
	}
	op_idx++;
	op_stack[op_idx] = op;
	need_operator = 0;
	return 1;
}


/* ----
 * do_op()
 * ----
 * apply an operator to the value stack
 */
int do_op(void)
{
	int val[2];
	int op;

	/* operator */
	op = op_stack[op_idx--];

	/* first arg */
	val[0] = val_stack[val_idx];

	/* second arg */
	if (op_pri[op] < 9)
		val[1] = val_stack[--val_idx];

	switch (op) {

	case OP_ADD:
		val[0] = val[1] + val[0];
        break;

	case OP_SUB:
		val[0] = val[1] - val[0];
		break;

	case OP_MUL:
		val[0] = val[1] * val[0];
        break;

    case OP_DIV:
        if (val[0] == 0) {
			error("Divide by zero!");
			return (0);
		}
        val[0] = val[1] / val[0];
        break;

	case OP_MOD:
        if (val[0] == 0) {
			error("Divide by zero!");
			return (0);
		}
        val[0] = val[1] % val[0];
        break;

    case OP_NEG:
		val[0] = -val[0];
        break;

    case OP_SHL:
        val[0] = val[1] << (val[0] & 0x1F);
        break;

    case OP_SHR:
        val[0] = val[1] >> (val[0] & 0x1f);
        break;

	case OP_OR:
        val[0] = val[1] | val[0];
		break;

    case OP_XOR:
        val[0] = val[1] ^ val[0];
		break;

    case OP_AND:
        val[0] = val[1] & val[0];
        break;

    case OP_COM:
        val[0] = ~val[0];
        break;

    case OP_NOT:
        val[0] = !val[0];
        break;

    case OP_EQUAL:
        val[0] = (val[1] == val[0]);
        break;

    case OP_NOT_EQUAL:
        val[0] = (val[1] != val[0]);
        break;

    case OP_LOWER:
        val[0] = (val[1] < val[0]);
        break;

    case OP_LOWER_EQUAL:
        val[0] = (val[1] <= val[0]);
        break;

    case OP_HIGHER:
        val[0] = (val[1] > val[0]);
        break;

    case OP_HIGHER_EQUAL:
        val[0] = (val[1] >= val[0]);
        break;

    default:
		error("Invalid operator in expression!");
		return (0);
    }

	/* result */
    val_stack[val_idx] = val[0];
    return (1);
}

int ib_get_one_arg(const char *name)
{
	char buf[64];
	/* argument checks */
	if (func_argtype[func_idx - 1][0] == NO_ARG) {
		sprintf(buf, "No argument for function %s", name);
		error(buf);
		return -1;
	}

	/* others must be empty */
	for (int i = 1; i < FUNC_ARG_COUNT; i++) {
		if (func_argtype[func_idx - 1][i] != NO_ARG) {
			sprintf(buf, "Too many arguments for function %s", name);
			error(buf);
			return -1;
		}
	}

	inbuilt_arg[func_idx]++;

	return 1; /* get arg 1 */
}

int ib_need_one_symbol(const char *name) {
	char buf[64];

	if (!expr_lablptr || expr_lablcnt == 0) {
		sprintf(buf, "No symbol specified for function %s", name);
		error(buf);
		return 0;
	}
	if (expr_lablcnt > 1) {
		sprintf(buf, "Too many symbols for function %s", name);
		error(buf);
		return 0;
	}

	return 1;
}

/*
return values for inbuilts:
0: OK, got value
1 - 9: Need arg #1 - 9 (go back, parse it and push it on the stack)
-1 error
 */
int ib_high(void)
{
	/* inbuilt_arg is a pseudo-sate variable */
	if (inbuilt_arg[func_idx] == 0)
		return(ib_get_one_arg("HIGH"));		/* must get back to parsing one */

	/* all arguments are present */
	val_stack[val_idx] = (val_stack[val_idx] & 0xFF00) >> 8;
	return 0;
}

int ib_low(void)
{
	/* inbuilt_arg is a pseudo-sate variable */
	if (inbuilt_arg[func_idx] == 0)
		return(ib_get_one_arg("LOW"));		/* must get back to parsing one */

	/* all arguments are present */
	val_stack[val_idx] &= 0xFF;
	return 0;
}

int ib_bank(void)
{
	if (inbuilt_arg[func_idx] == 0)
		return(ib_get_one_arg("BANK"));

	if (!ib_need_one_symbol("BANK"))
		return -1;

	if (pass == LAST_PASS && expr_lablptr->bank == RESERVED_BANK)
		error("No BANK index for this symbol!");

	val_stack[val_idx] = expr_lablptr->bank;
	return 0;
}

int ib_pow(void)
{
	if (inbuilt_arg[func_idx] == 0) {
		char buf[64];
		/* argument checks */
		if (func_argtype[func_idx - 1][0] == NO_ARG || func_argtype[func_idx - 1][1] == NO_ARG) {
			error("Missing Argument for function POW");
			return -1;
		}

		/* others must be empty */
		for (int i = 2; i < FUNC_ARG_COUNT; i++) {
			if (func_argtype[func_idx - 1][i] != NO_ARG) {
				sprintf(buf, "Too many arguments for function POW");
				error(buf);
				return -1;
			}
		}

		inbuilt_arg[func_idx]++;

		return 1; /* get arg 1 */
	}
	else if (inbuilt_arg[func_idx] == 1) {
		inbuilt_arg[func_idx]++;
		return 2; /* get arg 2 */
	}
	else {
		if (val_stack[val_idx] == 0)
			val_stack[--val_idx] = 1;
		else for (int exp = val_stack[val_idx--], base = val_stack[val_idx]; exp > 1; exp--)
			val_stack[val_idx] *= base;
		return 0;
	}
}

int ib_page(void)
{
	if (inbuilt_arg[func_idx] == 0)
		return(ib_get_one_arg("PAGE"));

	val_stack[val_idx] >>= 13;

	if (val_stack[val_idx] > 7) {
		error("Page index out of range!");
		return -1;
	}

	return 0;
}

int ib_vram(void) {
	if (inbuilt_arg[func_idx] == 0)
		return(ib_get_one_arg("VRAM"));

	if (!ib_need_one_symbol("VRAM"))
		return -1;

	if (pass == LAST_PASS) {
		if (expr_lablptr->vram == -1)
			error("No palette index for this symbol!");
	}

	val_stack[val_idx] = expr_lablptr->vram;
}

int ib_pal(void) {
	if (inbuilt_arg[func_idx] == 0)
		return(ib_get_one_arg("PAL"));

	if (!ib_need_one_symbol("PAL"))
		return -1;

	if (pass == LAST_PASS) {
		if (expr_lablptr->pal == -1)
			error("No palette index for this symbol!");
	}

	val_stack[val_idx] = expr_lablptr->pal;
}

int ib_sizeof(void) {
	if (inbuilt_arg[func_idx] == 0)
		return(ib_get_one_arg("SIZEOF"));

	if (!ib_need_one_symbol("SIZEOF"))
		return -1;

	if (pass == LAST_PASS) {
		if (expr_lablptr->data_type == -1) {
			error("No size attributes for this symbol!");
			return -1;
		}
	}

	val_stack[val_idx] = expr_lablptr->data_size;
	return 0;
}

int ib_square(void) {
	if (inbuilt_arg[func_idx] == 0)
		return(ib_get_one_arg("SQUARE"));

	val_stack[val_idx] *= val_stack[val_idx];
	return 0;
}

int ib_defined(void) {
	if (inbuilt_arg[func_idx] == 0)
		return(ib_get_one_arg("DEFINED"));

	if (!ib_need_one_symbol("DEFINED"))
		return -1;

	if (expr_lablptr->type != IFUNDEF && expr_lablptr->type != UNDEF)
		val_stack[val_idx] = 1;
	else {
		val_stack[val_idx] = 0;
		undef--;
	}

	return 0;
}
