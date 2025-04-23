#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

class ArrowButton : public juce::Button
{
  public:
    ArrowButton(const juce::String &buttonName, float arrowDirection);

    /** Destructor. */
    ~ArrowButton() override;

    enum ColourIds {
        kArrowColourId = 0x1912323,
    };

    void setColour(juce::Colour clr) { colour_ = clr; }

    /** @internal */
    void paintButton(juce::Graphics &, bool, bool) override;

  private:
    juce::Path path_;
    juce::Colour colour_{0xff000000};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArrowButton)
};

} // namespace aether
