#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

class Led : public juce::Component, private juce::Timer
{
  public:
    static constexpr auto kTimerMs    = 40;
    static constexpr auto kLengthMs   = 80;
    static constexpr auto kSmoothCoef = 0.72f;

    Led(std::atomic<bool> &switchIndicator) : switchIndicator_(switchIndicator)
    {
        startTimer(kTimerMs);
    }

    void setFadeOut(int milliseconds);
    void timerCallback() override;
    void paint(juce::Graphics &g) override;

  private:
    std::atomic<bool> &switchIndicator_;
    int counter_{0};
    float intensity_{0.f};
};

} // namespace aether
