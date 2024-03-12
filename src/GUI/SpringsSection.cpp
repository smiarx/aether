#include "SpringsSection.h"

SpringsSection::SpringsSection(juce::AudioProcessorValueTreeState &apvts) :
    springs{
        {apvts, 0}, {apvts, 1}, {apvts, 2}, {apvts, 3},
        {apvts, 4}, {apvts, 5}, {apvts, 6}, {apvts, 7},
    }
{
    unsigned int i = 0;
    for (auto &l : labels) {
        l.setText(std::get<1>(elements[i]), juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
        ++i;
    }

    for (auto &s : springs) addAndMakeVisible(s);
}

void SpringsSection::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fb.alignContent  = juce::FlexBox::AlignContent::center;

    juce::FlexBox fbLabels;
    fbLabels.flexDirection = juce::FlexBox::Direction::row;
    fbLabels.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fbLabels.alignContent  = juce::FlexBox::AlignContent::center;
    for (auto &l : labels) fbLabels.items.add(juce::FlexItem(l).withFlex(1.f));
    fb.items.add(juce::FlexItem(fbLabels).withHeight(30.f));

    for (auto &s : springs) fb.items.add(juce::FlexItem(s).withFlex(1.f));

    fb.performLayout(getLocalBounds());
}

SpringsSection::Spring::Spring(juce::AudioProcessorValueTreeState &apvts,
                               int id) :
    params{
        SpringParam(apvts, std::get<0>(elements[0]), id),
        SpringParam(apvts, std::get<0>(elements[1]), id),
        SpringParam(apvts, std::get<0>(elements[2]), id),
        SpringParam(apvts, std::get<0>(elements[3]), id),
        SpringParam(apvts, std::get<0>(elements[4]), id),
        SpringParam(apvts, std::get<0>(elements[5]), id),
        SpringParam(apvts, std::get<0>(elements[6]), id),
        SpringParam(apvts, std::get<0>(elements[7]), id),
        SpringParam(apvts, std::get<0>(elements[8]), id),
    }
{
    for (auto &p : params) addAndMakeVisible(p.slider);
}

void SpringsSection::Spring::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;
    fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fb.alignContent  = juce::FlexBox::AlignContent::center;

    for (auto &p : params) fb.items.add(juce::FlexItem(p.slider).withFlex(1));

    fb.performLayout(getLocalBounds());
}
