#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

class CustomLNF : public juce::LookAndFeel_V4
{
  public:
    static constexpr auto boxRoundSize = 10.f;
    static constexpr auto margin       = 8;
    static constexpr auto padding      = 14;
    static constexpr auto sepWidth     = 1.2f;
    static constexpr auto subtitleSize = 18;

    static constexpr auto textPointHeight = 13;

    static constexpr auto delayMainColour   = 0xff609ccd;
    static constexpr auto delayBackColour   = 0xffd2d6d8;
    static constexpr auto springsMainColour = 0xfff88261;
    static constexpr auto springsBackColour = 0xffe9e5e2;

    static juce::Typeface::Ptr titleTypeface;
    static juce::Typeface::Ptr symbolsTypeface;
    static juce::Typeface::Ptr defaultTypeface;
    static juce::Typeface::Ptr defaultMonoTypeface;
    CustomLNF();

    void drawRotarySlider(juce::Graphics &g, int x, int y, int width,
                          int height, float sliderPos,
                          const float rotaryStartAngle,
                          const float rotaryEndAngle, juce::Slider &) override;

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
    virtual void drawToggleButton(juce::Graphics &, juce::ToggleButton &,
                                  bool shouldDrawButtonAsHighlighted,
                                  bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics &g, int, int, bool, int, int, int, int,
                      juce::ComboBox &) override;
    virtual juce::Font getComboBoxFont(juce::ComboBox &) override;
    virtual void positionComboBoxText(juce::ComboBox &box,
                                      juce::Label &label) override;

    //////////////////////////////////
    static constexpr auto popupElementSizeFontRatio = 1.3f;
    juce::Font getPopupMenuFont() override;
    void getIdealPopupMenuItemSize(const juce::String &text,
                                   const bool isSeparator,
                                   int standardMenuItemHeight, int &idealWidth,
                                   int &idealHeight) override;
    void drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area,
                           const bool isSeparator, const bool isActive,
                           const bool isHighlighted, const bool isTicked,
                           const bool hasSubMenu, const juce::String &text,
                           const juce::String &shortcutKeyText,
                           const juce::Drawable *icon,
                           const juce::Colour *const textColourToUse) override;

  private:
    juce::Image noise{juce::Image::PixelFormat::SingleChannel, 40, 40, false};
};

} // namespace aether
