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
    m_activeAttachment(apvts, "delay_active", m_active),
    m_mode(juce::Colour{0xffffef03}),
    m_modeAttachment(apvts, "delay_mode", m_mode.getComboBox()),
    m_timeType(juce::Colour{0xffffef03}),
    m_timeTypeAttachment(apvts, "delay_time_type", m_timeType.getComboBox())
{
    addAndMakeVisible(m_title);
    m_title.setText(u8"Delay", juce::dontSendNotification);
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
    m_sliders[Time].setLabelVisible(false);

    m_sliders[DryWet].getComponent().setTextValueSuffix("%");
    m_sliders[Time].getComponent().setTextValueSuffix("s");
    m_sliders[Feedback].getComponent().setTextValueSuffix("%");
    m_sliders[CutLow].getComponent().setTextValueSuffix("Hz");
    m_sliders[CutHi].getComponent().setTextValueSuffix("Hz");
    m_sliders[Saturation].getComponent().setTextValueSuffix("dB");
    m_sliders[Drift].getComponent().setTextValueSuffix("%");

    m_sliders[DryWet].getComponent().setTooltip(
        "Dry/wet proportion of the output signal");
    m_sliders[Time].getComponent().setTooltip("Delay time.");
    m_sliders[Feedback].getComponent().setTooltip(
        "How much of the delayed signal is fed back into the delay.");
    m_sliders[CutLow].getComponent().setTooltip(
        "Cutoff frequency of the low pass filter.");
    m_sliders[CutHi].getComponent().setTooltip(
        "Cutoff frequency of the high pass filter.");
    m_sliders[Saturation].getComponent().setTooltip(
        "Saturation level in decibels of the delayed signal. Saturations of "
        "more than 0dB allows to set feedback levels of more than 100%.");
    m_sliders[Drift].getComponent().setTooltip(
        "How much the tape speed will be modulated. This effects as pitch "
        "wobble.");
    m_mode.getComboBox().setTooltip(
        "Delay mode: [Normal] forward direction, [Back & Forth] alternates "
        "between forwards and reverse - [Reverse] Reverse echoes");

    addAndMakeVisible(m_mode);
    m_mode.getComboBox().addItemList(
        apvts.getParameter("delay_mode")->getAllValueStrings(), 1);

    addAndMakeVisible(m_timeType);
    auto &timeTypeComboBox = m_timeType.getComboBox();
    timeTypeComboBox.addItemList(
        apvts.getParameter("delay_time_type")->getAllValueStrings(), 1);

    timeTypeComboBox.onChange = [this, &apvts]() {
        auto &component  = m_sliders[Time].getComponent();
        auto &attachment = m_sliders[Time].getAttachment();
        juce::String id;
        juce::String suffix;
        switch (m_timeType.getComboBox().getSelectedId()) {
        default:
        case 1:
            id     = "delay_seconds";
            suffix = "s";
            break;
        case 2:
            id     = "delay_beats";
            suffix = "";
            break;
        }
        // me must do this weird trick because we cannot replace attachment
        attachment.~SliderAttachment();
        new (&attachment) juce::AudioProcessorValueTreeState::SliderAttachment(
            apvts, id, component);

        component.setTextValueSuffix(suffix);
    };

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
        juce::FlexItem(m_mode).withFlex(1.f),
    };
    titleFb.performLayout(titleBounds);

    bounds.removeFromTop(CustomLNF::padding);
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
        juce::GridItem(m_sliders[Time])
            .withArea(1, 1, Span(2), Span(2))
            .withMargin({0, margin, margin, 0}),
        juce::GridItem(m_sliders[Feedback])
            .withArea(1, 3, Span(1), Span(2))
            .withMargin({0, 0, margin, margin}),
        juce::GridItem(m_sliders[DryWet])
            .withArea(2, 3, Span(1), Span(2))
            .withMargin({margin, 0, 0, margin}),
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
    grid.performLayout(bounds);

    // place time type
    {
        auto timeBounds     = m_sliders[Time].getBounds();
        auto timeTypeBounds = timeBounds.removeFromBottom(20);
        m_sliders[Time].setBounds(timeBounds);
        m_timeType.setBounds(timeTypeBounds);
    }
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
    auto ySep = (m_sliders[DryWet].getBoundsInParent().getBottom() +
                 m_sliders[CutLow].getBoundsInParent().getY()) /
                2.f;
    g.setColour(findColour(PluginEditor::Separator));
    g.fillRect(bounds.getX(), ySep - CustomLNF::sepWidth / 2.f,
               bounds.getWidth(), CustomLNF::sepWidth);
}
