#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_gui_basics/juce_gui_basics.h"

#include "SpreadSlider.h"
#include "Widgets.h"

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

        int m_id;
        Slider params[elements.size()];

        juce::TextButton mute;
        juce::TextButton solo;
        juce::AudioProcessorValueTreeState::ButtonAttachment muteAttachment;
        juce::AudioProcessorValueTreeState::ButtonAttachment soloAttachment;

        juce::Rectangle<float> leftPanelRect;
    } springs[MAXSPRINGS];

    struct Macros : public juce::Component {
        Macros(juce::AudioProcessorValueTreeState &apvts);
        void resized() override;

        struct Macro : SpreadSlider {
            Macro(juce::AudioProcessorValueTreeState &apvts,
                  const juce::String &id) :
                SpreadSlider(apvts, "springs_" + id, "springs_spread_" + id)
            {
                setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox,
                                true, 0, 0);
            }
        };
        Macro params[elements.size() - 2];
    } macros;

    juce::Label labels[elements.size()];
};
