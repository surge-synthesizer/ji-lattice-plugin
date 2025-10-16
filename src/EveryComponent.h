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

#include "Components/LatticeComponent.h"
#include "Components/MenuBarComponent.h"
#include "Components/MTSWarningComponent.h"
#include "melatonin_inspector/melatonin_inspector.h"

//==============================================================================
struct EveryComponent : public juce::Component, juce::MultiTimer
{
    EveryComponent(LatticesProcessor &p, int w, int h) : processor(p), width(w), height(h)
    {
        latticeComponent = std::make_unique<LatticeComponent>(p);
        addAndMakeVisible(*latticeComponent);
        latticeComponent->setBufferedToImage(true);

        warningComponent = std::make_unique<MTSWarningComponent>(p);
        addAndMakeVisible(*warningComponent);

        menuComponent = std::make_unique<MenuBarComponent>(processor);
        addAndMakeVisible(*menuComponent);
        menuComponent->setBounds(0, 0, this->getLocalBounds().getWidth(), 30);

        if (p.registeredMTS)
        {
            inited = true;
            startTimer(0, 5);
            resized();
        }
        else
        {
            latticeComponent->setEnabled(false);
            menuComponent->setVisible(false);
            startTimer(1, 50);
        }
    }

    void resized() override
    {
        auto b = this->getLocalBounds();
        latticeComponent->setBounds(b);

        if (inited)
        {
            auto h = 30;
            if (visOpen)
            {
                h = 330;
            }
            else if (setOpen)
            {
                h = 240;
            }

            menuComponent->setBounds(0, 0, b.getWidth(), h);
        }
        else
        {
            warningComponent->setBounds((width / 2) - 100, (height / 2) - 100, 200, 200);
        }
    }

    float backgroundWidth() { return this->getLocalBounds().getWidth(); }

  private:
    LatticesProcessor &processor;
    melatonin::Inspector inspector{*this, false};
    //    juce::ComponentBoundsConstrainer constraints;

    int width{0}, height{0};

    std::unique_ptr<LatticeComponent> latticeComponent;
    std::unique_ptr<MTSWarningComponent> warningComponent;
    std::unique_ptr<MenuBarComponent> menuComponent;

    bool inited{false};

    bool setOpen{false};
    bool visOpen{false};
    void timerCallback(int timerID) override
    {
        if (timerID == 1)
        {
            if (processor.registeredMTS)
            {
                warningComponent->setVisible(false);
                warningComponent->setEnabled(false);
                stopTimer(1);
                latticeComponent->setEnabled(true);
                menuComponent->setVisible(true);
                inited = true;
                startTimer(0, 5);
                resized();
            }
        }

        if (timerID == 0)
        {
            if (processor.loadedState)
            {
                menuComponent->resetAll();
                processor.loadedState = false;
            }

            bool edvi = menuComponent->visC->isVisible();
            bool edse = menuComponent->settingsC->isVisible();

            if (edvi != visOpen || edse != setOpen)
            {
                visOpen = edvi;
                setOpen = edse;
                resized();
            }

            if (processor.editingVisitors != edvi)
            {
                int g = menuComponent->visC->selectedGroup;
                processor.editVisitors(edvi, g);
                latticeComponent->repaint();
            }
            latticeComponent->setEnabled(!(edvi || edse));
        }
    }
};
