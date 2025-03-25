#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p) :
    AudioProcessorEditor(&p), delaySection{p.getAPVTS()},
    springsSection{p.getAPVTS()}
{
    setResizable(true, true);

    setInterceptsMouseClicks(false, true);
    juce::Desktop::getInstance().addGlobalMouseListener(this);

    setLookAndFeel(&lookandfeel);
    addAndMakeVisible(title);
    addAndMakeVisible(tooltip);
    addAndMakeVisible(delaySection);
    addAndMakeVisible(springsSection);

    setSize(600, 280);

    title.setText(juce::String::fromUTF8(u8"æther"),
                  juce::dontSendNotification);
    title.setFont(juce::Font(titleHeight, juce::Font::bold));
    title.setColour(juce::Label::textColourId, juce::Colours::black);

    tooltip.setColour(juce::Label::textColourId, juce::Colours::black);
}

void PluginEditor::resized()
{
    juce::FlexBox fbMain;
    fbMain.flexDirection = juce::FlexBox::Direction::column;

    juce::FlexBox fbTitle;
    fbTitle.flexDirection = juce::FlexBox::Direction::row;
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;

    fbTitle.items.addArray({
        juce::FlexItem(title).withFlex(0.68f).withMargin(0.f),
        juce::FlexItem(tooltip).withFlex(1.f).withMargin(0.f),
    });

    fb.items.addArray({
        juce::FlexItem(delaySection).withFlex(0.68f).withMargin(0.f),
        juce::FlexItem(springsSection).withFlex(1.f).withMargin(0.f),
    });

    fbMain.items.addArray({
        juce::FlexItem(fbTitle)
            .withFlex(0.f)
            .withHeight(headerHeight)
            .withMargin({0, 0, 0, titleMargin}),
        juce::FlexItem(fb).withFlex(1.f),
    });

    auto bounds = getLocalBounds();
    bounds.reduce(CustomLNF::margin, CustomLNF::margin);
    fbMain.performLayout(bounds);
}

void PluginEditor::paint(juce::Graphics &g)
{
    g.fillAll(findColour(backgroundColourId));
}

void PluginEditor::mouseMove(const juce::MouseEvent &event)
{
    juce::AudioProcessorEditor::mouseMove(event);

    const auto mouseSource = juce::Desktop::getInstance().getMainMouseSource();
    auto *underMouse =
        mouseSource.isTouch() ? nullptr : mouseSource.getComponentUnderMouse();

    tooltip.setFromComponent(underMouse);
}
