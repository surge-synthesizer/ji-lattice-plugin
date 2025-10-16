/*
 Lattices - A Just-Intonation graphical MTS-ESP Source

 Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

 This code is released under the MIT licence, but do note that it depends
 on the JUCE library, see licence for more details.

 Source available at https://github.com/Andreya-Autumn/lattices
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <set>
#include <atomic>
#include <cmath>
#include <string>
#include <cstdint>
#include <mutex>

#include "JIMath.h"
#include "ScaleData.h"

class LatticesProcessor : public juce::AudioProcessor,
                          juce::MultiTimer,
                          private juce::AudioProcessorParameter::Listener
{
  public:
    //==============================================================================
    LatticesProcessor();
    ~LatticesProcessor();

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;
    //==============================================================================

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    void timerCallback(int timerID) override;

    void modeSwitch(int m);
    void updateMIDICC(int hCC);
    void updateMIDIChannel(int C);
    void updateFreq(double f);
    double updateRoot(int r);
    void updateDistance(int dist);
    bool newVisitorGroup();
    void resetVisitorGroup();
    void deleteVisitorGroup(int idx);
    void selectVisitorGroup(int g);
    void editVisitors(bool editing, int g);
    void updateVisitor(int d, int v);
    void updateDegreeCoord(int d);
    void updateAllCoords();

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void shift(int dir);

    bool registeredMTS{false};
    bool MTSreInit{false};
    bool MTStryAgain{false};

    enum Mode
    {
        Duodene,
        Syntonic,
    };
    std::atomic<Mode> mode = Duodene;
    std::atomic<bool> changed{false};
    std::atomic<int> numClients{0};

    int homeCC = 5;
    int listenOnChannel = 1;

    // key, frequency and name of the origin note
    int originalRefNote{0};
    double originalRefFreq{261.6255653005986};
    std::pair<uint8_t, int> originNoteName = {1, 0};

    // key, ratio and coordinates of the current center note
    int currentRefNote{0};
    double ratioToOriginal{1.0};

    std::pair<int, int> positionXY{0, 0};
    // coordinates of current scale
    std::pair<int, int> coOrds[12]{};

    std::vector<lattices::scaledata::ScaleData> visitorGroups;
    lattices::scaledata::ScaleData *currentVisitors;
    int numVisitorGroups{0};
    bool editingVisitors{false};
    int priorSelectedGroup{0};

    bool loadedState{false};

    uint16_t maxDistance{24};

  private:
    // fallbacks for the origin
    static constexpr int defaultRefNote{0};
    static constexpr double defaultRefFreq{261.6255653005986};
    std::pair<uint8_t, int> defaultOriginNoteName = {1, 0};

    const JIMath jim;

    enum Direction
    {
        Home,
        West,
        East,
        North,
        South
    };

    void returnToOrigin();

    void respondToMidi(const juce::MidiMessage &m);
    std::vector<bool> hold = {false, false, false, false, false};
    std::vector<bool> wait = {false, false, false, false, false};

    std::mutex visLock;

    void locate();
    void updateTuning();

    double freqs[128]{};

    juce::AudioParameterFloat *xParam;
    juce::AudioParameterFloat *yParam;
    juce::AudioParameterFloat *vParam;
    juce::AudioParameterFloat *fParam;

    // define these here lest the lambda functions
    // below throw an annoying "not defined" warning

    double toVisitorParam(int input)
    {
        if (visitorGroups.size() <= 1)
            return 0.0;
        return static_cast<double>(input) / (visitorGroups.size() - 1);
    }

    inline int fromVisitorParam(float input)
    {
        if (visitorGroups.size() <= 1)
            return 0;
        return static_cast<int>(std::round(input * (visitorGroups.size() - 1)));
    }

    inline double toXYParam(int input, bool v = false)
    {
        return static_cast<double>(input + maxDistance) / (2.0 * maxDistance);
    }

    inline int fromXYParam(float input, bool v = false)
    {
        return static_cast<int>(std::round((input - 0.5) * 2 * maxDistance));
    }

    juce::String toStringX(float value)
    {
        int v = fromXYParam(value);

        juce::String dir{};
        if (v == 0)
        {
            dir = "Home";
            return dir;
        }

        if (v == 1)
        {
            dir = "1 Step East";
            return dir;
        }
        else if (v == -1)
        {
            dir = "1 Step West";
            return dir;
        }

        dir = std::to_string(std::abs(v)) + ((v > 1) ? " Steps East" : " Steps West");
        return dir;
    }

    juce::String toStringY(float value)
    {
        int v = fromXYParam(value);

        juce::String dir{};
        if (v == 0)
        {
            dir = "Home";
            return dir;
        }
        if (v == 1)
        {
            dir = "1 Step North";
            return dir;
        }
        else if (v == -1)
        {
            dir = "1 Step South";
            return dir;
        }

        dir = std::to_string(std::abs(v)) + ((v > 1) ? " Steps North" : " Steps South");
        return dir;
    }

    std::string whereAreWe(float VX, float VY)
    {
        auto tmpX = toStringX(VX).toStdString();
        auto EW = (tmpX != "Home") ? tmpX : "";

        auto tmpY = toStringY(VY).toStdString();
        auto NS = (tmpY != "Home") ? tmpY : "";

        if (EW.empty() && NS.empty())
        {
            return "Home";
        }
        else
        {
            EW += (EW.empty() == NS.empty()) ? ", " : "";
            EW += NS;

            return EW.c_str();
        }
    }

    static constexpr double minFreq{100.0};
    static constexpr double maxFreq{1000.0};

    inline float toFreqParam(double input)
    {
        return std::sqrt((input - minFreq) / (maxFreq - minFreq));
    }

    inline double fromFreqParam(float input)
    {
        return minFreq + (maxFreq - minFreq) * input * input;
    }

    const juce::AudioParameterFloatAttributes distanceReadoutX =
        juce::AudioParameterFloatAttributes{}
            .withStringFromValueFunction(
                [this](float value, int maximumStringLength) -> juce::String
                { return toStringX(value); })
            .withValueFromStringFunction(
                [this](juce::String str)
                {
                    if (str == "Home")
                    {
                        return 0.5;
                    }

                    if (str == "1 Step East")
                    {
                        return toXYParam(1);
                    }

                    if (str == "1 Step West")
                    {
                        return toXYParam(-1);
                    }

                    double res{};
                    if (str.endsWith("East"))
                    {
                        str = str.trimCharactersAtEnd(" Steps East");
                        res = toXYParam(str.getIntValue());
                    }

                    if (str.endsWith("East"))
                    {
                        str = str.trimCharactersAtEnd(" Steps West");
                        res = toXYParam(str.getIntValue());
                        res *= -1;
                    }

                    return res;
                });

    const juce::AudioParameterFloatAttributes distanceReadoutY =
        juce::AudioParameterFloatAttributes{}
            .withStringFromValueFunction(
                [this](float value, int maximumStringLength) -> juce::String
                { return toStringY(value); })
            .withValueFromStringFunction(
                [this](juce::String str)
                {
                    if (str == "Home")
                    {
                        return 0.5;
                    }

                    if (str == "1 Step North")
                    {
                        return toXYParam(1);
                    }

                    if (str == "1 Step South")
                    {
                        return toXYParam(-1);
                    }

                    double res{};
                    if (str.endsWith("North"))
                    {
                        str = str.trimCharactersAtEnd(" Steps North");
                        res = toXYParam(str.getIntValue());
                        return res;
                    }

                    if (str.endsWith("South"))
                    {
                        str = str.trimCharactersAtEnd(" Steps South");
                        res = toXYParam(str.getIntValue());
                        res *= -1;
                    }

                    return res;
                });

    const juce::AudioParameterFloatAttributes visitorsReadout =
        juce::AudioParameterFloatAttributes{}
            .withStringFromValueFunction(
                [this](float value, int maximumStringLength) -> juce::String
                { return std::to_string(fromVisitorParam(value)); })
            .withValueFromStringFunction([this](juce::String str)
                                         { return toVisitorParam(str.getIntValue()); });

    const juce::AudioParameterFloatAttributes frequencyReadout =
        juce::AudioParameterFloatAttributes{}
            .withStringFromValueFunction(
                [this](float value, int maximumStringLength) -> juce::String
                { return std::to_string(fromFreqParam(value)); })
            .withValueFromStringFunction([this](juce::String str)
                                         { return toFreqParam(str.getDoubleValue()); });

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LatticesProcessor)
};
