#include "PresetComponent.h"
#include "../Presets/PresetManager.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <cstddef>
#include <memory>

namespace aether
{

PresetComponent::PresetComponent(PresetManager &presetManager) :
    presetManager_(presetManager), prevButton_("presetPrev", 0.5f),
    nextButton_("presetNext", 0.f)
{
    updatePresetName();

    presetManager_.addListener(this);
    prevButton_.addListener(this);
    nextButton_.addListener(this);
    presetButton_.addListener(this);

    addAndMakeVisible(prevButton_);
    addAndMakeVisible(nextButton_);
    addAndMakeVisible(presetButton_);

    presetButton_.setTitle("Preset");
    presetButton_.setTooltip("Edit, load and save presets");
}

PresetComponent::~PresetComponent()
{
    presetManager_.removeListener(this);
    prevButton_.removeListener(this);
    nextButton_.removeListener(this);
    presetButton_.removeListener(this);
}

void PresetComponent::paint(juce::Graphics &g)
{
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PresetComponent::resized()
{
    auto area        = getLocalBounds().toFloat();
    auto buttonWidth = area.getWidth() * 0.1f;
    prevButton_.setBounds(area.removeFromLeft(buttonWidth)
                              .withSizeKeepingCentre(buttonWidth, buttonWidth)
                              .toNearestInt());
    nextButton_.setBounds(area.removeFromRight(buttonWidth)
                              .withSizeKeepingCentre(buttonWidth, buttonWidth)
                              .toNearestInt());
    area.reduce(3, 0);
    presetButton_.setBounds(area.toNearestInt());
}

void PresetComponent::buttonClicked(juce::Button *button)
{
    if (button == &prevButton_) {
        // Load previous factory preset
        presetManager_.prevPreset();
        presetButton_.setButtonText(presetManager_.getPresetName());
    } else if (button == &nextButton_) {
        // Load next factory preset
        presetManager_.nextPreset();
        presetButton_.setButtonText(presetManager_.getPresetName());
    } else if (button == &presetButton_) {
        juce::PopupMenu popupMenu;

        constexpr auto kDefaultId = 1;
        popupMenu.addItem(kDefaultId, PresetManager::kDefaultName);

        int index = kDefaultId;
        for (const auto &data : presetManager_.getFactoryPresets()) {
            popupMenu.addItem(++index, std::get<0>(data));
        }
        popupMenu.addSeparator();
        popupMenu.setLookAndFeel(&getLookAndFeel());

        constexpr auto kSaveId = 100;
        constexpr auto kLoadId = 101;
        popupMenu.addItem(kSaveId, "Save Preset to File...");
        popupMenu.addItem(kLoadId, "Load Preset from File...");

        popupMenu.showMenuAsync(
            juce::PopupMenu::Options().withTargetComponent(&presetButton_),
            [this](int result) {
                if (result == kDefaultId) {
                    presetManager_.loadDefault();
                } else if (result > kDefaultId &&
                           result <=
                               PresetManager::kNFactoryPreset + kDefaultId) {
                    presetManager_.loadFactoryPreset(
                        static_cast<size_t>(result - kDefaultId - 1));
                } else if (result == kSaveId) {
                    constexpr auto kFileBrowserFlags =
                        juce::FileBrowserComponent::saveMode |
                        juce::FileBrowserComponent::canSelectFiles;
                    fileChooser_ = std::make_unique<juce::FileChooser>(
                        juce::String("Save Preset"), juce::String(),
                        juce::String("*." +
                                     juce::String(PresetManager::kExtension)));
                    fileChooser_->launchAsync(
                        kFileBrowserFlags, [this](const juce::FileChooser &fc) {
                            auto file = fc.getResult();
                            presetManager_.savePresetToFile(file);
                        });
                } else if (result == kLoadId) {
                    constexpr auto kFileBrowserFlags =
                        juce::FileBrowserComponent::openMode |
                        juce::FileBrowserComponent::canSelectFiles;
                    fileChooser_ = std::make_unique<juce::FileChooser>(
                        juce::String("Load Preset"), juce::String(),
                        juce::String("*." +
                                     juce::String(PresetManager::kExtension)));
                    fileChooser_->launchAsync(
                        kFileBrowserFlags, [this](const juce::FileChooser &fc) {
                            auto file = fc.getResult();
                            if (file.exists()) {
                                presetManager_.loadPresetFromFile(file);
                            }
                        });
                }
            });
    }
}

void PresetComponent::updatePresetName()
{
    auto name = presetManager_.getPresetName();
    if (presetManager_.isPresetNotSaved()) {
        name += "*";
    }
    presetButton_.setButtonText(name);
}

void PresetComponent::presetManagerChanged(PresetManager & /*presetManager*/)
{
    updatePresetName();
}

} // namespace aether
