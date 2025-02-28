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
    static constexpr std::array<
        std::tuple<const char *, const char *, const char *>, 9>
        elements{{
            {"vol", "Volume", "dB"},
            {"pan", "Panning", ""},
            {"length", "Length", "s"},
            {"decay", "Decay", "s"},
            {"damp", "Damp", "Hz"},
            {"hilo", "Lo/Hi", ""},
            {"dispersion", "Dispersion", ""},
            {"chaos", "Chaos", "s"},
            {"springness", "Springness", ""},
        }};

    enum {
        Vol        = 0,
        Pan        = 1,
        Length     = 2,
        Decay      = 3,
        Damp       = 4,
        Hilo       = 5,
        Dispersion = 6,
        Chaos      = 7,
        Springness = 8,
    };

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

        static constexpr auto numRemoveMacros = 2;
        Macro params[elements.size() - numRemoveMacros];

        Slider drywet;
        Slider width;
    } macros;
};
