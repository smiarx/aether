#pragma once

#include "PluginProcessor.h"

#include "Springs.h"
#include "TapeDelay.h"

#if DSP_AVX
#define LOADFUNC loadPluginAVX2
#else
#define LOADFUNC loadPluginDefault
#endif

namespace aelapse::DSP_ARCH_NAMESPACE
{

class PluginProcessor final : public aelapse::PluginProcessor
{
  public:
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    using AudioProcessor::processBlock;

    const float *getRMSStack() const override
    {
        return (float *)springs_.getRMSStack();
    }
    std::atomic<bool> &getSwitchIndicator() override
    {
        return tapedelay_.getSwitchIndicator();
    }

  private:
    processors::TapeDelay tapedelay_;
    processors::Springs springs_;
};

} // namespace aelapse::DSP_ARCH_NAMESPACE
