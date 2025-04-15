#pragma once

#include "CustomLNF.h"
#include "Slider.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

using APVTS = juce::AudioProcessorValueTreeState;

template <class Comp> class Widget : public juce::Component
{
  public:
    static constexpr auto textSize    = CustomLNF::textPointHeight;
    static constexpr auto labelMargin = -3;

    template <class... Ts>
    Widget(const juce::String &name, Ts &&...args) : m_component(args...)
    {
        addAndMakeVisible(m_component);
        addAndMakeVisible(m_label);

        m_component.setTitle(name);
        m_label.setText(name.toLowerCase(),
                        juce::NotificationType::dontSendNotification);
        m_label.setJustificationType(juce::Justification::centred);

        auto font =
            juce::Font(CustomLNF::defaultTypeface).withPointHeight(textSize);
        m_label.setFont(font);
    }
    virtual void resized() override
    {
        auto bounds = getLocalBounds();

        if (m_labelVisible) {
            auto textBox = bounds.removeFromBottom(
                labelMargin + m_label.getFont().getHeight());
            textBox.removeFromTop(labelMargin);
            m_label.setBounds(textBox);
        }
        m_component.setBounds(bounds);
    }

    void setLabelVisible(bool labelVisible)
    {
        m_labelVisible = labelVisible;
        if (m_labelVisible) {
            addAndMakeVisible(m_label);
        } else {
            removeChildComponent(&m_label);
        }
    }
    bool isLabelVisible() const { return m_labelVisible; }

    Comp &getComponent() { return m_component; }
    auto &getLabel() { return m_label; }

    void setColour(int colourID, juce::Colour colour)
    {
        m_component.setColour(colourID, colour);
    }

  protected:
    Comp m_component;
    juce::Label m_label;

  private:
    bool m_labelVisible{true};
};

class SliderWithLabel : public Widget<Slider>
{
  public:
    SliderWithLabel(APVTS &apvts, const juce::String &id,
                    const juce::String &name) :
        Widget<Slider>(name), m_attachment(apvts, id, m_component)
    {
        m_component.setSliderStyle(
            juce::Slider::SliderStyle::RotaryVerticalDrag);
        m_component.setTextBoxStyle(
            juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
    }

    int getMaxHeight()
    {
        int removeFromHeight = 0;
        if (isLabelVisible()) {
            removeFromHeight = labelMargin + m_label.getFont().getHeight();
        }
        auto bounds = getLocalBounds();
        auto size   = juce::jmin(bounds.getHeight() - removeFromHeight,
                                 bounds.getWidth());
        size += removeFromHeight;
        return size;
    }

    void resized() override
    {
        // set height to maximum possible value
        auto maxHeight = getMaxHeight();
        auto bounds    = getBounds();
        setBounds(bounds.withHeight(maxHeight).withCentre(bounds.getCentre()));

        // call parent
        Widget<Slider>::resized();

        // update width to be equal to height
        bounds = m_component.getLocalBounds();
        m_component.setBounds(bounds.withWidth(bounds.getHeight())
                                  .withCentre(bounds.getCentre()));
    }

    void setTextBoxVisible(bool textBoxVisible)
    {
        m_component.setTextBoxStyle(
            textBoxVisible ? juce::Slider::TextEntryBoxPosition::TextBoxBelow
                           : juce::Slider::NoTextBox,
            true, 100, textSize);
    }

    void setValueAsLabel()
    {
        auto font = juce::Font(CustomLNF::defaultMonoTypeface)
                        .withPointHeight(textSize - 1);
        m_label.setFont(font);

        m_component.setPopupDisplayEnabled(false, false, nullptr);

        m_component.onValueChange = [this] {
            m_label.setText(
                m_component.getTextFromValue(m_component.getValue()),
                juce::NotificationType::dontSendNotification);
        };
    }

    juce::Slider &getSlider() { return getComponent(); }
    APVTS::SliderAttachment &getAttachment() { return m_attachment; }

  private:
    APVTS::SliderAttachment m_attachment;
};

} // namespace aether
