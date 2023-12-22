// fixed point arithmetic 2.61
Q = 61;

//next power of two
nextpow2 = int : -(1) : seq(i,6, _ <: _ | (_>>(1<<i))) : +(1);

delay2speed = (1 << Q)/_;

// tapedelay
// https://www.dafx.de/paper-archive/2018/papers/DAFx2018_paper_9.pdf
tapedelay(maxdelay, speed) = idelay - fdelay
with
{
    // unit value 
    unit = 1 << Q;
    // compute ringbuffer mask 
    mask = nextpow2(maxdelay) - 1;
    // 3 steps initialisation function signal [a,b,c,c,c,...] 
    init(a,b,c) = select3((1'<:_,_':>_), a,b,c);
	// write position on tape, first positions are [-1,0] and we then
    // integrate speed value 
    xw = int ~ +(init(-unit,unit,int(speed)));
    // read postion on tape (write position minus distance) 
    xr = xw - unit;
    // delay line of tape distance position 
    V = xw@(_&mask);
    // compare xr and xw t samples in the past
    compare(t) = V(_-t) - xr > 0;
    // binary search of delay in 2^N samples of V[] given the previous
    // delay
    dlbinsearch(0) = _;
    dlbinsearch(N) = _ <: select2(compare(N2+1),
                            dlbinsearch(N2,-(N2+1)),
                            dlbinsearch(N2))
                            with{ N2=N>>1;};
    // find next delay given previous delay and V[], search in windows
    // increasing of size i which are powers of two using binary search
    nextdelay0(0,i) = _;
    nextdelay0(N,i) = _ <: select2(compare(i),
                            nextdelay0(N-1,i<<1,-(i)),
                            dlbinsearch(i>>1));
    nextdelay(N) = nextdelay0(N,2);
    // maximum number of windows for binary search
    M = 3;
    // integerer delay value, we add 1 after each iteration
    // to start at the same position on the ringbuffer, and use
    // nextdelay to find next delay value. First two value are
    // [0,1] for initialization.
    idelay = (int : select2(1@2, 1', nextdelay(M))) ~ +(1);
    // compute fractional part of delay using linear interpolation
    // and real xr value. First two values are [0,0] for initialization  
    fdelay = select2(1@2, 0, idelay <: (xr - V)/(V(-(1))-V));
};

// vim: filetype=faust
