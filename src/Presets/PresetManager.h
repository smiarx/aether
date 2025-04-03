#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class PresetManager
{
  public:
    static constexpr auto extension   = "preset";
    static constexpr auto defaultName = "default";

    using factoryPreset_t = std::tuple<const char *, const char *, size_t>;
    static constexpr auto nFactoryPreset = 1;
    static const std::array<factoryPreset_t, nFactoryPreset> factoryPresets;

    PresetManager(juce::AudioProcessorValueTreeState &apvts);

    void nextPreset();
    void prevPreset();

    void loadDefault();
    void loadFactoryPreset(size_t index);
    void loadPresetFromFile(const juce::File &file);
    void savePresetToFile(const juce::File &file);

    juce::ValueTree getCurrentPreset() const { return m_apvts.copyState(); }
    auto &getPresetName() const { return m_presetName; }

    auto &getFactoryPresets() const { return factoryPresets; }

  private:
    juce::AudioProcessorValueTreeState &m_apvts;
    size_t m_presetId{0};
    juce::String m_presetName{defaultName};

    const juce::ValueTree m_default;

    void loadPreset(const juce::String &name, const juce::ValueTree &preset);
};
