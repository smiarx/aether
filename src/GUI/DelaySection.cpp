#include "DelaySection.h"

DelaySection::DelaySection(juce::AudioProcessorValueTreeState &apvts) :
    m_sliders{
        DelaySlider(apvts, std::get<0>(elements[0])),
        DelaySlider(apvts, std::get<0>(elements[1])),
        DelaySlider(apvts, std::get<0>(elements[2])),
        DelaySlider(apvts, std::get<0>(elements[3])),
        DelaySlider(apvts, std::get<0>(elements[4])),
        DelaySlider(apvts, std::get<0>(elements[5])),
        DelaySlider(apvts, std::get<0>(elements[6])),
    },
    m_mode{}, m_modeAttachment(apvts, "delay_mode", m_mode)
{
    for (auto &slider : m_sliders) addAndMakeVisible(slider);

    for (size_t i = 0; i < elements.size(); ++i) {
        auto &label = m_labels[i];
        addAndMakeVisible(label);
        label.setText(std::get<1>(elements[i]),
                      juce::NotificationType::dontSendNotification);
        label.attachToComponent(&m_sliders[i], false);
        label.setJustificationType(juce::Justification::centred);
    }

    addAndMakeVisible(m_mode);
    m_mode.addItemList(apvts.getParameter("delay_mode")->getAllValueStrings(),
                       1);

    addAndMakeVisible(m_labels);
    m_modeLabel.setText("Mode", juce::NotificationType::dontSendNotification);
    m_modeLabel.attachToComponent(&m_mode, false);
    m_modeLabel.setJustificationType(juce::Justification::centred);
}

void DelaySection::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;
    fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fb.alignContent  = juce::FlexBox::AlignContent::center;
    // fb.alignItems = juce::FlexBox::AlignItems::center;

    for (auto &slider : m_sliders)
        fb.items.add(juce::FlexItem(slider).withFlex(1.f));

    fb.items.add(juce::FlexItem(m_mode).withFlex(1.f).withMaxHeight(30.f));

    auto bounds = getLocalBounds();
    auto shift  = m_labels[0].getHeight();
    bounds.removeFromTop(shift);
    fb.performLayout(bounds);
}
