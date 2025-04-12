#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class ArrowButton : public juce::Button
{
  public:
    ArrowButton(const juce::String &buttonName, float arrowDirection);

    /** Destructor. */
    ~ArrowButton() override;

    enum ColourIds {
        arrowColourId = 0x1912323,
    };

    void setColour(juce::Colour clr) { colour = clr; }

    /** @internal */
    void paintButton(juce::Graphics &, bool, bool) override;

  private:
    juce::Path path;
    juce::Colour colour{0xff000000};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArrowButton)
};
