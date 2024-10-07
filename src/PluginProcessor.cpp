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
        setupPyth12();
        setupDuodene();
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



void LatticesProcessor::setupPyth12()
{
    if (currentlyDuodene)
    {
        for (int i = 0; i < 12; ++i)
        {
            if (i == NM[0] || i == NM[1] || i == NM[2] || i==NM[3])
            {
                ratios[i] *= jim.comma(jim.syntonic, false);
                coOrds[i].first -= 4;
                coOrds[i].second +=1;
            }
            if (i == SM[0] || i == SM[1] || i == SM[2] || i == SM[3])
            {
                ratios[i] *= jim.comma(jim.syntonic, true);
                coOrds[i].first += 4;
                coOrds[i].second -= 1;
            }

        }
    }
    
    /*
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
    */
}

void LatticesProcessor::setupDuodene()
{
    if (currentlyDuodene)
    {
        return;
    }
    
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
        int A = coOrds[i].first;
        int B = coOrds[i].second;
        
        std::cout << "(" << A << ", " << B << ")" << std::endl;
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
/*
void LatticesProcessor::shiftLegacy(int dir) // 0 = east, 1 = west, 2 = north, 3 = south
{

    switch (dir)
    {
        case West:
            --positionX;
            updateCoords();
            currentRefNote += 5;
            currentRefFreq *= ratios[5];
            std::cout << "I Went West" << std::endl;
            break;
        case East:
            ++positionX;
            updateCoords();
            currentRefNote += 7;
            currentRefFreq *= ratios[7];
            std::cout << "I Went East" << std::endl;
            break;
        case North:
            ++positionY;
            updateCoords();
            currentRefNote += 4;
            currentRefFreq *= ratios[4];
            std::cout << "I Went North" << std::endl;
            break;
        case South:
            --positionY;
            updateCoords();
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
    changed = true;
}*/

void LatticesProcessor::shift(int dir) // 0 = east, 1 = west, 2 = north, 3 = south
{
    bool mode = false; // for when we eventually do the Duodene modes
    
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
            
//            if (mode)
//            {
//                if (i == 2)
//                {
//                    coOrds[EM[i]].second -= 3;
//                    ratios[EM[i]] *= jim.comma(jim.diesis, false);
//                }
//                // rotate somehow?
//            }
            
            std::cout << "I Went West" << std::endl;
            std::cout << "WM = " << WM[0] << WM[1] << WM[2] << std::endl;
            std::cout << "EM = " << EM[0] << EM[1] << EM[2] << std::endl;
            std::cout << "NM = " << NM[0] << NM[1] << NM[2] << NM[3] << std::endl;
            std::cout << "SM = " << SM[0] << SM[1] << SM[2] << SM[3] << std::endl;
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
            
//            if (mode)
//            {
//
//                coOrds[WM[i]].second += 3;
//                ratios[WM[i]] *= jim.comma(jim.diesis, true);
//                // rotate somehow?
//            }

            
            std::cout << "I Went East" << std::endl;
            std::cout << "WM = " << WM[0] << WM[1] << WM[2] << std::endl;
            std::cout << "EM = " << EM[0] << EM[1] << EM[2] << std::endl;
            std::cout << "NM = " << NM[0] << NM[1] << NM[2] << NM[3] << std::endl;
            std::cout << "SM = " << SM[0] << SM[1] << SM[2] << SM[3] << std::endl;
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

            
            std::cout << "I Went North" << std::endl;
            std::cout << "WM = " << WM[0] << WM[1] << WM[2] << std::endl;
            std::cout << "EM = " << EM[0] << EM[1] << EM[2] << std::endl;
            std::cout << "NM = " << NM[0] << NM[1] << NM[2] << NM[3] << std::endl;
            std::cout << "SM = " << SM[0] << SM[1] << SM[2] << SM[3] << std::endl;
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

            std::cout << "I Went South" << std::endl;
            std::cout << "WM = " << WM[0] << WM[1] << WM[2] << std::endl;
            std::cout << "EM = " << EM[0] << EM[1] << EM[2] << std::endl;
            std::cout << "NM = " << NM[0] << NM[1] << NM[2] << NM[3] << std::endl;
            std::cout << "SM = " << SM[0] << SM[1] << SM[2] << SM[3] << std::endl;
            break;
        case Home:
            returnToOrigin();
            break;
    }
    updateTuning();
    changed = true;
}


void LatticesProcessor::returnToOrigin()
{
    std::cout << "I Went Home or whatever" << std::endl;
    currentRefNote = originalRefNote;
    currentRefFreq = originalRefFreq;
    
    
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
        ratios[i] = OGratios[i];
        coOrds[i].first = OGcoOrds[i].first;
        coOrds[i].second = OGcoOrds[i].second;
    }

    
    
    setupDuodene();
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
