PREFIX=${HOME}/opt
DESTDIR=${PREFIX}

default: rpcalc base.so

SO_LDFLAGS=-fPIC -dynamiclib -Wl,-undefined,dynamic_lookup # MacOSX
#SO_LDFLAGS=-fPIC -shared # Linux

rpcalc: rpcalc.c rpcalc_base.c load_wrapper.c
	gcc -Wall -Wextra -Wno-unused-parameter -g -lm -DPREFIX=\"${PREFIX}\" -o $@ $^

base.so: rpcalc_base.c
	gcc -g ${SO_LDFLAGS} -o $@ $^

install: rpcalc base.so
	cp rpcalc ${DESTDIR}/bin
	mkdir -p ${DESTDIR}/etc/rpcalc
	cp rpcalc_init.def ${DESTDIR}/etc/rpcalc/rpcalc_init
	mkdir -p ${DESTDIR}/lib/rpcalc
	cp base.so ${DESTDIR}/lib/rpcalc

clean:
	rm rpcalc base.so
