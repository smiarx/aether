#include "Title.h"

namespace aether
{

class ToolTip : public juce::Component
{
  public:
    static constexpr auto kTextHeight = 17;
    void setFromComponent(juce::Component *component);
    void paint(juce::Graphics &) override;

  private:
    juce::TextLayout textLayout_;
    juce::Component *component_{nullptr};
};

} // namespace aether
