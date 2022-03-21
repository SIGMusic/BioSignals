#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
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
  addAndMakeVisible(&levelSlider);
  levelSlider.setRange(0.0f, 1.0f);
  addAndMakeVisible(&freqSlider);
  freqSlider.setRange(20.0f, 12000.0f);
  freqLabel.attachToComponent(&freqSlider, true);
  addAndMakeVisible(&qSlider);
  qSlider.setRange(0.01f, 100.0f);
  qLabel.attachToComponent(&qSlider, true);
  
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
    
  sp = std::unique_ptr<SerialPort>(new SerialPort(
    portlist.getAllValues()[0],
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
    instream->setNotify(SerialPortInputStream::NOTIFY_ON_CHAR);
    juce::Logger::getCurrentLogger()->writeToLog("opened serial port");
  } else {
    juce::Logger::getCurrentLogger()->writeToLog("NO SERIAL PORT FOUND!!!");
  }
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
  auto q = juce::jmax((float) qSlider.getValue(), 0.01f);
  
  bp_filter.setCoefficients(
      juce::IIRCoefficients::makeBandPass(sample_rate, freq, q));

  bufferToFill.clearActiveBufferRegion();
  for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel) {
    auto* buffer = bufferToFill.buffer->getWritePointer(channel);
    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample) {
      buffer[sample] = level * (random.nextFloat() * 0.25f - 0.125f); // noise
    }
    bp_filter.processSamples(buffer, bufferToFill.numSamples); // filter
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
  qSlider.setTopLeftPosition(10, 110);
  qSlider.setSize(200, 50);
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
  juce::Logger::getCurrentLogger()->outputDebugString("HERE");
  if (source == instream.get()) {
    juce::Logger::getCurrentLogger()->outputDebugString("HERE");
    char* buf = new char[16];
    instream->read(buf, 16);
    long new_val = strtol(buf, nullptr, 10);
    levelSlider.setValue(
        juce::jmin((double) new_val / 100.0, levelSlider.getMaximum()));
  }
}
