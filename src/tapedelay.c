#include "tapedelay.h"
#include "fastmath.h"

// return true if x > y
int compare(int64_t x, int64_t y)
{
    /* transform into unsigned to allow overflow */
    uint64_t tmp = -y;
    int64_t diff = x + tmp;
    return diff > 0;
}

void tapedelay_init(tapedelay_t *tapedelay, tapedelay_desc_t *desc,
                    float samplerate)
{
    memset(tapedelay, 0, sizeof(tapedelay_t));

    tapedelay->desc = *desc;

    tapedelay->samplerate      = samplerate;
    tapedelay->ringbuffer[0].V = -DELAYUNIT;
    tapedelay->ringbuffer[1].V = 0;
    tapedelay->nwrite          = 2;
    tapedelay->nread           = 0;
    tapedelay->prev_fnread     = 0.;

    for (int c = 0; c < NCHANNELS; ++c)
        tapedelay->ym1[c] = 0, tapedelay->y0[c] = 0, tapedelay->y1[c] = 0,
        tapedelay->y2[c] = 0;

    tapedelay->desc = *desc;
    tapedelay_set_delay(tapedelay, desc->delay);
}

void tapedelay_update(tapedelay_t *tapedelay, tapedelay_desc_t *desc)
{
    if (tapedelay->desc.delay != desc->delay)
        tapedelay_set_delay(tapedelay, desc->delay);
}

void tapedelay_set_delay(tapedelay_t *tapedelay, float delay)
{
    tapedelay->desc.delay = delay;
    tapedelay->speed = ((float)DELAYUNIT) / (delay * tapedelay->samplerate);
}

void tapedelay_process(tapedelay_t *restrict tapedelay, float **restrict in,
                       float **restrict out, int count)
{
    for (int n = 0; n < count; ++n) {
        size_t nwrite   = tapedelay->nwrite;
        uint64_t xwrite = tapedelay->ringbuffer[nwrite].V + tapedelay->speed;
        uint64_t xread  = xwrite - DELAYUNIT;

        size_t nread      = tapedelay->nread;
        size_t searchsize = 2;

        /* find minimum binary search size */
        for (uint64_t xsearch;
             xsearch =
                 tapedelay->ringbuffer[(nread + searchsize) & DELAYMASK].V,
             compare(xread, xsearch);
             nread += searchsize, searchsize *= 2) {
        }

        /* binary search */
        while (searchsize > 1) {
            size_t searchsize2 = searchsize / 2;
            uint64_t xsearch =
                tapedelay->ringbuffer[(nread + searchsize2) & DELAYMASK].V;
            if (compare(xread, xsearch)) nread += searchsize2;
            searchsize = searchsize2;
        }

        nread &= DELAYMASK;
        uint64_t xfound  = tapedelay->ringbuffer[nread].V;
        uint64_t xfound1 = tapedelay->ringbuffer[(nread + 1) & DELAYMASK].V;
        float fnread     = ((float)(xread - xfound)) / (xfound1 - xfound);

        float ym1[NCHANNELS], y0[NCHANNELS], y1[NCHANNELS], y2[NCHANNELS];

#if AA == 1
        float playbackspeed = ((nread - tapedelay->nread) & DELAYMASK) +
                              fnread - tapedelay->prev_fnread;
        tapedelay->prev_fnread = fnread;

        if (playbackspeed <= 1.01) {
            if (tapedelay->nread != nread) {
                tapedelay->nread = nread;
                for (int c = 0; c < NCHANNELS; ++c) {
                    tapedelay->ym1[c] = tapedelay->y0[c],
                    tapedelay->y0[c]  = tapedelay->y1[c],
                    tapedelay->y1[c]  = tapedelay->y2[c];
                    tapedelay->y2[c] =
                        tapedelay
                            ->ringbuffer[(tapedelay->nread + 2) & DELAYMASK]
                            .y[c];
                }
            }
        } else {
            /* lowpass factor */
            float a = 1. / playbackspeed;
            do {
                tapedelay->nread = (tapedelay->nread + 1) & DELAYMASK;
                for (int c = 0; c < NCHANNELS; ++c) {
                    tapedelay->ym1[c] = tapedelay->y0[c],
                    tapedelay->y0[c]  = tapedelay->y1[c],
                    tapedelay->y1[c]  = tapedelay->y2[c];
                    /* filter */
                    tapedelay->y2[c] =
                        (1 - a) *
                            (tapedelay->y2[c] + tapedelay->y1[c] +
                             tapedelay->y0[c] + tapedelay->ym1[c]) /
                            4. +
                        a * tapedelay
                                ->ringbuffer[(tapedelay->nread + 2) & DELAYMASK]
                                .y[c];
                }
            } while (compare(nread, tapedelay->nread));
        }
        for (int c = 0; c < NCHANNELS; ++c) {
            ym1[c] = tapedelay->ym1[c];
            y0[c]  = tapedelay->y0[c];
            y1[c]  = tapedelay->y1[c];
            y2[c]  = tapedelay->y2[c];
        }

#else
        tapedelay->nread = nread;
        for (int c = 0; c < NCHANNELS; ++c) {
            ym1[c] = tapedelay->ringbuffer[(nread - 1) & DELAYMASK].y[c];
            y0[c]  = tapedelay->ringbuffer[nread].y[c];
            y1[c]  = tapedelay->ringbuffer[(nread + 1) & DELAYMASK].y[c];
            y2[c]  = tapedelay->ringbuffer[(nread + 2) & DELAYMASK].y[c];
        }
#endif

        for (int c = 0; c < NCHANNELS; ++c) {
            float y = hermite(ym1[c], y0[c], y1[c], y2[c], fnread);

            int c0 = c;
#if NCHANNELS == 2
            if (tapedelay->desc.pingpong) c0 ^= 1;
#endif
            tapedelay->ringbuffer[tapedelay->nwrite].y[c0] =
                in[c][n] + (tapedelay->desc.feedback * y);

            float drywet = tapedelay->desc.drywet;
            out[c][n]    = drywet * y + (1.f - drywet) * in[c][n];
        }

        tapedelay->nwrite = (tapedelay->nwrite + 1) & DELAYMASK;
        tapedelay->ringbuffer[tapedelay->nwrite].V = xwrite;
    }
}
