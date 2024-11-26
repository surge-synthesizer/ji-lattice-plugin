/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do note that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#include "LatticesProcessor.h"
#include "LatticesEditor.h"
#include "libMTSMaster.h"

//==============================================================================
LatticesProcessor::LatticesProcessor()
    : juce::AudioProcessor(juce::AudioProcessor::BusesProperties().withOutput(
          "Output", juce::AudioChannelSet::stereo(), true))
{
    auto distanceReadoutX = juce::AudioParameterIntAttributes{}
                                .withStringFromValueFunction(
                                    [](int value, int maximumStringLength) -> juce::String
                                    {
                                        juce::String dir{};
                                        if (value == 0)
                                        {
                                            dir = "Home";
                                            return dir;
                                        }

                                        if (value == 1)
                                        {
                                            dir = "1 Step East";
                                            return dir;
                                        }
                                        else if (value == -1)
                                        {
                                            dir = "1 Step West";
                                            return dir;
                                        }

                                        dir = std::to_string(std::abs(value)) +
                                              ((value > 1) ? " Steps East" : " Steps West");
                                        return dir;
                                    })
                                .withValueFromStringFunction(
                                    [](juce::String str)
                                    {
                                        if (str == "Home")
                                        {
                                            return 0;
                                        }

                                        if (str == "1 Step East")
                                        {
                                            return 1;
                                        }

                                        if (str == "1 Step West")
                                        {
                                            return -1;
                                        }

                                        int res{};
                                        if (str.endsWith("East"))
                                        {
                                            str = str.trimCharactersAtEnd(" Steps East");
                                            res = str.getIntValue();
                                        }

                                        if (str.endsWith("East"))
                                        {
                                            str = str.trimCharactersAtEnd(" Steps West");
                                            res = str.getIntValue();
                                            res *= -1;
                                        }

                                        return res;
                                    });

    auto distanceReadoutY = juce::AudioParameterIntAttributes{}
                                .withStringFromValueFunction(
                                    [](int value, int maximumStringLength) -> juce::String
                                    {
                                        juce::String dir{};
                                        if (value == 0)
                                        {
                                            dir = "Home";
                                            return dir;
                                        }

                                        if (value == 1)
                                        {
                                            dir = "1 Step North";
                                            return dir;
                                        }
                                        else if (value == -1)
                                        {
                                            dir = "1 Step South";
                                            return dir;
                                        }

                                        dir = std::to_string(std::abs(value)) +
                                              ((value > 1) ? " Steps North" : " Steps South");
                                        return dir;
                                    })
                                .withValueFromStringFunction(
                                    [](juce::String str)
                                    {
                                        if (str == "Home")
                                        {
                                            return 0;
                                        }

                                        if (str == "1 Step North")
                                        {
                                            return 1;
                                        }

                                        if (str == "1 Step South")
                                        {
                                            return -1;
                                        }

                                        int res{};
                                        if (str.endsWith("North"))
                                        {
                                            str = str.trimCharactersAtEnd(" Steps North");
                                            res = str.getIntValue();
                                            return res;
                                        }

                                        if (str.endsWith("South"))
                                        {
                                            str = str.trimCharactersAtEnd(" Steps South");
                                            res = str.getIntValue();
                                            res *= -1;
                                        }

                                        return res;
                                    });

    addParameter(xParam = new juce::AudioParameterInt("px", "X Position", -maxDistance, maxDistance,
                                                      0, distanceReadoutX));
    addParameter(yParam = new juce::AudioParameterInt("py", "Y Position", -maxDistance, maxDistance,
                                                      0, distanceReadoutY));

    xParam->addListener(this);
    yParam->addListener(this);

    if (MTS_CanRegisterMaster())
    {
        MTS_RegisterMaster();
        registeredMTS = true;
        std::cout << "registered OK" << std::endl;
    }
    else
    {
        startTimer(0, 50);
    }

    if (registeredMTS == true)
    {
        mode = Duodene;
        originalRefFreq = defaultRefFreq;
        originalRefNote = defaultRefNote;
        currentRefFreq = originalRefFreq;
        currentRefNote = originalRefNote;
        returnToOrigin();
        startTimer(1, 50);
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
void LatticesProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {}
void LatticesProcessor::releaseResources() {}
bool LatticesProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const { return true; }
const juce::String LatticesProcessor::getName() const { return JucePlugin_Name; }
bool LatticesProcessor::acceptsMidi() const { return true; }
bool LatticesProcessor::producesMidi() const { return false; }
bool LatticesProcessor::isMidiEffect() const { return false; }
double LatticesProcessor::getTailLengthSeconds() const { return 0.0; }
int LatticesProcessor::getNumPrograms() { return 1; }
int LatticesProcessor::getCurrentProgram() { return 0; }
void LatticesProcessor::setCurrentProgram(int index) {}
const juce::String LatticesProcessor::getProgramName(int index) { return {}; }
void LatticesProcessor::changeProgramName(int index, const juce::String &newName) {}
bool LatticesProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor *LatticesProcessor::createEditor() { return new LatticesEditor(*this); }
void LatticesProcessor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting) {}
//==============================================================================

void LatticesProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("Lattices"));

    xml->setAttribute("SavedMode", static_cast<int>(mode));

    for (int i = 0; i < 5; ++i)
    {
        juce::String c = juce::String("ccs_") + std::to_string(i);
        int v = shiftCCs[i];
        xml->setAttribute(c, v);
    }
    xml->setAttribute("channel", listenOnChannel);

    int n = originalRefNote;
    xml->setAttribute("note", n);

    double f = originalRefFreq;
    xml->setAttribute("freq", f);

    double X = (double)(xParam->get() + maxDistance) / (2 * maxDistance);
    double Y = (double)(yParam->get() + maxDistance) / (2 * maxDistance);

    xml->setAttribute("xp", X);
    xml->setAttribute("yp", Y);

    copyXmlToBinary(*xml, destData);
}

void LatticesProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName("Lattices"))
        {
            int m = xmlState->getIntAttribute("SavedMode");

            switch (m)
            {
            case Syntonic:
                mode = Syntonic;
                break;
            case Duodene:
                mode = Duodene;
            }

            for (int i = 0; i < 5; ++i)
            {
                juce::String c = juce::String("ccs_") + std::to_string(i);
                int g = xmlState->getIntAttribute(c);
                shiftCCs[i] = g;
            }

            int mc = xmlState->getIntAttribute("channel");
            listenOnChannel = mc;

            originalRefNote = xmlState->getIntAttribute("note");
            originalRefFreq = xmlState->getDoubleAttribute("freq");

            float x = xmlState->getDoubleAttribute("xp");
            float y = xmlState->getDoubleAttribute("yp");

            xParam->setValueNotifyingHost(x);
            yParam->setValueNotifyingHost(y);

            locate();
            updateHostDisplay(
                juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged(true));
        }
    }
    else
    {
        std::cout << "yo wtf" << std::endl;
    }
}

//==============================================================================

void LatticesProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                     juce::MidiBuffer &midiMessages)
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

void LatticesProcessor::timerCallback(int timerID)
{
    if (timerID == 0)
    {
        if (MTStryAgain)
        {
            if (MTS_CanRegisterMaster())
            {
                MTS_RegisterMaster();
                registeredMTS = true;
            }

            if (registeredMTS)
            {
                std::cout << "registered OK" << std::endl;
                mode = Duodene;
                originalRefFreq = defaultRefFreq;
                originalRefNote = defaultRefNote;
                currentRefFreq = originalRefFreq;
                currentRefNote = originalRefNote;
                returnToOrigin();
                stopTimer(0);
                startTimer(1, 50);
            }
            MTStryAgain = false;
        }

        if (MTSreInit)
        {
            MTS_Reinitialize();
            MTS_RegisterMaster();
            registeredMTS = true;
            MTSreInit = false;
            std::cout << "registered OK" << std::endl;
            mode = Duodene;
            originalRefFreq = defaultRefFreq;
            originalRefNote = defaultRefNote;
            currentRefFreq = originalRefFreq;
            currentRefNote = originalRefNote;
            returnToOrigin();
            stopTimer(0);
            startTimer(1, 50);
        }
    }

    if (timerID == 1)
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
}

void LatticesProcessor::modeSwitch(int m)
{
    switch (m)
    {
    case Syntonic:
        mode = Syntonic;
        break;
    case Duodene:
        mode = Duodene;
    }

    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged(true));

    returnToOrigin();
}

void LatticesProcessor::updateMIDI(int wCC, int eCC, int nCC, int sCC, int hCC, int C)
{
    // TODO: This doesn't set project dirty flags, investigate

    shiftCCs[0] = wCC;
    shiftCCs[1] = eCC;
    shiftCCs[2] = nCC;
    shiftCCs[3] = sCC;
    shiftCCs[4] = hCC;
    listenOnChannel = C;

    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged(true));
}

void LatticesProcessor::updateFreq(double f)
{
    originalRefFreq = f;
    returnToOrigin();

    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged(true));
}

double LatticesProcessor::updateRoot(int r)
{
    double nf = freqs[60 + r];

    originalRefNote = r;
    originalRefFreq = nf;
    returnToOrigin();

    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged(true));

    return nf;
}

void LatticesProcessor::updateVisitors(int *v)
{
    for (int d = 0; d < 12; ++d)
    {
        visitor[d] = v[d];
        setVisitorTuning(d, v[d]);
    }
    locate();
}

void LatticesProcessor::setVisitorTuning(int d, int c)
{
    bool major = (d == 7 || d == 2 || d == 9 || d == 4 || d == 11 || d == 6);

    switch (c)
    {
    case 0:
        visitorTuning[d] = 1.0;
        break;
    case 1:
        visitorTuning[d] = jim.comma(jim.syntonic, major);
        break;
    case 2:
        visitorTuning[d] = jim.comma(jim.seven, major);
        break;
    case 3:
        visitorTuning[d] = jim.comma(jim.eleven, major);
        break;
    case 4:
        visitorTuning[d] = jim.comma(jim.thirteen, major);
        break;
    case 5:
        visitorTuning[d] = jim.comma(jim.seventeen, major);
        break;
    case 6:
        visitorTuning[d] = jim.comma(jim.nineteen, major);
        break;
    case 7:
        visitorTuning[d] = jim.comma(jim.twentythree, major);
        break;
    }
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

    for (int i = 0; i < 12; ++i)
    {
        ratios[i] = duo12[i];
        coOrds[i] = duoCo[i];
        setVisitorTuning(i, defvis[i]);
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

void LatticesProcessor::parameterValueChanged(int parameterIndex, float newValue) { locate(); }

void LatticesProcessor::shift(int dir)
{
    float X = xParam->get();
    float Y = yParam->get();

    switch (dir)
    {
    case West:
        xParam->beginChangeGesture();
        X = GNV(X - 1);
        xParam->setValueNotifyingHost(X);
        xParam->endChangeGesture();
        break;
    case East:
        xParam->beginChangeGesture();
        X = GNV(X + 1);
        xParam->setValueNotifyingHost(X);
        xParam->endChangeGesture();
        break;
    case North:
        yParam->beginChangeGesture();
        Y = GNV(Y + 1);
        yParam->setValueNotifyingHost(Y);
        yParam->endChangeGesture();
        break;
    case South:
        yParam->beginChangeGesture();
        Y = GNV(Y - 1);
        yParam->setValueNotifyingHost(Y);
        yParam->endChangeGesture();
        break;
    case Home:
        returnToOrigin();
        break;
    };

    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged(true));
}

void LatticesProcessor::locate()
{
    positionX = xParam->get();
    positionY = yParam->get();

    if (mode == Syntonic)
    {
        float quarter = static_cast<float>(positionX) / 4;
        int syntYOff = std::floor(quarter);
        syntYOff *= -1;

        positionY += syntYOff;
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
    while (nn >= 12)
    {
        nn -= 12;
        nf *= 0.5;
    }

    currentRefNote = nn;
    currentRefFreq = originalRefFreq * nf;

    //    for (int i = 0; i < 12; ++i) // no visitors
    //    {
    //        coOrds[i].first = duoCo[i].first + positionX;
    //        coOrds[i].second = duoCo[i].second + positionY;
    //
    //        ratios[i] = duo12[i];
    //    }

    for (int i = 0; i < 12; ++i) // visitors
    {
        auto vx{0};
        auto vy{0};

        if ((i == 9 || i == 4 || i == 11 || i == 6) && visitor[i] == 0)
        {
            vx = 4;
            vy = -1;
        }
        if ((i == 5 || i == 0) && visitor[i] != 0)
        {
            vx = 4;
            vy = -1;
        }
        if ((i == 7 || i == 2) && visitor[i] != 0)
        {
            vx = -4;
            vy = 1;
        }
        if ((i == 1 || i == 8 || i == 3 || i == 10) && visitor[i] == 0)
        {
            vx = -4;
            vy = 1;
        }

        coOrds[i].first = duoCo[i].first + positionX + vx;
        coOrds[i].second = duoCo[i].second + positionY + vy;

        ratios[i] = pyth12[i] * visitorTuning[i];
    }

    if (mode == Syntonic) // syntonic should ignore visitors
    {
        int syntShape = ((positionX % 4) + 4) % 4;

        ratios[6] = (syntShape > 0) ? (double)36 / 25 : (double)45 / 32;
        ratios[11] = (syntShape > 1) ? (double)48 / 25 : (double)15 / 8;
        ratios[4] = (syntShape == 3) ? (double)32 / 25 : (double)5 / 4;

        coOrds[6].second = (syntShape > 0) ? positionY - 2 : positionY + 1;
        coOrds[11].second = (syntShape > 1) ? positionY - 2 : positionY + 1;
        coOrds[4].second = (syntShape == 3) ? positionY - 2 : positionY + 1;
    }

    updateTuning();
}

void LatticesProcessor::updateTuning()
{
    changed = true;

    int refMidiNote = currentRefNote + 60;
    for (int note = 0; note < 128; ++note)
    {
        double octaveShift = std::pow(2, std::floor(((double)note - refMidiNote) / 12.0));

        int degree = (note - refMidiNote) % 12;
        if (degree < 0)
        {
            degree += 12;
        }

        freqs[note] = currentRefFreq * ratios[degree] * octaveShift;
    }

    MTS_SetNoteTunings(freqs);

    // later...
    MTS_SetScaleName("JI is nice yeah?");
}

inline float LatticesProcessor::GNV(int input)
{
    float res = input + maxDistance;
    res /= (2 * maxDistance);

    return res;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new LatticesProcessor(); }
