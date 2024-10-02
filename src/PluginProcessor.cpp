/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "libMTSMaster.h"
#include "JIMath.h"

#include <cmath>



//==============================================================================
JIMTSSourceProcessor::JIMTSSourceProcessor()
    : juce::AudioProcessor (juce::AudioProcessor::BusesProperties()
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
        )
{
    if (MTS_CanRegisterMaster())
    {
        registeredMTS = true;
        repushTuning = true;
        MTS_RegisterMaster();
    }
    /*
    else
    {
        if (MTS_HasIPC())
        {
            Warn user another master is already connected, but provide an option to reinitialize MTS-ESP in case there was a crash and no master is connected any more;
            if (user clicks to reinitialize MTS-ESP)
            {
                MTS_Reinitialize();
                MTS_RegisterMaster();
            }
        }
        else
           Warn user another master is already connected, do not provide an option to reinitialize MTS-ESP;
    }
    */
    
    startTimer(10);
    
    if (registeredMTS == true)
    {
        currentRefFreq = originalRefFreq;
        currentRefNote = originalRefNote;
        initDuodene();
        updateTuning();
    }
}

JIMTSSourceProcessor::~JIMTSSourceProcessor()
{
    if (registeredMTS)
        MTS_DeregisterMaster();
}

//==============================================================================
const juce::String JIMTSSourceProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JIMTSSourceProcessor::acceptsMidi() const
{
    return true;
}

bool JIMTSSourceProcessor::producesMidi() const
{
    return false;
}

bool JIMTSSourceProcessor::isMidiEffect() const
{
    return false;
}

double JIMTSSourceProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JIMTSSourceProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int JIMTSSourceProcessor::getCurrentProgram()
{
    return 0;
}

void JIMTSSourceProcessor::setCurrentProgram (int index)
{
}

const juce::String JIMTSSourceProcessor::getProgramName (int index)
{
    return {};
}

void JIMTSSourceProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void JIMTSSourceProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void JIMTSSourceProcessor::timerCallback()
{
    for (int i = 0; i < 4; ++i)
    {
        if (careful[i] > 0)
        {
            careful[i]--;
        }
    }
}

void JIMTSSourceProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool JIMTSSourceProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}

void JIMTSSourceProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (!registeredMTS)
        return;
    
    numClients = MTS_GetNumClients();
    
    for (const auto metadata : midiMessages)
    {
        respondToMidi(metadata.getMessage());
    }
}

void JIMTSSourceProcessor::respondToMidi(const juce::MidiMessage &m)
{
    if (m.isController() && m.getChannel() == 1)
    {
        auto num = m.getControllerNumber();
        auto val = m.getControllerValue();
        
        for (int i = 0; i < 5; ++i)
        {
            if (num == shiftCCs[i])
            {
                if (val == 127 && hold[i] == false)
                {
                    shift(i);
                    hold[i] = true;
                }
                
                if (val < 127 && hold[i] == true)
                {
                    hold[i] = false;
                    careful[i] = 5;
                }
            }
        }
    }
}

//==============================================================================
bool JIMTSSourceProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor*JIMTSSourceProcessor::createEditor()
{
    return new JIMTSSourceEditor(*this);
}

//==============================================================================
void JIMTSSourceProcessor::getStateInformation (juce::MemoryBlock& destData) {}

void JIMTSSourceProcessor::setStateInformation (const void* data, int sizeInBytes) {}

void JIMTSSourceProcessor::initPythagorean12()
{
    ratios[0] = 1.0;
    ratios[1] = (double)256/243;
    ratios[2] = (double)9/8;
    ratios[3] = (double)32/27;
    ratios[4] = (double)81/64;
    ratios[5] = (double)4/3;
    ratios[6] = (double)729/512;
    ratios[7] = (double)3/2;
    ratios[8] = (double)128/81;
    ratios[9] = (double)27/16;
    ratios[10] = (double)16/9;
    ratios[11] = (double)243/128;
}

void JIMTSSourceProcessor::initDuodene()
{
    ratios[0] = 1.0;
    ratios[1] = (double)16/15;
    ratios[2] = (double)9/8;
    ratios[3] = (double)6/5;
    ratios[4] = (double)5/4;
    ratios[5] = (double)4/3;
    ratios[6] = (double)45/32;
    ratios[7] = (double)3/2;
    ratios[8] = (double)8/5;
    ratios[9] = (double)5/3;
    ratios[10] = (double)9/5;
    ratios[11] = (double)15/8;
}



void JIMTSSourceProcessor::shift(int dir) // 0 = east, 1 = west, 2 = north, 3 = south
{

    switch (dir)
    {
        case East:
            positionX++;
            currentRefNote += 7;
            currentRefFreq *= ratios[7];
            std::cout << "I Went East" << std::endl;
            break;
        case West:
            positionX--;
            currentRefNote += 5;
            currentRefFreq *= ratios[5];
            std::cout << "I Went West" << std::endl;
            break;
        case North:
            positionY++;
            currentRefNote += 4;
            currentRefFreq *= ratios[4];
            std::cout << "I Went North" << std::endl;
            break;
        case South:
            positionY--;
            currentRefNote += 8;
            currentRefFreq *= ratios[8];
            std::cout << "I Went South" << std::endl;
            break;
        case Home:
            returnToOrigin();
            break;
    }
    
    if (positionX != 0 || positionY != 0)
    {
        if (currentRefNote >= originalRefNote + 12)
        {
            currentRefNote -= 12;
            currentRefFreq /= 2.0;
        }
        updateTuning();
    }
    else if (positionX == 0 && positionY == 0)
    {
        returnToOrigin();
    }
}

void JIMTSSourceProcessor::returnToOrigin()
{
    std::cout << "I Went Home" << std::endl;
    positionX = 0;
    positionY = 0;
    currentRefNote = originalRefNote;
    currentRefFreq = originalRefFreq;
    
    // initDuodene(); no need for now
    updateTuning();
}

void JIMTSSourceProcessor::updateTuning()
{
    std::cout << "refFreq = " << currentRefFreq << std::endl;
    std::cout << "refNote = " << currentRefNote << std::endl;
    int refMidiNote = currentRefNote + 60;
    for (int note = 0; note < 128; ++note)
    {
        double octaveShift = std::pow(2, std::floor(((double)note - refMidiNote) / 12.0));
        
        int degree = (note - refMidiNote) % 12;
        if (degree < 0) {degree += 12;}
        
        freqs[note] = currentRefFreq * ratios[degree] * octaveShift;
    }
    
    MTS_SetNoteTunings(freqs);
    
    // later...
    MTS_SetScaleName("JI is nice yeah?");
}

std::pair<int,int> JIMTSSourceProcessor::checkLocation()
{
    std::cout << "I checked the position" << std::endl;
    return {positionX, positionY};
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JIMTSSourceProcessor();
}
