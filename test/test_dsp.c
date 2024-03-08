#include <springreverb.h>
#include <stdlib.h>
#include <unity.h>
#include <windows.h>

void setUp(void) {}

void tearDown(void) {}

void test_hann(void)
{
#define TESTSIZE 256
    float hann[TESTSIZE];
    hann_init(hann, TESTSIZE);

    /* test similar formula for hann */
    for (int n = 0; n < TESTSIZE; ++n) {
        float c = cosf((float)(n + 1) / (TESTSIZE + 1) * M_PI / 2.f);
        TEST_ASSERT_FLOAT_WITHIN(0.000001f, c * c, hann[n]);
    }

    /* test fadein and fadout should equal to 1 */
    for (int n = 0; n < (TESTSIZE + 1) / 2; ++n) {
        float fadeout = hann[n];
        float fadein  = hann[TESTSIZE - 1 - n];
        TEST_ASSERT_EQUAL_FLOAT(1.f, fadeout + fadein);
    }
#undef TESTSIZE
}

void test_rms(void)
{
#define TESTSIZE RMS_SIZE*RMS_NOVERLAPS
    /* test input */
    springsfloat y[TESTSIZE];
    for (int n = 0; n < TESTSIZE; ++n) {
        for (int i = 0; i < MAXSPRINGS; ++i) {
            y[n][i] = (float)rand() / RAND_MAX;
        }
    }
    const int nrms = TESTSIZE / RMS_OVERLAP_SIZE;

    /* expected result */
    springsfloat expect[nrms];
    for (int i = 0; i < MAXSPRINGS; ++i) {
        for (int k = 0; k < nrms; ++k) {
            const int nend = (k + 1) * RMS_OVERLAP_SIZE;
            int nstart     = nend - RMS_SIZE;
            nstart         = nstart < 0 ? 0 : nstart;
            float sum      = 0.f;
            for (int n = nstart; n < nend; ++n) {
                sum += y[n][i] * y[n][i];
            }
            expect[k][i] = sqrtf(sum / RMS_SIZE);
        }
    }

    /* test one call */
    {
        struct rms rms = {0};
        rms_process(&rms, y, TESTSIZE);
        TEST_ASSERT_EQUAL_INT(nrms, rms.rms_id);
        TEST_ASSERT_EQUAL_FLOAT_ARRAY(&expect[0][0], &rms.rms[0][0],
                                      nrms * MAXSPRINGS);
    }

    /* test smaller blocks calls */
    {
        const int blocksize = 8;
        struct rms rms      = {0};
        for (int j = 0; j < TESTSIZE / blocksize; ++j)
            rms_process(&rms, y + j * blocksize, blocksize);
        ;
        TEST_ASSERT_EQUAL_INT(nrms, rms.rms_id);
        TEST_ASSERT_EQUAL_FLOAT_ARRAY(&expect[0][0], &rms.rms[0][0],
                                      nrms * MAXSPRINGS);
    }
#undef TESTSIZE
}

/* test rms of sine (should be equal to 1/sqrt(2))*/
void test_rms_sine(void)
{
    springsfloat y[RMS_SIZE];
    springsint freq;
    for (int i = 0; i < MAXSPRINGS; ++i) freq[i] = rand() % (RMS_SIZE / 2 - 1);
    for (int n = 0; n < RMS_SIZE; ++n) {
        for (int i = 0; i < MAXSPRINGS; ++i)
            y[n][i] = sinf(n * freq[i] * (1.f / RMS_SIZE) * M_PI);
    }

    struct rms rms = {0};
    rms_process(&rms, y, RMS_SIZE);

    TEST_ASSERT_EACH_EQUAL_FLOAT(1.f / sqrtf(2.f), rms.rms[RMS_NOVERLAPS - 1],
                                 MAXSPRINGS);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_hann);
    RUN_TEST(test_rms);
    RUN_TEST(test_rms_sine);
    return UNITY_END();
}
