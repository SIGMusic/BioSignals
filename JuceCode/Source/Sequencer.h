/*
  ==============================================================================

    Arpeggiator.h
    Created: 28 Mar 2022 4:57:13pm
    Author:  Andrew Orals

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>

class Sequencer : public juce::AudioSource
{
public:
  Sequencer(const juce::Array<juce::uint8>& sequence, double tempo = 60.0);

  /*
  *  Set the tempo of this Sequencer.
  *
  *  @param notesPerMinute how quickly to change sequencer steps,
                           e.g. 60.0 times per minute
  */
  void setTempo(double notesPerMinute);

  virtual void prepareToPlay(
      int samplesPerBlockExpected, double sampleRate) override;

  virtual void releaseResources() override;

  virtual void getNextAudioBlock(
      const juce::AudioSourceChannelInfo &bufferToFill) override;

private:
  static inline double midiToFreq(juce::uint8 midi_note);

  juce::Array<juce::uint8> sequence_;
  juce::ToneGeneratorAudioSource synth_;
  int samplesPerBlockExpected_;
  double sampleRate_ = 48000.0 /* default sample rate */;

  size_t samplesPerNote_;
  size_t currPeriodSamples_;
  juce::uint8 currIdx_;
};
