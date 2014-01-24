#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define TOKLEN 64
#define TOKFMT "%64s"
struct token {
	struct token *next;
	char name[TOKLEN];
};
typedef struct token token_t;

struct filedata {
	struct filedata *next;
	struct token *data;
};
typedef struct filedata filedata_t;

struct macro {
	struct macro *next;
	char name[TOKLEN];
	struct token *data;
};
typedef struct macro macro_t;

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

typedef status_t (*fun_t)(val_t *stack, size_t stack_max, long *stack_top);

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

status_t fun_add(val_t *stack, size_t stack_max, long *stack_top)
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

status_t fun_mult(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(2);
	val_type_t type1 = STACK_ARG(1).type, type0 = STACK_ARG(0).type;
	if(type1 == type0) {
		if(type1 == VAL_LONG) STACK_ARG(1).u.l *= STACK_ARG(0).u.l;
		else STACK_ARG(1).u.d *= STACK_ARG(0).u.d;
	} else {
		if(type1 == VAL_LONG) STACK_ARG(1).u.d = STACK_ARG(1).u.l;
		if(type0 == VAL_LONG) STACK_ARG(0).u.d = STACK_ARG(0).u.l;
		STACK_ARG(1).u.d *= STACK_ARG(0).u.d;
		STACK_ARG(1).type = VAL_DOUBLE;
	}
	STACK_CONSUME(1);
}

status_t fun_sub(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(2);
	val_type_t type1 = STACK_ARG(1).type, type0 = STACK_ARG(0).type;
	if(type1 == type0) {
		if(type1 == VAL_LONG) STACK_ARG(1).u.l -= STACK_ARG(0).u.l;
		else STACK_ARG(1).u.d -= STACK_ARG(0).u.d;
	} else {
		if(type1 == VAL_LONG) STACK_ARG(1).u.d = STACK_ARG(1).u.l;
		if(type0 == VAL_LONG) STACK_ARG(0).u.d = STACK_ARG(0).u.l;
		STACK_ARG(1).u.d -= STACK_ARG(0).u.d;
		STACK_ARG(1).type = VAL_DOUBLE;
	}
	STACK_CONSUME(1);
}

status_t fun_div(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(2);
	val_type_t type1 = STACK_ARG(1).type, type0 = STACK_ARG(0).type;
	if(type1 == type0) {
		if(type1 == VAL_LONG) STACK_ARG(1).u.l /= STACK_ARG(0).u.l;
		else STACK_ARG(1).u.d /= STACK_ARG(0).u.d;
	} else {
		if(type1 == VAL_LONG) STACK_ARG(1).u.d = STACK_ARG(1).u.l;
		if(type0 == VAL_LONG) STACK_ARG(0).u.d = STACK_ARG(0).u.l;
		STACK_ARG(1).u.d /= STACK_ARG(0).u.d;
		STACK_ARG(1).type = VAL_DOUBLE;
	}
	STACK_CONSUME(1);
}

status_t fun_ceil(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_LONG) STACK_ARG(0).u.d = ceil(STACK_ARG(0).u.d);
	STACK_CONSUME(0);
}

status_t fun_floor(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_LONG) STACK_ARG(0).u.d = floor(STACK_ARG(0).u.d);
	STACK_CONSUME(0);
}

status_t fun_ln(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type == VAL_LONG) STACK_ARG(0).u.d = log(STACK_ARG(0).u.l);
	else STACK_ARG(0).u.d = log(STACK_ARG(0).u.d);
	STACK_ARG(0).type = VAL_DOUBLE;
	STACK_CONSUME(0);
}

status_t fun_exp(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type == VAL_LONG) STACK_ARG(0).u.d = exp(STACK_ARG(0).u.l);
	else STACK_ARG(0).u.d = exp(STACK_ARG(0).u.d);
	STACK_ARG(0).type = VAL_DOUBLE;
	STACK_CONSUME(0);
}

status_t fun_push_pi(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_OUTPUT_NEED(1);
	STACK_ARG(-1).u.d = M_PI;
	STACK_ARG(-1).type = VAL_DOUBLE;
	STACK_EMIT(1);
}

status_t fun_swap(val_t *stack, size_t stack_max, long *stack_top)
{
	val_t r;
	STACK_INPUT_NEED(2);
	r = STACK_ARG(0);
	STACK_ARG(0) = STACK_ARG(1);
	STACK_ARG(0) = r;
	STACK_CONSUME(0);
}

status_t fun_drop(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	STACK_CONSUME(1);
}

status_t fun_dup(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	STACK_OUTPUT_NEED(1);
	STACK_ARG(-1) = STACK_ARG(0);
	STACK_EMIT(1);
}

status_t fun_rot(val_t *stack, size_t stack_max, long *stack_top)
{
	val_t r;
	STACK_INPUT_NEED(3);
	r = STACK_ARG(0);
	STACK_ARG(0) = STACK_ARG(1);
	STACK_ARG(1) = STACK_ARG(2);
	STACK_ARG(2) = r;
	STACK_CONSUME(0);
}

status_t fun_clear(val_t *stack, size_t stack_max, long *stack_top)
{
	*stack_top = 0;
	STACK_CONSUME(0);
}

status_t fun_top(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type == VAL_LONG) printf("%ld\n", STACK_ARG(0).u.l);
	else printf("%20.20f\n", STACK_ARG(0).u.d);
	STACK_CONSUME(0);
}

status_t fun_stack(val_t *stack, size_t stack_max, long *stack_top)
{
	long i;
	for(i = 0; i <= *stack_top; i++) {
		if(stack[i].type == VAL_LONG) printf("%ld\n", stack[i].u.l);
		else printf("%20.20f\n", stack[i].u.d);
	}
	STACK_CONSUME(0);
}

struct { char *name; fun_t fun; } builtins[] = {
	{ "add", fun_add },
	{ "mul", fun_mult },
	{ "sub", fun_sub },
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
load+exec file: ::filename
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

int tokenize_file(char *path, filedata_t **dat)
{
	FILE *infile = fopen(path, "r");
	if(! infile) return -1;
	char token[TOKLEN];
	*dat = calloc(1, sizeof(filedata_t));
	assert(dat);
	token_t *last = NULL;
	int status;

	while( (status = fscanf(infile, TOKFMT, token)) > 0) {
		token_t *tok = calloc(1, sizeof(token_t));
		assert(tok);
		strncpy(tok->name, token, sizeof(tok->name));
		tok->name[sizeof(tok->name)-1] = 0;
		if( last == NULL ) {
			(*dat)->data = last = tok;
		} else {
			last->next = tok;
			last = tok;
		}
	}

	fclose(infile);
	return 0;
}

int generate_macro(token_t *tok_in, macro_t **macros, token_t **tok_out)
{
	macro_t *newmac = calloc(1, sizeof(macro_t));
	token_t *newtok, *cursor, *last = NULL;

	strncpy(newmac->name, tok_in->name + 5, sizeof(newmac->name));

	for(cursor = tok_in->next;
	    cursor && strcmp(cursor->name, ":");
	    cursor = cursor->next) {
		newtok = calloc(1, sizeof(token_t));
		assert(newtok);
		memcpy(newtok, cursor, sizeof(*newtok));
		newtok->next = NULL;
		if(last == NULL) {
			newmac->data = last = newtok;
		} else {
			last->next = newtok;
			last = newtok;
		}
	}
	if(! cursor) {
		fprintf(stderr, "non-terminated macro definition!\n");
		return -1;
	}

	*tok_out = cursor;
	newmac->next = *macros;
	*macros = newmac;
	return 0;
}

#define ARRLEN(X) ( sizeof(X) / sizeof((X)[0]) )

int exec_token(token_t *tok, char *name, filedata_t **files,
               macro_t *macros,
               val_t *dstack, long *dstack_top, size_t dstack_max,
               token_t **cstack, long *cstack_top, size_t cstack_max)
{
	size_t j;
	char *strend;
	double d;
	long l;
	status_t status;

	while(macros) {
		if(strcmp(macros->name, tok->name) == 0) {
			tok = cstack[++ (*cstack_top)] = macros->data;
			break;
		}
		macros = macros->next;
	}

	if(tok->name[0] == ':') {
		if(strncmp(tok->name, ":load:", 6) == 0) {
			// read specified file and begin executing it
			filedata_t *newdat;
			int status = tokenize_file(tok->name+6, &newdat);
			newdat->next = *files;
			*files = newdat;
			tok = cstack[++ (*cstack_top)] = newdat->data;
		}
		// TODO: jump to label
		// TODO: conditional goto label
		// TODO: conditional call
	}

	for(j = 0; j < ARRLEN(builtins); j++) {
		if(strcmp(tok->name, builtins[j].name) == 0) {
			status = builtins[j].fun(dstack, ARRLEN(dstack), dstack_top);
			if(status != FUN_OK) {
				return status;
			}
			break;
		}
	}
	if(j != ARRLEN(builtins)) return 0;

	if(*dstack_top == dstack_max) {
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
	dstack[++(*dstack_top)].u.l = l;
	dstack[*dstack_top].type = VAL_LONG;
	return 0;

	try_double:
	d = strtod(tok->name, &strend);
	if(*strend) {
		// incomplete conversion
		fprintf(stderr, "%s: unknown token: %s\n", name, tok->name);
		return -1;
	}

	dstack[++(*dstack_top)].u.d = d;
	dstack[*dstack_top].type = VAL_DOUBLE;

	return 0;
}

int main(int argc, char *argv[])
{
	int i, j;
	status_t status;
	char *name = argv[0];

	#define DSTACK_CAP 256
	val_t dstack[DSTACK_CAP];
	long dstack_top = -1;
	
	#define CSTACK_CAP 256
	token_t *cstack[CSTACK_CAP];
	long cstack_top = -1;

	filedata_t *files = calloc(1, sizeof(filedata_t));
	assert(files);
	files->data = NULL;
	token_t *tok_last = NULL;

	macro_t *macros = NULL;

	for(i = 1; i < argc; i++) {
		token_t *tok = calloc(1, sizeof(token_t));
		assert(tok);
		strncpy(tok->name, argv[i], sizeof(tok->name));
		tok->name[sizeof(tok->name)-1] = 0;
		if(tok_last == NULL) {
			files->data = tok_last = tok;
		} else {
			tok_last = tok_last->next = tok;
		}
	}

	for(cstack[++cstack_top] = files->data;
	    cstack_top >= 0;
		cstack[cstack_top] = cstack[cstack_top]->next) {

		token_t *tok = (token_t *) cstack[cstack_top];

		// implicit return on end of token list
		if(!tok) {
			if(cstack_top) {
				cstack_top--;
				continue;
			}
			break;
		}

		if(tok->name[0] == ':') {
			if(strncmp(tok->name, ":def:", 5) == 0) {
				generate_macro(tok, &macros, &cstack[cstack_top]);
				continue;
			}
		}

		int status = exec_token(tok, name, &files, macros,
		                        dstack, &dstack_top, ARRLEN(dstack),
		                        cstack, &cstack_top, ARRLEN(cstack));
		if(status != 0) break;
	}

	while(files) {
		filedata_t *old;
		while(files->data) {
			token_t *old = files->data;
			files->data = files->data->next;
			free(old);
		}
		old = files;
		files = files->next;
		free(old);
	}

	return 0;
}
