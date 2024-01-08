#ifndef _SPRINGREVERB_H
#define _SPRINGREVERB_H

#ifndef MAXSPRINGS
#define MAXSPRINGS 8
#endif
#ifndef NSPRINGS
#define NSPRINGS MAXSPRINGS
#endif

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
#define MLOWBUFSIZE 32
#define MLOWBUFMASK (MLOWBUFSIZE - 1)

#define MLOWEQSIZE (MLOWBUFSIZE * 2)
#define MLOWEQMASK (MLOWEQSIZE - 1)

#define LOWDELAY1SIZE      2048
#define LOWDELAY1MASK      (LOWDELAY1SIZE - 1)
#define LOWDELAYECHOSIZE   512
#define LOWDELAYECHOMASK   (LOWDELAYECHOSIZE - 1)
#define LOWDELAYRIPPLESIZE 128
#define LOWDELAYRIPPLEMASK (LOWDELAYRIPPLESIZE - 1)

#define gecho   0.01f
#define gripple 0.01f
#define glf     -0.95f

#define amod 0.997f
#define gmod 8.3f

#define MHIGH         158
#define HIGHDELAYSIZE LOWDELAY1SIZE
#define HIGHDELAYMASK (HIGHDELAYSIZE - 1)
#define ghf           -0.94f

#define glow  1.0f
#define ghigh (glow / 1000.f)

#define glow2high (0.99f * glow)
#define ghigh2low (0.4 * ghigh)

// filter
#define FILTERMEMSIZE 4
#define FILTERMEMMASK (FILTERMEMSIZE - 1)
#define NAASOS        5 // number of aa 2nd order filter
#define NLOWPASSSOS   5 // number of lowpass 2nd order filter

#ifdef __cplusplus
#define springparam(type, name) type name[MAXSPRINGS]
#else
#define springparam(type, name) \
    type name[MAXSPRINGS] __attribute__((aligned(sizeof(type) * MAXSPRINGS)))
#endif

typedef struct {
    /* down sampling */
    int downsampleM;
    int downsampleid;
    springparam(float, aasos[NAASOS][2][3]);
    springparam(float, aamem[NAASOS][FILTERMEMSIZE]);
    int aaid;

    /* set with ftr */
    springparam(float, K);
    springparam(int, iK);
    springparam(float, a2);
    /*--------*/

    springparam(float, a1);

    springparam(float, adc);
    springparam(float, dcmem);

    /* modulation */
    springparam(int, randseed);
    springparam(int, randstate);
    springparam(float, Lmodmem);

    struct {
        springparam(float, mem2);
        springparam(float, mem1[MLOWBUFSIZE]);
    } lowstate[MLOW];
    int lowbufid;

    springparam(float, L1);
    springparam(float, Lecho);
    springparam(float, Lripple);
    springparam(float, lowdelay1[LOWDELAY1SIZE]);
    springparam(float, lowdelayecho[LOWDELAYECHOSIZE]);
    springparam(float, lowdelayripple[LOWDELAYRIPPLESIZE]);
    int lowdelay1id;
    int lowdelayechoid;
    int lowdelayrippleid;

    struct {
        springparam(int, Keq);
        springparam(float, b0);
        springparam(float, ak);
        springparam(float, a2k);
        springparam(float, mem[MLOWEQSIZE]);
        int id;
    } loweq;

    springparam(float, lowpasssos[NLOWPASSSOS][2][3]);
    springparam(float, lowpassmem[NLOWPASSSOS][FILTERMEMSIZE]);
    int lowpassmemid;

    springparam(float, ahigh);
    springparam(float, highmem[MHIGH]);

    springparam(int, iLhigh);
    springparam(float, fLhigh);
    springparam(float, highdelay[HIGHDELAYSIZE]);
    int highdelayid;

    float samplerate;
} springs_t;

void springs_init(springs_t *springs, float samplerate);
void springs_set_dccutoff(springs_t *springs, float *fcutoff);
void springs_set_ftr(springs_t *springs, float *ftr);
void springs_set_a1(springs_t *springs, float *a1);
void springs_set_Td(springs_t *springs, float *Td);
void springs_set_Nripple(springs_t *springs, float Nripple);
void springs_set_ahigh(springs_t *springs, float *a1);

#ifdef __cplusplus
#define restrict __restrict__
#endif

void springs_lowdelayline(springs_t *restrict springs, float *restrict y);
void springs_lowdc(springs_t *restrict springs, float *restrict y);
void springs_lowallpasschain(springs_t *restrict springs, float *restrict y);
void springs_lowlpf(springs_t *restrict springs, float *restrict y);
void springs_loweq(springs_t *restrict springs, float *restrict y);
void springs_highallpasschain(springs_t *restrict springs, float *restrict y);
void springs_highdelayline(springs_t *restrict springs, float *restrict y);
void springs_process(springs_t *restrict springs, float **restrict in,
                     float **restrict out, int count);

#endif
