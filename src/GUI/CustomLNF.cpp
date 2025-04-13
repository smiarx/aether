#include "CustomLNF.h"
#include "BinaryData.h"
#include "DelaySection.h"
#include "PluginEditor.h"
#include "SpringsSection.h"
#include "juce_core/juce_core.h"

namespace aether
{

juce::Typeface::Ptr CustomLNF::defaultTypeface = nullptr;

CustomLNF::CustomLNF()
{
    static const uint32_t definedColours[] = {
        juce::BubbleComponent::backgroundColourId,
        0xff000000,
        juce::BubbleComponent::outlineColourId,
        0xffdddddd,
        juce::ResizableWindow::backgroundColourId,
        0xffffffb7,
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
        PluginEditor::Separator,
        0xff011b2d,
        juce::ToggleButton::tickDisabledColourId,
        0xff999999,
        juce::ToggleButton::tickColourId,
        0xff00ff00,
        juce::ToggleButton::textColourId,
        0xffffffff,
    };

    for (int i = 0; i < juce::numElementsInArray(definedColours); i += 2)
        setColour((int)definedColours[i], juce::Colour(definedColours[i + 1]));

    // noise
    for (int x = 0; x < noise.getWidth(); ++x) {
        for (int y = 0; y < noise.getWidth(); ++y) {
            juce::PixelAlpha alpha;
            alpha.setAlpha(std::rand());
            noise.setPixelAt(x, y, alpha);
        }
    }

    if (defaultTypeface == nullptr) {
        defaultTypeface = juce::Typeface::createSystemTypefaceFor(
            BinaryData::Lexend300_ttf, BinaryData::Lexend300_ttfSize);
    }
    setDefaultSansSerifTypeface(defaultTypeface);
}

void CustomLNF::drawRotarySlider(juce::Graphics &g, int x, int y, int width,
                                 int height, float sliderPos,
                                 const float rotaryStartAngle,
                                 const float rotaryEndAngle,
                                 juce::Slider &slider)
{

    auto fx      = static_cast<float>(x);
    auto fy      = static_cast<float>(y);
    auto fwidth  = static_cast<float>(width);
    auto fheight = static_cast<float>(height);

    auto radius    = juce::jmin(fwidth, fheight) / 2.f;
    auto centre    = juce::Point<float>(fx + fwidth / 2.f, fy + fheight / 2.f);
    auto rectangle = juce::Rectangle<float>(fx, fy, width, height);

    auto posAngle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    auto trackColour  = slider.findColour(juce::Slider::trackColourId);
    auto sliderColour = slider.isEnabled()
                            ? slider.findColour(juce::Slider::thumbColourId)
                            : trackColour;

    auto arcWidth = juce::jmin(6.f, juce::jmax(2.f, radius * 0.14f));

    /* DIAL */
    constexpr auto outlinePercent = 0.24f;
    auto outlineSize = radius < 50.f ? 0.f : radius * outlinePercent;
    auto dialMargin  = arcWidth * 2.3f + outlineSize * 0.5f;
    auto dialRadius  = radius - dialMargin;
    auto dialRect = juce::Rectangle<float>(2.f * dialRadius, 2.f * dialRadius)
                        .withCentre(centre);
    juce::Path dial;
    dial.addEllipse(dialRect);

    auto dialShadow = juce::DropShadow(juce::Colour(0x77000000), 9,
                                       {-static_cast<int>(dialRadius * 0.08f),
                                        static_cast<int>(dialRadius * 0.36f)});
    dialShadow.drawForPath(g, dial);

    auto dialGradient = juce::ColourGradient();
    auto dialGradPoint =
        juce::Point(dialRadius, 0.f)
            .rotatedAboutOrigin(-juce::MathConstants<float>::pi / 4);
    dialGradient.isRadial = false;
    dialGradient.point1   = centre - dialGradPoint;
    dialGradient.point2   = centre + dialGradPoint;
    dialGradient.addColour(0.0, juce::Colour(0xff757575));
    dialGradient.addColour(1.0, juce::Colour(0xfff0f0f0));
    g.setGradientFill(dialGradient);
    g.strokePath(dial, juce::PathStrokeType(outlineSize));

    dialGradient.clearColours();
    dialGradient.isRadial = true;
    dialGradient.point1 = centre + juce::Point{-dialRadius, dialRadius} * 0.25f;
    dialGradient.point2 = dialRect.getTopRight();
    dialGradient.addColour(0.0, juce::Colour(0xfff4f4f4));
    dialGradient.addColour(0.68, juce::Colour(0xffcacaca));
    dialGradient.addColour(0.7072, juce::Colour(0xffcacaca));
    g.setGradientFill(dialGradient);
    g.fillPath(dial);

    /* THUMB */
    auto thumbWidth  = juce::jmin(6.f, juce::jmax(1.f, radius * 0.08f));
    auto thumbLength = 0.6f * dialRadius + outlineSize * 0.5f;

    float thumbStart     = dialRadius + outlineSize * 0.5f;
    juce::Point thumbPos = {centre.getX() + thumbStart - thumbLength,
                            centre.getY() - thumbWidth / 2.f};
    auto thumbRect =
        juce::Rectangle(thumbLength, thumbWidth).withPosition(thumbPos);

    auto thumbAngle   = posAngle - juce::MathConstants<float>::halfPi;
    auto thumbRotated = juce::Point(1.f, 0.f).rotatedAboutOrigin(thumbAngle);

    juce::Path thumb;
    thumb.addRectangle(thumbRect);

    juce::ColourGradient thumbGradient(
        juce::Colour(0xff303030), centre, juce::Colour(0x60000000),
        centre + thumbRotated * thumbStart, false);
    thumbGradient.addColour(1 - outlinePercent, juce::Colour(0xff303030));
    g.setGradientFill(thumbGradient);
    // g.setColour(juce::Colour(0xff202020));
    g.fillPath(thumb, juce::AffineTransform::rotation(thumbAngle, centre.getX(),
                                                      centre.getY()));

    /* NOISE */
    g.saveState();
    g.reduceClipRegion(dial);
    g.setOpacity(0.07f);
    g.addTransform(juce::AffineTransform::rotation(posAngle, centre.getX(),
                                                   centre.getY()));

    if (dialRect.getWidth() < noise.getWidth() &&
        dialRect.getHeight() < noise.getHeight()) {
        g.drawImageAt(noise, dialRect.getX(), dialRect.getY());
    } else {
        g.drawImage(noise, dialRect);
    }
    g.restoreState();

    /* INDICATOR ARC */
    auto arcRadius = radius - arcWidth / 2.f;
    juce::Path arcActive;
    arcActive.addCentredArc(centre.getX(), centre.getY(), arcRadius, arcRadius,
                            0.f, rotaryStartAngle, posAngle, true);

    juce::ColourGradient arcGradient;
    arcGradient.point1 = rectangle.getBottomLeft();
    arcGradient.point2 = rectangle.getTopLeft();

    arcGradient.addColour(0.f, sliderColour.darker(0.2));
    arcGradient.addColour(1.f, sliderColour.brighter(0.2));
    g.setGradientFill(arcGradient);
    g.strokePath(arcActive,
                 juce::PathStrokeType(arcWidth, juce::PathStrokeType::curved,
                                      juce::PathStrokeType::square));

    juce::Path arcInactive;
    arcInactive.addCentredArc(centre.getX(), centre.getY(), arcRadius,
                              arcRadius, 0.f, posAngle, rotaryEndAngle, true);

    arcGradient.clearColours();
    arcGradient.addColour(0.f, trackColour.darker(0.2));
    arcGradient.addColour(1.f, trackColour.brighter(0.2));
    g.setGradientFill(arcGradient);
    g.strokePath(arcInactive,
                 juce::PathStrokeType(arcWidth, juce::PathStrokeType::curved,
                                      juce::PathStrokeType::square));

    if (radius > 120.f) {
        auto insideRadius = dialRadius * 0.76f;
        auto insideRect =
            juce::Rectangle(insideRadius * 2.f, insideRadius * 2.f)
                .withCentre(centre);
        juce::Path inside;
        inside.addEllipse(insideRect);
        juce::ColourGradient insideGradient = {
            sliderColour.brighter(0.2),
            centre + juce::Point{-0.25f * insideRadius, 0.25f * insideRadius},
            sliderColour.darker(0.2), insideRect.getTopRight(), true};

        g.setGradientFill(insideGradient);
        g.fillPath(inside);

        insideGradient.clearColours();
        insideGradient.isRadial = false;
        insideGradient.point1   = centre - dialGradPoint;
        insideGradient.point2   = centre + dialGradPoint;
        insideGradient.addColour(0.0, juce::Colour(0xaacfcfcf));
        insideGradient.addColour(1.0, juce::Colour(0xaa808080));
        g.setGradientFill(insideGradient);
        g.strokePath(inside, juce::PathStrokeType(1.8f));
    }
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

void CustomLNF::drawButtonBackground(juce::Graphics &g, juce::Button &button,
                                     const juce::Colour &backgroundColour,
                                     bool shouldDrawButtonAsHighlighted,
                                     bool shouldDrawButtonAsDown)
{
    juce::LookAndFeel_V4::drawButtonBackground(g, button, backgroundColour,
                                               shouldDrawButtonAsHighlighted,
                                               shouldDrawButtonAsDown);
}

void CustomLNF::drawToggleButton(
    juce::Graphics &g, juce::ToggleButton &button,
    [[maybe_unused]] bool shouldDrawButtonAsHighlighted,
    [[maybe_unused]] bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();

    /* BOX */
    auto boxWidth  = bounds.getHeight() * 1.5f;
    auto boxBounds = bounds.removeFromLeft(boxWidth);
    auto diameter  = boxBounds.getHeight();
    auto radius    = diameter / 2.f;

    juce::Path mainBox;
    mainBox.addRoundedRectangle(boxBounds, radius);

    juce::Path circle;
    auto circleBounds = button.getToggleState()
                            ? boxBounds.removeFromRight(diameter)
                            : boxBounds.removeFromLeft(diameter);
    auto circleReduce = radius * 0.22f;
    circleBounds      = circleBounds.reduced(circleReduce);
    circle.addEllipse(circleBounds);

    auto boxColour = button.findColour(
        button.getToggleState()
            ? juce::ToggleButton::ColourIds::tickColourId
            : juce::ToggleButton::ColourIds::tickDisabledColourId);
    g.setColour(boxColour);
    g.fillPath(mainBox);

    // shadow
    auto shadowRadius = circleBounds.getWidth() * 0.1f;
    juce::DropShadow(juce::Colour(0xff303030), shadowRadius,
                     {0, -static_cast<int>(circleReduce - shadowRadius)})
        .drawForPath(g, circle);

    // circle
    auto circleColour = juce::Colour(0xffcecec3);
    g.setColour(circleColour);
    g.fillPath(circle);

    /*  TEXT */
    auto textColour =
        button.findColour(juce::ToggleButton::ColourIds::textColourId);
    g.setColour(textColour);
    auto fontSize = button.getHeight();
    g.setFont(juce::Font(defaultTypeface).withHeight(fontSize));

    if (!button.isEnabled()) g.setOpacity(0.5f);

    g.drawFittedText(button.getButtonText(),
                     button.getLocalBounds()
                         .withTrimmedLeft(juce::roundToInt(boxWidth) + 4)
                         .withTrimmedRight(2),
                     juce::Justification::centredLeft, 1);
}

juce::Font CustomLNF::getTextButtonFont(juce::TextButton &button,
                                        int buttonHeight)
{
    return juce::LookAndFeel_V4::getTextButtonFont(button, buttonHeight);
}

int CustomLNF::getTextButtonWidthToFitText(juce::TextButton &b,
                                           int buttonHeight)
{
    return getTextButtonFont(b, buttonHeight).getStringWidth(b.getButtonText());
}

void CustomLNF::drawButtonText(juce::Graphics &g, juce::TextButton &button,
                               bool /*shouldDrawButtonAsHighlighted*/,
                               bool /*shouldDrawButtonAsDown*/)
{
    juce::Font font(getTextButtonFont(button, button.getHeight()));
    g.setFont(font);
    g.setColour(button
                    .findColour(button.getToggleState()
                                    ? juce::TextButton::textColourOnId
                                    : juce::TextButton::textColourOffId)
                    .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));

    const int textWidth = button.getWidth();
    if (textWidth > 0)
        g.drawFittedText(button.getButtonText(), 0, 0, textWidth,
                         button.getHeight(), juce::Justification::centred, 1);
}

void CustomLNF::drawComboBox(juce::Graphics &, int, int, bool, int, int, int,
                             int, juce::ComboBox &)
{
}

juce::Font CustomLNF::getComboBoxFont(juce::ComboBox &)
{
    return juce::Font(defaultTypeface)
        .withPointHeight(CustomLNF::textPointHeight);
}
void CustomLNF::positionComboBoxText(juce::ComboBox &box, juce::Label &label)
{
    label.setBounds(0, 0, box.getWidth(), box.getHeight());
    label.setJustificationType(juce::Justification::centred);

    label.setFont(getComboBoxFont(box));
}

} // namespace aether
