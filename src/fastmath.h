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

#endif
