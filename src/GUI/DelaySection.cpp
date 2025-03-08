#include "DelaySection.h"
#include "CustomLNF.h"
#include "PluginEditor.h"

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

    m_sliders[DryWet].getComponent().setTextValueSuffix("%");
    m_sliders[Time].getComponent().setTextValueSuffix("s");
    m_sliders[Feedback].getComponent().setTextValueSuffix("%");
    m_sliders[CutLow].getComponent().setTextValueSuffix("Hz");
    m_sliders[CutHi].getComponent().setTextValueSuffix("Hz");
    m_sliders[Saturation].getComponent().setTextValueSuffix("dB");
    m_sliders[Drift].getComponent().setTextValueSuffix("%");

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
    m_sliders[DryWet].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Time].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Feedback].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[CutLow].setColour(juce::Slider::thumbColourId, smallColour);
    m_sliders[CutHi].setColour(juce::Slider::thumbColourId, smallColour);
    m_sliders[Saturation].setColour(juce::Slider::thumbColourId, smallColour);
    m_sliders[Drift].setColour(juce::Slider::thumbColourId, smallColour);

    setColour(PluginEditor::Separator, juce::Colour{0xff0f3d43});
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
    auto bounds = getLocalBounds().toFloat();
    bounds.reduce(CustomLNF::padding, CustomLNF::padding);

    g.setColour(findColour(backgroundColourId));
    g.fillRoundedRectangle(bounds.toFloat(), CustomLNF::boxRoundSize);

    // separator
    auto ySep = (m_sliders[Feedback].getBoundsInParent().getBottom() +
                 m_sliders[CutLow].getBoundsInParent().getY()) /
                2.f;
    g.setColour(findColour(PluginEditor::Separator));
    g.fillRect(bounds.getX(), ySep - CustomLNF::sepWidth / 2.f,
               bounds.getWidth(), CustomLNF::sepWidth);
}
