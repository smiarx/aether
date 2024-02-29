#include "SpreadSlider.h"

SpreadSlider::SpreadSlider()
{
    m_spreadAttachment.onParameterChangedAsync = [&]() {
        repaint();
    };
}

void SpreadSlider::setSpreadParameter(juce::RangedAudioParameter *parameter)
{
    m_spreadAttachment.attachToParameter(parameter);
}

void SpreadSlider::paint(juce::Graphics &g)
{
    auto &lf              = getLookAndFeel();
    auto layout           = lf.getSliderLayout(*this);
    const auto sliderRect = layout.sliderBounds;

    const auto sliderPos = (float)valueToProportionOfLength(getValue());
    const auto spreadPos = m_spreadAttachment.getValue();

    if (isRotary())
        drawRotarySlider(g, sliderRect.getX(), sliderRect.getY(),
                         sliderRect.getWidth(), sliderRect.getHeight(),
                         sliderPos, spreadPos);
    else
        drawLinearSlider(g, sliderRect.getX(), sliderRect.getY(),
                         sliderRect.getWidth(), sliderRect.getHeight(),
                         sliderPos, spreadPos);
}

void SpreadSlider::drawLinearSlider(juce::Graphics &g, int x, int y, int width,
                                    int height, float sliderPos,
                                    float spreadPos)
{
}

void SpreadSlider::drawRotarySlider(juce::Graphics &g, int x, int y, int width,
                                    int height, float sliderPos,
                                    float spreadPos)
{
    const auto rotaryParams     = getRotaryParameters();
    const auto rotaryStartAngle = rotaryParams.startAngleRadians;
    const auto rotaryEndAngle   = rotaryParams.endAngleRadians;

    auto outline = findColour(Slider::rotarySliderOutlineColourId);
    auto fill    = findColour(Slider::rotarySliderFillColourId);

    auto bounds =
        juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);

    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW     = juce::jmin(8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
                                arcRadius, arcRadius, 0.0f, rotaryStartAngle,
                                rotaryEndAngle, true);

    g.setColour(outline);
    g.strokePath(backgroundArc,
                 juce::PathStrokeType(lineW, juce::PathStrokeType::curved,
                                      juce::PathStrokeType::rounded));

    if (isEnabled()) {
        juce::Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
                               arcRadius, arcRadius, 0.0f, rotaryStartAngle,
                               toAngle, true);

        g.setColour(fill);
        g.strokePath(valueArc,
                     juce::PathStrokeType(lineW, juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));

        /* draw spread value */
        const auto spreadAdd  = std::atan(0.5f * lineW / arcRadius);
        auto spreadAngleStart = juce::jmax(
            rotaryStartAngle,
            rotaryStartAngle +
                (sliderPos - spreadPos) * (rotaryEndAngle - rotaryStartAngle) -
                spreadAdd);
        auto spreadAngleEnd = juce::jmin(
            rotaryEndAngle,
            rotaryStartAngle +
                (sliderPos + spreadPos) * (rotaryEndAngle - rotaryStartAngle) +
                spreadAdd);
        juce::Path spreadArc;
        spreadArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
                                arcRadius, arcRadius, 0.0f, spreadAngleStart,
                                spreadAngleEnd, true);
        g.setColour(juce::Colour(0xff9cbcbd));
        g.strokePath(spreadArc,
                     juce::PathStrokeType(lineW, juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    auto thumbWidth = lineW * 2.0f;
    juce::Point<float> thumbPoint(
        bounds.getCentreX() +
            arcRadius * std::cos(toAngle - juce::MathConstants<float>::halfPi),
        bounds.getCentreY() +
            arcRadius * std::sin(toAngle - juce::MathConstants<float>::halfPi));

    g.setColour(findColour(Slider::thumbColourId));
    g.fillEllipse(
        juce::Rectangle<float>(thumbWidth, thumbWidth).withCentre(thumbPoint));
}

void SpreadSlider::mouseDrag(const juce::MouseEvent &e)
{
    if (!e.mods.isCtrlDown()) {
        /* set spread from drag radius */
        auto &lf              = getLookAndFeel();
        auto layout           = lf.getSliderLayout(*this);
        const auto sliderRect = layout.sliderBounds;

        auto dx = e.position.x - (float)sliderRect.getCentreX();
        auto dy = e.position.y - (float)sliderRect.getCentreY();
        auto radius =
            juce::jmin(sliderRect.getWidth(), sliderRect.getHeight()) / 2.0f;

        auto dist = (std::sqrt(dx * dx + dy * dy) - radius) /
                    (radius * spreadIntensity);
        dist = juce::jmax(0.f, juce::jmin(1.f, dist));

        m_spreadAttachment.setNormalisedValue(dist);
    }

    /* set value from slider */
    if (!e.mods.isShiftDown()) {
        foleys::AutoOrientationSlider::mouseDrag(e);
    }
}
//====================================================================
SpreadSliderItem::SpreadSliderItem(foleys::MagicGUIBuilder &builder,
                                   const juce::ValueTree &node) :
    GuiItem(builder, node)
{
    setColourTranslation(
        {{"slider-background", juce::Slider::backgroundColourId},
         {"slider-thumb", juce::Slider::thumbColourId},
         {"slider-track", juce::Slider::trackColourId},
         {"rotary-fill", juce::Slider::rotarySliderFillColourId},
         {"rotary-outline", juce::Slider::rotarySliderOutlineColourId},
         {"slider-text", juce::Slider::textBoxTextColourId},
         {"slider-text-background", juce::Slider::textBoxBackgroundColourId},
         {"slider-text-highlight", juce::Slider::textBoxHighlightColourId},
         {"slider-text-outline", juce::Slider::textBoxOutlineColourId}});

    addAndMakeVisible(slider);
}

void SpreadSliderItem::update()
{
    attachment.reset();

    auto type = getProperty(pSliderType).toString();
    slider.setAutoOrientation(type.isEmpty() || type == pSliderTypes[0]);

    if (type == pSliderTypes[1])
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
    else if (type == pSliderTypes[2])
        slider.setSliderStyle(juce::Slider::LinearVertical);
    else if (type == pSliderTypes[3])
        slider.setSliderStyle(juce::Slider::Rotary);
    else if (type == pSliderTypes[4])
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    else if (type == pSliderTypes[5])
        slider.setSliderStyle(juce::Slider::IncDecButtons);

    auto textbox = getProperty(pSliderTextBox).toString();
    if (textbox == pTextBoxPositions[0])
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false,
                               slider.getTextBoxWidth(),
                               slider.getTextBoxHeight());
    else if (textbox == pTextBoxPositions[1])
        slider.setTextBoxStyle(juce::Slider::TextBoxAbove, false,
                               slider.getTextBoxWidth(),
                               slider.getTextBoxHeight());
    else if (textbox == pTextBoxPositions[3])
        slider.setTextBoxStyle(juce::Slider::TextBoxLeft, false,
                               slider.getTextBoxWidth(),
                               slider.getTextBoxHeight());
    else if (textbox == pTextBoxPositions[4])
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false,
                               slider.getTextBoxWidth(),
                               slider.getTextBoxHeight());
    else
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                               slider.getTextBoxWidth(),
                               slider.getTextBoxHeight());

    double minValue = getProperty(pMinValue);
    double maxValue = getProperty(pMaxValue);
    double interval = getProperty(pInterval);
    if (maxValue > minValue) slider.setRange(minValue, maxValue, interval);

    auto suffix = getProperty(pSuffix).toString();
    slider.setTextValueSuffix(suffix);

    auto valueID = configNode.getProperty(pValue, juce::String()).toString();
    if (valueID.isNotEmpty())
        slider.getValueObject().referTo(
            getMagicState().getPropertyAsValue(valueID));

    auto paramID = getProperty(foleys::IDs::parameter).toString();
    if (paramID.isNotEmpty())
        attachment = getMagicState().createAttachment(paramID, slider);

    auto spreadParamID = getProperty(pSpreadParameter).toString();
    if (spreadParamID.isNotEmpty())
        slider.setSpreadParameter(dynamic_cast<juce::RangedAudioParameter *>(
            getMagicState().getParameter(spreadParamID)));
}

std::vector<foleys::SettableProperty>
SpreadSliderItem::getSettableProperties() const
{
    std::vector<foleys::SettableProperty> props;

    props.push_back({configNode,
                     foleys::IDs::parameter,
                     foleys::SettableProperty::Choice,
                     {},
                     magicBuilder.createParameterMenuLambda()});
    props.push_back({configNode,
                     pSpreadParameter,
                     foleys::SettableProperty::Choice,
                     {},
                     magicBuilder.createParameterMenuLambda()});
    props.push_back({configNode, pSliderType, foleys::SettableProperty::Choice,
                     pSliderTypes[0],
                     magicBuilder.createChoicesMenuLambda(pSliderTypes)});
    props.push_back({configNode, pSliderTextBox,
                     foleys::SettableProperty::Choice, pTextBoxPositions[2],
                     magicBuilder.createChoicesMenuLambda(pTextBoxPositions)});
    props.push_back({configNode, pValue, foleys::SettableProperty::Choice, 1.0f,
                     magicBuilder.createPropertiesMenuLambda()});
    props.push_back(
        {configNode, pMinValue, foleys::SettableProperty::Number, 0.0f, {}});
    props.push_back(
        {configNode, pMaxValue, foleys::SettableProperty::Number, 2.0f, {}});
    props.push_back(
        {configNode, pInterval, foleys::SettableProperty::Number, 0.0f, {}});
    props.push_back(
        {configNode, pSuffix, foleys::SettableProperty::Text, {}, {}});

    return props;
}

const juce::Identifier SpreadSliderItem::pSliderType{"slider-type"};
const juce::StringArray SpreadSliderItem::pSliderTypes{
    "auto",   "linear-horizontal",          "linear-vertical",
    "rotary", "rotary-horizontal-vertical", "inc-dec-buttons"};
const juce::Identifier SpreadSliderItem::pSpreadParameter{"spread-parameter"};
const juce::Identifier SpreadSliderItem::pSliderTextBox{"slider-textbox"};
const juce::StringArray SpreadSliderItem::pTextBoxPositions{
    "no-textbox", "textbox-above", "textbox-below", "textbox-left",
    "textbox-right"};
const juce::Identifier SpreadSliderItem::pValue{"value"};
const juce::Identifier SpreadSliderItem::pMinValue{"min-value"};
const juce::Identifier SpreadSliderItem::pMaxValue{"max-value"};
const juce::Identifier SpreadSliderItem::pInterval{"interval"};
const juce::Identifier SpreadSliderItem::pSuffix{"suffix"};
