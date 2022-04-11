/*
  ==============================================================================

    Arpeggiator.cpp
    Created: 28 Mar 2022 4:57:13pm
    Author:  Andrew Orals

  ==============================================================================
*/

#include "Sequencer.h"

Sequencer::Sequencer(const juce::Array<juce::uint8>& sequence, double tempo) :
    sequence_(sequence), samplesPerNote_(sampleRate_) { }

/*
*  Set the tempo of this Sequencer.
*
*  @param notesPerMinute how quickly to change sequencer steps,
                         e.g. 60.0 times per minute
*/
void Sequencer::setTempo(double notesPerMinute)
{
  // sampleRate = samps/sec
  // notesPerMinute = notes/min = notes/(60 sec)
  // samplesPerSecond / notesPerSecond = samps/note
  samplesPerNote_ = (size_t) (sampleRate_ / (notesPerMinute / 60.0));
}

void Sequencer::prepareToPlay(
    int samplesPerBlockExpected, double sampleRate)
{
  samplesPerBlockExpected_ = samplesPerBlockExpected;
  sampleRate_ = sampleRate;
}

void Sequencer::releaseResources()
{
  // nothing for now
}

void Sequencer::getNextAudioBlock(
    const juce::AudioSourceChannelInfo &bufferToFill)
{
  for (size_t curr_sample = 0;
       curr_sample < samplesPerBlockExpected_;
       ++curr_sample, ++currPeriodSamples_)
  {
    // if past threshold, then change frequency of synthesizer
    if (currPeriodSamples_ > samplesPerNote_)
    {
      currPeriodSamples_ = 0;
      synth_.setFrequency(midiToFreq((++currIdx_) % sequence_.size()));
    }
    
    // fill the buffer here
  }
}

inline double Sequencer::midiToFreq(juce::uint8 midi_note) {
  return 440.0 * std::powf(2.0, midi_note / 12.0);
}
