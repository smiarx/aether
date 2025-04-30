#include "CustomLNF.h"
#include "DelaySection.h"
#include "PluginEditor.h"
#include "Slider.h"
#include "SpringsSection.h"
#include "Typefaces.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <cstdint>
#include <cstdlib>

namespace aether
{

CustomLNF::CustomLNF()
{
    constexpr auto kTextColour                  = 0xff000000;
    constexpr auto kTitleColour                 = 0xffffffff;
    constexpr auto kBackgroundColour            = 0xff393939;
    static constexpr uint32_t kDefinedColours[] = {
        juce::BubbleComponent::backgroundColourId,
        kBackgroundColour,
        juce::BubbleComponent::outlineColourId,
        0xffeeeeee,
        juce::ResizableWindow::backgroundColourId,
        kBackgroundColour,
        juce::Label::textColourId,
        kTextColour,
        juce::ComboBox::ColourIds::textColourId,
        kTextColour,
        DelaySection::kBackgroundColourId,
        kDelayBackColour,
        SpringsSection::kBackgroundColourId,
        kSpringsBackColour,
        juce::Slider::rotarySliderFillColourId,
        0xfff5f5f5,
        juce::Slider::trackColourId,
        0xff999999,
        juce::Slider::rotarySliderOutlineColourId,
        0xff333333,
        PluginEditor::kSeparator,
        0xc9000000,
        juce::ToggleButton::tickDisabledColourId,
        0xff999999,
        juce::ToggleButton::tickColourId,
        0xff00ff00,
        juce::ToggleButton::textColourId,
        kTextColour,

        juce::PopupMenu::ColourIds::backgroundColourId,
        kBackgroundColour,
        juce::PopupMenu::ColourIds::textColourId,
        kTitleColour,
        juce::PopupMenu::ColourIds::highlightedBackgroundColourId,
        0xffa0a0a0,
        juce::PopupMenu::ColourIds::highlightedTextColourId,
        kTextColour,
        juce::PopupMenu::ColourIds::headerTextColourId,
        0xffbfbfbf,

        juce::TextButton::ColourIds::buttonColourId,
        0xff5b5b5b,
        juce::TextButton::ColourIds::buttonOnColourId,
        0xff707070,
        juce::TextButton::ColourIds::textColourOffId,
        0xffffffff,
        juce::TextButton::ColourIds::textColourOnId,
        0xffffffff,
    };

    for (int i = 0; i < juce::numElementsInArray(kDefinedColours); i += 2)
        setColour((int)kDefinedColours[i],
                  juce::Colour(kDefinedColours[i + 1]));

    // noise
    for (int x = 0; x < noise_.getWidth(); ++x) {
        for (int y = 0; y < noise_.getWidth(); ++y) {
            juce::PixelAlpha alpha;
            alpha.setAlpha(static_cast<uint8_t>(std::rand() & 0xff));
            noise_.setPixelAt(x, y, alpha);
        }
    }

    setDefaultSansSerifTypeface(Typefaces::getInstance()->dfault);
}

void CustomLNF::drawRotarySlider(juce::Graphics &g, int x, int y, int width,
                                 int height, float sliderPos,
                                 const float rotaryStartAngle,
                                 const float rotaryEndAngle,
                                 juce::Slider &slider)
{

    /* colours */
    const juce::Colour dialCenterColour{0xff656565};
    const juce::Colour dialCornerColour{0xff434343};
    const juce::Colour dialStrokeDark{0xff303030};
    const juce::Colour dialStrokeLight{0xff707070};
    const juce::Colour thumbDark{0xffd3d3d3};
    const juce::Colour thumbLight{0xffe5e5e5};

    auto fx      = static_cast<float>(x);
    auto fy      = static_cast<float>(y);
    auto fwidth  = static_cast<float>(width);
    auto fheight = static_cast<float>(height);

    auto radius    = juce::jmin(fwidth, fheight) / 2.f;
    auto centre    = juce::Point<float>(fx + fwidth / 2.f, fy + fheight / 2.f);
    auto rectangle = juce::Rectangle<int>(x, y, width, height).toFloat();

    auto posAngle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto middleAngle = (rotaryStartAngle + rotaryEndAngle) * 0.5f;

    auto trackColour  = slider.findColour(juce::Slider::trackColourId);
    auto sliderColour = slider.isEnabled()
                            ? slider.findColour(juce::Slider::thumbColourId)
                            : trackColour;

    auto arcWidth = juce::jmin(6.f, juce::jmax(2.f, radius * 0.14f));

    // parameters
    Slider::Polarity polarity = Slider::kUnipolar;
    bool hasOutline           = false;
    float maxPos              = 1.0f;
    auto *aetherSlider        = dynamic_cast<Slider *>(&slider);
    if (aetherSlider != nullptr) {
        polarity   = aetherSlider->getPolarity();
        hasOutline = aetherSlider->getHasOutline();
        maxPos     = aetherSlider->getMaxPos();
    }

    /* INDICATOR ARC */
    // get polarity of slider
    auto lowColour = polarity == Slider::kUnipolar ? sliderColour : trackColour;
    auto highColour =
        polarity == Slider::kUnipolar ? trackColour : sliderColour;
    auto stopAngle = posAngle;
    if (maxPos < sliderPos) {
        stopAngle =
            rotaryStartAngle + maxPos * (rotaryEndAngle - rotaryStartAngle);
    }
    auto endAngle = rotaryEndAngle;

    if (polarity == Slider::kBipolar) {
        if (stopAngle > middleAngle) {
            stopAngle = middleAngle;
            endAngle  = posAngle;
        } else {
            endAngle = middleAngle;
        }
    }

    auto strokeType = juce::PathStrokeType(
        arcWidth, juce::PathStrokeType::curved, juce::PathStrokeType::square);

    auto arcRadius = radius - arcWidth / 2.f;
    juce::Path arcActive;
    arcActive.addCentredArc(centre.getX(), centre.getY(), arcRadius, arcRadius,
                            0.f, rotaryStartAngle, stopAngle, true);

    juce::ColourGradient arcGradient;
    arcGradient.point1 = rectangle.getBottomLeft();
    arcGradient.point2 = rectangle.getTopLeft();

    arcGradient.addColour(0.f, lowColour.darker(0.2f));
    arcGradient.addColour(1.f, lowColour.brighter(0.2f));
    g.setGradientFill(arcGradient);
    g.strokePath(arcActive, strokeType);

    if (polarity == Slider::kBipolar) {
        juce::Path arcBipolar;
        arcBipolar.addCentredArc(centre.getX(), centre.getY(), arcRadius,
                                 arcRadius, 0.f, endAngle, rotaryEndAngle,
                                 true);
        g.strokePath(arcBipolar, strokeType);
    }

    juce::Path arcInactive;
    arcInactive.addCentredArc(centre.getX(), centre.getY(), arcRadius,
                              arcRadius, 0.f, stopAngle, endAngle, true);

    arcGradient.clearColours();
    arcGradient.addColour(0.f, highColour.darker(0.2f));
    arcGradient.addColour(1.f, highColour.brighter(0.2f));
    g.setGradientFill(arcGradient);
    g.strokePath(arcInactive, strokeType);

    /* DIAL */
    constexpr auto kOutlinePercent = 0.24f;
    auto outlineSize = hasOutline ? radius * kOutlinePercent : 0.f;
    auto dialMargin  = arcWidth * 2.3f + outlineSize * 0.5f;
    auto dialRadius  = radius - dialMargin;
    auto dialRect = juce::Rectangle<float>(2.f * dialRadius, 2.f * dialRadius)
                        .withCentre(centre);
    juce::Path dial;
    dial.addEllipse(dialRect);

    // shadow
    {
        juce::Path dialShadowPath;
        auto dialShadowRadius = dialRadius + outlineSize * 0.5f;
        dialShadowPath.addEllipse(
            juce::Rectangle(dialShadowRadius * 2.f, dialShadowRadius * 2.f)
                .withCentre(centre));
        auto dialShadow =
            juce::DropShadow(juce::Colour(0x3f000000), 9, {-4, 12});
        dialShadow.drawForPath(g, dialShadowPath);
    }

    auto dialGradient = juce::ColourGradient();
    auto dialGradPoint =
        juce::Point(dialRadius, 0.f)
            .rotatedAboutOrigin(-juce::MathConstants<float>::pi / 4);
    dialGradient.isRadial = false;
    dialGradient.point1   = centre - dialGradPoint;
    dialGradient.point2   = centre + dialGradPoint;
    dialGradient.addColour(0.0, dialStrokeDark);
    dialGradient.addColour(1.0, dialStrokeLight);
    g.setGradientFill(dialGradient);
    g.strokePath(dial, juce::PathStrokeType(outlineSize));

    dialGradient.clearColours();
    dialGradient.isRadial = true;
    dialGradient.point1 = centre + juce::Point{-dialRadius, dialRadius} * 0.25f;
    dialGradient.point2 = dialRect.getTopRight();
    dialGradient.addColour(0.0, dialCenterColour);
    dialGradient.addColour(0.68, dialCornerColour);
    dialGradient.addColour(0.7072, dialCornerColour);
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

    juce::ColourGradient thumbGradient(thumbDark, centre, thumbLight,
                                       centre + thumbRotated * thumbStart,
                                       false);
    thumbGradient.addColour(1 - kOutlinePercent, thumbDark);
    g.setGradientFill(thumbGradient);
    g.fillPath(thumb, juce::AffineTransform::rotation(thumbAngle, centre.getX(),
                                                      centre.getY()));

    /* NOISE */
    g.saveState();
    g.reduceClipRegion(dial);
    g.setOpacity(0.02f);
    g.addTransform(juce::AffineTransform::rotation(posAngle, centre.getX(),
                                                   centre.getY()));

    auto idialRect = dialRect.toNearestInt();
    if (idialRect.getWidth() < noise_.getWidth() &&
        idialRect.getHeight() < noise_.getHeight()) {
        g.drawImageAt(noise_, idialRect.getX(), idialRect.getY());
    } else {
        g.drawImage(noise_, dialRect);
    }
    g.restoreState();
}

void CustomLNF::drawBubble(juce::Graphics &g, juce::BubbleComponent &comp,
                           const juce::Point<float> & /*tip*/,
                           const juce::Rectangle<float> &body)
{
    juce::Path p;
    p.addRoundedRectangle(body, 2.f);

    g.setColour(comp.findColour(juce::BubbleComponent::backgroundColourId));
    g.fillPath(p);
}

juce::Font CustomLNF::getSliderPopupFont(juce::Slider &)
{
    auto font = juce::Font(Typefaces::getInstance()->defaultMono)
                    .withPointHeight(kTextPointHeight);
    return font;
}

int CustomLNF::getSliderPopupPlacement(juce::Slider &)
{
    return juce::BubbleComponent::left | juce::BubbleComponent::right;
}

void CustomLNF::setComponentEffectForBubbleComponent(
    juce::BubbleComponent & /*bubbleComponent*/)
{
}

////////////////////////////////////////////
void CustomLNF::drawButtonBackground(juce::Graphics &g, juce::Button &button,
                                     const juce::Colour &backgroundColour,
                                     bool shouldDrawButtonAsHighlighted,
                                     bool shouldDrawButtonAsDown)
{
    auto cornerSize = 6.0f;
    auto bounds     = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

    auto baseColour =
        backgroundColour
            .withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f
                                                                    : 0.9f)
            .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour =
            baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

    g.setColour(baseColour);

    auto flatOnLeft   = button.isConnectedOnLeft();
    auto flatOnRight  = button.isConnectedOnRight();
    auto flatOnTop    = button.isConnectedOnTop();
    auto flatOnBottom = button.isConnectedOnBottom();

    if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom) {
        juce::Path path;
        path.addRoundedRectangle(
            bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
            cornerSize, cornerSize, !(flatOnLeft || flatOnTop),
            !(flatOnRight || flatOnTop), !(flatOnLeft || flatOnBottom),
            !(flatOnRight || flatOnBottom));

        g.fillPath(path);
    } else {
        g.fillRoundedRectangle(bounds, cornerSize);
    }
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
    auto shadowRadius = static_cast<int>(circleBounds.getWidth() * 0.2f);
    juce::DropShadow(juce::Colour(0x7f000000), shadowRadius,
                     {0, static_cast<int>(circleReduce)})
        .drawForPath(g, circle);

    // circle
    auto circleColour = juce::Colour(0xffeeeeee);
    g.setColour(circleColour);
    g.fillPath(circle);

    /*  TEXT */
    auto textColour =
        button.findColour(juce::ToggleButton::ColourIds::textColourId);
    g.setColour(textColour);
    auto fontSize = static_cast<float>(button.getHeight());
    g.setFont(
        juce::Font(Typefaces::getInstance()->dfault).withHeight(fontSize));

    if (!button.isEnabled()) g.setOpacity(0.5f);

    g.drawFittedText(button.getButtonText(),
                     button.getLocalBounds()
                         .withTrimmedLeft(juce::roundToInt(boxWidth) + 4)
                         .withTrimmedRight(2),
                     juce::Justification::centredLeft, 1);
}

juce::Font
CustomLNF::getTextButtonFont([[maybe_unused]] juce::TextButton &button,
                             int buttonHeight)
{
    return juce::Font(Typefaces::getInstance()->dfault)
        .withPointHeight(
            juce::jmin((float)kTextPointHeight, (float)buttonHeight * 0.6f));
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

juce::Font CustomLNF::getComboBoxFont(juce::ComboBox &comboBox)
{
    if (comboBox.getName() == "TimeType") {
        return juce::Font(Typefaces::getInstance()->defaultMono)
            .withPointHeight(CustomLNF::kTextPointHeight - 1);
    }
    return juce::Font(Typefaces::getInstance()->dfault)
        .withPointHeight(CustomLNF::kTextPointHeight);
}
void CustomLNF::positionComboBoxText(juce::ComboBox &box, juce::Label &label)
{
    label.setBounds(0, 0, box.getWidth(), box.getHeight());
    label.setJustificationType(juce::Justification::centred);

    label.setFont(getComboBoxFont(box));
}

///////////////////////////////////////////////
juce::Font CustomLNF::getPopupMenuFont()
{
    return juce::Font(Typefaces::getInstance()->dfault)
        .withPointHeight(kTextPointHeight);
}

void CustomLNF::getIdealPopupMenuItemSize(
    const juce::String &text, const bool isSeparator,
    [[maybe_unused]] int standardMenuItemHeight, int &idealWidth,
    int &idealHeight)
{
    if (isSeparator) {
        idealWidth = 50;
        idealHeight =
            standardMenuItemHeight > 0 ? standardMenuItemHeight / 2 : 10;
    } else {
        juce::Font font = getPopupMenuFont();

        idealHeight =
            juce::roundToInt(font.getHeight() * kPopupElementSizeFontRatio);
        idealWidth = font.getStringWidth(text) + idealHeight;
    }
}

void CustomLNF::drawPopupMenuItem(
    juce::Graphics &g, const juce::Rectangle<int> &area, const bool isSeparator,
    const bool isActive, const bool isHighlighted,
    [[maybe_unused]] const bool isTicked, const bool hasSubMenu,
    const juce::String &text, const juce::String &shortcutKeyText,
    [[maybe_unused]] const juce::Drawable *icon,
    const juce::Colour *const textColourToUse)
{
    if (isSeparator) {
        auto r = area.reduced(5, 0);
        r.removeFromTop(r.getHeight() / 2 - 1);

        g.setColour(juce::Colour(0x33000000));
        g.fillRect(r.removeFromTop(1));

        g.setColour(juce::Colour(0x66ffffff));
        g.fillRect(r.removeFromTop(1));
    } else {
        auto textColour = findColour(juce::PopupMenu::textColourId);

        if (textColourToUse != nullptr) textColour = *textColourToUse;

        auto r = area;

        if (isHighlighted) {
            g.setColour(
                findColour(juce::PopupMenu::highlightedBackgroundColourId));
            g.fillRect(r);

            g.setColour(findColour(juce::PopupMenu::highlightedTextColourId));
        } else {
            g.setColour(textColour);
        }

        if (!isActive) g.setOpacity(0.3f);

        juce::Font font(getPopupMenuFont());

        auto maxFontHeight =
            (float)area.getHeight() / kPopupElementSizeFontRatio;

        if (font.getHeight() > maxFontHeight) font.setHeight(maxFontHeight);

        g.setFont(font);

        // left margin
        r.removeFromLeft(r.getHeight() / 2);

        if (hasSubMenu) {
            auto arrowH = 0.6f * getPopupMenuFont().getAscent();

            auto x     = (float)r.removeFromRight((int)arrowH).getX();
            auto halfH = (float)r.getCentreY();

            juce::Path p;
            p.addTriangle(x, halfH - arrowH * 0.5f, x, halfH + arrowH * 0.5f,
                          x + arrowH * 0.6f, halfH);

            g.fillPath(p);
        }

        r.removeFromRight(3);
        g.drawFittedText(text, r, juce::Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty()) {
            juce::Font f2(font);
            f2.setHeight(f2.getHeight() * 0.75f);
            f2.setHorizontalScale(0.95f);
            g.setFont(f2);

            g.drawText(shortcutKeyText, r, juce::Justification::centredRight,
                       true);
        }
    }
}

} // namespace aether
