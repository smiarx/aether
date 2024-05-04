#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

using APVTS = juce::AudioProcessorValueTreeState;

template <class Comp> class Widget : public juce::Component
{
  public:
    static constexpr auto textSize    = 16;
    static constexpr auto labelMargin = 12;
    static constexpr auto labelSize   = textSize + labelMargin;

    template <class... Ts>
    Widget(const juce::String &name, Ts &&...args) : m_component(args...)
    {
        addAndMakeVisible(m_component);
        addAndMakeVisible(m_label);

        m_label.setText(name, juce::NotificationType::dontSendNotification);
        m_label.setJustificationType(juce::Justification::centred);

        auto font = juce::Font(10);
        m_label.setFont(font);
    }
    virtual void resized() override
    {
        auto bounds = getLocalBounds();

        if (m_labelVisible) {
            auto textBox = bounds.removeFromBottom(labelSize);
            textBox.removeFromTop(labelMargin);
            m_label.setBounds(textBox);
        }
        m_component.setBounds(getLocalBounds());
    }

    void setLabelVisiblie(bool labelVisible)
    {
        m_labelVisible = labelVisible;
        if (m_labelVisible) {
            addAndMakeVisible(m_label);
        } else {
            removeChildComponent(&m_label);
        }
    }

    Comp &getComponent() { return m_component; }

    void setColour(int colourID, juce::Colour colour)
    {
        m_component.setColour(colourID, colour);
    }

    /* set the size of the widget to be squared (for slider,..) */
    void squareSized()
    {
        auto bounds      = getBounds();
        const auto width = bounds.getWidth();
        auto height      = bounds.getHeight();

        /* remove text size if label is visible */
        if (m_labelVisible) height -= labelSize;

        const auto size = width > height ? height : width;

        const auto centre = bounds.getCentre();
        bounds.setWidth(size);
        bounds.setHeight(m_labelVisible ? size + labelSize : size);
        bounds.setCentre(centre);
        setBounds(bounds);
    }

  protected:
    Comp m_component;
    juce::Label m_label;

  private:
    bool m_labelVisible{true};
};

class Slider : public Widget<juce::Slider>
{
  public:
    Slider(APVTS &apvts, const juce::String &id, const juce::String &name) :
        Widget<juce::Slider>(name), m_attachment(apvts, id, m_component)
    {
        m_component.setSliderStyle(
            juce::Slider::SliderStyle::RotaryVerticalDrag);
        m_component.setTextBoxStyle(
            juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
    }

    void resized() override
    {
        if (getSlider().isRotary()) squareSized();
        Widget<juce::Slider>::resized();
    }

    juce::Slider &getSlider() { return getComponent(); }

  private:
    APVTS::SliderAttachment m_attachment;
};
