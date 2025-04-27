#include <juce_gui_basics/juce_gui_basics.h>

#include "ArrowButton.h"

namespace aether
{

class ComboBox : public juce::Component
{
  public:
    ComboBox(const juce::String &name = {}) :
        left_{"prev", 0.5}, right_{"next", 0}, comboBox_{name}
    {
        addAndMakeVisible(comboBox_);
        addAndMakeVisible(left_);
        addAndMakeVisible(right_);

        comboBox_.onChange = [this] {
            defaultCallback();
        };

        left_.onClick = [this]() {
            const auto numItems = comboBox_.getNumItems();
            comboBox_.setSelectedId(
                ((selected_ - 1) + numItems - 1) % numItems + 1);
        };
        right_.onClick = [this]() {
            const auto numItems = comboBox_.getNumItems();
            comboBox_.setSelectedId(((selected_ - 1) + 1) % numItems + 1);
        };
    }

    void defaultCallback()
    {
        auto selected = comboBox_.getSelectedId();
        if (selected != 0) selected_ = selected;
    }

    void resized() override
    {
        constexpr auto kArrowSize = 13;
        auto bounds               = getLocalBounds();
        left_.setBounds(bounds.removeFromLeft(kArrowSize));
        right_.setBounds(bounds.removeFromRight(kArrowSize));
        comboBox_.setBounds(bounds);
    }

    juce::ComboBox &getComboBox() { return comboBox_; }

    void setArrowsColour(juce::Colour colour)
    {
        left_.setColour(colour);
        right_.setColour(colour);
    }

  private:
    ArrowButton left_;
    ArrowButton right_;
    juce::ComboBox comboBox_;
    int selected_{0};
};

} // namespace aether
