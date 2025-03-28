#pragma once

#include "../PluginProcessor.h"
#include "CustomLNF.h"
#include "DelaySection.h"
#include "SpringsSection.h"
#include "ToolTip.h"
#include <juce_audio_processors/juce_audio_processors.h>

class PluginEditor : public juce::AudioProcessorEditor
{
  public:
    static constexpr auto titleHeight  = 38.f;
    static constexpr auto titleMargin  = 5.f;
    static constexpr auto headerHeight = titleHeight + 2.f;

    enum ColourIDs {
        backgroundColourId = 0x1610200,
        Separator          = 0x1312039,
    };

    PluginEditor(PluginProcessor &);
    ~PluginEditor() override { setLookAndFeel(nullptr); }

    //===================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

    virtual void mouseMove(const juce::MouseEvent &event) override;

  private:
    CustomLNF lookandfeel;
    juce::Label title;
    ToolTip tooltip;
    DelaySection delaySection;
    SpringsSection springsSection;

    juce::ComponentBoundsConstrainer constrainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
