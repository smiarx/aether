#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

using APVTS = juce::AudioProcessorValueTreeState;

class Slider : public juce::Component {
    static constexpr auto labelMaxHeight = 20.f;
public:
    Slider(APVTS& apvts, const juce::String& id, const juce::String& name) :
        m_slider(juce::Slider::SliderStyle::RotaryVerticalDrag,
                juce::Slider::TextEntryBoxPosition::NoTextBox),
        m_attachment(apvts, id, m_slider)
    {
        addAndMakeVisible(m_slider);
        addAndMakeVisible(m_label);
        m_slider.setPopupDisplayEnabled(true, false, getTopLevelComponent());
        m_label.setText(name, juce::NotificationType::dontSendNotification);
        m_label.setJustificationType(juce::Justification::centred);

        auto font = juce::Font(9);
        m_label.setFont(font);
    }

    virtual void resized() override
    {
        juce::FlexBox fb;
        fb.flexDirection = juce::FlexBox::Direction::column;
        fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
        fb.alignContent  = juce::FlexBox::AlignContent::center;

        fb.items.addArray({
                juce::FlexItem(m_label).withFlex(0.3f).withMaxHeight(labelMaxHeight),
                juce::FlexItem(m_slider).withFlex(1.f)
                });

        fb.performLayout(getLocalBounds());
    }

    juce::Slider& getSlider() { return m_slider;}

private:
    juce::Slider m_slider;
    APVTS::SliderAttachment m_attachment;
    juce::Label m_label;

    
};

