
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "JIComponent.h"

//==============================================================================
/**
*/
class JIMTSSourceEditor : public juce::AudioProcessorEditor, juce::Timer
{
public:
  JIMTSSourceEditor(JIMTSSourceProcessor &);
    ~JIMTSSourceEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    std::unique_ptr<JIComponent> jiComponent;

    // Do this later
    std::unique_ptr<juce::Timer> idleTimer;
    void idle();

private:

    int priorX{0}, priorY{0};
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    JIMTSSourceProcessor &processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JIMTSSourceEditor)
};
