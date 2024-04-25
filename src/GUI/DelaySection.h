#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_gui_basics/juce_gui_basics.h"

#include "Widgets.h"

class DelaySection : public juce::Component
{
    static constexpr std::array<std::tuple<const char *, const char *>, 7>
        elements{{
            {"delay_drywet", "Dry/Wet"},
            {"delay_time", "Time"},
            {"delay_feedback", "Feedback"},
            {"delay_cutoff", "Cutoff"},
            {"delay_drive", "Drive"},
            {"delay_drift", "Drift"},
            {"delay_driftfreq", "Drift Freq"},
        }};

  public:
    enum ColourIds {
        backgroundColourId = 0x1312000,
    };

    DelaySection(APVTS &apvts);

    virtual void resized() override;
    virtual void paint(juce::Graphics &g) override;

  private:
    Slider m_sliders[elements.size()];

    juce::ComboBox m_mode;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment m_modeAttachment;
    juce::Label m_modeLabel;
};
