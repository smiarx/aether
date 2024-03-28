#include "SpringsSection.h"

constexpr auto VolPanFlex = 3;

SpringsSection::SpringsSection(juce::AudioProcessorValueTreeState &apvts) :
    springs{
        {apvts, 0}, {apvts, 1}, {apvts, 2}, {apvts, 3},
        {apvts, 4}, {apvts, 5}, {apvts, 6}, {apvts, 7},
    },
    macros{apvts}
{
    unsigned int i = 0;
    for (auto &l : labels) {
        l.setText(std::get<1>(elements[i]), juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
        ++i;
    }

    for (auto &s : springs) {
        addAndMakeVisible(s);
    }

    addAndMakeVisible(macros);
}

void SpringsSection::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fb.alignContent  = juce::FlexBox::AlignContent::center;

    fb.items.add(juce::FlexItem(macros).withFlex(1.f));

    juce::FlexBox fbLabels;
    fbLabels.flexDirection = juce::FlexBox::Direction::row;
    fbLabels.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fbLabels.alignContent  = juce::FlexBox::AlignContent::stretch;
    fbLabels.alignItems    = juce::FlexBox::AlignItems::stretch;
    fbLabels.items.add(juce::FlexItem().withFlex(VolPanFlex));
    for (unsigned int i = 2; i < elements.size(); ++i)
        fbLabels.items.add(juce::FlexItem(labels[i]).withFlex(1.f));
    fb.items.add(juce::FlexItem(fbLabels).withHeight(headerHeight));

    for (auto &s : springs) fb.items.add(juce::FlexItem(s).withFlex(1.f));

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
    for (auto &p : params) {
        addAndMakeVisible(p);
    }
}

void SpringsSection::Spring::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;
    fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fb.alignContent  = juce::FlexBox::AlignContent::center;

    juce::FlexBox fbLeft;
    fbLeft.flexDirection = juce::FlexBox::Direction::column;
    fbLeft.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fbLeft.alignContent  = juce::FlexBox::AlignContent::center;

    juce::FlexBox fbTop;
    fbTop.flexDirection = juce::FlexBox::Direction::row;
    fbTop.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fbTop.alignContent  = juce::FlexBox::AlignContent::center;

    fbTop.items.addArray({
        juce::FlexItem(params[1]).withFlex(3).withMargin(0),
        juce::FlexItem(mute).withFlex(1),
        juce::FlexItem(solo).withFlex(1),
    });

    fbLeft.items.addArray({
        juce::FlexItem(fbTop).withFlex(1).withMargin(0),
        juce::FlexItem(params[0]).withFlex(0.5).withMargin(0),
    });

    fb.items.add(juce::FlexItem(fbLeft).withFlex(VolPanFlex));
    for (unsigned int i = 2; i < elements.size(); ++i)
        fb.items.add(juce::FlexItem(params[i]).withFlex(1).withMargin(0));

    fb.performLayout(getLocalBounds());

    leftPanelRect = fb.items[0].currentBounds;
}

void SpringsSection::Spring::paint(juce::Graphics &g)
{
    g.setColour(m_id % 2 ? juce::Colours::royalblue
                         : juce::Colours::mediumseagreen);
    g.fillRect(getLocalBounds());

    g.setColour(juce::Colours::mediumpurple.withAlpha(0.8f));
    g.fillRect(leftPanelRect);
}

SpringsSection::Macros::Macros(juce::AudioProcessorValueTreeState &apvts) :
    params{
        Macro(apvts, std::get<0>(elements[2])),
        Macro(apvts, std::get<0>(elements[3])),
        Macro(apvts, std::get<0>(elements[4])),
        Macro(apvts, std::get<0>(elements[5])),
        Macro(apvts, std::get<0>(elements[6])),
        Macro(apvts, std::get<0>(elements[7])),
        Macro(apvts, std::get<0>(elements[8])),
    }
{
    for (auto &macro : params) {
        addAndMakeVisible(macro);
        macro.setPopupDisplayEnabled(true, false, getTopLevelComponent());
    }
}

void SpringsSection::Macros::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;
    fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fb.alignContent  = juce::FlexBox::AlignContent::center;

    fb.items.add(juce::FlexItem({}).withFlex(VolPanFlex));
    for (auto &macro : params)
        fb.items.add(juce::FlexItem(macro).withFlex(1.f));

    fb.performLayout(getLocalBounds());
}
