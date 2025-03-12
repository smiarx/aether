#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_gui_basics/juce_gui_basics.h"

#include "Widgets.h"

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
            {"delay_drywet", "Dry/Wet"},
            {"delay_time", "Time"},
            {"delay_feedback", "Feedback"},
            {"delay_cutoff_low", "Low Pass"},
            {"delay_cutoff_hi", "High Pass"},
            {"delay_saturation", "Saturation"},
            {"delay_drift", "Drift"},
        }};

  public:
    enum ColourIds {
        backgroundColourId = 0x1312000,
    };

    DelaySection(APVTS &apvts);

    virtual void resized() override;
    virtual void paint(juce::Graphics &g) override;

  private:
    juce::Label m_title;

    Slider m_sliders[elements.size()];

    juce::TextButton m_active;
    juce::AudioProcessorValueTreeState::ButtonAttachment m_activeAttachment;

    juce::ComboBox m_mode;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment m_modeAttachment;
};
