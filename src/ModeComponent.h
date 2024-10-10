#pragma once

//==============================================================================
struct ModeComponent : public juce::ToggleButton
{
    ModeComponent(int m)
    {
        pythButton.setBounds(5,5,110,35);
        addAndMakeVisible(pythButton);
        pythButton.onClick = [this]{ updateToggleState(&pythButton, "Pythagorean"); };
        pythButton.setClickingTogglesState(true);
        pythButton.setRadioGroupId(1);
        
        syntonicButton.setBounds(5,45,110,35);
        addAndMakeVisible(syntonicButton);
        syntonicButton.onClick = [this]{ updateToggleState(&syntonicButton, "Syntonic"); };
        syntonicButton.setClickingTogglesState(true);
        syntonicButton.setRadioGroupId(1);
        
        duodeneButton.setBounds(5,85,110,35);
        addAndMakeVisible(duodeneButton);
        duodeneButton.onClick = [this]{ updateToggleState(&duodeneButton, "Duodene"); };
        duodeneButton.setClickingTogglesState(true);
        duodeneButton.setRadioGroupId(1);
        
        switch (m)
        {
            case LatticesProcessor::Pyth:
                pythButton.setToggleState(true, juce::dontSendNotification);
                break;
            case LatticesProcessor::Syntonic:
                syntonicButton.setToggleState(true, juce::dontSendNotification);
                break;
            case LatticesProcessor::Duodene:
                duodeneButton.setToggleState(true, juce::dontSendNotification);
                break;
            default:
                duodeneButton.setToggleState(true, juce::dontSendNotification);
                break;
        }
    }
    
    void updateToggleState (juce::Button* button, juce::String name)
    {
        modeChanged = true;
    }
    
    void paint(juce::Graphics &g) override
    {
        g.setColour(juce::Colours::darkolivegreen);
        g.fillRect(this->getLocalBounds());
        
        g.setColour(juce::Colours::chartreuse);
        g.drawRect(this->getLocalBounds());
        
        
        // why don't I need to draw these???
//        pythButton.paintButton(g, false, false);
//        syntonicButton.paintButton(g, false, false);
//        duodeneButton.paintButton(g, false, false);
    }
    
    int whichMode()
    {
        if (duodeneButton.getToggleState() == true)
        {
            return 2;
        }
        if (syntonicButton.getToggleState() == true)
        {
            return 1;
        }
        if (pythButton.getToggleState() == true)
        {
            return 0;
        }
        std::cout << "something went wrong" << std::endl;
        return 0;
    }
    
    bool modeChanged = false;
    
private:
    //    juce::Label modeLabel{ {}, "Mode" };
    juce::TextButton pythButton { "Pythagorean" };
    juce::TextButton syntonicButton { "Syntonic" };
    juce::TextButton duodeneButton { "Duodene" };
};


