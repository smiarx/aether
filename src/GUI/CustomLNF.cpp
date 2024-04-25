#include "CustomLNF.h"
#include "DelaySection.h"
#include "PluginEditor.h"
#include "SpringsSection.h"

CustomLNF::CustomLNF()
{
    static const uint32_t definedColours[] = {
        juce::BubbleComponent::backgroundColourId, 0xff000000,
        juce::BubbleComponent::outlineColourId,    0xffdddddd,
        PluginEditor::backgroundColourId,          0xffffffb7,
        DelaySection::backgroundColourId,          0xff011b2d,
        SpringsSection::backgroundColourId,        0xff013559,
        SpringsSection::allBackgroundColourId,     0xffc2ebf1,
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
    auto insideColour = slider.findColour(juce::Slider::thumbColourId);

    auto radius         = juce::jmin(width, height) / 2.f * 0.98f;
    auto lineWidth      = juce::jmax(0.5f, radius * 0.02f);
    auto indicatorWidth = juce::jmax(1.f, radius * 0.14f);
    auto arcRadius      = radius - lineWidth / 2.f;
    auto arcRadiusExt   = arcRadius * 0.06f;
    bool drawExtLine    = true;

    if (arcRadiusExt < 1.8f) {
        drawExtLine = false;
    } else {
        arcRadius -= arcRadiusExt;
        radius = arcRadius * 0.92f;
    }

    auto centre  = juce::Point<float>(x + width / 2.f, y + height / 2.f);
    auto corner  = centre - juce::Point<float>(radius, 0.f);
    auto rw      = radius * 2.f;
    auto rwSmall = rw * 0.72f;

    auto posAngle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    if (drawExtLine) {
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(centre.getX(), centre.getY(), arcRadius,
                                    arcRadius, 0.f, rotaryStartAngle,
                                    rotaryEndAngle, true);

        g.setColour(juce::Colours::black.brighter(0.3));
        g.strokePath(backgroundArc, juce::PathStrokeType(
                                        lineWidth, juce::PathStrokeType::curved,
                                        juce::PathStrokeType::rounded));

        auto startP =
            juce::Point<float>(-sinf(rotaryStartAngle), cosf(rotaryStartAngle));
        auto endP =
            juce::Point<float>(-sinf(rotaryEndAngle), cosf(rotaryEndAngle));
        g.drawLine(
            juce::Line<float>(centre - startP * (arcRadius + arcRadiusExt),
                              centre - startP * (arcRadius - arcRadiusExt)),
            lineWidth);
        g.drawLine(
            juce::Line<float>(centre - endP * (arcRadius + arcRadiusExt),
                              centre - endP * (arcRadius - arcRadiusExt)),
            lineWidth);
    }

    juce::ColourGradient gradient1{juce::Colours::black.brighter(0.1f), centre,
                                   juce::Colours::black.brighter(1.0f), corner,
                                   true};
    gradient1.addColour(0.95, gradient1.getColour(0).brighter(0.2));
    // gradient1.addColour(0.96,
    // gradient1.getColour(gradient1.getNumColours()-1).darker(0.3));
    auto fill1 = juce::FillType{gradient1}.transformed(
        juce::AffineTransform::translation(-radius * 0.02, radius * 0.018));

    juce::ColourGradient gradient2{
        insideColour, centre, insideColour.withRotatedHue(0.05).brighter(0.1),
        corner, true};

    g.setFillType(fill1);
    g.fillEllipse(juce::Rectangle<float>(rw, rw).withCentre(centre));

    juce::Path indicator;
    auto roundedCorner = indicatorWidth / 2.f;
    indicator.addRoundedRectangle(
        juce::Rectangle<float>(radius * 0.97, indicatorWidth)
            .withPosition(centre.getX(), centre.getY() - indicatorWidth / 2),
        roundedCorner);
    g.setColour(juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 1.f).darker(0.1));
    g.fillPath(indicator, juce::AffineTransform::rotation(
                              posAngle - juce::MathConstants<float>::halfPi,
                              centre.getX(), centre.getY()));

    g.setGradientFill(gradient2);
    g.fillEllipse(juce::Rectangle<float>(rwSmall, rwSmall).withCentre(centre));
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
