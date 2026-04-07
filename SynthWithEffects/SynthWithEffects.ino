// Include Digital Communication, Audio, and Flashing Dependencies
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// Include Custom Classes
#include "FirstOrderLPF.h"
#include "Gain.h"

// -----------------------------------------------
// Class Declarations and Virtual Patching
// -----------------------------------------------
AudioSynthWaveform        waveform1;
FirstOrderLPF             FirstOrderLPF1;
Gain                      Gain1;
AudioOutputI2S            i2s1;
AudioOutputAnalogStereo   dacs1;
AudioControlSGTL5000      sgtl5000_1;

// Patches
AudioConnection patch0(waveform1, 0, Gain1, 0);
AudioConnection patch1(Gain1, 0, FirstOrderLPF1, 0);
AudioConnection patch2(FirstOrderLPF1, 0, i2s1, 0);
AudioConnection patch3(FirstOrderLPF1, 0, i2s1, 1);

// -----------------------------------------------
// Global Variables And Hardware Defines
// -----------------------------------------------

// Hardware Defines
#define FREQUENCY_POT 26
#define GAIN_POT 25

// Global Variables
unsigned long prevTime = 0;
float gain1;
float frequency1 = 200;
float smoothed;

// -----------------------------------------------
// Setup And Loop
// -----------------------------------------------
void setup() {
  Serial.begin(9600);
  AudioMemory(10);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.1);

  waveform1.begin(WAVEFORM_SAWTOOTH);
  waveform1.amplitude(0.5);
  waveform1.frequency(120);

}


void loop() {
  frequency1 = frequencyPotRead();
  gain1 = gainPotRead();

  FirstOrderLPF1.setCutoff(frequency1, 44100);
  Gain1.setGain(gain1);

  if (millis() - prevTime >= 300) {
    Serial.print("Cutoff Frequency: ");
    Serial.print(frequency1);
    Serial.print("   |   Gain: ");
    Serial.println(gain1);
    prevTime = millis();
  }
 
}

float frequencyPotRead(){
  float rawData = analogRead(FREQUENCY_POT);
  return map(rawData, 0, 1023, 20, 20000);
}

float gainPotRead(){
  float rawData = analogRead(GAIN_POT);
  return map(rawData, 0, 1023, 0, 3);
}





