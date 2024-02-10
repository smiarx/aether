#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fastmath.h"
#include "springreverb.h"

#define loopsprings(i) for (int i = 0; i < NSPRINGS; ++i)

// inc macros
#define setinc(param, new, i) (param).inc[i] = (new - (param).val[i]) / count
#define addinc(param, i)      (param).val[i] += (param).inc[i]
#define resetinc(param)       loopsprings(i)(param).inc[i] = 0.f

/* set int & frac delay values */
#pragma omp declare simd linear(i) uniform(tap)
static inline void tap_set_delay(struct delay_tap *tap, float delay, int i)
{
    int idelay     = delay;
    float fdelay   = delay - (float)idelay;
    tap->idelay[i] = idelay;
    tap->fdelay[i] = fdelay;
}

/* springs */

void springs_init(springs_t *springs, springs_desc_t *desc, float samplerate)
{
    memset(springs, 0, sizeof(springs_t));
    loopsprings(i) springs->rand.seed[i] = rand();
    springs->desc                        = *desc;
    springs->samplerate                  = samplerate;

    springs_set_ftr(springs, springs->desc.ftr);
    springs_set_a1(springs, springs->desc.a1);
    springs_set_Nripple(springs, 0.5);
    springs_set_Td(springs, springs->desc.Td);
    springs_set_glf(springs, springs->desc.glf);
    springs_set_ghf(springs, springs->desc.ghf);
    springs_set_vol(springs, springs->desc.vol, 1);
    springs_set_pan(springs, springs->desc.pan, 1);
    springs_set_drywet(springs, springs->desc.drywet, 1);
    springs_set_hilomix(springs, springs->desc.hilomix, 1);

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

#undef param_update
}

void springs_set_dccutoff(springs_t *springs,
                          float fcutoff[restrict MAXSPRINGS])
{
    loopsprings(i)
    {
        springs->desc.fcutoff[i] = fcutoff[i];
        springs->low_dc.a[i] =
            tanf(M_PI / 4.f - M_PI * fcutoff[i] / springs->samplerate);
        springs->low_dc.b[i] = (1.f + springs->low_dc.a[i]) / 2.f;
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
        springs->low_eq.Keq[i] = Keq;
        springs->low_eq.b0[i]  = (1 - R * R) / 2.f;
        springs->low_eq.ak[i]  = -2.f * R * cos0;
        springs->low_eq.a2k[i] = R * R;
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
    loopsprings(i) springs->desc.ahigh[i] = springs->high_cascade.a[i] =
        ahigh[i];
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

        struct low_delayline *ldl = &springs->low_delayline;
        float Lecho               = L / 5.f;
        float Lripple = ldl->tap_ripple.idelay[i] + ldl->tap_ripple.fdelay[i];
        float L1      = L - Lecho - Lripple;

        tap_set_delay(&ldl->tap_echo, L / 5, i);
        ldl->L1[i] = L1;

        struct high_delayline *hdl = &springs->high_delayline;
        float Lhigh                = L / 1.8f * (float)springs->downsampleM;
        hdl->L[i]                  = Lhigh;
    }

    /* find block size */
    int blocksize = MAXBLOCKSIZE;
    loopsprings(i) if (springs->high_delayline.L[i] < blocksize) blocksize =
        (int)springs->high_delayline.L[i];
    springs->blocksize = blocksize;
}

void springs_set_Nripple(springs_t *springs, float Nripple)
{
    loopsprings(i)
    {
        float Lripple = 2.f * springs->K[i] * Nripple;
        tap_set_delay(&springs->low_delayline.tap_ripple, Lripple, i);
    }
}

#define gfunc(gname, part)                                     \
    void springs_set_##gname(springs_t *springs,               \
                             float gname[restrict MAXSPRINGS]) \
    {                                                          \
        loopsprings(i) springs->desc.gname[i] =                \
            springs->part##_delayline.gname[i] = gname[i];     \
    }

gfunc(gripple, low) gfunc(gecho, low) gfunc(glf, low) gfunc(ghf, high)
#undef gfunc

    void springs_set_vol(springs_t *springs, float vol[restrict MAXSPRINGS],
                         int count)
{
    loopsprings(i)
    {
        float db = springs->desc.vol[i] = vol[i];
        float ratio          = springs->desc.hilomix[i];
        float gain                      = powf(10.f, db / 20.f);
        float ghigh =
            atanf(5.f * (ratio - 0.5)) / atanf(5.f) / 2.f + .5f; // TODO change
        float glow = 1.f - ghigh;
        ghigh *= gain;
        glow *= gain;
        setinc(springs->glow, glow, i);
        setinc(springs->ghigh, ghigh, i);
    }
    springs->increment_glowhigh = 1;
}
void springs_set_hilomix(springs_t *springs, float hilomix[restrict MAXSPRINGS],
                         int count)
{
    loopsprings(i) { springs->desc.hilomix[i] = hilomix[i]; }
    springs_set_vol(springs, springs->desc.vol, count);
}

void springs_set_pan(springs_t *springs, float pan[restrict MAXSPRINGS],
                     int count)
{
    loopsprings(i)
    {
        springs->desc.pan[i] = pan[i];
#if NCHANNELS == 2
        float theta             = (1.f + pan[i]) * M_PI / 4.f;
        float gleft             = cosf(theta);
        float gright            = sinf(theta);

        setinc(springs->gchannel[0], gleft, i);
        setinc(springs->gchannel[1], gright, i);
#endif
    }
    springs->increment_gchannel = 1;
}

void springs_set_drywet(springs_t *springs, float drywet, int count)
{
    springs->desc.drywet = drywet;
    if (springs->drywet != springs->desc.drywet) {
        springs->drywet_inc = (springs->desc.drywet - springs->drywet) / count;
    }
}

#pragma omp declare simd linear(i) uniform(rd)
static const float rand_get(struct rand *rd, int i)
{
    rd->state[i] = rd->seed[i] + rd->state[i] * 1103515245;
    return rd->state[i] / ((float)INT_MAX);
}

/* delay line */
#pragma omp declare simd linear(i) uniform(tap, buffer, mask)
static inline float tap_linear(struct delay_tap *tap,
                               float buffer[][MAXSPRINGS], int mask, int i)
{
    int id  = (tap->id - tap->idelay[i]) & mask;
    int id1 = (id - 1) & mask;
    id      = (id * MAXSPRINGS) + i;
    id1     = (id1 * MAXSPRINGS) + i;

    float x0 = buffer[0][id];
    float x1 = buffer[0][id1];

    return x0 + tap->fdelay[i] * (x1 - x0);
}

#pragma omp declare simd linear(i) uniform(tap, buffer, mask)
static inline float tap_cubic(struct delay_tap *tap, float buffer[][MAXSPRINGS],
                              int mask, int i)
{
    int id   = (tap->id - tap->idelay[i]) & mask;
    int id1  = (id - 1) & mask;
    int id2  = (id - 2) & mask;
    int idm1 = (id + 1) & mask;
    id       = (id * MAXSPRINGS) + i;
    id1      = (id1 * MAXSPRINGS) + i;
    id2      = (id2 * MAXSPRINGS) + i;
    idm1     = (idm1 * MAXSPRINGS) + i;

    float y0  = buffer[0][id];
    float y1  = buffer[0][id1];
    float y2  = buffer[0][id2];
    float ym1 = buffer[0][idm1];

    return hermite(ym1, y0, y1, y2, tap->fdelay[i]);
}

void low_delayline_process(struct low_delayline *restrict dl,
                           struct rand *restrict rd,
                           float y[restrict MAXSPRINGS])
{
    y = __builtin_assume_aligned(y, sizeof(float) * MAXSPRINGS);

#pragma omp simd
    loopsprings(i)
    {
        /* modulation */
        float mod       = rand_get(rd, i);
        dl->modstate[i] = mod += amod * (dl->modstate[i] - mod);

        // todo gmod should be divised by downsampleM
        tap_set_delay(&dl->tap1, dl->L1[i] + mod * gmod, i);
    }
#pragma omp simd
    loopsprings(i)
    {
        float tap1 = tap_cubic(&dl->tap1, dl->buffer1, LOWDELAY1MASK, i);

        dl->buffer_echo[dl->tap_echo.id][i] = tap1 * (1.f - dl->gecho[i]);
        float tap_echo =
            tap_linear(&dl->tap_echo, dl->buffer_echo, LOWDELAYECHOMASK, i);
        tap_echo += tap1 * dl->gecho[i];

        dl->buffer_ripple[dl->tap_ripple.id][i] =
            tap_echo * (1.f - dl->gripple[i]);
        float tap_ripple = tap_linear(&dl->tap_ripple, dl->buffer_ripple,
                                      LOWDELAYRIPPLEMASK, i);
        tap_ripple += tap_echo * dl->gripple[i];

        y[i] += tap_ripple * dl->glf[i];
    }

    /* increment buffer ids */
    dl->tap1.id       = (dl->tap1.id + 1) & LOWDELAY1MASK;
    dl->tap_echo.id   = (dl->tap_echo.id + 1) & LOWDELAYECHOMASK;
    dl->tap_ripple.id = (dl->tap_ripple.id + 1) & LOWDELAYRIPPLEMASK;
}

void high_delayline_process(struct high_delayline *restrict dl,
                            struct rand *restrict rd,
                            float y[restrict MAXSPRINGS])
{
    y = __builtin_assume_aligned(y, sizeof(float) * MAXSPRINGS);

#pragma omp simd
    loopsprings(i)
    {
        /* modulation */
        float mod       = rand_get(rd, i);
        dl->modstate[i] = mod += amod * (dl->modstate[i] - mod);

        // todo gmod should be divised by downsampleM
        tap_set_delay(&dl->tap, dl->L[i] + mod * gmod, i);
    }
#pragma omp simd
    loopsprings(i)
    {
        float tap = tap_linear(&dl->tap, dl->buffer, HIGHDELAYMASK, i);

        y[i] += tap * dl->ghf[i];
    }

    /* increment buffer ids */
    dl->tap.id = (dl->tap.id + 1) & HIGHDELAYMASK;
}

void low_dc_process(struct low_dc *dc, float y[restrict MAXSPRINGS])
{
    y = __builtin_assume_aligned(y, sizeof(float) * MAXSPRINGS);

#pragma omp simd
    loopsprings(i)
    {
        /* update filter state */
        float s1 = dc->state[i];
        float s0 = y[i] + dc->a[i] * s1;
        /* set output */
        y[i] = dc->b[i] * (s0 - s1);

        dc->state[i] = s0;
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

void low_eq_process(struct low_eq *le, float y[restrict MAXSPRINGS])
{
    y = __builtin_assume_aligned(y, sizeof(float) * MAXSPRINGS);

    int idk[MAXSPRINGS];
    int id2k[MAXSPRINGS];
#pragma omp simd
    loopsprings(i)
    {
        idk[i]  = (le->id - le->Keq[i]) & LOW_EQ_STATE_MASK;
        idk[i]  = idk[i] * MAXSPRINGS + i;
        id2k[i] = (le->id - le->Keq[i] * 2) & LOW_EQ_STATE_MASK;
        id2k[i] = id2k[i] * MAXSPRINGS + i;
    }
    float *state = &le->state[0][0];
#pragma omp simd
    loopsprings(i)
    {
        float b0  = le->b0[i];
        float ak  = le->ak[i];
        float a2k = le->a2k[i];

        float sk  = state[idk[i]];
        float s2k = state[id2k[i]];
        float s0  = y[i] - ak * sk - a2k * s2k;

        le->state[le->id][i] = s0;
        y[i]                 = b0 * (s0 - s2k);
    }
    le->id = (le->id + 1) & LOW_EQ_STATE_MASK;
}

void high_cascade_process(struct high_cascade *hc, float y[restrict MAXSPRINGS])
{
    y = __builtin_assume_aligned(y, sizeof(float) * MAXSPRINGS);
    /* high chirp allpass chain is stretched by a factor of two,
     * this isn't the way it's supposed to be but it sounds better so ehh..
     */

    float a[MAXSPRINGS];
    loopsprings(i) a[i] = hc->a[i];

    for (int j = 0; j < HIGH_CASCADE_N; ++j) {
#pragma omp simd
        loopsprings(i)
        {
            float sk = hc->state[j][HIGH_CASCADE_STRETCH - 1][i];
            float s0 = y[i] - hc->a[i] * sk;

            y[i] = a[i] * s0 + sk;

            for (int k = HIGH_CASCADE_STRETCH - 1; k > 0; --k)
                hc->state[j][k][i] = hc->state[j][k - 1][i];
            hc->state[j][0][i] = s0;
        }
    }
}

#define loopsamples(n) for (int n = 0; n < blocksize; ++n)
#define loopdownsamples(n) \
    for (int n = downsamplestart; n < blocksize; n += springs->downsampleM)

// only flatten in clang, gcc seems to break vectorization
__attribute__((flatten)) void springs_process(springs_t *restrict springs,
                                              const float *const *in,
                                              float *const *restrict out,
                                              int count)
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
                /* compute sources (left,right,mono)*/
                float sources[NCHANNELS + 1];
                sources[NCHANNELS] = 0.f;
                for (int c = 0; c < NCHANNELS; ++c) {
                    sources[c] = in[c][nbase+n];
                    sources[NCHANNELS] += in[c][nbase+n] / NCHANNELS;
                }

                float ylowin[MAXSPRINGS];
#pragma omp simd
                loopsprings(i) ylowin[i] = yhigh[n][i] =
                    sources[springs->desc.source[i]];
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
                /* compute sources (left,right,mono)*/
                float sources[NCHANNELS + 1];
                sources[NCHANNELS] = 0.f;
                for (int c = 0; c < NCHANNELS; ++c) {
                    sources[c] = in[c][nbase+n];
                    sources[NCHANNELS] += in[c][nbase+n] / NCHANNELS;
                }
#pragma omp simd
                loopsprings(i) ylow[n][i] = yhigh[n][i] =
                    sources[springs->desc.source[i]];
            }
        }

        /* low chirps */
        // get delay tap
        int lowdelay1id = springs->low_delayline.tap1.id;
        loopdownsamples(n)
        {
            low_delayline_process(&springs->low_delayline, &springs->rand,
                                  ylow[n]);
        }
        springs->low_delayline.tap1.id = lowdelay1id;
        // get delay tap
        int highdelayid = springs->high_delayline.tap.id;
        loopsamples(n)
        {
            high_delayline_process(&springs->high_delayline, &springs->rand,
                                   yhigh[n]);
        }
        springs->high_delayline.tap.id = highdelayid;

        // dc filter
        loopdownsamples(n) low_dc_process(&springs->low_dc, ylow[n]);
        // allpass cascade
        loopdownsamples(n) low_cascade_process(&springs->low_cascade, ylow[n]);

        // feed delayline
        loopdownsamples(n)
        {
            struct low_delayline *dl = &springs->low_delayline;
#pragma omp simd
            loopsprings(i) dl->buffer1[dl->tap1.id][i] =
                ylow[n][i]; // + ghigh2low*yhigh[i];
            dl->tap1.id = (dl->tap1.id + 1) & LOWDELAY1MASK;
        }

        // low chirps equalize
        loopdownsamples(n) low_eq_process(&springs->low_eq, ylow[n]);

        // filter higher freq, also for interpolation
        loopsamples(n) springs_lowlpf(springs, ylow[n]);

        /* high chirps */

        // allpass cascade
        loopsamples(n) high_cascade_process(&springs->high_cascade, yhigh[n]);

        // feed delayline
        loopsamples(n)
        {
            struct high_delayline *dl = &springs->high_delayline;
#pragma omp simd
            loopsprings(i) dl->buffer[dl->tap.id][i] =
                yhigh[n][i] + glow2high * ylow[n][i];
            dl->tap.id = (dl->tap.id + 1) & HIGHDELAYMASK;
        }

        // sum high and low
#define sum_hilo(inc)                                                        \
    {                                                                        \
        struct springparam glow  = springs->glow;                            \
        struct springparam ghigh = springs->ghigh;                           \
        loopsamples(n) _Pragma("omp simd") loopsprings(i)                    \
        {                                                                    \
            if (inc) {                                                       \
                addinc(glow, i);                                             \
                addinc(ghigh, i);                                            \
            }                                                                \
            y[n][i] = glow.val[i] * ylow[n][i] + ghigh.val[i] * yhigh[n][i]; \
        }                                                                    \
        if (inc) {                                                           \
            springs->glow  = glow;                                           \
            springs->ghigh = ghigh;                                          \
        }                                                                    \
    }

        if (springs->increment_glowhigh) {
            sum_hilo(1);
        } else {
            sum_hilo(0);
        }

        /* sum springs */
        float drywet;
        const float drywet_inc = springs->drywet_inc;

#define sum_springs(inc_drywet, inc_gchannel)                              \
    {                                                                      \
        for (int c = 0; c < NCHANNELS; ++c) {                              \
            drywet = springs->drywet;                                      \
            loopsamples(n)                                                 \
            {                                                              \
                float ysum = 0.f;                                          \
                _Pragma("omp simd") loopsprings(i)                         \
                {                                                          \
                    if (inc_gchannel) addinc(springs->gchannel[c], i);     \
                    ysum += springs->gchannel[c].val[i] * y[n][i];         \
                }                                                          \
                                                                           \
                if (inc_drywet) drywet += drywet_inc;                      \
                out[c][nbase + n] =                                        \
                    in[c][nbase + n] + drywet * (ysum - in[c][nbase + n]); \
            }                                                              \
        }                                                                  \
        if (inc_drywet) springs->drywet = drywet;                          \
    }

        if (drywet_inc != 0.f) {
            if (springs->increment_gchannel)
                sum_springs(1, 1) else sum_springs(1, 0)
        } else {
            if (springs->increment_gchannel)
                sum_springs(0, 1) else sum_springs(0, 0)
        }
#undef sum_springs

        nbase += blocksize;
        count -= blocksize;
    }

    springs->drywet_inc = 0.f;
    if (springs->increment_gchannel)
        for (int c = 0; c < NCHANNELS; ++c) resetinc(springs->gchannel[c]);
    if (springs->increment_glowhigh) {
        resetinc(springs->glow);
        resetinc(springs->ghigh);
    }
}
