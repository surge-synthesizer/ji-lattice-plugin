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
#include "melatonin_inspector/melatonin_inspector.h"
#include "LatticesProcessor.h"
#include "LatticeComponent.h"
#include "ModeComponent.h"
#include "MIDIMenuComponent.h"
#include "OriginComponent.h"
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

    void showTuningMenu();
    void showMidiMenu();
    void resetMTS();

    void timerCallback(int timerID) override;

    //    std::unique_ptr<juce::Timer> idleTimer;
    void idle();

  private:
    static constexpr int width{900};
    static constexpr int height{600};

    melatonin::Inspector inspector{*this};

    juce::Colour backgroundColour = juce::Colour{.5f, .5f, 0.f, 1.f};

    std::unique_ptr<LatticeComponent> latticeComponent;

    std::unique_ptr<juce::TextButton> tuningButton;
    std::unique_ptr<OriginComponent> originComponent;
    std::unique_ptr<ModeComponent> modeComponent;

    std::unique_ptr<juce::TextButton> midiButton;
    std::unique_ptr<MIDIMenuComponent> midiComponent;

    std::unique_ptr<MTSWarningComponent> warningComponent;

    void init();
    bool inited{false};

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LatticesProcessor &processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LatticesEditor)
};
