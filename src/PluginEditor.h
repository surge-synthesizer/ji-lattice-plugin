
#pragma once

#include <memory>

#include <juce_audio_processors/juce_audio_processors.h>
#include "melatonin_inspector/melatonin_inspector.h"
#include "PluginProcessor.h"
#include "LatticeComponent.h"
#include "ModeComponent.h"
#include "MIDIMenuComponent.h"
#include "OriginComponent.h"

//==============================================================================
/**
*/
class LatticesEditor : public juce::AudioProcessorEditor, juce::Timer
{
public:
  LatticesEditor(LatticesProcessor &);
    ~LatticesEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void showModeMenu();
    void showMidiMenu();
    void showOriginMenu();
    
    void timerCallback() override;
    
    std::unique_ptr<juce::Timer> idleTimer;
    void idle();
    

private:
    static constexpr int width{900};
    static constexpr int height{600};
    
//    class altLAF : public juce::LookAndFeel_V4
//    {
//    public:
//        altLAF(){}
//        ~altLAF(){}
//        
//        void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool, bool isButtonDown) override
//        {
//            auto cornerSize = 6.0f;
//            auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);
//            
//            auto baseColour = backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f).withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);
//
//            if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
//                baseColour = baseColour.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.05f);
//            
//            g.setColour (baseColour);
//            
//            auto flatOnLeft   = button.isConnectedOnLeft();
//            auto flatOnRight  = button.isConnectedOnRight();
//            auto flatOnTop    = button.isConnectedOnTop();
//            auto flatOnBottom = button.isConnectedOnBottom();
//            
//            if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
//            {
//                Path path;
//                path.addRoundedRectangle (bounds.getX(), bounds.getY(),
//                                              bounds.getWidth(), bounds.getHeight(),
//                                              cornerSize, cornerSize,
//                                              ! (flatOnLeft  || flatOnTop),
//                                              ! (flatOnRight || flatOnTop),
//                                              ! (flatOnLeft  || flatOnBottom),
//                                              ! (flatOnRight || flatOnBottom));
//
//                    g.fillPath (path);
//
//                    g.setColour (button.findColour (ComboBox::outlineColourId));
//                    g.strokePath (path, PathStrokeType (1.0f));
//                }
//                else
//                {
//                    g.fillRoundedRectangle (bounds, cornerSize);
//
//                    g.setColour (button.findColour (ComboBox::outlineColourId));
//                    g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
//                }
//            }
//    };
    
    melatonin::Inspector inspector { *this };
    
    std::unique_ptr<LatticeComponent> latticeComponent;
    
    std::unique_ptr<juce::TextButton> modeButton;
    std::unique_ptr<ModeComponent> modeComponent;
    
    std::unique_ptr<juce::TextButton> midiButton;
    std::unique_ptr<MIDIMenuComponent> midiComponent;
    
    std::unique_ptr<juce::TextButton> originButton;
    std::unique_ptr<OriginComponent> originComponent;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LatticesProcessor &processor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LatticesEditor)
};
