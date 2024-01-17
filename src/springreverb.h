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

#define LOWDELAY1SIZE      (2048 << 1)
#define LOWDELAY1MASK      (LOWDELAY1SIZE - 1)
#define LOWDELAYECHOSIZE   (512 << 1)
#define LOWDELAYECHOMASK   (LOWDELAYECHOSIZE - 1)
#define LOWDELAYRIPPLESIZE (128 << 1)
#define LOWDELAYRIPPLEMASK (LOWDELAYRIPPLESIZE - 1)

#define gmod 10.f
#define amod (1.f - 0.05f / gmod)

#define MHIGH         70
#define HIGHSTRETCH   2
#define HIGHDELAYSIZE LOWDELAY1SIZE
#define HIGHDELAYMASK (HIGHDELAYSIZE - 1)

#define glow2high (0.0f)
#define ghigh2low (0.0f)

// filter
#define FILTERMEMSIZE 4
#define FILTERMEMMASK (FILTERMEMSIZE - 1)
#define NAASOS        5 // number of aa 2nd order filter
#define NLOWPASSSOS   5 // number of lowpass 2nd order filter

#define MAXBLOCKSIZE 512

#define springparam(type, name) \
    type name[MAXSPRINGS] __attribute__((aligned(sizeof(type) * MAXSPRINGS)))

/* spring parameters */
typedef struct {
    springparam(float, ftr);
    springparam(float, a1);
    springparam(float, ahigh);
    springparam(float, Td);
    springparam(float, fcutoff);
    springparam(float, gripple);
    springparam(float, gecho);
    springparam(float, glf);
    springparam(float, ghf);
    springparam(float, vol);
    springparam(float, hilomix);
} springs_desc_t;

typedef struct {
    springs_desc_t desc;

    /* block compute */
    int blocksize;
    springparam(float, ylow[MAXBLOCKSIZE]);
    springparam(float, yhigh[MAXBLOCKSIZE]);

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
    springparam(float, gecho);
    springparam(float, gripple);
    springparam(float, glf);
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
    springparam(float, highmem[MHIGH][HIGHSTRETCH]);
    int highmemid;

    springparam(float, Lmodhighmem);
    springparam(float, Lhigh);
    springparam(float, highdelay[HIGHDELAYSIZE]);
    springparam(float, ghf);
    int highdelayid;

    /* low and high gain */
    springparam(float, glow);
    springparam(float, ghigh);

    float samplerate;
} springs_t;

#ifdef __cplusplus
#define restrict
#endif

void springs_init(springs_t *springs, springs_desc_t *desc, float samplerate);
void springs_update(springs_t *springs, springs_desc_t *desc);
void springs_set_dccutoff(springs_t *springs,
                          float fcutoff[restrict MAXSPRINGS]);
void springs_set_ftr(springs_t *springs, float ftr[restrict MAXSPRINGS]);
void springs_set_a1(springs_t *springs, float a1[restrict MAXSPRINGS]);
void springs_set_Td(springs_t *springs, float Td[restrict MAXSPRINGS]);
void springs_set_Nripple(springs_t *springs, float Nripple);
void springs_set_ahigh(springs_t *springs, float a1[restrict MAXSPRINGS]);
void springs_set_gripple(springs_t *springs,
                         float gripple[restrict MAXSPRINGS]);
void springs_set_gecho(springs_t *springs, float gecho[restrict MAXSPRINGS]);
void springs_set_glf(springs_t *springs, float glf[restrict MAXSPRINGS]);
void springs_set_ghf(springs_t *springs, float ghf[restrict MAXSPRINGS]);
void springs_set_vol(springs_t *springs, float vol[restrict MAXSPRINGS]);
void springs_set_hilomix(springs_t *springs,
                         float hilomix[restrict MAXSPRINGS]);

void springs_lowdelayline(springs_t *restrict springs,
                          float y[restrict MAXSPRINGS]);
void springs_lowdc(springs_t *restrict springs, float y[restrict MAXSPRINGS]);
void springs_lowallpasschain(springs_t *restrict springs,
                             float y[restrict MAXSPRINGS]);
void springs_lowlpf(springs_t *restrict springs, float y[restrict MAXSPRINGS]);
void springs_loweq(springs_t *restrict springs, float y[restrict MAXSPRINGS]);
void springs_highallpasschain(springs_t *restrict springs,
                              float y[restrict MAXSPRINGS]);
void springs_highdelayline(springs_t *restrict springs,
                           float y[restrict MAXSPRINGS]);
void springs_process(springs_t *restrict springs, float **restrict in,
                     float **restrict out, int count);

#endif
