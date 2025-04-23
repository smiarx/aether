#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_gui_basics/juce_gui_basics.h"

#include "../PluginProcessor.h"
#include "ComboBox.h"
#include "Led.h"
#include "Widgets.h"

namespace aether
{

class DelaySection : public juce::Component
{
    enum {
        kDryWet     = 0,
        kTime       = 1,
        kFeedback   = 2,
        kCutLow     = 3,
        kCutHi      = 4,
        kSaturation = 5,
        kDrift      = 6,
        kNumParams,
    };

    static constexpr std::array<std::tuple<const char *, const char *>,
                                kNumParams>
        kElements{{
            {"delay_drywet", "Dry/Wet"},
            {"delay_seconds", "Time"},
            {"delay_feedback", "Feedback"},
            {"delay_cutoff_low", "Low pass"},
            {"delay_cutoff_hi", "High pass"},
            {"delay_saturation", "Gain"},
            {"delay_drift", "Drift"},
        }};

  public:
    enum ColourIds {
        kBackgroundColourId = 0x1312000,
    };

    DelaySection(PluginProcessor &processor);

    void resized() override;
    void paint(juce::Graphics &g) override;

  private:
    SliderWithLabel sliders_[kElements.size()];

    juce::ToggleButton active_;
    juce::AudioProcessorValueTreeState::ButtonAttachment activeAttachment_;

    ComboBox mode_;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment modeAttachment_;

    bool useBeats_{false};
    ComboBox timeType_;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment timeTypeAttachment_;

    Led led_;
};

} // namespace aether
