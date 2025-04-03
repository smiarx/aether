#include "PresetManager.h"
#include "BinaryData.h"

const std::array<PresetManager::factoryPreset_t, PresetManager::nFactoryPreset>
    PresetManager::factoryPresets = {
        {{"test", BinaryData::test_preset, BinaryData::test_presetSize}}};

PresetManager::PresetManager(juce::AudioProcessorValueTreeState &apvts) :
    m_apvts(apvts), m_default(apvts.copyState())
{
    m_apvts.state.addListener(this);
}

PresetManager::~PresetManager() { m_apvts.state.removeListener(this); }

void PresetManager::valueTreePropertyChanged(
    juce::ValueTree & /*treeWhosePropertyHasChanged*/,
    const juce::Identifier & /*property*/)
{
    if (!m_presetNotSaved) {
        m_presetNotSaved = true;
        callListeners();
    }
}

void PresetManager::nextPreset()
{
    constexpr auto nPresets = factoryPresets.size() + 1;
    auto presetId           = (m_presetId + 1) % nPresets;
    if (presetId == 0) {
        loadDefault();
    }
    loadFactoryPreset(presetId - 1);
}

void PresetManager::prevPreset()
{
    constexpr auto nPresets = factoryPresets.size() + 1;
    auto presetId           = (m_presetId - 1 + nPresets) % nPresets;
    if (presetId == 0) {
        loadDefault();
    }
    loadFactoryPreset(presetId - 1);
}

void PresetManager::loadDefault()
{
    m_presetId = 0;
    loadPreset(defaultName, m_default);
}

void PresetManager::loadFactoryPreset(size_t index)
{
    if (index < factoryPresets.size()) {
        auto [name, data, size] = factoryPresets[index];
        juce::String xmlString(data, size);
        std::unique_ptr<juce::XmlElement> xml(
            juce::XmlDocument::parse(xmlString));
        if (xml != nullptr) {
            juce::ValueTree preset = juce::ValueTree::fromXml(*xml);
            if (preset.isValid()) {
                m_presetId = index + 1;
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
            loadPreset(file.getFileNameWithoutExtension(), std::move(preset));
        }
    }
}

void PresetManager::savePresetToFile(const juce::File &file)
{
    std::unique_ptr<juce::XmlElement> xml(getCurrentPreset().createXml());
    if (xml != nullptr) {
        xml->writeTo(file, {});
        m_presetName     = file.getFileNameWithoutExtension();
        m_presetNotSaved = false;
        callListeners();
    }
}

void PresetManager::loadPreset(const juce::String &name,
                               const juce::ValueTree &preset)
{
    m_apvts.state.copyPropertiesAndChildrenFrom(preset, nullptr);
    m_presetName     = name;
    m_presetNotSaved = false;
    callListeners();
}

void PresetManager::callListeners()
{
    for (auto *listener : m_listeners) {
        listener->presetManagerChanged(*this);
    }
}
