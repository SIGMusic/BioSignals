#pragma once

#include <JuceHeader.h>
#include "JUCESerial/juce_serialport.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class WavetableOscillator;

class MainComponent  : public juce::AudioAppComponent
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
  void createSawWavetable();
    //==============================================================================
//    juce::Random random;
//    juce::IIRFilter bp_filter;
//    juce::IIRCoefficients filter_params;
  const unsigned int tableSize = 1 << 7;
  juce::AudioSampleBuffer sawTable;
  WavetableOscillator* osc;

    juce::Slider levelSlider{juce::Slider::SliderStyle::LinearHorizontal,
                             juce::Slider::TextEntryBoxPosition::TextBoxBelow};
    juce::Slider freqSlider{juce::Slider::SliderStyle::LinearHorizontal,
                            juce::Slider::TextEntryBoxPosition::TextBoxBelow};
    juce::Label freqLabel;
//    juce::Slider qSlider{juce::Slider::SliderStyle::LinearHorizontal,
//                            juce::Slider::TextEntryBoxPosition::TextBoxBelow};
//    juce::Label qLabel;
    double sample_rate = 48000.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
