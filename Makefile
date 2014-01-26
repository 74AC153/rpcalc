default: rpcalc rpcalc_base.so

SO_LDFLAGS=-fPIC -dynamiclib -Wl,-undefined,dynamic_lookup # MacOSX
#SO_LDFLAGS=-fPIC -shared -lgcov -fprofile-arcs # Linux

rpcalc: rpcalc.c rpcalc_base.c load_wrapper.c
	gcc -Wall -Wextra -Wno-unused-parameter -g -lm -o rpcalc rpcalc.c load_wrapper.c

rpcalc_base.so: rpcalc_base.c
	gcc -g ${SO_LDFLAGS} -o $@ $^

clean:
	rm rpcalc
