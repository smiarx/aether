#include "SpringsSection.h"

static constexpr auto springTopHeight = 50.f;
static constexpr auto springBoxWidth  = 150.f;
static constexpr auto indent          = 3;

SpringsSection::SpringsSection(juce::AudioProcessorValueTreeState &apvts) :
    springs{
        {apvts, 0}, {apvts, 1}, {apvts, 2}, {apvts, 3},
        {apvts, 4}, {apvts, 5}, {apvts, 6}, {apvts, 7},
    },
    macros{apvts}
{
    for (auto &s : springs) {
        addAndMakeVisible(s);
    }
}

void SpringsSection::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fb.alignContent  = juce::FlexBox::AlignContent::center;

    juce::FlexBox fbSprings;
    fbSprings.flexDirection = juce::FlexBox::Direction::row;
    fbSprings.flexWrap      = juce::FlexBox::Wrap::wrap;
    fbSprings.alignContent  = juce::FlexBox::AlignContent::center;
    for (auto &s : springs)
        fbSprings.items.add(juce::FlexItem(s)
                                .withFlex(1.f)
                                .withMinWidth(springBoxWidth)
                                .withMinHeight(180.f));

    fb.items.add(juce::FlexItem(fbSprings).withFlex(8.f));

    fb.performLayout(getLocalBounds());
}

#define sliderid(string, id) ("spring" + juce::String(id) + "_" + string)

SpringsSection::Spring::Spring(juce::AudioProcessorValueTreeState &apvts,
                               int id) :
    m_id(id),
    params{
        Slider(apvts, sliderid(std::get<0>(elements[0]), id),
               std::get<1>(elements[0])),
        Slider(apvts, sliderid(std::get<0>(elements[1]), id),
               std::get<1>(elements[1])),
        Slider(apvts, sliderid(std::get<0>(elements[2]), id),
               std::get<1>(elements[2])),
        Slider(apvts, sliderid(std::get<0>(elements[3]), id),
               std::get<1>(elements[3])),
        Slider(apvts, sliderid(std::get<0>(elements[4]), id),
               std::get<1>(elements[4])),
        Slider(apvts, sliderid(std::get<0>(elements[5]), id),
               std::get<1>(elements[5])),
        Slider(apvts, sliderid(std::get<0>(elements[6]), id),
               std::get<1>(elements[6])),
        Slider(apvts, sliderid(std::get<0>(elements[7]), id),
               std::get<1>(elements[7])),
        Slider(apvts, sliderid(std::get<0>(elements[8]), id),
               std::get<1>(elements[8])),
    },
    muteAttachment(apvts, "spring" + juce::String(id) + "_mute", mute),
    soloAttachment(apvts, "spring" + juce::String(id) + "_solo", solo)
{
#undef sliderid
    setText("Spring " + juce::String(id + 1));
    setTextLabelPosition(juce::Justification::centredTop);

    mute.setButtonText("M");
    solo.setButtonText("S");
    mute.setToggleable(true);
    solo.setToggleable(true);
    mute.setClickingTogglesState(true);
    solo.setClickingTogglesState(true);
    addAndMakeVisible(mute);
    addAndMakeVisible(solo);

    params[0].getSlider().setSliderStyle(
        juce::Slider::SliderStyle::LinearHorizontal);
    /* remove label for volume and pan */
    params[0].setLabelVisiblie(false);
    params[1].setLabelVisiblie(false);

    for (auto &p : params) {
        addAndMakeVisible(p);
        p.getComponent().setPopupDisplayEnabled(true, false,
                                                getTopLevelComponent());
    }
}

void SpringsSection::Spring::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(indent, indent * 3);
    auto topBounds = bounds.removeFromTop(springTopHeight);

    juce::FlexBox fbTop;
    fbTop.flexDirection = juce::FlexBox::Direction::row;
    fbTop.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fbTop.alignContent  = juce::FlexBox::AlignContent::center;
    fbTop.alignItems    = juce::FlexBox::AlignItems::center;

    constexpr auto elHeight = 30;
    constexpr auto elMargin = 1;
    fbTop.items.addArray({
        juce::FlexItem(params[0])
            .withFlex(4)
            .withHeight(elHeight)
            .withMargin(0)
            .withMargin(elMargin),
        juce::FlexItem(mute)
            .withFlex(1)
            .withHeight(elHeight)
            .withMaxWidth(elHeight)
            .withMargin(elMargin),
        juce::FlexItem(solo)
            .withFlex(1)
            .withHeight(elHeight)
            .withMaxWidth(elHeight)
            .withMargin(elMargin),
        juce::FlexItem(params[1])
            .withFlex(1)
            .withHeight(elHeight)
            .withMargin(0)
            .withMargin(elMargin),
    });
    fbTop.performLayout(topBounds);

    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;
    fb.flexWrap      = juce::FlexBox::Wrap::wrap;
    fb.alignContent  = juce::FlexBox::AlignContent::center;
    fb.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;

    constexpr auto dialSize = 45.f;
    for (unsigned int i = 2; i < elements.size(); ++i)
        fb.items.add(juce::FlexItem(params[i])
                         .withFlex(1)
                         .withMinWidth(springBoxWidth / 4.f)
                         .withMinHeight(dialSize)
                         .withMargin(0));

    fb.performLayout(bounds);
}

void SpringsSection::Spring::paint(juce::Graphics &g)
{
    const auto bounds = getLocalBounds();
    auto outline      = findColour(juce::GroupComponent::outlineColourId);
    auto lineY        = bounds.getY() + indent * 3 + springTopHeight;

    auto fillTopBounds = bounds.toFloat();
    fillTopBounds.reduce(indent, indent * 3);
    fillTopBounds = fillTopBounds.removeFromTop(springTopHeight);
    g.setColour(juce::Colours::darkgrey.darker(0.3));
    g.fillRoundedRectangle(fillTopBounds, 4.f);

    auto fillBottomBounds = bounds.toFloat();
    fillBottomBounds.reduce(indent, indent);
    fillBottomBounds.removeFromTop(springTopHeight + 2 * indent);
    g.setColour(juce::Colours::darkgrey.brighter(0.15));
    g.fillRoundedRectangle(fillBottomBounds, 4.f);

    getLookAndFeel().drawGroupComponentOutline(g, bounds.getWidth(),
                                               bounds.getHeight(), getText(),
                                               getTextLabelPosition(), *this);

    g.setColour(outline);
    g.drawLine(bounds.getX() + indent, lineY,
               bounds.getX() + bounds.getWidth() - indent, lineY, 2.f);
}

SpringsSection::Macros::Macros(juce::AudioProcessorValueTreeState &apvts) :
    params{
        Macro(apvts, std::get<0>(elements[2]), std::get<1>(elements[2])),
        Macro(apvts, std::get<0>(elements[3]), std::get<1>(elements[3])),
        Macro(apvts, std::get<0>(elements[4]), std::get<1>(elements[4])),
        Macro(apvts, std::get<0>(elements[5]), std::get<1>(elements[5])),
        Macro(apvts, std::get<0>(elements[6]), std::get<1>(elements[6])),
        Macro(apvts, std::get<0>(elements[7]), std::get<1>(elements[7])),
        Macro(apvts, std::get<0>(elements[8]), std::get<1>(elements[8])),
    }
{
    for (auto &macro : params) {
        addAndMakeVisible(macro);
        macro.getComponent().setPopupDisplayEnabled(true, false,
                                                    getTopLevelComponent());
    }
}

void SpringsSection::Macros::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;
    fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fb.alignContent  = juce::FlexBox::AlignContent::center;

    for (auto &macro : params)
        fb.items.add(juce::FlexItem(macro).withFlex(1.f));

    fb.performLayout(getLocalBounds());
}
