#include "ArrowButton.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

namespace aether
{

ArrowButton::ArrowButton(const juce::String &name,
                         float arrowDirectionInRadians) : juce::Button(name)
{
    // path.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path_.startNewSubPath({0.f, 0.f});
    path_.lineTo({1.f, 0.5f});
    path_.lineTo({0.f, 1.f});
    path_.lineTo({0.25f, 0.5f});
    path_.closeSubPath();
    path_.applyTransform(juce::AffineTransform::rotation(
        juce::MathConstants<float>::twoPi * arrowDirectionInRadians, 0.5f,
        0.5f));
}

ArrowButton::~ArrowButton() = default;

void ArrowButton::paintButton(juce::Graphics &g,
                              bool /*shouldDrawButtonAsHighlighted*/,
                              bool shouldDrawButtonAsDown)
{
    juce::Path p(path_);

    constexpr auto kMaxOffset = 2.0f;
    const float offset = shouldDrawButtonAsDown ? 2 * kMaxOffset : kMaxOffset;
    p.applyTransform(path_.getTransformToScaleToFit(
        offset, offset, (float)getWidth() - 2 * kMaxOffset,
        (float)getHeight() - 2 * kMaxOffset, false));

    juce::DropShadow(juce::Colours::black.withAlpha(0.22f),
                     shouldDrawButtonAsDown ? 1 : 2, juce::Point<int>())
        .drawForPath(g, p);

    auto arrowColour = isEnabled() ? colour_ : juce::Colour(0xff808080);
    g.setColour(arrowColour);
    g.fillPath(p);

    g.setColour(arrowColour.darker(0.1f));
    g.strokePath(p, juce::PathStrokeType(1.f));
}

} // namespace aether
