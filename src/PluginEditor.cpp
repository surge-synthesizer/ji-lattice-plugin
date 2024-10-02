/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JIMTSSourceEditor::JIMTSSourceEditor(JIMTSSourceProcessor &p)
    : juce::AudioProcessorEditor(&p), processor(p)
{
    jiComponent = std::make_unique<JIComponent>(p.positionX,p.positionY);
    addAndMakeVisible(*jiComponent);
    
    startTimer(5);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(1200, 900);
    setResizable(true, true);
}

JIMTSSourceEditor::~JIMTSSourceEditor() {}

//==============================================================================
void JIMTSSourceEditor::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::black);
}

void JIMTSSourceEditor::idle() {}

void JIMTSSourceEditor::resized()
{
    jiComponent->setBounds(getLocalBounds());
}

void JIMTSSourceEditor::timerCallback()
{
    int x = processor.positionX;
    int y = processor.positionY;
    
    if (x != priorX || y != priorY)
    {
        
        jiComponent->updateLocation(x, y);
        jiComponent->repaint();
        priorX = x;
        priorY = y;
    }
}
