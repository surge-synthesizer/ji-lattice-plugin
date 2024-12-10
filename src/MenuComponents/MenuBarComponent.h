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
#include "SettingsComponent.h"
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

        settingsRect.setBounds(600, 0, 120, 30);
        // settingsRect.setBounds(this->getLocalBounds().getRight() - 240, 0, 240, 30);

        visButton = std::make_unique<juce::ShapeButton>("Visitors", off, offo, offd);

        juce::Path vS = {};
        vS.addRectangle(visRect);
        visButton->setShape(vS, true, true, false);
        addAndMakeVisible(*visButton);
        visButton->onClick = [this] { showVisitorMenu(); };
        visButton->setClickingTogglesState(true);
        visButton->setToggleState(false, juce::dontSendNotification);

        std::string bruh[1] = {"names of groups here eventually"};
        visC = std::make_unique<VisitorsComponent>(processor.numVisitorGroups, bruh);
        addAndMakeVisible(*visC);
        visC->setVisible(false);

        settingsButton = std::make_unique<juce::ShapeButton>("Settings", off, offo, offd);
        juce::Path sS = {};
        sS.addRectangle(settingsRect);
        settingsButton->setShape(sS, true, true, false);
        addAndMakeVisible(*settingsButton);
        settingsButton->onClick = [this] { showSettingsMenu(); };
        settingsButton->setClickingTogglesState(true);
        settingsButton->setToggleState(false, juce::dontSendNotification);

        settingsC = std::make_unique<SettingsComponent>(processor.homeCC, processor.listenOnChannel,
                                                        processor.mode, processor.maxDistance);
        addAndMakeVisible(*settingsC);
        settingsC->setVisible(false);

        originC =
            std::make_unique<OriginComponent>(processor.originalRefNote, processor.originalRefFreq);
        addAndMakeVisible(*originC);
        originC->setVisible(false);
    }

    void resized() override
    {
        auto b = this->getLocalBounds();

        visButton->setBounds(visRect);
        visC->setBounds(0, 30, 750, 300);

        settingsButton->setBounds(settingsRect);
        settingsC->setBounds(600, 30, 120, 200);
        originC->setBounds(360, 30, 240, 95);
    }

    void paint(juce::Graphics &g) override
    {
        auto b = this->getLocalBounds();

        g.setColour(menuColour);
        g.fillRect(0, 0, b.getWidth(), 30);

        if (visButton->getToggleState())
            g.drawRect(visRect);
        if (settingsButton->getToggleState())
            g.drawRect(settingsRect);

        g.setColour(juce::Colours::ghostwhite);
        g.fillRect(0, 29, b.getWidth(), 2);

        g.drawRect(visRect);
        g.drawRect(settingsRect);

        g.setFont(stoke);

        if (processor.mode == processor.Mode::Duodene)
        {
            g.drawText("Visitors", visRect, 12, false);
        }
        g.drawText("Settings", settingsRect, 12, false);
    }

    std::unique_ptr<OriginComponent> originC;
    std::unique_ptr<SettingsComponent> settingsC;
    std::unique_ptr<VisitorsComponent> visC;

  private:
    juce::ReferenceCountedObjectPtr<juce::Typeface> Stoke{juce::Typeface::createSystemTypefaceFor(
        LatticesBinary::Stoke_otf, LatticesBinary::Stoke_otfSize)};

    juce::Font stoke{juce::FontOptions(Stoke).withPointHeight(20)};

    juce::Colour menuColour{.475f, .5f, 0.2f, 1.f};
    juce::Colour onColour{.975f, .5f, 0.3f, 1.f};

    juce::Rectangle<int> visRect{0, 0, 600, 30};
    juce::Rectangle<int> settingsRect{};

    std::unique_ptr<juce::ShapeButton> settingsButton;
    std::unique_ptr<juce::ShapeButton> visButton;

    void showSettingsMenu()
    {
        bool show = settingsButton->getToggleState();

        if (show)
        {
            settingsC->setVisible(true);
            originC->setVisible(true);
            visButton->setToggleState(false, juce::sendNotification);
        }
        else
        {
            settingsC->setVisible(false);
            originC->setVisible(false);
        }
    }

    void showVisitorMenu()
    {
        bool show = (visButton->getToggleState() && processor.mode == processor.Mode::Duodene);

        if (show)
        {
            visC->setVisible(true);
            settingsButton->setToggleState(false, juce::sendNotification);
        }
        else
        {
            visC->setVisible(false);
        }
    }

    LatticesProcessor &processor;
};
