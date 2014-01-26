#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


struct token {
	struct token *next;
	char *name;
};
typedef struct token token_t;

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
	FUN_LONG_ARGS_ONLY,
} status_t;

typedef status_t (*builtin_t)(val_t *stack, size_t stack_max, long *stack_top);

struct filedata {
	struct filedata *next;
	struct token *data;
	char *buf;
	size_t buflen;
};
typedef struct filedata filedata_t;

struct macro {
	struct macro *next;
	char *name;
	struct token *data;
};
typedef struct macro macro_t;


#define STACK_OUTPUT_NEED(N) \
	do { if(*stack_top == (long)stack_max-(N)-1) return FUN_OVERFLOW; } while(0)

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

status_t fun_bit_nor(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(2);
	val_type_t type1 = STACK_ARG(1).type, type0 = STACK_ARG(0).type;
	if(type1 != VAL_LONG || type1 != type0) {
		fprintf(stderr, "bit_nor requires LONG\n");
		return FUN_LONG_ARGS_ONLY;
	}
	STACK_ARG(1).u.l = ~( STACK_ARG(1).u.l | STACK_ARG(0).u.l );
	STACK_CONSUME(1);
}

status_t fun_int(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_LONG)
		STACK_ARG(0).u.l = STACK_ARG(0).u.d;
	STACK_ARG(0).type = VAL_LONG;
	STACK_CONSUME(0);
}

status_t fun_ceil(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_LONG)
		STACK_ARG(0).u.d = ceil(STACK_ARG(0).u.d);
	STACK_CONSUME(0);
}

status_t fun_floor(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_LONG)
		STACK_ARG(0).u.d = floor(STACK_ARG(0).u.d);
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

status_t fun_sin(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type == VAL_LONG) STACK_ARG(0).u.d = sin(STACK_ARG(0).u.l);
	else STACK_ARG(0).u.d = sin(STACK_ARG(0).u.d);
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

status_t fun_height(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_OUTPUT_NEED(1);
	STACK_ARG(-1).u.l = *stack_top;
	STACK_EMIT(1);
}

status_t fun_lt_q(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(2);
	double x, y;

	if(STACK_ARG(1).type == VAL_LONG) x = STACK_ARG(1).u.l;
	else x = STACK_ARG(1).u.d;

	if(STACK_ARG(0).type == VAL_LONG) y = STACK_ARG(0).u.l;
	else y = STACK_ARG(0).u.d;

	STACK_ARG(1).u.l = x < y;
	STACK_ARG(1).type = VAL_LONG;

	STACK_CONSUME(1);
}

status_t fun_inf_q(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_DOUBLE) STACK_ARG(0).u.l = 0;
	STACK_ARG(0).u.l = isinf(STACK_ARG(0).u.d);
	STACK_ARG(0).type = VAL_LONG;
	STACK_EMIT(0);
}

status_t fun_nan_q(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_DOUBLE) STACK_ARG(0).u.l = 0;
	STACK_ARG(0).u.l = isnan(STACK_ARG(0).u.d);
	STACK_ARG(0).type = VAL_LONG;
	STACK_EMIT(0);
}

status_t fun_int_q(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	STACK_ARG(0).u.l = STACK_ARG(0).type == VAL_LONG;
	STACK_ARG(0).type = VAL_LONG;
	STACK_EMIT(0);
}

struct { char *name; builtin_t fun; } builtins[] = {
	{ "add", fun_add },
	{ "mul", fun_mult },
	{ "sub", fun_sub },
	{ "div", fun_div },
	{ "bit-nor", fun_bit_nor },
	{ "int", fun_int },
	{ "ceil", fun_ceil },
	{ "floor", fun_floor },
	{ "ln", fun_ln },
	{ "exp", fun_exp },
	{ "sin", fun_sin },
	{ "_pi", fun_push_pi },
	{ "swap", fun_swap },
	{ "drop", fun_drop },
	{ "dup", fun_dup },
	{ "rot", fun_rot },
	{ "clear", fun_clear },
	{ "top", fun_top },
	{ "stack", fun_stack },
	{ "height", fun_height },
	{ "lt?", fun_lt_q },
	{ "inf?", fun_inf_q },
	{ "nan?", fun_nan_q },
	{ "int?", fun_int_q },
};



int read_file(char *path, char **data, size_t *len)
{
	struct stat sb;
	int fd;

	if((fd = open(path, O_RDONLY)) < 0) return -1;
	if(fstat(fd, &sb) < 0) return -2;
	if((*data = malloc(*len = sb.st_size)) == NULL) return -3;
	if(read(fd, *data, sb.st_size) != sb.st_size) return -4;

	close(fd);
	return 0;
}

void tokenize_argv(int argc, char *argv[], filedata_t **files)
{
	int i;
	token_t *tok_last = NULL;
	filedata_t *newfile = calloc(1, sizeof(filedata_t));
	assert(files);
	(newfile)->data = NULL;
	(newfile)->buf = NULL;
	(newfile)->buflen = 0;

	for(i = 1; i < argc; i++) {
		token_t *tok = calloc(1, sizeof(token_t));
		assert(tok);
		tok->name = argv[i];
		if(tok_last == NULL) {
			newfile->data = tok_last = tok;
		} else {
			tok_last = tok_last->next = tok;
		}
	}

	newfile->next = *files;
	*files = newfile;
}

int tokenize_file(char *path, filedata_t **dat)
{
	char *data, *start, *end;
	size_t len;
	token_t *last = NULL;
	int status = read_file(path, &data, &len);
	if(status < 0) return status;

	*dat = calloc(1, sizeof(filedata_t));
	assert(*dat);
	(*dat)->buf = data;
	(*dat)->buflen = len;
	
	for(start = data; (start-data) < len; start = end) {
		if(isspace(*start)) {
			// skip whitespace
			end = start+1;
		} else if(*start == '#') {
			// skip comments
			for(end = start; (end-data) < len && *end != '\n'; end++);
		} else {
			// find end of string segment
			for(end = start; (end-data) < len && ! isspace(*end); end++);
			*end++ = 0;
			token_t *tok = calloc(1, sizeof(token_t));
			assert(tok);
			tok->name = start;
			if( last == NULL ) {
				(*dat)->data = last = tok;
			} else {
				last->next = tok;
				last = tok;
			}
		}
	}

	return 0;
}

int generate_macro(token_t *tok_in, macro_t **macros, token_t **tok_out)
{
	macro_t *newmac = calloc(1, sizeof(macro_t));
	token_t *newtok, *cursor, *last = NULL;

	newmac->name = tok_in->name + 5;

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
	cursor = cursor->next;

	*tok_out = cursor;
	newmac->next = *macros;
	*macros = newmac;
	return 0;
}

#define ARRLEN(X) ( sizeof(X) / sizeof((X)[0]) )

int exec_token(token_t *tok, char *progname, filedata_t **files,
               macro_t *macros,
               val_t *dstack, long *dstack_top, size_t dstack_max,
               token_t **cstack, long *cstack_top, size_t cstack_max)
{
	size_t j;
	char *strend;
	double d;
	long l;
	status_t status;
	char *name;
	bool jump_only = false;

	name = tok->name;

	// conditional
	if(name[0] == '?') {
		if(*dstack_top >= 0 && dstack[(*dstack_top)--].u.l == 0) {
			return 0;
		}
		name++;
	}

	if(name[0] == '/') {
		jump_only = true;
		name++;
	}

	// macro call / goto
	while(macros) {
		if(strcmp(macros->name, name) == 0) {
			*cstack_top += !jump_only;
			tok = cstack[*cstack_top] = macros->data;
			return 0;
		}
		macros = macros->next;
	}
	if(jump_only) {
		fprintf(stderr, "%s: unknown jump target: %s\n", progname, name);
		return -1;
	}

	// builtin
	for(j = 0; j < ARRLEN(builtins); j++) {
		if(strcmp(name, builtins[j].name) == 0) {
			status = builtins[j].fun(dstack, ARRLEN(dstack), dstack_top);
			if(status != FUN_OK) {
				return status;
			}
			break;
		}
	}
	if(j != ARRLEN(builtins)) return 1;

	// value
	if(*dstack_top == (long) dstack_max) {
		// overflow
		fprintf(stderr, "%s: stack overflow\n", progname);
		return -1;
	}
		
	l = strtol(name, &strend, 0);
	if(strend == tok->name) {
		// no conversion
		fprintf(stderr, "%s: unknown token: %s\n", progname, tok->name);
		return -1;
	}
	if(*strend) {
		// incomplete conversion
		goto try_double;
	}
	dstack[++(*dstack_top)].u.l = l;
	dstack[*dstack_top].type = VAL_LONG;
	return 1;

	try_double:
	d = strtod(name, &strend);
	if(*strend) {
		// incomplete conversion
		fprintf(stderr, "%s: unknown token: %s\n", progname, tok->name);
		return -1;
	}

	dstack[++(*dstack_top)].u.d = d;
	dstack[*dstack_top].type = VAL_DOUBLE;

	return 1;
}

int main(int argc, char *argv[])
{
	int i, j;
	status_t status;
	char *progname = argv[0];

	#define DSTACK_CAP 256
	val_t dstack[DSTACK_CAP];
	long dstack_top = -1;
	
	#define CSTACK_CAP 256
	token_t *cstack[CSTACK_CAP];
	long cstack_top = -1;

	macro_t *macros = NULL;
	filedata_t *files = NULL;

	tokenize_argv(argc, argv, &files);

	cstack[++cstack_top] = files->data;

	while(cstack_top >= 0) {
		token_t *tok = (token_t *) cstack[cstack_top];

		// implicit return on end of token list
		if(!tok) {
			if(cstack_top) {
				cstack_top--;
				cstack[cstack_top] = cstack[cstack_top]->next;
				continue;
			}
			break;
		}

		if(tok->name[0] == ':') {
			if(strncmp(tok->name, ":def:", 5) == 0) {
				// macro definition
				generate_macro(tok, &macros, &cstack[cstack_top]);
			} else if(strncmp(tok->name, ":load:", 6) == 0) {
				// file load
				filedata_t *newfile;
				int status = tokenize_file(tok->name+6, &newfile);
				newfile->next = files;
				files = newfile;
				tok = cstack[++ cstack_top] = newfile->data;
			} else if(strcmp(tok->name, ":ret:") == 0) {
				// return from macro
				cstack_top--;
				cstack[cstack_top] = cstack[cstack_top]->next;
			}
			continue;
		}

		int status = exec_token(tok, progname, &files, macros,
		                        dstack, &dstack_top, ARRLEN(dstack),
		                        cstack, &cstack_top, ARRLEN(cstack));
		if(status < 0) break;
		if(status == 0) continue;

		cstack[cstack_top] = cstack[cstack_top]->next;
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
		free(old->buf);
		free(old);
	}

	return 0;
}
