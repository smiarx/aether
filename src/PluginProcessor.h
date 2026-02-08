#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "readerwriterqueue.h"

#include "Presets/PresetManager.h"
#include "dsp/cpu/infos.h"

#include "TapeDelay.h"

namespace aelapse
{

//==============================================================================
class PluginProcessor : public juce::AudioProcessor,
                        juce::AudioProcessorParameter::Listener
{
  public:
    //==============================================================================
    PluginProcessor();
    ~PluginProcessor() override;

    //==============================================================================
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

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
        kDelayActive,
        kDelayDrywet,
        kDelayTimeType,
        kDelaySeconds,
        kDelayBeats,
        kDelayFeedback,
        kDelayCutLow,
        kDelayCutHi,
        kDelaySaturation,
        kDelayDrift,
        kDelayMode,
        kSpringsActive,
        kSpringsDryWet,
        kSpringsWidth,
        kSpringsLength,
        kSpringsDecay,
        kSpringsDamp,
        kSpringsShape,
        kSpringsTone,
        kSpringsScatter,
        kSpringsChaos,
    };

    enum BeatMult {
        kBeat132,
        kBeat116,
        kBeat18,
        kBeat16,
        kBeat14,
        kBeat13,
        kBeat12,
        kBeat1,
        kBeat2,
        kBeat4,
    };

    struct ParamEvent {
        ParamEvent() = default;
        ParamEvent(int tId, float tValue) :
            id(static_cast<ParamId>(tId)), value(tValue)
        {
        }
        ParamId id{};
        float value{};
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void addProcessorAsListener(juce::AudioProcessorParameter *param);
    void parameterValueChanged(int id, float newValue) override;
    void parameterGestureChanged(int /*parameterIndex*/,
                                 bool /*gestureIsStarting*/) override
    {
    }

    juce::AudioProcessorValueTreeState &getAPVTS() { return parameters_; }
    const juce::AudioProcessorValueTreeState &getAPVTS() const
    {
        return parameters_;
    }

    virtual const float *getRMSStack() const = 0;
    const auto *getRMSStackPos() const { return &rmsPos_; }

    virtual std::atomic<bool> &getSwitchIndicator() = 0;
    auto *getShakeAtomic() { return &shake_; }

    PresetManager &getPresetManager() { return presetManager_; }

  private:
    juce::AudioProcessorValueTreeState parameters_;
    PresetManager presetManager_{parameters_};

  protected:
    moodycamel::ReaderWriterQueue<ParamEvent> paramEvents_{32};

    bool activeTapeDelay_{true};
    bool activeSprings_{true};

    // atomics
    std::atomic<int> rmsPos_{0};
    std::atomic<bool> shake_{};

    bool useBeats_{false};
    bool isDotted_{false};
    double beatsMult_{1};
    double bpm_{120};

    // used to sync reverse delay
    bool isPlaying_{false};
    double nextSync_{-1};

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};

#if DSP_AVX || DSP_X86_DISPATCH
juce::AudioProcessor *loadPluginAVX2();
#endif
#if !DSP_AVX
juce::AudioProcessor *loadPluginDefault();
#endif
} // namespace aelapse
