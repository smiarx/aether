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
                     cos(2.f * M_PI * fpeak * Keq / springs->samplerate);
        springs->loweq.Keq[i] = Keq;
        springs->loweq.b0[i]  = (1 - R * R) / 2.f;
        springs->loweq.ak[i]  = -2.f * R * cos0;
        springs->loweq.a2k[i] = R * R;
        ;
    }
}

void springs_set_a1(springs_t *springs, float *a1)
{
    loopsprings(i) springs->a1[i] = a1[i];
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
    springs->lowdelay1id    = (springs->lowdelay1id + 1) & LOWDELAY1MASK;
    springs->lowdelayechoid = (springs->lowdelayechoid + 1) & LOWDELAYECHOMASK;
    springs->lowdelayrippleid =
        (springs->lowdelayrippleid + 1) & LOWDELAYRIPPLEMASK;
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
    const float filtersos[][2][3] = {
        {{2.18740696e-03, 6.16778351e-04, 2.18740696e-03},
         {1.00000000e+00, -1.70313982e+00, 7.48223150e-01}},
        {{1.00000000e+00, -1.31079983e+00, 1.00000000e+00},
         {1.00000000e+00, -1.68138176e+00, 8.53580218e-01}},
        {{1.00000000e+00, -1.55984297e+00, 1.00000000e+00},
         {1.00000000e+00, -1.66400724e+00, 9.39118831e-01}},
        {{1.00000000e+00, -1.62171918e+00, 1.00000000e+00},
         {1.00000000e+00, -1.65694459e+00, 9.78755965e-01}},
        {{1.00000000e+00, -1.63865216e+00, 1.00000000e+00},
         {1.00000000e+00, -1.65708087e+00, 9.94971669e-01}}};
    int id                                       = springs->lowpassmemid;
    loopsprings(i) springs->lowpassmem[0][id][i] = y[i];
    for (int j = 0; j < LOWPASSN2ND; ++j) {
        float acc[MAXSPRINGS] = {0};
        for (int k = 0; k < 3; ++k) loopsprings(i)
            {
                acc[i] += filtersos[j][0][k] *
                          springs->lowpassmem[j][(id - k) & LOWPASSMEMMASK][i];
                if (k > 0)
                    acc[i] -=
                        filtersos[j][1][k] *
                        springs
                            ->lowpassmem[j + 1][(id - k) & LOWPASSMEMMASK][i];
            }
        loopsprings(i) springs->lowpassmem[j + 1][id][i] = acc[i];
    }
    loopsprings(i) y[i]   = springs->lowpassmem[LOWPASSN2ND][id][i];
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

void springs_process(springs_t *restrict springs, float **restrict in,
                     float **restrict out, int count)
{
    for (int n = 0; n < count; ++n) {
        float y[MAXSPRINGS];
        loopsprings(i) y[i] = in[i * NCHANNELS / NSPRINGS][n];

        springs_lowdelayline(springs, y);
        springs_lowallpasschain(springs, y);

        /* feed delayline */
        loopsprings(i) { springs->lowdelay1[springs->lowdelay1id][i] = y[i]; }

        springs_loweq(springs, y);
        springs_lowlpf(springs, y);

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
