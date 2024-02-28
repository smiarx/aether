#pragma once

#include <foleys_gui_magic/foleys_gui_magic.h>

class SpreadSlider : public foleys::AutoOrientationSlider
{
  public:
    SpreadSlider();

    void paint(juce::Graphics &g) override;
    void drawRotarySlider(juce::Graphics &g, int x, int y, int width,
                          int height, float sliderPos, float spreadPos);
    void drawLinearSlider(juce::Graphics &g, int x, int y, int width,
                          int height, float sliderPos, float spreadPos);

    void setSpreadParameter(juce::RangedAudioParameter *parameter);

  private:
    foleys::ParameterAttachment<float> m_spreadAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpreadSlider)
};

//====================================================================
class SpreadSliderItem : public foleys::GuiItem
{
  public:
    FOLEYS_DECLARE_GUI_FACTORY(SpreadSliderItem)

    static const juce::Identifier pSliderType;
    static const juce::StringArray pSliderTypes;

    static const juce::Identifier pSpreadParameter;

    static const juce::Identifier pSliderTextBox;
    static const juce::StringArray pTextBoxPositions;

    static const juce::Identifier pValue;
    static const juce::Identifier pMinValue;
    static const juce::Identifier pMaxValue;
    static const juce::Identifier pInterval;
    static const juce::Identifier pSuffix;

    SpreadSliderItem(foleys::MagicGUIBuilder &builder,
                     const juce::ValueTree &node);

    void update() override;
    juce::Component *getWrappedComponent() override { return &slider; }

    [[nodiscard]] std::vector<foleys::SettableProperty>
    getSettableProperties() const override;

  private:
    SpreadSlider slider;
    std::unique_ptr<juce::SliderParameterAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpreadSliderItem)
};
