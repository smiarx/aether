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
        DryWet     = 0,
        Time       = 1,
        Feedback   = 2,
        CutLow     = 3,
        CutHi      = 4,
        Saturation = 5,
        Drift      = 6,
        _NumParams,
    };

    static constexpr std::array<std::tuple<const char *, const char *>,
                                _NumParams>
        elements{{
            {"delay_drywet", "dry/wet"},
            {"delay_seconds", ""},
            {"delay_feedback", "feedback"},
            {"delay_cutoff_low", "low"},
            {"delay_cutoff_hi", "high"},
            {"delay_saturation", "gain"},
            {"delay_drift", "drift"},
        }};

  public:
    enum ColourIds {
        backgroundColourId = 0x1312000,
    };

    DelaySection(PluginProcessor &processor, APVTS &apvts);

    virtual void resized() override;
    virtual void paint(juce::Graphics &g) override;

  private:
    SliderWithLabel m_sliders[elements.size()];

    juce::ToggleButton m_active;
    juce::AudioProcessorValueTreeState::ButtonAttachment m_activeAttachment;

    ComboBox m_mode;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment m_modeAttachment;

    bool m_useBeats{false};
    ComboBox m_timeType;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment m_timeTypeAttachment;

    Led m_led;
};

} // namespace aether
