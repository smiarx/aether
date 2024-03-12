#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_gui_basics/juce_gui_basics.h"

extern "C" {
#include "../dsp/springreverb.h"
}

class SpringsSection : public juce::Component
{
    static constexpr std::array<std::tuple<const char *, const char *>, 9>
        elements{{
            {"vol", "Volume"},
            {"pan", "Panning"},
            {"hilo", "Lo/Hi"},
            {"length", "Length"},
            {"decay", "Decay"},
            {"dispersion", "Dispersion"},
            {"damp", "Damp"},
            {"chaos", "Chaos"},
            {"springness", "Springness"},
        }};

  public:
    static constexpr auto headerHeight = 30.f;

    SpringsSection(juce::AudioProcessorValueTreeState &apvts);
    void resized() override;

  private:
    struct Spring : public juce::Component {
        Spring(juce::AudioProcessorValueTreeState &apvts, int id);
        void resized() override;
        void paint(juce::Graphics &g) override;

        struct SpringParam {
            SpringParam(juce::AudioProcessorValueTreeState &apvts,
                        const juce::String &parameterId, int id) :
                slider(juce::Slider::SliderStyle::Rotary,
                       juce::Slider::TextEntryBoxPosition::NoTextBox),
                attachment(apvts,
                           "spring" + juce::String(id) + "_" + parameterId,
                           slider)
            {
            }
            juce::Slider slider;

          private:
            juce::AudioProcessorValueTreeState::SliderAttachment attachment;
        };

        int m_id;
        SpringParam params[elements.size()];

        juce::TextButton mute;
        juce::TextButton solo;
        juce::AudioProcessorValueTreeState::ButtonAttachment muteAttachment;
        juce::AudioProcessorValueTreeState::ButtonAttachment soloAttachment;

        juce::Rectangle<float> leftPanelRect;

    } springs[MAXSPRINGS];

    juce::Label labels[elements.size()];
};
