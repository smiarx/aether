import("stdfaust.lib");


// Allpass filter
//          a + z⁻D
//  H(z) = ---------
//         1 + a⋅z⁻D
//
// using more efficient recursion
// s[n] = x[n] - a⋅s[n-1]
// y[n] = a⋅s[n] + s[n-1]
allpass(d,a) = (+ <: _@(d-1),*(a))~*(-a) : mem,_ : +;

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
    id = int(d);
    fd = d - id;
    a2 = (1-fd)/(1+fd);
};

allpassfrac0(d,a1) = _ <:
	(+ <: a2*_@(id-1)+_@id,*(-a2))~(*(-a1),_ : +),_ :
	*(1-a1*a1),!,*(a1) : +
with {
    id = int(d);
    fd = d - id;
    a2 = (1-fd)/(1+fd);
};


allpassfrac1(d,a1) = _ : (+ <: (allpass(1,a2):_@(d-2)),*(a1))~*(-a1) : mem,_ : +
with {
    id = int(d-1/2);
    fd = d - id;
    a2 = (1-fd)/(1+fd);
};


//process = seq(i,10,allpassfrac(hslider("a",10,1,20,0.01),hslider("b",0.3,0,1,0.01)));

process = seq(i, 3, allpassfrac(10.3,0.3));
