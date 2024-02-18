#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

extern "C" {
#include "dsp/springreverb.h"
#include "dsp/tapedelay.h"
}

//==============================================================================
class PluginProcessor final : public juce::AudioProcessor,
                              juce::AudioProcessorParameter::Listener
{
  public:
    //==============================================================================
    PluginProcessor();
    ~PluginProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    //=============================================================================
    enum class ParamId {
        DelayDrywet,
        DelayTime,
        DelayFeedback,
        DelayCutoff,
        DelayDrive,
        DelayDrift,
        DelayDriftFreq,
        DelayMode,
        SpringsDryWet,
        SpringsHiLo,
        SpringsLength,
        SpringsDecay,
        SpringsDispersion,
        SpringsDamp,
        SpringsChaos,
        SpringsSpringness,
        SpringParamBegin,
        SpringVolume,
        SpringSolo,
        SpringMute,
        SpringSource,
        SpringPan,
        SpringHiLo,
        SpringLength,
        SpringDecay,
        SpringDispersion,
        SpringDamp,
        SpringChaos,
        SpringSpringness,
        SpringParamEnd,
    };

    struct ParamEvent {
        ParamEvent(int t_id, float t_value) :
            id(static_cast<ParamId>(t_id)), value(t_value)
        {
        }
        ParamId id;
        float value;
    };

    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void addProcessorAsListener(juce::AudioProcessorParameter *param);
    void parameterValueChanged(int id, float newValue);
    void parameterGestureChanged(int /*parameterIndex*/,
                                 bool /*gestureIsStarting*/)
    {
    }

  private:
    juce::AudioProcessorValueTreeState m_parameters;
    std::deque<ParamEvent> m_paramEvents;

    tapedelay_t m_tapedelay;
    springs_t m_springreverb;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
