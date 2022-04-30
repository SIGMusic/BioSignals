/*
  ==============================================================================

    WavetableOsc.h
    Created: 11 Apr 2022 4:41:13pm
    Author:  Andrew Orals

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace BioSignals
{

/**
 From the juce tutorial
 */
class WavetableOscillator : public juce::ToneGeneratorAudioSource
{
public:
  WavetableOscillator (const juce::AudioSampleBuffer& wavetableToUse);
  
  void setAmplitude(float amp);

  void setFrequency(float frequency);
  
  virtual void prepareToPlay(
      int samplesPerBlockExpected, double sampleRate) override;

  virtual void releaseResources() override { /* Nothing */ }

  virtual void getNextAudioBlock(
      const juce::AudioSourceChannelInfo &bufferToFill) override;
  
  static std::unique_ptr<juce::AudioSampleBuffer> createWavetableBLITSaw(
      unsigned int table_size, unsigned int num_harmonics);

//  static WavetableOscillator& loadFromFile(const std::ifstream& input_file)
//  {
//
//  }
private:
  forcedinline float getNextSample() noexcept
  {
      auto index0 = (unsigned int) currentIndex;
      auto index1 = index0 + 1;

      auto frac = currentIndex - (float) index0;

      auto* table = wavetable.getReadPointer (0);
      auto value0 = table[index0];
      auto value1 = table[index1];

      auto currentSample = value0 + frac * (value1 - value0);

      if ((currentIndex += tableDelta) > (float) tableSize)
          currentIndex -= (float) tableSize;

      return currentSample;
  }

  const juce::AudioSampleBuffer& wavetable;
  const int tableSize;
  double sampleRate_ = 48000.0;
  float amp_ = 0.5f;
  float frequency_ = 440.0f;
  float currentIndex = 0.0f, tableDelta = 0.0f;
};

} // namespace BioSignals
