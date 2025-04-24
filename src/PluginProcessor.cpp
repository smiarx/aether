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
            "delay_drywet", "Dry/Wet",
            juce::NormalisableRange<float>{0.f, 100.f, 0.1f}, 20.f),
        std::make_unique<juce::AudioParameterChoice>(
            "delay_time_type", "Type", juce::StringArray{"seconds", "beats"},
            0),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_seconds", "Delay Seconds",
            juce::NormalisableRange<float>{
                0.01f, processors::TapeDelay::kMaxDelay, 0.001f, 0.5f},
            0.2f),
        std::make_unique<juce::AudioParameterChoice>(
            "delay_beats", "Delay Beats",
            juce::StringArray{"1/32", "1/16", "1/8", "1/4", "1/3", "1/2", "1",
                              "2"},
            kBeat1),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_feedback", "Feedback",
            juce::NormalisableRange{0.0f, 120.f, 0.1f}, 80.f),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_cutoff_low", "Cut Low",
            juce::NormalisableRange{100.f, 20000.f, 1.f, 0.5f}, 20000.f),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_cutoff_hi", "Cut High",
            juce::NormalisableRange{20.f, 3000.f, 1.f, 0.5f}, 20.f),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_saturation", "saturation", -40.f, 15.f, -40.f),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_drift", "Drift", juce::NormalisableRange{0.f, 100.f, 0.1f},
            0.f),
        std::make_unique<juce::AudioParameterChoice>(
            "delay_mode", "Mode",
            juce::StringArray{"Normal", "Back & Forth", "Reverse"}, 0)));

    layout.add(std::make_unique<juce::AudioProcessorParameterGroup>(
        "springs", "Reverb", "|",
        std::make_unique<juce::AudioParameterBool>("springs_active", "Active",
                                                   true),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_drywet", "Dry/Wet",
            juce::NormalisableRange<float>{0.f, 100.f, 0.1f}, 20.f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_width", "Width",
            juce::NormalisableRange<float>{0.f, 100.f, 0.1f}, 100.f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_length", "Length",
            juce::NormalisableRange<float>{0.02f, 0.2f, 0.001f, 0.6f}, 0.05f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_decay", "Decay",
            juce::NormalisableRange<float>{0.01f, 10.f, 0.001f, 0.6f}, 3.f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_damp", "Damp",
            juce::NormalisableRange<float>{200.f, 12000.f, 1.f, 0.5f}, 4500.f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_shape", "Shape",
            juce::NormalisableRange<float>{-5.f, 5.f, 0.01f, 0.3f, true}, 0.5f),
        std::make_unique<juce::AudioParameterFloat>("springs_tone", "Tone", 0.f,
                                                    1.f, 0.5f),
        std::make_unique<juce::AudioParameterFloat>(
            "springs_scatter", "Scatter", 0.f, 120.f, 0.1f),
        std::make_unique<juce::AudioParameterFloat>("springs_chaos", "Chaos",
                                                    0.f, 100.f, 0.1f)));
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
    return presetManager_.getPresetId();
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

//==============================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    auto fSampleRate = static_cast<float>(sampleRate);
    springs_.prepare(fSampleRate, samplesPerBlock);
    tapedelay_.prepare(fSampleRate, samplesPerBlock);

    ///* Set springgl uniform values */
    // SpringsGL::setUniforms(m_springs.rms.rms, &m_springs.rms.rms_id,
    //                        &m_springs.desc.length,
    //                        &m_springs.desc.ftr);
}

void PluginProcessor::releaseResources()
{
    springs_.free();
    tapedelay_.free();
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

void PluginProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                   juce::MidiBuffer &midiMessages)
{
    juce::ignoreUnused(midiMessages);

    int count = buffer.getNumSamples();

    ParamEvent event;
    while (paramEvents_.try_dequeue(event)) {
        switch (event.id) {
        case ParamId::kDelayActive:
            activeTapeDelay_ = event.value > 0.f;
            break;
        case ParamId::kDelayDrywet:
            tapedelay_.setDryWet(event.value / 100.f, count);
            break;
        case ParamId::kDelayTimeType:
            useBeats_ = event.value > 0.f;
            if (useBeats_) {
                auto *param = static_cast<juce::AudioParameterChoice *>(
                    getParameters()[static_cast<size_t>(ParamId::kDelayBeats)]);
                event.value = static_cast<float>(*param);
            } else {
                auto *param = static_cast<juce::AudioParameterFloat *>(
                    getParameters()[static_cast<size_t>(
                        ParamId::kDelaySeconds)]);
                event.value = *param;
            }
        case ParamId::kDelayBeats:
            if (useBeats_) {
                auto id = static_cast<int>(event.value);
                double mult;
                switch (id) {
                case kBeat132:
                    mult = 1.0 / 32.0;
                    break;
                case kBeat116:
                    mult = 1.0 / 16.0;
                    break;
                case kBeat18:
                    mult = 1.0 / 8.0;
                    break;
                case kBeat14:
                    mult = 1.0 / 4.0;
                    break;
                case kBeat13:
                    mult = 1.0 / 3.0;
                    break;
                case kBeat12:
                    mult = 1.0 / 2.0;
                    break;
                default:
                case kBeat1:
                    mult = 1.0;
                    break;
                case kBeat2:
                    mult = 2.0;
                    break;
                }
                beatsMult_ = mult;
                auto time  = static_cast<float>(60 * mult / bpm_);
                tapedelay_.setDelay(time, count);
                break;
            } else if (event.id == ParamId::kDelayBeats) {
                break;
            }
        case ParamId::kDelaySeconds:
            if (!useBeats_) {
                tapedelay_.setDelay(event.value, count);
            }
            break;
        case ParamId::kDelayFeedback:
            tapedelay_.setFeedback(event.value / 100.f, count);
            break;
        case ParamId::kDelayCutLow:
            tapedelay_.setCutLowPass(event.value, count);
            break;
        case ParamId::kDelayCutHi:
            tapedelay_.setCutHiPass(event.value, count);
            break;
        case ParamId::kDelaySaturation:
            tapedelay_.setSaturation(event.value, count);
            break;
        case ParamId::kDelayDrift:
            tapedelay_.setDrift(event.value / 100.f, count);
            break;
        case ParamId::kDelayMode:
            tapedelay_.setMode(
                static_cast<decltype(tapedelay_)::Mode>(event.value), count);
            break;
        case ParamId::kSpringsActive:
            activeSprings_ = event.value > 0;
            break;
        case ParamId::kSpringsDryWet:
            springs_.setDryWet(event.value / 100.f, count);
            break;
        case ParamId::kSpringsWidth:
            springs_.setWidth(event.value / 100.f, count);
            break;
        case ParamId::kSpringsLength:
            springs_.setTd(event.value, count);
            break;
        case ParamId::kSpringsDecay:
            springs_.setT60(event.value, count);
            break;
        case ParamId::kSpringsTone:
            springs_.setTone(event.value, count);
            break;
        case ParamId::kSpringsScatter:
            springs_.setScatter(event.value / 100.f, count);
            break;
        case ParamId::kSpringsDamp:
            springs_.setFreq(event.value, count);
            break;
        case ParamId::kSpringsChaos:
            springs_.setChaos(event.value / 100.f, count);
            break;
        case ParamId::kSpringsShape:
            springs_.setRes(event.value, count);
            break;
        default:
            break;
        }
    }

    if (useBeats_) {
        const auto position = getPlayHead()->getPosition();
        if (position.hasValue()) {
            auto bpm = position->getBpm();
            if (bpm.hasValue() && *bpm != bpm_) {
                bpm_      = *bpm;
                auto time = static_cast<float>(60.0 * beatsMult_ / bpm_);
                tapedelay_.setDelay(time, count);
            }

            if (tapedelay_.getMode() != processors::TapeDelay::Mode::kNormal) {
                if (position->getIsPlaying()) {
                    auto ppq = position->getPpqPosition();
                    if (ppq.hasValue()) {
                        if (!isPlaying_) {
                            isPlaying_ = true;
                            nextSync_ =
                                static_cast<double>(static_cast<int>(*ppq + 1));
                        } else {
                            if (nextSync_ > 0 && *ppq > nextSync_) {
                                // today, sample accurate sync
                                tapedelay_.setMode(tapedelay_.getMode(), count);
                                nextSync_ = -1;
                            }
                        }
                    }
                } else if (isPlaying_) {
                    isPlaying_ = false;
                }
            }
        }
    }

    // shake springs
    if (shake_.load()) {
        shake_.store(false);
        springs_.shake();
    }

    const float *const *ins = buffer.getArrayOfReadPointers();
    float *const *outs      = buffer.getArrayOfWritePointers();

    if (activeTapeDelay_) {
        tapedelay_.process(ins, outs, count);
        ins = outs;
    }
    if (activeSprings_) {
        springs_.process(ins, outs, count);
        ins = outs;
    }
    assert(ins == outs);

    // update rms buffer position
    rmsPos_.store(*springs_.getRMSStackPos());
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
    return new aether::PluginProcessor();
}
