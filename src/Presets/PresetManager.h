#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace aether
{

class PresetManager : juce::ValueTree::Listener
{
  public:
    static constexpr auto kExtension   = "preset";
    static constexpr auto kDefaultName = "default";

    using factoryPreset_t = std::tuple<const char *, const char *, size_t>;
    static constexpr auto kNFactoryPreset = 1;
    static const std::array<factoryPreset_t, kNFactoryPreset> kFactoryPresets;

    PresetManager(juce::AudioProcessorValueTreeState &apvts);
    ~PresetManager() override;

    // preset manager listener
    class Listener
    {
      public:
        virtual void presetManagerChanged(PresetManager &presetManager) = 0;
    };

    void addListener(Listener *listener)
    {
        if (listener != nullptr) {
            listeners_.push_back(listener);
        }
    }
    void removeListener(Listener *listener)
    {
        if (listener != nullptr) {
            auto it = std::find(listeners_.begin(), listeners_.end(), listener);
            if (it != listeners_.end()) {
                listeners_.erase(it);
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

    [[nodiscard]] bool isPresetNotSaved() const { return presetNotSaved_; }

    static size_t getNumPresets() { return kNFactoryPreset + 1; }
    [[nodiscard]] size_t getPresetId() const { return presetId_; }
    [[nodiscard]] static juce::String getPresetName(size_t id);

    [[nodiscard]] juce::ValueTree getCurrentPreset() const
    {
        return apvts_.copyState();
    }
    [[nodiscard]] auto &getPresetName() const { return presetName_; }

    [[nodiscard]] static auto &getFactoryPresets() { return kFactoryPresets; }

  private:
    juce::AudioProcessorValueTreeState &apvts_;
    bool presetNotSaved_{false};
    size_t presetId_{0};
    juce::String presetName_{kDefaultName};

    const juce::ValueTree default_;

    std::vector<Listener *> listeners_;

    void loadPreset(const juce::String &name, const juce::ValueTree &preset);
    void callListeners();
    void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                  const juce::Identifier &property) override;
};

} // namespace aether
