/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "libMTSMaster.h"

#include <cmath>
#include <iostream>


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
        std::cout << "Registered ok" << std::endl;
    }
    /*
     TODO:
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
    
    if (registeredMTS == true) //TODO: State loading
    {
        mode = Duodene;
        currentRefFreq = originalRefFreq;
        currentRefNote = originalRefNote;
        setup();
        updateTuning();
        startTimer(50);
    }
}

void LatticesProcessor::setup()
{
    // Init to pythagorean
    // I could do that programmatically instead but whatever...
    for (int i = 0; i < 12; ++i)
    {
        ratios[i] = pyth12[i];
        coOrds[i] = pythCo[i];
    }
    
    if (mode == Pyth)
    {
        // we're done
        return;
    }
    
    // else add prime 5 to taste
    for (int i = 0; i < 12; ++i)
    {
        if (i == NM[0] || i == NM[1] || i == NM[2] || i==NM[3])
        {
            ratios[i] *= jim.comma(jim.syntonic, true);
            coOrds[i].first -= 4;
            coOrds[i].second += 1;
        }
        if (i == SM[0] || i == SM[1] || i == SM[2] || i == SM[3])
        {
            ratios[i] *= jim.comma(jim.syntonic, false);
            coOrds[i].first += 4;
            coOrds[i].second -= 1;
        }
    }
}

LatticesProcessor::~LatticesProcessor()
{
    if (registeredMTS)
        MTS_DeregisterMaster();
}

//==============================================================================
void LatticesProcessor::prepareToPlay(double sampleRate, int samplesPerBlock){}
void LatticesProcessor::releaseResources() {}
bool LatticesProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {return true;}
const juce::String LatticesProcessor::getName() const {return JucePlugin_Name;}
bool LatticesProcessor::acceptsMidi() const {return true;}
bool LatticesProcessor::producesMidi() const {return false;}
bool LatticesProcessor::isMidiEffect() const {return false;}
double LatticesProcessor::getTailLengthSeconds() const {return 0.0;}
int LatticesProcessor::getNumPrograms() {return 1;}
int LatticesProcessor::getCurrentProgram() {return 0;}
void LatticesProcessor::setCurrentProgram(int index) {}
const juce::String LatticesProcessor::getProgramName(int index){return {};}
void LatticesProcessor::changeProgramName(int index, const juce::String& newName){}
bool LatticesProcessor::hasEditor() const {return true;}
juce::AudioProcessorEditor* LatticesProcessor::createEditor()
{
    return new LatticesEditor(*this);
}
//==============================================================================

void LatticesProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    
}

void LatticesProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    
}

//==============================================================================

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
    if (m.isController() && m.getChannel() == listenOnChannel)
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
                    wait[i] = true;
                }
            }
        }
    }
}

void LatticesProcessor::timerCallback()
{
    for (int i = 0; i < 5; ++i)
    {
        if (wait[i])
        {
            wait[i] = false;
            hold[i] = false;
        }
    }
}

void LatticesProcessor::modeSwitch(int m)
{
    switch (m)
    {
        case Pyth:
            mode = Pyth;
            break;
        case Syntonic:
            mode = Syntonic;
            break;
        case Duodene:
            mode = Duodene;
    }
    returnToOrigin();
}

void LatticesProcessor::shift(int dir)
{
    switch (dir)
    {
        case West:
            --positionX;
            
            if (mode == Syntonic)
            {
                syntShape = ((positionX % 4) + 4) % 4;
                
                if (syntShape == 3)
                {
                    ++positionY;
                }
            }
            break;
        case East:
            ++positionX;
            if (mode == Syntonic)
            {
                syntShape = ((positionX % 4) + 4) % 4;
                if (syntShape == 0)
                {
                    --positionY;
                }
            }
            break;
        case North:
            if (mode == Pyth) return;
            ++positionY;
            break;
        case South:
            if (mode == Pyth) return;
            --positionY;
            break;
        case Home:
            returnToOrigin();
            break;
        default:
            return;
    };
    
    
    if (mode == Pyth)
    {
        shiftPyth();
    }
    else
    {
        shiftDuodene();
    }
    
    updateTuning();
}

void LatticesProcessor::shiftPyth()
{
    if (positionX == 0)
    {
        returnToOrigin();
    }

    int nn = originalRefNote;
    double nf = 1.0;
    
    int absx = std::abs(positionX);
    double mul = positionX < 0 ? 1 / ratios[7] : ratios[7];
    int add = positionX < 0 ? -7 : 7;
    for (int i = 0; i < absx; ++i)
    {
        nn += add;
        nf *= mul;
    }
    
    while (nn < 0)
    {
        nn += 12;
        nf *= 2.0;
    }
    while (nn > 12)
    {
        nn -= 12;
        nf *= 0.5;
    }
    
    currentRefNote = nn;
    currentRefFreq = originalRefFreq * nf;
    pythCoords();
}

void LatticesProcessor::shiftDuodene()
{
    if (positionX == 0 && positionY == 0)
    {
       returnToOrigin();
       return;
    }
    
    int nn = originalRefNote;
    double nf = 1.0;
    
    int absx = std::abs(positionX);
    double mul = positionX < 0 ? 1 / ratios[7] : ratios[7];
    int add = positionX < 0 ? -7 : 7;
    for (int i = 0; i < absx; ++i)
    {
        nn += add;
        nf *= mul;
    }
    double third = 1.25;
    
    int absy = std::abs(positionY);
    mul = positionY < 0 ? 1 / third : third;
    add = positionY < 0 ? -4 : 4;
    for (int i = 0; i < absy; ++i)
    {
        nn += add;
        nf *= mul;
    }
    
    while (nn < 0)
    {
        nn += 12;
        nf *= 2.0;
    }
    while (nn > 12)
    {
        nn -= 12;
        nf *= 0.5;
    }
    
    currentRefNote = nn;
    currentRefFreq = originalRefFreq * nf;
    
    if (mode == Syntonic)
        setShape();
    
    duodeneCoords();
}
void LatticesProcessor::setShape()
{
    for (int i = 0; i < 12; ++i)
    {
        if (syntShape == 1) ratios[i] = synt1[i];
        else if (syntShape == 2) ratios[i] = synt2[i];
        else if (syntShape == 3) ratios[i] = synt3[i];
        else ratios[i] = duodene[i];
    }
}

void LatticesProcessor::updateFreq(double f)
{
    originalRefFreq = f;
    returnToOrigin();
}

double LatticesProcessor::updateRoot(int r)
{
    double nf = freqs[60 + r];
    
    originalRefNote = r;
    originalRefFreq = nf;
    returnToOrigin();
    
    return nf;
}

void LatticesProcessor::returnToOrigin()
{
    syntShape = 0;
    currentRefNote = originalRefNote;
    currentRefFreq = originalRefFreq;
    positionX = 0;
    positionY = 0;
    setup();
    
    updateTuning();
}

void LatticesProcessor::updateTuning()
{
    changed = true;
//    std::cout << "refFreq = " << currentRefFreq << std::endl;
//    std::cout << "refNote = " << currentRefNote << std::endl;
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

void LatticesProcessor::pythCoords()
{
    coOrds[0].first = positionX;
    coOrds[1].first = positionX - 5;
    coOrds[2].first = positionX + 2;
    coOrds[3].first = positionX - 3;
    coOrds[4].first = positionX + 4;
    coOrds[5].first = positionX - 1;
    coOrds[6].first = positionX + 6;
    coOrds[7].first = positionX + 1;
    coOrds[8].first = positionX - 4;
    coOrds[9].first = positionX + 3;
    coOrds[10].first = positionX - 2;
    coOrds[11].first = positionX + 5;

    for (int i = 0; i < 12; ++i)
    {
        coOrds[i].second = 0;
    }
}

inline void LatticesProcessor::duodeneCoords()
{
    coOrds[0].first = positionX;
    coOrds[0].second = positionY;

    coOrds[1].first = positionX - 1;
    coOrds[1].second = positionY - 1;

    coOrds[2].first = positionX + 2;
    coOrds[2].second = positionY;

    coOrds[3].first = positionX + 1;
    coOrds[3].second = positionY - 1;

    coOrds[4].first = positionX;
    coOrds[4].second = positionY + 1;

    coOrds[5].first = positionX - 1;
    coOrds[5].second = positionY;

    coOrds[6].first = positionX + 2;
    coOrds[6].second = positionY + 1;

    coOrds[7].first = positionX + 1;
    coOrds[7].second = positionY;

    coOrds[8].first = positionX;
    coOrds[8].second = positionY - 1;

    coOrds[9].first = positionX - 1;
    coOrds[9].second = positionY + 1;

    coOrds[10].first = positionX + 2;
    coOrds[10].second = positionY - 1;

    coOrds[11].first = positionX + 1;
    coOrds[11].second = positionY + 1;
    
    if (mode == Syntonic)
    {
        for (int i = 0; i < syntShape; ++i)
        {
            coOrds[shapeChange[i]].second -= 3;
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LatticesProcessor();
}
