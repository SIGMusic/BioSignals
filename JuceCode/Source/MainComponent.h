#pragma once

#include <JuceHeader.h>
#include "JUCESerial/juce_serialport.h"
#include "SequenceEditor.h"
#include "Sequencer.h"
#include "WavetableOsc.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener,
                       public juce::Slider::Listener
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
  void sliderValueChanged(juce::Slider* slider_source) override;

  //==============================================================================
  void paint (juce::Graphics& g) override;
  void resized() override;

private:
  juce::String getPortBlockingSerialDialog(const juce::StringPairArray& portlist);
  //==============================================================================
  std::unique_ptr<juce::AudioSampleBuffer> wavetable_ =
          BioSignals::WavetableOscillator::createWavetableBLITSaw(8192, 27);
  BioSignals::WavetableOscillator synth_wavetable_;
  BioSignals::Sequencer sequencer_;
  juce::IIRFilter low_pass_filter_ch1;
  juce::IIRFilter low_pass_filter_ch2;

  BioSignals::SequenceEditor sequence_editor_;
  
  juce::Slider tempoSlider{juce::Slider::SliderStyle::LinearHorizontal,
                           juce::Slider::TextEntryBoxPosition::TextBoxBelow};
  juce::Slider freqSlider{juce::Slider::SliderStyle::LinearHorizontal,
                          juce::Slider::TextEntryBoxPosition::TextBoxBelow};
  juce::Label freqLabel;
  juce::Slider volumeSlider{juce::Slider::SliderStyle::LinearHorizontal,
                            juce::Slider::TextEntryBoxPosition::TextBoxBelow};

  juce::Label volumeLabel;

  float volume = 0.0f;
  double sample_rate = 48000.0;

  std::unique_ptr<SerialPort> sp;
  std::unique_ptr<SerialPortInputStream> instream;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
