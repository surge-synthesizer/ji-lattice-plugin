
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "melatonin_inspector/melatonin_inspector.h"
#include "PluginProcessor.h"
#include "LatticeComponent.h"
#include "ModeComponent.h"
#include "MIDIMenuComponent.h"

//==============================================================================
/**
*/
class LatticesEditor : public juce::AudioProcessorEditor, juce::Timer
{
public:
  LatticesEditor(LatticesProcessor &);
    ~LatticesEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    
    std::unique_ptr<juce::Timer> idleTimer;
    void idle();

private:
    melatonin::Inspector inspector { *this };
    
    std::unique_ptr<LatticeComponent> latticeComponent;
    std::unique_ptr<ModeComponent> modeComponent;
    std::unique_ptr<MIDIMenuComponent> midiComponent;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LatticesProcessor &processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LatticesEditor)
};
