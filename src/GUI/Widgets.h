#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

using APVTS = juce::AudioProcessorValueTreeState;

template <class Comp> class Widget : public juce::Component
{
    static constexpr auto labelMaxHeight = 20.f;

  public:
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
        juce::FlexBox fb;
        fb.flexDirection = juce::FlexBox::Direction::columnReverse;
        fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
        fb.alignContent  = juce::FlexBox::AlignContent::center;

        if (m_labelVisible)
            fb.items.add(juce::FlexItem(m_label).withFlex(0.4f).withMaxHeight(
                labelMaxHeight));

        fb.items.add(juce::FlexItem(m_component).withFlex(1.f));

        fb.performLayout(getLocalBounds());
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

    juce::Slider &getSlider() { return getComponent(); }

  private:
    APVTS::SliderAttachment m_attachment;
};
