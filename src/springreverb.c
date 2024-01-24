#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "springreverb.h"

#define loopsprings(i) for (int i = 0; i < NSPRINGS; ++i)

/* springs */

void springs_init(springs_t *springs, springs_desc_t *desc, float samplerate)
{
    memset(springs, 0, sizeof(springs_t));
    loopsprings(i) springs->randseed[i] = rand();
    springs->desc                       = *desc;
    springs->samplerate                 = samplerate;

    springs_set_ftr(springs, springs->desc.ftr);
    springs_set_a1(springs, springs->desc.a1);
    springs_set_Nripple(springs, 0.5);
    springs_set_Td(springs, springs->desc.Td);
    springs_set_glf(springs, springs->desc.glf);
    springs_set_ghf(springs, springs->desc.ghf);
    springs_set_vol(springs, springs->desc.vol);
    springs_set_hilomix(springs, springs->desc.hilomix);

    float fcutoff[] = {20, 20, 20, 20, 20, 20, 20, 20};
    springs_set_dccutoff(springs, fcutoff);
    float gripple[] = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01};
    springs_set_gripple(springs, gripple);
    float gecho[] = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01};
    springs_set_gecho(springs, gecho);
}

void springs_update(springs_t *springs, springs_desc_t *desc)
{
#define param_update(name)                                             \
    {                                                                  \
        int test = 0;                                                  \
        loopsprings(i) test |= springs->desc.name[i] != desc->name[i]; \
        if (test) springs_set_##name(springs, desc->name);             \
    }

    param_update(ftr);
    param_update(Td);
    param_update(a1);
    param_update(ahigh);
    param_update(gripple);
    param_update(gecho);
    param_update(glf);
    param_update(ghf);
    param_update(vol);
    param_update(hilomix);

#undef param_update
}

void springs_set_dccutoff(springs_t *springs,
                          float fcutoff[restrict MAXSPRINGS])
{
    loopsprings(i)
    {
        springs->desc.fcutoff[i] = fcutoff[i];
        springs->adc[i] =
            tanf(M_PI / 4.f - M_PI * fcutoff[i] / springs->samplerate);
    }
}

void springs_set_ftr(springs_t *springs, float ftr[restrict MAXSPRINGS])
{
    loopsprings(i) springs->desc.ftr[i] = ftr[i];

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
        if (Mtmp == 0) Mtmp = 1;
        if (Mtmp < M) M = Mtmp;
    }

    if (M != springs->downsampleM) {
        float aafreq[MAXSPRINGS];

        springs->downsampleM     = M;
        springs->downsampleid    = 0;
        loopsprings(i) aafreq[i] = springs->samplerate * 0.5f / M;

        /* scipy.signal.cheby1(10,2,1,analog=True,output='sos') */
        const float analogaasos[][2][3] = {
            {{0., 0., 0.00255383}, {1., 0.21436212, 0.0362477}},
            {{0., 0., 1.}, {1., 0.19337886, 0.21788333}},
            {{0., 0., 1.}, {1., 0.15346633, 0.51177596}},
            {{0., 0., 1.}, {1., 0.09853145, 0.80566858}},
            {{0., 0., 1.}, {1., 0.03395162, 0.98730422}}};
        filter(_sos_analog)(springs->aafilter, analogaasos, aafreq,
                            springs->samplerate, NAASOS);

        /* other parameters are dependent of downsample M */
        springs_set_dccutoff(springs, springs->desc.fcutoff);
        springs_set_Td(springs, springs->desc.Td);
        springs_set_a1(springs, springs->desc.a1);
    }

    float samplerate = springs->samplerate / (float)M;
    loopsprings(i)
    {
        springs->K[i] /= (float)M;
        int iK   = springs->K[i] - .5f;
        float fd = springs->K[i] - iK;
        float a2 = (1 - fd) / (1 + fd);

        springs->low_cascade.iK[i] = iK;
        springs->low_cascade.a2[i] = a2;

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
    filter(_sos_analog)(springs->lowpassfilter, analoglowpasssos, ftr,
                        springs->samplerate, NLOWPASSSOS);
}

void springs_set_a1(springs_t *springs, float a1[restrict MAXSPRINGS])
{
    loopsprings(i)
    {
        springs->desc.a1[i] = a1[i];
        if (springs->low_cascade.iK[i] <= 1) {
            /* revert back to classic 2nd order allpass filter */
            springs->low_cascade.iK[i] = 1;
            springs->low_cascade.a2[i] =
                -2.f * sqrtf(a1[i]) * cosf(M_PI / springs->K[i]) / (1 + a1[i]);
        }
        springs->low_cascade.a1[i] = a1[i];
    }
}

void springs_set_ahigh(springs_t *springs, float ahigh[restrict MAXSPRINGS])
{
    loopsprings(i) springs->desc.ahigh[i] = springs->ahigh[i] = ahigh[i];
}

void springs_set_Td(springs_t *springs, float Td[restrict MAXSPRINGS])
{
    float samplerate = springs->samplerate / (float)springs->downsampleM;
    loopsprings(i)
    {
        springs->desc.Td[i] = Td[i];
        float a1            = springs->low_cascade.a1[i];
        float L =
            fmaxf(0.f, Td[i] * samplerate -
                           springs->K[i] * LOW_CASCADE_N * (1 - a1) / (1 + a1));
        springs->Lecho[i] = L / 5;
        springs->L1[i]    = L - springs->Lecho[i] - springs->Lripple[i];

        float Lhigh       = L / 1.8f * (float)springs->downsampleM;
        springs->Lhigh[i] = Lhigh;
    }

    /* find block size */
    int blocksize = MAXBLOCKSIZE;
    loopsprings(i) if (springs->Lhigh[i] < blocksize) blocksize =
        (int)springs->Lhigh[i];
    springs->blocksize = blocksize;
}

void springs_set_Nripple(springs_t *springs, float Nripple)
{
    loopsprings(i) { springs->Lripple[i] = 2.f * springs->K[i] * Nripple; }
}

#define gfunc(gname)                                                          \
    void springs_set_##gname(springs_t *springs,                              \
                             float gname[restrict MAXSPRINGS])                \
    {                                                                         \
        loopsprings(i) springs->desc.gname[i] = springs->gname[i] = gname[i]; \
    }

gfunc(gripple) gfunc(gecho) gfunc(glf) gfunc(ghf)
#undef gfunc

#define set_glow_ghigh(db, ratio)                                                \
    {                                                                            \
        float gain        = powf(10.f, db / 20.f);                               \
        float ghigh       = atanf(5.f * (ratio - 0.5)) / atanf(5.f) / 2.f + .5f; \
        float glow        = 1.f - ghigh;                                         \
        springs->glow[i]  = 20.f * glow * gain;                                  \
        springs->ghigh[i] = ghigh * gain;                                        \
    }

    void springs_set_vol(springs_t *springs, float vol[restrict MAXSPRINGS])
{
    loopsprings(i)
    {
        springs->desc.vol[i] = vol[i];
        float db             = vol[i];
        float ratio          = springs->desc.hilomix[i];
        set_glow_ghigh(db, ratio);
    }
}
void springs_set_hilomix(springs_t *springs, float hilomix[restrict MAXSPRINGS])
{
    loopsprings(i)
    {
        springs->desc.hilomix[i] = hilomix[i];
        float db                 = springs->desc.vol[i];
        float ratio              = hilomix[i];
        set_glow_ghigh(db, ratio);
    }
}
#undef set_glow_ghigh

void springs_lowdelayline(springs_t *restrict springs,
                          float y[restrict MAXSPRINGS])
{
    y = __builtin_assume_aligned(y, sizeof(float) * MAXSPRINGS);

/* delay modulation */
#pragma omp simd
    loopsprings(i)
    {
        springs->randstate[i] =
            springs->randseed[i] + springs->randstate[i] * 1103515245;
        float mod = springs->randstate[i] / ((float)INT_MAX);
        // filter modulation
        springs->Lmodmem[i] = mod += amod * (springs->Lmodmem[i] - mod);
    }
    /* tap low delayline */
#define delayvecs(name)                                   \
    int idx##name[MAXSPRINGS], idx##name##m1[MAXSPRINGS]; \
    float fdelay##name[MAXSPRINGS]
    delayvecs(1);
    delayvecs(echo);
    delayvecs(ripple);
#undef delayvecs
#pragma omp simd
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

        float delay1 =
            springs->L1[i] + gmod * springs->Lmodmem[i] / springs->downsampleM;
        setidx(1, 1);
        float delayecho = fdelay1[i] + springs->Lecho[i];
        setidx(echo, ECHO);
        float delayripple = fdelayecho[i] + springs->Lripple[i];
        setidx(ripple, RIPPLE);
#undef setidx
    }

#pragma omp simd
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

void springs_lowdc(springs_t *restrict springs, float y[restrict MAXSPRINGS])
{
    y = __builtin_assume_aligned(y, sizeof(float) * MAXSPRINGS);

#pragma omp simd
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
void low_cascade_process(struct low_cascade *lc, float y[restrict MAXSPRINGS])
{
    y = __builtin_assume_aligned(y, sizeof(float) * MAXSPRINGS);

    // load mem id
    int idx[MAXSPRINGS];
#pragma omp simd
    loopsprings(i) idx[i] = (lc->id - lc->iK[i]) & LOW_CASCADE_STATE_MASK;

    // load parameters
    float a1[MAXSPRINGS], a2[MAXSPRINGS];
#pragma omp simd
    loopsprings(i) a1[i] = lc->a1[i], a2[i] = lc->a2[i];

    for (int j = 0; j < LOW_CASCADE_N; ++j) {
        float s1mem[MAXSPRINGS];
        loopsprings(i) s1mem[i] = lc->state[j].s1[idx[i]][i];
#pragma omp simd
        loopsprings(i)
        {
            /* compute internal allpass1 */
            float s1 = y[i] - a1[i] * s1mem[i];
            y[i]     = a1[i] * s1 + s1mem[i];

            /* compute allpass2 */
            float s2mem = lc->state[j].s2[i];
            float s2    = s1 - a2[i] * s2mem;
            float y2    = a2[i] * s2 + s2mem;

            lc->state[j].s2[i]         = s2;
            lc->state[j].s1[lc->id][i] = y2;
        }
    }
    lc->id = (lc->id + 1) & LOW_CASCADE_STATE_MASK;
}

__attribute__((flatten)) void springs_lowlpf(springs_t *restrict springs,
                                             float y[restrict MAXSPRINGS])
{
    filter(_process)(springs->lowpassfilter, y, NLOWPASSSOS);
}

void springs_loweq(springs_t *restrict springs, float y[restrict MAXSPRINGS])
{
    y = __builtin_assume_aligned(y, sizeof(float) * MAXSPRINGS);

    int id = springs->loweq.id;
    int idk[MAXSPRINGS];
    int id2k[MAXSPRINGS];
#pragma omp simd
    loopsprings(i)
    {
        idk[i]  = (id - springs->loweq.Keq[i]) & MLOWEQMASK;
        idk[i]  = idk[i] * MAXSPRINGS + i;
        id2k[i] = (id - springs->loweq.Keq[i] * 2) & MLOWEQMASK;
        id2k[i] = id2k[i] * MAXSPRINGS + i;
    }
    float *loweqmem = (float *)springs->loweq.mem;
#pragma omp simd
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

void springs_highallpasschain(springs_t *restrict springs,
                              float y[restrict MAXSPRINGS])
{
    y = __builtin_assume_aligned(y, sizeof(float) * MAXSPRINGS);
    /* high chirp allpass chain is stretched by a factor of two,
     * this isn't the way it's supposed to be but it sounds better so ehh..
     */
    int id = springs->highmemid;
    for (int j = 0; j < MHIGH; ++j) {
#pragma omp simd
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

void springs_highdelayline(springs_t *restrict springs,
                           float y[restrict MAXSPRINGS])
{
    y = __builtin_assume_aligned(y, sizeof(float) * MAXSPRINGS);

    int idx0[MAXSPRINGS], idx1[MAXSPRINGS];
    float fLhigh[MAXSPRINGS];

/* delay modulation */
#pragma omp simd
    loopsprings(i)
    {
        springs->randstate[i] =
            springs->randseed[i] + springs->randstate[i] * 1103515256;
        float mod = springs->randstate[i] / ((float)INT_MAX);
        // filter modulation
        springs->Lmodhighmem[i] = mod += amod * (springs->Lmodhighmem[i] - mod);
    }

#pragma omp simd
    loopsprings(i)
    {
        float Lhigh = springs->Lhigh[i] + gmod * springs->Lmodhighmem[i];
        int iLhigh  = (int)Lhigh;
        fLhigh[i]   = Lhigh - iLhigh;
        idx0[i]     = (springs->highdelayid - iLhigh) & HIGHDELAYMASK;
        idx1[i]     = (idx0[i] - 1) & HIGHDELAYMASK;

        idx0[i] = idx0[i] * MAXSPRINGS + i;
        idx1[i] = idx1[i] * MAXSPRINGS + i;
    }

    float *delayline = (float *)&springs->highdelay[0][0];
#pragma omp simd
    loopsprings(i)
    {
        float x0 = delayline[idx0[i]];
        float x1 = delayline[idx1[i]];
        float x  = x0 + fLhigh[i] * (x1 - x0);

        y[i] += x * springs->ghf[i];
    }
}

#define loopsamples(n) for (int n = 0; n < blocksize; ++n)
#define loopdownsamples(n) \
    for (int n = downsamplestart; n < blocksize; n += springs->downsampleM)

// only flatten in clang, gcc seems to break vectorization
__attribute__((flatten)) void springs_process(springs_t *restrict springs,
                                              float **restrict in,
                                              float **restrict out, int count)
{
    /* springs */
    float(*y)[MAXSPRINGS]     = springs->ylow;
    float(*ylow)[MAXSPRINGS]  = springs->ylow;
    float(*yhigh)[MAXSPRINGS] = springs->yhigh;

    int nbase = 0;

    while (count) {
        // block size to process
        int blocksize = count < springs->blocksize ? count : springs->blocksize;

        // when do the downsampled samples start ?
        int downsamplestart = (springs->downsampleM - springs->downsampleid) %
                              springs->downsampleM;

        // load value and aa filter for downsampling
        if (springs->downsampleM > 1) {
            loopsamples(n)
            {
                float ylowin[MAXSPRINGS];
#pragma omp simd
                loopsprings(i) ylowin[i] = yhigh[n][i] =
                    in[i * NCHANNELS / NSPRINGS][n];
                // aa filter
                filter(_process)(springs->aafilter, ylowin, NAASOS);

                if (springs->downsampleid == 0)
#pragma omp simd
                    loopsprings(i) ylow[n][i] =
                        ylowin[i] * (float)springs->downsampleM;
                else
#pragma omp simd
                    loopsprings(i) ylow[n][i] = 0.f;

                springs->downsampleid =
                    (springs->downsampleid + 1) % springs->downsampleM;
            }
        } else {
            loopsamples(n)
            {
#pragma omp simd
                loopsprings(i) ylow[n][i] = yhigh[n][i] =
                    in[i * NCHANNELS / NSPRINGS][n];
            }
        }

        /* low chirps */
        // get delay tap
        int lowdelay1id = springs->lowdelay1id;
        loopdownsamples(n)
        {
            springs_lowdelayline(springs, ylow[n]);
            springs->lowdelay1id = (springs->lowdelay1id + 1) & LOWDELAY1MASK;
        }
        springs->lowdelay1id = lowdelay1id;

        // dc filter
        loopdownsamples(n) springs_lowdc(springs, ylow[n]);
        // allpass cascade
        loopdownsamples(n) low_cascade_process(&springs->low_cascade, ylow[n]);

        // feed delayline
        loopdownsamples(n)
        {
#pragma omp simd
            loopsprings(i) springs->lowdelay1[springs->lowdelay1id][i] =
                ylow[n][i]; // + ghigh2low*yhigh[i];
            springs->lowdelay1id = (springs->lowdelay1id + 1) & LOWDELAY1MASK;
        }

        // low chirps equalize
        loopdownsamples(n) springs_loweq(springs, ylow[n]);

        // filter higher freq, also for interpolation
        loopsamples(n) springs_lowlpf(springs, ylow[n]);

        /* high chirps */
        // get delay tap
        int highdelayid = springs->highdelayid;
        loopsamples(n)
        {
            springs_highdelayline(springs, yhigh[n]);
            springs->highdelayid = (springs->highdelayid + 1) & HIGHDELAYMASK;
        }
        springs->highdelayid = highdelayid;

        // allpass cascade
        loopsamples(n) springs_highallpasschain(springs, yhigh[n]);

        // feed delayline
        loopsamples(n)
        {
#pragma omp simd
            loopsprings(i) springs->highdelay[springs->highdelayid][i] =
                yhigh[n][i] + glow2high * ylow[n][i];
            springs->highdelayid = (springs->highdelayid + 1) & HIGHDELAYMASK;
        }

        // sum high and low
        loopsamples(n)
#pragma omp simd
            loopsprings(i) y[n][i] =
                springs->glow[i] * ylow[n][i] + springs->ghigh[i] * yhigh[n][i];

        /* sum springs */
        loopsamples(n)
        {
            for (int c = 0; c < NCHANNELS; ++c) {
                int offset = c * NSPRINGS / NCHANNELS;
                for (int i = NSPRINGS / 2 / NCHANNELS; i > 0; i /= 2) {
                    for (int j = 0; j < i; ++j)
                        y[n][offset + j] += y[n][offset + j + i];
                }
                out[c][nbase + n] = y[n][offset];
            }
        }

        nbase += blocksize;
        count -= blocksize;
    }
}
