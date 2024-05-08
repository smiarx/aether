#include "CustomLNF.h"
#include "DelaySection.h"
#include "PluginEditor.h"
#include "SpringsSection.h"

CustomLNF::CustomLNF()
{
    static const uint32_t definedColours[] = {
        juce::BubbleComponent::backgroundColourId,
        0xff000000,
        juce::BubbleComponent::outlineColourId,
        0xffdddddd,
        PluginEditor::backgroundColourId,
        0xffffffb7,
        DelaySection::backgroundColourId,
        0xff011b2d,
        SpringsSection::backgroundColourId,
        0xff013559,
        SpringsSection::allBackgroundColourId,
        0xffc2ebf1,
        juce::Slider::rotarySliderFillColourId,
        0xfff5f5f5,
        juce::Slider::trackColourId,
        0xff808080,
        juce::Slider::rotarySliderOutlineColourId,
        0xff333333,
    };

    for (int i = 0; i < juce::numElementsInArray(definedColours); i += 2)
        setColour((int)definedColours[i], juce::Colour(definedColours[i + 1]));
}

void CustomLNF::drawRotarySlider(juce::Graphics &g, int x, int y, int width,
                                 int height, float sliderPos,
                                 const float rotaryStartAngle,
                                 const float rotaryEndAngle,
                                 juce::Slider &slider)
{
    auto radius          = juce::jmin(width, height) / 2.f;
    auto lineWidth       = juce::jmin(6.f, juce::jmax(2.f, radius * 0.09f));
    auto arcRadius       = radius - lineWidth / 2.f;
    auto dialMargin      = juce::jmin(16.f, juce::jmax(4.f, radius * 0.22f));
    auto outlineSize     = 1.f;
    auto dialRadius      = radius - dialMargin;
    auto centerRadius    = dialRadius * 0.7f;
    auto indicatorWidth  = juce::jmin(6.f, juce::jmax(1.f, radius * 0.12f));
    auto indicatorLength = 0.7f * dialRadius;

    auto thumbColour = slider.findColour(juce::Slider::thumbColourId);
    auto dialColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
    auto indicColour = dialColour.contrasting(0.8f);
    auto outlineColour =
        slider.findColour(juce::Slider::rotarySliderOutlineColourId);
    auto trackColour = slider.findColour(juce::Slider::trackColourId);

    auto centre = juce::Point<float>(x + width / 2.f, y + height / 2.f);

    auto posAngle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    juce::Path arcActive;
    arcActive.addCentredArc(centre.getX(), centre.getY(), arcRadius, arcRadius,
                            0.f, rotaryStartAngle, posAngle, true);
    g.setColour(thumbColour);
    g.strokePath(arcActive,
                 juce::PathStrokeType(lineWidth, juce::PathStrokeType::curved,
                                      juce::PathStrokeType::square));

    juce::Path arcInactive;
    arcInactive.addCentredArc(centre.getX(), centre.getY(), arcRadius,
                              arcRadius, 0.f, posAngle, rotaryEndAngle, true);
    g.setColour(trackColour);
    g.strokePath(arcInactive,
                 juce::PathStrokeType(lineWidth, juce::PathStrokeType::curved,
                                      juce::PathStrokeType::square));

    /* dial */
    juce::ColourGradient gradient{
        dialColour.brighter(0.3f), juce::Point<float>(x, y),
        dialColour.darker(0.3f), juce::Point<float>(x, y + height), false};
    g.setFillType(juce::FillType{gradient});
    g.fillEllipse(juce::Rectangle<float>(2.f * dialRadius, 2.f * dialRadius)
                      .withCentre(centre));

    float indicatorStart = dialRadius;
    juce::Path indicator;
    auto roundedCorner = indicatorWidth / 2.f;
    indicator.addRoundedRectangle(
        juce::Rectangle<float>(indicatorLength - outlineSize, indicatorWidth)
            .withPosition(centre.getX() + indicatorStart - indicatorLength,
                          centre.getY() - indicatorWidth / 2),
        roundedCorner);
    g.setColour(indicColour);
    g.fillPath(indicator, juce::AffineTransform::rotation(
                              posAngle - juce::MathConstants<float>::halfPi,
                              centre.getX(), centre.getY()));

    if (radius > 20.f) {
        g.setFillType(juce::FillType{juce::ColourGradient{
            thumbColour, centre, thumbColour.darker(0.05f),
            centre + juce::Point<float>(0, centerRadius / 2.f), true}});
        // g.setColour(thumbColour);
        g.fillEllipse(
            juce::Rectangle<float>(2.f * centerRadius, 2.f * centerRadius)
                .withCentre(centre));
        g.setColour(outlineColour);
        g.drawEllipse(
            juce::Rectangle<float>(2.f * centerRadius, 2.f * centerRadius)
                .withCentre(centre),
            outlineSize);
    }

    g.setColour(outlineColour);
    g.drawEllipse(juce::Rectangle<float>(2.f * dialRadius, 2.f * dialRadius)
                      .withCentre(centre),
                  outlineSize);
}

void CustomLNF::drawBubble(juce::Graphics &g, juce::BubbleComponent &comp,
                           const juce::Point<float> & /*tip*/,
                           const juce::Rectangle<float> &body)
{
    juce::Path p;
    p.addRoundedRectangle(body, 3.f);

    g.setColour(comp.findColour(juce::BubbleComponent::backgroundColourId));
    g.fillPath(p);

    g.setColour(comp.findColour(juce::BubbleComponent::outlineColourId));
    g.strokePath(p, juce::PathStrokeType(1.0f));
}

void CustomLNF::setComponentEffectForBubbleComponent(
    juce::BubbleComponent & /*bubbleComponent*/)
{
}
