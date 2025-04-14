#include "PluginProcessor.h"
#include "GUI/PluginEditor.h"

namespace aether
{

//==============================================================================
PluginProcessor::PluginProcessor() :
    AudioProcessor(
        BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    m_parameters(*this, nullptr, juce::Identifier(PROJECT_NAME), createLayout())
{
    for (auto *param : getParameters()) addProcessorAsListener(param);
}

PluginProcessor::~PluginProcessor() {}

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
            "delay_seconds", "Delay",
            juce::NormalisableRange<float>{0.01f, 2.f, 0.001f, 0.5f}, 0.12f),
        std::make_unique<juce::AudioParameterChoice>(
            "delay_beats", "Delay",
            juce::StringArray{"1/32", "1/16", "1/8", "1/4", "1/3", "1/2", "1",
                              "2"},
            Beat1),
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
    return m_presetManager.nFactoryPreset + 1;
}

int PluginProcessor::getCurrentProgram()
{
    return m_presetManager.getPresetId();
}

void PluginProcessor::setCurrentProgram(int index)
{
    m_presetManager.loadPresetWithId(static_cast<size_t>(index));
}

const juce::String PluginProcessor::getProgramName(int index)
{
    return m_presetManager.getPresetName(static_cast<size_t>(index));
}

void PluginProcessor::changeProgramName(int index, const juce::String &newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void PluginProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    auto state = m_parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(
        getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(m_parameters.state.getType()))
            m_parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    auto fSampleRate = static_cast<float>(sampleRate);
    m_springs.prepare(fSampleRate, samplesPerBlock);
    m_tapedelay.prepare(fSampleRate, samplesPerBlock);

    ///* Set springgl uniform values */
    // SpringsGL::setUniforms(m_springs.rms.rms, &m_springs.rms.rms_id,
    //                        &m_springs.desc.length,
    //                        &m_springs.desc.ftr);
}

void PluginProcessor::releaseResources()
{
    m_springs.free();
    m_tapedelay.free();
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
    while (m_paramEvents.try_dequeue(event)) {
        switch (event.id) {
        case ParamId::DelayActive:
            m_activeTapeDelay = event.value > 0.f;
            break;
        case ParamId::DelayDrywet:
            m_tapedelay.setDryWet(event.value / 100.f, count);
            break;
        case ParamId::DelayTimeType:
            m_useBeats = event.value > 0.f;
            if (m_useBeats) {
                auto *param = static_cast<juce::AudioParameterChoice *>(
                    getParameters()[static_cast<size_t>(ParamId::DelayBeats)]);
                event.value = static_cast<float>(*param);
            } else {
                auto *param = static_cast<juce::AudioParameterFloat *>(
                    getParameters()[static_cast<size_t>(
                        ParamId::DelaySeconds)]);
                event.value = *param;
            }
        case ParamId::DelayBeats:
            if (m_useBeats) {
                auto id = static_cast<int>(event.value);
                double mult;
                switch (id) {
                case Beat1_32:
                    mult = 1.0 / 32.0;
                    break;
                case Beat1_16:
                    mult = 1.0 / 16.0;
                    break;
                case Beat1_8:
                    mult = 1.0 / 8.0;
                    break;
                case Beat1_4:
                    mult = 1.0 / 4.0;
                    break;
                case Beat1_3:
                    mult = 1.0 / 3.0;
                    break;
                case Beat1_2:
                    mult = 1.0 / 2.0;
                    break;
                default:
                case Beat1:
                    mult = 1.0;
                    break;
                case Beat2:
                    mult = 2.0;
                    break;
                }
                m_beatsMult = mult;
                auto time   = static_cast<float>(60 * mult / m_bpm);
                m_tapedelay.setDelay(time, count);
                break;
            } else if (event.id == ParamId::DelayBeats) {
                break;
            }
        case ParamId::DelaySeconds:
            if (!m_useBeats) {
                m_tapedelay.setDelay(event.value, count);
            }
            break;
        case ParamId::DelayFeedback:
            m_tapedelay.setFeedback(event.value / 100.f, count);
            break;
        case ParamId::DelayCutLow:
            m_tapedelay.setCutLowPass(event.value, count);
            break;
        case ParamId::DelayCutHi:
            m_tapedelay.setCutHiPass(event.value, count);
            break;
        case ParamId::DelaySaturation:
            m_tapedelay.setSaturation(event.value, count);
            break;
        case ParamId::DelayDrift:
            m_tapedelay.setDrift(event.value / 100.f, count);
            break;
        case ParamId::DelayMode:
            m_tapedelay.setMode(
                static_cast<decltype(m_tapedelay)::Mode>(event.value), count);
            break;
        case ParamId::SpringsActive:
            m_activeSprings = event.value > 0;
            break;
        case ParamId::SpringsDryWet:
            m_springs.setDryWet(event.value / 100.f, count);
            break;
        case ParamId::SpringsWidth:
            m_springs.setWidth(event.value / 100.f, count);
            break;
        case ParamId::SpringsLength:
            m_springs.setTd(event.value, count);
            break;
        case ParamId::SpringsDecay:
            m_springs.setT60(event.value, count);
            break;
        case ParamId::SpringsTone:
            m_springs.setTone(event.value, count);
            break;
        case ParamId::SpringsScatter:
            m_springs.setScatter(event.value / 100.f, count);
            break;
        case ParamId::SpringsDamp:
            m_springs.setFreq(event.value, count);
            break;
        case ParamId::SpringsChaos:
            m_springs.setChaos(event.value / 100.f, count);
            break;
        case ParamId::SpringsShape:
            m_springs.setRes(event.value, count);
            break;
        default:
            break;
        }
    }

    if (m_useBeats) {
        const auto position = getPlayHead()->getPosition();
        if (position.hasValue()) {
            auto bpm = position->getBpm();
            if (bpm.hasValue() && *bpm != m_bpm) {
                m_bpm     = *bpm;
                auto time = static_cast<float>(60.0 * m_beatsMult / m_bpm);
                m_tapedelay.setDelay(time, count);
            }

            if (m_tapedelay.getMode() != processors::TapeDelay::Mode::Normal) {
                if (position->getIsPlaying()) {
                    auto ppq = position->getPpqPosition();
                    if (ppq.hasValue()) {
                        if (!m_isPlaying) {
                            m_isPlaying = true;
                            m_nextSync =
                                static_cast<double>(static_cast<int>(*ppq + 1));
                        } else {
                            if (m_nextSync > 0 && *ppq > m_nextSync) {
                                // today, sample accurate sync
                                m_tapedelay.setMode(m_tapedelay.getMode(),
                                                    count);
                                m_nextSync = -1;
                            }
                        }
                    }
                } else if (m_isPlaying) {
                    m_isPlaying = false;
                }
            }
        }
    }

    const float *const *ins = buffer.getArrayOfReadPointers();
    float *const *outs      = buffer.getArrayOfWritePointers();

    if (m_activeTapeDelay) {
        m_tapedelay.process(ins, outs, count);
        ins = outs;
    }
    if (m_activeSprings) {
        m_springs.process(ins, outs, count);
        ins = outs;
    }
    assert(ins == outs);

    // update rms buffer position
    m_rmsPos.store(*m_springs.getRMSStackPos());
}

//==============================================================================
void PluginProcessor::parameterValueChanged(int id, float newValue)
{
    auto *ptr = static_cast<juce::RangedAudioParameter *>(getParameters()[id]);
    float value = ptr->convertFrom0to1(newValue);

    m_paramEvents.enqueue({id, value});
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
