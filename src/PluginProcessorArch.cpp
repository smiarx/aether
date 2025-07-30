#include "PluginProcessorArch.h"

namespace aether::DSP_ARCH_NAMESPACE
{

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
                isDotted_   = event.value > 1.f;
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
                case kBeat16:
                    mult = 1.0 / 6.0;
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
                case kBeat4:
                    mult = 4.0;
                    break;
                }
                if (isDotted_) {
                    mult += mult * 0.5;
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
    rmsPos_.store(static_cast<int>(*springs_.getRMSStackPos()));
}
} // namespace aether::DSP_ARCH_NAMESPACE

namespace aether
{

#if DSP_AVX
#define LOADFUNC loadPluginAVX2
#else
#define LOADFUNC loadPluginDefault
#endif

juce::AudioProcessor *LOADFUNC()
{
    return new DSP_ARCH_NAMESPACE::PluginProcessor();
}

} // namespace aether
