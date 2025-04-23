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
        prevButton_.setColour(colour);
        nextButton_.setColour(colour);
    }

  private:
    void updatePresetName();
    void presetManagerChanged(PresetManager &presetManager) override;
    void buttonClicked(juce::Button *button) override;

    PresetManager &presetManager_;

    ArrowButton prevButton_;
    ArrowButton nextButton_;
    juce::TextButton presetButton_;

    std::unique_ptr<juce::FileChooser> fileChooser_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetComponent)
};

} // namespace aether
