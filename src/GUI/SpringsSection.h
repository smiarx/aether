#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_gui_basics/juce_gui_basics.h"

#include "Widgets.h"

class SpringsSection : public juce::Component
{
    enum {
        DryWet = 0,
        Width  = 1,
        Length = 2,
        Decay  = 3,
        Damp   = 4,
        Shape  = 5,
        Diff   = 6,
        Chaos  = 7,
        _NumParams,
    };

    static constexpr std::array<
        std::tuple<const char *, const char *, const char *>, _NumParams>
        elements{{
            {"springs_drywet", "Dry/Wet", ""},
            {"springs_width", "Width", ""},
            {"springs_length", "Length", "s"},
            {"springs_decay", "Decay", "s"},
            {"springs_damp", "Damp", "Hz"},
            {"springs_shape", "Shape", ""},
            {"springs_diff", "Diffusion", ""},
            {"springs_chaos", "Chaos", ""},
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
    Slider m_sliders[elements.size()];
};
