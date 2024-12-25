#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fastmath.h"
#include "springreverb.h"

#define NRIPPLE           0.5f
#define T60_HILO_RATIO    0.8f
#define NOISE_FREQ        400.f
#define LENGTH_HILO_RATIO (1.f / 1.8f)

#define loopsprings(i) for (int i = 0; i < NSPRINGS; ++i)

// inc macros
#define setinc(param, new, count, i) \
    (param).inc[i] = (new - (param).val[i]) / count
#define addinc(param, i) (param).val[i] += (param).inc[i]
#define resetinc(param)  loopsprings(i)(param).inc[i] = 0.f

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
void springs_init(springs_t *springs, springs_desc_t *desc, float samplerate,
                  int count)
{
    memset(springs, 0, sizeof(springs_t));
    loopsprings(i) springs->low_delayline.noise.seed[i]  = rand();
    loopsprings(i) springs->high_delayline.noise.seed[i] = rand();
    springs->desc                                        = *desc;
    springs->samplerate                                  = samplerate;

    springs_set_stages(springs, springs->desc.stages);
    springs_set_ftr(springs, springs->desc.ftr, count);
    springs_set_a1(springs, springs->desc.a1, count);
    springs_set_ahigh(springs, springs->desc.ahigh, count);
    springs_set_length(springs, springs->desc.length, count);
    springs_set_t60(springs, springs->desc.t60, count);
    springs_set_chaos(springs, springs->desc.chaos, count);
    springs_set_vol(springs, springs->desc.vol, count);
    springs_set_pan(springs, springs->desc.pan, count);
    springs_set_drywet(springs, springs->desc.drywet, count);
    springs_set_hilomix(springs, springs->desc.hilomix, count);

    float fcutoff[] = {20, 20, 20, 20, 20, 20, 20, 20};
    springs_set_dccutoff(springs, fcutoff);
    float gripple[] = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01};
    springs_set_gripple(springs, gripple);
    float gecho[] = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01};
    springs_set_gecho(springs, gecho);

    loopsprings(i)
    {
        springs->high_delayline.noise.freq = NOISE_FREQ / samplerate;
    }
}

void springs_update(springs_t *springs, springs_desc_t *desc)
{
#define param_update(name)                                             \
    {                                                                  \
        int test = 0;                                                  \
        loopsprings(i) test |= springs->desc.name[i] != desc->name[i]; \
        if (test) springs_set_##name(springs, desc->name);             \
    }

    param_update(gripple);
    param_update(gecho);

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

void springs_set_ftr(springs_t *springs, float ftr[restrict MAXSPRINGS],
                     int count)
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
        springs_set_length(springs, springs->desc.length, count);
        springs_set_a1(springs, springs->desc.a1, count);

        /* low delayline noise freq */
        springs->low_delayline.noise.freq =
            NOISE_FREQ / springs->samplerate * M;
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

void springs_set_stages(springs_t *restrict springs,
                        unsigned int stages[MAXSPRINGS])
{
    loopsprings(i)
    {
        springs->desc.stages[i] = stages[i];
        springs->low_cascade.stages[i] =
            stages[i] > LOW_CASCADE_N ? LOW_CASCADE_N : stages[i];
    }

    unsigned int max = 0;
    unsigned int min = UINT_MAX;
    loopsprings(i)
    {
        unsigned int stage = springs->low_cascade.stages[i];
        if (max < stage) max = stage;
        if (min > stage) min = stage;
    }
    springs->low_cascade.max_stages = max;
    springs->low_cascade.min_stages = min - 1;
}

void springs_set_a1(springs_t *springs, float a1[restrict MAXSPRINGS],
                    int count)
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
        setinc(springs->low_cascade.a1, a1[i], count, i);
    }
    springs->increment_a1 = INCREMENT;
}

void springs_set_ahigh(springs_t *springs, float ahigh[restrict MAXSPRINGS],
                       int count)
{
    loopsprings(i)
    {
        springs->desc.ahigh[i] = ahigh[i];
        setinc(springs->high_cascade.a, ahigh[i], count, i);
    }
    springs->increment_ahigh = INCREMENT;
}

/* set spring length (changes time delay) */
void springs_set_length(springs_t *springs, float length[restrict MAXSPRINGS],
                        int count)
{
    float Lhigh[MAXSPRINGS];

    float samplerate = springs->samplerate / (float)springs->downsampleM;
    loopsprings(i)
    {
        springs->desc.length[i] = length[i];
        float time_delay        = length[i];
        float a1                = springs->low_cascade.a1.val[i];
        float L =
            fmaxf(0.f, time_delay * samplerate -
                           springs->K[i] * LOW_CASCADE_N * (1 - a1) / (1 + a1));

        struct low_delayline *ldl = &springs->low_delayline;
        float Lecho               = L / 5.f;
        float Lripple             = 2.f * springs->K[i] * NRIPPLE;
        float L1                  = L - Lecho - Lripple;

        struct high_delayline *hdl = &springs->high_delayline;
        Lhigh[i] = L * LENGTH_HILO_RATIO * (float)springs->downsampleM;
        setinc(hdl->L, Lhigh[i], count, i);

        /* low delay line is downsampled */
        {
            int countM = count / springs->downsampleM;
            countM     = countM < 1 ? 1 : countM;
            setinc(ldl->L1, L1, countM, i);
            setinc(ldl->Lecho, Lecho, countM, i);
            setinc(ldl->Lripple, Lripple, countM, i);
        }
    }

    springs->increment_delaytime = INCREMENT;

    /* update t60 */
    springs_set_t60(springs, springs->desc.t60, count);
    springs_set_chaos(springs, springs->desc.chaos, count);

    /* find block size */
    int blocksize                                      = MAXBLOCKSIZE;
    loopsprings(i) if (Lhigh[i] < blocksize) blocksize = (int)Lhigh[i];
    springs->blocksize                                 = blocksize;
}

#define gfunc(gname, part)                                     \
    void springs_set_##gname(springs_t *springs,               \
                             float gname[restrict MAXSPRINGS]) \
    {                                                          \
        loopsprings(i) springs->desc.gname[i] =                \
            springs->part##_delayline.gname[i] = gname[i];     \
    }

gfunc(gripple, low) gfunc(gecho, low)
#undef gfunc

    void springs_set_t60(springs_t *springs, float t60[restrict MAXSPRINGS],
                         int count)
{
    struct low_delayline *ldl  = &springs->low_delayline;
    struct high_delayline *hdl = &springs->high_delayline;
    loopsprings(i)
    {
        /* get delay length values */
        float L              = springs->desc.length[i];
        springs->desc.t60[i] = t60[i];
        float t60samples     = t60[i];

        float ghf =
            -powf(0.001, L * LENGTH_HILO_RATIO / (t60samples * T60_HILO_RATIO));
        setinc(hdl->ghf, ghf, count, i);

        float glf = -powf(0.001, L / t60samples);
        // low delay line is downsampled
        {
            int countM = count / springs->downsampleM;
            countM     = countM < 1 ? 1 : countM;
            setinc(ldl->glf, glf, countM, i);
        }
    }

    springs->increment_t60 = INCREMENT;
}

void springs_set_chaos(springs_t *springs, float chaos[restrict MAXSPRINGS],
                       int count)
{
    struct low_delayline *ldl  = &springs->low_delayline;
    struct high_delayline *hdl = &springs->high_delayline;

    loopsprings(i)
    {
        springs->desc.chaos[i] = chaos[i];
        /* get delay length values */
        float L1 =
            ldl->L1.val[i] + ldl->L1.inc[i] * count / springs->downsampleM;
        float Lhigh = hdl->L.val[i] + hdl->L.inc[i] * count;

        springs->low_delayline.noise.amount[i]  = L1 * chaos[i];
        springs->high_delayline.noise.amount[i] = Lhigh * chaos[i];
    }
}

void springs_set_vol(springs_t *springs, float vol[restrict MAXSPRINGS],
                     int count)
{
    int use_solo = 0;
    loopsprings(i) use_solo |= springs->desc.solo[i] != 0;

    loopsprings(i)
    {
        float db = springs->desc.vol[i] = vol[i];
        float ratio                     = springs->desc.hilomix[i];

        /* set gain from db, solo & mute */
        float gain;
        if ((!use_solo || springs->desc.solo[i] != 0) &&
            springs->desc.mute[0] == 0)
            gain = powf(10.f, db / 20.f);
        else
            gain = 0.f;

        float ghigh =
            atanf(5.f * (ratio - 0.5)) / atanf(5.f) / 2.f + .5f; // TODO change
        float glow = 1.f - ghigh;
        ghigh *= gain;
        glow *= gain;
        setinc(springs->glow, glow, count, i);
        setinc(springs->ghigh, ghigh, count, i);
    }
    springs->increment_glowhigh = INCREMENT;
}
void springs_set_hilomix(springs_t *springs, float hilomix[restrict MAXSPRINGS],
                         int count)
{
    loopsprings(i) { springs->desc.hilomix[i] = hilomix[i]; }
    springs_set_vol(springs, springs->desc.vol, count);
}

void springs_set_solo(springs_t *restrict springs, int solo[MAXSPRINGS],
                      int count)
{
    loopsprings(i) springs->desc.solo[i] = solo[i];
    springs_set_vol(springs, springs->desc.vol, count);
}

void springs_set_mute(springs_t *restrict springs, int mute[MAXSPRINGS],
                      int count)
{
    loopsprings(i) springs->desc.mute[i] = mute[i];
    springs_set_vol(springs, springs->desc.vol, count);
}

void springs_set_pan(springs_t *springs, float pan[restrict MAXSPRINGS],
                     int count)
{
    loopsprings(i)
    {
        springs->desc.pan[i] = pan[i];
#if NCHANNELS == 2
        float theta  = (1.f + pan[i]) * M_PI / 4.f;
        float gleft  = cosf(theta);
        float gright = sinf(theta);

        setinc(springs->gchannel[0], gleft, count, i);
        setinc(springs->gchannel[1], gright, count, i);
#endif
    }
    springs->increment_gchannel = INCREMENT;
}

void springs_set_drywet(springs_t *springs, float drywet, int count)
{
    springs->desc.drywet = drywet;
    if (springs->drywet != springs->desc.drywet) {
        springs->drywet_inc = (springs->desc.drywet - springs->drywet) / count;
    }
}

static void noise_process(struct noise *restrict ns, float y[MAXSPRINGS])
{
    if (ns->phase > 1.f) {
        ns->phase -= 1.f;
#pragma omp simd
        loopsprings(i)
        {
            ns->state[i]    = ns->seed[i] + ns->state[i] * 1103515245;
            float nextpoint = ns->state[i] / ((float)INT_MAX) * ns->amount[i];
            float ym1       = ns->y[0][i];
            float y0        = ns->y[1][i];
            float y1        = ns->y[2][i];
            float y2        = nextpoint;

            float c0 = y0;
            float c1 = .5 * (y1 - ym1);
            float c2 = ym1 - 2.5 * y0 + 2. * y1 - .5 * y2;
            float c3 = .5 * (y2 - ym1) + 1.5 * (y0 - y1);

            ns->c[0][i] = c0;
            ns->c[1][i] = c1;
            ns->c[2][i] = c2;
            ns->c[3][i] = c3;

            ns->y[0][i] = y0;
            ns->y[1][i] = y1;
            ns->y[2][i] = y2;
        }
    }

    float x = ns->phase;
#pragma omp simd
    loopsprings(i)
    {
        float c0 = ns->c[0][i];
        float c1 = ns->c[1][i];
        float c2 = ns->c[2][i];
        float c3 = ns->c[3][i];
        y[i]     = c0 + (x * (c1 + x * (c2 + x * c3)));
    }

    ns->phase += ns->freq;
}

/* delay line */
#pragma omp declare simd linear(i) uniform(tap, buffer, mask)
static inline float tap_linear(struct delay_tap *tap, springsfloat buffer[],
                               int mask, int i)
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
static inline float tap_cubic(struct delay_tap *tap, springsfloat buffer[],
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
                           float y[MAXSPRINGS], enum inc inc_delaytime,
                           enum inc inc_t60)
{
    y = __builtin_assume_aligned(y, sizeof(springsfloat));

    float mod[MAXSPRINGS];
    /* modulation */
    noise_process(&dl->noise, mod);

#pragma omp simd
    loopsprings(i)
    {
        if (inc_delaytime) {
            addinc(dl->L1, i);
            addinc(dl->Lecho, i);
            addinc(dl->Lripple, i);

            tap_set_delay(&dl->tap_echo, dl->Lecho.val[i], i);
            tap_set_delay(&dl->tap_ripple, dl->Lripple.val[i], i);
        }
        tap_set_delay(&dl->tap1, dl->L1.val[i] + mod[i], i);
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

        if (inc_t60) addinc(dl->glf, i);
        y[i] += tap_ripple * dl->glf.val[i];
    }

    /* increment buffer ids */
    dl->tap1.id       = (dl->tap1.id + 1) & LOWDELAY1MASK;
    dl->tap_echo.id   = (dl->tap_echo.id + 1) & LOWDELAYECHOMASK;
    dl->tap_ripple.id = (dl->tap_ripple.id + 1) & LOWDELAYRIPPLEMASK;
}

void high_delayline_process(struct high_delayline *restrict dl,
                            float y[MAXSPRINGS], enum inc inc_delaytime,
                            enum inc inc_t60)
{
    y = __builtin_assume_aligned(y, sizeof(springsfloat));

    float mod[MAXSPRINGS];
    /* modulation */
    noise_process(&dl->noise, mod);

#pragma omp simd
    loopsprings(i)
    {
        if (inc_delaytime) addinc(dl->L, i);
        // todo gmod should be divised by downsampleM
        tap_set_delay(&dl->tap, dl->L.val[i] + mod[i], i);
    }
#pragma omp simd
    loopsprings(i)
    {
        float tap = tap_linear(&dl->tap, dl->buffer, HIGHDELAYMASK, i);

        if (inc_t60) addinc(dl->ghf, i);
        y[i] += tap * dl->ghf.val[i];
    }

    /* increment buffer ids */
    dl->tap.id = (dl->tap.id + 1) & HIGHDELAYMASK;
}

void low_dc_process(struct low_dc *restrict dc, float y[MAXSPRINGS])
{
    y = __builtin_assume_aligned(y, sizeof(springsfloat));

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
void low_cascade_process(struct low_cascade *restrict lc, float y[MAXSPRINGS],
                         enum inc inc_a1)
{
    y = __builtin_assume_aligned(y, sizeof(springsfloat));

    // load mem id
    springsint idx;
#pragma omp simd
    loopsprings(i) idx[i] = (lc->id - lc->iK[i]) & LOW_CASCADE_STATE_MASK;

    // load parameters
    springsfloat a1, a2;
#pragma omp simd
    loopsprings(i)
    {
        if (inc_a1) addinc(lc->a1, i);
        a1[i] = lc->a1.val[i], a2[i] = lc->a2[i];
    }

    for (int j = 0; j < lc->max_stages; ++j) {
        float s1mem[MAXSPRINGS];
        loopsprings(i) s1mem[i] = lc->state[j].s1[idx[i]][i];
#pragma omp simd
        loopsprings(i)
        {
            /* compute internal allpass1 */
            float s1 = y[i] - a1[i] * s1mem[i];
            float y1 = a1[i] * s1 + s1mem[i];
            /* output value if spring has corresponding number of stages */
            if (j < lc->min_stages) y[i] = y1;
            else
                y[i] = j < lc->stages[i] ? y1 : y[i];

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
                                             float y[MAXSPRINGS])
{
    filter(_process)(springs->lowpassfilter, y, NLOWPASSSOS);
}

void low_eq_process(struct low_eq *restrict le, float y[MAXSPRINGS])
{
    y = __builtin_assume_aligned(y, sizeof(springsfloat));

    springsint idk;
    springsint id2k;
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

void high_cascade_process(struct high_cascade *restrict hc, float y[MAXSPRINGS],
                          enum inc inc_ahigh)
{
    y = __builtin_assume_aligned(y, sizeof(springsfloat));
    /* high chirp allpass chain is stretched by a factor of two,
     * this isn't the way it's supposed to be but it sounds better so ehh..
     */

    springsfloat a;
    loopsprings(i)
    {
        if (inc_ahigh) addinc(hc->a, i);
        a[i] = hc->a.val[i];
    }

    for (int j = 0; j < HIGH_CASCADE_N; ++j) {
#pragma omp simd
        loopsprings(i)
        {
            float sk = hc->state[j][HIGH_CASCADE_STRETCH - 1][i];
            float s0 = y[i] - a[i] * sk;

            y[i] = a[i] * s0 + sk;

            for (int k = HIGH_CASCADE_STRETCH - 1; k > 0; --k)
                hc->state[j][k][i] = hc->state[j][k - 1][i];
            hc->state[j][0][i] = s0;
        }
    }
}

void rms_process(struct rms *restrict rms, const float y[][MAXSPRINGS],
                 int count)
{
    y = __builtin_assume_aligned(y, sizeof(springsfloat));

    /* get ceil of count/RMS_OVERLAPP_SIZE */
    const int nwindows = (count + RMS_OVERLAP_SIZE - 1) / RMS_OVERLAP_SIZE;

    int n = 0;

    for (int w = 0; w < nwindows; ++w) {
        /* compute square in new overlap window */
        int overlap_n;
        for (overlap_n = rms->overlap_n;
             overlap_n < RMS_OVERLAP_SIZE && n < count; ++overlap_n) {
            loopsprings(i) rms->yoverlap[0][i] += y[n][i] * y[n][i];
            ++n;
        }
        rms->overlap_n = (overlap_n)&RMS_OVERLAP_MASK;

        /* only if count and RMS_OVERLAPP_SIZE exactly overlap and all squares
         * have been computed */
        if (rms->overlap_n == 0) {
            /* sum and rms */
            springsfloat sum = {0};
            for (int overlap_id = 0; overlap_id < RMS_NOVERLAPS; ++overlap_id) {
                loopsprings(i) sum[i] += rms->yoverlap[overlap_id][i];
            }
            loopsprings(i) rms->rms[rms->rms_id][i] =
                sqrtf(sum[i] / ((float)RMS_SIZE));

            /* cycle overlap values order */
            for (int overlap_id = RMS_NOVERLAPS - 1; overlap_id > 0;
                 --overlap_id) {
                loopsprings(i) rms->yoverlap[overlap_id][i] =
                    rms->yoverlap[overlap_id - 1][i];
            }
            loopsprings(i) rms->yoverlap[0][i] = 0.f;

            rms->rms_id = (rms->rms_id + 1) & RMS_BUFFER_MASK;
        }
    }
}

#define loopsamples(n) for (int n = 0; n < blocksize; ++n)
#define loopdownsamples(n) \
    for (int n = downsamplestart; n < blocksize; n += springs->downsampleM)

// only flatten in clang, gcc seems to break vectorization
__attribute__((flatten)) void springs_process(springs_t *restrict springs,
                                              const float *const *in,
                                              float *const *out, int count)
{
    /* springs */
    springsfloat *y     = springs->ylow;
    springsfloat *ylow  = springs->ylow;
    springsfloat *yhigh = springs->yhigh;

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
                    sources[c] = in[c][nbase + n];
                    sources[NCHANNELS] += in[c][nbase + n] / NCHANNELS;
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
                    sources[c] = in[c][nbase + n];
                    sources[NCHANNELS] += in[c][nbase + n] / NCHANNELS;
                }
#pragma omp simd
                loopsprings(i) ylow[n][i] = yhigh[n][i] =
                    sources[springs->desc.source[i]];
            }
        }

        /* low chirps */
        // get delay tap
        int lowdelay1id = springs->low_delayline.tap1.id;
        loopdownsamples(n) low_delayline_process(
            &springs->low_delayline, ylow[n], springs->increment_delaytime,
            springs->increment_t60);
        springs->low_delayline.tap1.id = lowdelay1id;

        // get delay tap
        int highdelayid = springs->high_delayline.tap.id;
        loopsamples(n) high_delayline_process(
            &springs->high_delayline, yhigh[n], springs->increment_delaytime,
            springs->increment_t60);
        springs->high_delayline.tap.id = highdelayid;

        // dc filter
        loopdownsamples(n) low_dc_process(&springs->low_dc, ylow[n]);
        // allpass cascade
        loopdownsamples(n) low_cascade_process(&springs->low_cascade, ylow[n],
                                               springs->increment_a1);

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
        loopsamples(n) high_cascade_process(&springs->high_cascade, yhigh[n],
                                            springs->increment_ahigh);

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
        {
            enum inc inc              = springs->increment_glowhigh;
            struct springparam *glow  = &springs->glow;
            struct springparam *ghigh = &springs->ghigh;
            loopsamples(n) _Pragma("omp simd") loopsprings(i)
            {
                if (inc) {
                    addinc(*glow, i);
                    addinc(*ghigh, i);
                }
                y[n][i] =
                    glow->val[i] * ylow[n][i] + ghigh->val[i] * yhigh[n][i];
            }
        }

        /* rms */
        rms_process(&springs->rms, y, blocksize);

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

        sum_springs(drywet_inc != 0.f, springs->increment_gchannel)
#undef sum_springs

            nbase += blocksize;
        count -= blocksize;
    }

    springs->drywet_inc = 0.f;
    if (springs->increment_gchannel) {
        springs->increment_gchannel = NO_INCREMENT;
        for (int c = 0; c < NCHANNELS; ++c) resetinc(springs->gchannel[c]);
    }
    if (springs->increment_glowhigh) {
        springs->increment_glowhigh = NO_INCREMENT;
        resetinc(springs->glow);
        resetinc(springs->ghigh);
    }
    if (springs->increment_delaytime) {
        springs->increment_delaytime = NO_INCREMENT;
        resetinc(springs->low_delayline.L1);
        resetinc(springs->low_delayline.Lecho);
        resetinc(springs->low_delayline.Lripple);
        resetinc(springs->high_delayline.L);
    }
    if (springs->increment_t60) {
        springs->increment_t60 = NO_INCREMENT;
        resetinc(springs->low_delayline.glf);
        resetinc(springs->high_delayline.ghf);
    }
    if (springs->increment_a1) {
        springs->increment_a1 = NO_INCREMENT;
        resetinc(springs->low_cascade.a1);
    }
    if (springs->increment_ahigh) {
        springs->increment_ahigh = NO_INCREMENT;
        resetinc(springs->high_cascade.a);
    }
}
