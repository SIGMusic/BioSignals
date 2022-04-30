#include "MainComponent.h"

// TODO
// get the serial data parsing to work
// create sequence type dropdown

enum SensorNums
{
  TEMP1 = 0x01,
  TEMP2 = 0x02,
  PULSE = 0x03,
  ACCLX = 0x04,
  ACCLY = 0x05,
  ACCLZ = 0x06,
};

const static float MIN_TEMP = 20.0f;
const static float MAX_TEMP = 27.0f;

//==============================================================================
MainComponent::MainComponent() : synth_wavetable_(*wavetable_),
                                 sequencer_(synth_wavetable_)
{
  // Make sure you set the size of the component after
  // you add any child components.
  setSize (800, 600);

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
  
  addAndMakeVisible(&sequence_editor_);
  
  for (auto& entry : sequence_editor_.sequence_entries_)
  {
    entry->addChangeListener(this);
  }
  
  // GUI stuffs
  addAndMakeVisible(&tempoSlider);
  tempoSlider.addListener(this);
  tempoSlider.setRange(10.0f, 600.0f);
  
  addAndMakeVisible(&freqSlider);
  freqSlider.setRange(20.0f, 12000.0f);
  freqSlider.addListener(this);
  freqLabel.attachToComponent(&freqSlider, true);

  addAndMakeVisible(&volumeSlider);
  volumeSlider.setRange(0.0f, 1.0f);
  volumeLabel.attachToComponent(&volumeSlider, true);
  volumeSlider.addListener(this);
  
  DebugFunction df = [](juce::String a, juce::String b) {
    juce::Logger* logger = juce::Logger::getCurrentLogger();
    logger->outputDebugString("---juce_serialport---");
    logger->outputDebugString("a: " + a);
    logger->outputDebugString("b: " + b);
  };

  //open the specified port on the system
  juce::StringPairArray portlist = SerialPort::getSerialPortPaths();
  if (portlist.size() == 0)
    juce::Logger::getCurrentLogger()->writeToLog("No serial ports available");
  for (const juce::String& key : portlist.getAllKeys()) {
    juce::Logger::getCurrentLogger()->writeToLog(key);
  }
  for (const juce::String& value : portlist.getAllValues()) {
    juce::Logger::getCurrentLogger()->writeToLog(value);
  }
  
  juce::String selection = getPortBlockingSerialDialog(portlist);
  juce::Logger::getCurrentLogger()->writeToLog("Selection: " + selection);

  sp = std::unique_ptr<SerialPort>(new SerialPort(
    "/dev/cu." + selection,
    SerialPortConfig(9600,
                     8,
                     SerialPortConfig::SERIALPORT_PARITY_NONE,
                     SerialPortConfig::STOPBITS_1,
                     SerialPortConfig::FLOWCONTROL_NONE),
    df
  ));
  if(sp->exists()) {
    //create stream for reading
    instream = std::unique_ptr<SerialPortInputStream>(
      new SerialPortInputStream(sp.get())
    );

    //ask to be notified whenever any character is received
    instream->addChangeListener(this);
    instream->setNotify(SerialPortInputStream::NOTIFY_ON_CHAR, '\n');
    juce::Logger::getCurrentLogger()->writeToLog("opened serial port");
  } else {
    juce::Logger::getCurrentLogger()->writeToLog("NO SERIAL PORT FOUND!!!");
  }
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
  low_pass_filter_ch1.setCoefficients(
      juce::IIRCoefficients::makeLowPass(sampleRate, 1000.0f)
  );
  low_pass_filter_ch2.setCoefficients(
      juce::IIRCoefficients::makeLowPass(sampleRate, 1000.0f)
  );
  sequencer_.setSequence(
    new BioSignals::FreqRandom((std::vector<juce::uint8>) {
      60,
      62,
      64,
      65,
      67,
      69,
      71,
      72
    }));
  sequencer_.prepareToPlay(samplesPerBlockExpected, sampleRate);

  juce::String message;
  message << "Preparing to play audio with ";
  message << samplesPerBlockExpected << " samples per block\n";
  message << "sample rate: " << sampleRate << "\n";
  juce::Logger::getCurrentLogger()->writeToLog(message);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
  sequencer_.getNextAudioBlock(bufferToFill);

  auto* ch1_buffer = bufferToFill.buffer->getWritePointer(0);
  auto* ch2_buffer = bufferToFill.buffer->getWritePointer(1);
  
  for (unsigned int idx = 0; idx < bufferToFill.numSamples; ++idx)
  {
    ch1_buffer[idx] *= volume;
    ch2_buffer[idx] *= volume;
  }

  low_pass_filter_ch1.processSamples(ch1_buffer, bufferToFill.numSamples);
  low_pass_filter_ch2.processSamples(ch2_buffer, bufferToFill.numSamples);
}

void MainComponent::releaseResources()
{
  sequencer_.releaseResources();
  juce::Logger::getCurrentLogger()->writeToLog("Releasing resources");
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
  g.fillAll (getLookAndFeel().findColour (
     juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
  auto area = getLocalBounds();
  auto slider_width = 100;
  auto sequence_editor_height = area.getHeight() - 200;
  sequence_editor_.setBounds(area.removeFromTop(sequence_editor_height));

  tempoSlider.setBounds(area.removeFromLeft(slider_width));
  freqSlider.setBounds(area.removeFromLeft(slider_width));
  volumeSlider.setBounds(area.removeFromLeft(slider_width));
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
  if (source == instream.get()) { // source is the serial input stream
    juce::String line = instream->readNextLine();
    juce::Logger::getCurrentLogger()->writeToLog(line);
    auto* buf = line.getCharPointer().getAddress();
    buf[line.length()] = '\0';
    juce::uint8 sensor_num = buf[0] - 48;
//    juce::Logger::getCurrentLogger()->outputDebugString("sensor_num: " + std::to_string(sensor_num));

    float new_val = strtof(buf + 1, NULL);
//    juce::Logger::getCurrentLogger()->outputDebugString("new value: " + std::to_string(new_val));
    
    if (sensor_num == TEMP2)
    {
      freqSlider.setValue(
          ((new_val - MIN_TEMP) / (MAX_TEMP - MIN_TEMP)) *
           (freqSlider.getMaximum() - freqSlider.getMinimum()) +
          freqSlider.getMinimum());
    }
    else if (sensor_num == PULSE)
    {
//      juce::Logger::getCurrentLogger()->outputDebugString("PULSE");
      tempoSlider.setValue(new_val);
    }
  } else { // from sequencer
//    juce::Logger::getCurrentLogger()->writeToLog("CALLBACK");
    sequencer_.setSequence(
      new BioSignals::FreqRandom(sequence_editor_.getSequence()));
  }
}

void MainComponent::sliderValueChanged(juce::Slider* slider_source)
{
  if (slider_source == &freqSlider)
  {
    low_pass_filter_ch1.setCoefficients(
        juce::IIRCoefficients::makeLowPass(sample_rate, freqSlider.getValue())
    );
    low_pass_filter_ch2.setCoefficients(
        juce::IIRCoefficients::makeLowPass(sample_rate, freqSlider.getValue())
    );
  }
  else if (slider_source == &tempoSlider)
  {
    sequencer_.setTempo(tempoSlider.getValue());
  }
  else if (slider_source == &volumeSlider)
  {
    volume = volumeSlider.getValue();
  }
}

//==============================================================================

juce::String MainComponent::getPortBlockingSerialDialog(
    const juce::StringPairArray& portlist) {
  juce::DialogWindow::LaunchOptions window_launcher;
  window_launcher.dialogTitle = "Select a serial port";
  window_launcher.componentToCentreAround = this;

  juce::ComboBox dropdown;
  window_launcher.content =
    juce::OptionalScopedPointer<juce::Component>(&dropdown, false /* no ownership */);

  const juce::StringArray& ports = portlist.getAllKeys();
  int id = 1;
  for (const juce::String& key : ports) {
    dropdown.addItem(key, id);
    ++id;
  }
  
  dropdown.setSize(300, 30);
  
  juce::String choice;

  dropdown.onChange = [&](void) {
    choice = ports[dropdown.getSelectedId() - 1];
  };
  dropdown.setSelectedId(1);
  
  int status = window_launcher.runModal();
  if (status != 0) {
    juce::JUCEApplicationBase::quit();
  }
  
  return choice;
}
