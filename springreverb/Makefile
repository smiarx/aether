all: bench

CFLAGS+= -Wall -march=native -mtune=native

springreverb.o : springreverb.c springreverb.h
	$(CC) $(CFLAGS) -O3 -DMONO -c -o $@ $<

bench: bench.c springreverb.o
	$(CC) $(CFLAGS) -O2 -c -o bench.o $<
	$(CC) -lm -lgomp bench.o springreverb.o -o bench

clean:
	rm -f springreverb.o bench.o
	rm -f bench

springreverb: springreverb.c springreverb.h springreverb_inj.cpp
	faust2jack -cn springreverb -inj springreverb_inj.cpp springreverb.dsp

dobench:
	@make clean > /dev/null
	@CFLAGS=-DNSPRINGS=1 make > /dev/null
	@echo "1 spring"
	./bench -r 5000
	@make clean > /dev/null
	@make > /dev/null
	@echo
	@echo "8 springs"
	./bench -r 5000

.PHONY: all clean dobench
