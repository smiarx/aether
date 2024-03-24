#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_gui_basics/juce_gui_basics.h"

using APVTS = juce::AudioProcessorValueTreeState;

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
    DelaySection(APVTS &apvts);

    virtual void resized() override;

  private:
    struct DelaySlider : juce::Slider {
        DelaySlider(APVTS &apvts, const juce::String &id) :
            juce::Slider(juce::Slider::SliderStyle::RotaryVerticalDrag,
                         juce::Slider::TextEntryBoxPosition::NoTextBox),
            attachment(apvts, id, *this)
        {
        }
        juce::Slider slider;

      private:
        APVTS::SliderAttachment attachment;
    };

    DelaySlider m_sliders[elements.size()];
    juce::Label m_labels[elements.size()];

    juce::ComboBox m_mode;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment m_modeAttachment;
    juce::Label m_modeLabel;
};
