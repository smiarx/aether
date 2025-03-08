#include "SpringsSection.h"
#include "CustomLNF.h"
#include "PluginEditor.h"

#include "../PluginProcessor.h"

static const auto mainColour  = juce::Colour(0xff8fe7f3);
static const auto smallColour = juce::Colour(0xfff130f1);

SpringsSection::SpringsSection(juce::AudioProcessorValueTreeState &apvts) :
    m_sliders{
        Slider(apvts, std::get<0>(elements[0]), std::get<1>(elements[0])),
        Slider(apvts, std::get<0>(elements[1]), std::get<1>(elements[1])),
        Slider(apvts, std::get<0>(elements[2]), std::get<1>(elements[2])),
        Slider(apvts, std::get<0>(elements[3]), std::get<1>(elements[3])),
        Slider(apvts, std::get<0>(elements[4]), std::get<1>(elements[4])),
        Slider(apvts, std::get<0>(elements[5]), std::get<1>(elements[5])),
        Slider(apvts, std::get<0>(elements[6]), std::get<1>(elements[6])),
        Slider(apvts, std::get<0>(elements[7]), std::get<1>(elements[7])),
    }
{
    for (auto &slider : m_sliders) {
        addAndMakeVisible(slider);
        slider.getComponent().setPopupDisplayEnabled(true, false,
                                                     getTopLevelComponent());
    }

    m_sliders[DryWet].getComponent().setTextValueSuffix("%");
    m_sliders[Width].getComponent().setTextValueSuffix("%");
    m_sliders[Length].getComponent().setTextValueSuffix("s");
    m_sliders[Decay].getComponent().setTextValueSuffix("s");
    m_sliders[Damp].getComponent().setTextValueSuffix("Hz");
    m_sliders[Chaos].getComponent().setTextValueSuffix("%");

    m_sliders[DryWet].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Width].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Length].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Decay].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Damp].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Shape].setColour(juce::Slider::thumbColourId, smallColour);
    m_sliders[Diff].setColour(juce::Slider::thumbColourId, smallColour);
    m_sliders[Chaos].setColour(juce::Slider::thumbColourId, smallColour);
}

void SpringsSection::resized()
{
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr    = juce::Grid::Fr;
    using Span  = juce::GridItem::Span;

    grid.templateColumns = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),
                            Track(Fr(1)), Track(Fr(2))};
    grid.templateRows    = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1))};

    grid.items = {
        juce::GridItem(m_sliders[Length]).withArea(1, 1, Span(2), Span(2)),
        juce::GridItem(m_sliders[Decay]).withArea(1, 3, Span(1), Span(2)),
        juce::GridItem(m_sliders[Damp]).withArea(2, 3, Span(1), Span(2)),
        juce::GridItem(m_sliders[Shape]).withArea(3, 1),
        juce::GridItem(m_sliders[Diff]).withArea(3, 2),
        juce::GridItem(m_sliders[Chaos]).withArea(3, 3),
        // juce::GridItem(m_sliders[6]).withArea(3, 4), // springness
        juce::GridItem(m_sliders[Width]).withArea(2, 5),
        juce::GridItem(m_sliders[DryWet]).withArea(3, 5),
    };

    auto bounds = getLocalBounds();
    bounds.reduce(CustomLNF::padding, CustomLNF::padding);
    grid.performLayout(bounds);
}

void SpringsSection::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(findColour(backgroundColourId));
    juce::Path box;
    box.addRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(),
                            bounds.getHeight(), CustomLNF::boxRoundSize,
                            CustomLNF::boxRoundSize, true, true, false, true);
    g.fillPath(box);

    bounds.reduce(CustomLNF::padding, CustomLNF::padding);
    g.setColour(findColour(backgroundColourId));
    g.fillRoundedRectangle(bounds, CustomLNF::boxRoundSize);

    // separators;
    g.setColour(findColour(PluginEditor::Separator));

    auto xSep = (m_sliders[Shape].getBoundsInParent().getRight() +
                 m_sliders[Width].getBoundsInParent().getX()) /
                2.f;
    g.fillRect(xSep - CustomLNF::sepWidth / 2.f, bounds.getY(),
               CustomLNF::sepWidth, bounds.getHeight());

    auto ySep = (m_sliders[Damp].getBoundsInParent().getBottom() +
                 m_sliders[Shape].getBoundsInParent().getY()) /
                2.f;
    auto yWidth = xSep - bounds.getX();
    g.fillRect(bounds.getX(), ySep - CustomLNF::sepWidth / 2.f, yWidth,
               CustomLNF::sepWidth);
}
