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

#include "load_wrapper.h"
#include "builtins.h"

#define ARRLEN(X) ( sizeof(X) / sizeof((X)[0]) )

struct token {
	struct token *next;
	char *name;
};
typedef struct token token_t;

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
	builtin_t fun;
};
typedef struct macro macro_t;

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

void tokenize_buf(char *data, size_t len, filedata_t **dat)
{
	char *start, *end;
	token_t *last = NULL;

	*dat = calloc(1, sizeof(filedata_t));
	assert(*dat);
	(*dat)->buf = data;
	(*dat)->buflen = len;
	
	for(start = data; (size_t) (start-data) < len; start = end) {
		if(isspace(*start)) {
			// skip whitespace
			end = start+1;
		} else if(*start == '#') {
			// skip comments
			for(end = start; (size_t) (end-data) < len && *end != '\n'; end++);
		} else {
			// find end of string segment
			for(end = start;
			    (size_t) (end-data) < len && !isspace(*end);
			    end++);
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
}

int tokenize_file(char *path, filedata_t **dat)
{
	char *data;
	size_t len;
	int status = read_file(path, &data, &len);
	if(status < 0) return status;
	tokenize_buf(data, len, dat);
	return 0;
}

void add_macro(macro_t **macros, char *name, token_t *tokens, builtin_t fun)
{
	macro_t *newmac = calloc(1, sizeof(macro_t));
	assert(newmac);
	newmac->name = name;
	newmac->data = tokens;
	newmac->fun = fun;

	newmac->next = *macros;
	*macros = newmac;
}

int generate_macro(token_t *tok_in, macro_t **macros, token_t **tok_out)
{
	token_t *first = NULL, *last = NULL, *newtok, *cursor;
	char *name = tok_in->name + 5;

	for(cursor = tok_in->next;
	    cursor && strcmp(cursor->name, ":");
	    cursor = cursor->next) {

		newtok = calloc(1, sizeof(token_t));
		assert(newtok);
		memcpy(newtok, cursor, sizeof(*newtok));
		newtok->next = NULL;
		if(last == NULL) {
			first = last = newtok;
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
	add_macro(macros, name, first, NULL);
	*tok_out = cursor;
	return 0;
}


macro_t *find_macro(macro_t *macros, char *name)
{
	for(; macros; macros = macros->next) {
		if(strcmp(macros->name, name) == 0) break;
	}
	return macros;
}

int push_value(char *name, char *progname, filedata_t **files,
               macro_t *macros,
               val_t *dstack, long *dstack_top, size_t dstack_max,
               token_t **cstack, long *cstack_top, size_t cstack_max)
{
	char *strend;
	double d;
	long l;

	// value
	if(*dstack_top == (long) dstack_max) {
		// overflow
		fprintf(stderr, "%s: stack overflow\n", progname);
		return -1;
	}
		
	l = strtol(name, &strend, 0);
	if(strend == name) {
		// no conversion
		fprintf(stderr, "%s: unknown token: %s\n", progname, name);
		return -1;
	}
	if(*strend == 0) {
		dstack[++(*dstack_top)].u.l = l;
		dstack[*dstack_top].type = VAL_LONG;
		return 0;
	}

	d = strtod(name, &strend);
	if(*strend == 0) {
		dstack[++(*dstack_top)].u.d = d;
		dstack[*dstack_top].type = VAL_DOUBLE;
	} else {
		// incomplete conversion
		fprintf(stderr, "%s: unknown token: %s\n", progname, name);
		return -1;
	}

	return 0;
}

int import_builtins(char *path, macro_t **macros)
{
	ldwrap_ent_t *funs, *cursor;
	int status = load_wrapper(path, &funs);
	if(status) {
		return status;
	}

	for(cursor = funs;
	    cursor->name && cursor->fun;
	    cursor++) {

		add_macro(macros, cursor->name, NULL, cursor->fun);
	}

	free(funs);
	return 0;
}

int main(int argc, char *argv[])
{
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
		bool jump_only = false;
		macro_t *macro;

		// implicit return on end of token list
		if(!tok) {
			if(cstack_top) {
				cstack_top--;
				cstack[cstack_top] = cstack[cstack_top]->next;
				continue;
			}
			break;
		}

		char *name = tok->name;

		// conditional
		if(name[0] == '?') {
			if(dstack_top >= 0 && dstack[dstack_top].u.l == 0) {
				cstack[cstack_top] = cstack[cstack_top]->next;
				continue;
			}
			name++;
		}

		// interpreter-specific
		if(name[0] == ':') {
			if(strncmp(name, ":def:", 5) == 0) {
				// macro definition
				generate_macro(tok, &macros, &cstack[cstack_top]);
			} else if(strncmp(name, ":load:", 6) == 0) {
				// file load
				filedata_t *newfile;
				if(tokenize_file(name+6, &newfile) < 0) break;
				newfile->next = files;
				files = newfile;
				tok = cstack[++ cstack_top] = newfile->data;
			} else if(strncmp(name, ":import:", 8) == 0) {
				import_builtins(name + 8, &macros);
				cstack[cstack_top] = cstack[cstack_top]->next;
			} else {
				fprintf(stderr, "%s: unknown directive: %s\n", progname, name);
				break;
			}
			continue;
		}

		// macro / goto
		if(name[0] == '/') {
			jump_only = true;
			name++;
		}
		if((macro = find_macro(macros, name))) {
			if(macro->fun) {
				if(jump_only) {
					fprintf(stderr, "%s: cannot goto builtin %s\n",
					        progname, macro->name);
					break;
				}
				status = macro->fun(dstack, ARRLEN(dstack), &dstack_top);
				if(status != FUN_OK) {
					return status;
				}
				cstack[cstack_top] = cstack[cstack_top]->next;
				continue;
			} else {
				cstack_top += !jump_only;
				cstack[cstack_top] = macro->data;
				continue;
			}
		}

		int status = push_value(name, progname, &files, macros,
		                        dstack, &dstack_top, ARRLEN(dstack),
		                        cstack, &cstack_top, ARRLEN(cstack));
		if(status) break;

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
