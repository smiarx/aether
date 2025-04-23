#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

class Slider : public juce::Slider
{
  public:
    enum Polarity {
        kUnipolar,
        kUnipolarReversed,
        kBipolar,
    };

    Slider() { setPaintingIsUnclipped(true); }

    void setPolarity(Polarity polarity) { polarity_ = polarity; }
    [[nodiscard]] Polarity getPolarity() const { return polarity_; }

    void setHasOutline(bool hasOutline) { hasOutline_ = hasOutline; }
    [[nodiscard]] bool getHasOutline() const { return hasOutline_; }

  private:
    Polarity polarity_{kUnipolar};
    bool hasOutline_{false};
};
} // namespace aether
