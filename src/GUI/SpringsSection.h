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
            {"length", "Length"},
            {"decay", "Decay"},
            {"damp", "Damp"},
            {"hilo", "Lo/Hi"},
            {"dispersion", "Dispersion"},
            {"chaos", "Chaos"},
            {"springness", "Springness"},
        }};

  public:
    enum ColourIDs {
        backgroundColourId    = 0x1610100,
        allBackgroundColourId = 0x1610900,
    };

    static constexpr auto headerHeight = 30.f;

    SpringsSection(juce::AudioProcessorValueTreeState &apvts);
    void resized() override;
    void paint(juce::Graphics &g) override;

  private:
    struct Spring : public juce::GroupComponent {
        Spring(juce::AudioProcessorValueTreeState &apvts, int id);
        void resized() override;
        void paint(juce::Graphics &) override;

        int m_id;
        Slider params[elements.size()];

        juce::TextButton mute;
        juce::TextButton solo;
        juce::AudioProcessorValueTreeState::ButtonAttachment muteAttachment;
        juce::AudioProcessorValueTreeState::ButtonAttachment soloAttachment;

        struct Source : public juce::AudioProcessorParameter::Listener {
            static constexpr auto groupId = 1000;

            Source(APVTS &apvts, int id);
            virtual void parameterValueChanged(int parameterIndex,
                                               float newValue) override;
            virtual void
            parameterGestureChanged(int parameterIndex,
                                    bool gestureIsStarting) override
            {
            }

            juce::TextButton left;
            juce::TextButton right;
            juce::TextButton middle;
        } source;

        juce::Rectangle<float> leftPanelRect;
    } springs[MAXSPRINGS];

  public:
    struct Macros : public juce::Component {
        Macros(juce::AudioProcessorValueTreeState &apvts);
        void resized() override;
        void paint(juce::Graphics &g) override;

        struct Macro : SpreadSlider {
            Macro(juce::AudioProcessorValueTreeState &apvts,
                  const juce::String &id, const juce::String &name) :
                SpreadSlider(apvts, "springs_" + id, "springs_spread_" + id,
                             name)
            {
            }
        };
        Macro params[elements.size() - 2];

        Slider drywet;
        Slider width;
    } macros;
};
