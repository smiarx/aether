#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class PresetManager : juce::ValueTree::Listener
{
  public:
    static constexpr auto extension   = "preset";
    static constexpr auto defaultName = "default";

    using factoryPreset_t = std::tuple<const char *, const char *, size_t>;
    static constexpr auto nFactoryPreset = 1;
    static const std::array<factoryPreset_t, nFactoryPreset> factoryPresets;

    PresetManager(juce::AudioProcessorValueTreeState &apvts);
    ~PresetManager();

    // preset manager listener
    class Listener
    {
      public:
        virtual void presetManagerChanged(PresetManager &presetManager) = 0;
    };

    void addListener(Listener *listener)
    {
        if (listener != nullptr) {
            m_listeners.push_back(listener);
        }
    }
    void removeListener(Listener *listener)
    {
        if (listener != nullptr) {
            auto it =
                std::find(m_listeners.begin(), m_listeners.end(), listener);
            if (it != m_listeners.end()) {
                m_listeners.erase(it);
            }
        }
    }

    void loadPresetWithId(size_t id);
    void nextPreset();
    void prevPreset();

    void loadDefault();
    void loadFactoryPreset(size_t index);
    void loadPresetFromFile(const juce::File &file);
    void savePresetToFile(const juce::File &file);

    bool isPresetNotSaved() const { return m_presetNotSaved; }

    static size_t getNumPresets() { return nFactoryPreset + 1; }
    size_t getPresetId() const { return m_presetId; }
    juce::String getPresetName(size_t id) const;

    juce::ValueTree getCurrentPreset() const { return m_apvts.copyState(); }
    auto &getPresetName() const { return m_presetName; }

    auto &getFactoryPresets() const { return factoryPresets; }

  private:
    juce::AudioProcessorValueTreeState &m_apvts;
    bool m_presetNotSaved{false};
    size_t m_presetId{0};
    juce::String m_presetName{defaultName};

    const juce::ValueTree m_default;

    std::vector<Listener *> m_listeners;

    void loadPreset(const juce::String &name, const juce::ValueTree &preset);
    void callListeners();
    virtual void
    valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                             const juce::Identifier &property) override;
};
