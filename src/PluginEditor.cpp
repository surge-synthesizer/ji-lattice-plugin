/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LatticesEditor::LatticesEditor(LatticesProcessor &p)
    : juce::AudioProcessorEditor(&p), processor(p)
{
    
    latticeComponent = std::make_unique<LatticeComponent>();
    if (processor.mode == LatticesProcessor::Pyth)
    {
        latticeComponent->update(p.positionX);
    }
    else if (processor.mode == LatticesProcessor::Syntonic)
    {
        latticeComponent->update(p.coOrds);
    }
    else
    {
        latticeComponent->update(p.positionX, p.positionY);
    }
    
    addAndMakeVisible(*latticeComponent);
    
    modeComponent = std::make_unique<ModeComponent>(p.mode);
    addAndMakeVisible(*modeComponent);
    
    midiComponent = std::make_unique<MIDIMenuComponent>();
    addAndMakeVisible(*midiComponent);
    
    startTimer(5);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(900, 600);
    setResizable(true, true);
}

LatticesEditor::~LatticesEditor() {}

//==============================================================================
void LatticesEditor::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::black);
}

void LatticesEditor::idle() {}

void LatticesEditor::resized()
{
    latticeComponent->setBounds(getLocalBounds());
    modeComponent->setBounds(10, 10, 120, 125);
    midiComponent->setBounds(10, 445, 90, 155);
}

void LatticesEditor::timerCallback()
{
    if (processor.changed)
    {
        if (processor.mode == LatticesProcessor::Pyth)
        {
            latticeComponent->update(processor.positionX);
        }
        else if (processor.mode == LatticesProcessor::Syntonic)
        {
            latticeComponent->update(processor.coOrds);
        }
        else
        {
            latticeComponent->update(processor.positionX, processor.positionY);
        }
        
        latticeComponent->repaint();
        processor.changed = false;
    }
    
    if (modeComponent->modeChanged == true)
    {
        processor.modeSwitch(modeComponent->whichMode());
        modeComponent->modeChanged = false;
    }
    
    if (midiComponent->anyMidiChanged == true)
    {
        processor.shiftCCs[0] = midiComponent->westCC;
        processor.shiftCCs[1] = midiComponent->eastCC;
        processor.shiftCCs[2] = midiComponent->northCC;
        processor.shiftCCs[3] = midiComponent->southCC;
        processor.shiftCCs[4] = midiComponent->homeCC;
        processor.listenOnChannel = midiComponent->midiChannel;
    }
}
