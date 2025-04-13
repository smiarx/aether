#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

class Led : public juce::Component, private juce::Timer
{
  public:
    static constexpr auto TimerMs    = 40;
    static constexpr auto LengthMs   = 80;
    static constexpr auto SmoothCoef = 0.72f;

    Led(std::atomic<bool> &switchIndicator) : m_switchIndicator(switchIndicator)
    {
        startTimer(TimerMs);
    }

    void setFadeOut(int milliseconds);
    virtual void timerCallback() override;
    void paint(juce::Graphics &g) override;

  private:
    std::atomic<bool> &m_switchIndicator;
    int m_counter{0};
    float m_intensity{0.f};
};

} // namespace aether
