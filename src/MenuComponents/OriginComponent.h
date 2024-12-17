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

//==============================================================================
struct OriginComponent : public juce::Component
{
    OriginComponent(LatticesProcessor &p) : proc(&p)
    {
        keyshape.addRectangle(keysize);

        for (int i = 0; i < 12; ++i)
        {
            key.add(new juce::ShapeButton(std::to_string(i), trans, over, over));
            key[i]->setShape(keyshape, true, true, false);
            addAndMakeVisible(*key[i]);
            key[i]->setRadioGroupId(1);
            key[i]->onClick = [this] { whichNote(); };
            key[i]->setClickingTogglesState(true);
            key[i]->setOutline(noColour, 0.f);
            key[i]->shouldUseOnColours(true);
            key[i]->setOnColours(sel, selover, selover);
        }

        key[proc->currentRefNote]->setToggleState(true, juce::dontSendNotification);

        addAndMakeVisible(freqEditor);
        freqEditor.setMultiLine(false);
        freqEditor.setReturnKeyStartsNewLine(false);
        freqEditor.setInputRestrictions(17, ".1234567890");
        freqEditor.setText(std::to_string(proc->currentRefFreq), false);
        freqEditor.setJustification(juce::Justification::left);
        freqEditor.setSelectAllWhenFocused(true);
        freqEditor.setColour(juce::TextEditor::outlineColourId, noColour);
        freqEditor.setColour(juce::TextEditor::focusedOutlineColourId, noColour);
        freqEditor.onReturnKey = [this] { returnKeyResponse(&freqEditor); };
        freqEditor.onEscapeKey = [this] { escapeKeyResponse(&freqEditor); };
        freqEditor.onFocusLost = [this] { focusLostResponse(&freqEditor); };

        addAndMakeVisible(freqLabel);
        freqLabel.setJustificationType(juce::Justification::left);
        freqLabel.setColour(juce::Label::backgroundColourId, menuColour);
        priorFreq = proc->currentRefFreq;
    }

    void resized() override
    {
        for (int i = 0; i < 12; ++i)
        {
            key[i]->setBounds(kw * i + 1, 1, kw, kh);
        }

        freqLabel.setBounds(1, kh, 106, 27);
        freqEditor.setBounds(kw * 5 + 5, kh, 106, 27);
    }

    void paint(juce::Graphics &g) override
    {
        g.setColour(menuColour);
        g.fillRect(this->getLocalBounds());

        for (int i = 0; i < 12; ++i)
        {
            auto c = (kb[i]) ? juce::Colours::black : juce::Colours::antiquewhite;
            g.setColour(c);
            g.fillRect(kw * i + 1, 1, kw, kh);
        }
        g.setColour(juce::Colours::black);
        g.fillRect(kw * 5, 1, 2, kh);

        g.setColour(juce::Colours::ghostwhite);
        g.drawRect(this->getLocalBounds());
    }

    void reset()
    {
        freqEditor.setText(std::to_string(proc->currentRefFreq), false);
        priorFreq = proc->currentRefFreq;
        key[proc->currentRefNote]->setToggleState(true, juce::dontSendNotification);
    }

  private:
    LatticesProcessor *proc;

    static constexpr int kw = 20;
    static constexpr int kh = 65;

    juce::OwnedArray<juce::ShapeButton> key;

    juce::Rectangle<int> keysize = juce::Rectangle(kw, kh);
    juce::Path keyshape;

    std::array<bool, 12> kb = {false, true,  false, true,  false, false,
                               true,  false, true,  false, true,  false};

    juce::Colour menuColour{.475f, .5f, 0.2f, 1.f};

    juce::Colour white = juce::Colours::antiquewhite;
    juce::Colour black = juce::Colours::black;

    juce::Colour trans{juce::Colours::transparentWhite};
    juce::Colour over{juce::Colours::darkgrey.withAlpha(.5f)};

    juce::Colour sel{juce::Colours::darkviolet};
    juce::Colour selover{juce::Colours::blueviolet};

    juce::Range<int> noRange{};
    juce::Colour noColour{};

    juce::Label freqLabel{{}, "Ref. Frequency = "};
    juce::TextEditor freqEditor{"Ref Freq"};

    double priorFreq{};

    void returnKeyResponse(juce::TextEditor *e)
    {
        e->setHighlightedRegion(noRange);
        this->unfocusAllComponents();
        auto input = e->getText().getDoubleValue();

        if (rejectBadInput(input))
        {
            e->setText(std::to_string(priorFreq));
            return;
        }

        proc->updateFreq(input);
        priorFreq = input;
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
        if (input <= 0)
        {
            return true;
        }

        if (input >= 10000)
        {
            return true;
        }

        return false;
    }

    void whichNote()
    {

        for (int i = 0; i < 12; ++i)
        {
            if (key[i]->getToggleState() == true)
            {
                double f = proc->updateRoot(i);
                freqEditor.setText(std::to_string(f), false);
                break;
            }
        }
    }
};
