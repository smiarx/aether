#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

class Slider : public juce::Slider
{
  public:
    enum Polarity {
        Unipolar,
        UnipolarReversed,
        Bipolar,
    };

    Slider() = default;
    void setPolarity(Polarity polarity) { polarity_ = polarity; }
    Polarity getPolarity() const { return polarity_; }

  private:
    Polarity polarity_{Unipolar};
};
} // namespace aether
