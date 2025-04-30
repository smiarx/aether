#include "PluginEditor.h"
#include "../PluginProcessor.h"
#include "CustomLNF.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <limits>

namespace aether
{

PluginEditor::PluginEditor(PluginProcessor &p) :
    AudioProcessorEditor(&p), preset_(p.getPresetManager()), delaySection_{p},
    springsSection_{p}
{
    setResizable(true, true);

    setInterceptsMouseClicks(false, true);
    juce::Desktop::getInstance().addGlobalMouseListener(this);

    juce::LookAndFeel::setDefaultLookAndFeel(&lookandfeel_);
    addAndMakeVisible(tooltip_);
    addAndMakeVisible(preset_);
    addAndMakeVisible(delaySection_);
    addAndMakeVisible(springsSection_);

    constexpr auto kWidth  = 720;
    constexpr auto kHeight = 400;
    setSize(kWidth, kHeight);
    setResizeLimits(kWidth, kHeight, std::numeric_limits<int>::max(),
                    std::numeric_limits<int>::max());

    preset_.setArrowsColour(juce::Colour(0xffffffff));
}

PluginEditor::~PluginEditor()
{
    setLookAndFeel(nullptr);
    juce::Desktop::getInstance().removeGlobalMouseListener(this);
}

void PluginEditor::resized()
{
    juce::FlexBox fbMain;
    fbMain.flexDirection = juce::FlexBox::Direction::column;

    juce::FlexBox fbTitle;
    fbTitle.flexDirection  = juce::FlexBox::Direction::row;
    fbTitle.alignItems     = juce::FlexBox::AlignItems::center;
    fbTitle.alignContent   = juce::FlexBox::AlignContent::center;
    fbTitle.justifyContent = juce::FlexBox::JustifyContent::center;
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;

    juce::Component flextitle;
    auto titleWidth    = title_.getMaxWidth();
    auto toolTipMargin = getBounds().toFloat().getWidth() * 0.03f;

    fbTitle.items.addArray({
        juce::FlexItem(flextitle)
            .withFlex(0.5f)
            .withMargin(0.f)
            .withHeight(kHeaderHeight)
            .withMaxWidth(titleWidth)
            .withMinWidth(titleWidth),
        juce::FlexItem(tooltip_)
            .withFlex(1.f)
            .withMargin({0, toolTipMargin, 0, toolTipMargin})
            .withHeight(kHeaderHeight),
        juce::FlexItem(preset_)
            .withFlex(0.55f)
            .withHeight(kHeaderHeight / 2)
            .withMaxWidth(130),
    });

    fb.items.addArray({
        juce::FlexItem(delaySection_).withFlex(0.68f).withMargin(0.f),
        juce::FlexItem(springsSection_).withFlex(1.f).withMargin(0.f),
    });

    fbMain.items.addArray({
        juce::FlexItem(fbTitle)
            .withFlex(0.f)
            .withHeight(kHeaderHeight)
            .withMargin({0, 0, 0, kTitleMargin}),
        juce::FlexItem(fb).withFlex(1.f),
    });

    auto bounds = getLocalBounds();
    bounds.reduce(CustomLNF::kMargin, 0);
    bounds.removeFromBottom(CustomLNF::kMargin);
    fbMain.performLayout(bounds);

    title_.setBounds(flextitle.getBounds().toFloat());
}

void PluginEditor::paint(juce::Graphics &g)
{
    g.fillAll(findColour(juce::ResizableWindow::backgroundColourId));

    juce::DropShadow shadow(juce::Colour{0x8f000000}, 5, {-2, 8});
    juce::Path win;
    win.addRoundedRectangle(
        delaySection_.getBounds().getUnion(springsSection_.getBounds()),
        CustomLNF::kBoxRoundSize);
    shadow.drawForPath(g, win);

    g.setColour(juce::Colour{0xffafafaf});
    g.strokePath(win, juce::PathStrokeType(2));

    title_.draw(g);
}

void PluginEditor::mouseMove(const juce::MouseEvent &event)
{
    juce::AudioProcessorEditor::mouseMove(event);

    const auto mouseSource = juce::Desktop::getInstance().getMainMouseSource();
    auto *underMouse =
        mouseSource.isTouch() ? nullptr : mouseSource.getComponentUnderMouse();

    tooltip_.setFromComponent(underMouse);
}

} // namespace aether
