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
LatticesProcessor::LatticesProcessor()
    : juce::AudioProcessor (juce::AudioProcessor::BusesProperties()
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
        )
{
    if (MTS_CanRegisterMaster())
    {
        registeredMTS = true;
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
        mode = Duodene;
        currentRefFreq = originalRefFreq;
        currentRefNote = originalRefNote;
        setup();
        updateTuning();
    }
}

LatticesProcessor::~LatticesProcessor()
{
    if (registeredMTS)
        MTS_DeregisterMaster();
}

//==============================================================================
const juce::String LatticesProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LatticesProcessor::acceptsMidi() const
{
    return true;
}

bool LatticesProcessor::producesMidi() const
{
    return false;
}

bool LatticesProcessor::isMidiEffect() const
{
    return false;
}

double LatticesProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LatticesProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LatticesProcessor::getCurrentProgram()
{
    return 0;
}

void LatticesProcessor::setCurrentProgram (int index)
{
}

const juce::String LatticesProcessor::getProgramName (int index)
{
    return {};
}

void LatticesProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LatticesProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void LatticesProcessor::timerCallback()
{
    for (int i = 0; i < 4; ++i)
    {
        if (careful[i] > 0)
        {
            careful[i]--;
        }
    }
}

void LatticesProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool LatticesProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}

void LatticesProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();
    
    if (!registeredMTS)
        return;
    
    numClients = MTS_GetNumClients();
    
    for (const auto metadata : midiMessages)
    {
        respondToMidi(metadata.getMessage());
    }
}

void LatticesProcessor::respondToMidi(const juce::MidiMessage &m)
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
                    if (mode == Pyth)
                    {
                        shiftPyth(i);
                    }
                    else if (mode == Syntonic)
                    {
                        shiftSyntonic(i);
                    }
                    else
                    {
                        shiftDuodene(i);
                    }
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
bool LatticesProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LatticesProcessor::createEditor()
{
    return new LatticesEditor(*this);
}

//==============================================================================
void LatticesProcessor::getStateInformation (juce::MemoryBlock& destData) {}

void LatticesProcessor::setStateInformation (const void* data, int sizeInBytes) {}

//==============================================================================

JIMath jim;

void LatticesProcessor::setup()
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
    
    
    coOrds[0] = {0, 0};
    coOrds[1] = {-5, 0};
    coOrds[2] = {2, 0};
    coOrds[3] =  {-3, 0};
    coOrds[4] = {4, 0};
    coOrds[5] =  {-1, 0};
    coOrds[6] = {6, 0};
    coOrds[7] =  {1, 0},
    coOrds[8] =  {-4, 0};
    coOrds[9] =  {3, 0};
    coOrds[10] =  {-2, 0};
    coOrds[11] =  {5, 0};
    
    
    if (mode != Pyth)
    {
        for (int i = 0; i < 12; ++i)
        {
            if (i == NM[0] || i == NM[1] || i == NM[2] || i==NM[3])
            {
                ratios[i] *= jim.comma(jim.syntonic, true);
                coOrds[i].first -= 4;
                coOrds[i].second +=1;
            }
            if (i == SM[0] || i == SM[1] || i == SM[2] || i == SM[3])
            {
                ratios[i] *= jim.comma(jim.syntonic, false);
                coOrds[i].first += 4;
                coOrds[i].second -= 1;
            }
        }
    }
    
    /*
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
    */
    
}



void LatticesProcessor::shiftPyth(int dir)
{
}

void LatticesProcessor::shiftSyntonic(int dir)
{
    switch (dir)
    {
        case West:
            for (int i = 0; i < 3; ++i)
            {
                if (EM[i] == 0) syntonicDrift--;
                coOrds[EM[i]].first -= 4;
                coOrds[EM[i]].second += 1;
                ratios[EM[i]] *= jim.comma(jim.syntonic, true);

                WM[i] = EM[i];
                EM[i] = (EM[i] + 5) % 12;
            }
            rotate(SM);
            rotate(NM);
            break;
            
        case East:
            for (int i = 0; i < 3; ++i)
            {
                if (WM[i] == 0) syntonicDrift++;
                coOrds[WM[i]].first += 4;
                coOrds[WM[i]].second -= 1;
                ratios[WM[i]] *= jim.comma(jim.syntonic, false);
                
                EM[i] = WM[i];
                WM[i] = (WM[i] + 7) % 12;
            }
            rotate(SM, true);
            rotate(NM, true);
            break;

        case North:
            for (int i = 0; i < 4; ++i)
            {
                if (SM[i] == 0) diesisDrift--;
                coOrds[SM[i]].second += 3;
                ratios[SM[i]] *= jim.comma(jim.diesis, true);
                
                NM[i] = SM[i];
                SM[i] = (SM[i] + 4) % 12;
            }
            rotate(WM, true);
            rotate(EM, true);
            break;
            
        case South:
            for (int i = 0; i < 4; ++i)
            {
                if (NM[i] == 0) diesisDrift++;
                coOrds[NM[i]].second -= 3;
                ratios[NM[i]] *= jim.comma(jim.diesis, false);
                
                SM[i] = NM[i];
                NM[i] = (NM[i] + 8) % 12;
            }
            rotate(WM);
            rotate(EM);
            break;
        case Home:
            returnToOrigin();
            break;
    }
    updateTuning();
    changed = true;
}

void LatticesProcessor::shiftDuodene(int dir)
{
    switch (dir)
    {
        case West:
            --positionX;
            currentRefNote += 5;
            currentRefFreq *= ratios[5];
            break;
        case East:
            ++positionX;
            currentRefNote += 7;
            currentRefFreq *= ratios[7];
            break;
        case North:
            ++positionY;
            currentRefNote += 4;
            currentRefFreq *= ratios[4];
            break;
        case South:
            --positionY;
            currentRefNote += 8;
            currentRefFreq *= ratios[8];
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
    
    changed = true;
}


void LatticesProcessor::returnToOrigin()
{
    currentRefNote = originalRefNote;
    currentRefFreq = originalRefFreq;
    
    setup();
    for (int i = 0; i < 12; ++i)
    {
        if (i < 3)
        {
            WM[i] = OGWM[i];
            EM[i] = OGEM[i];
        }
        if (i < 4)
        {
            NM[i] = OGNM[i];
            SM[i] = OGSM[i];
        }
    }
    
    updateTuning();
    changed = true;
}

void LatticesProcessor::updateTuning()
{
    // std::cout << "refFreq = " << currentRefFreq << std::endl;
    // std::cout << "refNote = " << currentRefNote << std::endl;
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

template<std::size_t S>
void LatticesProcessor::rotate(std::array<int, S>& arr, bool backwards)
{
    int last = arr.size() - 1;
    
    if (backwards)
    {
        auto temp = arr[0];
        
        for (int i = 0; i < last; ++i)
        {
            arr[i] = arr[i + 1];
        }
        
        arr[last] = temp;
    }
    else
    {
        auto temp = arr[last];
        
        for (int i = last; i > 0; --i)
        {
            arr[i] = arr[i - 1];
        }
        
        arr[0] = temp;
    }
}




//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LatticesProcessor();
}
