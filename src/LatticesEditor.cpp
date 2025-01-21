/*
  Lattices - A Just-Intonation graphical MTS-ESP Source

  Copyright 2023-2024 Andreya Ek Frisk and Paul Walker.

  This code is released under the MIT licence, but do note that it depends
  on the JUCE library, see licence for more details.

  Source available at https://github.com/Andreya-Autumn/lattices
*/

#include "LatticesProcessor.h"
#include "LatticesEditor.h"

//==============================================================================
LatticesEditor::LatticesEditor(LatticesProcessor &p) : juce::AudioProcessorEditor(&p), processor(p)
{
    everyComponent = std::make_unique<EveryComponent>(p, width, height);
    everyComponent->setBufferedToImage(true);
    everyComponent->setBounds(0, 0, width, height);
    addAndMakeVisible(*everyComponent);

    setSize(width, height);
    getConstrainer()->setFixedAspectRatio(height / width);

    setResizable(true, true);
}

LatticesEditor::~LatticesEditor() {}

//==============================================================================

void LatticesEditor::paint(juce::Graphics &g) { g.fillAll(backgroundColour); }

void LatticesEditor::idle() {}

void LatticesEditor::resized()
{
    auto b = this->getLocalBounds();

    float scale = b.getWidth() / everyComponent->backgroundWidth();
    everyComponent->setTransform(juce::AffineTransform().scaled(scale));
}
