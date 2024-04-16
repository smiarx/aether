#include "SpreadSlider.h"

_SpreadSlider::_SpreadSlider(juce::AudioProcessorValueTreeState &apvts,
                             const juce::String &spreadId) :
    m_spread{apvts.getParameter(spreadId)->getValue()},
    m_spreadAttachment(
        *apvts.getParameter(spreadId),
        [&](float value) {
            m_spread = value;
            repaint();
        },
        apvts.undoManager),
    m_spreadRange(apvts.getParameter(spreadId)->getNormalisableRange())
{
    setSliderStyle(juce::Slider::RotaryVerticalDrag);
}

void _SpreadSlider::paint(juce::Graphics &g)
{
    auto &lf              = getLookAndFeel();
    auto layout           = lf.getSliderLayout(*this);
    const auto sliderRect = layout.sliderBounds;

    const auto sliderPos = (float)valueToProportionOfLength(getValue());
    const auto spreadPos = m_spread;

    drawSpreadSlider(g, sliderRect.getX(), sliderRect.getY(),
                     sliderRect.getWidth(), sliderRect.getHeight(), sliderPos,
                     spreadPos);
}

void _SpreadSlider::drawSpreadSlider(juce::Graphics &g, int x, int y, int width,
                                     int height, float sliderPos,
                                     float spreadPos)
{
    const auto rotaryParams     = getRotaryParameters();
    const auto rotaryStartAngle = rotaryParams.startAngleRadians;
    const auto rotaryEndAngle   = rotaryParams.endAngleRadians;

    getLookAndFeel().drawRotarySlider(g, x, y, width, height, sliderPos,
                                      rotaryStartAngle, rotaryEndAngle, *this);

    /* spread arc */
    auto radius       = juce::jmin(width, height) / 2.f * 0.98f;
    auto lineWidth    = juce::jmax(0.5f, radius * 0.02f);
    auto arcRadius    = radius - lineWidth / 2.f;
    auto arcRadiusExt = arcRadius * 0.06f;

    float spreadWidth;

    if (arcRadiusExt < 1.8f) {
        spreadWidth = radius * 0.16f;
    } else {
        arcRadius -= arcRadiusExt;
        radius      = arcRadius * 0.92f;
        spreadWidth = arcRadiusExt + lineWidth;
    }

    auto centre       = juce::Point<float>(x + width / 2.f, y + height / 2.f);
    auto spreadRadius = radius + (spreadWidth + 1.f) / 2.f;

    /* draw spread value */
    const auto spreadAdd  = 0.f; // std::atan(0.5f * lineW / arcRadius);
    auto spreadAngleStart = juce::jmax(
        rotaryStartAngle + 0.01f,
        rotaryStartAngle +
            (sliderPos - spreadPos) * (rotaryEndAngle - rotaryStartAngle) -
            spreadAdd);
    auto spreadAngleEnd = juce::jmin(
        rotaryEndAngle - 0.01f,
        rotaryStartAngle +
            (sliderPos + spreadPos) * (rotaryEndAngle - rotaryStartAngle) +
            spreadAdd);
    juce::Path spreadArc;
    spreadArc.addCentredArc(centre.getX(), centre.getY(), spreadRadius,
                            spreadRadius, 0.0f, spreadAngleStart,
                            spreadAngleEnd, true);
    g.setColour(juce::Colour(0xff9cbcbd));
    g.strokePath(spreadArc,
                 juce::PathStrokeType(spreadWidth, juce::PathStrokeType::curved,
                                      juce::PathStrokeType::rounded));
}

void _SpreadSlider::mouseDown(const juce::MouseEvent &e)
{
    m_mouseDragStartPos = e.position;
    m_valueDragStartPos = m_spreadRange.convertTo0to1(m_spread);

    if (!e.mods.isAltDown()) Slider::mouseDown(e);
}

void _SpreadSlider::mouseDrag(const juce::MouseEvent &e)
{
    constexpr auto pixelForFullDragExtent = 250;
    if (!e.mods.isShiftDown() && !e.mods.isCtrlDown()) {
        auto diff = e.position.x - m_mouseDragStartPos.x;
        float value =
            m_valueDragStartPos + diff * (1.f / pixelForFullDragExtent);
        value = juce::jlimit(0.f, 1.f, value);

        m_spreadAttachment.setValueAsPartOfGesture(
            m_spreadRange.convertFrom0to1(value));
    }

    /* set value from slider */
    if (!e.mods.isAltDown()) {
        juce::Slider::mouseDrag(e);
    }
}
