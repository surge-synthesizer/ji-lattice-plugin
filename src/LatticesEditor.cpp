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
    latticeComponent = std::make_unique<LatticeComponent>(p.coOrds);
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

        menuComponent->setBounds(b);
    }
    else
    {
        warningComponent->setBounds((width / 2) - 100, (height / 2) - 100, 200, 200);
    }
}

void LatticesEditor::showVisitorsMenu()
{
    bool show = visitorsButton->getToggleState();
    visitorsComponent->setVisible(show);

    //    if (show)
    //    {
    //        visitorsButton->setConnectedEdges(0);
    //    }
    //    else
    //    {
    //        visitorsButton->setConnectedEdges(!0);
    //    }
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
        bool edvi = menuComponent->visC->isVisible();
        menuComponent->visC->setEnabled(edvi);
        if (processor.editingVisitors != edvi)
        {
            int g = menuComponent->visC->selectedGroup;
            processor.editVisitors(edvi, g);
        }
        latticeComponent->setEnabled(!menuComponent->visC->isVisible());
        latticeComponent->repaint();

        if (processor.changed)
        {

            latticeComponent->update(processor.coOrds, processor.currentVisitors->dimensions);

            latticeComponent->repaint();

            processor.changed = false;
        }

        if (menuComponent->settingsC->modeChanged)
        {
            processor.modeSwitch(menuComponent->settingsC->whichMode());
            menuComponent->settingsC->modeChanged = false;
        }

        if (menuComponent->settingsC->settingChanged)
        {
            processor.updateMIDI(menuComponent->settingsC->homeCC,
                                 menuComponent->settingsC->midiChannel);
        }

        if (menuComponent->settingsC->distanceChanged)
        {
            processor.updateDistance(menuComponent->settingsC->maxDistance);
            menuComponent->settingsC->distanceChanged = false;
        }

        if (menuComponent->originC->freqChanged)
        {
            processor.updateFreq(menuComponent->originC->whatFreq);
            menuComponent->originC->freqChanged = false;
        }

        if (menuComponent->originC->rootChanged)
        {
            int r = menuComponent->originC->whichNote();
            menuComponent->originC->resetFreqOnRootChange(processor.updateRoot(r));
            // Updates root in processor, and returns the frequency the note previously
            // had, which is fed back to the originComponent so it can update its readout
        }

        if (menuComponent->visC->reselect)
        {
            if (menuComponent->visC->madeNewGroup)
            {
                processor.newVisitorGroup();
                menuComponent->visC->madeNewGroup = false;
            }
            else
            {
                int g = menuComponent->visC->selectedGroup;
                // updates processor and returns the array of the selected groups values,
                // so the visitorComponent can update itself
                menuComponent->visC->setGroupData(processor.selectVisitorGroup(g));
                menuComponent->visC->reselect = false;
            }
        }

        if (menuComponent->visC->update)
        {
            int d = menuComponent->visC->selectedNote;
            int v = menuComponent->visC->dd[d];

            processor.updateVisitor(d, v);
            menuComponent->visC->update = false;
        }
    }
}
