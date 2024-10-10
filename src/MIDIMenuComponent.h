#pragma once

#include <string>

#include "MIDISelectComponent.h"

//==============================================================================
struct MIDIMenuComponent : public juce::Component, public juce::Timer
{
    MIDIMenuComponent()
    {
        WestEditor.setTopLeftPosition(5,5);
        addAndMakeVisible(WestEditor);
        
        EastEditor.setTopLeftPosition(30,5);
        addAndMakeVisible(EastEditor);
        
        NorthEditor.setTopLeftPosition(55,5);
        addAndMakeVisible(NorthEditor);
        
        SouthEditor.setTopLeftPosition(80,5);
        addAndMakeVisible(SouthEditor);
        
        HomeEditor.setTopLeftPosition(105,5);
        addAndMakeVisible(HomeEditor);
        
        ChannelEditor.setTopLeftPosition(130,5);
        addAndMakeVisible(ChannelEditor);
        
        startTimer(5);
    }
    
    void paint(juce::Graphics &g) override
    {
        g.setColour(juce::Colours::thistle);
        g.fillRect(this->getLocalBounds());
        
        g.setColour(juce::Colours::royalblue);
        g.drawRect(this->getLocalBounds());
    }
    
    void timerCallback() override
    {
        if (WestEditor.settingChanged == true)
        {
            westCC = WestEditor.midiCC;
            anyMidiChanged = true;
            WestEditor.settingChanged = false;
        }
        
        if (EastEditor.settingChanged == true)
        {
            westCC = EastEditor.midiCC;
            anyMidiChanged = true;
            EastEditor.settingChanged = false;
        }
        
        if (NorthEditor.settingChanged == true)
        {
            westCC = NorthEditor.midiCC;
            anyMidiChanged = true;
            NorthEditor.settingChanged = false;
        }
        
        if (SouthEditor.settingChanged == true)
        {
            westCC = SouthEditor.midiCC;
            anyMidiChanged = true;
            SouthEditor.settingChanged = false;
        }
        
        if (HomeEditor.settingChanged == true)
        {
            westCC = HomeEditor.midiCC;
            anyMidiChanged = true;
            HomeEditor.settingChanged = false;
        }
        
        if (ChannelEditor.settingChanged == true)
        {
            westCC = ChannelEditor.midiCC;
            anyMidiChanged = true;
            ChannelEditor.settingChanged = false;
        }
    }
    
    std::atomic<bool> anyMidiChanged = false;
    int westCC, eastCC, northCC, southCC, homeCC, midiChannel;
    
private:
    //    juce::Label modeLabel{ {}, "Mode" };
    
    std::unique_ptr<juce::Timer> t;

    MIDISelectComponent WestEditor{"West CC"};
    MIDISelectComponent EastEditor{"East CC"};
    MIDISelectComponent NorthEditor{"North CC"};
    MIDISelectComponent SouthEditor{"South CC"};
    MIDISelectComponent HomeEditor{"Home CC"};
    MIDISelectComponent ChannelEditor{"Channel", true};
};


