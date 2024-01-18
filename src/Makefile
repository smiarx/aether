all: bench

CFLAGS+= -Wall -march=native -mtune=native -fopenmp-simd

filter.o : filter.c filter.h
	$(CC) $(CFLAGS) -Ofast -c -o $@ $<

springreverb.s : springreverb.c springreverb.h
	$(CC) $(CFLAGS) -Ofast -c -S -o $@ $<

springreverb.o : springreverb.c springreverb.h
	$(CC) $(CFLAGS) -Ofast -c -o $@ $<

tapedelay.s : tapedelay.c tapedelay.h
	$(CC) $(CFLAGS) -Ofast -c -S -o $@ $<

tapedelay.o : tapedelay.c tapedelay.h
	$(CC) $(CFLAGS) -Ofast -c -o $@ $<

bench.o: bench.c
	$(CC) $(CFLAGS) -O2 -c -o $@ $<
bench: bench.o springreverb.o filter.o
	$(CC) -lm -lmvec -lgomp -o $@ $^


clean:
	rm -f filter.o
	rm -f springreverb.o bench.o
	rm -f bench
	rm -f springreverb_faust.o springreverb
	rm -f tapedelay.o tapedelay_faust.o tapedelay

springreverb_faust.o : springreverb_inj.cpp springreverb.h
	CXXFLAGS="-c" faust2jack -cn springreverb -inj springreverb_inj.cpp dummy.dsp
	mv dummy springreverb_faust.o

springreverb: springreverb.o springreverb_faust.o filter.o
	$(CXX) `pkg-config --cflags --libs alsa jack gtk+-2.0` -lm $^ -o springreverb
	
tapedelay_faust.o : tapedelay_inj.cpp tapedelay.h
	CXXFLAGS="-c" faust2jack -cn tapedelay -inj tapedelay_inj.cpp dummy.dsp
	mv dummy tapedelay_faust.o

tapedelay: tapedelay.o tapedelay_faust.o
	$(CXX) `pkg-config --cflags --libs alsa jack gtk+-2.0` -lm tapedelay.o tapedelay_faust.o -o tapedelay

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
