#include <juce_gui_basics/juce_gui_basics.h>

class ComboBox : public juce::Component
{
  public:
    ComboBox(juce::Colour colour) :
        m_left{"prev", 0.5, colour}, m_right{"next", 0, colour}
    {
        addAndMakeVisible(m_comboBox);
        addAndMakeVisible(m_left);
        addAndMakeVisible(m_right);

        m_left.onClick = [this]() {
            const auto numItems = m_comboBox.getNumItems();
            m_comboBox.setSelectedId(
                ((m_comboBox.getSelectedId() - 1) + numItems - 1) % numItems +
                1);
        };
        m_right.onClick = [this]() {
            const auto numItems = m_comboBox.getNumItems();
            m_comboBox.setSelectedId(
                ((m_comboBox.getSelectedId() - 1) + 1) % numItems + 1);
        };
    }

    void resized() override
    {
        constexpr auto arrowSize = 13;
        auto bounds              = getLocalBounds();
        m_left.setBounds(bounds.removeFromLeft(arrowSize));
        m_right.setBounds(bounds.removeFromRight(arrowSize));
        m_comboBox.setBounds(bounds);
    }

    juce::ComboBox &getComboBox() { return m_comboBox; }

  private:
    juce::ArrowButton m_left;
    juce::ArrowButton m_right;
    juce::ComboBox m_comboBox;
};
