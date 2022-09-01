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

const extern std::pair<GeneratorType, const char*> generator_types[2] = {{RANDOM, "Random"}, {SEQUENCE, "Sequence"}};


FrequencyGenerator* constructFreqGenerator(GeneratorType gen_type,
                                           const std::vector<float>& sequence)
{
  switch (gen_type) {
    case SEQUENCE:
      return new FreqSequence(sequence);
      break;
    case RANDOM:
      return new FreqRandom(sequence);
      break;
    default:
      return nullptr;
  }
}

Sequencer::Sequencer(BioSignals::WavetableOscillator& tgas,
                     FrequencyGenerator* fg,
                     double tempo) :
    freqGen_(fg), synth_(tgas), samplesPerNote_(sampleRate_) { }

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
