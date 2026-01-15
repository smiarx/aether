#include "Title.h"
#include "CustomLNF.h"
#include "PluginEditor.h"
#include "Typefaces.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"

namespace aelapse
{

Title::Title()
{
    const auto mainColour = juce::Colour(CustomLNF::kDelayMainColour);
    const auto backColour = juce::Colour(CustomLNF::kDelayBackColour);

    auto titleFont = juce::Font(Typefaces::getInstance()->title)
                         .withHeight(PluginEditor::kHeaderHeight - 0.5);
    juce::Font::setDefaultMinimumHorizontalScaleFactor(1.f);

    juce::AttributedString attrString;

    attrString.append(juce::String::fromUTF8(u8"Æ"), titleFont, mainColour);
    attrString.append(juce::String("LAPSE"), titleFont, backColour);

    // fix width
    maxWidth_ =
        titleFont.getStringWidthFloat(juce::String::fromUTF8(kTitleString));
    maxWidth_ += 0.08f * maxWidth_;

    textLayout_.createLayout(attrString, maxWidth_,
                             PluginEditor::kHeaderHeight);
}

void Title::draw(juce::Graphics &g) { textLayout_.draw(g, bounds_); }

} // namespace aelapse
