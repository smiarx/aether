#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_gui_basics/juce_gui_basics.h"

#include "SpringsGL.h"
#include "Widgets.h"

namespace aether
{

class SpringsSection : public juce::Component
{
    enum {
        DryWet  = 0,
        Width   = 1,
        Length  = 2,
        Decay   = 3,
        Damp    = 4,
        Shape   = 5,
        Tone    = 6,
        Scatter = 7,
        Chaos   = 8,
        _NumParams,
    };

    static constexpr std::array<std::tuple<const char *, const char *>,
                                _NumParams>
        elements{{
            {"springs_drywet", "Dry/Wet"},
            {"springs_width", "Width"},
            {"springs_length", "Length"},
            {"springs_decay", "Decay"},
            {"springs_damp", "Damp"},
            {"springs_shape", "Shape"},
            {"springs_tone", "Tone"},
            {"springs_scatter", "Scatter"},
            {"springs_chaos", "Chaos"},
        }};

  public:
    enum ColourIDs {
        backgroundColourId    = 0x1610100,
        allBackgroundColourId = 0x1610900,
    };

    static constexpr auto headerHeight = 30.f;

    SpringsSection(const PluginProcessor &processor,
                   juce::AudioProcessorValueTreeState &apvts);
    void resized() override;
    void paint(juce::Graphics &g) override;

  private:
    Slider m_sliders[elements.size()];

    juce::ToggleButton m_active;
    juce::AudioProcessorValueTreeState::ButtonAttachment m_activeAttachment;

    SpringsGL m_springsGL;
};

} // namespace aether
