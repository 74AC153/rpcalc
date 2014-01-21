default: rpcalc

rpcalc: rpcalc.c
	gcc -g -lm -o rpcalc rpcalc.c
