#include "Title.h"

namespace aether
{

class ToolTip : public juce::Component
{
  public:
    void setFromComponent(juce::Component *component);
    void paint(juce::Graphics &);

  private:
    juce::TextLayout m_textLayout;
    juce::Component *m_component{nullptr};
};

} // namespace aether
