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
        kDryWet  = 0,
        kWidth   = 1,
        kLength  = 2,
        kDecay   = 3,
        kDamp    = 4,
        kShape   = 5,
        kTone    = 6,
        kScatter = 7,
        kChaos   = 8,
        kNumParams,
    };

    static constexpr std::array<std::tuple<const char *, const char *>,
                                kNumParams>
        kElements{{
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
        kBackgroundColourId = 0x1610100,
    };

    static constexpr auto kHeaderHeight = 30.f;

    SpringsSection(PluginProcessor &processor);
    void resized() override;
    void paint(juce::Graphics &g) override;

  private:
    SliderWithLabel sliders_[kElements.size()];

    juce::ToggleButton active_;
    juce::AudioProcessorValueTreeState::ButtonAttachment activeAttachment_;

    SpringsGL springsGl_;
};

} // namespace aether
