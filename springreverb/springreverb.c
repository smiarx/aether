#define NSPRINGS 8

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

#define MLOW        120
#define MLOWBUFSIZE 256
#define MLOWBUFMASK (MLOWBUFSIZE - 1)

#define LOWDELAY1SIZE      2048
#define LOWDELAY1MASK      (LOWDELAY1SIZE - 1)
#define LOWDELAYECHOSIZE   512
#define LOWDELAYECHOMASK   (LOWDELAYECHOSIZE - 1)
#define LOWDELAYRIPPLESIZE 128
#define LOWDELAYRIPPLEMASK (LOWDELAYRIPPLESIZE - 1)

#define gecho   0.01f
#define gripple 0.01f
#define glf     0.95f

#define amod 0.997f
#define gmod 8.3f

#define LOWPASSN2ND    5 // number of lowpass 2nd order filter
#define LOWPASSMEMSIZE 4
#define LOWPASSMEMMASK (LOWPASSMEMSIZE - 1)

#include <immintrin.h>
#include <limits.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <string.h>

#define N 512
#define R 150

#define loopsprings(i) for (int i = 0; i < NSPRINGS; ++i)

#define springparam(typescal, name) typescal name[NSPRINGS];

typedef struct {
    /* set with ftr */
    springparam(float, K);
    springparam(int32_t, iK);
    springparam(float, a2);
    /*--------*/

    springparam(float, a1);
    springparam(float, lowmem2[MLOW]);
    springparam(float, lowmem1[MLOW][MLOWBUFSIZE]);
    int lowbufid;

    springparam(float, lowdelay1[LOWDELAY1SIZE]);
    springparam(float, lowdelayecho[LOWDELAYECHOSIZE]);
    springparam(float, lowdelayripple[LOWDELAYRIPPLESIZE]);
    springparam(float, L1);
    springparam(float, Lecho);
    springparam(float, Lripple);
    int lowdelay1id;
    int lowdelayechoid;
    int lowdelayrippleid;

    /* modulation */
    springparam(int, randseed);
    springparam(int, randstate);
    springparam(float, Lmodmem);

    springparam(float, lowpassmem[LOWPASSN2ND + 1][LOWPASSMEMSIZE]);
    int lowpassmemid;

    float samplerate;
} springs_t;

void springs_init(springs_t *springs, float samplerate)
{
    memset(springs->lowmem1, 0, sizeof(springs->lowmem1));
    memset(springs->lowmem2, 0, sizeof(springs->lowmem2));
    springs->lowbufid = 0;

    memset(springs->lowdelay1, 0, sizeof(springs->lowdelay1));
    memset(springs->lowdelayecho, 0, sizeof(springs->lowdelayecho));
    memset(springs->lowdelayripple, 0, sizeof(springs->lowdelayripple));
    springs->lowdelay1id      = 0;
    springs->lowdelayechoid   = 0;
    springs->lowdelayrippleid = 0;

    loopsprings(i) springs->randseed[i] = rand();
    memset(springs->randstate, 0, sizeof(springs->randstate));
    memset(springs->Lmodmem, 0, sizeof(springs->Lmodmem));

    memset(springs->lowpassmem, 0.f, sizeof(springs->lowpassmem));
    springs->lowpassmemid = 0;

    springs->samplerate = samplerate;
}

void springs_set_ftr(springs_t *springs, float *ftr)
{
    loopsprings(i)
    {
        springs->K[i]  = (springs->samplerate / 2.f) / (ftr[i]);
        springs->iK[i] = springs->K[i] - .5f;
        float fd       = springs->K[i] - (float)springs->iK[i];
        springs->a2[i] = (1 - fd) / (1 + fd);
    }
}

void springs_set_a1(springs_t *springs, float *a1)
{
    loopsprings(i) springs->a1[i] = a1[i];
}

void springs_set_Td(springs_t *springs, float *Td)
{
    loopsprings(i)
    {
        float a1          = springs->a1[i];
        float L           = fmaxf(0.f, Td[i] * springs->samplerate -
                                           springs->K[i] * MLOW * (1 - a1) / (1 + a1));
        springs->Lecho[i] = L / 5;
        springs->L1[i]    = L - springs->Lecho[i] - springs->Lripple[i];
    }
}

void springs_set_Nripple(springs_t *springs, float Nripple)
{
    loopsprings(i) { springs->Lripple[i] = 2.f * springs->K[i] * Nripple; }
}

void springs_process(springs_t *springs, float **in, float **out, int count)
{
    for (int n = 0; n < count; ++n) {
        float y[NSPRINGS];
        loopsprings(i) y[i] = in[i * NCHANNELS / NSPRINGS][n];

        /* delay modulation */
        loopsprings(i)
        {
            springs->randstate[i] =
                springs->randseed[i] + springs->randstate[i] * 1103515245;
            float mod = springs->randstate[i] / ((float)INT_MAX);
            // filter modulation
            springs->Lmodmem[i] = mod =
                (1 - amod) * mod + amod * springs->Lmodmem[i];
        }
        /* tap low delayline */
        loopsprings(i)
        {

#define tap(name, NAME)                                                      \
    int idelay##name   = delay##name;                                        \
    float fdelay##name = delay##name - (float)idelay##name;                  \
    int idx##name =                                                          \
        (springs->lowdelay##name##id - idelay##name) & LOWDELAY##NAME##MASK; \
    float val0##name = springs->lowdelay##name[idx##name][i];                \
    float val1##name =                                                       \
        springs->lowdelay##name[(idx##name - 1) & LOWDELAY##NAME##MASK][i];  \
    float tap##name = val0##name + (val1##name - val0##name) * fdelay##name

            float delay1 = springs->L1[i] + gmod * springs->Lmodmem[i];
            tap(1, 1);
            springs->lowdelayecho[springs->lowdelayechoid][i] =
                tap1 * (1.f - gecho);

            float delayecho = fdelay1 + springs->Lecho[i];
            tap(echo, ECHO);
            tapecho += tap1 * gecho;
            springs->lowdelayripple[springs->lowdelayrippleid][i] =
                tapecho * (1.f - gripple);

            float delayripple = fdelayecho + springs->Lripple[i];
            tap(ripple, RIPPLE);
            tapripple += tapecho * gripple;

            y[i] += tapripple * glf;
#undef tap
        }

        /* low allpass filter chain */
        int idx[NSPRINGS];
        loopsprings(i) idx[i] =
            (springs->lowbufid - springs->iK[i]) & MLOWBUFMASK;
        for (int j = 0; j < MLOW; ++j) {
            loopsprings(i)
            {
                /* compute internal allpass1 */
                float s1mem = springs->lowmem1[j][idx[i]][i];
                float s1    = y[i] - springs->a1[i] * s1mem;

                /* compute allpass2 */
                float s2mem            = springs->lowmem2[j][i];
                float s2               = s1 - springs->a2[i] * s2mem;
                float y2               = springs->a2[i] * s2 + s2mem;
                springs->lowmem2[j][i] = s2;

                /* compute allpass1 */
                y[i] = springs->a1[i] * s1 + s1mem;
                springs->lowmem1[j][springs->lowbufid][i] = y2;
            }
        }

        /* feed delayline */
        loopsprings(i) { springs->lowdelay1[springs->lowdelay1id][i] = y[i]; }

        /* low pass filter */
        const float filtersos[][2][3] = {
            {{2.18740696e-03, 6.16778351e-04, 2.18740696e-03},
             {1.00000000e+00, -1.70313982e+00, 7.48223150e-01}},
            {{1.00000000e+00, -1.31079983e+00, 1.00000000e+00},
             {1.00000000e+00, -1.68138176e+00, 8.53580218e-01}},
            {{1.00000000e+00, -1.55984297e+00, 1.00000000e+00},
             {1.00000000e+00, -1.66400724e+00, 9.39118831e-01}},
            {{1.00000000e+00, -1.62171918e+00, 1.00000000e+00},
             {1.00000000e+00, -1.65694459e+00, 9.78755965e-01}},
            {{1.00000000e+00, -1.63865216e+00, 1.00000000e+00},
             {1.00000000e+00, -1.65708087e+00, 9.94971669e-01}}};
        loopsprings(i)
        {
            int id                        = springs->lowpassmemid;
            springs->lowpassmem[0][id][i] = y[i];

            for (int j = 0; j < LOWPASSN2ND; ++j) {
                float a_acc = 0, b_acc = 0;
                for (int k = 1; k < 3; ++k)
                    a_acc +=
                        filtersos[j][1][k] *
                        springs
                            ->lowpassmem[j + 1][(id - k) & LOWPASSMEMMASK][i];
                for (int k = 0; k < 3; ++k)
                    b_acc +=
                        filtersos[j][0][k] *
                        springs->lowpassmem[j][(id - k) & LOWPASSMEMMASK][i];
                springs->lowpassmem[j + 1][id][i] = b_acc - a_acc;
            }
            y[i] = springs->lowpassmem[LOWPASSN2ND][id][i];
        }
        springs->lowpassmemid = (springs->lowpassmemid + 1) & LOWPASSMEMMASK;

        /* sum springs */
        for (int c = 0; c < NCHANNELS; ++c) {
            int offset = c * NSPRINGS / NCHANNELS;
            for (int i = NSPRINGS / 2 / NCHANNELS; i > 0; i /= 2) {
                for (int j = 0; j < i; ++j) y[offset + j] += y[offset + j + i];
            }
            out[c][n] = y[offset];
        }

        /* advance buffer ids */
        springs->lowbufid    = (springs->lowbufid + 1) & MLOWBUFMASK;
        springs->lowdelay1id = (springs->lowdelay1id + 1) & LOWDELAY1MASK;
        springs->lowdelayechoid =
            (springs->lowdelayechoid + 1) & LOWDELAYECHOMASK;
        springs->lowdelayrippleid =
            (springs->lowdelayrippleid + 1) & LOWDELAYRIPPLEMASK;
    }
}

float test(springs_t *springs, void (*fp)(springs_t *, float **, float **, int))
{
    float a[NCHANNELS][N];
    float b[NCHANNELS][N];
    double dtime;

    float *in[2], *out[2];
    for (int c = 0; c < NCHANNELS; ++c) {
        in[c]  = &a[c][0];
        out[c] = &b[c][0];
    }

    for (int i = 0; i < N; i++) a[0][i] = (i == 0);
    fp(springs, in, out, N);
    dtime = -omp_get_wtime();
    for (int i = 0; i < R; i++) fp(springs, in, out, N);
    dtime += omp_get_wtime();
    return dtime;
}

int main()
{
    float samplerate = 48000;
    float ftr[]      = {4210, 4106, 4200, 4300, 3930, 4118, 4190, 4310};
    float a1[]       = {0.18, 0.21, 0.312, 0.32, 0.32, 0.23, 0.21, 0.2};
    float Td[]       = {0.0552, 0.04366, 0.04340, 0.04370,
                        .0552,  0.04423, 0.04367, 0.0432};

    float ins[NCHANNELS][N];
    float outs[NCHANNELS][N];
    float *in[2], *out[2];

    for (int c = 0; c < NCHANNELS; ++c) {
        in[c]  = &ins[c][0];
        out[c] = &outs[c][0];
    }

    memset(ins, 0, sizeof(ins));

    springs_t springs;
    springs_init(&springs, samplerate);
    springs_set_a1(&springs, a1);
    springs_set_ftr(&springs, ftr);
    springs_set_Nripple(&springs, 0.5);
    springs_set_Td(&springs, Td);

    void (*fp[4])(springs_t *, float **, float **, int);
    fp[0]      = springs_process;
    int ntests = 1;
    double dtime;

    for (int i = 0; i < ntests; i++) {
        printf("test %d\n", i);
        test(&springs, fp[i]);
        dtime = test(&springs, fp[i]);
        printf("%.4f      ", dtime);
        printf("\n");
    }

    // for(int r=0; r < R; ++r)
    //{
    //     ins[0][0] = r==0 ? 1.f : 0.f;
    //     springs_process(&springs, in, out, N);
    //     for(int i = 0; i < N; ++i)
    //     {
    //         printf("%.7f, ", out[0][i]);
    //         if((i+1)%8==0)
    //             printf("\n");
    //     }
    //     printf("\n");
    // }
    // printf("\n");
}
