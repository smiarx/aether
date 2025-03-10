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

    constexpr auto margin  = CustomLNF::sliderMargin;
    constexpr auto margin2 = 2.f * margin;

    grid.items = {
        juce::GridItem(m_sliders[DryWet])
            .withArea(1, 3, Span(2), Span(2))
            .withMargin({0, 0, 0, margin}),
        juce::GridItem(m_sliders[Time])
            .withArea(1, 1, Span(1), Span(2))
            .withMargin({0, margin, margin, 0}),
        juce::GridItem(m_sliders[Feedback])
            .withArea(2, 1, Span(1), Span(2))
            .withMargin({margin2, margin, 0, 0}),
        juce::GridItem(m_sliders[CutLow])
            .withArea(3, 1)
            .withMargin({margin2, margin, 0, 0}),
        juce::GridItem(m_sliders[CutHi])
            .withArea(3, 2)
            .withMargin({margin2, margin, 0, 0}),
        juce::GridItem(m_sliders[Saturation])
            .withArea(3, 3)
            .withMargin({margin2, margin, 0, 0}),
        juce::GridItem(m_sliders[Drift])
            .withArea(3, 4)
            .withMargin({margin2, 0, 0, 0}),
    };

    auto bounds = getLocalBounds();
    bounds.reduce(CustomLNF::padding, CustomLNF::padding);
    grid.performLayout(bounds);
}

void DelaySection::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(findColour(backgroundColourId));
    juce::Path box;
    box.addRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(),
                            bounds.getHeight(), CustomLNF::boxRoundSize,
                            CustomLNF::boxRoundSize, true, false, true, false);
    g.fillPath(box);

    // separator
    auto ySep = (m_sliders[Feedback].getBoundsInParent().getBottom() +
                 m_sliders[CutLow].getBoundsInParent().getY()) /
                2.f;
    g.setColour(findColour(PluginEditor::Separator));
    g.fillRect(bounds.getX(), ySep - CustomLNF::sepWidth / 2.f,
               bounds.getWidth(), CustomLNF::sepWidth);
}
