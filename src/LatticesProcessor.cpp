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
    juce::NormalisableRange r = juce::NormalisableRange(0.f, 1.f);

    addParameter(xParam =
                     new juce::AudioParameterFloat("px", "X Position", r, 0.5, distanceReadoutX));
    addParameter(yParam =
                     new juce::AudioParameterFloat("py", "Y Position", r, 0.5, distanceReadoutY));
    addParameter(vParam = new juce::AudioParameterFloat("pv", "Visitors", r, 0, visitorsReadout));

    xParam->addListener(this);
    yParam->addListener(this);
    vParam->addListener(this);

    numVisitorGroups = 1;
    Visitors dg{"Nobody here"};
    visitorGroups.push_back(std::move(dg));
    currentVisitors = &visitorGroups[0];

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
        returnToOrigin();
        startTimer(1, 50);
    }
}

LatticesProcessor::~LatticesProcessor()
{
    xParam->removeListener(this);
    yParam->removeListener(this);
    vParam->removeListener(this);

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

    int X = fromParam(xParam->get());
    int Y = fromParam(yParam->get());
    int V = fromParam(vParam->get(), true);

    xml->setAttribute("xp", X);
    xml->setAttribute("yp", Y);
    xml->setAttribute("vp", V);

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

            int tx = xmlState->getIntAttribute("xp");
            int ty = xmlState->getIntAttribute("yp");
            int tv = xmlState->getIntAttribute("vp");

            float X = toParam(tx);
            float Y = toParam(ty);
            float V = toParam(tv, true);

            xParam->beginChangeGesture();
            xParam->setValueNotifyingHost(X);
            xParam->endChangeGesture();
            yParam->beginChangeGesture();
            yParam->setValueNotifyingHost(Y);
            yParam->endChangeGesture();
            vParam->beginChangeGesture();
            vParam->setValueNotifyingHost(V);
            vParam->endChangeGesture();

            float shj = xParam->get();
            int dX = fromParam(shj);

            locate();
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
                if (visitorGroups.empty())
                    newVisitorGroup();
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
            if (visitorGroups.empty())
                newVisitorGroup();
            returnToOrigin();
            stopTimer(0);
            startTimer(1, 50);
        }
    }

    if (timerID == 1)
    {
        for (int i = 0; i < 6; ++i)
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

void LatticesProcessor::updateVisitor(int d, int v)
{
    //    if (d == 0) return;
    currentVisitors->dimensions[d] = v;
    setVisitorTuning(d, v);

    locate();
}

inline void LatticesProcessor::setVisitorTuning(int d, int v)
{
    bool major = (d == 7 || d == 2 || d == 9 || d == 4 || d == 11 || d == 6);

    switch (v)
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

int *LatticesProcessor::selectVisitorGroup(int g)
{
    currentVisitors = &visitorGroups[g];

    float v = toParam(g, true);
    vParam->setValueNotifyingHost(v);

    locate();
    return currentVisitors->dimensions;
}

void LatticesProcessor::newVisitorGroup()
{
    ++numVisitorGroups;
    Visitors ng{"new"};
    visitorGroups.push_back(std::move(ng));

    float v = toParam(numVisitorGroups - 1, true);
    vParam->setValueNotifyingHost(v);

    currentVisitors = &visitorGroups[numVisitorGroups - 1];
    changed = true;
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
    vParam->beginChangeGesture();
    vParam->setValueNotifyingHost(0.0);
    vParam->endChangeGesture();

    for (int i = 0; i < 12; ++i)
    {
        ratios[i] = duo12[i];
        coOrds[i] = duoCo[i];
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

        //        if (num == 15)
        //        {
        //            if (val == 127 && hold[5] == false)
        //            {
        //                visitorsActive = !visitorsActive;
        //                hold[5] = true;
        //                locate();
        //            }
        //
        //            if (val < 127 && hold[5] == true)
        //            {
        //                wait[5] = true;
        //            }
        //        }
    }
}

void LatticesProcessor::parameterValueChanged(int parameterIndex, float newValue) { locate(); }

void LatticesProcessor::shift(int dir)
{
    double X = fromParam(xParam->get());
    double Y = fromParam(yParam->get());

    switch (dir)
    {
    case West:
        xParam->beginChangeGesture();
        --X;
        xParam->setValueNotifyingHost(toParam(X));
        xParam->endChangeGesture();
        break;
    case East:
        xParam->beginChangeGesture();
        ++X;
        xParam->setValueNotifyingHost(toParam(X));
        xParam->endChangeGesture();
        break;
    case North:
        yParam->beginChangeGesture();
        ++Y;
        yParam->setValueNotifyingHost(toParam(Y));
        yParam->endChangeGesture();
        break;
    case South:
        yParam->beginChangeGesture();
        --Y;
        yParam->setValueNotifyingHost(toParam(Y));
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
    positionX = fromParam(xParam->get());
    positionY = fromParam(yParam->get());

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

    if (mode == Syntonic) // syntonic ignores visitors
    {
        for (int i = 0; i < 12; ++i)
        {
            coOrds[i].first = duoCo[i].first + positionX;
            coOrds[i].second = duoCo[i].second + positionY;
            ratios[i] = duo12[i];
        }

        int syntShape = ((positionX % 4) + 4) % 4;

        ratios[6] = (syntShape > 0) ? (double)36 / 25 : (double)45 / 32;
        ratios[11] = (syntShape > 1) ? (double)48 / 25 : (double)15 / 8;
        ratios[4] = (syntShape == 3) ? (double)32 / 25 : (double)5 / 4;

        coOrds[6].second = (syntShape > 0) ? positionY - 2 : positionY + 1;
        coOrds[11].second = (syntShape > 1) ? positionY - 2 : positionY + 1;
        coOrds[4].second = (syntShape == 3) ? positionY - 2 : positionY + 1;
    }
    else
    {
        int vis = fromParam(vParam->get(), true);
        if (!editingVisitors)
        {
            currentVisitors = &visitorGroups[vis];
        }

        if ((!editingVisitors && vis == 0) || visitorGroups.empty())
        {
            for (int i = 0; i < 12; ++i) // no visitors
            {
                coOrds[i].first = duoCo[i].first + positionX;
                coOrds[i].second = duoCo[i].second + positionY;
                ratios[i] = duo12[i];
            }
        }
        else
        {
            for (int i = 0; i < 12; ++i) // visitors
            {
                auto vx{0};
                auto vy{0};

                if ((i == 9 || i == 4 || i == 11 || i == 6) && currentVisitors->dimensions[i] == 0)
                {
                    vx = 4;
                    vy = -1;
                }
                if ((i == 5 || i == 0) && currentVisitors->dimensions[i] != 0)
                {
                    vx = 4;
                    vy = -1;
                }
                if ((i == 7 || i == 2) && currentVisitors->dimensions[i] != 0)
                {
                    vx = -4;
                    vy = 1;
                }
                if ((i == 1 || i == 8 || i == 3 || i == 10) && currentVisitors->dimensions[i] == 0)
                {
                    vx = -4;
                    vy = 1;
                }

                coOrds[i].first = duoCo[i].first + positionX + vx;
                coOrds[i].second = duoCo[i].second + positionY + vy;

                ratios[i] = pyth12[i] * visitorTuning[i];
            }
        }
    }
    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged(true));
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

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new LatticesProcessor(); }
