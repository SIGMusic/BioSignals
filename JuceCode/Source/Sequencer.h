/*
  ==============================================================================

    Arpeggiator.h
    Created: 28 Mar 2022 4:57:13pm
    Author:  Andrew Orals

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>

class FrequencyGenerator
{
public:
  virtual ~FrequencyGenerator() = default;
  virtual double getNextFreq() = 0;
  static inline double midiToFreq(juce::uint8 midi_note)
  {
    return 440.0 * std::powf(2.0, midi_note / 12.0);
  }
};

class FreqSequence : public FrequencyGenerator
{
public:
  FreqSequence(const juce::Array<juce::uint8>& sequence) :
      sequence_(sequence) { };
  ~FreqSequence() = default;
  
  virtual double getNextFreq() override
  {
    return midiToFreq(sequence_[++currIdx_] % sequence_.size());
  }
private:
  juce::Array<juce::uint8> sequence_;
  juce::uint8 currIdx_ = 0;
};

class Sequencer : public juce::AudioSource
{
public:
  Sequencer(FrequencyGenerator* fg, double tempo = 60.0);

  /*
  *  Set the tempo of this Sequencer.
  *
  *  @param notesPerMinute how quickly to change sequencer steps,
  *                        e.g. 60.0 times per minute
  */
  void setTempo(double notesPerMinute);

  virtual void prepareToPlay(
      int samplesPerBlockExpected, double sampleRate) override;

  virtual void releaseResources() override;

  virtual void getNextAudioBlock(
      const juce::AudioSourceChannelInfo &bufferToFill) override;

private:
  std::unique_ptr<FrequencyGenerator> freqGen_;
  juce::ToneGeneratorAudioSource synth_;
  int samplesPerBlockExpected_;
  double sampleRate_ = 48000.0 /* default sample rate */;

  size_t samplesPerNote_;
  size_t currPeriodSamples_;
};
