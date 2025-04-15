#pragma once

#include "../Presets/PresetManager.h"
#include "ArrowButton.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

class PresetComponent : public juce::Component,
                        public juce::Button::Listener,
                        public PresetManager::Listener
{
  public:
    PresetComponent(PresetManager &presetManager);
    ~PresetComponent() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    void setArrowsColour(juce::Colour colour)
    {
        m_prevButton.setColour(colour);
        m_nextButton.setColour(colour);
    }

  private:
    void updatePresetName();
    virtual void presetManagerChanged(PresetManager &presetManager) override;
    void buttonClicked(juce::Button *button) override;

    PresetManager &m_presetManager;

    ArrowButton m_prevButton;
    ArrowButton m_nextButton;
    juce::TextButton m_presetButton;

    std::unique_ptr<juce::FileChooser> m_fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetComponent)
};

} // namespace aether
