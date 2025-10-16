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

#include "LatticesProcessor.h"
#include "EveryComponent.h"

//==============================================================================
class LatticesEditor : public juce::AudioProcessorEditor
{
  public:
    LatticesEditor(LatticesProcessor &);
    ~LatticesEditor();

    //==============================================================================

    void paint(juce::Graphics &) override;
    void resized() override;
    void idle();

  private:
    static constexpr int width{1100};
    static constexpr int height{580};

    juce::Colour backgroundColour = juce::Colour{.475f, 1.f, 0.05f, 1.f};
    std::unique_ptr<EveryComponent> everyComponent;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LatticesProcessor &processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LatticesEditor)
};
