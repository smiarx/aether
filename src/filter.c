#include <assert.h>
#include <math.h>

#ifndef _FILTER_H
#define _FILTER_C
#endif
#include "filter.h"

#ifdef __cplusplus
#define restrict
#endif

#ifdef FILTER_VECSIZE

typedef struct {
    float b[3][FILTER_VECSIZE];
    float a[3][FILTER_VECSIZE];
    float s[2][FILTER_VECSIZE];
} filter(_t);

void filter(_butterworth_lp)(filter(_t) * filter, float samplerate, float fr);
void filter(_sos_analog)(filter(_t) filter[], const float ba[][2][3],
                         const float freqs[FILTER_VECSIZE], float samplerate,
                         int nsos);

#ifdef _FILTER_C
void filter(_butterworth_lp)(filter(_t) * filter, float samplerate, float fr)
{
    float c   = tanf(M_PI * fr / samplerate);
    float csq = c * c;
    float d   = 1.f / (1.f + sqrtf(2.f) * c + csq);

    float a0 = 1.f;
    float a1 = 2.f * (csq - 1.f) * d;
    float a2 = (1.f - sqrtf(2.f) * c + csq) * d;
    float b0 = csq * d;
    float b1 = 2.f * b0;
    float b2 = b0;
    for (int i = 0; i < FILTER_VECSIZE; ++i)
        filter->a[0][i] = a0, filter->a[1][i] = a1, filter->a[2][i] = a2,
        filter->b[0][i] = b0, filter->b[1][i] = b1, filter->b[2][i] = b2;
}

void filter(_sos_analog)(filter(_t) filter[], const float ba[][2][3],
                         const float freqs[FILTER_VECSIZE], float samplerate,
                         int nsos)
{
    /* filter params */
    /* we define a filter designed with analog coefficients
     * and use bilinear transform to find the corresponding digital coefficients
     * for the desired frequency
     *
     * we use second order section filters (sos) for stability
     */

    for (int i = 0; i < FILTER_VECSIZE; ++i) {
        /* bilinear transform */
        float w1  = 2.f * M_PI * freqs[i];
        float c   = 1 / tanf(w1 * 0.5f / samplerate);
        float csq = c * c;
        for (int j = 0; j < nsos; ++j) {
            assert(ba[j][1][0] == 1.f);
            float a0  = ba[j][1][2];
            float a1  = ba[j][1][1];
            float b0  = ba[j][0][2];
            float b1  = ba[j][0][1];
            float b2  = ba[j][0][0];
            float d   = 1.f / (a0 + a1 * c + csq);
            float b0d = (b0 + b1 * c + b2 * csq) * d;
            float b1d = 2 * (b0 - b2 * csq) * d;
            float b2d = (b0 - b1 * c + b2 * csq) * d;
            float a1d = 2 * (a0 - csq) * d;
            float a2d = (a0 - a1 * c + csq) * d;

            filter[j].b[0][i] = b0d;
            filter[j].b[1][i] = b1d;
            filter[j].b[2][i] = b2d;
            filter[j].a[0][i] = 1.f;
            filter[j].a[1][i] = a1d;
            filter[j].a[2][i] = a2d;
        }
    }
}
#endif

static inline void filter(_process)(filter(_t) * filter,
                                    float y[restrict FILTER_VECSIZE], int nsos)
{
    y = (float *)__builtin_assume_aligned(y, sizeof(float) * FILTER_VECSIZE);

    for (int j = 0; j < nsos; ++j) {
        /* use direct form II */
        for (int i = 0; i < FILTER_VECSIZE; ++i) {
            float a1 = filter[j].a[1][i];
            float a2 = filter[j].a[2][i];
            float b0 = filter[j].b[0][i];
            float b1 = filter[j].b[1][i];
            float b2 = filter[j].b[2][i];

            float s1 = filter[j].s[0][i];
            float s2 = filter[j].s[1][i];
            float s0 = y[i] - a1 * s1 - a2 * s2;
            y[i]     = b0 * s0 + b1 * s1 + b2 * s2;

            filter[j].s[1][i] = filter[j].s[0][i];
            filter[j].s[0][i] = s0;
        }
    }
}
#endif
