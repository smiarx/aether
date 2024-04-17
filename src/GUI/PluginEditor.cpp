#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p) :
    AudioProcessorEditor(&p), audioProcessor(p), springsSection{p.getAPVTS()},
    delaySection{p.getAPVTS()}
{
    setLookAndFeel(&lookandfeel);
    addAndMakeVisible(springsSection);
    addAndMakeVisible(springsSection.macros);
    addAndMakeVisible(delaySection);
    addAndMakeVisible(springsGL);
    addAndMakeVisible(resizableCorner);

    setSize(700, 600);
}

void PluginEditor::resized()
{
    resizableCorner.setBounds(getWidth() - 16, getHeight() - 16, 16, 16);

    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;

    juce::FlexBox fbMain;
    fbMain.flexDirection = juce::FlexBox::Direction::row;

    fbMain.items.addArray({
        juce::FlexItem(delaySection).withFlex(0.68f).withMargin(0.f),
        juce::FlexItem(springsSection.macros).withFlex(1.f).withMargin(0.f),
    });

    fb.items.addArray({
        juce::FlexItem(fbMain).withFlex(1.f),
        juce::FlexItem(springsSection).withFlex(0.75f).withMargin(0.f),
    });

    // fb.items.add(
    //      juce::FlexItem(springsGL).withFlex(1.f).withMargin(0.f));

    fb.performLayout(getLocalBounds());
}

void PluginEditor::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::darkgrey);
}
