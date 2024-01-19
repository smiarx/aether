#ifndef _TAPEDELAY_H
#define _TAPEDELAY_H

#include <stdint.h>
#include <string.h>

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

enum tape_direction {
    FORWARDS  = 1,
    BACKWARDS = -1,
};

typedef struct {
    float delay;
    float feedback;
    float drywet;
#if NCHANNELS == 2
    int pingpong;
#endif
    float reverse;
} tapedelay_desc_t;

typedef struct {
    float samplerate;
    tapedelay_desc_t desc;
    struct {
        uint64_t V;
        float y[NCHANNELS];
    } ringbuffer[DELAYSIZE];

    size_t nread, nwrite;
    uint64_t xrevread, xrevstop;
    float prev_fnread;
    uint64_t speed;

    enum tape_direction direction;

    float ym1[NCHANNELS], y0[NCHANNELS], y1[NCHANNELS], y2[NCHANNELS];
} tapedelay_t;

#ifdef __cplusplus
#define restrict
#endif

void tapedelay_init(tapedelay_t *tapedelay, tapedelay_desc_t *desc,
                    float samplerate);
void tapedelay_update(tapedelay_t *tapedelay, tapedelay_desc_t *desc);
void tapedelay_set_delay(tapedelay_t *tapedelay, float delay);
void tapedelay_set_reverse(tapedelay_t *tapedelay, float reverse);
void tapedelay_process(tapedelay_t *restrict tapedelay, float **restrict in,
                       float **restrict out, int count);

#undef restrict

#endif
