#include "SpringsSection.h"
#include "CustomLNF.h"
#include "PluginEditor.h"

namespace aether
{

SpringsSection::SpringsSection(PluginProcessor &processor) :
    m_sliders{
        SliderWithLabel(processor.getAPVTS(), std::get<0>(elements[0]),
                        std::get<1>(elements[0])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(elements[1]),
                        std::get<1>(elements[1])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(elements[2]),
                        std::get<1>(elements[2])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(elements[3]),
                        std::get<1>(elements[3])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(elements[4]),
                        std::get<1>(elements[4])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(elements[5]),
                        std::get<1>(elements[5])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(elements[6]),
                        std::get<1>(elements[6])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(elements[7]),
                        std::get<1>(elements[7])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(elements[8]),
                        std::get<1>(elements[8])),
    },
    m_active("Reverb"),
    m_activeAttachment(processor.getAPVTS(), "springs_active", m_active),
    m_springsGL(processor)
{
    addAndMakeVisible(m_springsGL);

    addAndMakeVisible(m_active);

    for (auto &slider : m_sliders) {
        addAndMakeVisible(slider);
        slider.getComponent().setPopupDisplayEnabled(true, false,
                                                     getTopLevelComponent());
    }

    m_sliders[Shape].getComponent().setPolarity(Slider::Bipolar);

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
    m_sliders[Tone].getComponent().setTooltip("Shape the tone of the reverb.");
    m_sliders[Chaos].getComponent().setTooltip(
        "How stochastic & unpredictible the springs become.");
    m_sliders[Scatter].getComponent().setTooltip(
        "How similar or different are the springs properties.");

    static const auto mainColour = juce::Colour(CustomLNF::springsMainColour);

    m_active.setColour(juce::ToggleButton::tickColourId, mainColour);

    m_sliders[DryWet].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Width].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Length].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Decay].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Damp].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Shape].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Tone].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Scatter].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Chaos].setColour(juce::Slider::thumbColourId, mainColour);

    m_sliders[Decay].setValueAsLabel();
    m_sliders[Decay].getComponent().setHasOutline(true);

    m_active.onClick = [this] {
        bool active = m_active.getToggleState();
        for (auto &slider : m_sliders) {
            slider.setEnabled(active);
        }
    };
}

void SpringsSection::resized()
{
    auto bounds = getLocalBounds();

    constexpr auto margin = CustomLNF::padding / 2;
    bounds.removeFromTop(margin);
    bounds.removeFromLeft(margin);

    auto titleBounds =
        bounds.removeFromTop(CustomLNF::subtitleSize + 2 * margin);

    juce::FlexBox titleFb;
    titleFb.flexDirection = juce::FlexBox::Direction::row;
    titleFb.alignContent  = juce::FlexBox::AlignContent::center;
    titleFb.items         = {
        juce::FlexItem(m_active).withFlex(1.f).withMargin(margin),
    };
    titleFb.performLayout(titleBounds);

    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr    = juce::Grid::Fr;
    using Span  = juce::GridItem::Span;

    grid.templateColumns = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),
                            Track(Fr(1)), Track(Fr(2))};
    grid.templateRows    = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1))};

    grid.items = {
        juce::GridItem(m_sliders[Decay])
            .withArea(1, 1, Span(2), Span(2))
            .withMargin({margin}),
        juce::GridItem(m_sliders[Length])
            .withArea(1, 3, Span(1), Span(2))
            .withMargin({margin}),
        juce::GridItem(m_sliders[Damp])
            .withArea(2, 3, Span(1), Span(2))
            .withMargin({margin}),
        juce::GridItem(m_sliders[Shape]).withArea(3, 1).withMargin({margin}),
        juce::GridItem(m_sliders[Scatter]).withArea(3, 2).withMargin({margin}),
        juce::GridItem(m_sliders[Chaos]).withArea(3, 3).withMargin({margin}),
        juce::GridItem(m_sliders[Tone]).withArea(3, 4).withMargin({margin}),
        juce::GridItem(m_springsGL).withArea(1, 5).withMargin({margin}),
        juce::GridItem(m_sliders[Width]).withArea(2, 5).withMargin({margin}),
        juce::GridItem(m_sliders[DryWet]).withArea(3, 5).withMargin({margin}),
    };
    grid.performLayout(bounds);

    // move middle element closer
    {
        auto &damp    = m_sliders[Damp].getComponent();
        auto dampSize = juce::jmin(damp.getWidth(), damp.getHeight());
        auto sizeDiff = (m_sliders[Decay].getWidth() - dampSize) * 0.5f;
        if (sizeDiff > 0.f) {
            auto interMargin = 0.15f * sizeDiff;
            m_sliders[Decay].setBounds(m_sliders[Decay].getBounds().translated(
                sizeDiff - interMargin, 0));
            m_sliders[Length].setBounds(
                m_sliders[Length].getBounds().translated(interMargin, 0));
            m_sliders[Damp].setBounds(
                m_sliders[Damp].getBounds().translated(interMargin, 0));
        }
    }

    // extend spring gl
    auto glBounds = m_springsGL.getBounds();
    glBounds.translate(0, -headerHeight);
    glBounds.setHeight(glBounds.getHeight() + headerHeight);
    m_springsGL.setBounds(glBounds);
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
    g.setColour(findColour(DelaySection::backgroundColourId));

    auto xSep = (m_sliders[Tone].getBoundsInParent().getRight() +
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

} // namespace aether
