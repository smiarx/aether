#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include "../PluginProcessor.h"
#include "CustomLNF.h"
#include "DelaySection.h"
#include "PresetComponent.h"
#include "SpringsSection.h"
#include "Title.h"
#include "ToolTip.h"

namespace aether
{

class PluginEditor : public juce::AudioProcessorEditor
{
  public:
    static constexpr auto kTitleHeight  = 50.f;
    static constexpr auto kTitleMargin  = 5.f;
    static constexpr auto kHeaderHeight = kTitleHeight + 2.f;

    enum ColourIDs {
        kSeparator = 0x1312039,
    };

    PluginEditor(PluginProcessor &);
    ~PluginEditor() override;

    //===================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

    void mouseMove(const juce::MouseEvent &event) override;

  private:
    CustomLNF lookandfeel_;
    Title title_;
    ToolTip tooltip_;
    PresetComponent preset_;
    DelaySection delaySection_;
    SpringsSection springsSection_;

    juce::ComponentBoundsConstrainer constrainer_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};

} // namespace aether
