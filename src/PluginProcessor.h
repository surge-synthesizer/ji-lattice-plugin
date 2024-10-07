/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <set>


class LatticesProcessor : public juce::AudioProcessor, juce::Timer
{
public:
    //==============================================================================
  LatticesProcessor();
    ~LatticesProcessor();

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
    
    void initCoords();
    void setupPyth12();
    void setupDuodene();
    template<std::size_t S>
    void rotate(std::array<int, S>& arr, bool backwards = false);
    
    // useful?
    std::array<int,3> WM = {1,5,9}; // WestMost in Duodene
    std::array<int,3> EM = {10,2,6}; // EastMost in Duodene
    std::array<int,4> NM = {9,4,11,6}; // Northmost in Duodene, WestMost in Pyth
    std::array<int,4> SM = {1,8,3,10}; // Southmost in Duodene, EastMost in Pyth
    
    int W[5] = {1,3,5,8,10}; // West side in Pyth
    int E[6] = {2,4,6,7,9,11}; // East side in Pyth
    
    
    int OGWM[3] = {1,5,9};
    int OGEM[3] = {10,2,6};
    int OGNM[4] = {9,4,11,6};
    int OGSM[4] = {1,8,3,10};
    
    
    void shift(int dir);

    void returnToOrigin();
    void updateTuning();
    
    int positionX{0};
    int positionY{0};

    
    bool currentlyPythagorean{true};
    bool currentlyDuodene{false};
    std::pair<int, int> coOrds[12]
    {
        {0, 0},
        {-5, 0},
        {2, 0},
        {-3, 0},
        {4, 0},
        {-1, 0},
        {6, 0},
        {1, 0},
        {-4, 0},
        {3, 0},
        {-2, 0},
        {5, 0}
    };
    
    std::pair<int, int> OGcoOrds[12]
    {
        {0, 0},
        {-5, 0},
        {2, 0},
        {-3, 0},
        {4, 0},
        {-1, 0},
        {6, 0},
        {1, 0},
        {-4, 0},
        {3, 0},
        {-2, 0},
        {5, 0}
    };
    
    int syntonicDrift = 0;
    int diesisDrift = 0;
    
    
    std::atomic<bool> changed{false};
    
    std::atomic<int> numClients{0};
private:
    
    int defaultRefNote{0};
    double defaultRefFreq{261.6255653005986}; // overkill precision
    
    int originalRefNote{2}; // C = 0, C# = 1, D = 2 etc
    int currentRefNote{-12};
    double originalRefFreq{293.3333333333333};
    double currentRefFreq{1.00000000};
   
    
    double ratios[12]
    {
        1.0,
        (double)256/243,
        (double)9/8,
        (double)32/27,
        (double)81/64,
        (double)4/3,
        (double)729/512,
        (double)3/2,
        (double)128/81,
        (double)27/16,
        (double)16/9,
        (double)243/128,
    };
    
    double OGratios[12]
    {
        1.0,
        (double)256/243,
        (double)9/8,
        (double)32/27,
        (double)81/64,
        (double)4/3,
        (double)729/512,
        (double)3/2,
        (double)128/81,
        (double)27/16,
        (double)16/9,
        (double)243/128,
    };
    double freqs[128]{};

    bool registeredMTS{false};
    std::atomic<bool> repushTuning{false};
    
    enum Direction
    {
        West,
        East,
        North,
        South,
        Home
    };
    int shiftCCs[5] = {6, 5, 7, 8, 9};
    bool hold[5] = {false, false, false, false, false};
    int careful[5] = {0, 0, 0, 0, 0};
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LatticesProcessor)
};
