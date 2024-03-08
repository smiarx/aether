#ifndef _WINDOWS_H
#define _WINDOWS_H

#include <math.h>

static inline void hann_init(float *win, unsigned int size)
{
    for (int n = 0; n < size; ++n)
        win[n] = 0.5f + 0.5f * cosf(M_PI * (float)(n + 1) / (size + 1));
}

#endif
