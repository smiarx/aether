#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "springreverb.h"

#define loopsprings(i) for (int i = 0; i < NSPRINGS; ++i)

/* filter function */
void filter_set_sos(const float analogsos[][2][3],
                    float digitalsos[][2][3][MAXSPRINGS], const float *freqs,
                    float samplerate, int nsos)
{

    /* filter params */
    /* we define a filter designed with analog coefficients
     * and use bilinear transform to find the corresponding digital coefficients
     * for the desired frequency
     *
     * we use second order section filters (sos) for stability
     */

    loopsprings(i)
    {
        /* bilinear transform */
        float w1  = 2.f * M_PI * freqs[i];
        float c   = 1 / tanf(w1 * 0.5f / samplerate);
        float csq = c * c;
        for (int j = 0; j < nsos; ++j) {
            float a0  = analogsos[j][1][2];
            float a1  = analogsos[j][1][1];
            float b0  = analogsos[j][0][2];
            float b1  = analogsos[j][0][1];
            float b2  = analogsos[j][0][0];
            float d   = 1.f / (a0 + a1 * c + csq);
            float b0d = (b0 + b1 * c + b2 * csq) * d;
            float b1d = 2 * (b0 - b2 * csq) * d;
            float b2d = (b0 - b1 * c + b2 * csq) * d;
            float a1d = 2 * (a0 - csq) * d;
            float a2d = (a0 - a1 * c + csq) * d;

            digitalsos[j][0][0][i] = b0d;
            digitalsos[j][0][1][i] = b1d;
            digitalsos[j][0][2][i] = b2d;
            digitalsos[j][1][0][i] = 1.f;
            digitalsos[j][1][1][i] = a1d;
            digitalsos[j][1][2][i] = a2d;
        }
    }
}

void filter_process(const float (*restrict sos)[2][3][MAXSPRINGS],
                    float (*restrict mem)[FILTERMEMSIZE][MAXSPRINGS],
                    int *restrict id, int nsos, float *restrict y)
{
    for (int j = 0; j < nsos; ++j) {
        /* use direct form II */
        loopsprings(i)
        {
            float a1 = sos[j][1][1][i];
            float a2 = sos[j][1][2][i];
            float b0 = sos[j][0][0][i];
            float b1 = sos[j][0][1][i];
            float b2 = sos[j][0][2][i];

            float v1       = mem[j][(*id - 1) & FILTERMEMMASK][i];
            float v2       = mem[j][(*id - 2) & FILTERMEMMASK][i];
            float v0       = y[i] - a1 * v1 - a2 * v2;
            y[i]           = b0 * v0 + b1 * v1 + b2 * v2;
            mem[j][*id][i] = v0;
        }
    }
    *id = (*id + 1) & FILTERMEMMASK;
}

/* springs */

void springs_init(springs_t *springs, float samplerate)
{
    memset(springs, 0, sizeof(springs_t));
    springs->samplerate = samplerate;
}

void springs_set_dccutoff(springs_t *springs, float *fcutoff)
{
    loopsprings(i)
    {
        springs->adc[i] =
            tanf(M_PI / 4.f - M_PI * fcutoff[i] / springs->samplerate);
    }
}

/* should be first function calledafter init as for now */
void springs_set_ftr(springs_t *springs, float *ftr)
{
    /* compute K factors */
    loopsprings(i) { springs->K[i] = (springs->samplerate / 2.f) / (ftr[i]); }

    /* find down sample factor M
     * the smallest of the K's, new samplerate should be at least
     * twice spring freq to prevent distortion from interpolation
     * then process antialiasing filter coefficient if the factor has
     * changed
     */
    int M = INT_MAX;
    loopsprings(i)
    {
        int Mtmp = springs->K[i] * 0.5f;
        if (Mtmp < M) M = Mtmp;
    }

    if (M != springs->downsampleM) {
        float aafreq[MAXSPRINGS];

        springs->downsampleM     = M;
        springs->downsampleid    = 0;
        loopsprings(i) aafreq[i] = springs->samplerate * 0.5f / M;

        const float analogaasos[][2][3] = {
            {{0., 0., 0.00255383}, {1., 0.21436212, 0.0362477}},
            {{0., 0., 1.}, {1., 0.19337886, 0.21788333}},
            {{0., 0., 1.}, {1., 0.15346633, 0.51177596}},
            {{0., 0., 1.}, {1., 0.09853145, 0.80566858}},
            {{0., 0., 1.}, {1., 0.03395162, 0.98730422}}};
        filter_set_sos(analogaasos, springs->aasos, aafreq, springs->samplerate,
                       NAASOS);
    }

    float samplerate = springs->samplerate / (float)M;
    loopsprings(i)
    {
        springs->K[i] /= (float)M;
        springs->iK[i] = springs->K[i] - .5f;
        float fd       = springs->K[i] - (float)springs->iK[i];
        springs->a2[i] = (1 - fd) / (1 + fd);

        /* EQ params */
        float B = 146.f, fpeak = 183.f;
        int Keq    = springs->K[i];
        float R    = 1.f - M_PI * B * Keq / samplerate;
        float cos0 = (1.f + R * R) / (2.f * R) *
                     cosf(2.f * M_PI * fpeak * Keq / samplerate);
        springs->loweq.Keq[i] = Keq;
        springs->loweq.b0[i]  = (1 - R * R) / 2.f;
        springs->loweq.ak[i]  = -2.f * R * cos0;
        springs->loweq.a2k[i] = R * R;
    }

    /* low pass filter */
    /* scipy.signal.ellip(10, 1, 60, 1.0, analog=True, output='sos') */
    const float analoglowpasssos[][2][3] = {
        {{1.00000000e-03, 0.00000000e+00, 1.43497622e-02},
         {1.00000000e+00, 4.79554974e-01, 1.41121071e-01}},
        {{1.00000000e+00, 0.00000000e+00, 2.24894555e+00},
         {1.00000000e+00, 2.72287841e-01, 5.26272885e-01}},
        {{1.00000000e+00, 0.00000000e+00, 1.33580569e+00},
         {1.00000000e+00, 1.11075112e-01, 8.24889759e-01}},
        {{1.00000000e+00, 0.00000000e+00, 1.12840768e+00},
         {1.00000000e+00, 3.84115770e-02, 9.56268315e-01}},
        {{1.00000000e+00, 0.00000000e+00, 1.07288063e+00},
         {1.00000000e+00, 9.05107247e-03, 9.99553018e-01}}};
    filter_set_sos(analoglowpasssos, springs->lowpasssos, ftr,
                   springs->samplerate, NLOWPASSSOS);
}

void springs_set_a1(springs_t *springs, float *a1)
{
    loopsprings(i)
    {
        if (springs->iK[i] <= 1) {
            /* revert back to classic 2nd order allpass filter */
            springs->iK[i] = 1;
            springs->a2[i] =
                -2.f * sqrtf(a1[i]) * cosf(M_PI / springs->K[i]) / (1 + a1[i]);
        }
        springs->a1[i] = a1[i];
    }
}

void springs_set_ahigh(springs_t *springs, float *ahigh)
{
    loopsprings(i) springs->ahigh[i] = ahigh[i];
}

void springs_set_Td(springs_t *springs, float *Td)
{
    float samplerate = springs->samplerate / (float)springs->downsampleM;
    loopsprings(i)
    {
        float a1          = springs->a1[i];
        float L           = fmaxf(0.f, Td[i] * samplerate -
                                           springs->K[i] * MLOW * (1 - a1) / (1 + a1));
        springs->Lecho[i] = L / 5;
        springs->L1[i]    = L - springs->Lecho[i] - springs->Lripple[i];

        float Lhigh        = L / 2.3f * (float)springs->downsampleM;
        springs->iLhigh[i] = (int)Lhigh;
        springs->fLhigh[i] = Lhigh - (float)springs->iLhigh[i];
    }
}

void springs_set_Nripple(springs_t *springs, float Nripple)
{
    loopsprings(i) { springs->Lripple[i] = 2.f * springs->K[i] * Nripple; }
}

#define gfunc(gname)                                           \
    void springs_set_##gname(springs_t *springs, float *gname) \
    {                                                          \
        loopsprings(i) springs->gname[i] = gname[i];           \
    }

gfunc(gripple) gfunc(gecho) gfunc(glf) gfunc(ghf)

    inline void springs_lowdelayline(springs_t *restrict springs,
                                     float *restrict y)
{
    /* delay modulation */
    loopsprings(i)
    {
        springs->randstate[i] =
            springs->randseed[i] + springs->randstate[i] * 1103515245;
        float mod = springs->randstate[i] / ((float)INT_MAX);
        // filter modulation
        springs->Lmodmem[i] = mod =
            (1 - amod) * mod + amod * springs->Lmodmem[i];
    }
    /* tap low delayline */
#define delayvecs(name)                                   \
    int idx##name[MAXSPRINGS], idx##name##m1[MAXSPRINGS]; \
    float fdelay##name[MAXSPRINGS]
    delayvecs(1);
    delayvecs(echo);
    delayvecs(ripple);
#undef delayvecs
    loopsprings(i)
    {
#define setidx(name, NAME)                                                   \
    int idelay##name = delay##name;                                          \
    fdelay##name[i]  = delay##name - (float)idelay##name;                    \
    idx##name[i] =                                                           \
        (springs->lowdelay##name##id - idelay##name) & LOWDELAY##NAME##MASK; \
    idx##name##m1[i] = (idx##name[i] - 1) & LOWDELAY##NAME##MASK;            \
    idx##name[i]     = idx##name[i] * MAXSPRINGS + i;                        \
    idx##name##m1[i] = idx##name##m1[i] * MAXSPRINGS + i

        float delay1 = springs->L1[i] + gmod * springs->Lmodmem[i];
        setidx(1, 1);
        float delayecho = fdelay1[i] + springs->Lecho[i];
        setidx(echo, ECHO);
        float delayripple = fdelayecho[i] + springs->Lripple[i];
        setidx(ripple, RIPPLE);
#undef setidx
    }

    loopsprings(i)
    {
#define tap(name)                                             \
    float *lowdelay##name = &springs->lowdelay##name[0][0];   \
    float val0##name      = lowdelay##name[idx##name[i]];     \
    float val1##name      = lowdelay##name[idx##name##m1[i]]; \
    float tap##name = val0##name + (val1##name - val0##name) * fdelay##name[i]

        tap(1);
        springs->lowdelayecho[springs->lowdelayechoid][i] =
            tap1 * (1.f - springs->gecho[i]);

        tap(echo);
        tapecho += tap1 * springs->gecho[i];
        springs->lowdelayripple[springs->lowdelayrippleid][i] =
            tapecho * (1.f - springs->gripple[i]);

        tap(ripple);
        tapripple += tapecho * springs->gripple[i];

        y[i] += tapripple * springs->glf[i];
#undef tap
    }
    /* advance buffer ids */
    // no this one
    // springs->lowdelay1id = (springs->lowdelay1id+1) & LOWDELAY1MASK;
    springs->lowdelayechoid = (springs->lowdelayechoid + 1) & LOWDELAYECHOMASK;
    springs->lowdelayrippleid =
        (springs->lowdelayrippleid + 1) & LOWDELAYRIPPLEMASK;
}

inline void springs_lowdc(springs_t *restrict springs, float *restrict y)
{
    loopsprings(i)
    {
        /* update filter state */
        float dcmem1 = springs->dcmem[i];
        float dcmem0 = y[i] + springs->adc[i] * dcmem1;
        /* set output */
        y[i] = (1.f + springs->adc[i]) / 2.f * (dcmem0 - dcmem1);

        springs->dcmem[i] = dcmem0;
    }
}

/* compute low all pass chain */
inline void springs_lowallpasschain(springs_t *restrict springs,
                                    float *restrict y)
{
    /* low allpass filter chain */
    int idx[MAXSPRINGS];
    loopsprings(i) idx[i] = (springs->lowbufid - springs->iK[i]) & MLOWBUFMASK;

    for (int j = 0; j < MLOW; ++j) {
        loopsprings(i)
        {
            /* compute internal allpass1 */
            float s1mem = springs->lowstate[j].mem1[idx[i]][i];
            float s1    = y[i] - springs->a1[i] * s1mem;
            y[i]        = springs->a1[i] * s1 + s1mem;

            /* compute allpass2 */
            float s2mem = springs->lowstate[j].mem2[i];
            float s2    = s1 - springs->a2[i] * s2mem;
            float y2    = springs->a2[i] * s2 + s2mem;

            springs->lowstate[j].mem2[i]                    = s2;
            springs->lowstate[j].mem1[springs->lowbufid][i] = y2;
        }
    }
    springs->lowbufid = (springs->lowbufid + 1) & MLOWBUFMASK;
}

inline void springs_lowlpf(springs_t *restrict springs, float *restrict y)
{
    filter_process(springs->lowpasssos, springs->lowpassmem,
                   &springs->lowpassmemid, NLOWPASSSOS, y);
}

inline void springs_loweq(springs_t *restrict springs, float *restrict y)
{
    int id = springs->loweq.id;
    int idk[MAXSPRINGS];
    int id2k[MAXSPRINGS];
    loopsprings(i)
    {
        idk[i]  = (id - springs->loweq.Keq[i]) & MLOWEQMASK;
        idk[i]  = idk[i] * MAXSPRINGS + i;
        id2k[i] = (id - springs->loweq.Keq[i] * 2) & MLOWEQMASK;
        id2k[i] = id2k[i] * MAXSPRINGS + i;
    }
    float *loweqmem = (float *)springs->loweq.mem;
    loopsprings(i)
    {
        float b0  = springs->loweq.b0[i];
        float ak  = springs->loweq.ak[i];
        float a2k = springs->loweq.a2k[i];

        float vk  = loweqmem[idk[i]];
        float v2k = loweqmem[id2k[i]];
        float v0  = y[i] - ak * vk - a2k * v2k;

        y[i] = b0 * (v0 - v2k);
    }
    springs->loweq.id = (springs->loweq.id + 1) & MLOWEQMASK;
}

inline void springs_highallpasschain(springs_t *restrict springs,
                                     float *restrict y)
{
    /* high chirp allpass chain is stretched by a factor of two,
     * this isn't the way it's supposed to be but it sounds better so ehh..
     */
    int id = springs->highmemid;
    for (int j = 0; j < MHIGH; ++j) {
        loopsprings(i)
        {
            float s2 = springs->highmem[j][id][i];
            float s0 = y[i] - springs->ahigh[i] * s2;

            y[i]                       = springs->ahigh[i] * s0 + s2;
            springs->highmem[j][id][i] = s0;
        }
    }
    springs->highmemid = (springs->highmemid + 1) & 1;
}

void springs_highdelayline(springs_t *restrict springs, float *restrict y)
{
    int idx0[MAXSPRINGS], idx1[MAXSPRINGS];

    loopsprings(i)
    {
        idx0[i] = (springs->highdelayid - springs->iLhigh[i]) & HIGHDELAYMASK;
        idx1[i] = (idx0[i] - 1) & HIGHDELAYMASK;

        idx0[i] = idx0[i] * MAXSPRINGS + 1;
        idx1[i] = idx1[i] * MAXSPRINGS + 1;
    }

    float *delayline = (float *)springs->highdelay;
    loopsprings(i)
    {
        float x0 = delayline[idx0[i]];
        float x1 = delayline[idx1[i]];
        float x  = x0 + springs->fLhigh[i] * (x1 - x0);

        y[i] += x * springs->ghf[i];
    }
}

void springs_process(springs_t *restrict springs, float **restrict in,
                     float **restrict out, int count)
{
    for (int n = 0; n < count; ++n) {
        float ylow[MAXSPRINGS], yhigh[MAXSPRINGS];
        loopsprings(i) yhigh[i] = ylow[i] = in[i * NCHANNELS / NSPRINGS][n];

        if (springs->downsampleM > 1)
            filter_process(springs->aasos, springs->aamem, &springs->aaid,
                           NAASOS, ylow);

        if (springs->downsampleid == 0) {
            springs_lowdelayline(springs, ylow);
            springs_lowdc(springs, ylow);
            springs_lowallpasschain(springs, ylow);

            float ylowin[MAXSPRINGS];
            loopsprings(i) ylowin[i] = ylow[i];
            /* feed delaylines */
            loopsprings(i) springs->lowdelay1[springs->lowdelay1id][i] =
                ylowin[i]; // + ghigh2low*yhigh[i];
            springs->lowdelay1id = (springs->lowdelay1id + 1) & LOWDELAY1MASK;

            springs_loweq(springs, ylow);

            loopsprings(i) ylow[i] *= (float)springs->downsampleM;
        } else {
            loopsprings(i) ylow[i] = 0.f;
        }
        if (springs->downsampleM > 1)
            springs->downsampleid =
                (springs->downsampleid + 1) % springs->downsampleM;

        springs_lowlpf(springs, ylow);

        /* high chirps */
        springs_highdelayline(springs, yhigh);
        springs_highallpasschain(springs, yhigh);

        /* feed high delayline */
        loopsprings(i) springs->highdelay[springs->highdelayid][i] =
            yhigh[i] + glow2high * ylow[i];
        springs->highdelayid = (springs->highdelayid + 1) & HIGHDELAYMASK;

        /* sum low and high */
        float y[MAXSPRINGS];
        loopsprings(i) y[i] = glow * ylow[i] + ghigh * yhigh[i];

        /* sum springs */
        for (int c = 0; c < NCHANNELS; ++c) {
            int offset = c * NSPRINGS / NCHANNELS;
            for (int i = NSPRINGS / 2 / NCHANNELS; i > 0; i /= 2) {
                for (int j = 0; j < i; ++j) y[offset + j] += y[offset + j + i];
            }
            out[c][n] = y[offset];
        }
    }
}
