/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <set>


class JIMTSSourceProcessor : public juce::AudioProcessor, juce::Timer
{
public:
    //==============================================================================
  JIMTSSourceProcessor();
    ~JIMTSSourceProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    void timerCallback() override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    void respondToMidi(const juce::MidiMessage &m);

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // == tuning support ==
    

    void initPythagorean12();
    void initDuodene();
    

    
    enum Direction
    {
        East,
        West,
        North,
        South,
        Home
    };
    
    void shift(int dir);

    void returnToOrigin();
    void updateTuning();
    
    std::pair<int,int> checkLocation();
    
    int positionX{0};
    int positionY{0};

    std::atomic<int> numClients{0};
private:
    
    int defaultRefNote{0};
    double defaultRefFreq{261.6255653005986}; // overkill precision
    
    int originalRefNote{2}; // C = 0, C# = 1, D = 2 etc
    int currentRefNote{-12};
    double originalRefFreq{293.3333333333333};
    double currentRefFreq{1.00000000};
    double drift{1.00000000};
    


    double ratios[12]{};
    double freqs[128]{};

    bool registeredMTS{false};
    std::atomic<bool> repushTuning{false};
    
    // 0 = east, 1 = west, 2 = north, 3 = south, 4 = return home
    int shiftCCs[5] = {5, 6, 7, 8, 9};
    bool hold[5] = {false, false, false, false, false};
    int careful[5] = {0, 0, 0, 0, 0};
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JIMTSSourceProcessor)
};
