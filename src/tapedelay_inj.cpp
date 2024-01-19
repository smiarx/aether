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

extern "C" {
#include "tapedelay.h"
}

class tapedelay : public dsp
{

  private:
    FAUSTFLOAT fButton0;
    float fVec0[2];
    int fSampleRate;

    tapedelay_t m_tapedelay;
    tapedelay_desc_t m_desc;

  public:
    tapedelay() {}

    void metadata(Meta *m) { m->declare("name", "tapedelay"); }

    virtual int getNumInputs() { return 2; }
    virtual int getNumOutputs() { return 2; }

    static void classInit(int sample_rate) {}

    virtual void instanceConstants(int sample_rate)
    {
        fSampleRate = sample_rate;
    }

    virtual void instanceResetUserInterface() { fButton0 = FAUSTFLOAT(0.0f); }

    virtual void instanceClear()
    {
        for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
            fVec0[l0] = 0.0f;
        }
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

        tapedelay_init(&m_tapedelay, &m_desc, sample_rate);
    }

    virtual tapedelay *clone() { return new tapedelay(); }

    virtual int getSampleRate() { return fSampleRate; }

    virtual void buildUserInterface(UI *ui_interface)
    {
        ui_interface->openVerticalBox("tapedelay");

        ui_interface->addHorizontalSlider("delay", &m_desc.delay,
                                          FAUSTFLOAT(0.200), FAUSTFLOAT(0.01),
                                          FAUSTFLOAT(1.0), FAUSTFLOAT(0.001));
        ui_interface->addHorizontalSlider(
            "feedback", &m_tapedelay.desc.feedback, FAUSTFLOAT(0.8f),
            FAUSTFLOAT(0.f), FAUSTFLOAT(1.f), FAUSTFLOAT(0.001f));
        ui_interface->addHorizontalSlider("drywet", &m_tapedelay.desc.drywet,
                                          FAUSTFLOAT(0.1f), FAUSTFLOAT(0.f),
                                          FAUSTFLOAT(1.f), FAUSTFLOAT(0.001f));
        ui_interface->addCheckButton("reverse", &m_desc.reverse);
        ui_interface->closeBox();
    }

    virtual void compute(int count, FAUSTFLOAT **RESTRICT inputs,
                         FAUSTFLOAT **RESTRICT outputs)
    {
        tapedelay_update(&m_tapedelay, &m_desc);
        tapedelay_process(&m_tapedelay, inputs, outputs, count);
    }
};

#endif
