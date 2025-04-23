#include "Title.h"

namespace aether
{

class ToolTip : public juce::Component
{
  public:
    void setFromComponent(juce::Component *component);
    void paint(juce::Graphics &) override;

  private:
    juce::TextLayout textLayout_;
    juce::Component *component_{nullptr};
};

} // namespace aether
