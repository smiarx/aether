#include "springreverb.h"
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 512
#define R 1000

int main(int argc, char **argv)
{
    int r = R, n = N;

    --argc;
    ++argv;

    while (argc > 0) {
        if (strcmp(*argv, "-n") == 0) {
            n = atoi(*++argv);
            --argc;
        } else if (strcmp(*argv, "-r") == 0) {
            r = atoi(*++argv);
            --argc;
        }
        ++argv;
        --argc;
    }

    float samplerate = 48000;
    float cutoff[]   = {40, 40, 38, 20, 49, 31, 32, 33};
    float ftr[]      = {4210, 4106, 4200, 4300, 3330, 4118, 4190, 4310};
    float a1[]       = {0.18, 0.21, 0.312, 0.32, 0.32, 0.23, 0.21, 0.2};
    float ahigh[]    = {-0.63, -0.56, -0.83, -0.37, -0.67, -0.48, -0.76, -0.32};
    float Td[]       = {0.0552, 0.04366, 0.04340, 0.04370,
                        .0552,  0.04423, 0.04367, 0.0432};

    float ins[NCHANNELS][8192];
    float outs[NCHANNELS][8192];
    float *in[2], *out[2];
    float y[MAXSPRINGS];

    for (int c = 0; c < NCHANNELS; ++c) {
        in[c]  = &ins[c][0];
        out[c] = &outs[c][0];
    }

    memset(ins, 0, sizeof(ins));

    springs_t springs;
    springs_init(&springs, samplerate);
    springs_set_ftr(&springs, ftr);
    springs_set_a1(&springs, a1);
    springs_set_dccutoff(&springs, cutoff);
    springs_set_Nripple(&springs, 0.5);
    springs_set_Td(&springs, Td);
    springs_set_ahigh(&springs, ahigh);

    printf("%d tests of %d samples\n", r, n);

    double time = 0, dtime = 0;
    for (int i = 0; i < n * r; i++) {
        dtime = -omp_get_wtime();
        springs_lowallpasschain(&springs, y);
        dtime += omp_get_wtime();
        time += dtime;
    }
    printf("low allpasschain: %.5f\n", time);

    time = 0, dtime = 0;
    for (int i = 0; i < n * r; i++) {
        dtime = -omp_get_wtime();
        springs_lowdelayline(&springs, y);
        dtime += omp_get_wtime();
        time += dtime;
    }
    printf("low delay line: %.5f\n", time);

    time = 0, dtime = 0;
    for (int i = 0; i < n * r; i++) {
        dtime = -omp_get_wtime();
        springs_lowdc(&springs, y);
        dtime += omp_get_wtime();
        time += dtime;
    }
    printf("low dc filter: %.5f\n", time);

    time = 0, dtime = 0;
    for (int i = 0; i < n * r; i++) {
        dtime = -omp_get_wtime();
        springs_loweq(&springs, y);
        dtime += omp_get_wtime();
        time += dtime;
    }
    printf("low eq: %.5f\n", time);

    time = 0, dtime = 0;
    for (int i = 0; i < n * r; i++) {
        dtime = -omp_get_wtime();
        springs_lowlpf(&springs, y);
        dtime += omp_get_wtime();
        time += dtime;
    }
    printf("low lpf: %.5f\n", time);

    time  = 0;
    dtime = 0;
    for (int i = 0; i < n * r; i++) {
        dtime = -omp_get_wtime();
        springs_highallpasschain(&springs, y);
        dtime += omp_get_wtime();
        time += dtime;
    }
    printf("high allpass: %.5f\n", time);

    time  = 0;
    dtime = 0;
    for (int i = 0; i < n * r; i++) {
        dtime = -omp_get_wtime();
        springs_highdelayline(&springs, y);
        dtime += omp_get_wtime();
        time += dtime;
    }
    printf("high delayline: %.5f\n", time);

    time  = 0;
    dtime = 0;
    for (int i = 0; i < r; i++) {
        dtime = -omp_get_wtime();
        springs_process(&springs, in, out, n);
        dtime += omp_get_wtime();
        time += dtime;
    }
    printf("springs process: %.5f\n", time);
}
