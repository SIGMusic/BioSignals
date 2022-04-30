/*
  ==============================================================================

    SequenceEditor.cpp
    Created: 30 Apr 2022 12:14:39am
    Author:  Andrew Orals

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SequenceEditor.h"

namespace BioSignals
{

SequenceEntry::SequenceEntry(float init_freq) :
        freq_(init_freq)
{
  juce::Logger::getCurrentLogger()->writeToLog("INIT FREQ: " + std::to_string(init_freq));
  addAndMakeVisible(&note_dropdown_);
  addAndMakeVisible(&octave_dropdown_);
  addAndMakeVisible(&freq_editor_);

  note_dropdown_.addItemList({
    "C",
    "C#/Db",
    "D",
    "D#/Eb",
    "E",
    "F",
    "F#/Gb",
    "G",
    "G#/Ab",
    "A",
    "A#/Bb",
    "B",
  }, 1);
  octave_dropdown_.addItemList({
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
  }, 1);
  freq_editor_.setEditable(true, false, true);
  freq_editor_.setFont(juce::Font(14.0f, juce::Font::bold));
  freq_editor_.setColour(juce::Label::textColourId, juce::Colours::white);
  freq_editor_.setJustificationType(juce::Justification::centred);
  
  auto freq_change_fn = [this]()
  {
    std::stringstream s(freq_editor_.getText().toStdString());
    float new_freq;
    if (!(s >> new_freq))
      new_freq = 440.0f;
    else
      new_freq > 20000.0f || new_freq < 20.0f ? freq_ =
        440.0f : freq_ = new_freq;
    auto midi_num = FrequencyGenerator::freqToMidi(freq_);
    note_dropdown_.setSelectedId(midi_num % 12 + 1, juce::NotificationType::dontSendNotification);
    octave_dropdown_.setSelectedId(midi_num / 12 + 1, juce::NotificationType::dontSendNotification);
    freq_editor_.setText(std::to_string(freq_), juce::dontSendNotification);
    sendChangeMessage();
  };

  auto midi_change_fn = [this]()
  {
    auto note_num = note_dropdown_.getSelectedItemIndex();
    auto octave_num = octave_dropdown_.getSelectedItemIndex();
    freq_ = FrequencyGenerator::midiToFreq(octave_num * 12 + note_num);
    freq_editor_.setText(std::to_string(freq_), juce::dontSendNotification);
    sendChangeMessage();
  };

  note_dropdown_.onChange = midi_change_fn;
  octave_dropdown_.onChange = midi_change_fn;
  freq_editor_.onTextChange = freq_change_fn;

  freq_editor_.setText(std::to_string(init_freq), juce::sendNotification);
}

SequenceEntry::~SequenceEntry()
{ /* Nothing */ }

void SequenceEntry::paint(juce::Graphics& g)
{
  g.fillAll(juce::Colours::black);
  g.setColour(juce::Colours::white);
  g.drawRect(getLocalBounds(), 2);
}

void SequenceEntry::resized()
{
  auto area = getLocalBounds();
  auto top_margin = 40;
  auto dropdown_height = 24;
  auto spacer = 20;
  auto editor_height = 32;
  area.setWidth(area.getWidth() / 1.5f);
  area.setX(area.getX() + area.getWidth() / 3.0f);
  area.removeFromTop(top_margin);
  note_dropdown_.setBounds(area.removeFromTop(dropdown_height));
  area.removeFromTop(spacer);
  octave_dropdown_.setBounds(area.removeFromTop(dropdown_height));
  area.removeFromTop(spacer);
  freq_editor_.setBounds(area.removeFromTop(editor_height));
}

float SequenceEntry::getFreq() const
{
  return freq_;
}

//==============================================================================
SequenceEditor::SequenceEditor()
{
  const float notes[7] = {
    FrequencyGenerator::midiToFreq(60),
    FrequencyGenerator::midiToFreq(62),
    FrequencyGenerator::midiToFreq(64),
    FrequencyGenerator::midiToFreq(65),
    FrequencyGenerator::midiToFreq(67),
    FrequencyGenerator::midiToFreq(69),
    FrequencyGenerator::midiToFreq(71),
  };
  for (unsigned int idx = 0; idx < 7; ++idx)
  {
    auto* entry = new SequenceEntry(notes[idx]);
    sequence_entries_.push_back(std::unique_ptr<SequenceEntry>(entry));
    addAndMakeVisible(entry);
  }
}

SequenceEditor::~SequenceEditor()
{ /* Nothing */ }

void SequenceEditor::paint (juce::Graphics& g)
{
  g.fillAll(getLookAndFeel().findColour(
    juce::ResizableWindow::backgroundColourId)
  );
}

void SequenceEditor::resized()
{
  auto area = getLocalBounds();
  auto entry_width = area.getWidth() / sequence_entries_.size();
  for (auto& entry : sequence_entries_)
  {
    entry->setBounds(area.removeFromLeft((int) entry_width));
  }
}

std::vector<float> SequenceEditor::getSequence() const
{
  std::vector<float> arr(sequence_entries_.size());
  for (unsigned int idx = 0; idx < sequence_entries_.size(); ++idx)
  {
    arr[idx] = sequence_entries_[idx]->getFreq();
  }
  return arr;
}

} // namespace BioSignals
