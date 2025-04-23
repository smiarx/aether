#pragma once

#include "CustomLNF.h"
#include "Slider.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

using APVTS = juce::AudioProcessorValueTreeState;

template <class Comp> class Widget : public juce::Component
{
  public:
    static constexpr auto kTextSize    = CustomLNF::kTextPointHeight;
    static constexpr auto kLabelMargin = -3;

    template <class... Ts>
    Widget(const juce::String &name, Ts &&...args) : component_(args...)
    {
        addAndMakeVisible(component_);
        addAndMakeVisible(label_);

        component_.setTitle(name);
        label_.setText(name.toLowerCase(),
                       juce::NotificationType::dontSendNotification);
        label_.setJustificationType(juce::Justification::centred);

        auto font =
            juce::Font(CustomLNF::defaultTypeface).withPointHeight(kTextSize);
        label_.setFont(font);
    }
    void resized() override
    {
        auto bounds = getLocalBounds();

        if (labelVisible_) {
            auto textBox = bounds.removeFromBottom(
                kLabelMargin + label_.getFont().getHeight());
            textBox.removeFromTop(kLabelMargin);
            label_.setBounds(textBox);
        }
        component_.setBounds(bounds);
    }

    void setLabelVisible(bool labelVisible)
    {
        labelVisible_ = labelVisible;
        if (labelVisible_) {
            addAndMakeVisible(label_);
        } else {
            removeChildComponent(&label_);
        }
    }
    [[nodiscard]] bool isLabelVisible() const { return labelVisible_; }

    Comp &getComponent() { return component_; }
    auto &getLabel() { return label_; }

    void setColour(int colourID, juce::Colour colour)
    {
        component_.setColour(colourID, colour);
    }

  protected:
    Comp component_;
    juce::Label label_;

  private:
    bool labelVisible_{true};
};

class SliderWithLabel : public Widget<Slider>
{
  public:
    SliderWithLabel(APVTS &apvts, const juce::String &id,
                    const juce::String &name) :
        Widget<Slider>(name), attachment_(apvts, id, component_)
    {
        component_.setSliderStyle(
            juce::Slider::SliderStyle::RotaryVerticalDrag);
        component_.setTextBoxStyle(
            juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
    }

    int getMaxHeight()
    {
        int removeFromHeight = 0;
        if (isLabelVisible()) {
            removeFromHeight = kLabelMargin + label_.getFont().getHeight();
        }
        auto bounds = getLocalBounds();
        auto size   = juce::jmin(bounds.getHeight() - removeFromHeight,
                                 bounds.getWidth());
        size += removeFromHeight;
        return size;
    }

    void resized() override
    {
        // set height to maximum possible value
        auto maxHeight = getMaxHeight();
        auto bounds    = getBounds();
        setBounds(bounds.withHeight(maxHeight).withCentre(bounds.getCentre()));

        // call parent
        Widget<Slider>::resized();

        // update width to be equal to height
        bounds = component_.getLocalBounds();
        component_.setBounds(bounds.withWidth(bounds.getHeight())
                                 .withCentre(bounds.getCentre()));
    }

    void setTextBoxVisible(bool textBoxVisible)
    {
        component_.setTextBoxStyle(
            textBoxVisible ? juce::Slider::TextEntryBoxPosition::TextBoxBelow
                           : juce::Slider::NoTextBox,
            true, 100, kTextSize);
    }

    void setValueAsLabel()
    {
        auto font = juce::Font(CustomLNF::defaultMonoTypeface)
                        .withPointHeight(kTextSize - 1);
        label_.setFont(font);

        component_.setPopupDisplayEnabled(false, false, nullptr);

        component_.onValueChange = [this] {
            label_.setText(component_.getTextFromValue(component_.getValue()),
                           juce::NotificationType::dontSendNotification);
        };
    }

    juce::Slider &getSlider() { return getComponent(); }
    APVTS::SliderAttachment &getAttachment() { return attachment_; }

  private:
    APVTS::SliderAttachment attachment_;
};

} // namespace aether
