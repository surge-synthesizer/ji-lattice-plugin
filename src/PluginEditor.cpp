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
    latticeComponent = std::make_unique<LatticeComponent>(p.coOrds);
    addAndMakeVisible(*latticeComponent);
    
    startTimer(5);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(1200, 900);
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
}

void LatticesEditor::timerCallback()
{
    if (processor.changed)
    {
        if (processor.mode == LatticesProcessor::Pyth)
        {
            
        }
        else if (processor.mode == LatticesProcessor::Syntonic)
        {
            latticeComponent->updateLocation(processor.coOrds);
        }
        else
        {
            latticeComponent->updateLocation(processor.positionX, processor.positionY);
        }
        
        latticeComponent->repaint();
        processor.changed = false;
    }
}
