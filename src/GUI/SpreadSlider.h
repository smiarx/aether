#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class SpreadSlider : public juce::Slider
{
  public:
    SpreadSlider(juce::AudioProcessorValueTreeState &apvts,
                 const juce::String &id, const juce::String &spreadId);

    void paint(juce::Graphics &g) override;
    void drawSpreadSlider(juce::Graphics &g, int x, int y, int width,
                          int height, float sliderPos, float spreadPos);
    void setSpread(float spread);

    virtual void mouseDown(const juce::MouseEvent &e) override;
    virtual void mouseDrag(const juce::MouseEvent &e) override;

  private:
    static constexpr float spreadIntensity = 6.f;

    juce::AudioProcessorValueTreeState::SliderAttachment m_attachment;

    float m_spread;
    const juce::RangedAudioParameter &m_spreadParameter;
    juce::ParameterAttachment m_spreadAttachment;
    const juce::NormalisableRange<float> &m_spreadRange;

    juce::Point<float> m_mouseDragStartPos;
    float m_valueDragStartPos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpreadSlider)
};
