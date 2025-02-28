#ifndef _TAPEDELAY_H
#define _TAPEDELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#undef FILTER_VECSIZE
#include "filter.h"
#define FILTER_VECSIZE 2
#include "lfo.h"

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

#ifndef AA
#define AA 1
#endif

#define DELAYSIZE (1 << 16)
#define DELAYMASK (DELAYSIZE - 1)
#define DELAYQ    61
#define DELAYUNIT (((uint64_t)1) << DELAYQ)

#define NTAPS    3
#define NBUFFERS 2
#define OPTTAP   2

enum tape_direction {
    FORWARDS  = 1,
    BACKWARDS = -1,
};

enum tape_mode {
    NORMAL,
    BACK_FORTH,
    REVERSE,
};

typedef struct {
    float delay;
    float feedback;
    float drywet;
    float cutoff;
    float drive;
    float drift;
    float drift_freq;
#if NCHANNELS == 2
    int pingpong;
#endif
    float fmode;
    enum tape_mode mode;
} tapedelay_desc_t;

typedef struct {
    size_t nread;
    uint64_t xread, xrevstop; // for reverse only
    float prev_fnread;
    float ym1[NCHANNELS], y0[NCHANNELS], y1[NCHANNELS], y2[NCHANNELS];

    int buffer_id;
    enum tape_direction direction;
} tap_t;

typedef struct {
    float samplerate;
    tapedelay_desc_t desc;
    struct {
        uint64_t V;
        float y[NBUFFERS][NCHANNELS];
    } ringbuffer[DELAYSIZE];

    size_t nwrite;
    tap_t tap[NTAPS];
    size_t tap_id;
    int fade;
    int fadepos;

    float predrive_gain;
    float postdrive_gain;

    uint64_t target_speed;
    uint64_t speed;
    struct lftri speed_lfo;
    uint64_t drift;

    filter(_t) lowpassfilter;

    int twobuffers;
} tapedelay_t;

void tapedelay_init(tapedelay_t *tapedelay, tapedelay_desc_t *desc,
                    float samplerate);
void tapedelay_update(tapedelay_t *tapedelay, tapedelay_desc_t *desc);
void tapedelay_set_delay(tapedelay_t *tapedelay, float delay);
void tapedelay_set_fmode(tapedelay_t *tapedelay, float fmode);
void tapedelay_set_feedback(tapedelay_t *tapedelay, float feedback);
void tapedelay_set_drywet(tapedelay_t *tapedelay, float drywet);
void tapedelay_set_cutoff(tapedelay_t *tapedelay, float cutoff);
void tapedelay_set_drive(tapedelay_t *tapedelay, float drive);
void tapedelay_set_drift(tapedelay_t *tapedelay, float drift);
void tapedelay_set_drift_freq(tapedelay_t *tapedelay, float drift_freq);
void tapedelay_process(tapedelay_t *restrict tapedelay, const float *const *in,
                       float *const *out, int count);

#ifdef __cplusplus
}
#endif

#endif
