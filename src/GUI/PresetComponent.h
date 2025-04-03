#pragma once

#include "../Presets/PresetManager.h"
#include <juce_gui_basics/juce_gui_basics.h>

class PresetComponent : public juce::Component, public juce::Button::Listener
{
  public:
    PresetComponent(PresetManager &presetManager);
    ~PresetComponent() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

  private:
    void buttonClicked(juce::Button *button) override;

    PresetManager &m_presetManager;

    juce::ArrowButton m_prevButton;
    juce::ArrowButton m_nextButton;
    juce::TextButton m_presetButton;

    std::unique_ptr<juce::FileChooser> m_fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetComponent)
};
