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
//    addParameter(positionX = new juce::AudioParameterInt{"posX", "PositionX", -128, 128, 0});
//    addParameter(positionY = new juce::AudioParameterInt{"posY", "PositionY", -128, 128, 0});
    
    
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
    
    if (mode == Pyth)
    {
        // we're done
        return;
    }
    
    {
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
    
    if (mode == Pyth)
    {
        shiftPyth(dir);
    }
    else if (mode == Syntonic)
    {
        shiftSyntonic(dir);
    }
    else
    {
        shiftDuodene(dir);
    }
}

void LatticesProcessor::shiftPyth(int dir)
{
    int d = coOrds[0].first;
    int w = 0;
    
    switch (dir)
    {
        case West:
            positionX--;
            currentRefNote += 5;
            currentRefFreq *= ratios[5];
            
            for (int i = 0; i < 12; ++i)
            {
                if (coOrds[i].first > d)
                {
                    d = coOrds[i].first;
                    w = i;
                }
            }
            coOrds[w].first -= 12;
            
            break;
        case East:
            positionX++;
            currentRefNote += 7;
            currentRefFreq *= ratios[7];
            
            for (int i = 0; i < 12; ++i)
            {
                if (coOrds[i].first < d)
                {
                    d = coOrds[i].first;
                    w = i;
                }
            }
            coOrds[w].first += 12;
            
            break;
        case Home:
            returnToOrigin();
            break;
        default:
            return;
    }
    
    if (positionX != 0)
    {
        if (currentRefNote >= originalRefNote + 12)
        {
            currentRefNote -= 12;
            currentRefFreq /= 2.0;
        }
        changed = true;
        updateTuning();
    }
    else if (positionX == 0 && positionY == 0)
    {
        returnToOrigin();
    }
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
            duodeneCoords();
            currentRefNote += 5;
            currentRefFreq *= ratios[5];
            break;
        case East:
            ++positionX;
            duodeneCoords();
            currentRefNote += 7;
            currentRefFreq *= ratios[7];
            break;
        case North:
            ++positionY;
            duodeneCoords();
            currentRefNote += 4;
            currentRefFreq *= ratios[4];
            break;
        case South:
            --positionY;
            duodeneCoords();
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
        changed = true;
        updateTuning();
    }
    else if (positionX == 0 && positionY == 0)
    {
        returnToOrigin();
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
    currentRefNote = originalRefNote;
    currentRefFreq = originalRefFreq;
    positionX = 0;
    positionY = 0;
    setup();
    
    updateTuning();
    changed = true;
}

void LatticesProcessor::updateTuning()
{
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

//void LatticesProcessor::pythCoords()
//{
//    coOrds[0].first = positionX;
//    coOrds[1].first = positionX - 5;
//    coOrds[2].first = positionX + 2;
//    coOrds[3].first = positionX - 3;
//    coOrds[4].first = positionX + 4;
//    coOrds[5].first = positionX - 1;
//    coOrds[6].first = positionX + 6;
//    coOrds[7].first = positionX + 1;
//    coOrds[8].first = positionX - 4;
//    coOrds[9].first = positionX + 3;
//    coOrds[10].first = positionX - 2;
//    coOrds[11].first = positionX + 5;
//
//    for (int i = 0; i < 12; ++i)
//    {
//        coOrds[i].second = 0;
//    }
//}

template<std::size_t S>
inline void LatticesProcessor::rotate(std::array<int, S>& arr, bool backwards)
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
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LatticesProcessor();
}
