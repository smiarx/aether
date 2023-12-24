#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "springreverb.h"

#define loopsprings(i) for (int i = 0; i < CSPRINGS; ++i)

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

/* compute low all pass chain */
void springs_lowallpasschain(springs_t *restrict springs, float *restrict y)
{
    /* low allpass filter chain */
    int idx[NSPRINGS];
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

void springs_process(springs_t *restrict springs, float **restrict in,
                     float **restrict out, int count)
{
    for (int n = 0; n < count; ++n) {
        float y[NSPRINGS];
        loopsprings(i) y[i] = in[i * NCHANNELS / NSPRINGS][n];

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
        loopsprings(i)
        {

#define tap(name, NAME)                                                      \
    int idelay##name   = delay##name;                                        \
    float fdelay##name = delay##name - (float)idelay##name;                  \
    int idx##name =                                                          \
        (springs->lowdelay##name##id - idelay##name) & LOWDELAY##NAME##MASK; \
    float val0##name = springs->lowdelay##name[idx##name][i];                \
    float val1##name =                                                       \
        springs->lowdelay##name[(idx##name - 1) & LOWDELAY##NAME##MASK][i];  \
    float tap##name = val0##name + (val1##name - val0##name) * fdelay##name

            float delay1 = springs->L1[i] + gmod * springs->Lmodmem[i];
            tap(1, 1);
            springs->lowdelayecho[springs->lowdelayechoid][i] =
                tap1 * (1.f - gecho);

            float delayecho = fdelay1 + springs->Lecho[i];
            tap(echo, ECHO);
            tapecho += tap1 * gecho;
            springs->lowdelayripple[springs->lowdelayrippleid][i] =
                tapecho * (1.f - gripple);

            float delayripple = fdelayecho + springs->Lripple[i];
            tap(ripple, RIPPLE);
            tapripple += tapecho * gripple;

            y[i] += tapripple * glf;
#undef tap
        }
        /* advance buffer ids */
        springs->lowdelay1id = (springs->lowdelay1id + 1) & LOWDELAY1MASK;
        springs->lowdelayechoid =
            (springs->lowdelayechoid + 1) & LOWDELAYECHOMASK;
        springs->lowdelayrippleid =
            (springs->lowdelayrippleid + 1) & LOWDELAYRIPPLEMASK;

        springs_lowallpasschain(springs, y);

        /* feed delayline */
        loopsprings(i) { springs->lowdelay1[springs->lowdelay1id][i] = y[i]; }

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
        loopsprings(i)
        {
            int id                        = springs->lowpassmemid;
            springs->lowpassmem[0][id][i] = y[i];

            for (int j = 0; j < LOWPASSN2ND; ++j) {
                float a_acc = 0, b_acc = 0;
                for (int k = 1; k < 3; ++k)
                    a_acc +=
                        filtersos[j][1][k] *
                        springs
                            ->lowpassmem[j + 1][(id - k) & LOWPASSMEMMASK][i];
                for (int k = 0; k < 3; ++k)
                    b_acc +=
                        filtersos[j][0][k] *
                        springs->lowpassmem[j][(id - k) & LOWPASSMEMMASK][i];
                springs->lowpassmem[j + 1][id][i] = b_acc - a_acc;
            }
            y[i] = springs->lowpassmem[LOWPASSN2ND][id][i];
        }
        springs->lowpassmemid = (springs->lowpassmemid + 1) & LOWPASSMEMMASK;

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
