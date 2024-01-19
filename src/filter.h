#ifndef _FILTER_H
#define _FILTER_H

#define gluename2(a, b, c) a##b##c
#define gluename(a, b, c)  gluename2(a, b, c)
#define filterN(N, name)   gluename(filter, N, name)
#define filter(name)       filterN(FILTER_VECSIZE, name)

#define FILTER_VECSIZE 1
#include "filter.c"
#undef FILTER_VECSIZE

#define FILTER_VECSIZE 2
#include "filter.c"
#undef FILTER_VECSIZE

#define FILTER_VECSIZE 4
#include "filter.c"
#undef FILTER_VECSIZE

#define FILTER_VECSIZE 8
#include "filter.c"
#undef FILTER_VECSIZE

#endif
