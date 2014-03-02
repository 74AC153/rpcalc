#include <math.h>
#include <stdio.h>
#include "load_wrapper.h"
#include "builtins.h"

status_t rpbase_add(val_t *stack, size_t stack_max, long *stack_top)
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

status_t rpbase_mult(val_t *stack, size_t stack_max, long *stack_top)
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

status_t rpbase_sub(val_t *stack, size_t stack_max, long *stack_top)
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

status_t rpbase_div(val_t *stack, size_t stack_max, long *stack_top)
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

status_t rpbase_bit_nor(val_t *stack, size_t stack_max, long *stack_top)
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

status_t rpbase_int(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_LONG)
		STACK_ARG(0).u.l = STACK_ARG(0).u.d;
	STACK_ARG(0).type = VAL_LONG;
	STACK_CONSUME(0);
}

status_t rpbase_ceil(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_LONG)
		STACK_ARG(0).u.d = ceil(STACK_ARG(0).u.d);
	STACK_CONSUME(0);
}

status_t rpbase_floor(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_LONG)
		STACK_ARG(0).u.d = floor(STACK_ARG(0).u.d);
	STACK_CONSUME(0);
}

status_t rpbase_ln(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type == VAL_LONG) STACK_ARG(0).u.d = log(STACK_ARG(0).u.l);
	else STACK_ARG(0).u.d = log(STACK_ARG(0).u.d);
	STACK_ARG(0).type = VAL_DOUBLE;
	STACK_CONSUME(0);
}

status_t rpbase_exp(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type == VAL_LONG) STACK_ARG(0).u.d = exp(STACK_ARG(0).u.l);
	else STACK_ARG(0).u.d = exp(STACK_ARG(0).u.d);
	STACK_ARG(0).type = VAL_DOUBLE;
	STACK_CONSUME(0);
}

status_t rpbase_sin(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type == VAL_LONG) STACK_ARG(0).u.d = sin(STACK_ARG(0).u.l);
	else STACK_ARG(0).u.d = sin(STACK_ARG(0).u.d);
	STACK_ARG(0).type = VAL_DOUBLE;
	STACK_CONSUME(0);
}

status_t rpbase_push_pi(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_OUTPUT_NEED(1);
	STACK_ARG(-1).u.d = M_PI;
	STACK_ARG(-1).type = VAL_DOUBLE;
	STACK_EMIT(1);
}

status_t rpbase_swap(val_t *stack, size_t stack_max, long *stack_top)
{
	val_t r;
	STACK_INPUT_NEED(2);
	r = STACK_ARG(0);
	STACK_ARG(0) = STACK_ARG(1);
	STACK_ARG(0) = r;
	STACK_CONSUME(0);
}

status_t rpbase_drop(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	STACK_CONSUME(1);
}

status_t rpbase_dup(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	STACK_OUTPUT_NEED(1);
	STACK_ARG(-1) = STACK_ARG(0);
	STACK_EMIT(1);
}

status_t rpbase_rot(val_t *stack, size_t stack_max, long *stack_top)
{
	val_t r;
	STACK_INPUT_NEED(3);
	r = STACK_ARG(0);
	STACK_ARG(0) = STACK_ARG(1);
	STACK_ARG(1) = STACK_ARG(2);
	STACK_ARG(2) = r;
	STACK_CONSUME(0);
}

status_t rpbase_clear(val_t *stack, size_t stack_max, long *stack_top)
{
	*stack_top = 0;
	STACK_CONSUME(0);
}

status_t rpbase_top(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type == VAL_LONG) printf("%ld\n", STACK_ARG(0).u.l);
	else printf("%20.20f\n", STACK_ARG(0).u.d);
	STACK_CONSUME(0);
}

status_t rpbase_stack(val_t *stack, size_t stack_max, long *stack_top)
{
	long i;
	for(i = 0; i <= *stack_top; i++) {
		if(stack[i].type == VAL_LONG) printf("%ld\n", stack[i].u.l);
		else printf("%20.20f\n", stack[i].u.d);
	}
	STACK_CONSUME(0);
}

status_t rpbase_height(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_OUTPUT_NEED(1);
	STACK_ARG(-1).u.l = *stack_top;
	STACK_EMIT(1);
}

status_t rpbase_lt_q(val_t *stack, size_t stack_max, long *stack_top)
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

status_t rpbase_inf_q(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_DOUBLE) STACK_ARG(0).u.l = 0;
	STACK_ARG(0).u.l = isinf(STACK_ARG(0).u.d);
	STACK_ARG(0).type = VAL_LONG;
	STACK_EMIT(0);
}

status_t rpbase_nan_q(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	if(STACK_ARG(0).type != VAL_DOUBLE) STACK_ARG(0).u.l = 0;
	STACK_ARG(0).u.l = isnan(STACK_ARG(0).u.d);
	STACK_ARG(0).type = VAL_LONG;
	STACK_EMIT(0);
}

status_t rpbase_int_q(val_t *stack, size_t stack_max, long *stack_top)
{
	STACK_INPUT_NEED(1);
	STACK_ARG(0).u.l = STACK_ARG(0).type == VAL_LONG;
	STACK_ARG(0).type = VAL_LONG;
	STACK_EMIT(0);
}

ldwrap_assoc_t base_data_names[] = {
	{ "rpbase_add", "add" },
	{ "rpbase_mult", "mul" },
	{ "rpbase_sub", "sub" },
	{ "rpbase_div", "div" },
	{ "rpbase_bit_nor", "bit-nor" },
	{ "rpbase_int", "int" },
	{ "rpbase_ceil", "ceil" },
	{ "rpbase_floor", "floor" },
	{ "rpbase_ln", "ln" },
	{ "rpbase_exp", "exp" },
	{ "rpbase_sin", "sin" },
	{ "rpbase_push_pi", "_pi" },
	{ "rpbase_swap", "swap" },
	{ "rpbase_drop", "drop" },
	{ "rpbase_dup", "dup" },
	{ "rpbase_rot", "rot" },
	{ "rpbase_clear", "clear" },
	{ "rpbase_top", "top" },
	{ "rpbase_stack", "stack" },
	{ "rpbase_height", "height" },
	{ "rpbase_lt_q", "lt?" },
	{ "rpbase_inf_q", "inf?" },
	{ "rpbase_nan_q", "nan?" },
	{ "rpbase_int_q", "int?" },
};

size_t base_data_count =
	sizeof(base_data_names) / sizeof(base_data_names[0]);
