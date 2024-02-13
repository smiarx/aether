#ifndef _SPRINGREVERB_H
#define _SPRINGREVERB_H

#ifndef MAXSPRINGS
#define MAXSPRINGS 8

#endif
#ifndef NSPRINGS
#define NSPRINGS MAXSPRINGS
#endif

/* include filter functions */
#include "filter.h"
#define FILTER_VECSIZE NSPRINGS

#ifndef STEREO
#ifndef MONO
#define STEREO
#endif
#endif
#ifdef STEREO
#define NCHANNELS 2
#else
#define NCHANNELS 1
#endif

#define BUFFERSIZE (1 << 12)
#define BUFFERMASK (BUFFERSIZE - 1)

#define LOW_CASCADE_N          120
#define LOW_CASCADE_STATE_SIZE 32
#define LOW_CASCADE_STATE_MASK (LOW_CASCADE_STATE_SIZE - 1)

#define LOW_EQ_STATE_SIZE (LOW_CASCADE_STATE_SIZE * 2)
#define LOW_EQ_STATE_MASK (LOW_EQ_STATE_SIZE - 1)

#define LOWDELAY1SIZE      (2048 << 2)
#define LOWDELAY1MASK      (LOWDELAY1SIZE - 1)
#define LOWDELAYECHOSIZE   (512 << 2)
#define LOWDELAYECHOMASK   (LOWDELAYECHOSIZE - 1)
#define LOWDELAYRIPPLESIZE (128 << 2)
#define LOWDELAYRIPPLEMASK (LOWDELAYRIPPLESIZE - 1)

#define gmod 10.f
#define amod (1.f - 0.05f / gmod)

#define HIGH_CASCADE_N       70
#define HIGH_CASCADE_STRETCH 2

#define HIGHDELAYSIZE LOWDELAY1SIZE
#define HIGHDELAYMASK (HIGHDELAYSIZE - 1)

#define glow2high (0.0f)
#define ghigh2low (0.0f)

// filter
#define FILTERMEMSIZE 4
#define FILTERMEMMASK (FILTERMEMSIZE - 1)
#define NAASOS        5 // number of aa 2nd order filter
#define NLOWPASSSOS   5 // number of lowpass 2nd order filter

#define MAXBLOCKSIZE 512

#define springvec(type, name) \
    type name[MAXSPRINGS] __attribute__((aligned(sizeof(type) * MAXSPRINGS)))

struct springparam {
    springvec(float, val);
    springvec(float, inc);
};
typedef int doinc_t;

/* spring parameters */
typedef struct {
    springvec(float, ftr);
    springvec(float, a1);
    springvec(float, ahigh);
    springvec(float, length);
    springvec(float, fcutoff);
    springvec(float, gripple);
    springvec(float, gecho);
    springvec(float, t60);
    springvec(float, chaos);
    springvec(float, vol);
    springvec(float, hilomix);
    springvec(int, source);
    springvec(float, pan);
    float drywet;
} springs_desc_t;

/* low dc filter */
struct low_dc {
    springvec(float, b);
    springvec(float, a);
    springvec(float, state);
};

/* low allpass cascade */
struct low_cascade {
    springvec(int, iK);
    springvec(float, a1);
    springvec(float, a2);
    struct {
        springvec(float, s2);
        springvec(float, s1[LOW_CASCADE_STATE_SIZE]);
    } state[LOW_CASCADE_N];
    int id;
};

/* low equalizer */
struct low_eq {
    springvec(int, Keq);
    springvec(float, b0);
    springvec(float, ak);
    springvec(float, a2k);
    springvec(float, state[LOW_EQ_STATE_SIZE]);
    int id;
};

/* high cascade */
struct high_cascade {
    springvec(float, a);
    springvec(float, state[HIGH_CASCADE_N][HIGH_CASCADE_STRETCH]);
};

/* perlin noise */
struct noise {
    springvec(int, seed);
    springvec(int, state);
    springvec(float, amount);
    springvec(float, y[3]);
    springvec(float, c[4]);
    float phase;
    float freq;
};
/* delay line */
struct delay_tap {
    springvec(int, idelay);
    springvec(float, fdelay);
    int id;
};

struct low_delayline {
    struct noise noise;

    struct springparam L1;
    struct delay_tap tap1;
    springvec(float, buffer1[LOWDELAY1SIZE]);

    struct springparam Lecho;
    struct delay_tap tap_echo;
    springvec(float, buffer_echo[LOWDELAYECHOSIZE]);
    springvec(float, gecho);

    struct springparam Lripple;
    struct delay_tap tap_ripple;
    springvec(float, buffer_ripple[LOWDELAYRIPPLESIZE]);
    springvec(float, gripple);

    struct springparam glf;
};

struct high_delayline {
    struct noise noise;

    struct springparam L;
    struct delay_tap tap;
    springvec(float, buffer[HIGHDELAYSIZE]);

    struct springparam ghf;
};

typedef struct {
    springs_desc_t desc;

    /* block compute */
    int blocksize;
    springvec(float, ylow[MAXBLOCKSIZE]);
    springvec(float, yhigh[MAXBLOCKSIZE]);

    /* down sampling */
    int downsampleM;
    int downsampleid;
    filter(_t) aafilter[NAASOS];

    /* set with ftr */
    springvec(float, K);
    /*--------*/

    struct low_dc low_dc;
    struct low_cascade low_cascade;

    struct low_delayline low_delayline;
    doinc_t increment_delaytime, increment_t60;
    struct high_delayline high_delayline;

    struct low_eq low_eq;

    filter(_t) lowpassfilter[NLOWPASSSOS];

    struct high_cascade high_cascade;

    /* low and high gain */
    struct springparam glow;
    struct springparam ghigh;
    doinc_t increment_glowhigh;

    struct springparam gchannel[NCHANNELS];
    doinc_t increment_gchannel;

    float drywet;
    float drywet_inc;

    float samplerate;
} springs_t;

void springs_init(springs_t *springs, springs_desc_t *desc, float samplerate);
void springs_update(springs_t *springs, springs_desc_t *desc);
void springs_set_dccutoff(springs_t *springs, float fcutoff[MAXSPRINGS]);
void springs_set_ftr(springs_t *springs, float ftr[MAXSPRINGS], int count);
void springs_set_a1(springs_t *springs, float a1[MAXSPRINGS]);
void springs_set_length(springs_t *springs, float length[MAXSPRINGS],
                        int count);
void springs_set_ahigh(springs_t *springs, float a1[MAXSPRINGS]);
void springs_set_gripple(springs_t *springs, float gripple[MAXSPRINGS]);
void springs_set_gecho(springs_t *springs, float gecho[MAXSPRINGS]);
void springs_set_t60(springs_t *springs, float t60[MAXSPRINGS], int count);
void springs_set_chaos(springs_t *springs, float chaos[MAXSPRINGS], int count);
void springs_set_vol(springs_t *springs, float vol[MAXSPRINGS], int count);
void springs_set_pan(springs_t *springs, float pan[MAXSPRINGS], int count);
void springs_set_drywet(springs_t *springs, float drywet, int count);
void springs_set_hilomix(springs_t *springs, float hilomix[MAXSPRINGS],
                         int count);

void low_delayline_process(struct low_delayline *dl, float y[MAXSPRINGS],
                           doinc_t inc_delaytime, doinc_t inc_t60);
void low_dc_process(struct low_dc *dc, float y[MAXSPRINGS]);
void low_cascade_process(struct low_cascade *lc, float y[MAXSPRINGS]);
void springs_lowlpf(springs_t *springs, float y[MAXSPRINGS]);
void low_eq_process(struct low_eq *le, float y[MAXSPRINGS]);
void high_cascade_process(struct high_cascade *hc, float y[MAXSPRINGS]);
void high_delayline_process(struct high_delayline *dl, float y[MAXSPRINGS],
                            doinc_t inc_delaytime, doinc_t inc_t60);
void springs_process(springs_t *springs, const float *const *in,
                     float *const *out, int count);

#endif
