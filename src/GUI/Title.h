#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

class Title
{
  public:
    static constexpr auto titleString = u8"ÆTHER";

    Title();
    void setBounds(juce::Rectangle<float> bounds) { m_bounds = bounds; }
    float getMaxWidth() const { return m_maxWidth; }
    void draw(juce::Graphics &g);

  private:
    juce::TextLayout m_textLayout;
    juce::Rectangle<float> m_bounds;
    float m_maxWidth{};
};
} // namespace aether
