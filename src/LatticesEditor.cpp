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
    visitorsButton = std::make_unique<juce::TextButton>("Visitors");
    addAndMakeVisible(*visitorsButton);
    visitorsButton->onClick = [this] { showVisitorsMenu(); };
    visitorsButton->setClickingTogglesState(true);
    visitorsButton->setToggleState(false, juce::dontSendNotification);
    
    visitorsComponent = std::make_unique<VisitorsComponent>();
    addAndMakeVisible(*visitorsComponent);
    visitorsComponent->setVisible(false);
    
    midiButton = std::make_unique<juce::TextButton>("MIDI Settings");
    addAndMakeVisible(*midiButton);
    midiButton->onClick = [this] { showMidiMenu(); };
    midiButton->setClickingTogglesState(true);
    midiButton->setToggleState(false, juce::dontSendNotification);

    midiComponent = std::make_unique<MIDIMenuComponent>(
        processor.shiftCCs[0], processor.shiftCCs[1], processor.shiftCCs[2], processor.shiftCCs[3],
        processor.shiftCCs[4], processor.listenOnChannel);
    addAndMakeVisible(*midiComponent);
    midiComponent->setVisible(false);

    tuningButton = std::make_unique<juce::TextButton>("Tuning Settings");
    addAndMakeVisible(*tuningButton);
    tuningButton->onClick = [this] { showTuningMenu(); };
    tuningButton->setClickingTogglesState(true);
    tuningButton->setToggleState(false, juce::dontSendNotification);

    modeComponent = std::make_unique<ModeComponent>(processor.mode);
    addAndMakeVisible(*modeComponent);
    modeComponent->setVisible(false);

    originComponent =
        std::make_unique<OriginComponent>(processor.originalRefNote, processor.originalRefFreq);
    addAndMakeVisible(*originComponent);
    originComponent->setVisible(false);

    auto b = this->getLocalBounds();

    midiButton->setBounds(10, b.getBottom() - 40, 120, 30);
    midiComponent->setBounds(10, b.getBottom() - 155 - 30 - 10, 120, 155);

    tuningButton->setBounds(b.getRight() - 216 - 10, b.getBottom() - 40, 216, 30);
    modeComponent->setBounds(b.getRight() - 216 - 10, b.getBottom() - 180 - 40, 216, 90);
    originComponent->setBounds(b.getRight() - 216 - 10, b.getBottom() - 95 - 40, 216, 95);
    
    visitorsButton->setBounds(10, 10, 120, 30);
    visitorsComponent->setBounds(10, 40, 260, 185);
    currentVisitors.reset();
    
    inited = true;
}

void LatticesEditor::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(backgroundColour);
}

void LatticesEditor::idle() {}

void LatticesEditor::resized()
{
    auto b = this->getLocalBounds();
    latticeComponent->setBounds(b);

    if (inited)
    {
        midiButton->setBounds(10, b.getBottom() - 40, 120, 30);
        midiComponent->setBounds(10, b.getBottom() - 155 - 30 - 10, 120, 155);

        tuningButton->setBounds(b.getRight() - 216 - 10, b.getBottom() - 40, 216, 30);
        modeComponent->setBounds(b.getRight() - 216 - 10, b.getBottom() - 180 - 40, 216, 90);
        originComponent->setBounds(b.getRight() - 216 - 10, b.getBottom() - 95 - 40, 216, 95);
        
        
        visitorsButton->setBounds(10, 10, 125, 30);
        visitorsComponent->setBounds(10, 40, 260, 185);
    }
    else
    {
        warningComponent->setBounds(375, 175, 200, 200);
    }
}

void LatticesEditor::showMidiMenu()
{
    bool show = midiButton->getToggleState();
    midiComponent->setVisible(show);

    if (show)
    {
        midiButton->setConnectedEdges(4);
    }
    else
    {
        midiButton->setConnectedEdges(!4);
    }
}

void LatticesEditor::showTuningMenu()
{
    bool show = tuningButton->getToggleState();
    originComponent->setVisible(show);
    modeComponent->setVisible(show);

    if (show)
    {
        tuningButton->setConnectedEdges(4);
    }
    else
    {
        tuningButton->setConnectedEdges(!4);
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
            init();
        }
    }

    if (timerID == 0)
    {
        if (processor.changed)
        {
            latticeComponent->update(processor.coOrds);
            latticeComponent->repaint();
            processor.changed = false;
        }

        if (modeComponent->modeChanged)
        {
            processor.modeSwitch(modeComponent->whichMode());
            modeComponent->modeChanged = false;
        }

        if (midiComponent->settingChanged)
        {
            processor.updateMIDI(midiComponent->data[0], midiComponent->data[1],
                                 midiComponent->data[2], midiComponent->data[3],
                                 midiComponent->data[4], midiComponent->midiChannel);
        }

        if (originComponent->freqChanged)
        {
            processor.updateFreq(originComponent->whatFreq);
            originComponent->freqChanged = false;
        }

        if (originComponent->rootChanged)
        {
            int r = originComponent->whichNote();
            originComponent->resetFreqOnRootChange(processor.updateRoot(r));
        }
        
        if (visitorsComponent->changed)
        {
            int t[12];
            for (int i = 0; i < 12; ++i)
            {
                int v = visitorsComponent->dd[i];
                t[i] = v;
                currentVisitors.setDegree(i, v);
            }
            
            latticeComponent->setVisitors(t);
            processor.updateVisitors(currentVisitors.comma);
            visitorsComponent->changed = false;
        }
    }
}
