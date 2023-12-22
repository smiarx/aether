#define NSPRINGS   4
#define BUFFERSIZE (1 << 12)
#define BUFFERMASK (BUFFERSIZE - 1)

#define MLOW        100
#define MLOWBUFSIZE 256
#define MLOWBUFMASK (MLOWBUFSIZE - 1)

#include <immintrin.h>
#include <omp.h>
#include <stdio.h>
#include <string.h>

#define N 48000
#define R 4

#define loopsprings(i) for (int i = 0; i < NSPRINGS; ++i)

typedef struct {
    /* set with ftr */
    union {
        float K[NSPRINGS];
        __m128 vK;
    };
    union {
        int32_t iK[NSPRINGS];
        __m128i viK;
    };
    union {
        float a2[NSPRINGS];
        __m128 va2;
    };
    /*--------*/

    union {
        float a1[NSPRINGS];
        __m128 va1;
    };

    union {
        float lowmem2[MLOW][NSPRINGS];
        __m128 vlowmem2[MLOW];
    };
    union {
        float lowmem1[MLOW][MLOWBUFSIZE][NSPRINGS];
        __m128 vlowmem1[MLOW][MLOWBUFSIZE];
    };
    int lowbufid;

    float samplerate;
} springs_t;

void springs_init(springs_t *springs, float samplerate)
{
    memset(springs->lowmem1, 0.f,
           MLOW * MLOWBUFSIZE * NSPRINGS * sizeof(float));
    memset(springs->lowmem2, 0.f, MLOW * NSPRINGS * sizeof(float));
    springs->lowbufid = 0;

    springs->samplerate = samplerate;
}

void springs_set_ftr_vec(springs_t *springs, float *ftr)
{
    __m128 vftr = _mm_load_ps(ftr);
    __m128 vK   = _mm_div_ps(_mm_set1_ps(springs->samplerate / 2.f), vftr);
    __m128i viK = _mm_cvtps_epi32(_mm_sub_ps(vK, _mm_set1_ps(.5f)));
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
        for (int j = 0; j < MLOW; ++j) {
            // int idx = (lowBufId - iK[i]) & MLOWBUFMASK;
            __m128i vidx = _mm_and_si128(
                _mm_sub_epi32(_mm_set1_epi32(springs->lowbufid), springs->viK),
                _mm_set1_epi32(MLOWBUFMASK));
            // needed to get correct offset
            vidx = _mm_slli_epi32(vidx, 2);
            vidx = _mm_add_epi32(vidx, _mm_set_epi32(3, 2, 1, 0));

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
        springs->lowbufid = (springs->lowbufid + 1) & MLOWBUFMASK;

        __m128 vres;
        vres   = _mm_hadd_ps(vy, vy);
        vres   = _mm_hadd_ps(vres, vres);
        out[n] = _mm_cvtss_f32(vres);
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
    memcpy(springs->a1, a1, NSPRINGS);
}

void springs_process(springs_t *springs, float in[], float out[], int count)
{
    for (int n = 0; n < count; ++n) {
        float x           = in[n];
        float y[NSPRINGS] = {x, x, x, x};
        for (int j = 0; j < MLOW; ++j) {
            loopsprings(i)
            {
                /* compute internal allpass1 */
                int idx = (springs->lowbufid - springs->iK[i]) & MLOWBUFMASK;
                float s1mem = springs->lowmem1[j][idx][i];
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
        springs->lowbufid = (springs->lowbufid + 1) & MLOWBUFMASK;

        for (int i = NSPRINGS / 2; i > 0; i /= 2) {
            for (int j = 0; j < i; ++j) y[j] += y[j + i];
        }
        out[n] = y[0];
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
    float ftr[]      = {4100, 4106, 4200, 4300};
    float a1[]       = {0.2, 0.2, 0.2, 0.2};

    float in[N];
    float out[N];
    memset(in, 0, N * sizeof(float));
    in[0] = 1;

    springs_t springs;
    springs_init(&springs, samplerate);
    springs_set_a1_vec(&springs, a1);
    springs_set_ftr_vec(&springs, ftr);

    void (*fp[4])(springs_t *, float *, float *, int);
    fp[0]      = springs_process_vec;
    fp[1]      = springs_process;
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
