#include "SpringsSection.h"

constexpr auto VolPanFlex = 3;

SpringsSection::SpringsSection(juce::AudioProcessorValueTreeState &apvts) :
    springs{
        {apvts, 0}, {apvts, 1}, {apvts, 2}, {apvts, 3},
        {apvts, 4}, {apvts, 5}, {apvts, 6}, {apvts, 7},
    }
{
    unsigned int i = 0;
    for (auto &l : labels) {
        l.setText(std::get<1>(elements[i]), juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
        ++i;
    }

    for (auto &s : springs) addAndMakeVisible(s);
}

void SpringsSection::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.flexWrap      = juce::FlexBox::Wrap::noWrap;
    fb.alignContent  = juce::FlexBox::AlignContent::center;

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

SpringsSection::Spring::Spring(juce::AudioProcessorValueTreeState &apvts,
                               int id) :
    m_id(id),
    params{
        SpringParam(apvts, std::get<0>(elements[0]), id),
        SpringParam(apvts, std::get<0>(elements[1]), id),
        SpringParam(apvts, std::get<0>(elements[2]), id),
        SpringParam(apvts, std::get<0>(elements[3]), id),
        SpringParam(apvts, std::get<0>(elements[4]), id),
        SpringParam(apvts, std::get<0>(elements[5]), id),
        SpringParam(apvts, std::get<0>(elements[6]), id),
        SpringParam(apvts, std::get<0>(elements[7]), id),
        SpringParam(apvts, std::get<0>(elements[8]), id),
    },
    muteAttachment(apvts, "spring" + juce::String(id) + "_mute", mute),
    soloAttachment(apvts, "spring" + juce::String(id) + "_solo", solo)
{
    mute.setButtonText("M");
    solo.setButtonText("S");
    mute.setToggleable(true);
    solo.setToggleable(true);
    mute.setClickingTogglesState(true);
    solo.setClickingTogglesState(true);
    addAndMakeVisible(mute);
    addAndMakeVisible(solo);

    params[0].slider.setSliderStyle(
        juce::Slider::SliderStyle::LinearHorizontal);
    for (auto &p : params) addAndMakeVisible(p.slider);
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
        juce::FlexItem(params[1].slider).withFlex(3).withMargin(0),
        juce::FlexItem(mute).withFlex(1),
        juce::FlexItem(solo).withFlex(1),
    });

    fbLeft.items.addArray({
        juce::FlexItem(fbTop).withFlex(1).withMargin(0),
        juce::FlexItem(params[0].slider).withFlex(0.5).withMargin(0),
    });

    fb.items.add(juce::FlexItem(fbLeft).withFlex(VolPanFlex));
    for (unsigned int i = 2; i < elements.size(); ++i)
        fb.items.add(
            juce::FlexItem(params[i].slider).withFlex(1).withMargin(0));

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
