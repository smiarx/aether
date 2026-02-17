#include "PluginProcessor.h"
#include <dsp/cpu/infos.h>

//==============================================================================
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
#if DSP_X86_64 && DSP_X86_DISPATCH && !DSP_AVX2
    auto infos = dsp::cpu::getInfos();

    if (infos.avx2 && infos.fma3_sse42) {
        return aelapse::loadPluginAVX2();
    }
#endif

    return aelapse::loadPlugin();
}
