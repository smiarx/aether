#include "ToolTip.h"

namespace aether
{

void ToolTip::setFromComponent(juce::Component *component)
{
    if (component == m_component) return;

    m_component = component;

    juce::TooltipClient *const ttc =
        dynamic_cast<juce::TooltipClient *>(component);

    juce::String toolTip;
    if (ttc != nullptr &&
        !(component->isMouseButtonDown() ||
          component->isCurrentlyBlockedByAnotherModalComponent())) {
        toolTip = ttc->getTooltip();
    }

    setText(toolTip, juce::dontSendNotification);
}

} // namespace aether
