#include "MainComponent.h"

// TODO
// get the serial data parsing to work
// create sequence ui component
// create sequence type dropdown
// link everything together with the UI

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
  juce::uint8 nn = 60; // C4
  sequencer_.setSequence(
    new BioSignals::FreqSequence({nn+0, nn+2, nn+4, nn+5, nn+7, nn+9, nn+11, nn+12}));
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
  tempoSlider.setTopLeftPosition(10, 10);
  tempoSlider.setSize(200, 50);
  freqSlider.setTopLeftPosition(10, 60);
  freqSlider.setSize(200, 50);
  volumeSlider.setTopLeftPosition(10, 110);
  volumeSlider.setSize(200, 50);
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
  if (source == instream.get()) { // source is the serial input stream
    juce::Logger::getCurrentLogger()->outputDebugString("HERE");
    char buf[1024];
    int num_bytes_read = instream->read(buf, 1024);
    if (num_bytes_read < 0)
    {
      juce::Logger::getCurrentLogger()->outputDebugString("Could not read buffer");
      return; // error
    }
    buf[num_bytes_read] = '\0'; // TODO check
    char sensor_num_str[3];
    sensor_num_str[2] = '\0';
    strncpy(sensor_num_str, buf, 2);
    juce::uint8 sensor_num = (juce::uint8) strtol(sensor_num_str, nullptr, 16);
    juce::Logger::getCurrentLogger()->outputDebugString("sensor_num: " + std::to_string(sensor_num));

    float new_val = strtof(buf + 3, NULL);
    juce::Logger::getCurrentLogger()->outputDebugString("new value: " + std::to_string(new_val));

    tempoSlider.setValue(
        juce::jmin((double) new_val / 100.0, tempoSlider.getMaximum()));
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
