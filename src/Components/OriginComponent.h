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
struct OriginComponent : public juce::Component, juce::Timer
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

        key[proc->originalRefNote]->setToggleState(true, juce::dontSendNotification);

        addAndMakeVisible(freqEditor);

        freqEditor.setRange(100.0, 1000.0);
        freqEditor.setTextValueSuffix(" Hz");
        freqEditor.setSkewFactor(0.5);
        freqEditor.setNumDecimalPlacesToDisplay(3);

        freqEditor.setColour(juce::Slider::textBoxTextColourId, gwhite);
        freqEditor.setColour(juce::Slider::textBoxOutlineColourId, gwhite);
        freqEditor.setColour(juce::Slider::thumbColourId, gwhite);
        freqEditor.onValueChange = [this] { editFrequency(&freqEditor); };
        freqEditor.setValue(proc->originalRefFreq);
        freqEditor.setVelocityBasedMode(true);

        //        addAndMakeVisible(freqLabel);
        //        freqLabel.setJustificationType(juce::Justification::left);
        //        freqLabel.setColour(juce::Label::backgroundColourId, menuColour);

        priorFreq = proc->originalRefFreq;

        startTimer(5);
    }

    void resized() override
    {
        for (int i = 0; i < 12; ++i)
        {
            key[i]->setBounds(kw * i + 1, 1, kw, kh);
        }

        //        freqLabel.setBounds(1, kh, 106, 27);
        freqEditor.setBounds(5, kh, 200, 30);
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
        freqEditor.setValue(proc->originalRefFreq);
        priorFreq = proc->originalRefFreq;
        key[proc->originalRefNote]->setToggleState(true, juce::dontSendNotification);
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

    juce::Colour gwhite = juce::Colours::ghostwhite;

    juce::Colour white = juce::Colours::antiquewhite;
    juce::Colour black = juce::Colours::black;

    juce::Colour trans{juce::Colours::transparentWhite};
    juce::Colour over{juce::Colours::darkgrey.withAlpha(.5f)};

    juce::Colour sel{juce::Colours::darkviolet};
    juce::Colour selover{juce::Colours::blueviolet};

    juce::Range<int> noRange{};
    juce::Colour noColour{};

    //    juce::Label freqLabel{{}, "Ref. Frequency = "};
    juce::Slider freqEditor{"Ref Freq"};

    double priorFreq{};

    void editFrequency(juce::Slider *s)
    {
        auto input = s->getValue();
        proc->updateFreq(input);
        priorFreq = input;
    }

    //    void freqFromText(juce::Slider *e)
    //    {
    //    }
    //
    //    void freqToText(juce::Slider *e)
    //    {
    //    }

    void whichNote()
    {
        for (int i = 0; i < 12; ++i)
        {
            if (key[i]->getToggleState() == true)
            {
                double f = proc->updateRoot(i);
                freqEditor.setValue(f);
                priorFreq = f;
                break;
            }
        }
    }

    void timerCallback() override
    {
        if (proc->originalRefFreq != priorFreq)
        {
            priorFreq = proc->originalRefFreq;
            freqEditor.setValue(priorFreq);
        }
    }
};
