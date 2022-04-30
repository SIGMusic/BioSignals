/*
  ==============================================================================

    SequenceEditor.h
    Created: 30 Apr 2022 12:14:39am
    Author:  Andrew Orals

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include "Sequencer.h"

namespace BioSignals
{

//==============================================================================
/*
*/
class SequenceEntry : public juce::Component, public juce::ChangeBroadcaster
{
public:
  SequenceEntry(float init_freq);
  SequenceEntry() : SequenceEntry(440.0f) { /* Nothing */ };
  ~SequenceEntry() override;
  
  void paint(juce::Graphics&) override;
  void resized() override;
  float getFreq() const;

  std::function<void()> onChange;
private:
  float freq_;
  juce::ComboBox note_dropdown_;
  juce::ComboBox octave_dropdown_;
  juce::Label    freq_editor_;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequenceEntry)
};

class SequenceEditor  : public juce::Component
{
public:
  SequenceEditor();
  ~SequenceEditor() override;

  void paint (juce::Graphics&) override;
  void resized() override;
  
//======================================================
  std::vector<float> getSequence() const;
  std::vector<std::unique_ptr<SequenceEntry>> sequence_entries_;
private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequenceEditor)
};

} // namespace BioSignals
