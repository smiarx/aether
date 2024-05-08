#include "SpringsSection.h"
#include "CustomLNF.h"

#include "../PluginProcessor.h"

static const auto mainColour  = juce::Colour(0xff8fe7f3);
static const auto smallColour = juce::Colour(0xfff130f1);

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
    /* draw springs boxes */
    constexpr auto nRows   = 2;
    constexpr auto rowSize = elements.size() / nRows;

    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fb.alignContent  = juce::FlexBox::AlignContent::center;

    juce::FlexBox fbRow[nRows];
    for (size_t i = 0; i < nRows; ++i) {
        fbRow[i].flexDirection = juce::FlexBox::Direction::row;
        fbRow[i].flexWrap      = juce::FlexBox::Wrap::noWrap;
        fbRow[i].alignContent  = juce::FlexBox::AlignContent::center;
        for (size_t j = 0; j < rowSize; ++j) {
            auto &s = springs[i * rowSize + j];
            fbRow[i].items.add(juce::FlexItem(s).withFlex(1.f).withMargin(1.f));
        }

        fb.items.add(juce::FlexItem(fbRow[i]).withFlex(1.f));
    }

    auto bounds = getLocalBounds();

    bounds.reduce(CustomLNF::padding, CustomLNF::padding);
    fb.performLayout(bounds);
}

void SpringsSection::paint(juce::Graphics &g)
{
    const auto bounds = getLocalBounds();
    g.setColour(findColour(allBackgroundColourId));
    g.fillRoundedRectangle(bounds.toFloat(), CustomLNF::boxRoundSize);
}

#define sliderid(string, id) ("spring" + juce::String(id) + "_" + string)

SpringsSection::Spring::Source::Source(APVTS &apvts, int id)
{
    constexpr auto sourceGroupId = 1000;
    left.setButtonText("L");
    right.setButtonText("R");
    middle.setButtonText("M");
    left.setClickingTogglesState(true);
    right.setClickingTogglesState(true);
    middle.setClickingTogglesState(true);
    left.setRadioGroupId(sourceGroupId + id);
    right.setRadioGroupId(sourceGroupId + id);
    middle.setRadioGroupId(sourceGroupId + id);

    const auto param = static_cast<juce::RangedAudioParameter *>(
        apvts.getParameter("spring" + juce::String(id) + "_source"));
    if (param == nullptr) return;
    param->addListener(this);
    parameterValueChanged(0, param->getValue());

    left.setTriggeredOnMouseDown(true);
    right.setTriggeredOnMouseDown(true);
    middle.setTriggeredOnMouseDown(true);

    left.onClick = [param] {
        param->setValueNotifyingHost(param->convertTo0to1(
            static_cast<int>(PluginProcessor::Source::Left)));
    };
    right.onClick = [param] {
        param->setValueNotifyingHost(param->convertTo0to1(
            static_cast<int>(PluginProcessor::Source::Right)));
    };
    middle.onClick = [param] {
        param->setValueNotifyingHost(param->convertTo0to1(
            static_cast<int>(PluginProcessor::Source::Mono)));
    };
}

void SpringsSection::Spring::Source::parameterValueChanged(
    int /*parameterIndex*/, float newValue)
{
    const auto iValue = static_cast<PluginProcessor::Source>(newValue * 2.f);
    switch (iValue) {
    case PluginProcessor::Source::Left:
        left.setToggleState(true, juce::NotificationType::dontSendNotification);
        break;
    case PluginProcessor::Source::Right:
        right.setToggleState(true,
                             juce::NotificationType::dontSendNotification);
        break;
    case PluginProcessor::Source::Mono:
        middle.setToggleState(true,
                              juce::NotificationType::dontSendNotification);
        break;
    default:
        break;
    }
}

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
    soloAttachment(apvts, "spring" + juce::String(id) + "_solo", solo),
    source(apvts, id)
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

    addAndMakeVisible(source.left);
    addAndMakeVisible(source.right);
    addAndMakeVisible(source.middle);

    params[0].getSlider().setSliderStyle(
        juce::Slider::SliderStyle::LinearHorizontal);
    /* remove label for parameters */
    for (auto &param : params) param.setLabelVisible(false);

    for (auto &p : params) {
        addAndMakeVisible(p);
        p.getComponent().setPopupDisplayEnabled(true, false,
                                                getTopLevelComponent());
    }

    // set colour
    params[0].setColour(juce::Slider::thumbColourId, mainColour);
    params[1].setColour(juce::Slider::thumbColourId, mainColour);
    params[2].setColour(juce::Slider::thumbColourId, mainColour);
    params[3].setColour(juce::Slider::thumbColourId, mainColour);
    params[4].setColour(juce::Slider::thumbColourId, mainColour);
    params[5].setColour(juce::Slider::thumbColourId, smallColour);
    params[6].setColour(juce::Slider::thumbColourId, smallColour);
    params[7].setColour(juce::Slider::thumbColourId, smallColour);
    params[8].setColour(juce::Slider::thumbColourId, smallColour);
}

void SpringsSection::Spring::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fb.alignContent  = juce::FlexBox::AlignContent::center;

    juce::FlexBox fbTop;
    fbTop.flexDirection = juce::FlexBox::Direction::row;
    fbTop.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fbTop.alignContent  = juce::FlexBox::AlignContent::center;

    juce::FlexBox fbSource;
    fbSource.flexDirection = juce::FlexBox::Direction::column;
    fbSource.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fbSource.alignContent  = juce::FlexBox::AlignContent::center;

    fbSource.items.addArray({
        juce::FlexItem(source.left).withFlex(1.f),
        juce::FlexItem(source.middle).withFlex(1.f),
        juce::FlexItem(source.right).withFlex(1.f),
    });

    juce::FlexBox fbMuteSolo;
    fbMuteSolo.flexDirection = juce::FlexBox::Direction::column;
    fbMuteSolo.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fbMuteSolo.alignContent  = juce::FlexBox::AlignContent::center;

    fbMuteSolo.items.addArray({
        juce::FlexItem(solo).withFlex(1.f),
        juce::FlexItem(mute).withFlex(1.f),
    });

    fbTop.items.addArray({
        juce::FlexItem(fbSource).withFlex(0.8f),
        juce::FlexItem(params[0]).withFlex(4.f),
        juce::FlexItem(params[1]).withFlex(1.f),
        juce::FlexItem(fbMuteSolo).withFlex(0.8f),
    });

    juce::FlexBox fbMid;
    fbMid.flexDirection = juce::FlexBox::Direction::row;
    fbMid.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fbMid.alignContent  = juce::FlexBox::AlignContent::center;
    fbMid.items.addArray({
        juce::FlexItem(params[2]).withFlex(1.f),
        juce::FlexItem(params[3]).withFlex(1.f),
        juce::FlexItem(params[4]).withFlex(1.f),
    });

    juce::FlexBox fbBot;
    fbBot.flexDirection = juce::FlexBox::Direction::row;
    fbBot.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fbBot.alignContent  = juce::FlexBox::AlignContent::center;
    fbBot.items.addArray({
        juce::FlexItem(params[5]).withFlex(1.f),
        juce::FlexItem(params[6]).withFlex(1.f),
        juce::FlexItem(params[7]).withFlex(1.f),
        juce::FlexItem(params[8]).withFlex(1.f),
    });

    constexpr auto margin = 4;
    fb.items.addArray({
        juce::FlexItem(fbTop).withFlex(1.f).withMargin(margin),
        juce::FlexItem(fbMid).withFlex(1.f).withMargin(margin),
        juce::FlexItem(fbBot).withFlex(0.8f).withMargin(margin),
    });

    constexpr auto indent = 3;
    auto bounds           = getLocalBounds();
    bounds.reduce(indent, indent);
    bounds.removeFromTop(indent);
    fb.performLayout(bounds);
}

void SpringsSection::Spring::paint(juce::Graphics &g)
{
    const auto bounds = getLocalBounds();
    g.setColour(findColour(backgroundColourId));
    g.fillRoundedRectangle(bounds.toFloat(), CustomLNF::boxRoundSize);
    getLookAndFeel().drawGroupComponentOutline(g, bounds.getWidth(),
                                               bounds.getHeight(), getText(),
                                               getTextLabelPosition(), *this);
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
    },
    drywet(apvts, "springs_drywet", "Dry/Wet"),
    width(apvts, "springs_width", "Width")
{
    for (auto &macro : params) {
        addAndMakeVisible(macro);
        macro.getComponent().setPopupDisplayEnabled(true, false,
                                                    getTopLevelComponent());
    }
    addAndMakeVisible(drywet);
    addAndMakeVisible(width);

    // set colours
    drywet.setColour(juce::Slider::thumbColourId, mainColour);
    width.setColour(juce::Slider::thumbColourId, mainColour);
    params[0].setColour(juce::Slider::thumbColourId, mainColour);
    params[1].setColour(juce::Slider::thumbColourId, mainColour);
    params[2].setColour(juce::Slider::thumbColourId, mainColour);
    params[3].setColour(juce::Slider::thumbColourId, smallColour);
    params[4].setColour(juce::Slider::thumbColourId, smallColour);
    params[5].setColour(juce::Slider::thumbColourId, smallColour);
    params[6].setColour(juce::Slider::thumbColourId, smallColour);
}

void SpringsSection::Macros::resized()
{
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr    = juce::Grid::Fr;
    using Span  = juce::GridItem::Span;

    grid.templateColumns = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),
                            Track(Fr(1)), Track(Fr(2))};
    grid.templateRows    = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1))};

    grid.items = {
        // juce::GridItem(params[0]).withArea("hilo"),
        juce::GridItem(params[0]).withArea(1, 1, Span(2), Span(2)), // length
        juce::GridItem(params[1]).withArea(1, 3, Span(1), Span(2)), // decay
        juce::GridItem(params[2]).withArea(2, 3, Span(1), Span(2)), // damp
        juce::GridItem(params[3]).withArea(3, 1),                   // hilo
        juce::GridItem(params[4]).withArea(3, 2),                   // disp
        juce::GridItem(params[5]).withArea(3, 3),                   // chaos
        juce::GridItem(params[6]).withArea(3, 4), // springness
        juce::GridItem(width).withArea(2, 5),
        juce::GridItem(drywet).withArea(3, 5),
    };

    auto bounds = getLocalBounds();
    bounds.reduce(CustomLNF::padding, CustomLNF::padding);
    grid.performLayout(bounds);
}

void SpringsSection::Macros::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(findColour(allBackgroundColourId));
    juce::Path box;
    box.addRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(),
                            bounds.getHeight(), CustomLNF::boxRoundSize,
                            CustomLNF::boxRoundSize, true, true, false, true);
    g.fillPath(box);

    bounds.reduce(CustomLNF::padding, CustomLNF::padding);
    g.setColour(findColour(backgroundColourId));
    g.fillRoundedRectangle(bounds, CustomLNF::boxRoundSize);
}
