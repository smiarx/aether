#include "PluginProcessor.h"
#include "GUI/PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor() :
    AudioProcessor(
        BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    m_parameters(*this, nullptr, juce::Identifier("Æther"), createLayout())
{
    for (auto *param : getParameters()) addProcessorAsListener(param);

    SpringsGL::setRMS(m_springs.getRMSStack(), m_springs.getRMSStackPos());
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
        std::make_unique<juce::AudioParameterFloat>(
            "delay_time", "Delay",
            juce::NormalisableRange<float>{0.01f, 2.f, 0.001f, 0.5f}, 0.12f),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_feedback", "Feedback",
            juce::NormalisableRange{0.0f, 120.f, 0.01f}, 80.f),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_cutoff_low", "Cut Low",
            juce::NormalisableRange{20.f, 20000.f, 0.1f, 0.34f}, 20000.f),
        std::make_unique<juce::AudioParameterFloat>(
            "delay_cutoff_hi", "Cut High",
            juce::NormalisableRange{20.f, 20000.f, 0.1f, 0.34f}, 20.f),
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
        std::make_unique<juce::AudioParameterFloat>("springs_diff", "Diffusion",
                                                    0.f, 1.f, 0.01f),
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

int PluginProcessor::getNumPrograms() { return 1; }

int PluginProcessor::getCurrentProgram() { return 0; }

void PluginProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String PluginProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
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
    m_springs.setSampleRate(sampleRate);
    m_springs.setBlockSize(samplesPerBlock);
    m_tapedelay.setSampleRate(sampleRate);
    m_tapedelay.setBlockSize(samplesPerBlock);

    ///* Set springgl uniform values */
    // SpringsGL::setUniforms(m_springs.rms.rms, &m_springs.rms.rms_id,
    //                        &m_springs.desc.length,
    //                        &m_springs.desc.ftr);
}

void PluginProcessor::releaseResources() {}

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

    while (!m_paramEvents.empty()) {
        auto event = m_paramEvents.front();
        m_paramEvents.pop_front();
        switch (event.id) {
        case ParamId::DelayActive:
            m_activeTapeDelay = event.value > 0.f;
            break;
        case ParamId::DelayDrywet:
            m_tapedelay.setDryWet(event.value / 100.f);
            break;
        case ParamId::DelayTime:
            m_tapedelay.setDelay(event.value);
            break;
        case ParamId::DelayFeedback:
            m_tapedelay.setFeedback(event.value / 100.f);
            break;
        case ParamId::DelayCutLow:
            m_tapedelay.setCutLowPass(event.value);
            break;
        case ParamId::DelayCutHi:
            m_tapedelay.setCutHiPass(event.value);
            break;
        case ParamId::DelaySaturation:
            m_tapedelay.setSaturation(event.value);
            break;
        case ParamId::DelayDrift:
            m_tapedelay.setDrift(event.value / 100.f);
            break;
        case ParamId::DelayMode:
            m_tapedelay.setMode(
                static_cast<decltype(m_tapedelay)::Mode>(event.value));
            break;
        case ParamId::SpringsActive:
            m_activeSprings = event.value > 0;
            break;
        case ParamId::SpringsDryWet:
            m_springs.setDryWet(event.value / 100.f);
            break;
        case ParamId::SpringsWidth:
            m_springs.setWidth(event.value / 100.f);
            break;
        case ParamId::SpringsLength:
            m_springs.setTd(event.value, m_springs.getChaos());
            SpringsGL::setLength(event.value);
            break;
        case ParamId::SpringsDecay:
            m_springs.setT60(event.value);
            break;
        case ParamId::SpringsDiff:
            m_springs.setDiffusion(event.value);
            break;
        case ParamId::SpringsScatter:
            m_springs.setScatter(event.value / 100.f);
            break;
        case ParamId::SpringsDamp:
            m_springs.setFreq(m_springs.getR(), event.value);
            SpringsGL::setDamp(event.value);
            break;
        case ParamId::SpringsChaos:
            m_springs.setTd(m_springs.getTd(), event.value / 100.f);
            break;
        case ParamId::SpringsShape:
            m_springs.setFreq(event.value, m_springs.getFreq());
            break;
        default:
            break;
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
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}

//==============================================================================
void PluginProcessor::parameterValueChanged(int id, float newValue)
{
    auto *ptr = static_cast<juce::RangedAudioParameter *>(getParameters()[id]);
    float value = ptr->convertFrom0to1(newValue);

    /* remove previous event with same id */
    for (auto it = m_paramEvents.begin(); it != m_paramEvents.end();) {
        if (it->id == static_cast<ParamId>(id)) m_paramEvents.erase(it);
        else
            ++it;
    }
    m_paramEvents.emplace_back(id, value);
}

void PluginProcessor::addProcessorAsListener(
    juce::AudioProcessorParameter *param)
{
    jassert(param != nullptr);
    param->addListener(this);
    parameterValueChanged(param->getParameterIndex(), param->getValue());
}
