/*
  ==============================================================================

    WavetableOsc.cpp
    Created: 28 Apr 2022 8:56:08pm
    Author:  Andrew Orals

  ==============================================================================
*/

#include "WavetableOsc.h"

namespace BioSignals
{

WavetableOscillator::WavetableOscillator (
    const juce::AudioSampleBuffer& wavetableToUse)
        : wavetable (wavetableToUse),
          tableSize (wavetable.getNumSamples() - 1)
{
  juce::Logger::getCurrentLogger()->writeToLog("Num samples: " + std::to_string(tableSize));
}

void WavetableOscillator::setAmplitude(float amp)
{
  juce::Logger::getCurrentLogger()->writeToLog("Amp: " + std::to_string(amp));
  amp_ = amp;
}

void WavetableOscillator::setFrequency(float frequency)
{
  juce::Logger::getCurrentLogger()->writeToLog("Freq: " + std::to_string(frequency));
  frequency_ = frequency;
  float tableSizeOverSampleRate = (float) tableSize / sampleRate_;
  tableDelta = frequency_ * tableSizeOverSampleRate;
}

void WavetableOscillator::prepareToPlay(
    int samplesPerBlockExpected, double sampleRate)
{
  (void) samplesPerBlockExpected; // into the abyss...
  sampleRate_ = sampleRate;
  juce::Logger::getCurrentLogger()->writeToLog("Sample rate: " + std::to_string(sampleRate));
  float tableSizeOverSampleRate = (float) tableSize / sampleRate;
  tableDelta = frequency_ * tableSizeOverSampleRate;
  juce::Logger::getCurrentLogger()->writeToLog("tableDelta: " + std::to_string(tableDelta));
}

void WavetableOscillator::getNextAudioBlock(
    const juce::AudioSourceChannelInfo &bufferToFill)
{
  bufferToFill.clearActiveBufferRegion();
  auto* buf0 = bufferToFill.buffer->getWritePointer(0);
  
  for (unsigned int idx = 0; idx < bufferToFill.numSamples; ++idx)
  {
    buf0[idx] = amp_ * getNextSample();
  }
  
  for (unsigned int chan_idx = 1;
       chan_idx < bufferToFill.buffer->getNumChannels();
       ++chan_idx)
  {
    auto* buf = bufferToFill.buffer->getWritePointer(chan_idx);
    for (unsigned int idx = 0; idx < bufferToFill.numSamples; ++idx)
    {
      buf[idx] = buf0[idx];
    }
  }
}

std::unique_ptr<juce::AudioSampleBuffer> WavetableOscillator::createWavetableBLITSaw(
    unsigned int table_size, unsigned int num_harmonics)
{
  jassert(table_size > 0 && num_harmonics > 0);

  std::unique_ptr<juce::AudioSampleBuffer> wavetable(
      new juce::AudioSampleBuffer(1, table_size + 1)
  );

  auto* samples = wavetable->getWritePointer(0);
  double angle_delta = juce::MathConstants<double>::twoPi /
                           (double) (table_size - 1);
  
  auto saw_fn = [&angle_delta, &num_harmonics] (unsigned int table_idx) {
    float curr_sample = 0.0f;
    for (unsigned int harm_idx = 0; harm_idx < num_harmonics; ++harm_idx)
    {
      curr_sample += ((1.0f / ((float) harm_idx + 1.0f)) *
                      std::sin(angle_delta * (harm_idx + 1) * table_idx));
    }
    return curr_sample;
  };

  float min = samples[0] = saw_fn(0);
  float max = min;
  for (unsigned int table_idx = 1; table_idx < table_size; ++table_idx)
  {
    float curr_sample = saw_fn(table_idx);
    samples[table_idx] = curr_sample;

    max = curr_sample > max ? curr_sample : max;
    min = curr_sample < min ? curr_sample : min;
  }
  
  // normalize table
  for (unsigned int table_idx = 0; table_idx < table_size; ++table_idx)
  {
    samples[table_idx] = 2.0f * (samples[table_idx] - min) / (max - min) - 1.0f;
  }
  
  samples[table_size] = samples[0];

  return wavetable;
}

} // namespace BioSignals
