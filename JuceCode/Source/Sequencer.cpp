/*
  ==============================================================================

    Arpeggiator.cpp
    Created: 28 Mar 2022 4:57:13pm
    Author:  Andrew Orals

  ==============================================================================
*/

#include "Sequencer.h"

namespace BioSignals
{

Sequencer::Sequencer(BioSignals::WavetableOscillator& tgas,
                     FrequencyGenerator* fg,
                     double tempo) :
    freqGen_(fg), synth_(tgas), samplesPerNote_(sampleRate_) { }

//Sequencer::Sequencer(Sequencer& other) : synth_(other.synth_)
//{
//  freqGen_ = other.freqGen_;
//  samplesPerBlockExpected_ = other.samplesPerBlockExpected_;
//  sampleRate_ = other.sampleRate_;
//  samplesPerNote_ = other.samplesPerNote_;
//  currPeriodSamples_ = other.currPeriodSamples_;
//  prepareToPlay(samplesPerBlockExpected_, sampleRate_);
//}

//Sequencer& Sequencer::operator=(Sequencer& other)
//{
//  freqGen_ = other.freqGen_;
//  samplesPerBlockExpected_ = other.samplesPerBlockExpected_;
//  sampleRate_ = other.sampleRate_;
//  samplesPerNote_ = other.samplesPerNote_;
//  currPeriodSamples_ = other.currPeriodSamples_;
//  prepareToPlay(samplesPerBlockExpected_, sampleRate_);
//  return *this;
//}

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

void Sequencer::setSequence(FrequencyGenerator* fg)
{
  freqGen_ = std::unique_ptr<FrequencyGenerator>(fg);
}

void Sequencer::prepareToPlay(
    int samplesPerBlockExpected, double sampleRate)
{
  samplesPerBlockExpected_ = samplesPerBlockExpected;
  sampleRate_ = sampleRate;
  synth_.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void Sequencer::releaseResources()
{
  synth_.releaseResources();
}

void Sequencer::getNextAudioBlock(
    const juce::AudioSourceChannelInfo &bufferToFill)
{
  if (!freqGen_)
    return; // not ready yet


  // if past threshold, then change frequency of synthesizer
  if (currPeriodSamples_ > samplesPerNote_)
  {
    currPeriodSamples_ = 0;
    double new_freq = freqGen_->getNextFreq();
    synth_.setFrequency(new_freq);
//      juce::Logger::getCurrentLogger()->writeToLog(
//          "new frequency: " + std::to_string(new_freq));
  }

  synth_.getNextAudioBlock(bufferToFill);
  
  currPeriodSamples_ += bufferToFill.numSamples;
}

} // namespace BioSignals
