#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p) :
    AudioProcessorEditor(&p), audioProcessor(p), springsSection{p.getAPVTS()}
{
    addAndMakeVisible(springsSection);
    addAndMakeVisible(resizableCorner);

    setSize(400, 400);
}

void PluginEditor::resized()
{
    resizableCorner.setBounds(getWidth() - 16, getHeight() - 16, 16, 16);

    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;

    fb.items.add(juce::FlexItem(springsSection).withFlex(1.f).withMargin(0.f));

    fb.performLayout(getLocalBounds());
}

void PluginEditor::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::crimson);
}
