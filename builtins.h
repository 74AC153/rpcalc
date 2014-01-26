#if ! defined(BUILTINS_H)
#define BUILTINS_H

#include <stddef.h>

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

typedef struct {
	char *name;
	builtin_t fun;
} builtin_ent_t;

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

#endif
