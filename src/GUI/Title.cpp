#include "Title.h"
#include "CustomLNF.h"
#include "PluginEditor.h"

namespace aether
{

Title::Title()
{
    const auto mainColour = juce::Colour(CustomLNF::delayMainColour);
    const auto backColour = juce::Colour(CustomLNF::delayBackColour);

    auto titleFont = juce::Font(CustomLNF::titleTypeface)
                         .withHeight(PluginEditor::headerHeight);
    titleFont.setDefaultMinimumHorizontalScaleFactor(1.f);

    juce::AttributedString attrString;

    attrString.append(juce::String::fromUTF8(u8"Æ"), titleFont, mainColour);
    attrString.append(juce::String("THER"), titleFont, backColour);

    // fix width
    m_maxWidth =
        titleFont.getStringWidthFloat(juce::String::fromUTF8(titleString));
    m_maxWidth += 0.08f * m_maxWidth;

    m_textLayout.createLayout(attrString, m_maxWidth,
                              PluginEditor::headerHeight);
}

void Title::draw(juce::Graphics &g) { m_textLayout.draw(g, m_bounds); }

} // namespace aether
