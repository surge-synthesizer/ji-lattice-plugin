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
    : juce::AudioProcessor(juce::AudioProcessor::BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    addParameter(xParam = new juce::AudioParameterInt("px", "X Position", -8, 8, 0));
    addParameter(yParam = new juce::AudioParameterInt("py", "Y Position", -8, 8, 0));
    
    xParam->addListener(this);
    yParam->addListener(this);
    
    
    
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
        returnToOrigin();
        startTimer(50);
    }
}

LatticesProcessor::~LatticesProcessor()
{
    xParam->removeListener(this);
    yParam->removeListener(this);
    
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
void LatticesProcessor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting){}
//==============================================================================

void LatticesProcessor::getStateInformation(juce::MemoryBlock& destData)
{
//    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("Lattices"));
//   xml->setAttribute("streamingVersion", (int)8524);
//
//    xml->setAttribute("Mode", mode);
//    xml->setAttribute("X", positionX);
//    xml->setAttribute("Y", positionY);
//
//    copyXmlToBinary(*xml, destData);
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
    
    xParam->beginChangeGesture();
    xParam->setValueNotifyingHost(0.5);
    xParam->endChangeGesture();
    yParam->beginChangeGesture();
    yParam->setValueNotifyingHost(0.5);
    yParam->endChangeGesture();

    if (mode == Pyth)
    {
        for (int i = 0; i < 12; ++i)
        {
            ratios[i] = pyth12[i];
            coOrds[i].first = pythCo[i];
            coOrds[i].second = 0;
        }
    }
    else
    {
        for (int i = 0; i < 12; ++i)
        {
            ratios[i] = duo12[i];
            coOrds[i] = duoCo[i];
        }
    }
    
    updateTuning();
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

void LatticesProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
    locate();
}

void LatticesProcessor::shift(int dir)
{
    float X = xParam->get();
    float Y = yParam->get();
    
    switch (dir)
    {
        case West:
            xParam->beginChangeGesture();
            --X;
            X += 8;
            X /= 16;
            xParam->setValueNotifyingHost(X);
            xParam->endChangeGesture();
            locate();
            break;
        case East:
            xParam->beginChangeGesture();
            ++X;
            X += 8;
            X /= 16;
            xParam->setValueNotifyingHost(X);
            xParam->endChangeGesture();
            locate();
            break;
        case North:
            if (mode == Pyth) return;
            yParam->beginChangeGesture();
            ++Y;
            Y += 8;
            Y /= 16;
            yParam->setValueNotifyingHost(Y);
            yParam->endChangeGesture();
            locate();
            break;
        case South:
            if (mode == Pyth) return;
            yParam->beginChangeGesture();
            --Y;
            Y += 8;
            Y /= 16;
            yParam->setValueNotifyingHost(Y);
            yParam->endChangeGesture();
            locate();
            break;
        case Home:
            returnToOrigin();
            break;
    };
}
    
void LatticesProcessor::locate()
{
    positionX = xParam->get();
    positionY = yParam->get();
    
    if (mode == Pyth)
    {

        int nn = originalRefNote;
        double nf = 1.0;
        
        int absx = std::abs(positionX);
        double mul = positionX < 0 ? 1 / 1.5 : 1.5; // fifth down : fifth up
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
        
        for (int i = 0; i < 12; ++i)
        {
            coOrds[i].first = positionX + pythCo[i];
            coOrds[i].second = 0;
        }
        
        updateTuning();
        return;
    }
    
    if (mode == Duodene || mode == Syntonic)
    {
        if (mode == Syntonic)
        {
            float quarter = static_cast<float>(positionX) / 4;
            int syntYOff = std::floor(quarter);
            syntYOff *= -1;

            positionY = yParam->get() + syntYOff;
        }
        
        int nn = originalRefNote;
        double nf = 1.0;
        
        int absx = std::abs(positionX);
        double mul = positionX < 0 ? 1 / 1.5 : 1.5; // fifth down : fifth up
        int add = positionX < 0 ? -7 : 7;
        for (int i = 0; i < absx; ++i)
        {
            nn += add;
            nf *= mul;
        }
        
        int absy = std::abs(positionY);
        mul = positionY < 0 ? 1 / 1.25 : 1.25; // third down : third up
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
        
        for (int i = 0; i < 12; ++i)
        {
            coOrds[i].first = duoCo[i].first + positionX;
            coOrds[i].second = duoCo[i].second + positionY;
        }
        
        if (mode == Syntonic)
        {
            int syntShape = ((positionX % 4) + 4) % 4;
            
            ratios[6] = (syntShape > 0) ? (double)36/25 : (double)45/32;
            ratios[11] = (syntShape > 1) ? (double)48/25 : (double)15/8;
            ratios[4] = (syntShape == 3) ? (double)32/25 : (double)5/4;
            
            coOrds[6].second = (syntShape > 0) ? positionY - 2 : positionY + 1;
            coOrds[11].second =  (syntShape > 1) ? positionY - 2 : positionY + 1;
            coOrds[4].second = (syntShape == 3) ? positionY - 2 : positionY + 1;
        }
        updateTuning();
    }
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


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LatticesProcessor();
}
