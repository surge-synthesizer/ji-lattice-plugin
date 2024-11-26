/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do note that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#pragma once

#include <memory>

#include <juce_audio_processors/juce_audio_processors.h>
// #include "melatonin_inspector/melatonin_inspector.h"
#include "LatticesProcessor.h"
#include "JIMath.h"
#include "LatticeComponent.h"
#include "ModeComponent.h"
#include "MIDIMenuComponent.h"
#include "OriginComponent.h"
#include "VisitorsComponent.h"
#include "MTSWarningComponent.h"


//==============================================================================
/**
 */
class LatticesEditor : public juce::AudioProcessorEditor, juce::MultiTimer
{
  public:
    LatticesEditor(LatticesProcessor &);
    ~LatticesEditor();

    //==============================================================================

    void paint(juce::Graphics &) override;
    void resized() override;

    void showVisitorsMenu();
    void showTuningMenu();
    void showMidiMenu();
    void resetMTS();

    void timerCallback(int timerID) override;

    //    std::unique_ptr<juce::Timer> idleTimer;
    void idle();
    void assignVisitors();

  private:
    static constexpr int width{900};
    static constexpr int height{600};

    //    melatonin::Inspector inspector{*this};

    juce::Colour backgroundColour = juce::Colour{.5f, .5f, 0.f, 1.f};

    std::unique_ptr<LatticeComponent> latticeComponent;

    std::unique_ptr<juce::TextButton> tuningButton;
    std::unique_ptr<OriginComponent> originComponent;
    std::unique_ptr<ModeComponent> modeComponent;

    std::unique_ptr<juce::TextButton> midiButton;
    std::unique_ptr<MIDIMenuComponent> midiComponent;

    std::unique_ptr<MTSWarningComponent> warningComponent;
    
    std::unique_ptr<juce::TextButton> visitorsButton;
    std::unique_ptr<VisitorsComponent> visitorsComponent;

    void init();
    bool inited{false};

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LatticesProcessor &processor;
    
    struct Visitors
    {
        JIMath jim;
        
        enum Dimension
        {
            Three,
            Five,
            Seven,
            Eleven,
            Thirteen,
            Seventeen,
            Nineteen,
            Twentythree
        };
        
        bool active{false};
        bool rootIsMajor{false};
        double comma[12] = {0.0};

        
        void setDegree(int d, int dim)
        {
            
            bool major = (d == 7 || d == 2 || d == 9 || d == 4 || d == 11 || d == 6);
            if (d == 0) major = rootIsMajor;
            
            switch (dim)
            {
                case Three:
                    comma[d] = 1.0;
                    break;
                case Five:
                    comma[d] = jim.comma(jim.syntonic, major);
                    break;
                case Seven:
                    comma[d] = jim.comma(jim.seven, major);
                    break;
                case Eleven:
                    comma[d] = jim.comma(jim.eleven, major);
                    break;
                case Thirteen:
                    comma[d] = jim.comma(jim.thirteen, major);
                    break;
                case Seventeen:
                    comma[d] = jim.comma(jim.seventeen, major);
                    break;
                case Nineteen:
                    comma[d] = jim.comma(jim.nineteen, major);
                    break;
                case Twentythree:
                    comma[d] = jim.comma(jim.twentythree, major);
                    break;
            }
        }
        
        void reset()
        {
            active = false;
            for (int i = 0; i < 12; ++i)
            {
                comma[i] = 1;
            }
        }
        
        void invite()
        {
            active = true;
        }
        
        void unInvite()
        {
            active = false;
        }
        
        void saveInvitation(double *c)
        {
            for (int i = 0; i < 12; ++i)
            {
                c[i] = comma[i];
            }
        }
        
        void loadInvitation(double *c)
        {
            for (int i = 0; i < 12; ++i)
            {
                comma[i] = c[i];
            }
        }
    };
    
    Visitors currentVisitors;
    bool previouslyActive{false};
    
    int visits[12] = {0};


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LatticesEditor)
};
