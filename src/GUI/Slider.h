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

    Slider() { setPaintingIsUnclipped(true); }

    void setPolarity(Polarity polarity) { polarity_ = polarity; }
    Polarity getPolarity() const { return polarity_; }

    void setHasOutline(bool hasOutline) { hasOutline_ = hasOutline; }
    bool getHasOutline() const { return hasOutline_; }

  private:
    Polarity polarity_{Unipolar};
    bool hasOutline_{false};
};
} // namespace aether
