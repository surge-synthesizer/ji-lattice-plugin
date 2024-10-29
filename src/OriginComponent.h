#pragma once

#include <string>

//==============================================================================
struct OriginComponent : public juce::Component
{
    OriginComponent(int o, double f)
    {
        keyshape.addRectangle(keysize);
        
        for (int i = 0; i < 12; ++i)
        {
            if (kb[i])
            {
                key.add(new juce::ShapeButton(std::to_string(i), b1, b2, sc));
            }
            else
            {
                key.add(new juce::ShapeButton(std::to_string(i), w1, w2, sc));
            }
            
            key[i]->setShape(keyshape,true,true,false);
            addAndMakeVisible(key[i]);
            key[i]->setRadioGroupId(1);
            key[i]->onClick = [this]{ updateOrigin(); };
            key[i]->setClickingTogglesState(true);
            key[i]->setOutline(juce::Colours::lightgrey, 1.5f);
            key[i]->shouldUseOnColours(true);
            key[i]->setOnColours(sc,so,sc);
            
            if (o == i)
            {
                key[i]->setToggleState(true, juce::dontSendNotification);
            }
            // prob not needed?
//            else
//            {
//                key[i]setToggleState(false, juce::dontSendNotification);
//            }
        }
        
        addAndMakeVisible(freqEditor);
        freqEditor.setMultiLine(false);
        freqEditor.setReturnKeyStartsNewLine(false);
        freqEditor.setInputRestrictions(17, ".1234567890");
        freqEditor.setText(std::to_string(f), false);
        freqEditor.setJustification(juce::Justification::left);
        freqEditor.setSelectAllWhenFocused(true);
        freqEditor.setColour(juce::TextEditor::outlineColourId, noColour);
        freqEditor.onReturnKey = [this]{ returnKeyResponse(&freqEditor); };
        freqEditor.onEscapeKey = [this]{ escapeKeyResponse(&freqEditor); };
        freqEditor.onFocusLost = [this]{ focusLostResponse(&freqEditor); };
        
        addAndMakeVisible(freqLabel);
        freqLabel.setJustificationType(juce::Justification::left);
        freqLabel.setColour(juce::Label::backgroundColourId, findColour(juce::TextEditor::backgroundColourId));
    }
    
    
    void resized() override
    {
        for (int i = 0; i < 12; ++i)
        {
            key[i]->setBounds(kw * i, 0, kw, kh);
        }
        
        freqLabel.setBounds(0, kh, 108, 30);
        freqEditor.setBounds(kw * 6, kh, 108, 30);
    }
    
    void updateOrigin()
    {
        originChanged = true;
    }
    
    void paint(juce::Graphics &g) override
    {
        g.setColour(findColour(juce::TextEditor::backgroundColourId));
        g.fillRect(this->getLocalBounds());
        
//        g.setColour(juce::Colours::thistle);
//        g.drawRect(this->getLocalBounds());
    }
    
    int whichNote()
    {
        for (int i = 0; i < 12; ++i)
        {
            if (key[i]->getToggleState() == true)
            {
                originChanged = false;
                return i;
            }
        }
        originChanged = false;
        return 0;
    }
    
    bool originChanged = false;
    
    double whatFreq{293.3333333333333};
    
private:
    static constexpr int kw = 18;
    static constexpr int kh = 65;
    
    juce::Label freqLabel{{}, "Ref. Frequency = " };
    
    juce::OwnedArray<juce::ShapeButton> key;
    
    juce::Rectangle<int> keysize = juce::Rectangle(kw, kh);
    juce::Path keyshape;
    
    std::array<bool, 12> kb = {false,true,false,true,false,false,true,false,true,false,true,false};
    
    juce::Colour w1 = juce::Colours::white;
    juce::Colour w2 = juce::Colours::antiquewhite;
    juce::Colour b1 = juce::Colours::black;
    juce::Colour b2 = juce::Colours::darkgrey;
    juce::Colour sc = juce::Colours::darkviolet;
    juce::Colour so = juce::Colours::blueviolet;
    
    juce::TextEditor freqEditor{"Ref Freq"};
    
    juce::Range<int> noRange{};
    juce::Colour noColour{};
    
    void returnKeyResponse(juce::TextEditor *e)
    {
        e->setHighlightedRegion(noRange);
        this->unfocusAllComponents();
        auto input = e->getText().getDoubleValue();
        
        if (rejectBadInput(input))
        {
            e->setText(std::to_string(whatFreq));
            return;
        }
        
        whatFreq = input;
    }
    
    void escapeKeyResponse(juce::TextEditor *e)
    {
        e->setHighlightedRegion(noRange);
        this->unfocusAllComponents();
    }
    
    void focusLostResponse(juce::TextEditor *e)
    {
        e->setHighlightedRegion(noRange);
        this->unfocusAllComponents();
    }
    
    bool rejectBadInput(double input)
    {
        if (input == 0)
        {
            return true;
        }
        
        return false;
    }
};
