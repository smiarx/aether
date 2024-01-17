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

    float samplerate    = 48000;
    springs_desc_t desc = {
        .ftr   = {4210, 4106, 4200, 4300, 4330, 4118, 4190, 4310},
        .a1    = {0.18, 0.21, 0.312, 0.32, 0.32, 0.23, 0.21, 0.2},
        .ahigh = {-0.63, -0.56, -0.83, -0.37, -0.67, -0.48, -0.76, -0.32},
        //.Td = {0.0552,0.04366,0.04340,0.04370,.0552,0.04423,0.04367,0.0432},
        .Td = {0.052, 0.054, 0.046, 0.048, 0.050, 0.05612, 0.04983, 0.051291},
        .fcutoff = {40, 40, 38, 20, 49, 31, 32, 33},
        .gripple = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01},
        .gecho   = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01},
        .glf     = {-0.98, -0.98, -0.98, -0.98, -0.98, -0.98, -0.98, -0.98},
        .ghf     = {-0.98, -0.98, -0.98, -0.98, -0.98, -0.98, -0.986, -0.98},
    };

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
    springs_init(&springs, &desc, samplerate);

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
