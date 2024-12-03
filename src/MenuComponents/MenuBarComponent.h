/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do note that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#pragma once

#include <memory>

#include "LatticesProcessor.h"

#include "OriginComponent.h"
#include "ModeComponent.h"
#include "MIDIMenuComponent.h"
#include "VisitorsComponent.h"
#include "LatticesBinary.h"

//==============================================================================
struct MenuBarComponent : juce::Component
{
    MenuBarComponent(LatticesProcessor &p) : processor(p)
    {
        juce::Colour off = juce::Colours::transparentWhite;
        juce::Colour offo = juce::Colours::ghostwhite.withAlpha(.15f);
        juce::Colour offd = juce::Colours::ghostwhite.withAlpha(.5f);

        originButton = std::make_unique<juce::ShapeButton>("Origin", off, offo, offd);
        juce::Path oS = {};
        oS.addRectangle(originRect);
        originButton->setShape(oS, true, true, false);
        addAndMakeVisible(*originButton);
        originButton->onClick = [this] { showOriginMenu(); };
        originButton->setClickingTogglesState(true);
        originButton->setToggleState(false, juce::dontSendNotification);

        originC =
            std::make_unique<OriginComponent>(processor.originalRefNote, processor.originalRefFreq);
        addAndMakeVisible(*originC);
        originC->setVisible(false);

        midiButton = std::make_unique<juce::ShapeButton>("MIDI", off, offo, offd);
        juce::Path miS = {};
        miS.addRectangle(midiRect);
        midiButton->setShape(miS, true, true, false);
        addAndMakeVisible(*midiButton);
        midiButton->onClick = [this] { showMidiMenu(); };
        midiButton->setClickingTogglesState(true);
        midiButton->setToggleState(false, juce::dontSendNotification);

        midiC = std::make_unique<MIDIMenuComponent>(
            processor.shiftCCs[0], processor.shiftCCs[1], processor.shiftCCs[2],
            processor.shiftCCs[3], processor.shiftCCs[4], processor.listenOnChannel);
        addAndMakeVisible(*midiC);
        midiC->setVisible(false);

        modeButton = std::make_unique<juce::ShapeButton>("Mode", off, offo, offd);

        juce::Path moS = {};
        moS.addRectangle(modeRect);
        modeButton->setShape(moS, true, true, false);
        addAndMakeVisible(*modeButton);
        modeButton->onClick = [this] { showModeMenu(); };
        modeButton->setClickingTogglesState(true);
        modeButton->setToggleState(false, juce::dontSendNotification);

        modeC = std::make_unique<ModeComponent>(processor.mode);
        addAndMakeVisible(*modeC);
        modeC->setVisible(false);

        visButton = std::make_unique<juce::ShapeButton>("Visitors", off, offo, offd);

        juce::Path vS = {};
        vS.addRectangle(visRect);
        visButton->setShape(vS, true, true, false);
        addAndMakeVisible(*visButton);
        visButton->onClick = [this] { showVisitorMenu(); };
        visButton->setClickingTogglesState(true);
        visButton->setToggleState(false, juce::dontSendNotification);

        std::string bruh[1] = {"names of groups here eventually"};
        visC = std::make_unique<VisitorsComponent>(processor.currentVisitors->dimensions,
                                                   processor.numVisitorGroups, bruh);
        addAndMakeVisible(*visC);
        visC->setVisible(false);
    }

    void resized() override
    {
        auto b = this->getLocalBounds();

        visButton->setBounds(visRect);
        visC->setBounds(0, 30, 600, 300);
        
        originButton->setBounds(originRect);
        originC->setBounds(600, 30, 240, 95);

        midiButton->setBounds(midiRect);
        midiC->setBounds(840, 30, 120, 155);

        modeButton->setBounds(modeRect);
        modeC->setBounds(960, 30, 120, 90);


    }

    void paint(juce::Graphics &g) override
    {
        auto b = this->getLocalBounds();

        g.setColour(menuColour);
        g.fillRect(0, 0, b.getWidth(), 30);

        g.setColour(onColour);
        if (originButton->getToggleState())
            g.drawRect(originRect);
        if (midiButton->getToggleState())
            g.drawRect(midiRect);
        if (modeButton->getToggleState())
            g.drawRect(modeRect);
        if (visButton->getToggleState())
            g.drawRect(visRect);

        g.setColour(juce::Colours::ghostwhite);
        g.fillRect(0, 29, b.getWidth(), 2);

        g.drawRect(originRect);
        g.drawRect(midiRect);
        g.drawRect(modeRect);
        g.drawRect(visRect);

        g.setFont(stoke);

        g.drawText("Origin", originRect, 12, false);
        g.drawText("MIDI", midiRect, 12, false);
        g.drawText("Mode", modeRect, 12, false);
        g.drawText("Visitors", visRect, 12, false);
    }

    std::unique_ptr<OriginComponent> originC;
    std::unique_ptr<MIDIMenuComponent> midiC;
    std::unique_ptr<ModeComponent> modeC;
    std::unique_ptr<VisitorsComponent> visC;

  private:
    juce::ReferenceCountedObjectPtr<juce::Typeface> Stoke{juce::Typeface::createSystemTypefaceFor(
        LatticesBinary::Stoke_otf, LatticesBinary::Stoke_otfSize)};

    juce::Font stoke{juce::FontOptions(Stoke).withPointHeight(20)};

    juce::Colour menuColour{.475f, .5f, 0.2f, 1.f};
    juce::Colour onColour{.975f, .5f, 0.3f, 1.f};

    juce::Rectangle<int> visRect{0, 0, 600, 30};
    juce::Rectangle<int> originRect{600, 0, 240, 30};
    juce::Rectangle<int> midiRect{840, 0, 120, 30};
    juce::Rectangle<int> modeRect{960, 0, 120, 30};
   

    std::unique_ptr<juce::ShapeButton> originButton;
    std::unique_ptr<juce::ShapeButton> midiButton;
    std::unique_ptr<juce::ShapeButton> modeButton;
    std::unique_ptr<juce::ShapeButton> visButton;

    void showOriginMenu()
    {
        bool show = originButton->getToggleState();

        if (show)
        {
            originC->setVisible(true);
            midiButton->setToggleState(false, juce::sendNotification);
            modeButton->setToggleState(false, juce::sendNotification);
            visButton->setToggleState(false, juce::sendNotification);
        }
        else
        {
            originC->setVisible(false);
        }
    }

    void showMidiMenu()
    {
        bool show = midiButton->getToggleState();

        if (show)
        {
            midiC->setVisible(true);
            originButton->setToggleState(false, juce::sendNotification);
            modeButton->setToggleState(false, juce::sendNotification);
            visButton->setToggleState(false, juce::sendNotification);
        }
        else
        {
            midiC->setVisible(false);
        }
    }

    void showModeMenu()
    {
        bool show = modeButton->getToggleState();

        if (show)
        {
            modeC->setVisible(true);
            originButton->setToggleState(false, juce::sendNotification);
            midiButton->setToggleState(false, juce::sendNotification);
            visButton->setToggleState(false, juce::sendNotification);
        }
        else
        {
            modeC->setVisible(false);
        }
    }

    void showVisitorMenu()
    {
        bool show = visButton->getToggleState();

        if (show)
        {
            visC->setVisible(true);
            originButton->setToggleState(false, juce::sendNotification);
            midiButton->setToggleState(false, juce::sendNotification);
            modeButton->setToggleState(false, juce::sendNotification);
        }
        else
        {
            visC->setVisible(false);
        }
    }

    LatticesProcessor &processor;
};
