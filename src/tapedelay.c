#include "tapedelay.h"
#include "fastmath.h"

#define DELAYSMOOTHQ 30
#define DELAYSMOOTHF 0.9995f
#define DELAYSMOOTH                                        \
    ((uint64_t)(((uint64_t)1 << (DELAYQ - DELAYSMOOTHQ)) * \
                (1.f - DELAYSMOOTHF)))
#define INIT_DELAY 0.02f

#define SPEEDLFO_FREQ     0.87f
#define SPEEDLFO_MAXDRIFT 0.06f
#define SPEEDLFOQ         30

void tapedelay_inittaphermite(tapedelay_t *tapedelay, tap_t *tap);
void tapedelay_inittap(tapedelay_t *tapedelay, tap_t *tap,
                       const enum tape_direction direction);
size_t tapedelay_movetap(tapedelay_t *tapedelay, tap_t *tap,
                         const enum tape_direction direction);
void tapedelay_tap(tapedelay_t *tapedelay, tap_t *tap, size_t nread,
                   const enum tape_direction direction,
                   float y[restrict NCHANNELS]);

#define FADESIZE 256
static float fadelut[FADESIZE];

// return true if x > y
int comparegt(int64_t x, int64_t y)
{
    /* transform into unsigned to allow overflow */
    uint64_t tmp = -y;
    int64_t diff = x + tmp;
    return diff > 0;
}

void tapedelay_initlut()
{
    for (int i = 0; i < FADESIZE; ++i) {
        fadelut[i] = 0.5f + 0.5f * cosf(M_PI * (float)(i + 1) / (FADESIZE + 1));
    }
}

void tapedelay_init(tapedelay_t *tapedelay, tapedelay_desc_t *desc,
                    float samplerate)
{
    memset(tapedelay, 0, sizeof(tapedelay_t));

    tapedelay->desc = *desc;

    tapedelay->samplerate                  = samplerate;
    tapedelay->ringbuffer[DELAYSIZE - 3].V = -DELAYUNIT;
    tapedelay->ringbuffer[DELAYSIZE - 2].V = -DELAYUNIT;
    tapedelay->ringbuffer[DELAYSIZE - 1].V = -DELAYUNIT;
    tapedelay->ringbuffer[0].V             = -DELAYUNIT;
    tapedelay->nwrite                      = 2;

    /* initial speed */
    tapedelay->speed = DELAYUNIT / (INIT_DELAY * samplerate);
    lfosc_set_freq(&tapedelay->speed_lfo, SPEEDLFO_FREQ / samplerate);

    for (int i = 0; i < NTAPS; ++i) {
        tapedelay->tap[i].direction = FORWARDS;
        tapedelay->tap[i].xread     = -DELAYUNIT;
    }

    tapedelay->desc = *desc;
    tapedelay_set_delay(tapedelay, desc->delay);
}

void tapedelay_inittaphermite(tapedelay_t *tapedelay, tap_t *tap)
{
    const enum tape_direction direction = tap->direction;

    for (int c = 0; c < NCHANNELS; ++c)
        tap->ym1[c] =
            tapedelay->ringbuffer[(tap->nread - direction) & DELAYMASK].y[c];
    for (int c = 0; c < NCHANNELS; ++c)
        tap->y0[c] = tapedelay->ringbuffer[tap->nread].y[c];
    for (int c = 0; c < NCHANNELS; ++c)
        tap->y1[c] =
            tapedelay->ringbuffer[(tap->nread + direction) & DELAYMASK].y[c];
    for (int c = 0; c < NCHANNELS; ++c)
        tap->y2[c] =
            tapedelay->ringbuffer[(tap->nread + 2 * direction) & DELAYMASK]
                .y[c];
}

void tapedelay_inittap(tapedelay_t *tapedelay, tap_t *tap,
                       const enum tape_direction direction)
{
    tap->direction  = direction;
    uint64_t xwrite = tapedelay->ringbuffer[tapedelay->nwrite].V;
    tap->nread      = tapedelay->nwrite;
    if (direction == FORWARDS) {
        tap->xread = xwrite - DELAYUNIT;
        tap->nread = tapedelay_movetap(tapedelay, tap, BACKWARDS);
    } else {
        tap->xread    = xwrite;
        tap->xrevstop = xwrite - DELAYUNIT / 2;
    }

    tapedelay_inittaphermite(tapedelay, tap);
}

void tapedelay_update(tapedelay_t *tapedelay, tapedelay_desc_t *desc)
{
#define param_update(name)                               \
    {                                                    \
        if (tapedelay->desc.name != desc->name)          \
            tapedelay_set_##name(tapedelay, desc->name); \
    }
    param_update(delay);
    param_update(reverse);
    param_update(cutoff);
    param_update(drive);
    param_update(drift);
#undef param_update
}

void tapedelay_set_delay(tapedelay_t *tapedelay, float delay)
{
    tapedelay->desc.delay = delay;
    tapedelay->target_speed =
        ((float)DELAYUNIT) / (delay * tapedelay->samplerate);

    tapedelay_set_drift(tapedelay, tapedelay->desc.drift);
}

void tapedelay_set_reverse(tapedelay_t *tapedelay, float reverse)
{
    tapedelay->desc.reverse = reverse;

    tapedelay->tap_id ^= 1;

    tap_t *tap                          = &tapedelay->tap[tapedelay->tap_id];
    const enum tape_direction direction = reverse == 0.f ? FORWARDS : BACKWARDS;
    tapedelay_inittap(tapedelay, tap, direction);

    tapedelay->fade    = 1;
    tapedelay->fadepos = 0;
}

void tapedelay_set_cutoff(tapedelay_t *tapedelay, float cutoff)
{
    tapedelay->desc.cutoff = cutoff;
    filter(_butterworth_lp)(&tapedelay->lowpassfilter, tapedelay->samplerate,
                            cutoff);
}

void tapedelay_set_drive(tapedelay_t *tapedelay, float drive)
{
    tapedelay->desc.drive = drive;

    tapedelay->predrive_gain  = db2gain(drive);
    tapedelay->postdrive_gain = 1.f / tapedelay->predrive_gain;
}

void tapedelay_set_drift(tapedelay_t *tapedelay, float drift)
{
    tapedelay->desc.drift = drift;

    tapedelay->drift =
        (uint64_t)(tapedelay->target_speed * SPEEDLFO_MAXDRIFT * drift) >>
        (DELAYQ - SPEEDLFOQ);
}

inline size_t tapedelay_movetap(tapedelay_t *tapedelay, tap_t *tap,
                                const enum tape_direction direction)
{
    uint64_t xread = tap->xread;

    size_t nread   = tap->nread;
    int searchsize = 2 * direction;

    /* find minimum binary search size */
    for (uint64_t xsearch;
         xsearch = tapedelay->ringbuffer[(nread + searchsize) & DELAYMASK].V,
         direction == FORWARDS ? comparegt(xread, xsearch)
                               : comparegt(xsearch, xread);
         nread += searchsize, searchsize *= 2) {
    }

    /* binary search */
    while (direction * searchsize > 1) {
        size_t searchsize2 = searchsize / 2;
        uint64_t xsearch =
            tapedelay->ringbuffer[(nread + searchsize2) & DELAYMASK].V;
        if (direction == FORWARDS ? comparegt(xread, xsearch)
                                  : comparegt(xsearch, xread))
            nread += searchsize2;
        searchsize = searchsize2;
    }
    return nread & DELAYMASK;
}

inline void tapedelay_tap(tapedelay_t *tapedelay, tap_t *tap, size_t nread,
                          const enum tape_direction direction,
                          float y[restrict NCHANNELS])
{
    uint64_t xread = tap->xread;

    uint64_t xfound  = tapedelay->ringbuffer[nread].V;
    uint64_t xfound1 = tapedelay->ringbuffer[(nread + direction) & DELAYMASK].V;
    float fnread     = direction == FORWARDS
                           ? ((float)(xread - xfound)) / (xfound1 - xfound)
                           : ((float)(xread - xfound1)) / (xfound - xfound1);

    float ym1[NCHANNELS], y0[NCHANNELS], y1[NCHANNELS], y2[NCHANNELS];

#if AA == 1
    float playbackspeed =
        direction == FORWARDS
            ? ((nread - tap->nread) & DELAYMASK) + fnread - tap->prev_fnread
            : ((tap->nread - nread) & DELAYMASK) - (fnread - tap->prev_fnread);
    tap->prev_fnread = fnread;

    if (direction == BACKWARDS) fnread = 1.f - fnread;

    if (playbackspeed <= 1.01) {
        if (tap->nread != nread) {
            tap->nread = nread;
            for (int c = 0; c < NCHANNELS; ++c) {
                tap->ym1[c] = tap->y0[c], tap->y0[c] = tap->y1[c],
                tap->y1[c] = tap->y2[c];
                tap->y2[c] =
                    tapedelay
                        ->ringbuffer[(tap->nread + 2 * direction) & DELAYMASK]
                        .y[c];
            }
        }
    } else {
        /* lowpass factor */
        float a = 1. / playbackspeed;
        do {
            tap->nread = (tap->nread + direction) & DELAYMASK;
            for (int c = 0; c < NCHANNELS; ++c) {
                tap->ym1[c] = tap->y0[c], tap->y0[c] = tap->y1[c],
                tap->y1[c] = tap->y2[c];
                /* filter */
                tap->y2[c] =
                    (1 - a) *
                        (tap->y2[c] + tap->y1[c] + tap->y0[c] + tap->ym1[c]) /
                        4. +
                    a * tapedelay
                            ->ringbuffer[(tap->nread + 2 * direction) &
                                         DELAYMASK]
                            .y[c];
            }
        } while (direction == FORWARDS ? comparegt(nread, tap->nread)
                                       : comparegt(tap->nread, nread));
    }
    for (int c = 0; c < NCHANNELS; ++c) {
        ym1[c] = tap->ym1[c];
        y0[c]  = tap->y0[c];
        y1[c]  = tap->y1[c];
        y2[c]  = tap->y2[c];
    }

#else
    tap->nread = nread;
    for (int c = 0; c < NCHANNELS; ++c) {
        ym1[c] = tapedelay->ringbuffer[(nread - direction) & DELAYMASK].y[c];
        y0[c]  = tapedelay->ringbuffer[nread].y[c];
        y1[c]  = tapedelay->ringbuffer[(nread + direction) & DELAYMASK].y[c];
        y2[c] = tapedelay->ringbuffer[(nread + 2 * direction) & DELAYMASK].y[c];
    }
#endif

    for (int c = 0; c < NCHANNELS; ++c)
        y[c] = hermite(ym1[c], y0[c], y1[c], y2[c], fnread);
}

static inline int tapedelay_fade(tapedelay_t *tapedelay,
                                 float y1[restrict NCHANNELS],
                                 const float y2[restrict NCHANNELS])
{

    float xfade = fadelut[tapedelay->fadepos];

    for (int c = 0; c < NCHANNELS; ++c) y1[c] += xfade * (y2[c] - y1[c]);

    ++tapedelay->fadepos;
    return tapedelay->fadepos == FADESIZE;
}

void tapedelay_process(tapedelay_t *restrict tapedelay, float **restrict in,
                       float **restrict out, int count)
{
    float y[NCHANNELS];
    tap_t *tap = &tapedelay->tap[tapedelay->tap_id];

    for (int n = 0; n < count; ++n) {
        /* smooth delay */
        tapedelay->speed +=
            DELAYSMOOTH *
            ((int64_t)(tapedelay->target_speed - tapedelay->speed) >>
             DELAYSMOOTHQ);
        /* delay speed lfo */
        uint64_t speed    = tapedelay->speed;
        int64_t lfo_value = lfosc_process(&tapedelay->speed_lfo);
        int64_t drift =
            (lfo_value >> (SPEEDLFOQ + LFOQ - DELAYQ)) * tapedelay->drift;
        speed += drift;

        size_t nwrite   = tapedelay->nwrite;
        uint64_t xwrite = tapedelay->ringbuffer[nwrite].V + speed;

        if (tap->direction == FORWARDS) {
            tap->xread += speed;
            size_t nread = tapedelay_movetap(tapedelay, tap, FORWARDS);
            tapedelay_tap(tapedelay, tap, nread, FORWARDS, y);
        } else if (tap->direction == BACKWARDS) {
            uint64_t xread = tap->xread -= speed;
            if (!tapedelay->fade && comparegt(tap->xrevstop, xread)) {
                xread             = tap->xread + DELAYUNIT;
                uint64_t xrevstop = tap->xrevstop + DELAYUNIT / 2;

                // put back xread
                tap->xread += speed;

                tapedelay->tap_id ^= 1;
                tap = &tapedelay->tap[tapedelay->tap_id];

                tap->xread       = xread;
                tap->xrevstop    = xrevstop;
                tap->nread       = tapedelay->nwrite;
                tap->prev_fnread = 0.f;
                tap->direction   = BACKWARDS;
                tapedelay_inittaphermite(tapedelay, tap);

                tapedelay->fade    = 1;
                tapedelay->fadepos = 0;
            }
            size_t nread = tapedelay_movetap(tapedelay, tap, BACKWARDS);
            tapedelay_tap(tapedelay, tap, nread, BACKWARDS, y);
        }

        if (tapedelay->fade) {
            tap_t *tap = &tapedelay->tap[tapedelay->tap_id ^ 1];
            float yfade[NCHANNELS];
            if (tap->direction == FORWARDS) {
                tap->xread += speed;
                size_t nread = tapedelay_movetap(tapedelay, tap, FORWARDS);
                tapedelay_tap(tapedelay, tap, nread, FORWARDS, yfade);

                if (tapedelay_fade(tapedelay, y, yfade)) tapedelay->fade = 0;
            } else if (tap->direction == BACKWARDS) {
                tap->xread -= speed;
                size_t nread = tapedelay_movetap(tapedelay, tap, BACKWARDS);
                tapedelay_tap(tapedelay, tap, nread, BACKWARDS, yfade);

                if (tapedelay_fade(tapedelay, y, yfade)) tapedelay->fade = 0;
            }
        }

        /* apply lowpass to tapeoutput */
        filter(_process)(&tapedelay->lowpassfilter, y, 1);

        for (int c = 0; c < NCHANNELS; ++c) {
            y[c] *= tapedelay->predrive_gain;
            y[c] = fasttanh(y[c]);
            y[c] *= tapedelay->postdrive_gain;
        }

        for (int c = 0; c < NCHANNELS; ++c) {
            int c0 = c;
#if NCHANNELS == 2
            if (tapedelay->desc.pingpong) c0 ^= 1;
#endif
            tapedelay->ringbuffer[tapedelay->nwrite].y[c0] =
                in[c][n] + (tapedelay->desc.feedback * y[c]);

            float drywet = tapedelay->desc.drywet;
            out[c][n]    = drywet * y[c] + (1.f - drywet) * in[c][n];
        }
        tapedelay->nwrite = (tapedelay->nwrite + 1) & DELAYMASK;
        tapedelay->ringbuffer[tapedelay->nwrite].V = xwrite;
    }
}
