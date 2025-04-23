#include "ToolTip.h"
#include "CustomLNF.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

namespace aether
{

void ToolTip::setFromComponent(juce::Component *component)
{
    if (component == component_) return;

    component_ = component;

    auto *const ttc = dynamic_cast<juce::TooltipClient *>(component);

    juce::String toolTip;
    if (ttc != nullptr &&
        !(component->isMouseButtonDown() ||
          component->isCurrentlyBlockedByAnotherModalComponent())) {

        toolTip = ttc->getTooltip();
        if (toolTip != "") {
            const auto mainColour = juce::Colour(CustomLNF::kDelayMainColour);
            const auto backColour = juce::Colour(CustomLNF::kDelayBackColour);

            juce::AttributedString attrStr;
            auto font =
                juce::Font(CustomLNF::defaultTypeface).withPointHeight(14);
            attrStr.append(component->getTitle() + ": ", font, mainColour);
            attrStr.append(toolTip, font, backColour);
            attrStr.setJustification(juce::Justification::verticallyCentred);

            auto bounds = getBounds();
            textLayout_.createLayout(attrStr, bounds.getWidth(),
                                     bounds.getHeight());
            repaint();
            return;
        }

        textLayout_.createLayout(juce::AttributedString(""), 0.f);
        repaint();
    }
}

void ToolTip::paint(juce::Graphics &g)
{
    textLayout_.draw(g, getLocalBounds().toFloat());
}

} // namespace aether
