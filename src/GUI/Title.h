#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

class Title
{
  public:
    static constexpr auto kTitleString = u8"ÆTHER";

    Title();
    void setBounds(juce::Rectangle<float> bounds) { bounds_ = bounds; }
    [[nodiscard]] float getMaxWidth() const { return maxWidth_; }
    void draw(juce::Graphics &g);

  private:
    juce::TextLayout textLayout_;
    juce::Rectangle<float> bounds_;
    float maxWidth_{};
};
} // namespace aether
