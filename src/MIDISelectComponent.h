#pragma once

#include <string>

//==============================================================================
struct MIDISelectComponent : public juce::TextEditor, public juce::TextEditor::Listener
{
    MIDISelectComponent(std::string name, bool isChannelSelector = false)
    {
        TextEditor textEditor(name);
        
        if (isChannelSelector == true)
        {
            channel = true;
        }
        
        textEditor.setSize(80,20);
        textEditor.setMultiLine(false);
        textEditor.setInputRestrictions(3, "1234567890");
        addAndMakeVisible(textEditor);
        textEditor.addListener(this);
    }
    
    MIDISelectComponent()
    {
        textEditor.removeListener(this);
    }
    
    void textEditorReturnKeyPressed(TextEditor &e) override
    {
        midiCC = e.getText().getIntValue();
        
        if (channel != true)
        {
            midiCC = std::clamp(midiCC, 1, 127);
        }
        else
        {
            midiCC = std::clamp(midiCC, 1, 16);
        }
        
        settingChanged = true;
    }
    
    std::atomic<bool> settingChanged = false;
    int midiCC;
    
private:
    //    juce::Label modeLabel{ {}, "Mode" };
    TextEditor textEditor;
    bool channel{false};

};


