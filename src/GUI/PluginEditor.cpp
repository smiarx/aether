#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p) :
    AudioProcessorEditor(&p), audioProcessor(p), delaySection{p.getAPVTS()},
    springsSection{p.getAPVTS()}
{
    setLookAndFeel(&lookandfeel);
    addAndMakeVisible(title);
    addAndMakeVisible(delaySection);
    addAndMakeVisible(springsSection);
    // addAndMakeVisible(springsGL);
    addAndMakeVisible(resizableCorner);

    setSize(600, 280);

    title.setText(juce::String::fromUTF8(u8"æther"),
                  juce::dontSendNotification);
    title.setFont(juce::Font(titleHeight, juce::Font::bold));
    title.setColour(juce::Label::textColourId, juce::Colours::black);
}

void PluginEditor::resized()
{
    resizableCorner.setBounds(getWidth() - 16, getHeight() - 16, 16, 16);

    juce::FlexBox fbMain;
    fbMain.flexDirection = juce::FlexBox::Direction::column;

    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;

    fb.items.addArray({
        juce::FlexItem(delaySection).withFlex(0.68f).withMargin(0.f),
        juce::FlexItem(springsSection).withFlex(1.f).withMargin(0.f),
    });

    fbMain.items.addArray({
        juce::FlexItem(title)
            .withFlex(0.f)
            .withHeight(headerHeight)
            .withMargin({0, 0, 0, titleMargin}),
        juce::FlexItem(fb).withFlex(1.f),
    });

    // fb.items.add(
    //      juce::FlexItem(springsGL).withFlex(1.f).withMargin(0.f));

    auto bounds = getLocalBounds();
    bounds.reduce(CustomLNF::margin, CustomLNF::margin);
    fbMain.performLayout(bounds);
}

void PluginEditor::paint(juce::Graphics &g)
{
    g.fillAll(findColour(backgroundColourId));
}
