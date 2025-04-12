#include <juce_gui_basics/juce_gui_basics.h>

#include "ArrowButton.h"

class ComboBox : public juce::Component
{
  public:
    ComboBox() : m_left{"prev", 0.5}, m_right{"next", 0}
    {
        addAndMakeVisible(m_comboBox);
        addAndMakeVisible(m_left);
        addAndMakeVisible(m_right);

        m_comboBox.onChange = [this] {
            defaultCallback();
        };

        m_left.onClick = [this]() {
            const auto numItems = m_comboBox.getNumItems();
            m_comboBox.setSelectedId(
                ((m_selected - 1) + numItems - 1) % numItems + 1);
        };
        m_right.onClick = [this]() {
            const auto numItems = m_comboBox.getNumItems();
            m_comboBox.setSelectedId(((m_selected - 1) + 1) % numItems + 1);
        };
    }

    void defaultCallback() { m_selected = m_comboBox.getSelectedId(); }

    void resized() override
    {
        constexpr auto arrowSize = 13;
        auto bounds              = getLocalBounds();
        m_left.setBounds(bounds.removeFromLeft(arrowSize));
        m_right.setBounds(bounds.removeFromRight(arrowSize));
        m_comboBox.setBounds(bounds);
    }

    juce::ComboBox &getComboBox() { return m_comboBox; }

    void setArrowsColour(juce::Colour colour)
    {
        m_left.setColour(colour);
        m_right.setColour(colour);
    }

  private:
    ArrowButton m_left;
    ArrowButton m_right;
    juce::ComboBox m_comboBox;
    int m_selected{0};
};
