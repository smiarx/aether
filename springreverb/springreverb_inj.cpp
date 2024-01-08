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
#include "springreverb.c"
#include "springreverb.h"
}

class springreverb : public dsp
{

  private:
    FAUSTFLOAT fButton0;
    float fVec0[2];
    int fSampleRate;

    springs_t springs;

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

        float cutoff[]  = {40, 40, 38, 20, 49, 31, 32, 33};
        float ftr[]     = {4210, 4106, 4200, 4300, 3330, 4118, 4190, 4310};
        float a1[]      = {0.18, 0.21, 0.312, 0.32, 0.32, 0.23, 0.21, 0.2};
        float ahigh[]   = {-0.63, -0.56, -0.83, -0.37,
                           -0.67, -0.48, -0.76, -0.32};
        float Td[]      = {0.0552, 0.04366, 0.04340, 0.04370,
                           .0552,  0.04423, 0.04367, 0.0432};
        float gripple[] = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01};
        float gecho[]   = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01};
        float glf[] = {-0.95, -0.95, -0.95, -0.97, -0.97, -0.95, -0.95, -0.95};
        float ghf[] = {-0.94, -0.94, -0.94, -0.94, -0.94, -0.94, -0.946, -0.94};

        springs_init(&springs, sample_rate);
        springs_set_ftr(&springs, ftr);
        springs_set_a1(&springs, a1);
        springs_set_dccutoff(&springs, cutoff);
        springs_set_Nripple(&springs, 0.5);
        springs_set_Td(&springs, Td);
        springs_set_ahigh(&springs, ahigh);
        springs_set_gripple(&springs, gripple);
        springs_set_gecho(&springs, gecho);
        springs_set_glf(&springs, glf);
        springs_set_ghf(&springs, ghf);
    }

    virtual springreverb *clone() { return new springreverb(); }

    virtual int getSampleRate() { return fSampleRate; }

    virtual void buildUserInterface(UI *ui_interface)
    {
        ui_interface->openVerticalBox("a");
        ui_interface->addButton("impulse", &fButton0);
        ui_interface->closeBox();
    }

    virtual void compute(int count, FAUSTFLOAT **RESTRICT inputs,
                         FAUSTFLOAT **RESTRICT outputs)
    {
        for (int i = 0; i < count; ++i) {
            float temp = fButton0 - fVec0[1];
            inputs[0][0] += temp * float(temp > 0);
            inputs[1][0] += temp * float(temp > 0);
            fVec0[1] = fButton0;
        }
        springs_process(&springs, inputs, outputs, count);
    }
};

#endif
