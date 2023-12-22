import("stdfaust.lib");


// Allpass filter
//          a + z⁻D
//  H(z) = ---------
//         1 + a⋅z⁻D
//
// using more efficient recursion
// s[n] = x[n] - a⋅s[n-1]
// y[n] = a⋅s[n] + s[n-1]
allpass(d,a) = _ : (+ <: _@(d-1),*(a))~*(-a) : mem,_ : +;

// Allpass fractional filter
//         a1 + a1⋅a2⋅z⁻¹ + a2⋅z⁻D + z⁻(D+1)
// H(z) = -----------------------------------
//        1 + a2⋅z⁻1 + a1⋅a2⋅z⁻D + a1⋅z⁻(D+1)
//
// where a2 = (1-d)/(1+d), d the fractional delay
//
// using more efficient recursion
// s[n] = x[n] - a2⋅s[n-1] - a1⋅a2⋅s[n-D] - a1⋅s[n-D-1]
// y[n] = a1⋅s[n] + a1⋅a2⋅s[sn-1] + a2⋅s[n-D] + s[n-D-1]
allpassfrac(d,a1,x) = a1*x + (1-a1*a1)*(a2*s@id + s@(id+1))
letrec
{
   's = x - a2*s - a1*(a2*s@(id-1) + s@id);
}
with {
    id = int(d-1/2);
    fd = d - id;
    a2 = (1-fd)/(1+fd);
};

leakyint(a) = _ : (+ : *(1-a))~*(a) : _;

spring(ftr,Td,glf,gecho,gripple) = _ <: (
    (_ <: (ro.interleave(2,2) : (+ : Chigh), (+ : Clow))~(*(c1),*(c2) : ro.cross(2))
           : *(ghigh),*(glow)
    ),
*(gdry)) :> _
with{
    c1=0.; c2=0.1;
    glow = 0.95; ghigh=glow/1000; gdry=1-glow-ghigh;

    //ftr = 4000;
    K = ma.SR/(2*ftr);
    a1 = 0.4175;
    Mlow = 78;

    //Td = 0.04037;
    L = max(0, Td*ma.SR - 4*Mlow*(1-a1)/(1+a1));

    lowchirp = seq(i,Mlow,allpassfrac(K,a1));

    dcfilter = _ <: _,_' : - : *((1+adc)/2) : (+)~*(adc)
    with {
        fcutoff = 20;
        adc = tan(ma.PI*(1/4 - fcutoff/ma.SR));
    };
    lowdelayline(L) =
        de.fdelay1(L+gmod,L0) <: (*(gecho), *(1-gecho)@Lecho) :> _ <: (*(gripple), *(1-gripple)@Lripple) :> *(glf)
    with {
        Nripple = 0.5;
        //gecho = 0.090909;
        //gripple = 0.090909;
        //glf = -0.8;
        gmod = 0.00012*ma.SR;

        mod = no.multinoise(2) : _,! : leakyint(0.93) : *(gmod) : min(1) : max(-1);
        Lmod = L + mod*gmod;

        Lecho = L/5;
        Lripple = 2*K*Nripple;
        L0 = max(0, Lmod-Lecho-Lripple);
    };

    eqfilter = _ <: _,@(2*Keq) : - : (+)~(_<:*(-aeq1),*(-aeq2) : _@(Keq-1), @(2*Keq-1) : +)
    with {
        fpeak = 110;
        B = 95;
        Keq = int(K) : max(1);
        R = 1 - ma.PI*B*Keq/ma.SR;
        aeq1 = 0-(1+R^2)*cos(2*ma.PI*fpeak*Keq/ma.SR);
        aeq2 = R^2;
    };

    lowpassfilter = fi.lowpass6e(ftr)// applycoeff(b) : +~(applycoeff(a) : *(-1))
    with{
        b = (4.66728421e-07, 4.66728421e-06, 2.10027789e-05, 5.60074105e-05,
        9.80129684e-05, 1.17615562e-04, 9.80129684e-05, 5.60074105e-05,
        2.10027789e-05, 4.66728421e-06, 4.66728421e-07);
        a = (-6.54174615e+00,  1.96842227e+01, -3.57633091e+01,
         4.33468099e+01, -3.65569185e+01,  2.16941305e+01, -8.93437192e+00,
         2.44133063e+00, -3.99342798e-01,  2.96727051e-02);

        applycoeff0((a1,a)) = _,(_<:*(a1),mem) : +,_ : applycoeff0(a);
        applycoeff0(a) = _,*(a) : +;
        applycoeff(a) = 0,_ : applycoeff0(a);
    };

    Clow = (+ : dcfilter : lowchirp) ~ lowdelayline(L-1) : eqfilter : lowpassfilter;

    Mhigh=15;//8;
    ahigh = -0.61;
    highchirp = seq(i,Mhigh, fi.allpassnn(1,ahigh)); 
    highdelayline(L) = de.fdelay1(L+gmod, Lmod)
    with {
        gmod = 0.00012*ma.SR;
        mod = no.multinoise(2) : !,_ : leakyint(0.93) : *(gmod) : min(1) : max(-1);
        Lmod = L/2.3 + mod*gmod;
    };

    Chigh = (+ : highchirp)~highdelayline(L);
};


spring_ui = spring(fcr,Td,glf,gecho,gripple)
with {
	fcr = hslider("Freq[0]", 4000, 100, 10000, 1);
	Td = hslider("delay[1]", 0.04174, 0.001, 0.5, 0.001);
	glf = hslider("glf[2]", -0.96, -0.99,0.99,0.001);
	gecho = hslider("gecho[3]", 0.090909, 0,1,0.001);
	gripple = hslider("gripple[4]", 0.090909, 0,1,0.001);
};

wet = hslider("wet", 0.4, 0,1,0.01);

nsprings = 3;
//process = os.impulse : seq(i,1,allpassfrac(11.5,0.3));
process = _ <: (par(i,nsprings,hgroup("spring %i", spring_ui)) :> /(nsprings))*wet <: _,_;

// s = x + (x-s')*a
// y = s - x + s'


