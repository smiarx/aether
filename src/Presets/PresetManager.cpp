#include "PresetManager.h"
#include "Factory.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_core/juce_core.h"
#include <array>
#include <cstddef>

namespace aether
{

const std::array<PresetManager::factoryPreset_t, PresetManager::kNFactoryPreset>
    PresetManager::kFactoryPresets = {
        {{"test", Factory::test_preset, Factory::test_presetSize}}};

PresetManager::PresetManager(juce::AudioProcessorValueTreeState &apvts) :
    apvts_(apvts), default_(apvts.copyState())
{
    apvts_.state.addListener(this);
}

PresetManager::~PresetManager() { apvts_.state.removeListener(this); }

void PresetManager::valueTreePropertyChanged(
    juce::ValueTree & /*treeWhosePropertyHasChanged*/,
    const juce::Identifier & /*property*/)
{
    if (!presetNotSaved_) {
        presetNotSaved_ = true;
        callListeners();
    }
}

void PresetManager::loadPresetWithId(size_t id)
{
    if (id == 0) {
        loadDefault();
    }
    loadFactoryPreset(id - 1);
}

void PresetManager::nextPreset()
{
    constexpr auto kNPresets = kFactoryPresets.size() + 1;
    auto presetId            = (presetId_ + 1) % kNPresets;
    loadPresetWithId(presetId);
}

void PresetManager::prevPreset()
{
    constexpr auto kNPresets = kFactoryPresets.size() + 1;
    auto presetId            = (presetId_ - 1 + kNPresets) % kNPresets;
    loadPresetWithId(presetId);
}

void PresetManager::loadDefault()
{
    presetId_ = 0;
    loadPreset(kDefaultName, default_);
}

void PresetManager::loadFactoryPreset(size_t index)
{
    if (index < kFactoryPresets.size()) {
        auto [name, data, size] = kFactoryPresets[index];
        juce::String xmlString(data, size);
        std::unique_ptr<juce::XmlElement> xml(
            juce::XmlDocument::parse(xmlString));
        if (xml != nullptr) {
            juce::ValueTree preset = juce::ValueTree::fromXml(*xml);
            if (preset.isValid()) {
                presetId_ = index + 1;
                loadPreset(name, preset);
            }
        }
    }
}

void PresetManager::loadPresetFromFile(const juce::File &file)
{
    std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(file));
    if (xml != nullptr) {
        juce::ValueTree preset = juce::ValueTree::fromXml(*xml);
        if (preset.isValid()) {
            loadPreset(file.getFileNameWithoutExtension(), preset);
        }
    }
}

void PresetManager::savePresetToFile(const juce::File &file)
{
    std::unique_ptr<juce::XmlElement> xml(getCurrentPreset().createXml());
    if (xml != nullptr) {
        xml->writeTo(file, {});
        presetName_     = file.getFileNameWithoutExtension();
        presetNotSaved_ = false;
        callListeners();
    }
}

void PresetManager::loadPreset(const juce::String &name,
                               const juce::ValueTree &preset)
{
    apvts_.state.copyPropertiesAndChildrenFrom(preset, nullptr);
    presetName_     = name;
    presetNotSaved_ = false;
    callListeners();
}

void PresetManager::callListeners()
{
    for (auto *listener : listeners_) {
        listener->presetManagerChanged(*this);
    }
}

juce::String PresetManager::getPresetName(size_t id)
{
    if (id == 0) {
        return kDefaultName;
    }

    id -= 1;
    if (id < kNFactoryPreset) {
        return std::get<0>(kFactoryPresets[id]);
    }
    return "";
}

} // namespace aether
