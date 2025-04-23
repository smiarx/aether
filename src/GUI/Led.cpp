#include "Led.h"
#include "juce_graphics/juce_graphics.h"
#include <algorithm>

namespace aether
{

void Led::timerCallback()
{
    bool ledOn = true;
    if (switchIndicator_.compare_exchange_strong(ledOn, false) && ledOn) {
        counter_ = 0;
    }
    if (counter_ < kLengthMs / kTimerMs) {
        ++counter_;
        intensity_ += 0.9f * (1.f - intensity_);
    } else {
        intensity_ *= (1.f - kSmoothCoef);
    }
    repaint();
}

void Led::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat();
    auto centre = bounds.getCentre();
    auto size   = std::min(bounds.getWidth(), bounds.getHeight());
    bounds.setWidth(size);
    bounds.setHeight(size);
    bounds.setCentre(centre);

    const auto colour = juce::Colours::red.brighter(0.2f);

    auto ledBounds = bounds.reduced(size / 5);
    auto ledSize   = ledBounds.getWidth();
    auto ledColour = colour.darker(1.f).interpolatedWith(colour, intensity_);

    juce::ColourGradient ledGradient{ledColour, centre,
                                     juce::Colours::black.withAlpha(0.6f),
                                     ledBounds.getTopLeft(), true};
    g.setGradientFill(ledGradient);
    g.fillEllipse(ledBounds);

    g.setGradientFill(
        {colour.withAlpha(0.7f * intensity_), centre, colour.withAlpha(0.f),
         bounds.reduced(size / 2 * (1.f - 0.707f)).getTopLeft(), true});
    g.fillEllipse(bounds);

    auto boundsLight = ledBounds.reduced(size * 0.22f);
    auto lightCentre = centre + juce::Point{ledSize / 7, -ledSize / 7};
    boundsLight.setCentre(lightCentre);
    g.setGradientFill({juce::Colours::white.withAlpha(0.5f), lightCentre,
                       juce::Colours::white.withAlpha(0.f),
                       boundsLight.getTopRight(), true});
    g.fillEllipse(boundsLight);
}

} // namespace aether
