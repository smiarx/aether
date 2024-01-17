import("interp.dsp");
import("tapedelay.dsp");

d = hslider("delay[0]", 0.1, 0.01, 0.99, 0.01) * ma.SR;
speed =  d : delay2speed: si.smooth(0.9999);

feedback = hslider("feedback[1]", 0.7, 0,1,0.01);
wet = hslider("wet[2]", 0.7, 0,1,0.01);


delay(n,d,x) = interpval : si.bus(4),fd : hermite
with {
    o = 2.9999;
    dmo = d-o;
    id = int(dmo) : min(n-16,max(10));
    fd = ma.frac(dmo);
    // playback speed
    speed = d'-d+1;
    ispeed = min(8,max(0,int(speed+0.99)));
    a = min(1,1/speed);
    // filter
    speedup(0,i) = si.bus(4);
    speedup(N,i) =
        (si.bus(4) <: 
            (x@(id+i-1),si.bus(4) : filter(a)),si.bus(4) :
            ro.interleave(4,2) :
            par(j,4,select2(i==0))
        ) : speedup(N-1,i-1);
    
    interpval = (si.bus(4) <: (speedup(8,ispeed),
                          (x@id,_,_,_,!)
                        ) :
            ro.interleave(4,2) :
            par(j,4,select2(ispeed == 1))) ~ si.bus(4);


    filter(a) = _,(_<:si.bus(2)),si.bus(3) : (a*_ + (1-a)*_),_,_,_,!;
};


process = _,! : (+ : delay((1<<16),tapedelay(1<<16,speed))) ~ (*(feedback) : ma.tanh) : *(wet) <: _,_;
