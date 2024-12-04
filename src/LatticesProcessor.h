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

#include "JIMath.h"
#include "Visitors.h"

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
    void updateMIDI(int hCC, int C);
    void updateFreq(double f);
    double updateRoot(int r);
    void updateDistance(int dist);
    void editVisitors(bool editing, int g);
    int *selectVisitorGroup(int g);
    void newVisitorGroup();
    void updateVisitor(int d, int v);
    inline void setVisitorTuning(int d, int v);

    void parameterValueChanged(int parameterIndex, float newValue) override;

    bool registeredMTS{false};
    bool MTSreInit{false};
    bool MTStryAgain{false};

    std::atomic<int> positionX{0};
    std::atomic<int> positionY{0};

    enum Mode
    {
        Duodene,
        Syntonic,
    };
    std::atomic<Mode> mode = Duodene;
    std::atomic<bool> changed{false};
    std::atomic<int> numClients{0};

    std::pair<int, int> coOrds[12]{};

    int syntonicDrift = 0;
    int diesisDrift = 0;

    int homeCC = 5;
    int listenOnChannel = 1;

    int originalRefNote{-12};
    double originalRefFreq{-1};

    Visitors *currentVisitors;
    int numVisitorGroups{0};
    std::atomic<bool> editingVisitors{false};

    uint16_t maxDistance{24};

  private:
    static constexpr int defaultRefNote{0};
    static constexpr double defaultRefFreq{261.6255653005986};

    JIMath jim;

    int currentRefNote{};
    double currentRefFreq{};

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

    void shift(int dir);
    void locate();

    void updateTuning();

    double ratios[12] = {};
    double freqs[128]{};

    double duo12[12]{1.0,           (double)16 / 15, (double)9 / 8,   (double)6 / 5,
                     (double)5 / 4, (double)4 / 3,   (double)45 / 32, (double)3 / 2,
                     (double)8 / 5, (double)5 / 3,   (double)9 / 5,   (double)15 / 8};
    std::pair<int, int> duoCo[12]{{0, 0}, {-1, -1}, {2, 0},  {1, -1}, {0, 1},  {-1, 0},
                                  {2, 1}, {1, 0},   {0, -1}, {-1, 1}, {2, -1}, {1, 1}};

    double pyth12[12]{1.0,
                      (double)256 / 243,
                      (double)9 / 8,
                      (double)32 / 27,
                      (double)81 / 64,
                      (double)4 / 3,
                      (double)729 / 512,
                      (double)3 / 2,
                      (double)128 / 81,
                      (double)27 / 16,
                      (double)16 / 9,
                      (double)243 / 128};

    std::vector<Visitors> visitorGroups;
    int priorSelectedGroup{0};
    double visitorTuning[12] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

    juce::AudioParameterFloat *xParam;
    juce::AudioParameterFloat *yParam;
    juce::AudioParameterFloat *vParam;

    // define these here lest the lambda functions
    // below throw an annoying "not defined" warning
    inline double toParam(int input, bool v = false)
    {
        if (v)
        {
            if (visitorGroups.size() <= 1)
            {
                return 0.0;
            }

            return static_cast<double>(input) / (visitorGroups.size() - 1);
        }
        else
        {
            return static_cast<double>(input + maxDistance) / (2.0 * maxDistance);
        }
    }
    inline int fromParam(double input, bool v = false)
    {
        if (v)
        {
            if (visitorGroups.size() <= 1)
            {
                return 0;
            }
            return static_cast<int>(std::round(input * (visitorGroups.size() - 1)));
        }
        else
        {
            return static_cast<int>(std::round((input - 0.5) * 2 * maxDistance));
        }
    }

    const juce::AudioParameterFloatAttributes distanceReadoutX =
        juce::AudioParameterFloatAttributes{}
            .withStringFromValueFunction(
                [this](float value, int maximumStringLength) -> juce::String
                {
                    int v = fromParam(value);

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
                })
            .withValueFromStringFunction(
                [this](juce::String str)
                {
                    if (str == "Home")
                    {
                        return 0.5;
                    }

                    if (str == "1 Step East")
                    {
                        return toParam(1);
                    }

                    if (str == "1 Step West")
                    {
                        return toParam(-1);
                    }

                    double res{};
                    if (str.endsWith("East"))
                    {
                        str = str.trimCharactersAtEnd(" Steps East");
                        res = toParam(str.getIntValue());
                    }

                    if (str.endsWith("East"))
                    {
                        str = str.trimCharactersAtEnd(" Steps West");
                        res = toParam(str.getIntValue());
                        res *= -1;
                    }

                    return res;
                });

    const juce::AudioParameterFloatAttributes distanceReadoutY =
        juce::AudioParameterFloatAttributes{}
            .withStringFromValueFunction(
                [this](float value, int maximumStringLength) -> juce::String
                {
                    int v = fromParam(value);

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
                })
            .withValueFromStringFunction(
                [this](juce::String str)
                {
                    if (str == "Home")
                    {
                        return 0.5;
                    }

                    if (str == "1 Step North")
                    {
                        return toParam(1);
                    }

                    if (str == "1 Step South")
                    {
                        return toParam(-1);
                    }

                    double res{};
                    if (str.endsWith("North"))
                    {
                        str = str.trimCharactersAtEnd(" Steps North");
                        res = toParam(str.getIntValue());
                        return res;
                    }

                    if (str.endsWith("South"))
                    {
                        str = str.trimCharactersAtEnd(" Steps South");
                        res = toParam(str.getIntValue());
                        res *= -1;
                    }

                    return res;
                });

    const juce::AudioParameterFloatAttributes visitorsReadout =
        juce::AudioParameterFloatAttributes{}
            .withStringFromValueFunction(
                [this](float value, int maximumStringLength) -> juce::String
                { return std::to_string(fromParam(value, true)); })
            .withValueFromStringFunction([this](juce::String str)
                                         { return toParam(str.getIntValue(), true); });

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LatticesProcessor)
};
