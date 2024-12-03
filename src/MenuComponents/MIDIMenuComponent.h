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
    MIDIMenuComponent(int wCC, int eCC, int nCC, int sCC, int hCC, int C)
    {
        data[0] = wCC;
        data[1] = eCC;
        data[2] = nCC;
        data[3] = sCC;
        data[4] = hCC;
        midiChannel = C;
        
        addAndMakeVisible(westLabel);
        westLabel.setJustificationType(juce::Justification::left);
        westLabel.setColour(juce::Label::backgroundColourId, bg);
        westLabel.setColour(juce::Label::outlineColourId, ol);

        addAndMakeVisible(westEditor);
        westEditor.setMultiLine(false);
        westEditor.setReturnKeyStartsNewLine(false);
        westEditor.setInputRestrictions(3, "1234567890");
        westEditor.setText(std::to_string(wCC), false);
        westEditor.setJustification(juce::Justification::centred);
        westEditor.setSelectAllWhenFocused(true);
        //        westEditor.setColour(juce::TextEditor::outlineColourId, noColour);
        westEditor.onReturnKey = [this] { returnKeyResponse(&westEditor); };
        westEditor.onEscapeKey = [this] { escapeKeyResponse(&westEditor); };
        westEditor.onFocusLost = [this] { focusLostResponse(&westEditor); };

        addAndMakeVisible(eastLabel);
        eastLabel.setJustificationType(juce::Justification::left);
        eastLabel.setColour(juce::Label::backgroundColourId, bg);
        eastLabel.setColour(juce::Label::outlineColourId, ol);

        addAndMakeVisible(eastEditor);
        eastEditor.setMultiLine(false);
        eastEditor.setReturnKeyStartsNewLine(false);
        eastEditor.setInputRestrictions(3, "1234567890");
        eastEditor.setText(std::to_string(eCC), false);
        eastEditor.setJustification(juce::Justification::centred);
        eastEditor.setSelectAllWhenFocused(true);
        //        eastEditor.setColour(juce::TextEditor::outlineColourId, noColour);
        eastEditor.onReturnKey = [this] { returnKeyResponse(&eastEditor); };
        eastEditor.onEscapeKey = [this] { escapeKeyResponse(&eastEditor); };
        eastEditor.onFocusLost = [this] { focusLostResponse(&eastEditor); };

        addAndMakeVisible(northLabel);
        northLabel.setJustificationType(juce::Justification::left);
        northLabel.setColour(juce::Label::backgroundColourId, bg);
        northLabel.setColour(juce::Label::outlineColourId, ol);

        addAndMakeVisible(northEditor);
        northEditor.setMultiLine(false);
        northEditor.setReturnKeyStartsNewLine(false);
        northEditor.setInputRestrictions(3, "1234567890");
        northEditor.setText(std::to_string(nCC), false);
        northEditor.setJustification(juce::Justification::centred);
        northEditor.setSelectAllWhenFocused(true);
        //        northEditor.setColour(juce::TextEditor::outlineColourId, noColour);
        northEditor.onReturnKey = [this] { returnKeyResponse(&northEditor); };
        northEditor.onEscapeKey = [this] { escapeKeyResponse(&northEditor); };
        northEditor.onFocusLost = [this] { focusLostResponse(&northEditor); };

        addAndMakeVisible(southLabel);
        southLabel.setJustificationType(juce::Justification::left);
        southLabel.setColour(juce::Label::backgroundColourId, bg);
        southLabel.setColour(juce::Label::outlineColourId, ol);

        addAndMakeVisible(southEditor);
        southEditor.setMultiLine(false);
        southEditor.setReturnKeyStartsNewLine(false);
        southEditor.setInputRestrictions(3, "1234567890");
        southEditor.setText(std::to_string(sCC), false);
        southEditor.setJustification(juce::Justification::centred);
        southEditor.setSelectAllWhenFocused(true);
        //        southEditor.setColour(juce::TextEditor::outlineColourId, noColour);
        southEditor.onReturnKey = [this] { returnKeyResponse(&southEditor); };
        southEditor.onEscapeKey = [this] { escapeKeyResponse(&southEditor); };
        southEditor.onFocusLost = [this] { focusLostResponse(&southEditor); };

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
        westLabel.setBounds(10, 5, 70, 20);
        eastLabel.setBounds(10, 30, 70, 20);
        northLabel.setBounds(10, 55, 70, 20);
        southLabel.setBounds(10, 80, 70, 20);
        homeLabel.setBounds(10, 105, 70, 20);
        channelLabel.setBounds(10, 130, 70, 20);

        westEditor.setBounds(80, 5, 30, 20);
        eastEditor.setBounds(80, 30, 30, 20);
        northEditor.setBounds(80, 55, 30, 20);
        southEditor.setBounds(80, 80, 30, 20);
        homeEditor.setBounds(80, 105, 30, 20);
        channelEditor.setBounds(80, 130, 30, 20);
    }

    std::atomic<bool> settingChanged = false;
    int midiChannel;
    int data[5];

  private:
    juce::Rectangle<int> outline1{10, 5, 100, 40};
    juce::Rectangle<int> outline2{10, 30, 100, 40};
    juce::Rectangle<int> outline3{10, 55, 100, 40};
    juce::Rectangle<int> outline4{10, 80, 100, 40};
    juce::Rectangle<int> outline5{10, 105, 100, 40};
    juce::Rectangle<int> outline6{10, 130, 100, 40};

    juce::TextEditor westEditor{"West"};
    juce::TextEditor eastEditor{"East"};
    juce::TextEditor northEditor{"North"};
    juce::TextEditor southEditor{"South"};
    juce::TextEditor homeEditor{"Home"};
    juce::TextEditor channelEditor{"Channel"};

    juce::Label westLabel{{}, "West CC"};
    juce::Label eastLabel{{}, "East CC"};
    juce::Label northLabel{{}, "North CC"};
    juce::Label southLabel{{}, "South CC"};
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

        for (int i = 0; i < 5; ++i)
        {
            if (input == data[i])
            {
                return true;
            }
        }

        return false;
    }

    void returnKeyResponse(juce::TextEditor *e)
    {
        e->setHighlightedRegion(noRange);
        this->unfocusAllComponents();

        int digit = e->getText().getIntValue();

        if (e == &westEditor)
        {
            if (rejectBadInput(digit))
            {
                e->setText(std::to_string(data[0]));
                return;
            }

            data[0] = digit;
            settingChanged = true;
        }

        if (e == &eastEditor)
        {
            if (rejectBadInput(digit))
            {
                e->setText(std::to_string(data[1]));
                return;
            }

            data[1] = digit;
            settingChanged = true;
        }

        if (e == &northEditor)
        {
            if (rejectBadInput(digit))
            {
                e->setText(std::to_string(data[2]));
                return;
            }

            data[2] = digit;
            settingChanged = true;
        }

        if (e == &southEditor)
        {
            if (rejectBadInput(digit))
            {
                e->setText(std::to_string(data[3]));
                return;
            }

            data[3] = digit;
            settingChanged = true;
        }

        if (e == &homeEditor)
        {
            if (rejectBadInput(digit))
            {
                e->setText(std::to_string(data[4]));
                return;
            }

            data[4] = digit;
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
