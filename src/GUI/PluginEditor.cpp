#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p) :
    AudioProcessorEditor(&p), audioProcessor(p), springsSection{p.getAPVTS()}
{
    setLookAndFeel(&lookandfeel);
    addAndMakeVisible(springsSection);
    addAndMakeVisible(springsGL);
    addAndMakeVisible(resizableCorner);

    setSize(400, 400);
}

void PluginEditor::resized()
{
    resizableCorner.setBounds(getWidth() - 16, getHeight() - 16, 16, 16);

    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;

    fb.items.add(juce::FlexItem(springsSection).withFlex(1.f).withMargin(0.f));
    fb.items.add(juce::FlexItem(springsGL).withFlex(0.4f).withMargin(0.f));

    fb.performLayout(getLocalBounds());

    /* remove header */
    auto area = springsGL.getBounds();
    area.removeFromTop(SpringsSection::headerHeight);
    springsGL.setBounds(area);
}

void PluginEditor::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::crimson);
}
