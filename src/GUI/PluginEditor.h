#pragma once

#include "../PluginProcessor.h"
#include "SpringsGL.h"
#include "SpringsSection.h"
#include <juce_audio_processors/juce_audio_processors.h>

class PluginEditor : public juce::AudioProcessorEditor
{
  public:
    PluginEditor(PluginProcessor &);
    ~PluginEditor(){};

    //===================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

  private:
    PluginProcessor &audioProcessor;

    SpringsSection springsSection;
    SpringsGL springsGL;

    juce::ComponentBoundsConstrainer constrainer;
    juce::ResizableCornerComponent resizableCorner{this, &constrainer};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
