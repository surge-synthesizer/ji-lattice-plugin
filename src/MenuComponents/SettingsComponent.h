/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do note that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#pragma once

#include "LatticesProcessor.h"

#include <string>
#include <cstdint>

//==============================================================================
struct SettingsComponent : public juce::Component // , juce::ToggleButton
{
    SettingsComponent(LatticesProcessor &p) : proc(&p)
    {
        priorCC = proc->homeCC;
        priorChannel = proc->listenOnChannel;
        priorDistance = proc->maxDistance;

        addAndMakeVisible(distLabel);
        distLabel.setJustificationType(juce::Justification::left);
        distLabel.setColour(juce::Label::backgroundColourId, bg);
        distLabel.setColour(juce::Label::outlineColourId, ol);

        addAndMakeVisible(distEditor);
        distEditor.setMultiLine(false);
        distEditor.setReturnKeyStartsNewLine(false);
        distEditor.setInputRestrictions(3, "1234567890");
        distEditor.setText(std::to_string(proc->maxDistance), false);
        distEditor.setJustification(juce::Justification::centred);
        distEditor.setSelectAllWhenFocused(true);
        distEditor.setColour(juce::TextEditor::outlineColourId, ol);
        distEditor.onReturnKey = [this] { returnKeyResponse(&distEditor); };
        distEditor.onEscapeKey = [this] { escapeKeyResponse(&distEditor); };
        distEditor.onFocusLost = [this] { focusLostResponse(&distEditor); };

        addAndMakeVisible(homeLabel);
        homeLabel.setJustificationType(juce::Justification::left);
        homeLabel.setColour(juce::Label::backgroundColourId, bg);
        homeLabel.setColour(juce::Label::outlineColourId, ol);

        addAndMakeVisible(homeEditor);
        homeEditor.setMultiLine(false);
        homeEditor.setReturnKeyStartsNewLine(false);
        homeEditor.setInputRestrictions(3, "1234567890");
        homeEditor.setText(std::to_string(proc->homeCC), false);
        homeEditor.setJustification(juce::Justification::centred);
        homeEditor.setSelectAllWhenFocused(true);
        homeEditor.setColour(juce::TextEditor::outlineColourId, ol);
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
        channelEditor.setText(std::to_string(proc->listenOnChannel), false);
        channelEditor.setJustification(juce::Justification::centred);
        channelEditor.setSelectAllWhenFocused(true);
        channelEditor.setColour(juce::TextEditor::outlineColourId, ol);
        channelEditor.onReturnKey = [this] { returnKeyResponse(&channelEditor); };
        channelEditor.onEscapeKey = [this] { escapeKeyResponse(&channelEditor); };
        channelEditor.onFocusLost = [this] { focusLostResponse(&channelEditor); };

        addAndMakeVisible(syntonicButton);
        syntonicButton.onClick = [this] { updateToggleState(); };
        syntonicButton.setClickingTogglesState(true);
        syntonicButton.setRadioGroupId(1);

        addAndMakeVisible(duodeneButton);
        duodeneButton.onClick = [this] { updateToggleState(); };
        duodeneButton.setClickingTogglesState(true);
        duodeneButton.setRadioGroupId(1);

        if (proc->mode == LatticesProcessor::Syntonic)
        {
            syntonicButton.setToggleState(true, juce::dontSendNotification);
        }
        else
        {
            duodeneButton.setToggleState(true, juce::dontSendNotification);
        }
    }

    void paint(juce::Graphics &g) override
    {
        g.setColour(bg);
        g.fillRect(this->getLocalBounds());

        g.setColour(ol);
        g.drawRect(this->getLocalBounds());
    }

    void resized() override
    {
        duodeneButton.setBounds(5, 5, 100, 35);
        syntonicButton.setBounds(5, 45, 100, 35);

        distLabel.setBounds(10, 100, 70, 20);
        distEditor.setBounds(80, 100, 30, 20);

        homeLabel.setBounds(10, 125, 70, 20);
        homeEditor.setBounds(80, 125, 30, 20);

        channelLabel.setBounds(10, 150, 70, 20);
        channelEditor.setBounds(80, 150, 30, 20);
    }

    void reset()
    {
        distEditor.setText(std::to_string(proc->maxDistance), false);
        homeEditor.setText(std::to_string(proc->homeCC), false);
        channelEditor.setText(std::to_string(proc->listenOnChannel), false);

        if (proc->mode == LatticesProcessor::Syntonic)
        {
            syntonicButton.setToggleState(true, juce::dontSendNotification);
        }
        else
        {
            duodeneButton.setToggleState(true, juce::dontSendNotification);
        }
    }

  private:
    LatticesProcessor *proc;

    uint8_t priorChannel;
    uint8_t priorCC;
    uint16_t priorDistance;

    juce::Label distLabel{{}, "Max Distance"};
    juce::TextEditor distEditor{"Distance"};

    juce::Label homeLabel{{}, "Home CC"};
    juce::TextEditor homeEditor{"Home"};

    juce::Label channelLabel{{}, "Channel"};
    juce::TextEditor channelEditor{"Channel"};

    juce::TextButton duodeneButton{"Duodene"};
    juce::TextButton syntonicButton{"Syntonic"};

    //    juce::Colour noColour{};
    juce::Colour bg{.475f, .5f, 0.2f, 1.f};
    juce::Colour ol{juce::Colours::ghostwhite};
    juce::Range<int> noRange{};

    void updateToggleState()
    {
        if (duodeneButton.getToggleState() == true)
        {
            proc->modeSwitch(0);
        }
        if (syntonicButton.getToggleState() == true)
        {
            proc->modeSwitch(1);
        }
    }

    bool rejectBadInput(int input, int type = 0)
    {
        // reject if out of range

        if (type == 1)
        {
            if (input < 1 || input > 16)
            {
                return true;
            }
            return false;
        }

        if (type == 2)
        {
            if (input < 1 || input > UINT16_MAX)
            {
                return true;
            }
            return false;
        }

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
                e->setText(std::to_string(priorCC));
                return;
            }

            proc->updateMIDICC(digit);
            priorCC = digit;
        }

        if (e == &channelEditor)
        {
            if (rejectBadInput(digit, 1))
            {
                e->setText(std::to_string(priorChannel));
                return;
            }

            proc->updateMIDIChannel(digit);
            priorChannel = digit;
        }

        if (e == &distEditor)
        {
            if (rejectBadInput(digit, 2))
            {
                e->setText(std::to_string(priorDistance));
                return;
            }

            proc->updateDistance(digit);
            priorDistance = digit;
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
