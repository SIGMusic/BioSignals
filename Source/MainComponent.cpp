#include "MainComponent.h"

class WavetableOscillator {
public:
  WavetableOscillator (const juce::AudioSampleBuffer& wavetableToUse)
      : wavetable (wavetableToUse)
  {
      jassert (wavetable.getNumChannels() == 1);
  }
  void setFrequency (float frequency, float sampleRate)
  {
      auto tableSizeOverSampleRate = (float) wavetable.getNumSamples() / sampleRate;
      tableDelta = frequency * tableSizeOverSampleRate;
  }
  float getNextSample() noexcept
  {
      auto tableSize = (unsigned int) wavetable.getNumSamples();

      auto index0 = (unsigned int) currentIndex;
      auto index1 = index0 == (tableSize - 1) ? (unsigned int) 0 : index0 + 1;

      auto frac = currentIndex - (float) index0;

      auto* table = wavetable.getReadPointer (0);
      auto value0 = table[index0];
      auto value1 = table[index1];

      auto currentSample = value0 + frac * (value1 - value0);

      if ((currentIndex += tableDelta) > (float) tableSize)
          currentIndex -= (float) tableSize;

      return currentSample;
  }
private:
    const juce::AudioSampleBuffer& wavetable;
    float currentIndex = 0.0f, tableDelta = 0.0f;
};

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

  createSawWavetable();

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
  addAndMakeVisible(&levelSlider);
  levelSlider.setRange(0.0f, 1.0f);
  addAndMakeVisible(&freqSlider);
  freqSlider.setRange(20.0f, 12000.0f);
  freqLabel.attachToComponent(&freqSlider, true);
//  addAndMakeVisible(&qSlider);
//  qSlider.setRange(0.01f, 100.0f);
//  qLabel.attachToComponent(&qSlider, true);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
  sample_rate = sampleRate;

  osc = new WavetableOscillator(sawTable);

  auto midiNote = juce::Random::getSystemRandom().nextDouble() * 36.0 + 48.0;
  auto frequency = 440.0 * pow (2.0, (midiNote - 69.0) / 12.0);

  osc->setFrequency ((float) frequency, (float) sampleRate);
  
  juce::String message;
  message << "Preparing to play audio with ";
  message << samplesPerBlockExpected << " samples per block\n";
  message << "sample rate: " << sampleRate << "\n";
  juce::Logger::getCurrentLogger()->writeToLog(message);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
  auto level = (float) levelSlider.getValue();
  auto freq = (float) freqSlider.getValue();
//  auto q = juce::jmax((float) qSlider.getValue(), 0.01f);
  
//  bp_filter.setCoefficients(
//      juce::IIRCoefficients::makeBandPass(sample_rate, freq, q));
  osc->setFrequency(freq, sample_rate);

  bufferToFill.clearActiveBufferRegion();
  for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel) {
    auto* buffer = bufferToFill.buffer->getWritePointer(channel);
    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample) {
//      buffer[sample] = level * (random.nextFloat() * 0.25f - 0.125f); // noise
      buffer[sample] = level * osc->getNextSample();
    }
//    bp_filter.processSamples(buffer, bufferToFill.numSamples); // filter
  }
    
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    juce::Logger::getCurrentLogger()->writeToLog("Releasing resources");
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
  levelSlider.setTopLeftPosition(10, 10);
  levelSlider.setSize(200, 50);
  freqSlider.setTopLeftPosition(10, 60);
  freqSlider.setSize(200, 50);
//  qSlider.setTopLeftPosition(10, 110);
//  qSlider.setSize(200, 50);
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

void MainComponent::createSawWavetable() {
  sawTable.setSize(1, (int) tableSize);
  auto* sample_arr = sawTable.getWritePointer(0);
  
  // Sine
  auto angleDelta = juce::MathConstants<double>::twoPi / ((double) tableSize - 1);
  /*
  // Saw
  auto angleDelta = 1.0 / (double) (tableSize - 1);
  */
  auto currAngle = 0.0;
  
  for (size_t i = 0; i < tableSize; ++i) {
    auto sample = currAngle;
    sample_arr[i] = (float) sample;
    currAngle += angleDelta;
  }
}
