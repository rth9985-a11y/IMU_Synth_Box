// Include Digital Communication, Audio, and Flashing Dependencies
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Include Custom Classes
#include "FirstOrderLPF.h"
// #include "SecondOrderLPF.h"
#include "Gain.h"

// Screen defines
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -----------------------------------------------
// Class Declarations and Virtual Patching
// -----------------------------------------------
AudioSynthWaveform        waveform1;
AudioSynthWaveform        waveform2;
FirstOrderLPF             FirstOrderLPF1;
FirstOrderLPF             FirstOrderLPF2;
// SecondOrderLPF            SecondOrderLPF1;
// SecondOrderLPF            SecondOrderLPF2;
Gain                      Gain1;
AudioOutputI2S            i2s1;
AudioOutputAnalogStereo   dacs1;
AudioControlSGTL5000      sgtl5000_1;

// Patches
AudioConnection patch0(waveform1, 0, Gain1, 0);
AudioConnection patch1(Gain1, 0, FirstOrderLPF1, 0);
AudioConnection patch2(FirstOrderLPF1, 0, i2s1, 0);
AudioConnection patch3(FirstOrderLPF1, 0, i2s1, 0);
AudioConnection patch4(FirstOrderLPF1, 0, i2s1, 1);

// -----------------------------------------------
// Global Variables And Hardware Defines
// -----------------------------------------------

// Hardware Defines
#define FOLPF_POT 28
#define SOLPF_POT 27
#define GAIN_POT 26
#define PITCH1_POT 29
#define PITCH2_POT 30

// Global Variables
unsigned long prevTime = 0;
float gain1;
float fc1 = 200.0f;
float fc2 = 200.0f;

// -----------------------------------------------
// Setup And Loop
// -----------------------------------------------
void setup() {
  Serial.begin(9600);
  AudioMemory(10);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.1);


  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
  Serial.println(F("SSD1306 allocation failed"));
  for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.setCursor(1, 1);
  display.print("PITCH: ");
  display.setCursor(50, 1);
  display.print("NaN");

  display.setCursor(1, 12);
  display.print("ROLL: ");
  display.setCursor(50, 12);
  display.print("NaN");

  display.setCursor(1, 24);
  display.print("LPF FC: ");
  display.setCursor(50, 24);
  display.print("NaN");

  display.setCursor(1, 36);
  display.print("HPF FC: ");
  display.setCursor(50, 36);
  display.print("NaN");

  // Accel Gyro box
  display.drawLine(81, 4, 81, 41, WHITE);
  display.drawLine(81, 4, 118, 4, WHITE);
  display.drawLine(118, 4, 118, 41, WHITE);
  display.drawLine(118, 41, 81, 41, WHITE);

  display.drawLine(1, 48, 128, 48, WHITE);
  display.drawLine(1, 50, 128, 50, WHITE);
  display.drawLine(1, 52, 128, 52, WHITE);
  display.drawLine(1, 54, 128, 54, WHITE);
  display.drawLine(1, 56, 128, 56, WHITE);
  display.drawLine(1, 58, 128, 58, WHITE);
  display.drawLine(1, 60, 128, 60, WHITE);
  display.drawLine(1, 62, 128, 62, WHITE);
  display.drawLine(1, 64, 128, 64, WHITE);

  display.display();

  // would be cool to have three waveforms that you can slide to the left or right in pitch
  // All the way left would be like the 4 chord, middle would be 1, right would be 5? 
  // Use some kind of faux inertia to make it so that whe you jolt it all the way to the right or left
  // there is some kind of padding or "goo" that makes it feel sticky
  // Visualize on the screen
  // Forward and backward parameters can be selected, forward = higher Fc on LPF cutoff
  // backward = granulation??? could be super cool!!!
  
  waveform1.begin(WAVEFORM_SAWTOOTH);
  waveform2.begin(WAVEFORM_SAWTOOTH);
  waveform1.amplitude(0.5);
  waveform2.amplitude(0.5);
  waveform1.frequency(120);
  waveform2.frequency(120);

}

void loop() {
  fc1 = frequencyPotRead(FOLPF_POT);
  fc2 = frequencyPotRead(SOLPF_POT);
  gain1 = gainPotRead();


  FirstOrderLPF1.setCutoff(fc1, 44100);
  // SecondOrderLPF1.setCutoff(fc2, 44100);
  Gain1.setGain(gain1);

  if (millis() - prevTime >= 300) {
    
  }
}

float frequencyPotRead(int frequencyPot){
  float rawData = analogRead(frequencyPot);
  return map(rawData, 0, 1023, 20, 20000);
}

float gainPotRead(){
  float rawData = analogRead(GAIN_POT);
  return map(rawData, 0, 1023, 0, 3);
}

// FSCALE !!! for log 


