#pragma once

#include "../PluginProcessor.h"
#include "CustomLNF.h"
#include "DelaySection.h"
#include "SpringsGL.h"
#include "SpringsSection.h"
#include <juce_audio_processors/juce_audio_processors.h>

class PluginEditor : public juce::AudioProcessorEditor
{
  public:
    enum ColourIDs {
        backgroundColourId = 0x1610200,
        Separator          = 0x1312039,
    };

    PluginEditor(PluginProcessor &);
    ~PluginEditor() { setLookAndFeel(nullptr); };

    //===================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

  private:
    PluginProcessor &audioProcessor;

    CustomLNF lookandfeel;
    SpringsSection springsSection;
    DelaySection delaySection;
    SpringsGL springsGL;

    juce::ComponentBoundsConstrainer constrainer;
    juce::ResizableCornerComponent resizableCorner{this, &constrainer};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
