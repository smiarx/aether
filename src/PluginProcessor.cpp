#include "PluginProcessor.h"
#include "GUI/PluginEditor.h"
#include "Presets/PresetManager.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_core/juce_core.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include <cassert>
#include <cstddef>
#include <memory>

namespace aether
{

//==============================================================================
PluginProcessor::PluginProcessor() :
    AudioProcessor(
        BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    parameters_(*this, nullptr, juce::Identifier(PROJECT_NAME), createLayout())
{
    for (auto *param : getParameters()) addProcessorAsListener(param);
}

PluginProcessor::~PluginProcessor()
{
    for (auto *param : getParameters()) {
        param->removeListener(this);
    }
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
PluginProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioProcessorParameterGroup>(
        "delay", "Delay", "|",
        std::make_unique<juce::AudioParameterBool>("delay_active", "Active",
                                                   true),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_drywet", "Delay Dry/Wet",
            juce::NormalisableRange<float>{0.f, 100.f, 0.1f}, 20.f),
        std::make_unique<juce::AudioParameterChoice>(
            "delay_time_type", "Delay Type",
            juce::StringArray{"seconds", "beats", "dotted"}, 0),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_seconds", "Delay Seconds",
            juce::NormalisableRange<float>{
                0.01f, processors::TapeDelay::kMaxDelay, 0.001f, 0.5f},
            0.2f),
        std::make_unique<juce::AudioParameterChoice>(
            "delay_beats", "Delay Beats",
            juce::StringArray{"1/32", "1/16", "1/8", "1/6", "1/4", "1/3", "1/2",
                              "1", "2", "4"},
            kBeat1),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_feedback", "Delay Feedback",
            juce::NormalisableRange{0.0f, 120.f, 0.1f}, 80.f),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_cutoff_low", "Delay Lowpass",
            juce::NormalisableRange{100.f, 20000.f, 1.f, 0.5f}, 20000.f),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_cutoff_hi", "Delay Highpass",
            juce::NormalisableRange{20.f, 3000.f, 1.f, 0.5f}, 20.f),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_saturation", "Delay Drive", -40.f, 15.f, -40.f),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_drift", "Delay Drift",
            juce::NormalisableRange{0.f, 100.f, 0.1f}, 0.f),
        std::make_unique<juce::AudioParameterChoice>(
            "delay_mode", "Delay Mode",
            juce::StringArray{"Normal", "Back & Forth", "Reverse"}, 0)));

    layout.add(std::make_unique<juce::AudioProcessorParameterGroup>(
        "springs", "Reverb", "|",
        std::make_unique<juce::AudioParameterBool>("springs_active",
                                                   "Reverb Active", true),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_drywet", "Reverb Dry/Wet",
            juce::NormalisableRange<float>{0.f, 100.f, 0.1f}, 20.f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_width", "Reverb Width",
            juce::NormalisableRange<float>{0.f, 100.f, 0.1f}, 100.f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_length", "Reverb Length",
            juce::NormalisableRange<float>{0.02f, 0.2f, 0.001f, 0.6f}, 0.05f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_decay", "Reverb Decay",
            juce::NormalisableRange<float>{0.3f, 10.f, 0.001f, 0.6f}, 3.f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_damp", "Reverb Damp",
            juce::NormalisableRange<float>{200.f, 12000.f, 1.f, 0.5f}, 4500.f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_shape", "Reverb Shape",
            juce::NormalisableRange<float>{-5.f, 5.f, 0.01f, 0.3f, true}, 0.5f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_tone", "Reverb Tone", 0.f, 1.f, 0.5f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_scatter", "Reverb Scatter",
            juce::NormalisableRange<float>{0.f, 120.f, 0.1f}, 50.f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_chaos", "Reverb Chaos",
            juce::NormalisableRange<float>{0.f, 100.f, 0.1f}, 25.f)));
    return layout;
}

//==============================================================================
juce::AudioProcessorEditor *PluginProcessor::createEditor()
{
    return new PluginEditor(*this);
}

//==============================================================================
const juce::String PluginProcessor::getName() const { return JucePlugin_Name; }

bool PluginProcessor::acceptsMidi() const { return false; }

bool PluginProcessor::producesMidi() const { return false; }

bool PluginProcessor::isMidiEffect() const { return false; }

double PluginProcessor::getTailLengthSeconds() const { return 0.0; }

int PluginProcessor::getNumPrograms()
{
    return aether::PresetManager::kNFactoryPreset + 1;
}

int PluginProcessor::getCurrentProgram()
{
    return static_cast<int>(presetManager_.getPresetId());
}

void PluginProcessor::setCurrentProgram(int index)
{
    presetManager_.loadPresetWithId(static_cast<size_t>(index));
}

const juce::String PluginProcessor::getProgramName(int index)
{
    return aether::PresetManager::getPresetName(static_cast<size_t>(index));
}

void PluginProcessor::changeProgramName(int index, const juce::String &newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void PluginProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    auto state = parameters_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(
        getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr)
        if (xmlState->hasTagName(parameters_.state.getType()))
            parameters_.replaceState(juce::ValueTree::fromXml(*xmlState));
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

//==============================================================================
void PluginProcessor::parameterValueChanged(int id, float newValue)
{
    auto *ptr = static_cast<juce::RangedAudioParameter *>(getParameters()[id]);
    float value = ptr->convertFrom0to1(newValue);

    paramEvents_.enqueue({id, value});
}

void PluginProcessor::addProcessorAsListener(
    juce::AudioProcessorParameter *param)
{
    jassert(param != nullptr);
    param->addListener(this);
    parameterValueChanged(param->getParameterIndex(), param->getValue());
}

} // namespace aether

//==============================================================================
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
#if DSP_X86_DISPATCH
    auto infos = dsp::cpu::getInfos();

    if (infos.avx2 && infos.fma3_sse42) {
        return aether::loadPluginAVX2();
    }
#endif

    return aether::loadPluginDefault();
}
