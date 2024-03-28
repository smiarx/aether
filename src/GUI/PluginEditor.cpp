#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p) :
    AudioProcessorEditor(&p), audioProcessor(p), springsSection{p.getAPVTS()},
    delaySection{p.getAPVTS()}
{
    setLookAndFeel(&lookandfeel);
    addAndMakeVisible(springsSection);
    addAndMakeVisible(delaySection);
    addAndMakeVisible(springsGL);
    addAndMakeVisible(resizableCorner);

    setSize(400, 400);
}

void PluginEditor::resized()
{
    resizableCorner.setBounds(getWidth() - 16, getHeight() - 16, 16, 16);

    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;

    juce::FlexBox fbSprings;
    fbSprings.flexDirection = juce::FlexBox::Direction::row;

    fbSprings.items.add(
        juce::FlexItem(springsSection).withFlex(1.f).withMargin(0.f));
    // fbSprings.items.add(
    //     juce::FlexItem(springsGL).withFlex(0.4f).withMargin(0.f));

    fb.items.add(juce::FlexItem(delaySection).withFlex(1.f).withMargin(0.f));
    fb.items.add(juce::FlexItem(fbSprings).withFlex(7.f).withMargin(0.f));

    fb.performLayout(getLocalBounds());
}

void PluginEditor::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::darkgrey);
}
