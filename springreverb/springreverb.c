#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "springreverb.h"

#define loopsprings(i) for (int i = 0; i < NSPRINGS; ++i)

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

void springs_set_ftr(springs_t *springs, float *ftr)
{
    loopsprings(i)
    {
        springs->K[i]  = (springs->samplerate / 2.f) / (ftr[i]);
        springs->iK[i] = springs->K[i] - .5f;
        float fd       = springs->K[i] - (float)springs->iK[i];
        springs->a2[i] = (1 - fd) / (1 + fd);

        /* EQ params */
        float B = 146.f, fpeak = 183.f;
        int Keq    = springs->K[i];
        float R    = 1.f - M_PI * B * Keq / springs->samplerate;
        float cos0 = (1.f + R * R) / (2.f * R) *
                     cosf(2.f * M_PI * fpeak * Keq / springs->samplerate);
        springs->loweq.Keq[i] = Keq;
        springs->loweq.b0[i]  = (1 - R * R) / 2.f;
        springs->loweq.ak[i]  = -2.f * R * cos0;
        springs->loweq.a2k[i] = R * R;
    }

    /* LowPass params */
    /* let's use a fixed ellptic lowpass filter designed with analog
     * coefficients and use bilinear transform to find the corresponding digital
     * coefficients for the desired frequency
     */

    /* scipy.signal.ellip(10, 1, 60, 1.0, analog=True, output='sos') */
    const float analogsos[][2][3] = {
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

    loopsprings(i)
    {
        /* bilinear transform */
        float w1  = 2.f * M_PI * ftr[i];
        float c   = 1 / tanf(w1 * 0.5f / springs->samplerate);
        float csq = c * c;
        for (int j = 0; j < NLOWPASSSOS; ++j) {
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

            springs->lowpasssos[j][0][0][i] = b0d;
            springs->lowpasssos[j][0][1][i] = b1d;
            springs->lowpasssos[j][0][2][i] = b2d;
            springs->lowpasssos[j][1][0][i] = 1.f;
            springs->lowpasssos[j][1][1][i] = a1d;
            springs->lowpasssos[j][1][2][i] = a2d;
        }
    }
}

void springs_set_a1(springs_t *springs, float *a1)
{
    loopsprings(i) springs->a1[i] = a1[i];
}

void springs_set_ahigh(springs_t *springs, float *ahigh)
{
    loopsprings(i) springs->ahigh[i] = ahigh[i];
}

void springs_set_Td(springs_t *springs, float *Td)
{
    loopsprings(i)
    {
        float a1          = springs->a1[i];
        float L           = fmaxf(0.f, Td[i] * springs->samplerate -
                                           springs->K[i] * MLOW * (1 - a1) / (1 + a1));
        springs->Lecho[i] = L / 5;
        springs->L1[i]    = L - springs->Lecho[i] - springs->Lripple[i];

        float Lhigh        = L / 2.3f;
        springs->iLhigh[i] = (int)Lhigh;
        springs->fLhigh[i] = Lhigh - (float)springs->iLhigh[i];
    }
}

void springs_set_Nripple(springs_t *springs, float Nripple)
{
    loopsprings(i) { springs->Lripple[i] = 2.f * springs->K[i] * Nripple; }
}

inline void springs_lowdelayline(springs_t *restrict springs, float *restrict y)
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
            tap1 * (1.f - gecho);

        tap(echo);
        tapecho += tap1 * gecho;
        springs->lowdelayripple[springs->lowdelayrippleid][i] =
            tapecho * (1.f - gripple);

        tap(ripple);
        tapripple += tapecho * gripple;

        y[i] += tapripple * glf;
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
    /* low pass filter */
    int id                                       = springs->lowpassmemid;
    loopsprings(i) springs->lowpassmem[0][id][i] = y[i];
    for (int j = 0; j < NLOWPASSSOS; ++j) {
        float acc[MAXSPRINGS] = {0};
        for (int k = 0; k < 3; ++k) loopsprings(i)
            {
                acc[i] += springs->lowpasssos[j][0][k][i] *
                          springs->lowpassmem[j][(id - k) & LOWPASSMEMMASK][i];
                if (k > 0)
                    acc[i] -=
                        springs->lowpasssos[j][1][k][i] *
                        springs
                            ->lowpassmem[j + 1][(id - k) & LOWPASSMEMMASK][i];
            }
        loopsprings(i) springs->lowpassmem[j + 1][id][i] = acc[i];
    }
    loopsprings(i) y[i]   = springs->lowpassmem[NLOWPASSSOS][id][i];
    springs->lowpassmemid = (springs->lowpassmemid + 1) & LOWPASSMEMMASK;
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

void springs_highallpasschain(springs_t *restrict springs, float *restrict y)
{
    for (int j = 0; j < MHIGH; ++j) {
        loopsprings(i)
        {
            float s1 = springs->highmem[j][i];
            float s0 = y[i] - springs->ahigh[i] * s1;

            y[i]                   = springs->ahigh[i] * s0 + s1;
            springs->highmem[j][i] = s0;
        }
    }
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

        y[i] += x * ghf;
    }
}

void springs_process(springs_t *restrict springs, float **restrict in,
                     float **restrict out, int count)
{
    for (int n = 0; n < count; ++n) {
        float ylow[MAXSPRINGS], yhigh[MAXSPRINGS];
        loopsprings(i) yhigh[i] = ylow[i] = in[i * NCHANNELS / NSPRINGS][n];

        springs_lowdelayline(springs, ylow);
        springs_lowdc(springs, ylow);
        springs_lowallpasschain(springs, ylow);

        float ylowin[MAXSPRINGS];
        loopsprings(i) ylowin[i] = ylow[i];
        springs_loweq(springs, ylow);
        springs_lowlpf(springs, ylow);

        /* high chirps */
        springs_highdelayline(springs, yhigh);
        springs_highallpasschain(springs, yhigh);

        /* feed delaylines */
        loopsprings(i) springs->lowdelay1[springs->lowdelay1id][i] =
            ylowin[i] + ghigh2low * yhigh[i];
        springs->lowdelay1id = (springs->lowdelay1id + 1) & LOWDELAY1MASK;

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
