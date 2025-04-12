#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class CustomLNF : public juce::LookAndFeel_V4
{
  public:
    static constexpr auto boxRoundSize = 10.f;
    static constexpr auto margin       = 8;
    static constexpr auto padding      = 10;
    static constexpr auto sliderMargin = 4.f;
    static constexpr auto sepWidth     = 1.2f;
    static constexpr auto subtitleSize = 18;

    static constexpr auto textPointHeight = 13;

    static juce::Typeface::Ptr defaultTypeface;
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

    void drawButtonBackground(juce::Graphics &, juce::Button &,
                              const juce::Colour &backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;
    juce::Font getTextButtonFont(juce::TextButton &, int buttonHeight) override;
    int getTextButtonWidthToFitText(juce::TextButton &b,
                                    int buttonHeight) override;
    void drawButtonText(juce::Graphics &g, juce::TextButton &button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;
    virtual void drawToggleButton(juce::Graphics &, juce::ToggleButton &,
                                  bool shouldDrawButtonAsHighlighted,
                                  bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics &g, int, int, bool, int, int, int, int,
                      juce::ComboBox &) override;
    virtual juce::Font getComboBoxFont(juce::ComboBox &) override;
    virtual void positionComboBoxText(juce::ComboBox &box,
                                      juce::Label &label) override;

  private:
    juce::Image noise{juce::Image::PixelFormat::SingleChannel, 40, 40, false};
};
