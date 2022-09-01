/*
  ==============================================================================

    Arpeggiator.h
    Created: 28 Mar 2022 4:57:13pm
    Author:  Andrew Orals

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>
#include <vector>
#include "WavetableOsc.h"

namespace BioSignals
{

enum GeneratorType {
  SEQUENCE,
  RANDOM
};

const extern std::pair<GeneratorType, const char*> generator_types[2];

class FrequencyGenerator
{
public:
  virtual ~FrequencyGenerator() = default;
  virtual double getNextFreq() = 0;
  static constexpr inline float midiToFreq(juce::uint8 midi_note)
  {
    return 440.0 * std::powf(2.0, (midi_note - 69) / 12.0);
  }
  static inline juce::uint8 freqToMidi(float freq)
  {
    return (juce::uint8) ((12 * std::log(freq / 220.0) / std::log(2.0)) + 57.01);
  }
};

class FreqSequence : public FrequencyGenerator
{
public:
  FreqSequence(const std::vector<juce::uint8>& sequence)
  {
    sequence_.resize(sequence.size());
    for (unsigned int idx = 0; idx < sequence.size(); ++idx)
    {
      sequence_[idx] = midiToFreq(sequence[idx]);
    }
  };
  FreqSequence(const std::vector<float>& sequence) :
      sequence_(sequence) { };
  ~FreqSequence() = default;
  
  virtual double getNextFreq() override
  {
    return sequence_[++currIdx_ %= sequence_.size()];
  }
private:
  std::vector<float> sequence_;
  juce::uint8 currIdx_ = 0;
};

class FreqRandom : public FrequencyGenerator
{
public:
  FreqRandom(const std::vector<juce::uint8>& sequence)
  {
    sequence_.resize(sequence.size());
    for (unsigned int idx = 0; idx < sequence.size(); ++idx)
    {
      sequence_[idx] = midiToFreq(sequence[idx]);
    }
  };
  FreqRandom(const std::vector<float>& sequence) :
      sequence_(sequence) { };
  ~FreqRandom() = default;
  
  virtual double getNextFreq() override
  {
    float rand = juce::Random::getSystemRandom().nextFloat();
    if (rand < threshold)
    {
      return sequence_[++currIdx_ %= sequence_.size()];
    }
    else
    {
      currIdx_ = juce::Random::getSystemRandom().nextInt() % sequence_.size();
      return sequence_[currIdx_];
    }
  }
private:
  float threshold = 0.5f;
  std::vector<float> sequence_;
  juce::uint8 currIdx_ = 0;
};

FrequencyGenerator* constructFreqGenerator(GeneratorType gen_type,
                                           const std::vector<float>& sequence);

class Sequencer : public juce::AudioSource
{
public:
  Sequencer(BioSignals::WavetableOscillator& tgas) :
          Sequencer(tgas, nullptr) { /* nothing */ }
  Sequencer(BioSignals::WavetableOscillator& tgas,
            FrequencyGenerator* fg,
            double tempo = 60.0);
//  Sequencer(Sequencer& other);
//  Sequencer& operator=(Sequencer& other);
  ~Sequencer() = default;

  /*
  *  Set the tempo of this Sequencer.
  *
  *  @param notesPerMinute how quickly to change sequencer steps,
  *                        e.g. 60.0 times per minute
  */
  void setTempo(double notesPerMinute);
  
  void setSequence(FrequencyGenerator* fg);

  virtual void prepareToPlay(
      int samplesPerBlockExpected, double sampleRate) override;

  virtual void releaseResources() override;

  virtual void getNextAudioBlock(
      const juce::AudioSourceChannelInfo &bufferToFill) override;

private:
  std::unique_ptr<FrequencyGenerator> freqGen_;
  BioSignals::WavetableOscillator& synth_;
  int samplesPerBlockExpected_;
  double sampleRate_ = 48000.0 /* default sample rate */;

  size_t samplesPerNote_;
  size_t currPeriodSamples_;
};

} // namespace BioSignals
