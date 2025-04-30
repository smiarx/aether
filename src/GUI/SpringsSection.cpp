#include "SpringsSection.h"
#include "../PluginProcessor.h"
#include "CustomLNF.h"
#include "DelaySection.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

namespace aether
{

SpringsSection::SpringsSection(PluginProcessor &processor) :
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
        SliderWithLabel(processor.getAPVTS(), std::get<0>(kElements[7]),
                        std::get<1>(kElements[7])),
        SliderWithLabel(processor.getAPVTS(), std::get<0>(kElements[8]),
                        std::get<1>(kElements[8])),
    },
    active_("Reverb"),
    activeAttachment_(processor.getAPVTS(), "springs_active", active_),
    springsGl_(processor)
{
    setName("Springs");

    addAndMakeVisible(springsGl_);
    addAndMakeVisible(active_);

    for (auto &slider : sliders_) {
        addAndMakeVisible(slider);
        slider.getComponent().setPopupDisplayEnabled(true, false,
                                                     getTopLevelComponent());
    }

    active_.setName("Reverb");
    active_.setTitle(active_.getName());
    springsGl_.setName("Springs");
    springsGl_.setTitle(springsGl_.getName());

    sliders_[kShape].getComponent().setPolarity(Slider::kBipolar);

    sliders_[kDryWet].getComponent().setTextValueSuffix("%");
    sliders_[kWidth].getComponent().setTextValueSuffix("%");
    sliders_[kLength].getComponent().setTextValueSuffix("s");
    sliders_[kDecay].getComponent().setTextValueSuffix("s");
    sliders_[kDamp].getComponent().setTextValueSuffix("Hz");
    sliders_[kChaos].getComponent().setTextValueSuffix("%");
    sliders_[kScatter].getComponent().setTextValueSuffix("%");

    active_.setTooltip("Bypass section.");
    sliders_[kDryWet].getComponent().setTooltip(
        "Dry/wet proportion of the output signal.");
    sliders_[kWidth].getComponent().setTooltip(
        "Stereo width of the output signal.");
    sliders_[kLength].getComponent().setTooltip(
        "Length of the echoes produced by the springs.");
    sliders_[kDecay].getComponent().setTooltip(
        "How long the reverb takes to fade out.");
    sliders_[kDamp].getComponent().setTooltip(
        "Frequency of the highest components produced by the reverb.");
    sliders_[kShape].getComponent().setTooltip(
        "Shape of the frequency dispertion of the springs.");
    sliders_[kTone].getComponent().setTooltip("Set the tone of the reverb.");
    sliders_[kChaos].getComponent().setTooltip(
        "How stochastic & unpredictible the springs become.");
    sliders_[kScatter].getComponent().setTooltip(
        "How similar or different are the springs properties.");
    springsGl_.setTooltip("Click to shake springs.");

    static const auto kMainColour = juce::Colour(CustomLNF::kSpringsMainColour);

    active_.setColour(juce::ToggleButton::tickColourId, kMainColour);

    sliders_[kDryWet].setColour(juce::Slider::thumbColourId, kMainColour);
    sliders_[kWidth].setColour(juce::Slider::thumbColourId, kMainColour);
    sliders_[kLength].setColour(juce::Slider::thumbColourId, kMainColour);
    sliders_[kDecay].setColour(juce::Slider::thumbColourId, kMainColour);
    sliders_[kDamp].setColour(juce::Slider::thumbColourId, kMainColour);
    sliders_[kShape].setColour(juce::Slider::thumbColourId, kMainColour);
    sliders_[kTone].setColour(juce::Slider::thumbColourId, kMainColour);
    sliders_[kScatter].setColour(juce::Slider::thumbColourId, kMainColour);
    sliders_[kChaos].setColour(juce::Slider::thumbColourId, kMainColour);

    sliders_[kDecay].setValueAsLabel();
    sliders_[kDecay].getComponent().setHasOutline(true);

    active_.onClick = [this] {
        bool active = active_.getToggleState();
        for (auto &slider : sliders_) {
            slider.setEnabled(active);
        }
    };
}

void SpringsSection::resized()
{
    auto bounds = getLocalBounds();

    constexpr auto kMargin = CustomLNF::kPadding / 2;
    bounds.removeFromTop(kMargin);
    bounds.removeFromLeft(kMargin);

    auto titleBounds =
        bounds.removeFromTop(CustomLNF::kSubtitleSize + 2 * kMargin);

    juce::FlexBox titleFb;
    titleFb.flexDirection = juce::FlexBox::Direction::row;
    titleFb.alignContent  = juce::FlexBox::AlignContent::center;
    titleFb.items         = {
        juce::FlexItem(active_)
            .withFlex(1.f)
            .withMaxWidth(100) // 100 is arbitrary we should compute value
            .withMargin(kMargin),
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
        juce::GridItem(sliders_[kDecay])
            .withArea(1, 1, Span(2), Span(2))
            .withMargin({kMargin}),
        juce::GridItem(sliders_[kLength])
            .withArea(1, 3, Span(1), Span(2))
            .withMargin({kMargin}),
        juce::GridItem(sliders_[kDamp])
            .withArea(2, 3, Span(1), Span(2))
            .withMargin({kMargin}),
        juce::GridItem(sliders_[kShape]).withArea(3, 1).withMargin({kMargin}),
        juce::GridItem(sliders_[kScatter]).withArea(3, 2).withMargin({kMargin}),
        juce::GridItem(sliders_[kChaos]).withArea(3, 3).withMargin({kMargin}),
        juce::GridItem(sliders_[kTone]).withArea(3, 4).withMargin({kMargin}),
        juce::GridItem(springsGl_).withArea(1, 5).withMargin({kMargin}),
        juce::GridItem(sliders_[kWidth]).withArea(2, 5).withMargin({kMargin}),
        juce::GridItem(sliders_[kDryWet]).withArea(3, 5).withMargin({kMargin}),
    };
    grid.performLayout(bounds);

    // move middle element closer
    {
        auto &damp    = sliders_[kDamp].getComponent();
        auto dampSize = juce::jmin(damp.getWidth(), damp.getHeight());
        auto sizeDiff =
            static_cast<float>(sliders_[kDecay].getWidth() - dampSize) * 0.5f;
        if (sizeDiff > 0.f) {
            auto interMargin  = 0.15f * sizeDiff;
            auto iInterMargin = static_cast<int>(interMargin);
            sliders_[kDecay].setBounds(sliders_[kDecay].getBounds().translated(
                static_cast<int>(sizeDiff - interMargin), 0));
            sliders_[kLength].setBounds(
                sliders_[kLength].getBounds().translated(iInterMargin, 0));
            sliders_[kDamp].setBounds(
                sliders_[kDamp].getBounds().translated(iInterMargin, 0));
        }
    }

    // extend spring gl
    auto glBounds = springsGl_.getBounds().toFloat();
    glBounds.translate(0, -kHeaderHeight);
    glBounds.setHeight(glBounds.getHeight() + kHeaderHeight);
    springsGl_.setBounds(glBounds.toNearestInt());
}

void SpringsSection::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(findColour(kBackgroundColourId));

    juce::Path box;
    box.addRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(),
                            bounds.getHeight(), CustomLNF::kBoxRoundSize,
                            CustomLNF::kBoxRoundSize, false, true, false, true);
    g.fillPath(box);

    // separators;
    g.setColour(findColour(DelaySection::kBackgroundColourId));

    auto xSep =
        static_cast<float>(sliders_[kTone].getBoundsInParent().getRight() +
                           springsGl_.getBoundsInParent().getX()) /
        2.f;
    g.fillRect(xSep - CustomLNF::kSepWidth / 2.f, bounds.getY(),
               CustomLNF::kSepWidth, bounds.getHeight());

    auto ySep =
        static_cast<float>(sliders_[kDamp].getBoundsInParent().getBottom() +
                           sliders_[kShape].getBoundsInParent().getY()) /
        2.f;
    auto yWidth = xSep - bounds.getX();
    g.fillRect(bounds.getX(), ySep - CustomLNF::kSepWidth / 2.f, yWidth,
               CustomLNF::kSepWidth);
}

} // namespace aether
