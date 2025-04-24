#include "DelaySection.h"
#include "../PluginProcessor.h"
#include "CustomLNF.h"
#include "SpringsSection.h"
#include "Typefaces.h"
#include "juce_core/juce_core.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

namespace aether
{

DelaySection::DelaySection(PluginProcessor &processor) :
    sliders_{
        SliderWithLabel(processor.getAPVTS(), std::get<0>(kElements[0]),
                        std::get<1>(kElements[0])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(kElements[1]),
                        std::get<1>(kElements[1])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(kElements[2]),
                        std::get<1>(kElements[2])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(kElements[3]),
                        std::get<1>(kElements[3])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(kElements[4]),
                        std::get<1>(kElements[4])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(kElements[5]),
                        std::get<1>(kElements[5])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(kElements[6]),
                        std::get<1>(kElements[6])),
    },
    active_("Delay"),
    activeAttachment_(processor.getAPVTS(), "delay_active", active_),

    mode_("Mode"),
    modeAttachment_(processor.getAPVTS(), "delay_mode", mode_.getComboBox()),
    timeType_("TimeType"),
    timeTypeAttachment_(processor.getAPVTS(), "delay_time_type",
                        timeType_.getComboBox()),
    led_(processor.getSwitchIndicator())
{
    setName("Delay");

    addAndMakeVisible(active_);
    addAndMakeVisible(led_);

    // set colours
    const auto mainColour = juce::Colour(CustomLNF::kDelayMainColour);

    mode_.setArrowsColour(mainColour);
    timeType_.setArrowsColour(mainColour);

    active_.setColour(juce::ToggleButton::tickColourId, mainColour);

    for (auto &slider : sliders_) {
        addAndMakeVisible(slider);
        slider.getComponent().setPopupDisplayEnabled(true, false,
                                                     getTopLevelComponent());
        slider.getComponent().setColour(juce::Slider::thumbColourId,
                                        mainColour);
    }

    sliders_[kTime].getComponent().setPopupDisplayEnabled(false, false,
                                                          nullptr);
    sliders_[kTime].getComponent().setHasOutline(true);
    sliders_[kTime].getLabel().setText(
        "", juce::NotificationType::dontSendNotification);

    const auto symbolFont = juce::Font(Typefaces::getInstance()->symbols)
                                .withPointHeight(CustomLNF::kTextPointHeight);
    sliders_[kCutHi].getComponent().setPolarity(Slider::kUnipolarReversed);
    sliders_[kCutHi].getLabel().setText(
        juce::String::fromUTF8(u8"╭"),
        juce::NotificationType::dontSendNotification);
    sliders_[kCutHi].getLabel().setFont(symbolFont);

    sliders_[kCutLow].getLabel().setText(
        juce::String::fromUTF8(u8"╮"),
        juce::NotificationType::dontSendNotification);
    sliders_[kCutLow].getLabel().setFont(symbolFont);

    sliders_[kDryWet].getComponent().setTextValueSuffix("%");
    sliders_[kTime].getComponent().setTextValueSuffix("s");
    sliders_[kFeedback].getComponent().setTextValueSuffix("%");
    sliders_[kCutLow].getComponent().setTextValueSuffix("Hz");
    sliders_[kCutHi].getComponent().setTextValueSuffix("Hz");
    sliders_[kSaturation].getComponent().setTextValueSuffix("dB");
    sliders_[kDrift].getComponent().setTextValueSuffix("%");

    active_.setName("Delay");
    active_.setTitle(active_.getName());

    active_.setTooltip("Bypass section.");
    sliders_[kDryWet].getComponent().setTooltip(
        "Dry/wet proportion of the output signal");
    sliders_[kTime].getComponent().setTooltip("Duration of the delay.");
    sliders_[kFeedback].getComponent().setTooltip(
        "How much of the delayed signal is fed back into the delay. Can be "
        "more than 100% if drive is positive.");
    sliders_[kCutLow].getComponent().setTooltip(
        "Cutoff frequency of the low pass filter.");
    sliders_[kCutHi].getComponent().setTooltip(
        "Cutoff frequency of the high pass filter.");
    sliders_[kSaturation].getComponent().setTooltip(
        "Saturation level of the delayed signal in decibels.");
    sliders_[kDrift].getComponent().setTooltip(
        "How much the tape speed will be modulated. This works as pitch "
        "wobble.");
    mode_.getComboBox().setTooltip(
        "[Normal] normal echoes - [Back & Forth] alternates "
        "between forwards and reverse - [Reverse] reverse echoes.");
    mode_.getComboBox().setTitle("Mode");
    timeType_.getComboBox().setTooltip(
        "Set delay in seconds or relative to the host bpm.");
    timeType_.getComboBox().setTitle("Type");

    // processor apvts
    auto &apvts = processor.getAPVTS();

    addAndMakeVisible(mode_);
    mode_.getComboBox().addItemList(
        apvts.getParameter("delay_mode")->getAllValueStrings(), 1);

    addAndMakeVisible(timeType_);
    auto &timeTypeComboBox = timeType_.getComboBox();
    timeTypeComboBox.addItemList(
        apvts.getParameter("delay_time_type")->getAllValueStrings(), 1);

    timeTypeComboBox.onChange = [this, &apvts]() {
        auto &component  = sliders_[kTime].getComponent();
        auto &attachment = sliders_[kTime].getAttachment();
        auto &comboBox   = timeType_.getComboBox();
        auto selected    = comboBox.getSelectedId();

        juce::String id;
        juce::String suffix;
        switch (selected) {
        default:
        case 1:
            id        = "delay_seconds";
            suffix    = "s";
            useBeats_ = false;
            break;
        case 2:
            id        = "delay_beats";
            suffix    = "";
            useBeats_ = true;
            break;
        }
        component.setTextValueSuffix(suffix);
        // me must do this weird trick because we cannot replace attachment
        attachment.~SliderAttachment();
        new (&attachment) juce::AudioProcessorValueTreeState::SliderAttachment(
            apvts, id, component);
        timeType_.defaultCallback();
        sliders_[kTime].getSlider().onValueChange();
    };

    // limit seconds size to one third when in reverse mode
    auto *secondsParam = processor.getAPVTS().getParameter("delay_seconds");
    auto oneThird      = secondsParam->convertTo0to1(
        secondsParam->convertFrom0to1(1.f) /
        processors::TapeDelay::kReverseDelayMaxRatio);
    mode_.getComboBox().onChange = [this, oneThird] {
        auto &comboBox = mode_.getComboBox();
        auto selected  = comboBox.getSelectedId();
        auto &slider   = sliders_[kTime].getComponent();

        slider.setMaxPos(selected == 3 ? oneThird : 1.f);
    };

    sliders_[kTime].getSlider().onValueChange = [this] {
        auto &timeTypeCombo = timeType_.getComboBox();
        auto &slider        = sliders_[kTime].getSlider();
        auto value          = slider.getValue();
        if (!useBeats_ && value < 1.f) {
            timeTypeCombo.setText(juce::String(juce::roundToInt(value * 1000)) +
                                      "ms",
                                  juce::NotificationType::dontSendNotification);
        } else {
            timeTypeCombo.setText(slider.getTextFromValue(value),
                                  juce::NotificationType::dontSendNotification);
        }
    };

    active_.onClick = [this]() {
        bool active = active_.getToggleState();
        for (auto &slider : sliders_) {
            slider.setEnabled(active);
        }
        timeType_.setEnabled(active);
        mode_.setEnabled(active);
    };
}

void DelaySection::resized()
{
    auto bounds = getLocalBounds();

    constexpr auto kMargin = CustomLNF::kPadding / 2;
    bounds.removeFromTop(kMargin);
    bounds.removeFromLeft(kMargin);
    bounds.removeFromRight(kMargin);

    auto titleBounds =
        bounds.removeFromTop(CustomLNF::kSubtitleSize + kMargin * 2);

    juce::FlexBox titleFb;
    titleFb.flexDirection = juce::FlexBox::Direction::row;
    titleFb.alignContent  = juce::FlexBox::AlignContent::center;
    titleFb.items         = {
        juce::FlexItem(active_).withFlex(1.f).withMargin({kMargin}),
        juce::FlexItem(mode_).withFlex(1.f).withMaxWidth(120).withMargin(
            {kMargin}),
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
        juce::GridItem(sliders_[kTime])
            .withArea(1, 1, Span(2), Span(2))
            .withMargin(kMargin),
        juce::GridItem(sliders_[kFeedback])
            .withArea(1, 3, Span(1), Span(2))
            .withMargin(kMargin),
        juce::GridItem(sliders_[kDryWet])
            .withArea(2, 3, Span(1), Span(2))
            .withMargin(kMargin),
        juce::GridItem(sliders_[kCutLow]).withArea(3, 1).withMargin(kMargin),
        juce::GridItem(sliders_[kCutHi]).withArea(3, 2).withMargin(kMargin),
        juce::GridItem(sliders_[kSaturation])
            .withArea(3, 3)
            .withMargin(kMargin),
        juce::GridItem(sliders_[kDrift]).withArea(3, 4).withMargin(kMargin),
    };
    grid.performLayout(bounds);

    // move middle element closer
    {
        auto &drywet    = sliders_[kDryWet].getComponent();
        auto drywetSize = juce::jmin(drywet.getWidth(), drywet.getHeight());
        auto sizeDiff   = (sliders_[kTime].getWidth() - drywetSize) * 0.5f;
        if (sizeDiff > 0.f) {
            auto interMargin = 0.15f * sizeDiff;
            sliders_[kTime].setBounds(sliders_[kTime].getBounds().translated(
                sizeDiff - interMargin, 0));
            sliders_[kFeedback].setBounds(
                sliders_[kFeedback].getBounds().translated(interMargin, 0));
            sliders_[kDryWet].setBounds(
                sliders_[kDryWet].getBounds().translated(interMargin, 0));
        }
    }

    // place time type & led
    {
        auto timeBounds     = sliders_[kTime].getBounds();
        auto timeTypeBounds = sliders_[kTime].getLabel().getBounds();
        timeTypeBounds.translate(timeBounds.getX(), timeBounds.getY());
        auto timeTypeWidth = juce::jmin(80, timeTypeBounds.getWidth());
        timeType_.setBounds(timeTypeBounds.withWidth(timeTypeWidth)
                                .withCentre(timeTypeBounds.getCentre()));

        constexpr auto kLedSize = 15;
        auto ledBounds          = timeBounds.removeFromTop(kLedSize);
        ledBounds               = ledBounds.removeFromRight(kLedSize);
        led_.setBounds(ledBounds);
    }
}

void DelaySection::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(findColour(kBackgroundColourId));
    juce::Path box;
    box.addRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(),
                            bounds.getHeight(), CustomLNF::kBoxRoundSize,
                            CustomLNF::kBoxRoundSize, true, false, true, false);
    g.fillPath(box);

    // separator
    auto ySep = (sliders_[kDryWet].getBoundsInParent().getBottom() +
                 sliders_[kCutLow].getBoundsInParent().getY()) /
                2.f;
    g.setColour(findColour(SpringsSection::kBackgroundColourId));
    g.fillRect(bounds.getX(), ySep - CustomLNF::kSepWidth / 2.f,
               bounds.getWidth(), CustomLNF::kSepWidth);
}

} // namespace aether
