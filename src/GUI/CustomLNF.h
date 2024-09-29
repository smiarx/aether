#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class CustomLNF : public juce::LookAndFeel_V4
{
  public:
    static constexpr auto boxRoundSize = 10.f;
    static constexpr auto margin       = 8.f;
    static constexpr auto padding      = 10.f;
    static constexpr auto sepWidth     = 1.2f;
    CustomLNF();

    void drawRotarySlider(juce::Graphics &g, int x, int y, int width,
                          int height, float sliderPos,
                          const float rotaryStartAngle,
                          const float rotaryEndAngle, juce::Slider &) override;

    void drawBubble(juce::Graphics &g, juce::BubbleComponent &comp,
                    const juce::Point<float> &tip,
                    const juce::Rectangle<float> &body) override;
    void setComponentEffectForBubbleComponent(
        juce::BubbleComponent &bubbleComponent) override;
};
