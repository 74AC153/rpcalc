#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "clnode.h"

#define TOKLEN 64
typedef struct {
	clnode_t hdr;
	char name[TOKLEN];
} token_t;

typedef enum {
	VAL_LONG,
	VAL_DOUBLE,
} val_type_t;

typedef struct {
	val_type_t type;
	union {
		double d;
		long l;
	} u;
} val_t;

typedef enum {
	FUN_OK,
	FUN_UNDERFLOW,
	FUN_OVERFLOW,
} status_t;

typedef status_t (*fun_t)(val_t *stack, size_t stack_max, size_t *stack_top);

#define STACK_OUTPUT_NEED(N) \
	do { if(*stack_top == stack_max-(N)-1) return FUN_OVERFLOW; } while(0)

#define STACK_INPUT_NEED(N) \
	do { if(*stack_top < (N)-1) return FUN_UNDERFLOW; } while(0)

#define STACK_ARG(N) \
	stack[*stack_top - (N)]

#define STACK_CONSUME(N) \
	do{ (*stack_top) -= (N); return FUN_OK; } while(0)

#define STACK_EMIT(N) \
	do{ (*stack_top) += (N); return FUN_OK; } while(0)

status_t fun_add(val_t *stack, size_t stack_max, size_t *stack_top)
{
	STACK_INPUT_NEED(2);
	val_type_t type1 = STACK_ARG(1).type, type0 = STACK_ARG(0).type;
	if(type1 == type0) {
		if(type1 == VAL_LONG) STACK_ARG(1).u.l += STACK_ARG(0).u.l;
		else STACK_ARG(1).u.d += STACK_ARG(0).u.d;
	} else {
		if(type1 == VAL_LONG) STACK_ARG(1).u.d = STACK_ARG(1).u.l;
		if(type0 == VAL_LONG) STACK_ARG(0).u.d = STACK_ARG(0).u.l;
		STACK_ARG(1).u.d += STACK_ARG(0).u.d;
		STACK_ARG(1).type = VAL_DOUBLE;
	}
	STACK_CONSUME(1);
}

status_t fun_mult(val_t *stack, size_t stack_max, size_t *stack_top)
{
	STACK_INPUT_NEED(2);
	val_type_t type1 = STACK_ARG(1).type, type0 = STACK_ARG(0).type;
	if(type1 == type0) {
		if(type1 == VAL_LONG) STACK_ARG(1).u.l *= STACK_ARG(0).u.l;
		else STACK_ARG(1).u.d += STACK_ARG(0).u.d;
	} else {
		if(type1 == VAL_LONG) STACK_ARG(1).u.d = STACK_ARG(1).u.l;
		if(type0 == VAL_LONG) STACK_ARG(0).u.d = STACK_ARG(0).u.l;
		STACK_ARG(1).u.d *= STACK_ARG(0).u.d;
		STACK_ARG(1).type = VAL_DOUBLE;
	}
	STACK_CONSUME(1);
}

status_t fun_neg(val_t *stack, size_t stack_max, size_t *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type == VAL_LONG) STACK_ARG(0).u.l *= -1;
	else STACK_ARG(0).u.d *= -1.0;
	STACK_CONSUME(0);
}

status_t fun_div(val_t *stack, size_t stack_max, size_t *stack_top)
{
	STACK_INPUT_NEED(2);
	val_type_t type1 = STACK_ARG(1).type, type0 = STACK_ARG(0).type;
	if(type1 == type0) {
		if(type1 == VAL_LONG) STACK_ARG(1).u.l /= STACK_ARG(0).u.l;
		else STACK_ARG(1).u.d += STACK_ARG(0).u.d;
	} else {
		if(type1 == VAL_LONG) STACK_ARG(1).u.d = STACK_ARG(1).u.l;
		if(type0 == VAL_LONG) STACK_ARG(0).u.d = STACK_ARG(0).u.l;
		STACK_ARG(1).u.d /= STACK_ARG(0).u.d;
		STACK_ARG(1).type = VAL_DOUBLE;
	}
	STACK_CONSUME(1);
}

status_t fun_ceil(val_t *stack, size_t stack_max, size_t *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_LONG) STACK_ARG(0).u.d = ceil(STACK_ARG(0).u.d);
	STACK_CONSUME(0);
}

status_t fun_floor(val_t *stack, size_t stack_max, size_t *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_LONG) STACK_ARG(0).u.d = floor(STACK_ARG(0).u.d);
	STACK_CONSUME(0);
}

status_t fun_ln(val_t *stack, size_t stack_max, size_t *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type == VAL_LONG) STACK_ARG(0).u.d = log(STACK_ARG(0).u.l);
	else STACK_ARG(0).u.d = log(STACK_ARG(0).u.d);
	STACK_ARG(0).type = VAL_DOUBLE;
	STACK_CONSUME(0);
}

status_t fun_exp(val_t *stack, size_t stack_max, size_t *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type == VAL_LONG) STACK_ARG(0).u.d = exp(STACK_ARG(0).u.l);
	else STACK_ARG(0).u.d = exp(STACK_ARG(0).u.d);
	STACK_ARG(0).type = VAL_DOUBLE;
	STACK_CONSUME(0);
}

status_t fun_push_pi(val_t *stack, size_t stack_max, size_t *stack_top)
{
	STACK_OUTPUT_NEED(1);
	STACK_ARG(-1).u.d = M_PI;
	STACK_ARG(-1).type = VAL_DOUBLE;
	STACK_EMIT(1);
}

status_t fun_swap(val_t *stack, size_t stack_max, size_t *stack_top)
{
	val_t r;
	STACK_INPUT_NEED(2);
	r = STACK_ARG(0);
	STACK_ARG(0) = STACK_ARG(1);
	STACK_ARG(0) = r;
	STACK_CONSUME(0);
}

status_t fun_drop(val_t *stack, size_t stack_max, size_t *stack_top)
{
	STACK_INPUT_NEED(1);
	STACK_CONSUME(1);
}

status_t fun_dup(val_t *stack, size_t stack_max, size_t *stack_top)
{
	STACK_INPUT_NEED(1);
	STACK_OUTPUT_NEED(1);
	STACK_ARG(-1) = STACK_ARG(0);
	STACK_EMIT(1);
}

status_t fun_rot(val_t *stack, size_t stack_max, size_t *stack_top)
{
	val_t r;
	STACK_INPUT_NEED(3);
	r = STACK_ARG(0);
	STACK_ARG(0) = STACK_ARG(1);
	STACK_ARG(1) = STACK_ARG(2);
	STACK_ARG(2) = r;
	STACK_CONSUME(0);
}

status_t fun_clear(val_t *stack, size_t stack_max, size_t *stack_top)
{
	*stack_top = 0;
	STACK_CONSUME(0);
}

status_t fun_top(val_t *stack, size_t stack_max, size_t *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type == VAL_LONG) printf("%ld\n", STACK_ARG(0).u.l);
	else printf("%20.20f\n", STACK_ARG(0).u.d);
	STACK_CONSUME(0);
}

status_t fun_stack(val_t *stack, size_t stack_max, size_t *stack_top)
{
	size_t i;
	for(i = 0; i <= *stack_top; i++) {
		if(stack[i].type == VAL_LONG) printf("%ld\n", stack[i].u.l);
		else printf("%20.20f\n", stack[i].u.d);
	}
	STACK_CONSUME(0);
}

struct { char *name; fun_t fun; } builtins[] = {
	{ "add", fun_add },
	{ "mul", fun_mult },
	{ "neg", fun_neg },
	{ "div", fun_div },
	{ "ceil", fun_ceil },
	{ "floor", fun_floor },
	{ "ln", fun_ln },
	{ "exp", fun_exp },
	{ "_pi", fun_push_pi },
	{ "swap", fun_swap },
	{ "drop", fun_drop },
	{ "dup", fun_dup },
	{ "rot", fun_rot },
	{ "clear", fun_clear },
	{ "top", fun_top },
	{ "stack", fun_stack },
};

/* TODO:
define new macro: :macro ... :
load+exec file: [filename]
goto: >name
cond-goto: %name
cond-call: ?name
ret

integer types
int command
bit shifts

!hex mode
!dec mode

clear
reset
*/

#define ARRLEN(X) ( sizeof(X) / sizeof((X)[0]) )

int main(int argc, char *argv[])
{
	int i, j;
	status_t status;
	char *name = argv[0];

	#define DSTACK_CAP 256
	val_t dstack[DSTACK_CAP];
	long dstack_top = -1;
	
	#define CSTACK_CAP 256
	clnode_t *cstack[CSTACK_CAP];
	long cstack_top = -1;

	clnode_t toplevel, *cursor;
	clnode_init(&toplevel);

	for(i = 1; i < argc; i++) {
		token_t *tok = malloc(sizeof(token_t));
		assert(tok);
		clnode_init(&tok->hdr);
		strncpy(tok->name, argv[i], sizeof(tok->name));
		tok->name[sizeof(tok->name)-1] = 0;
		clnode_splice(&toplevel, &tok->hdr);
	}

	for(cstack[++cstack_top] = clnode_next(&toplevel);
	    (cstack_top >= 0) && (cstack[cstack_top] != &toplevel);
		cstack[cstack_top] = clnode_next(cstack[cstack_top])) {

		token_t *tok = (token_t *) cstack[cstack_top];

		for(j = 0; j < sizeof(builtins) / sizeof(builtins[0]); j++) {
			if(strcmp(tok->name, builtins[j].name) == 0) {
				status = builtins[j].fun(dstack, ARRLEN(dstack), &dstack_top);
				if(status != FUN_OK) {
					return status;
				}
				break;
			}
		}

		if(j != ARRLEN(builtins)) continue;
		
		char *strend;
		double d;
		long l;

		if(dstack_top == ARRLEN(dstack)) {
			// overflow
			fprintf(stderr, "%s: stack overflow\n", name);
			return -1;
		}
			
		l = strtol(tok->name, &strend, 0);
		if(strend == tok->name) {
			// no conversion
			fprintf(stderr, "%s: unknown token: %s\n", name, tok->name);
			return -1;
		}
		if(*strend) {
			// incomplete conversion
			goto try_double;
		}
		dstack[++dstack_top].u.l = l;
		dstack[dstack_top].type = VAL_LONG;
		continue;

		try_double:
		d = strtod(tok->name, &strend);
		if(*strend) {
			// incomplete conversion
			fprintf(stderr, "%s: unknown token: %s\n", name, tok->name);
			return -1;
		}

		dstack[++dstack_top].u.d = d;
		dstack[dstack_top].type = VAL_DOUBLE;
	}


	while(! clnode_singleton(&toplevel)) {
		cursor = clnode_next(&toplevel);
		clnode_remove(cursor);
		free(cursor);
	}
	return 0;
}
