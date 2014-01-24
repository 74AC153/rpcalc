default: rpcalc

rpcalc: rpcalc.c
	gcc -Wall -Wextra -Wno-unused-parameter -g -lm -o rpcalc rpcalc.c

clean:
	rm rpcalc
