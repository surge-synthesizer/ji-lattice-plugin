/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <memory>

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
    
    
    modeButton = std::make_unique<juce::TextButton>("Mode");
    addAndMakeVisible(*modeButton);
    modeButton->onClick = [this]{ showModeMenu(); };
    modeButton->setClickingTogglesState(true);
    modeButton->setToggleState(false, juce::dontSendNotification);
    
    modeComponent = std::make_unique<ModeComponent>(p.mode);
    addAndMakeVisible(*modeComponent);
    modeComponent->setVisible(false);
    
    
    midiButton = std::make_unique<juce::TextButton>("MIDI Settings");
    addAndMakeVisible(*midiButton);
    midiButton->onClick = [this]{ showMidiMenu(); };
    midiButton->setClickingTogglesState(true);
    midiButton->setToggleState(false, juce::dontSendNotification);
    
    midiComponent = std::make_unique<MIDIMenuComponent>(p.shiftCCs[0],
                                                        p.shiftCCs[1],
                                                        p.shiftCCs[2],
                                                        p.shiftCCs[3],
                                                        p.shiftCCs[4],
                                                        p.listenOnChannel);
    addAndMakeVisible(*midiComponent);
    midiComponent->setVisible(false);
    
    
    originButton = std::make_unique<juce::TextButton>("Origin");
    addAndMakeVisible(*originButton);
    originButton->onClick = [this]{ showOriginMenu(); };
    originButton->setClickingTogglesState(true);
    originButton->setToggleState(false, juce::dontSendNotification);
    
    originComponent = std::make_unique<OriginComponent>(p.originalRefNote,
                                                        p.originalRefFreq);
    addAndMakeVisible(*originComponent);
    originComponent->setVisible(false);
    
    startTimer(5);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(width, height);
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
    auto b = this->getLocalBounds();
    
    latticeComponent->setBounds(b);
    
    
    modeButton->setBounds(10, 10, 120, 30);
    modeComponent->setBounds(10, 40, 120, 125);
    
    midiButton->setBounds(10, b.getBottom() - 40, 120, 30);
    midiComponent->setBounds(10, b.getBottom() - 155 - 30 - 10, 120, 155);
    
    originButton->setBounds(b.getRight() - 216 - 10, b.getBottom() - 40, 216, 30);
    originComponent->setBounds(b.getRight() - 216 - 10, b.getBottom() - 95 - 40, 216, 95);
}

void LatticesEditor::showModeMenu()
{
    
    if (modeButton->getToggleState())
    {
        modeComponent->setVisible(true);
        modeButton->setConnectedEdges(8);
    }
    else
    {
        modeComponent->setVisible(false);
        modeButton->setConnectedEdges(!8);
    }
}

void LatticesEditor::showMidiMenu()
{
    if (midiButton->getToggleState())
    {
        midiComponent->setVisible(true);
        midiButton->setConnectedEdges(4);
    }
    else
    {
        midiComponent->setVisible(false);
        midiButton->setConnectedEdges(!4);
    }
}

void LatticesEditor::showOriginMenu()
{
    if (originButton->getToggleState())
    {
        originComponent->setVisible(true);
        originButton->setConnectedEdges(4);
    }
    else
    {
        originComponent->setVisible(false);
        originButton->setConnectedEdges(!4);
    }
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
    
    if (modeComponent->modeChanged)
    {
        processor.modeSwitch(modeComponent->whichMode());
        modeComponent->modeChanged = false;
    }
    
    if (midiComponent->settingChanged)
    {
        processor.shiftCCs[0] = midiComponent->data[0];
        processor.shiftCCs[1] = midiComponent->data[1];
        processor.shiftCCs[2] = midiComponent->data[2];
        processor.shiftCCs[3] = midiComponent->data[3];
        processor.shiftCCs[4] = midiComponent->data[4];
        processor.listenOnChannel = midiComponent->midiChannel;
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
}
