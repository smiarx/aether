#include "PluginEditor.h"
#include "BinaryData.h"

namespace aether
{

PluginEditor::PluginEditor(PluginProcessor &p) :
    AudioProcessorEditor(&p), preset(p.getPresetManager()),
    delaySection{p, p.getAPVTS()}, springsSection{p, p.getAPVTS()}
{
    setResizable(true, true);

    setInterceptsMouseClicks(false, true);
    juce::Desktop::getInstance().addGlobalMouseListener(this);

    setLookAndFeel(&lookandfeel);
    addAndMakeVisible(title);
    addAndMakeVisible(preset);
    addAndMakeVisible(tooltip);
    addAndMakeVisible(delaySection);
    addAndMakeVisible(springsSection);

    setSize(940, 480);
    setResizeLimits(600, 400, std::numeric_limits<int>::max(),
                    std::numeric_limits<int>::max());

    title.setText(juce::String::fromUTF8(titleString),
                  juce::dontSendNotification);
    auto titleFont =
        juce::Font(CustomLNF::titleTypeface).withHeight(headerHeight);
    titleFont.setDefaultMinimumHorizontalScaleFactor(1.f);
    titleFont.setExtraKerningFactor(0.3f);
    title.setFont(titleFont);
    title.setColour(juce::Label::textColourId, juce::Colour{0xffffffff});

    // fix width
    titleWidth =
        titleFont.getStringWidthFloat(juce::String::fromUTF8(titleString));
    titleWidth += 0.08f * titleWidth;

    tooltip.setFont(juce::Font(CustomLNF::defaultTypeface).withPointHeight(14));
    tooltip.setColour(juce::Label::textColourId, juce::Colour{0xffffffff});
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
    fbTitle.alignItems     = juce::FlexBox::AlignItems::flexEnd;
    fbTitle.alignContent   = juce::FlexBox::AlignContent::center;
    fbTitle.justifyContent = juce::FlexBox::JustifyContent::center;
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::row;

    fbTitle.items.addArray({
        juce::FlexItem(title)
            .withFlex(0.5f)
            .withMargin(0.f)
            .withHeight(headerHeight)
            .withMaxWidth(titleWidth)
            .withMinWidth(titleWidth),
        juce::FlexItem(tooltip).withFlex(1.f).withMargin(0.f).withHeight(
            headerHeight),
        juce::FlexItem(preset)
            .withFlex(0.55f)
            .withMargin({0.f, 0.f, 5.f, 0.f})
            .withHeight(headerHeight / 2)
            .withMaxWidth(130),
    });

    fb.items.addArray({
        juce::FlexItem(delaySection).withFlex(0.68f).withMargin(0.f),
        juce::FlexItem(springsSection).withFlex(1.f).withMargin(0.f),
    });

    fbMain.items.addArray({
        juce::FlexItem(fbTitle)
            .withFlex(0.f)
            .withHeight(headerHeight)
            .withMargin({0, 0, 0, titleMargin}),
        juce::FlexItem(fb).withFlex(1.f),
    });

    auto bounds = getLocalBounds();
    bounds.reduce(CustomLNF::margin, 0);
    bounds.removeFromBottom(CustomLNF::margin);
    fbMain.performLayout(bounds);
}

void PluginEditor::paint(juce::Graphics &g)
{
    g.fillAll(findColour(juce::ResizableWindow::backgroundColourId));

    juce::DropShadow shadow(juce::Colour{0x8f000000}, 5, {-2, 8});
    juce::Path win;
    win.addRoundedRectangle(
        delaySection.getBounds().getUnion(springsSection.getBounds()),
        CustomLNF::boxRoundSize);
    shadow.drawForPath(g, win);

    g.setColour(juce::Colour{0xffafafaf});
    g.strokePath(win, juce::PathStrokeType(2));
}

void PluginEditor::mouseMove(const juce::MouseEvent &event)
{
    juce::AudioProcessorEditor::mouseMove(event);

    const auto mouseSource = juce::Desktop::getInstance().getMainMouseSource();
    auto *underMouse =
        mouseSource.isTouch() ? nullptr : mouseSource.getComponentUnderMouse();

    tooltip.setFromComponent(underMouse);
}

} // namespace aether
