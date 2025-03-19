#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Springs.h"
#include "TapeDelay.h"

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
    bool hasEditor() const override { return true; }
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

    //=============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    //=============================================================================
    enum class ParamId {
        DelayActive,
        DelayDrywet,
        DelayTimeType,
        DelaySeconds,
        DelayBeats,
        DelayFeedback,
        DelayCutLow,
        DelayCutHi,
        DelaySaturation,
        DelayDrift,
        DelayMode,
        SpringsActive,
        SpringsDryWet,
        SpringsWidth,
        SpringsLength,
        SpringsDecay,
        SpringsDamp,
        SpringsShape,
        SpringsDiff,
        SpringsScatter,
        SpringsChaos,
    };

    enum BeatMult {
        Beat1_32,
        Beat1_16,
        Beat1_8,
        Beat1_4,
        Beat1_3,
        Beat1_2,
        Beat1,
        Beat2,
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
    void parameterValueChanged(int id, float newValue) override;
    void parameterGestureChanged(int /*parameterIndex*/,
                                 bool /*gestureIsStarting*/) override
    {
    }

    juce::AudioProcessorValueTreeState &getAPVTS() { return m_parameters; }

  private:
    juce::AudioProcessorValueTreeState m_parameters;
    std::deque<ParamEvent> m_paramEvents;

    bool m_activeTapeDelay{true};
    bool m_activeSprings{true};

    bool m_useBeats{false};
    double m_beatsMult{1};
    double m_bpm{120};

    processors::TapeDelay m_tapedelay;
    processors::Springs m_springs;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
