all: bench

CFLAGS+= -Wall -march=native -mtune=native

ifeq ($(CC),clang)
	CFLAGS+= -fopenmp-simd
endif

filter.o : filter.c filter.h
	$(CC) $(CFLAGS) -Ofast -c -o $@ $<

springreverb.s : springreverb.c springreverb.h
	$(CC) $(CFLAGS) -Ofast -c -S -o $@ $<

springreverb.o : springreverb.c springreverb.h
	$(CC) $(CFLAGS) -Ofast -c -o $@ $<

bench: bench.c springreverb.o
	$(CC) $(CFLAGS) -O2 -c -o bench.o $<
	$(CC) -lm -lmvec -lgomp bench.o springreverb.o -o bench

clean:
	rm -f filter.o
	rm -f springreverb.o bench.o
	rm -f bench
	rm -f springreverb_faust.o springreverb

springreverb_faust.o : springreverb_inj.cpp springreverb.h
	CXXFLAGS="-c" faust2jack -cn springreverb -inj springreverb_inj.cpp dummy.dsp
	mv dummy springreverb_faust.o

springreverb: springreverb.o springreverb_faust.o filter.o
	$(CXX) `pkg-config --cflags --libs alsa jack gtk+-2.0` -lm $^ -o springreverb
	

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
