/*
  ==============================================================================

    Arpeggiator.cpp
    Created: 28 Mar 2022 4:57:13pm
    Author:  Andrew Orals

  ==============================================================================
*/

#include "Sequencer.h"

Sequencer::Sequencer(const juce::Array<juce::uint8>& sequence, double tempo)
{
  sequence_ = sequence;
  notesPerMinute_ = tempo;
}

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
  notesPerMinute_ = notesPerMinute;
}

void Sequencer::prepareToPlay(
    int samplesPerBlockExpected, double sampleRate) override
{
  samplesPerBlockExpected_ = samplesPerBlockExpected;
  sampleRate_ = sampleRate;
}

void Sequencer::releaseResources() override
{
  
}

void Sequencer::getNextAudioBlock(
    const juce::AudioSourceChannelInfo &bufferToFill) override
{  
  // if past threshold, then change frequency of synthesizer
  // [6 3 8 5 2]
  
}

