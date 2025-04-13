#include "PresetComponent.h"
#include <memory.h>

namespace aether
{

PresetComponent::PresetComponent(PresetManager &presetManager) :
    m_presetManager(presetManager), m_prevButton("presetPrev", 0.5f),
    m_nextButton("presetNext", 0.f)
{
    updatePresetName();

    m_presetManager.addListener(this);
    m_prevButton.addListener(this);
    m_nextButton.addListener(this);
    m_presetButton.addListener(this);

    addAndMakeVisible(m_prevButton);
    addAndMakeVisible(m_nextButton);
    addAndMakeVisible(m_presetButton);
}

PresetComponent::~PresetComponent()
{
    m_presetManager.removeListener(this);
    m_prevButton.removeListener(this);
    m_nextButton.removeListener(this);
    m_presetButton.removeListener(this);
}

void PresetComponent::paint(juce::Graphics &g)
{
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PresetComponent::resized()
{
    auto area = getLocalBounds();
    m_prevButton.setBounds(area.removeFromLeft(area.getHeight()));
    m_nextButton.setBounds(area.removeFromRight(area.getHeight()));
    m_presetButton.setBounds(area);
}

void PresetComponent::buttonClicked(juce::Button *button)
{
    if (button == &m_prevButton) {
        // Load previous factory preset
        m_presetManager.prevPreset();
        m_presetButton.setButtonText(m_presetManager.getPresetName());
    } else if (button == &m_nextButton) {
        // Load next factory preset
        m_presetManager.nextPreset();
        m_presetButton.setButtonText(m_presetManager.getPresetName());
    } else if (button == &m_presetButton) {
        juce::PopupMenu popupMenu;

        constexpr auto defaultId = 1;
        popupMenu.addItem(defaultId, PresetManager::defaultName);

        int index = defaultId;
        for (const auto &data : m_presetManager.getFactoryPresets()) {
            popupMenu.addItem(++index, std::get<0>(data));
        }
        popupMenu.addSeparator();

        constexpr auto saveId = 100;
        constexpr auto loadId = 101;
        popupMenu.addItem(saveId, "Save Preset to File...");
        popupMenu.addItem(loadId, "Load Preset from File...");

        popupMenu.showMenuAsync(
            juce::PopupMenu::Options().withTargetComponent(&m_presetButton),
            [this](int result) {
                if (result == defaultId) {
                    m_presetManager.loadDefault();
                } else if (result > defaultId &&
                           result <=
                               PresetManager::nFactoryPreset + defaultId) {
                    m_presetManager.loadFactoryPreset(
                        static_cast<size_t>(result - defaultId - 1));
                } else if (result == saveId) {
                    constexpr auto fileBrowserFlags =
                        juce::FileBrowserComponent::saveMode |
                        juce::FileBrowserComponent::canSelectFiles;
                    m_fileChooser = std::make_unique<juce::FileChooser>(
                        juce::String("Save Preset"), juce::String(),
                        juce::String("*." +
                                     juce::String(PresetManager::extension)));
                    m_fileChooser->launchAsync(
                        fileBrowserFlags, [this](const juce::FileChooser &fc) {
                            auto file = fc.getResult();
                            m_presetManager.savePresetToFile(file);
                        });
                } else if (result == loadId) {
                    constexpr auto fileBrowserFlags =
                        juce::FileBrowserComponent::openMode |
                        juce::FileBrowserComponent::canSelectFiles;
                    m_fileChooser = std::make_unique<juce::FileChooser>(
                        juce::String("Load Preset"), juce::String(),
                        juce::String("*." +
                                     juce::String(PresetManager::extension)));
                    m_fileChooser->launchAsync(
                        fileBrowserFlags, [this](const juce::FileChooser &fc) {
                            auto file = fc.getResult();
                            if (file.exists()) {
                                m_presetManager.loadPresetFromFile(file);
                            }
                        });
                }
            });
    }
}

void PresetComponent::updatePresetName()
{
    auto name = m_presetManager.getPresetName();
    if (m_presetManager.isPresetNotSaved()) {
        name += "*";
    }
    m_presetButton.setButtonText(name);
}

void PresetComponent::presetManagerChanged(PresetManager & /*presetManager*/)
{
    updatePresetName();
}

} // namespace aether
