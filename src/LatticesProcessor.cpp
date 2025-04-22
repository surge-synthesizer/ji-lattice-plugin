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
    addParameter(fParam = new juce::AudioParameterFloat("pf", "Reference Frequency", r, 0.5, frequencyReadout));

    xParam->addListener(this);
    yParam->addListener(this);
    vParam->addListener(this);
    fParam->addListener(this);

    numVisitorGroups = 1;
    Visitors dg{"Nobody Here", jim};
    visitorGroups.push_back(std::move(dg));
    hold.emplace_back(false);
    wait.emplace_back(false);
    currentVisitors = &visitorGroups[0];

    if (MTS_CanRegisterMaster())
    {
        MTS_RegisterMaster();
        registeredMTS = true;
        std::cout << "registered OK" << std::endl;
        returnToOrigin();
        startTimer(1, 5);
    }
    else
    {
        startTimer(0, 50);
    }
}

LatticesProcessor::~LatticesProcessor()
{
    xParam->removeListener(this);
    yParam->removeListener(this);
    vParam->removeListener(this);
    fParam->removeListener(this);

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

    xml->setAttribute("cc", homeCC);
    xml->setAttribute("channel", listenOnChannel);

    int n = originalRefNote;
    xml->setAttribute("note", n);

    int X = fromParam(xParam->get());
    int Y = fromParam(yParam->get());
    int V = fromParam(vParam->get(), true);
    double F = fromFreqParam(fParam->get());

    xml->setAttribute("xp", X);
    xml->setAttribute("yp", Y);
    xml->setAttribute("vp", V);
    xml->setAttribute("fp", F);

    xml->setAttribute("nvg", numVisitorGroups);

    if (numVisitorGroups > 1) // no need to store number 0 since it's the default
    {
        for (int v = 1; v < numVisitorGroups; ++v)
        {
            auto vs = juce::String("visitor_") + std::to_string(v) + juce::String("_");

            juce::String n = vs + juce::String("name");
            juce::String name{visitorGroups[v].name};

            xml->setAttribute(n, name);

            for (int d = 0; d < 12; ++d)
            {
                auto b = vs + juce::String("idx_") + std::to_string(d);
                int i = visitorGroups[v].vis[d];
                xml->setAttribute(b, i);
            }
        }
    }

    xml->setAttribute("md", maxDistance);

    copyXmlToBinary(*xml, destData);
}

void LatticesProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName("Lattices"))
        {
            int m = xmlState->getIntAttribute("SavedMode", 1);

            switch (m)
            {
            case Syntonic:
                mode = Syntonic;
                break;
            case Duodene:
                mode = Duodene;
            }

            homeCC = xmlState->getIntAttribute("cc", 5);
            listenOnChannel = xmlState->getIntAttribute("channel", 1);

            originalRefNote = xmlState->getIntAttribute("note", 0);
            switch (originalRefNote)
            {
            case 0:
                originNoteName.first = 1;
                originNoteName.second = 0;
                break;
            case 1:
                originNoteName.first = 3;
                originNoteName.second = -1;
                break;
            case 2:
                originNoteName.first = 3;
                originNoteName.second = 0;
                break;
            case 3:
                originNoteName.first = 5;
                originNoteName.second = -1;
                break;
            case 4:
                originNoteName.first = 5;
                originNoteName.second = 0;
                break;
            case 5:
                originNoteName.first = 0;
                originNoteName.second = 0;
                break;
            case 6:
                originNoteName.first = 0;
                originNoteName.second = 1;
                break;
            case 7:
                originNoteName.first = 2;
                originNoteName.second = 0;
                break;
            case 8:
                originNoteName.first = 4;
                originNoteName.second = -1;
                break;
            case 9:
                originNoteName.first = 4;
                originNoteName.second = 0;
                break;
            case 10:
                originNoteName.first = 6;
                originNoteName.second = -1;
                break;
            case 11:
                originNoteName.first = 6;
                originNoteName.second = 0;
                break;
            }

            maxDistance = xmlState->getIntAttribute("md", 24);

            numVisitorGroups = xmlState->getIntAttribute("nvg", 1);

            visitorGroups.clear();
            Visitors dg{"Nobody Here", jim};
            visitorGroups.push_back(std::move(dg));

            if (numVisitorGroups > 1)
            {
                for (int v = 1; v < numVisitorGroups; ++v)
                {
                    auto vs = juce::String("visitor_") + std::to_string(v) + juce::String("_");
                    juce::String n = vs + juce::String("name");

                    juce::String jsn{xmlState->getStringAttribute(n)};
                    std::string name = jsn.toStdString();
                    int vds[12]{};
                    for (int d = 0; d < 12; ++d)
                    {
                        auto b = vs + juce::String("idx_") + std::to_string(d);
                        vds[d] = xmlState->getIntAttribute(b);
                    }
                    Visitors ng{name, jim, vds};
                    visitorGroups.push_back(std::move(ng));
                }
            }

            std::lock_guard<std::mutex> lock(visLock);

            hold.clear();
            wait.clear();
            for (int i = 0; i < 5 + numVisitorGroups - 1; ++i)
            {
                hold.emplace_back(false);
                wait.emplace_back(false);
            }

            int tv = xmlState->getIntAttribute("vp", 0);
            int tx = xmlState->getIntAttribute("xp", 0);
            int ty = xmlState->getIntAttribute("yp", 0);
            originalRefFreq = xmlState->getDoubleAttribute("fp", 261.6255653005986);
            
            float X = toParam(tx);
            float Y = toParam(ty);
            float V = toParam(tv, true);
            float F = toParam(originalRefFreq);

            xParam->beginChangeGesture();
            yParam->beginChangeGesture();
            vParam->beginChangeGesture();
            fParam->beginChangeGesture();

            xParam->setValueNotifyingHost(X);
            yParam->setValueNotifyingHost(Y);
            vParam->setValueNotifyingHost(V);
            fParam->setValueNotifyingHost(originalRefFreq);

            xParam->endChangeGesture();
            yParam->endChangeGesture();
            vParam->endChangeGesture();
            fParam->endChangeGesture();

            loadedState = true;
            changed = true;

            // no need for locate(); since paramChanged calls it
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

    for (const auto metadata : midiMessages)
    {
        respondToMidi(metadata.getMessage());
    }
}

void LatticesProcessor::respondToMidi(const juce::MidiMessage &m)
{
    if (editingVisitors)
        return;

    if (m.isController() && m.getChannel() == listenOnChannel)
    {
        auto num = m.getControllerNumber();
        auto val = m.getControllerValue();

        int numCCs = 5 + numVisitorGroups - 1;

        for (int i = 0; i < 5 + numVisitorGroups - 1; ++i)
        {
            if (num == homeCC + i)
            {
                if (val == 127 && !hold[i])
                {
                    hold[i] = true;
                }

                if (val < 127 && hold[i] && wait[i])
                {
                    hold[i] = false;
                    wait[i] = false;
                }
            }
        }
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
                std::cout << "registered OK" << std::endl;
                stopTimer(0);
                startTimer(1, 5);
                locate();
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
            stopTimer(0);
            startTimer(1, 5);
            locate();
        }
    }

    if (timerID == 1)
    {
        for (int i = 0; i < 5 + numVisitorGroups - 1; ++i)
        {
            if (hold[i] && !wait[i])
            {
                if (i < 5)
                {
                    shift(i);
                    wait[i] = true;
                }
                else
                {
                    int cv = fromParam(vParam->get(), true);
                    int nv = i - 4;

                    if (cv == nv)
                    {
                        vParam->beginChangeGesture();
                        vParam->setValueNotifyingHost(0);
                        vParam->endChangeGesture();
                    }
                    else
                    {
                        vParam->beginChangeGesture();
                        vParam->setValueNotifyingHost(toParam(nv, true));
                        vParam->endChangeGesture();
                    }
                    wait[i] = true;
                }
            }
        }

        numClients = MTS_GetNumClients();
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

void LatticesProcessor::updateMIDICC(int hCC)
{
    homeCC = hCC;

    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged(true));
}

void LatticesProcessor::updateMIDIChannel(int C)
{
    listenOnChannel = C;

    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged(true));
}

void LatticesProcessor::updateFreq(double f)
{
    fParam->beginChangeGesture();
    fParam->setValueNotifyingHost(toFreqParam(f));
    originalRefFreq = f;
    fParam->endChangeGesture();
}

double LatticesProcessor::updateRoot(int r)
{
    double nf = freqs[60 + r];

    originalRefNote = r;
    originalRefFreq = nf;

    switch (r)
    {
    case 0:
        originNoteName.first = 1;
        originNoteName.second = 0;
        break;
    case 1:
        originNoteName.first = 3;
        originNoteName.second = -1;
        break;
    case 2:
        originNoteName.first = 3;
        originNoteName.second = 0;
        break;
    case 3:
        originNoteName.first = 5;
        originNoteName.second = -1;
        break;
    case 4:
        originNoteName.first = 5;
        originNoteName.second = 0;
        break;
    case 5:
        originNoteName.first = 0;
        originNoteName.second = 0;
        break;
    case 6:
        originNoteName.first = 0;
        originNoteName.second = 1;
        break;
    case 7:
        originNoteName.first = 2;
        originNoteName.second = 0;
        break;
    case 8:
        originNoteName.first = 4;
        originNoteName.second = -1;
        break;
    case 9:
        originNoteName.first = 4;
        originNoteName.second = 0;
        break;
    case 10:
        originNoteName.first = 6;
        originNoteName.second = -1;
        break;
    case 11:
        originNoteName.first = 6;
        originNoteName.second = 0;
        break;
    }

    returnToOrigin();

    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged(true));

    return nf;
}

void LatticesProcessor::updateDistance(int dist)
{
    maxDistance = dist;
    returnToOrigin();
    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged(true));
}

void LatticesProcessor::editVisitors(bool editing, int g)
{
    editingVisitors = editing;

    if (editing)
    {
        priorSelectedGroup = fromParam(vParam->get());
        selectVisitorGroup(g);
    }
    else
    {
        vParam->setValueNotifyingHost(toParam(priorSelectedGroup));
        changed = true;
    }
}

int *LatticesProcessor::selectVisitorGroup(int g)
{
    currentVisitors = &visitorGroups[g];
    locate();

    return currentVisitors->vis;
}

void LatticesProcessor::resetVisitorGroup()
{
    currentVisitors->resetToDefault();
    changed = true;
    locate();
}

bool LatticesProcessor::newVisitorGroup()
{
    if (visitorGroups.size() > 32) // > 32 because the 0th doesn't count
        return false;

    auto name = std::to_string(numVisitorGroups);

    Visitors ng{name, jim};
    visitorGroups.push_back(std::move(ng));
    // probably not necessary since process returns early if the visitors
    // editor is open, but let's do it anyway.
    std::lock_guard<std::mutex> lock(visLock);
    hold.emplace_back(false);
    wait.emplace_back(false);
    ++numVisitorGroups;

    selectVisitorGroup(numVisitorGroups - 1);
    return true;
}

void LatticesProcessor::deleteVisitorGroup(int idx)
{
    if (idx == 0)
        return; // illegal, shouldn't happen

    currentVisitors = &visitorGroups[idx - 1];
    visitorGroups.erase(visitorGroups.begin() + idx);

    // probably not necessary etc
    std::lock_guard<std::mutex> lock(visLock);
    hold.pop_back();
    wait.pop_back();
    --numVisitorGroups;

    locate();
}

void LatticesProcessor::updateVisitor(int d, int v)
{
    currentVisitors->setDegree(d, v);
    locate();
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

    for (int d = 0; d < 12; ++d)
    {
        ratios[d] = duo12[d];
        coOrds[d] = duoCo[d];
    }

    updateTuning();
}

void LatticesProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
    switch (parameterIndex)
    {
        case 0:
            locate();
        break;
        case 1:
            locate();
        break;
        case 2:
            if (!editingVisitors)
            {
                int vis = fromParam(vParam->get(), true);
                currentVisitors = &visitorGroups[vis];
                locate();
            }
        break;
        case 3:
            originalRefFreq = fromFreqParam(fParam->get());
            updateTuning();
        break;
    }
}

void LatticesProcessor::shift(int dir)
{
    double X = fromParam(xParam->get());
    double Y = fromParam(yParam->get());

    switch (dir)
    {
    case Home:
        returnToOrigin();
        break;
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

    if (editingVisitors)
    {
        currentRefNote = originalRefNote;
        currentRefFreq = originalRefFreq;
    }
    else
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
        currentRefFreq = nf;
    }

    if (mode == Syntonic)
    {
        if (currentVisitors != &visitorGroups[0])
        {
            currentVisitors = &visitorGroups[0];
        }

        for (int d = 0; d < 12; ++d)
        {
            coOrds[d].first = duoCo[d].first + positionX;
            coOrds[d].second = duoCo[d].second + positionY;
            ratios[d] = duo12[d];
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
        for (int d = 0; d < 12; ++d)
        {
            auto vx{0};
            auto vy{0};

            if ((d == 9 || d == 4 || d == 11 || d == 6) && currentVisitors->vis[d] == 0)
            {
                vx = 4;
                vy = -1;
            }
            if ((d == 5 || d == 0) && currentVisitors->vis[d] != 0)
            {
                vx = 4;
                vy = -1;
            }
            if ((d == 7 || d == 2) && currentVisitors->vis[d] != 0)
            {
                vx = -4;
                vy = 1;
            }
            if ((d == 1 || d == 8 || d == 3 || d == 10) && currentVisitors->vis[d] == 0)
            {
                vx = -4;
                vy = 1;
            }

            coOrds[d].first = duoCo[d].first + positionX + vx;
            coOrds[d].second = duoCo[d].second + positionY + vy;

            ratios[d] = pyth12[d] * currentVisitors->tuning[d];
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

        freqs[note] = originalRefFreq * currentRefFreq * ratios[degree] * octaveShift;
    }

    MTS_SetNoteTunings(freqs);

    // later...
    MTS_SetScaleName((whereAreWe(xParam->get(), yParam->get()).c_str()));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new LatticesProcessor(); }
