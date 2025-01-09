/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do note that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#include "LatticesProcessor.h"
#include "LatticesEditor.h"

//==============================================================================
LatticesEditor::LatticesEditor(LatticesProcessor &p) : juce::AudioProcessorEditor(&p), processor(p)
{
    latticeComponent = std::make_unique<LatticeComponent>(p);
    addAndMakeVisible(*latticeComponent);
    latticeComponent->setBufferedToImage(true);

    warningComponent = std::make_unique<MTSWarningComponent>(p);
    addAndMakeVisible(*warningComponent);

    if (p.registeredMTS)
    {
        startTimer(0, 5);
        init();
    }
    else
    {
        startTimer(1, 50);
    }

    setSize(width, height);
    setResizable(true, true);
}

LatticesEditor::~LatticesEditor() {}

//==============================================================================

void LatticesEditor::init()
{
    menuComponent = std::make_unique<MenuBarComponent>(processor);
    addAndMakeVisible(*menuComponent);
    menuComponent->setBounds(this->getLocalBounds());

    zoomOutButton = std::make_unique<juce::TextButton>("-");
    addAndMakeVisible(*zoomOutButton);
    zoomOutButton->onClick = [this] { latticeComponent->zoomOut(); };

    zoomInButton = std::make_unique<juce::TextButton>("+");
    addAndMakeVisible(*zoomInButton);
    zoomInButton->onClick = [this] { latticeComponent->zoomIn(); };

    inited = true;
    startTimer(0, 5);
}

void LatticesEditor::paint(juce::Graphics &g) { g.fillAll(backgroundColour); }

void LatticesEditor::idle() {}

void LatticesEditor::resized()
{
    auto b = this->getLocalBounds();
    latticeComponent->setBounds(b);

    if (inited)
    {
        auto h = 30;
        if (menuComponent->visC->isVisible())
        {
            h = 330;
        }
        else if (menuComponent->settingsC->isVisible())
        {
            h = 240;
        }

        menuComponent->setBounds(0, 0, b.getWidth(), h);
        zoomOutButton->setBounds(20, b.getBottom() - 55, 35, 35);
        zoomInButton->setBounds(60, b.getBottom() - 55, 35, 35);
    }
    else
    {
        warningComponent->setBounds((width / 2) - 100, (height / 2) - 100, 200, 200);
    }
}

void LatticesEditor::timerCallback(int timerID)
{
    if (timerID == 1)
    {
        if (processor.registeredMTS)
        {
            warningComponent->setVisible(false);
            warningComponent->setEnabled(false);
            stopTimer(1);
            init();
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
        menuComponent->visC->setEnabled(edvi);

        bool mevi = (edvi || menuComponent->settingsC->isVisible());

        if (mevi != menuWasOpen)
        {
            resized();
            menuWasOpen = mevi;
        }

        if (processor.editingVisitors != edvi)
        {
            int g = menuComponent->visC->selectedGroup;
            processor.editVisitors(edvi, g);
            latticeComponent->repaint();
        }
        latticeComponent->setEnabled(!menuComponent->visC->isVisible());
    }
}
