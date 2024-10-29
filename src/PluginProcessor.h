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
    
    int positionX{0};
    int positionY{0};
    
    int syntonicDrift = 0;
    int diesisDrift = 0;
    
    std::pair<int, int> coOrds[12]{};
    
    enum Mode
    {
        Pyth,
        Syntonic,
        Duodene,
    };
    std::atomic<Mode> mode = Duodene;
    
    void modeSwitch(int m);
    
    std::atomic<bool> changed{false};
    std::atomic<int> numClients{0};
    
    
    bool registeredMTS{false};
    
    int shiftCCs[5] = {5, 6, 7, 8, 9};
    int listenOnChannel = 1;
    
    int originalRefNote{2}; // C = 0, C# = 1, D = 2 etc
    double originalRefFreq{293.3333333333333};
    
    
    void updateFreq(double f);
    double updateRoot(int r);
    
private:
    
    int currentRefNote{-12};
    double currentRefFreq{1.00000000};
    
    enum Direction
    {
        West,
        East,
        North,
        South,
        Home
    };
    
    void setup();
    

    
    void shift(int dir);
    void shiftPyth(int dir);
    void shiftDuodene(int dir);
    void shiftSyntonic(int dir);
    
    void returnToOrigin();
    void updateTuning();
    
    template<std::size_t S>
    void rotate(std::array<int, S>& arr, bool backwards = false);
    
    int defaultRefNote{0};
    double defaultRefFreq{261.6255653005986}; // overkill precision
    
    double ratios[12] = {};
    double pyth12[12]
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
    std::pair<int, int> pythCo[12]
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
    double freqs[128]{};
    
    // These ints refer to 0-indexed scale degrees
    // They specify which notes are on the edges of the Duodene scale
    // We need that in Syntonic mode, see shiftSyntonic() for implementation
    std::array<int,3> WM = {1,5,9}; // WestMost
    std::array<int,3> EM = {10,2,6}; // EastMost
    std::array<int,4> NM = {9,4,11,6}; // Northmost in Duodene
    std::array<int,4> SM = {1,8,3,10}; // Southmost in Duodene
    
    int W[5] = {1,3,5,8,10}; // West side in Pyth
    int E[6] = {2,4,6,7,9,11}; // East side in Pyth
    
    int OGWM[3] = {1,5,9};
    int OGEM[3] = {10,2,6};
    int OGNM[4] = {9,4,11,6};
    int OGSM[4] = {1,8,3,10};
    
    
    bool hold[5] = {false, false, false, false, false};
    int careful[5] = {0, 0, 0, 0, 0};
//    int priorCC[5]{};
//    int priorChannel{};
    bool first = true;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LatticesProcessor)
};
