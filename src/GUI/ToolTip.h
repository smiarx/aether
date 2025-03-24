#include <juce_gui_basics/juce_gui_basics.h>

class ToolTip : public juce::Label
{
  public:
    void setFromComponent(juce::Component *component);

  private:
    juce::Component *m_component{nullptr};
};
