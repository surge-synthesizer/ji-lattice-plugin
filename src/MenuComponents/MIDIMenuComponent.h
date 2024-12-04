/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do note that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#pragma once

#include <string>

//==============================================================================
struct MIDIMenuComponent : public juce::Component
{
    MIDIMenuComponent(int hCC, int C)
    {
        homeCC = hCC;
        midiChannel = C;

        addAndMakeVisible(homeLabel);
        homeLabel.setJustificationType(juce::Justification::left);
        homeLabel.setColour(juce::Label::backgroundColourId, bg);
        homeLabel.setColour(juce::Label::outlineColourId, ol);

        addAndMakeVisible(homeEditor);
        homeEditor.setMultiLine(false);
        homeEditor.setReturnKeyStartsNewLine(false);
        homeEditor.setInputRestrictions(3, "1234567890");
        homeEditor.setText(std::to_string(hCC), false);
        homeEditor.setJustification(juce::Justification::centred);
        homeEditor.setSelectAllWhenFocused(true);
        //        homeEditor.setColour(juce::TextEditor::outlineColourId, noColour);
        homeEditor.onReturnKey = [this] { returnKeyResponse(&homeEditor); };
        homeEditor.onEscapeKey = [this] { escapeKeyResponse(&homeEditor); };
        homeEditor.onFocusLost = [this] { focusLostResponse(&homeEditor); };

        addAndMakeVisible(channelLabel);
        channelLabel.setJustificationType(juce::Justification::left);
        channelLabel.setColour(juce::Label::backgroundColourId, bg);
        channelLabel.setColour(juce::Label::outlineColourId, ol);

        addAndMakeVisible(channelEditor);
        channelEditor.setMultiLine(false);
        channelEditor.setReturnKeyStartsNewLine(false);
        channelEditor.setInputRestrictions(2, "1234567890");
        channelEditor.setText(std::to_string(C), false);
        channelEditor.setJustification(juce::Justification::centred);
        channelEditor.setSelectAllWhenFocused(true);
        //        channelEditor.setColour(juce::TextEditor::outlineColourId, noColour);
        channelEditor.onReturnKey = [this] { returnKeyResponse(&channelEditor); };
        channelEditor.onEscapeKey = [this] { escapeKeyResponse(&channelEditor); };
        channelEditor.onFocusLost = [this] { focusLostResponse(&channelEditor); };
    }

    ~MIDIMenuComponent() {}

    void paint(juce::Graphics &g) override
    {
        g.setColour(bg);
        g.fillRect(this->getLocalBounds());

        g.setColour(ol);
        g.drawRect(this->getLocalBounds());
    }

    void resized() override
    {
        homeLabel.setBounds(10, 105, 70, 20);
        channelLabel.setBounds(10, 130, 70, 20);

        homeEditor.setBounds(80, 105, 30, 20);
        channelEditor.setBounds(80, 130, 30, 20);
    }

    std::atomic<bool> settingChanged = false;
    int midiChannel;
    int homeCC;

  private:
    juce::TextEditor homeEditor{"Home"};
    juce::TextEditor channelEditor{"Channel"};

    juce::Label homeLabel{{}, "Home CC"};
    juce::Label channelLabel{{}, "Channel"};

    //    juce::Colour noColour{};
    juce::Colour bg{.475f, .5f, 0.2f, 1.f};
    juce::Colour ol{juce::Colours::ghostwhite};
    juce::Range<int> noRange{};

    bool rejectBadInput(int input, bool channel = false)
    {
        // if it's midi channel, reject if out of range

        if (channel)
        {
            if (input < 1 || input > 16)
            {
                return true;
            }
            return false;
        }

        // if it's a CC, reject if out of range or if already occupied
        if (input < 1 || input > 127)
        {
            return true;
        }

        return false;
    }

    void returnKeyResponse(juce::TextEditor *e)
    {
        e->setHighlightedRegion(noRange);
        this->unfocusAllComponents();

        int digit = e->getText().getIntValue();

        if (e == &homeEditor)
        {
            if (rejectBadInput(digit))
            {
                e->setText(std::to_string(homeCC));
                return;
            }

            homeCC = digit;
            settingChanged = true;
        }

        if (e == &channelEditor)
        {
            if (rejectBadInput(digit, true))
            {
                e->setText(std::to_string(midiChannel));
                return;
            }

            midiChannel = digit;
            settingChanged = true;
        }
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
};
