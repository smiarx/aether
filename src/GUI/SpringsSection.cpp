#include "SpringsSection.h"
#include "CustomLNF.h"
#include "PluginEditor.h"

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
        Slider(apvts, std::get<0>(elements[8]), std::get<1>(elements[8])),
    },
    m_activeAttachment(apvts, "springs_active", m_active)
{
    addAndMakeVisible(m_springsGL);

    addAndMakeVisible(m_title);
    m_title.setText(u8"Reverb", juce::dontSendNotification);
    m_title.setFont(juce::Font(CustomLNF::subtitleSize));

    addAndMakeVisible(m_active);
    m_active.setButtonText(juce::String::fromUTF8(u8"⏻"));
    m_active.setToggleable(true);
    m_active.setClickingTogglesState(true);

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
    m_sliders[Scatter].getComponent().setTextValueSuffix("%");

    m_sliders[DryWet].getComponent().setTooltip(
        "Dry/wet proportion of the output signal.");
    m_sliders[Width].getComponent().setTooltip(
        "Stereo width of the output signal..");
    m_sliders[Length].getComponent().setTooltip(
        "Length of the echoes produced by the springs.");
    m_sliders[Decay].getComponent().setTooltip(
        "How long the reverb takes to fade out.");
    m_sliders[Damp].getComponent().setTooltip(
        "Frequency of the highest compenents produced by the reverb.");
    m_sliders[Shape].getComponent().setTooltip(
        "Shape of the frequency dispertion of the springs.");
    m_sliders[Diff].getComponent().setTooltip(
        "How much the reverberated sound is diffused.");
    m_sliders[Chaos].getComponent().setTooltip(
        "How stochastic & unpredictible the springs become.");
    m_sliders[Scatter].getComponent().setTooltip(
        "How similar or different are the springs properties.");

    m_sliders[DryWet].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Width].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Length].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Decay].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Damp].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Shape].setColour(juce::Slider::thumbColourId, smallColour);
    m_sliders[Diff].setColour(juce::Slider::thumbColourId, smallColour);
    m_sliders[Scatter].setColour(juce::Slider::thumbColourId, smallColour);
    m_sliders[Chaos].setColour(juce::Slider::thumbColourId, smallColour);
}

void SpringsSection::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(CustomLNF::padding, CustomLNF::padding);

    auto titleBounds = bounds.removeFromTop(CustomLNF::subtitleSize);

    juce::FlexBox titleFb;
    titleFb.flexDirection = juce::FlexBox::Direction::row;
    titleFb.alignContent  = juce::FlexBox::AlignContent::center;
    auto activeWidth = m_active.getBestWidthForHeight(CustomLNF::subtitleSize);
    titleFb.items    = {
        juce::FlexItem(m_active)
            .withFlex(0.1f)
            .withMinWidth(activeWidth)
            .withMaxWidth(activeWidth),
        juce::FlexItem(m_title).withFlex(1.f),
    };
    titleFb.performLayout(titleBounds);

    bounds.removeFromTop(CustomLNF::padding);
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr    = juce::Grid::Fr;
    using Span  = juce::GridItem::Span;

    grid.templateColumns = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),
                            Track(Fr(1)), Track(Fr(2))};
    grid.templateRows    = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1))};

    constexpr auto margin = CustomLNF::sliderMargin;

    grid.items = {
        juce::GridItem(m_sliders[Length])
            .withArea(1, 1, Span(2), Span(2))
            .withMargin({0, margin, margin, 0}),
        juce::GridItem(m_sliders[Decay])
            .withArea(1, 3, Span(1), Span(2))
            .withMargin({0, margin, margin, 0}),
        juce::GridItem(m_sliders[Damp])
            .withArea(2, 3, Span(1), Span(2))
            .withMargin({margin, margin, 0, 0}),
        juce::GridItem(m_sliders[Shape])
            .withArea(3, 1)
            .withMargin({margin, margin, 0, 0}),
        juce::GridItem(m_sliders[Diff])
            .withArea(3, 2)
            .withMargin({margin, margin, 0, 0}),
        juce::GridItem(m_sliders[Scatter])
            .withArea(3, 3)
            .withMargin({margin, margin, 0, 0}),
        juce::GridItem(m_sliders[Chaos])
            .withArea(3, 4)
            .withMargin({margin, margin, 0, 0}),
        juce::GridItem(m_springsGL)
            .withArea(1, 5)
            .withMargin({0, 0, margin * 2, margin}),
        juce::GridItem(m_sliders[Width])
            .withArea(2, 5)
            .withMargin({margin, 0, margin, margin}),
        juce::GridItem(m_sliders[DryWet])
            .withArea(3, 5)
            .withMargin({margin, 0, 0, margin}),
    };
    grid.performLayout(bounds);
}

void SpringsSection::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(findColour(backgroundColourId));

    juce::Path box;
    box.addRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(),
                            bounds.getHeight(), CustomLNF::boxRoundSize,
                            CustomLNF::boxRoundSize, false, true, false, true);
    g.fillPath(box);

    // separators;
    g.setColour(findColour(PluginEditor::Separator));

    auto xSep = (m_sliders[Chaos].getBoundsInParent().getRight() +
                 m_springsGL.getBoundsInParent().getX()) /
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
