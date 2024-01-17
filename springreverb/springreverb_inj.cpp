/* ------------------------------------------------------------
name: "a"
Code generated with Faust 2.69.3 (https://faust.grame.fr)
Compilation options: -lang cpp -ct 1 -cn springreverb -es 1 -mcd 16 -single -ftz
0
------------------------------------------------------------ */

#ifndef __springreverb_H__
#define __springreverb_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>

#ifndef FAUSTCLASS
#define FAUSTCLASS springreverb
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
#include "springreverb.h"
}

class springreverb : public dsp
{

  private:
    FAUSTFLOAT fButton0;
    float fVec0[2];
    int fSampleRate;

    springs_t springs;
    springs_desc_t desc;

  public:
    springreverb() {}

    void metadata(Meta *m)
    {
        m->declare("basics.lib/name", "Faust Basic Element Library");
        m->declare("basics.lib/tabulateNd",
                   "Copyright (C) 2023 Bart Brouns <bart@magnetophon.nl>");
        m->declare("basics.lib/version", "1.11.1");
        m->declare(
            "compile_options",
            "-lang cpp -ct 1 -cn springreverb -es 1 -mcd 16 -single -ftz 0");
        m->declare("filename", "a.dsp");
        m->declare("name", "a");
    }

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

        springs_init(&springs, &desc, sample_rate);
    }

    virtual springreverb *clone() { return new springreverb(); }

    virtual int getSampleRate() { return fSampleRate; }

    virtual void buildUserInterface(UI *ui_interface)
    {
        ui_interface->openVerticalBox("springreverb");

        for (int i = 0; i < MAXSPRINGS; ++i) {
            char name[16];
            sprintf(name, "spring %d", i);
            ui_interface->openHorizontalBox(name);
            ui_interface->addHorizontalSlider(
                "freq", &desc.ftr[i], FAUSTFLOAT(4.5e+03f), FAUSTFLOAT(2e+02f),
                FAUSTFLOAT(1e+04f), FAUSTFLOAT(1.0f));
            ui_interface->addHorizontalSlider(
                "a1", &desc.a1[i], FAUSTFLOAT(0.4), FAUSTFLOAT(0.f),
                FAUSTFLOAT(1.f), FAUSTFLOAT(0.0001f));
            ui_interface->addHorizontalSlider(
                "ahigh", &desc.ahigh[i], FAUSTFLOAT(-0.4f), FAUSTFLOAT(-1.f),
                FAUSTFLOAT(0.f), FAUSTFLOAT(0.0001f));
            ui_interface->addHorizontalSlider(
                "Td", &desc.Td[i], FAUSTFLOAT(0.05f), FAUSTFLOAT(0.02),
                FAUSTFLOAT(0.2), FAUSTFLOAT(0.0001f));
            ui_interface->addHorizontalSlider(
                "fcutoff", &desc.fcutoff[i], FAUSTFLOAT(20.f), FAUSTFLOAT(1.f),
                FAUSTFLOAT(80.f), FAUSTFLOAT(0.0001f));
            ui_interface->addHorizontalSlider(
                "gripple", &desc.gripple[i], FAUSTFLOAT(0.01f), FAUSTFLOAT(0.f),
                FAUSTFLOAT(1.f), FAUSTFLOAT(0.0001f));
            ui_interface->addHorizontalSlider(
                "gecho", &desc.gecho[i], FAUSTFLOAT(0.01f), FAUSTFLOAT(0.f),
                FAUSTFLOAT(1.f), FAUSTFLOAT(0.0001f));
            ui_interface->addHorizontalSlider(
                "glf", &desc.glf[i], FAUSTFLOAT(0.95f), FAUSTFLOAT(0.f),
                FAUSTFLOAT(1.f), FAUSTFLOAT(0.0001f));
            ui_interface->addHorizontalSlider(
                "ghf", &desc.ghf[i], FAUSTFLOAT(0.95f), FAUSTFLOAT(0.f),
                FAUSTFLOAT(1.f), FAUSTFLOAT(0.0001f));
            ui_interface->addHorizontalSlider(
                "hilomix", &desc.hilomix[i], FAUSTFLOAT(0.f), FAUSTFLOAT(0.f),
                FAUSTFLOAT(1.f), FAUSTFLOAT(0.0001f));
            ui_interface->addHorizontalSlider(
                "volume", &desc.vol[i], FAUSTFLOAT(0.0), FAUSTFLOAT(-80.f),
                FAUSTFLOAT(0.f), FAUSTFLOAT(0.0001f));
            ui_interface->closeBox();
        }
        ui_interface->closeBox();
    }

    virtual void compute(int count, FAUSTFLOAT **RESTRICT inputs,
                         FAUSTFLOAT **RESTRICT outputs)
    {
        springs_update(&springs, &desc);
        springs_process(&springs, inputs, outputs, count);
    }
};

#endif
