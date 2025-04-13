#include "ArrowButton.h"

namespace aether
{

ArrowButton::ArrowButton(const juce::String &name,
                         float arrowDirectionInRadians) : juce::Button(name)
{
    // path.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.startNewSubPath({0.f, 0.f});
    path.lineTo({1.f, 0.5f});
    path.lineTo({0.f, 1.f});
    path.lineTo({0.25f, 0.5f});
    path.closeSubPath();
    path.applyTransform(juce::AffineTransform::rotation(
        juce::MathConstants<float>::twoPi * arrowDirectionInRadians, 0.5f,
        0.5f));
}

ArrowButton::~ArrowButton() {}

void ArrowButton::paintButton(juce::Graphics &g,
                              bool /*shouldDrawButtonAsHighlighted*/,
                              bool shouldDrawButtonAsDown)
{
    juce::Path p(path);

    const float offset = shouldDrawButtonAsDown ? 1.0f : 0.0f;
    p.applyTransform(
        path.getTransformToScaleToFit(offset, offset, (float)getWidth() - 3.0f,
                                      (float)getHeight() - 3.0f, false));

    juce::DropShadow(juce::Colours::black.withAlpha(0.3f),
                     shouldDrawButtonAsDown ? 2 : 4, juce::Point<int>())
        .drawForPath(g, p);

    g.setColour(isEnabled() ? colour : juce::Colour(0xff808080));
    g.fillPath(p);
}

} // namespace aether
