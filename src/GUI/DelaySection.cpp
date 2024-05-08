#include "DelaySection.h"
#include "CustomLNF.h"

DelaySection::DelaySection(juce::AudioProcessorValueTreeState &apvts) :
    m_sliders{
        Slider(apvts, std::get<0>(elements[0]), std::get<1>(elements[0])),
        Slider(apvts, std::get<0>(elements[1]), std::get<1>(elements[1])),
        Slider(apvts, std::get<0>(elements[2]), std::get<1>(elements[2])),
        Slider(apvts, std::get<0>(elements[3]), std::get<1>(elements[3])),
        Slider(apvts, std::get<0>(elements[4]), std::get<1>(elements[4])),
        Slider(apvts, std::get<0>(elements[5]), std::get<1>(elements[5])),
        Slider(apvts, std::get<0>(elements[6]), std::get<1>(elements[6])),
    },
    m_mode{}, m_modeAttachment(apvts, "delay_mode", m_mode)
{
    for (auto &slider : m_sliders) {
        addAndMakeVisible(slider);
        slider.getComponent().setPopupDisplayEnabled(true, false,
                                                     getTopLevelComponent());
    }

    // wet dry
    m_sliders[0].getComponent().setTextValueSuffix("%");
    // delay time
    m_sliders[1].getComponent().setTextValueSuffix("s");
    // feedback
    m_sliders[2].getComponent().setTextValueSuffix("%");
    // cutoff
    m_sliders[3].getComponent().setTextValueSuffix("Hz");
    // drive
    m_sliders[4].getComponent().setTextValueSuffix("dB");
    // drift rate
    m_sliders[6].getComponent().setTextValueSuffix("Hz");

    addAndMakeVisible(m_mode);
    m_mode.addItemList(apvts.getParameter("delay_mode")->getAllValueStrings(),
                       1);

    addAndMakeVisible(m_modeLabel);
    m_modeLabel.setText("Mode", juce::NotificationType::dontSendNotification);
    m_modeLabel.attachToComponent(&m_mode, false);
    m_modeLabel.setJustificationType(juce::Justification::centred);

    // set colours
    const auto mainColour  = juce::Colour(0xffffef03);
    const auto smallColour = juce::Colour(0xffffa301);
    m_sliders[0].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[1].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[2].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[3].setColour(juce::Slider::thumbColourId, smallColour);
    m_sliders[4].setColour(juce::Slider::thumbColourId, smallColour);
    m_sliders[5].setColour(juce::Slider::thumbColourId, smallColour);
    m_sliders[6].setColour(juce::Slider::thumbColourId, smallColour);
}

void DelaySection::resized()
{
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr    = juce::Grid::Fr;
    using Span  = juce::GridItem::Span;

    grid.templateColumns = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),
                            Track(Fr(1))};
    grid.templateRows    = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1))};

    grid.items = {
        juce::GridItem(m_sliders[0])
            .withArea(1, 3, Span(2), Span(2)), // dry/wet
        juce::GridItem(m_sliders[1]).withArea(1, 1, Span(1), Span(2)), // time
        juce::GridItem(m_sliders[2])
            .withArea(2, 1, Span(1), Span(2)),       // feedback
        juce::GridItem(m_sliders[3]).withArea(3, 1), // cutoff
        juce::GridItem(m_sliders[4]).withArea(3, 2), // drive
        juce::GridItem(m_sliders[5]).withArea(3, 3), // drift
        juce::GridItem(m_sliders[6]).withArea(3, 4), // drift freq
    };

    auto bounds = getLocalBounds();
    bounds.reduce(CustomLNF::padding, CustomLNF::padding);
    grid.performLayout(bounds);
}

void DelaySection::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds();
    bounds.reduce(CustomLNF::padding, CustomLNF::padding);

    g.setColour(findColour(backgroundColourId));
    g.fillRoundedRectangle(bounds.toFloat(), CustomLNF::boxRoundSize);
}
