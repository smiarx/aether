#define NSPRINGS   4
#define BUFFERSIZE (1 << 12)
#define BUFFERMASK (BUFFERSIZE - 1)

#define MLOW        50
#define MLOWBUFSIZE 256
#define MLOWBUFMASK (MLOWBUFSIZE - 1)

#define a1 0.2

#include <immintrin.h>
#include <omp.h>
#include <stdio.h>
#include <string.h>

#define N 2048
#define R 100

float test(void (*fp)(float, float[], float[], float[], int))
{
    float a[N];
    float b[N];
    double dtime;

    float samplerate = 48000;
    float ftr[]      = {4100, 4106, 4200, 4300};

    for (int i = 0; i < N; i++) a[i] = (i == 0);
    fp(samplerate, ftr, a, b, N);
    dtime = -omp_get_wtime();
    for (int i = 0; i < R; i++) fp(samplerate, ftr, a, b, N);
    dtime += omp_get_wtime();
    return dtime;
}

void vec(float samplerate, float ftr[], float in[], float out[], int count)
{
    __m128 lowMem[MLOW];
    __m128 lowBuf[MLOW][MLOWBUFSIZE];
    int lowBufId = 0;
    memset(lowMem, 0, MLOW * sizeof(__m128));
    memset(lowBuf, 0, MLOW * MLOWBUFSIZE * sizeof(__m128));

    __m128 va1 = _mm_set1_ps(a1);

    __m128 vftr = _mm_load_ps(ftr);
    __m128 vK   = _mm_div_ps(_mm_set1_ps(samplerate / 2.f), vftr);
    __m128i viK = _mm_cvtps_epi32(_mm_sub_ps(vK, _mm_set1_ps(.5f)));
    __m128 vfd  = _mm_sub_ps(vK, _mm_cvtepi32_ps(viK));
    __m128 vone = _mm_set1_ps(1.f);
    // a2[i] = (1-fd)/(1+fd);
    __m128 va2 = _mm_div_ps(_mm_sub_ps(vone, vfd), _mm_add_ps(vone, vfd));

    for (int n = 0; n < count; ++n) {
        float x   = in[n];
        __m128 vx = _mm_set1_ps(x);

        __m128 vy = vx;
        for (int j = 0; j < MLOW; ++j) {
            /* compute internal allpass1 */
            // int idx = (lowBufId - iK[i]) & MLOWBUFMASK;
            __m128i vidx =
                _mm_and_si128(_mm_sub_epi32(_mm_set1_epi32(lowBufId), viK),
                              _mm_set1_epi32(MLOWBUFMASK));
            /* needed to get correct offset */
            vidx = _mm_mul_epi32(vidx, _mm_set1_epi32(NSPRINGS));
            vidx = _mm_add_epi32(vidx, _mm_set_epi32(3, 2, 1, 0));

            // float s1mem = lowBuf[j][idx][i];
            __m128 vs1mem =
                _mm_i32gather_ps((float *)&lowBuf[j][0], vidx, sizeof(float));

            // float s1 = y[i] - a1*s1mem;
            __m128 vs1 = _mm_fnmadd_ps(va1, vs1mem, vy);

            /* compute allpass2 */
            // float s2mem = lowMem[j][i];
            __m128 vs2mem = lowMem[j];
            // float s2 = s1 - a2[i]*s2mem;
            __m128 vs2 = _mm_fnmadd_ps(va2, vs2mem, vs1);
            // float y2 = a2[i]*s2 + s2mem;
            __m128 vy2 = _mm_fmadd_ps(va2, vs2, vs2mem);
            // lowMem[j][i] = s2;
            lowMem[j] = vs2;

            /* compute allpass1 */
            // y[i] = a1*s1 + s1mem;
            vy = _mm_fmadd_ps(va1, vs1, vs1mem);
            // lowBuf[j][lowBufId][i] = y2;
            lowBuf[j][lowBufId] = vy2;
        }
        lowBufId = (lowBufId + 1) & MLOWBUFMASK;

        __m128 vres;
        vres   = _mm_hadd_ps(vy, vy);
        vres   = _mm_hadd_ps(vres, vres);
        out[n] = _mm_cvtss_f32(vres);
    }
}

#define loopsprings(i) for (int i = 0; i < NSPRINGS; ++i)
void scal(float samplerate, float ftr[], float in[], float out[], int count)
{
    float K[NSPRINGS];
    int iK[NSPRINGS];
    float a2[NSPRINGS];

    float lowMem[MLOW][NSPRINGS];
    float lowBuf[MLOW][MLOWBUFSIZE][NSPRINGS];
    int lowBufId = 0;
    memset(lowMem, 0, MLOW * NSPRINGS * sizeof(float));
    memset(lowBuf, 0, MLOW * MLOWBUFSIZE * NSPRINGS * sizeof(float));

    /* compute K */
    loopsprings(i)
    {
        K[i]     = (samplerate / 2.f) / (ftr[i]);
        iK[i]    = K[i] - .5f;
        float fd = K[i] - (float)iK[i];
        a2[i]    = (1 - fd) / (1 + fd);
    }

    for (int n = 0; n < count; ++n) {
        float x           = in[n];
        float y[NSPRINGS] = {x, x, x, x};
        for (int j = 0; j < MLOW; ++j) {
            loopsprings(i)
            {
                /* compute internal allpass1 */
                int idx     = (lowBufId - iK[i]) & MLOWBUFMASK;
                float s1mem = lowBuf[j][idx][i];
                float s1    = y[i] - a1 * s1mem;

                /* compute allpass2 */
                float s2mem  = lowMem[j][i];
                float s2     = s1 - a2[i] * s2mem;
                float y2     = a2[i] * s2 + s2mem;
                lowMem[j][i] = s2;

                /* compute allpass1 */
                y[i]                   = a1 * s1 + s1mem;
                lowBuf[j][lowBufId][i] = y2;
            }
        }
        lowBufId = (lowBufId + 1) & MLOWBUFMASK;

        for (int i = NSPRINGS / 2; i > 0; i /= 2) {
            for (int j = 0; j < i; ++j) y[j] += y[j + i];
        }
        out[n] = y[0];
    }
}

int main()
{
    float samplerate = 48000;
    float ftr[]      = {4100, 4106, 4200, 4300};

    float in[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    float out[16];
    int count = 16;

    void (*fp[4])(float, float[], float[], float[], int);
    fp[0]      = scal;
    fp[1]      = vec;
    int ntests = 2;
    double dtime;

    for (int i = 0; i < ntests; i++) {
        printf("test %d\n", i);
        test(fp[i]);
        dtime = test(fp[i]);
        printf("%.2f      ", dtime);
        printf("\n");
    }

    /*
    scal(samplerate, ftr, in, out, count);
    for(int i = 0; i < count; ++i)
        printf("%f, ", out[i]);
    printf("\n");


    vec(samplerate, ftr, in, out, count);
    for(int i = 0; i < count; ++i)
        printf("%f, ", out[i]);
    printf("\n");
    */
}
