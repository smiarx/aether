#include "SpringsSection.h"

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
            fbRow[i].items.add(juce::FlexItem(s).withFlex(1.f));
        }

        fb.items.add(juce::FlexItem(fbRow[i]).withFlex(1.f));
    }

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
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fb.alignContent  = juce::FlexBox::AlignContent::center;

    juce::FlexBox fbTop;
    fbTop.flexDirection = juce::FlexBox::Direction::row;
    fbTop.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fbTop.alignContent  = juce::FlexBox::AlignContent::center;

    juce::FlexBox fbMuteSolo;
    fbMuteSolo.flexDirection = juce::FlexBox::Direction::column;
    fbMuteSolo.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fbMuteSolo.alignContent  = juce::FlexBox::AlignContent::center;

    fbMuteSolo.items.addArray({
        juce::FlexItem(solo).withFlex(1.f),
        juce::FlexItem(mute).withFlex(1.f),
    });

    fbTop.items.addArray({
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
