#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "Widgets.h"

class _SpreadSlider : public juce::Slider
{
  public:
    _SpreadSlider(juce::AudioProcessorValueTreeState &apvts,
                  const juce::String &spreadId);

    void paint(juce::Graphics &g) override;
    void drawSpreadSlider(juce::Graphics &g, int x, int y, int width,
                          int height, float sliderPos, float spreadPos);
    void setSpread(float spread);

    virtual void mouseDown(const juce::MouseEvent &e) override;
    virtual void mouseDrag(const juce::MouseEvent &e) override;

  private:
    static constexpr float spreadIntensity = 6.f;

    float m_spread;
    juce::ParameterAttachment m_spreadAttachment;
    const juce::NormalisableRange<float> &m_spreadRange;

    juce::Point<float> m_mouseDragStartPos;
    float m_valueDragStartPos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(_SpreadSlider)
};

class SpreadSlider : public Widget<_SpreadSlider>
{
  public:
    enum ColourIDs {
        spreadColourId = 0x3732712,
    };

    SpreadSlider(APVTS &apvts, const juce::String &id,
                 const juce::String &spreadId, const juce::String &name) :
        Widget<_SpreadSlider>(name, apvts, spreadId),
        m_attachment(apvts, id, m_component)
    {
        m_component.setSliderStyle(
            juce::Slider::SliderStyle::RotaryVerticalDrag);
        m_component.setTextBoxStyle(
            juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
        m_component.setPopupDisplayEnabled(true, false, getTopLevelComponent());
    }

    juce::Slider &getSlider() { return getComponent(); }

    void resized() override
    {
        squareSized();
        Widget<_SpreadSlider>::resized();
    }

  private:
    APVTS::SliderAttachment m_attachment;
};
