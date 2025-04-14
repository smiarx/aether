#include "DelaySection.h"
#include "CustomLNF.h"
#include "PluginEditor.h"

namespace aether
{

DelaySection::DelaySection(PluginProcessor &processor,
                           juce::AudioProcessorValueTreeState &apvts) :
    m_sliders{
        SliderWithLabel(apvts, std::get<0>(elements[0]),
                        std::get<1>(elements[0])),
        SliderWithLabel(apvts, std::get<0>(elements[1]),
                        std::get<1>(elements[1])),
        SliderWithLabel(apvts, std::get<0>(elements[2]),
                        std::get<1>(elements[2])),
        SliderWithLabel(apvts, std::get<0>(elements[3]),
                        std::get<1>(elements[3])),
        SliderWithLabel(apvts, std::get<0>(elements[4]),
                        std::get<1>(elements[4])),
        SliderWithLabel(apvts, std::get<0>(elements[5]),
                        std::get<1>(elements[5])),
        SliderWithLabel(apvts, std::get<0>(elements[6]),
                        std::get<1>(elements[6])),
    },
    m_active("Delay"), m_activeAttachment(apvts, "delay_active", m_active),
    m_modeAttachment(apvts, "delay_mode", m_mode.getComboBox()),
    m_timeTypeAttachment(apvts, "delay_time_type", m_timeType.getComboBox()),
    m_led(processor.getSwitchIndicator())
{
    addAndMakeVisible(m_active);

    addAndMakeVisible(m_led);

    for (auto &slider : m_sliders) {
        addAndMakeVisible(slider);
        slider.getComponent().setPopupDisplayEnabled(true, false,
                                                     getTopLevelComponent());
    }
    m_sliders[Time].getComponent().setPopupDisplayEnabled(false, false,
                                                          nullptr);
    m_sliders[Time].getComponent().setHasOutline(true);

    m_sliders[CutHi].getComponent().setPolarity(Slider::UnipolarReversed);

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
        auto &comboBox   = m_timeType.getComboBox();
        auto selected    = comboBox.getSelectedId();

        juce::String id;
        juce::String suffix;
        switch (selected) {
        default:
        case 1:
            id         = "delay_seconds";
            suffix     = "s";
            m_useBeats = false;
            break;
        case 2:
            id         = "delay_beats";
            suffix     = "";
            m_useBeats = true;
            break;
        }
        component.setTextValueSuffix(suffix);
        // me must do this weird trick because we cannot replace attachment
        attachment.~SliderAttachment();
        new (&attachment) juce::AudioProcessorValueTreeState::SliderAttachment(
            apvts, id, component);
        m_timeType.defaultCallback();
        m_sliders[Time].getSlider().onValueChange();
    };

    m_sliders[Time].getSlider().onValueChange = [this] {
        auto &timeTypeCombo = m_timeType.getComboBox();
        auto &slider        = m_sliders[Time].getSlider();
        auto value          = slider.getValue();
        if (!m_useBeats && value < 1.f) {
            timeTypeCombo.setText(juce::String(value * 1000) + "ms",
                                  juce::NotificationType::dontSendNotification);
        } else {
            timeTypeCombo.setText(slider.getTextFromValue(value),
                                  juce::NotificationType::dontSendNotification);
        }
    };

    m_active.onClick = [this]() {
        bool active = m_active.getToggleState();
        for (auto &slider : m_sliders) {
            slider.setEnabled(active);
        }
        m_timeType.setEnabled(active);
        m_mode.setEnabled(active);
    };

    // set colours
    const auto mainColour = juce::Colour(0xff609ccd);
    m_sliders[DryWet].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Time].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Feedback].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[CutLow].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[CutHi].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Saturation].setColour(juce::Slider::thumbColourId, mainColour);
    m_sliders[Drift].setColour(juce::Slider::thumbColourId, mainColour);

    m_mode.setArrowsColour(mainColour);
    m_timeType.setArrowsColour(mainColour);

    m_active.setColour(juce::ToggleButton::tickColourId, mainColour);
}

void DelaySection::resized()
{
    auto bounds = getLocalBounds();

    constexpr auto margin = CustomLNF::padding / 2;
    bounds.removeFromTop(margin);
    bounds.removeFromLeft(margin);
    bounds.removeFromRight(margin);

    auto titleBounds =
        bounds.removeFromTop(CustomLNF::subtitleSize + margin * 2);

    juce::FlexBox titleFb;
    titleFb.flexDirection = juce::FlexBox::Direction::row;
    titleFb.alignContent  = juce::FlexBox::AlignContent::center;
    titleFb.items         = {
        juce::FlexItem(m_active).withFlex(1.f).withMargin({margin}),
        juce::FlexItem(m_mode).withFlex(1.f).withMaxWidth(120).withMargin(
            {margin}),
    };
    titleFb.performLayout(titleBounds);

    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr    = juce::Grid::Fr;
    using Span  = juce::GridItem::Span;

    grid.templateColumns = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),
                            Track(Fr(1))};
    grid.templateRows    = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1))};

    grid.items = {
        juce::GridItem(m_sliders[Time])
            .withArea(1, 1, Span(2), Span(2))
            .withMargin(margin),
        juce::GridItem(m_sliders[Feedback])
            .withArea(1, 3, Span(1), Span(2))
            .withMargin(margin),
        juce::GridItem(m_sliders[DryWet])
            .withArea(2, 3, Span(1), Span(2))
            .withMargin(margin),
        juce::GridItem(m_sliders[CutLow]).withArea(3, 1).withMargin(margin),
        juce::GridItem(m_sliders[CutHi]).withArea(3, 2).withMargin(margin),
        juce::GridItem(m_sliders[Saturation]).withArea(3, 3).withMargin(margin),
        juce::GridItem(m_sliders[Drift]).withArea(3, 4).withMargin(margin),
    };
    grid.performLayout(bounds);

    // move middle element closer
    {
        auto &drywet    = m_sliders[DryWet].getComponent();
        auto drywetSize = juce::jmin(drywet.getWidth(), drywet.getHeight());
        auto sizeDiff   = (m_sliders[Time].getWidth() - drywetSize) * 0.5f;
        if (sizeDiff > 0.f) {
            auto interMargin = 0.15f * sizeDiff;
            m_sliders[Time].setBounds(m_sliders[Time].getBounds().translated(
                sizeDiff - interMargin, 0));
            m_sliders[Feedback].setBounds(
                m_sliders[Feedback].getBounds().translated(interMargin, 0));
            m_sliders[DryWet].setBounds(
                m_sliders[DryWet].getBounds().translated(interMargin, 0));
        }
    }

    // place time type & led
    {
        auto timeBounds     = m_sliders[Time].getBounds();
        auto timeTypeBounds = m_sliders[Time].getLabel().getBounds();
        timeTypeBounds.translate(timeBounds.getX(), timeBounds.getY());
        auto timeTypeWidth = juce::jmin(80, timeTypeBounds.getWidth());
        m_timeType.setBounds(timeTypeBounds.withWidth(timeTypeWidth)
                                 .withCentre(timeTypeBounds.getCentre()));

        constexpr auto ledSize = 15;
        auto ledBounds         = timeBounds.removeFromTop(ledSize);
        ledBounds              = ledBounds.removeFromRight(ledSize);
        m_led.setBounds(ledBounds);
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
    g.setColour(findColour(SpringsSection::backgroundColourId));
    g.fillRect(bounds.getX(), ySep - CustomLNF::sepWidth / 2.f,
               bounds.getWidth(), CustomLNF::sepWidth);
}

} // namespace aether
