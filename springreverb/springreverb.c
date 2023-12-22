#define NSPRINGS   8
#define BUFFERSIZE (1 << 12)
#define BUFFERMASK (BUFFERSIZE - 1)

#define MLOW        100
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
#define glf     0.999f

#define LOWPASSN2ND    5 // number of lowpass 2nd order filter
#define LOWPASSMEMSIZE 4
#define LOWPASSMEMMASK (LOWPASSMEMSIZE - 1)

#include <immintrin.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <string.h>

#define N 48000
#define R 4

#define loopsprings(i) for (int i = 0; i < NSPRINGS; ++i)

#define springparam(typescal, typevec, name) \
    union {                                  \
        typescal name[NSPRINGS];             \
        typevec v##name;                     \
    }

typedef struct {
    /* set with ftr */
    springparam(float, __m128, K);
    springparam(int32_t, __m128i, iK);
    springparam(float, __m128, a2);
    /*--------*/

    springparam(float, __m128, a1);
    springparam(float, __m128, lowmem2[MLOW]);
    springparam(float, __m128, lowmem1[MLOW][MLOWBUFSIZE]);
    int lowbufid;

    springparam(float, __m128, lowdelay1[LOWDELAY1SIZE]);
    springparam(float, __m128, lowdelayecho[LOWDELAYECHOSIZE]);
    springparam(float, __m128, lowdelayripple[LOWDELAYRIPPLESIZE]);
    springparam(float, __m128, L1);
    springparam(float, __m128, Lecho);
    springparam(float, __m128, Lripple);
    int lowdelay1id;
    int lowdelayechoid;
    int lowdelayrippleid;

    springparam(float, __m128, lowpassmem[LOWPASSN2ND + 1][LOWPASSMEMSIZE]);
    int lowpassmemid;

    float samplerate;
} springs_t;

void springs_init(springs_t *springs, float samplerate)
{
    memset(springs->lowmem1, 0.f,
           MLOW * MLOWBUFSIZE * NSPRINGS * sizeof(float));
    memset(springs->lowmem2, 0.f, MLOW * NSPRINGS * sizeof(float));
    springs->lowbufid = 0;

    memset(springs->lowdelay1, 0.f, LOWDELAY1SIZE * NSPRINGS * sizeof(float));
    memset(springs->lowdelayecho, 0.f,
           LOWDELAYECHOSIZE * NSPRINGS * sizeof(float));
    memset(springs->lowdelayripple, 0.f,
           LOWDELAYRIPPLESIZE * NSPRINGS * sizeof(float));
    springs->lowdelay1id      = 0;
    springs->lowdelayechoid   = 0;
    springs->lowdelayrippleid = 0;

    memset(springs->lowpassmem, 0.f, sizeof(springs->lowpassmem));
    springs->lowpassmemid = 0;

    springs->samplerate = samplerate;
}

void springs_set_ftr_vec(springs_t *springs, float *ftr)
{
    __m128 vftr = _mm_load_ps(ftr);
    __m128 vK   = _mm_div_ps(_mm_set1_ps(springs->samplerate / 2.f), vftr);
    __m128i viK = _mm_sub_epi32(_mm_cvtps_epi32(vK), _mm_set1_epi32(1));
    __m128 vfd  = _mm_sub_ps(vK, _mm_cvtepi32_ps(viK));
    __m128 vone = _mm_set1_ps(1.f);
    // a2[i] = (1-fd)/(1+fd);
    __m128 va2 = _mm_div_ps(_mm_sub_ps(vone, vfd), _mm_add_ps(vone, vfd));

    springs->vK  = vK;
    springs->viK = viK;
    springs->va2 = va2;
}

void springs_set_a1_vec(springs_t *springs, float *a1)
{
    springs->va1 = _mm_load_ps(a1);
}

void springs_process_vec(springs_t *springs, float in[], float out[], int count)
{
    for (int n = 0; n < count; ++n) {
        float x   = in[n];
        __m128 vx = _mm_set1_ps(x);

        __m128 vy = vx;

        ///* tap low delayline */
#define tap(name, NAME)                                                       \
    __m128i videlay##name = _mm_cvttps_epi32(vdelay##name);                   \
    __m128i vidx##name    = _mm_and_si128(                                    \
        _mm_sub_epi32(_mm_set1_epi32(springs->lowdelay##name##id),         \
                         videlay##name),                                      \
        _mm_set1_epi32(LOWDELAY##NAME##MASK));                             \
    vidx##name        = _mm_slli_epi32(vidx##name, 2);                        \
    vidx##name        = _mm_add_epi32(vidx##name, _mm_set_epi32(3, 2, 1, 0)); \
    __m128 vtap##name = _mm_i32gather_ps((float *)&springs->vlowdelay##name,  \
                                         vidx##name, sizeof(float));

        __m128 vdelay1 = springs->vL1;
        tap(1, 1);
        springs->vlowdelayecho[springs->lowdelayechoid] =
            _mm_mul_ps(vtap1, _mm_set1_ps(1.f - gecho));

        __m128 vdelayecho = _mm_add_ps(
            _mm_sub_ps(vdelay1, _mm_cvtepi32_ps(videlay1)), springs->vLecho);
        tap(echo, ECHO);
        vtapecho = _mm_fmadd_ps(vtap1, _mm_set1_ps(gecho), vtapecho);
        springs->vlowdelayripple[springs->lowdelayrippleid] =
            _mm_mul_ps(vtapecho, _mm_set1_ps(1.f - gripple));

        __m128 vdelayripple =
            _mm_add_ps(_mm_sub_ps(vdelayecho, _mm_cvtepi32_ps(videlayecho)),
                       springs->vLripple);
        tap(ripple, RIPPLE);
        vtapripple = _mm_fmadd_ps(vtapecho, _mm_set1_ps(gripple), vtapripple);

        vy = _mm_fmadd_ps(vtapripple, _mm_set1_ps(glf), vy);
#undef tap

        /* low allpass filter chain */
        // int idx = (lowBufId - iK[i]) & MLOWBUFMASK;
        __m128i vidx = _mm_and_si128(
            _mm_sub_epi32(_mm_set1_epi32(springs->lowbufid), springs->viK),
            _mm_set1_epi32(MLOWBUFMASK));
        // needed to get correct offset
        vidx = _mm_slli_epi32(vidx, 2);
        vidx = _mm_add_epi32(vidx, _mm_set_epi32(3, 2, 1, 0));
        for (int j = 0; j < MLOW; ++j) {

            // float s1mem = lowmem1[j][idx][i];
            __m128 vs1mem = _mm_i32gather_ps((float *)&springs->vlowmem1[j][0],
                                             vidx, sizeof(float));

            // float s1 = y[i] - a1*s1mem;
            __m128 vs1 = _mm_fnmadd_ps(springs->va1, vs1mem, vy);

            /* compute allpass2 */
            // float s2mem = lowmem2[j][i];
            __m128 vs2mem = springs->vlowmem2[j];
            // float s2 = s1 - a2[i]*s2mem;
            __m128 vs2 = _mm_fnmadd_ps(springs->va2, vs2mem, vs1);
            // float y2 = a2[i]*s2 + s2mem;
            __m128 vy2 = _mm_fmadd_ps(springs->va2, vs2, vs2mem);
            // lowMem[j][i] = s2;
            springs->vlowmem2[j] = vs2;

            /* compute allpass1 */
            // y[i] = a1*s1 + s1mem;
            vy = _mm_fmadd_ps(springs->va1, vs1, vs1mem);
            // lowBuf[j][lowBufId][i] = y2;
            springs->vlowmem1[j][springs->lowbufid] = vy2;
        }

        /* input to lowdelay line */
        springs->vlowdelay1[springs->lowdelay1id] = vy;

        __m128 vres;
        vres   = _mm_hadd_ps(vy, vy);
        vres   = _mm_hadd_ps(vres, vres);
        out[n] = _mm_cvtss_f32(vres);

        /* advance buffer ids */
        springs->lowbufid    = (springs->lowbufid + 1) & MLOWBUFMASK;
        springs->lowdelay1id = (springs->lowdelay1id + 1) & LOWDELAY1MASK;
        springs->lowdelayechoid =
            (springs->lowdelayechoid + 1) & LOWDELAYECHOMASK;
        springs->lowdelayrippleid =
            (springs->lowdelayrippleid + 1) & LOWDELAYRIPPLEMASK;
    }
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
                                           4 * MLOW * (1 - a1) / (1 + a1));
        springs->Lecho[i] = L / 5;
        springs->L1[i]    = L - springs->Lecho[1] - springs->Lripple[i];
    }
}

void springs_set_Nripple(springs_t *springs, float Nripple)
{
    loopsprings(i) { springs->Lripple[i] = 2.f * springs->K[i] * Nripple; }
}

void springs_process(springs_t *springs, float in[], float out[], int count)
{
    for (int n = 0; n < count; ++n) {
        float x = in[n];
        float y[NSPRINGS];
        loopsprings(i) y[i] = x;

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

            float delay1 = springs->L1[i];
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
        for (int i = NSPRINGS / 2; i > 0; i /= 2) {
            for (int j = 0; j < i; ++j) y[j] += y[j + i];
        }
        out[n] = y[0];

        /* advance buffer ids */
        springs->lowbufid    = (springs->lowbufid + 1) & MLOWBUFMASK;
        springs->lowdelay1id = (springs->lowdelay1id + 1) & LOWDELAY1MASK;
        springs->lowdelayechoid =
            (springs->lowdelayechoid + 1) & LOWDELAYECHOMASK;
        springs->lowdelayrippleid =
            (springs->lowdelayrippleid + 1) & LOWDELAYRIPPLEMASK;
    }
}

float test(springs_t *springs, void (*fp)(springs_t *, float *, float *, int))
{
    float a[N];
    float b[N];
    double dtime;

    for (int i = 0; i < N; i++) a[i] = (i == 0);
    fp(springs, a, b, N);
    dtime = -omp_get_wtime();
    for (int i = 0; i < R; i++) fp(springs, a, b, N);
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

    float in[N];
    float out[N];
    memset(in, 0, N * sizeof(float));

    springs_t springs;
    springs_init(&springs, samplerate);
    springs_set_a1(&springs, a1);
    springs_set_ftr(&springs, ftr);
    springs_set_Nripple(&springs, 0.5);
    springs_set_Td(&springs, Td);

    void (*fp[4])(springs_t *, float *, float *, int);
    fp[0]      = springs_process;
    fp[1]      = springs_process_vec;
    int ntests = 2;
    double dtime;

    for (int i = 0; i < ntests; i++) {
        printf("test %d\n", i);
        test(&springs, fp[i]);
        dtime = test(&springs, fp[i]);
        printf("%.4f      ", dtime);
        printf("\n");
    }

    // springs_process(&springs, in, out, N);
    // for(int i = 0; i < N; ++i)
    //{
    //     printf("%.7f, ", out[i]);
    //     if((i+1)%8==0)
    //         printf("\n");
    // }
    // printf("\n");
    // printf("\n");

    // springs_init(&springs, samplerate);
    // springs_process_vec(&springs, in, out, N);
    // for(int i = 0; i < N; ++i)
    //{
    //     printf("%.7f, ", out[i]);
    //     if((i+1)%8==0)
    //         printf("\n");
    // }
}
