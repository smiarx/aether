#include "ToolTip.h"
#include "CustomLNF.h"

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
        if (toolTip != "") {
            const auto mainColour = juce::Colour(CustomLNF::delayMainColour);
            const auto backColour = juce::Colour(CustomLNF::delayBackColour);

            juce::AttributedString attrStr;
            auto font =
                juce::Font(CustomLNF::defaultTypeface).withPointHeight(14);
            attrStr.append(component->getTitle() + ": ", font, mainColour);
            attrStr.append(toolTip, font, backColour);
            attrStr.setJustification(juce::Justification::verticallyCentred);

            auto bounds = getBounds();
            m_textLayout.createLayout(attrStr, bounds.getWidth(),
                                      bounds.getHeight());
            repaint();
            return;
        }

        m_textLayout.createLayout(juce::AttributedString(""), 0.f);
        repaint();
    }
}

void ToolTip::paint(juce::Graphics &g)
{
    m_textLayout.draw(g, getLocalBounds().toFloat());
}

} // namespace aether
