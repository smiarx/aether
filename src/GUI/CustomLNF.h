#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

class CustomLNF : public juce::LookAndFeel_V4
{
  public:
    static constexpr auto kBoxRoundSize = 10.f;
    static constexpr auto kMargin       = 8;
    static constexpr auto kPadding      = 14;
    static constexpr auto kSepWidth     = 1.2f;
    static constexpr auto kSubtitleSize = 18;

    static constexpr auto kTextPointHeight = 13;

    static constexpr auto kDelayMainColour   = 0xff609ccd;
    static constexpr auto kDelayBackColour   = 0xffd2d6d8;
    static constexpr auto kSpringsMainColour = 0xfff88261;
    static constexpr auto kSpringsBackColour = 0xffe9e5e2;
    CustomLNF();

    void drawRotarySlider(juce::Graphics &g, int x, int y, int width,
                          int height, float sliderPos, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider &) override;

    void drawBubble(juce::Graphics &g, juce::BubbleComponent &comp,
                    const juce::Point<float> &tip,
                    const juce::Rectangle<float> &body) override;

    juce::Font getSliderPopupFont(juce::Slider &) override;
    int getSliderPopupPlacement(juce::Slider &) override;

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
    void drawToggleButton(juce::Graphics &, juce::ToggleButton &,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics &g, int, int, bool, int, int, int, int,
                      juce::ComboBox &) override;
    juce::Font getComboBoxFont(juce::ComboBox &) override;
    void positionComboBoxText(juce::ComboBox &box, juce::Label &label) override;

    //////////////////////////////////
    static constexpr auto kPopupElementSizeFontRatio = 1.3f;
    juce::Font getPopupMenuFont() override;
    void getIdealPopupMenuItemSize(const juce::String &text, bool isSeparator,
                                   int standardMenuItemHeight, int &idealWidth,
                                   int &idealHeight) override;
    void drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String &text,
                           const juce::String &shortcutKeyText,
                           const juce::Drawable *icon,
                           const juce::Colour *textColourToUse) override;

  private:
    juce::Image noise_{
// SingleChannel doesn't seem to work on macos
#ifdef __APPLE__
        juce::Image::PixelFormat::ARGB,
#else
        juce::Image::PixelFormat::SingleChannel,
#endif
        40, 40, false};
};

} // namespace aether
