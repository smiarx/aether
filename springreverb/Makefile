all: bench

CFLAGS+= -Wall -march=native -mtune=native

springreverb.o : springreverb.c springreverb.h
	$(CC) $(CFLAGS) -O3 -DMONO -c -o $@ $<

bench: bench.c springreverb.o
	$(CC) $(CFLAGS) -O2 -c -o bench.o $<
	$(CC) -lm -lgomp bench.o springreverb.o -o bench

clean:
	rm springreverb.o bench.o
	rm bench

.PHONY: all clean
