#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "readerwriterqueue.h"

#include "Presets/PresetManager.h"

#include "Springs.h"
#include "TapeDelay.h"

namespace aether
{

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
        SpringsTone,
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
        ParamEvent() = default;
        ParamEvent(int t_id, float t_value) :
            id(static_cast<ParamId>(t_id)), value(t_value)
        {
        }
        ParamId id{};
        float value{};
    };

    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void addProcessorAsListener(juce::AudioProcessorParameter *param);
    void parameterValueChanged(int id, float newValue) override;
    void parameterGestureChanged(int /*parameterIndex*/,
                                 bool /*gestureIsStarting*/) override
    {
    }

    juce::AudioProcessorValueTreeState &getAPVTS() { return m_parameters; }
    const juce::AudioProcessorValueTreeState &getAPVTS() const
    {
        return m_parameters;
    }

    auto &getSprings() const { return m_springs; }

    const auto *getRMSStack() const { return m_springs.getRMSStack(); }
    const auto *getRMSStackPos() const { return &m_rmsPos; }

    auto &getSwitchIndicator() { return m_tapedelay.getSwitchIndicator(); }

    PresetManager &getPresetManager() { return m_presetManager; }

  private:
    juce::AudioProcessorValueTreeState m_parameters;
    PresetManager m_presetManager{m_parameters};
    moodycamel::ReaderWriterQueue<ParamEvent> m_paramEvents{32};

    bool m_activeTapeDelay{true};
    bool m_activeSprings{true};

    // atomics
    std::atomic<int> m_rmsPos{0};

    bool m_useBeats{false};
    double m_beatsMult{1};
    double m_bpm{120};

    // used to sync reverse delay
    bool m_isPlaying{false};
    double m_nextSync{-1};

    processors::TapeDelay m_tapedelay;
    processors::Springs m_springs;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
} // namespace aether
