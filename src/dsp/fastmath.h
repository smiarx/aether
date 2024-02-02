#ifndef _FASTMATH_H
#define _FASTMATH_H

static inline float hermite(float ym1, float y0, float y1, float y2, float x)
{
    float c0 = y0;
    float c1 = .5 * (y1 - ym1);
    float c2 = ym1 - 2.5 * y0 + 2. * y1 - .5 * y2;
    float c3 = .5 * (y2 - ym1) + 1.5 * (y0 - y1);
    return c0 + (x * (c1 + x * (c2 + x * c3)));
}

/* https://varietyofsound.wordpress.com/2011/02/14/efficient-tanh-computation-using-lamberts-continued-fraction/
 */
#pragma omp declare simd
static inline float fasttanh(float y)
{
    float x      = y;
    float xsq    = x * x;
    float num    = x * (135135.f + xsq * (17325.f + xsq * (378.f + xsq)));
    float den    = 135135.f + xsq * (62370.f + xsq * (3150.f + 28.f * xsq));
    float result = num / den;
    result       = result > 1.f ? 1.f : result;
    result       = result < -1.f ? -1.f : result;
    return result;
}

static inline float db2gain(float db) { return powf(10.f, db / 20.f); }

#endif
