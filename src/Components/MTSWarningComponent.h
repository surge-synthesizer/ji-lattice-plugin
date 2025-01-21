/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do note that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#pragma once

#include "LatticesProcessor.h"

//==============================================================================
struct MTSWarningComponent : public juce::Component
{
    MTSWarningComponent(LatticesProcessor &p) : processor(p)
    {
        addAndMakeVisible(warningText);
        warningText.setText("MTS-ESP connection failed. If another master is already loaded, "
                            "please unload it and press re-connect. If not, a prior master failed "
                            "to de-register itself, in which case press re-initialize.",
                            juce::dontSendNotification);

        addAndMakeVisible(reInitButton);
        reInitButton.onClick = [this] { reInitPls(); };

        addAndMakeVisible(reConnectButton);
        reConnectButton.onClick = [this] { reConnectPls(); };
    }

    void resized() override
    {
        warningText.setVisible(true);

        reInitButton.setVisible(true);
        reInitButton.setEnabled(true);

        reConnectButton.setVisible(true);
        reConnectButton.setEnabled(true);

        warningText.setBounds(5, 5, 190, 100);
        reInitButton.setBounds(40, 150, 120, 35);
        reConnectButton.setBounds(40, 110, 120, 35);
    }

    void paint(juce::Graphics &g) override
    {
        auto b = this->getLocalBounds();

        g.setColour(bg);
        g.fillRect(b);
        g.setColour(juce::Colours::lightgrey);
        g.drawRect(b);
    }

    void reInitPls() { processor.MTSreInit = true; }
    void reConnectPls() { processor.MTStryAgain = true; }

  private:
    juce::Colour bg = findColour(juce::TextEditor::backgroundColourId);

    juce::TextButton reConnectButton{"Re-attempt connection"};
    juce::TextButton reInitButton{"Re-Initialize MTS-ESP library"};
    juce::Label warningText{"MTS-ESP Status Error", ""};

    LatticesProcessor &processor;
};
