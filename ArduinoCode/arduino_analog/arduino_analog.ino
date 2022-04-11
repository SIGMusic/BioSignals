#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math.
#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library.

const int PulseWire = 0;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
const int LED13 = 13;          // The on-board Arduino LED, close to PIN 13.
static int Threshold = 550;           // Determine which Signal to "count as a beat" and which to ignore.
                               // Use the "Gettting Started Project" to fine-tune Threshold Value beyond default setting.
                               // Otherwise leave the default "550" value.

// Signal Identifiers
#define TEMP1 (0x01)
#define TEMP2 (0x02)
#define PULSE (0x03)

// Signal Pins
#define PIN_TEMP1 (A1)
#define PIN_TEMP2 (A2)
#define PIN_PULSE (A0)

// For debugging mode uncomment this line
#define DEBUG (1)

PulseSensorPlayground pulseSensor;  // Creates an instance of the PulseSensorPlayground object called "pulseSensor"

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // Configure the PulseSensor object, by assigning our variables to it. 
  pulseSensor.analogInput(PulseWire);   
  pulseSensor.blinkOnPulse(LED13);       //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);   

  // Double-check the "pulseSensor" object was created and "began" seeing a signal. 
   if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset.  
  }
}

#ifndef DEBUG
void loop() {
  rw_temp1();
  rw_temp2();
  rw_pulse();
}
#else
void loop() {
  rw_temp1_debug();
  rw_temp2_debug();
  rw_pulse_debug();
}
#endif // DEBUG

void rw_temp1() {
//Temperature Sensor 1:
  int sensorValue = analogRead(PIN_TEMP1);   // read the input on analog pin 0:
  
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5 / 1023.0);
  float temp = ((voltage*1000)-500) / 10; // Celsius
  
  Serial.print(TEMP1);
  Serial.println(temp);
  return;
}

void rw_temp2() {
  //Temperature Sensor 2:
  int sensorValue = analogRead(PIN_TEMP2);   // read the input on analog pin 0:

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5 / 1023.0);
  float temp = ((voltage*1000)-500) / 10; // Celsius
  
  // print out the value you read:
  Serial.print(TEMP2);
  Serial.println(temp);
  return;
}

void rw_pulse() {
  int myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 

  if (pulseSensor.sawStartOfBeat()) {            // Constantly test to see if "a beat happened". 
   //Serial.println("----------------");
   Serial.print(PULSE);                        // Print phrase "BPM: " 
   Serial.println(myBPM);                        // Print the value inside of myBPM. 
   //Serial.println("------------------");
  }

  //delay(20);                    // considered best practice in a simple sketch.
  return;
}

void rw_temp1_debug() {
  //Temperature Sensor 1:
  int sensorValue = analogRead(PIN_TEMP1);   // read the input on analog pin 0:
  
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5 / 1023.0);
  float temp = ((voltage*1000)-500) / 10; // Celsius
  
  Serial.print("Temp 1: ");
  Serial.println(temp);
  return;
}

void rw_temp2_debug() {
  //Temperature Sensor 2:
  int sensorValue = analogRead(PIN_TEMP2);   // read the input on analog pin 0:

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5 / 1023.0);
  float temp = ((voltage*1000)-500) / 10; // Celsius
  
  // print out the value you read:
  Serial.print("Temp 2:" );
  Serial.println(temp);
  return;
}

void rw_pulse_debug() {
  int myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 

  if (pulseSensor.sawStartOfBeat()) {            // Constantly test to see if "a beat happened". 
   Serial.print("BPM: ");                        // Print phrase "BPM: " 
   Serial.println(myBPM);                        // Print the value inside of myBPM. 
  }

  //delay(20);                    // considered best practice in a simple sketch.
  return;
}
