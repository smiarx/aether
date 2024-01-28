#ifndef _LFO_H
#define _LFO_H

#define PHASEQ    30
#define PHASEUNIT ((int64_t)1 << PHASEQ)
#define LFOQ      (PHASEQ * 2)
#define LFOUNIT   ((int64_t)1 << LFOQ)

struct lfosc {
    int64_t phase;
    uint64_t freq;
};

static inline void lfosc_set_freq(struct lfosc *lfo, float freq)
{
    lfo->freq = 4 * PHASEUNIT * freq;
}

static inline int64_t lfosc_process(struct lfosc *lfo)
{
    int64_t y;
    if (lfo->phase < PHASEUNIT) {
        int64_t z = lfo->phase;
        y         = LFOUNIT - z * z;
    } else if (lfo->phase < 3 * PHASEUNIT) {
        int64_t z = lfo->phase - 2 * PHASEUNIT;
        y         = z * z - LFOUNIT;
    } else {
        lfo->phase -= 4 * PHASEUNIT;
        int64_t z = lfo->phase;
        y         = LFOUNIT - z * z;
    }
    lfo->phase += lfo->freq;
    return y;
}

#endif
