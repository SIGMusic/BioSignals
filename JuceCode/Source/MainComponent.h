#pragma once

#include <JuceHeader.h>
#include "JUCESerial/juce_serialport.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class WavetableOscillator;

class MainComponent  : public juce::AudioAppComponent, public juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::String getPortBlockingSerialDialog(const juce::StringPairArray& portlist);
    //==============================================================================
    juce::Random random;
    juce::IIRFilter bp_filter;
    juce::IIRCoefficients filter_params;

    juce::Slider levelSlider{juce::Slider::SliderStyle::LinearHorizontal,
                             juce::Slider::TextEntryBoxPosition::TextBoxBelow};
    juce::Slider freqSlider{juce::Slider::SliderStyle::LinearHorizontal,
                            juce::Slider::TextEntryBoxPosition::TextBoxBelow};
    juce::Label freqLabel;
    juce::Slider qSlider{juce::Slider::SliderStyle::LinearHorizontal,
                            juce::Slider::TextEntryBoxPosition::TextBoxBelow};

    juce::Label qLabel;
    double sample_rate = 48000.0;
  
    std::unique_ptr<SerialPort> sp;
    std::unique_ptr<SerialPortInputStream> instream;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
