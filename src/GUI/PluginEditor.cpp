#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p) :
    AudioProcessorEditor(&p), audioProcessor(p), delaySection{p.getAPVTS()},
    springsSection{p.getAPVTS()}
{
    setLookAndFeel(&lookandfeel);
    addAndMakeVisible(delaySection);
    addAndMakeVisible(springsSection);
    // addAndMakeVisible(springsGL);
    addAndMakeVisible(resizableCorner);

    setSize(700, 600);
}

void PluginEditor::resized()
{
    resizableCorner.setBounds(getWidth() - 16, getHeight() - 16, 16, 16);

    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;

    fb.items.addArray({
        juce::FlexItem(delaySection).withFlex(0.68f).withMargin(0.f),
        juce::FlexItem(springsSection).withFlex(1.f).withMargin(0.f),
    });

    // fb.items.add(
    //      juce::FlexItem(springsGL).withFlex(1.f).withMargin(0.f));

    auto bounds = getLocalBounds();
    bounds.reduce(CustomLNF::margin, CustomLNF::margin);
    fb.performLayout(bounds);
}

void PluginEditor::paint(juce::Graphics &g)
{
    g.fillAll(findColour(backgroundColourId));
}
