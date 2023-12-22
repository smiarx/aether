/* ------------------------------------------------------------
name: "a"
Code generated with Faust 2.68.1 (https://faust.grame.fr)
Compilation options: -lang cpp -ct 1 -es 1 -mcd 16 -single -ftz 0
------------------------------------------------------------ */

#ifndef __tapedelay_H__
#define __tapedelay_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>

#ifndef FAUSTCLASS
#define FAUSTCLASS tapedelay
#endif

#ifdef __APPLE__
#define exp10f __exp10f
#define exp10  __exp10
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

#ifndef CHANS
#define CHANS 1
#endif
#ifndef AA
#define AA 1
#endif

FAUSTFLOAT hermite(FAUSTFLOAT ym1, FAUSTFLOAT y0, FAUSTFLOAT y1, FAUSTFLOAT y2,
                   float x)
{
    FAUSTFLOAT c0 = y0;
    FAUSTFLOAT c1 = .5 * (y1 - ym1);
    FAUSTFLOAT c2 = ym1 - 2.5 * y0 + 2. * y1 - .5 * y2;
    FAUSTFLOAT c3 = .5 * (y2 - ym1) + 1.5 * (y0 - y1);
    return c0 + (x * (c1 + x * (c2 + x * c3)));
}

bool compare(int64_t x, int64_t y)
{
    /* transform into unsigned to allow overflow */
    uint64_t tmp = -y;
    int64_t diff = x + tmp;
    return diff > 0;
}

class tapedelay : public dsp
{

  private:
    int fSampleRate;

    static constexpr auto tablesize = 1 << 16;
    static constexpr auto mask      = tablesize - 1;
    static constexpr auto Q         = 61;
    static constexpr uint64_t unit  = ((uint64_t)1) << Q;
    struct {
        uint64_t V;
        FAUSTFLOAT y[CHANS];
    } m_ringbuf[tablesize];

    FAUSTFLOAT m_delay;
    FAUSTFLOAT m_feedback;
#if CHANS == 2
    FAUSTFLOAT m_pingpong;
#endif

    size_t m_nr, m_nw;
    float m_prevfnr;
    float m_speed;

    FAUSTFLOAT m_ym1[CHANS], m_y0[CHANS], m_y1[CHANS], m_y2[CHANS];

  public:
    tapedelay() {}

    void metadata(Meta *m) {}

    virtual int getNumInputs() { return CHANS; }
    virtual int getNumOutputs() { return CHANS; }

    static void classInit(int sample_rate) {}

    virtual void instanceConstants(int sample_rate)
    {
        fSampleRate = sample_rate;
    }

    virtual void instanceResetUserInterface()
    {
        m_delay    = 0.5;
        m_feedback = 0.7;
#if CHANS == 2
        m_pingpong = 0.;
#endif
    }

    virtual void instanceClear()
    {
        memset(m_ringbuf, 0, tablesize * sizeof(m_ringbuf[0]));
        m_ringbuf[0].V = -unit;
        m_ringbuf[1].V = 0;
        m_nw           = 2;
        m_nr           = 0;
        m_prevfnr      = 0.;
        m_speed        = float(unit) / (m_delay * fSampleRate);

        for (auto c = 0; c < CHANS; ++c)
            m_ym1[c] = 0, m_y0[c] = 0, m_y1[c] = 0, m_y2[c] = 0;
    }

    virtual void init(int sample_rate)
    {
        classInit(sample_rate);
        instanceInit(sample_rate);
    }

    virtual void instanceInit(int sample_rate)
    {
        instanceConstants(sample_rate);
        instanceResetUserInterface();
        instanceClear();
    }

    virtual tapedelay *clone() { return new tapedelay(); }

    virtual int getSampleRate() { return fSampleRate; }

    virtual void buildUserInterface(UI *ui_interface)
    {
        ui_interface->openVerticalBox("tapedelay");
        ui_interface->declare(&m_delay, "0", "");
        ui_interface->addHorizontalSlider("delay", &m_delay, FAUSTFLOAT(0.5f),
                                          FAUSTFLOAT(0.01f), FAUSTFLOAT(1.0f),
                                          FAUSTFLOAT(0.01f));
        ui_interface->declare(&m_feedback, "1", "");
        ui_interface->addHorizontalSlider("feedback", &m_feedback,
                                          FAUSTFLOAT(0.7f), FAUSTFLOAT(0.f),
                                          FAUSTFLOAT(1.0f), FAUSTFLOAT(0.001f));
#if CHANS == 2
        ui_interface->declare(&m_pingpong, "2", "");
        ui_interface->addCheckButton("pingpong", &m_pingpong);
#endif
        ui_interface->closeBox();
    }

    virtual void compute(int count, FAUSTFLOAT **RESTRICT inputs,
                         FAUSTFLOAT **RESTRICT outputs)
    {

        auto speedf           = float(unit) / (m_delay * fSampleRate);
        auto constexpr speeda = 0.999;

        for (int i0 = 0; i0 < count; i0 = i0 + 1) {
            m_speed        = speeda * m_speed + (1 - speeda) * speedf;
            uint64_t speed = (uint64_t)m_speed;
            auto xw        = m_ringbuf[m_nw].V + speed;
            auto xr        = xw - unit;

            auto nr           = m_nr;
            size_t searchsize = 2;

            for (; compare(xr, m_ringbuf[(nr + searchsize) & mask].V);
                 nr += searchsize, searchsize *= 2) {
            }

            while (searchsize > 1) {
                auto searchsize2 = searchsize / 2;
                if (compare(xr, m_ringbuf[(nr + searchsize2) & mask].V))
                    nr += searchsize2;
                searchsize = searchsize2;
            }

            nr        = nr & mask;
            float fnr = float(xr - m_ringbuf[nr].V) /
                        (m_ringbuf[(nr + 1) & mask].V - m_ringbuf[nr].V);

#if AA == 1
            auto playbackspeed = ((nr - m_nr) & mask) + fnr - m_prevfnr;
            m_prevfnr          = fnr;

            if (playbackspeed <= 1.01) {
                if (m_nr != nr) {
                    m_nr = nr;
                    for (auto c = 0; c < CHANS; ++c) {
                        m_ym1[c] = m_y0[c], m_y0[c] = m_y1[c],
                        m_y1[c] = m_y2[c];
                        m_y2[c] = m_ringbuf[(m_nr + 2) & mask].y[c];
                    }
                }
            } else {
                auto a = 1. / playbackspeed;
                do {
                    m_nr = (m_nr + 1) & mask;
                    for (auto c = 0; c < CHANS; ++c) {
                        m_ym1[c] = m_y0[c], m_y0[c] = m_y1[c],
                        m_y1[c] = m_y2[c];
                        /* filter */
                        m_y2[c] = (1 - a) *
                                      (m_y2[c] + m_y1[c] + m_y0[c] + m_ym1[c]) /
                                      4. +
                                  a * m_ringbuf[(m_nr + 2) & mask].y[c];
                    }
                } while (compare(nr, m_nr));
            }
            for (int c = 0; c < CHANS; ++c) {
                auto ym1 = m_ym1[c];
                auto y0  = m_y0[c];
                auto y1  = m_y1[c];
                auto y2  = m_y2[c];

#else
            m_nr = nr;
            for (int c = 0; c < CHANS; ++c) {
                auto ym1 = m_ringbuf[(nr - 1) & mask].y[c];
                auto y0  = m_ringbuf[nr].y[c];
                auto y1  = m_ringbuf[(nr + 1) & mask].y[c];
                auto y2  = m_ringbuf[(nr + 2) & mask].y[c];
#endif
                auto y = hermite(ym1, y0, y1, y2, fnr);

                auto c0 = c;
#if CHANS == 2
                if (m_pingpong == 1) c0 ^= 1;
#endif
                m_ringbuf[m_nw].y[c0] = inputs[c][i0] + (m_feedback * y);
                outputs[c][i0]        = FAUSTFLOAT(y);
            }

            m_nw              = (m_nw + 1) & mask;
            m_ringbuf[m_nw].V = xw;
        }
    }
};

#endif
