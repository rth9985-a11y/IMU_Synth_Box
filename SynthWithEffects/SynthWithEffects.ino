// Include Digital Communication, Audio, and Flashing Dependencies
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ISM330DHCX.h>
#include "FirstOrderLPF.h"

// IMU Defines + Constructor
#define LSM_CS 10
Adafruit_ISM330DHCX ism330dhcx;

// Screen Defines + Constructor
#define SCREEN_WIDTH      128
#define SCREEN_HEIGHT     64
#define OLED_RESET        -1
#define SCREEN_ADDRESS    0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define MASTER_VOLUME 27
#define GRANULAR_MEMORY_SIZE 25600

// -----------------------------------------------
// Class Declarations + Virtual Patching
// -----------------------------------------------
AudioSynthWaveform        waveform1;
AudioSynthWaveform        waveform2;
AudioSynthWaveform        waveform3;
AudioSynthWaveform        waveform4;
AudioSynthWaveform        lfo;
AudioEffectMultiply       vca;
FirstOrderLPF             FirstOrderLPF1;
AudioMixer4               Mixer1;
AudioOutputI2S            i2s1;
AudioOutputAnalogStereo   dacs1;
AudioControlSGTL5000      sgtl5000_1;

// Patches
AudioConnection           patchCord0(lfo, 0, vca, 1);
AudioConnection           patchCord1(waveform1, 0, Mixer1, 0);
AudioConnection           patchCord2(waveform2, 0, Mixer1, 1);
AudioConnection           patchCord3(waveform3, 0, Mixer1, 2);
AudioConnection           patchCord4(waveform4, 0, Mixer1, 3);
AudioConnection           patchCord6(Mixer1, 0, vca, 0);
AudioConnection           patchCord7(vca, 0, FirstOrderLPF1, 0);
AudioConnection           patchCord8(FirstOrderLPF1, 0, i2s1, 0);
AudioConnection           patchCord9(FirstOrderLPF1, 0, i2s1, 1);

// -----------------------------------------------
// Float map — Arduino's map() is integer only
// -----------------------------------------------
float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float mapIMU(float raw, float out_min, float out_max) {
  if (raw >= 10.0f)  raw = 10.0f;
  if (raw <= -10.0f) raw = -10.0f;
  return fmap(raw, -10.0f, 10.0f, out_min, out_max);
}


// -----------------------------------------------
// Setup And Loop
// -----------------------------------------------
void setup() {
  Serial.begin(115200);
  AudioMemory(10);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.1);

  // IMU setup from library
  if (!ism330dhcx.begin_I2C()) {
    // if (!ism330dhcx.begin_SPI(LSM_CS)) {
    // if (!ism330dhcx.begin_SPI(LSM_CS, LSM_SCK, LSM_MISO, LSM_MOSI)) {
    Serial.println("Failed to find ISM330DHCX chip");
    for(;;);
  }
  ism330dhcx.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  ism330dhcx.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
  ism330dhcx.setAccelDataRate(LSM6DS_RATE_12_5_HZ);
  ism330dhcx.setGyroDataRate(LSM6DS_RATE_12_5_HZ);
  ism330dhcx.configInt1(false, false, true); // accelerometer DRDY on INT1
  ism330dhcx.configInt2(false, true, false); // gyro DRDY on INT2

  // Check if the screen is found
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Prep display settings
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  // Row 1
  display.setCursor(1, 1);
  display.print("PITCH: ");
  // Row 2
  display.setCursor(1, 12);
  display.print("ROLL: ");
  // Row 3
  display.setCursor(1, 24);
  display.print("LPF FC: ");
  // Row 4
  display.setCursor(1, 36);
  display.print("VOL: ");

  // Position box
  // Left
  display.drawLine(81, 4, 81, 41, WHITE);
  //Top
  display.drawLine(81, 4, 118, 4, WHITE);
  // Right
  display.drawLine(118, 4, 118, 41, WHITE);
  // Bottom
  display.drawLine(118, 41, 81, 41, WHITE);

  // Bottom lines
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
  
  waveform1.begin(WAVEFORM_SAWTOOTH);
  waveform2.begin(WAVEFORM_SAWTOOTH);
  waveform3.begin(WAVEFORM_SAWTOOTH);
  waveform4.begin(WAVEFORM_SAWTOOTH);
  lfo.begin(WAVEFORM_SINE);
  waveform1.amplitude(0.22);
  waveform2.amplitude(0.22);
  waveform3.amplitude(0.22);
  waveform4.amplitude(0.33);
  lfo.amplitude(0.1);
  waveform1.frequency(196.0);
  waveform2.frequency(220.00);
  waveform3.frequency(246.94);
  waveform4.frequency(98.0);
  lfo.frequency(5.0);
  
}

// Loop variables 
float accelXRaw = 0;
float accelYRaw = 0;

int fcLPF = 0;
int fcHPF = 0;

int accelXPrint = 0;
int accelYPrint = 0;
int accelXVisual = 0;
int accelYVisual = 0;

// float gyroX = 0;
// float gyroY = 0;
// float gyroZ = 0;

unsigned long prevTime = 0;
unsigned long prevTime1 = 0;
unsigned long prevTime2 = 0;

float masterVol = 0;

// Smoothing values for pitch control
float fc1Target = 1000.0f;
float fc1 = 1000.0f;
float FC_SMOOTH = 0.085f;

float freq1 = 196.0f;
float freq2 = 246.94f;
float freq3 = 293.99f;
float freq4 = 97.999f;

float lfoFreq = 0.0f;
float lfoDepth = 0.0f;

unsigned long tremTime = 0;

struct pitchMaps{
  const float osc1Left = 174.61;
  const float osc2Left = 196.00;
  const float osc3Left = 220.00;
  const float osc4Left = 43.654;

  const float osc1right = 196.0;
  const float osc2right = 220.0;
  const float osc3right = 261.63;
  const float osc4right = 43.654;

  const float osc1Center = 196.0;
  const float osc2Center = 220.0;
  const float osc3Center = 246.94;
  const float osc4Center = 48.999;
};

struct pitchMaps pitch;

void loop() {
  unsigned long currentTime = millis();
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  ism330dhcx.getEvent(&accel, &gyro, &temp);

  accelXRaw = accel.acceleration.x;
  accelYRaw = accel.acceleration.y;

  accelXPrint = (int) mapIMU(accelXRaw, -100, 100);
  accelYPrint = (int) mapIMU(accelYRaw, -100, 100);
  accelXPrint = -accelXPrint; // Reverse sign to make it work with screen orientation

  // Control update
  if (currentTime - prevTime1 >= 40){
    if (accelXPrint <= 0) {
      // maybe try just an octave spread of the 1 chord?
      freq1 = fmap(accelXPrint, -100.0, 0.0, pitch.osc1Left, pitch.osc1Center);
      freq2 = fmap(accelXPrint, -100.0, 0.0, pitch.osc2Left, pitch.osc2Center);
      freq3 = fmap(accelXPrint, -100.0, 0.0, pitch.osc3Left, pitch.osc3Center);
      freq4 = fmap(accelXPrint, -100.0, 0.0, pitch.osc4Left, pitch.osc4Center);
      waveform1.frequency(freq1);
      waveform2.frequency(freq2);
      waveform3.frequency(freq3);
      waveform4.frequency(freq4);
    }
    if (accelXPrint > 0){
      freq3 = fmap(accelXPrint, 0, 100, pitch.osc3Center, pitch.osc3right);
      freq4 = fmap(accelXPrint, 0, 100, pitch.osc4Center, pitch.osc4right);
      waveform3.frequency(freq3);
      waveform4.frequency(freq4);
    }

    masterVol = map((float) analogRead(MASTER_VOLUME), 0.0f, 1023.0f, 0.0f, 1.0f);
    sgtl5000_1.volume(masterVol);

    fc1Target = fmap(accelYRaw, -10.0f, 10.0f, 150.0f, 10000.0f);
    fc1 += FC_SMOOTH * (fc1Target - fc1);
    FirstOrderLPF1.setCutoff(fc1, 44100.0f);

    if (accelXPrint < 0){
      tremTime = fmap(accelXPrint, 0, 100, 2000, 250);
    }

    if (accelYPrint > 20){
      lfoFreq = fmap(accelYPrint, 0.0, 100.0, 0.0, 6.0);
      // lfoDepth = fmap(accelYPrint, 0.0, 100.0, 0.5, 0.1);
      
    }
    else {
      lfoFreq = 0;
      lfoDepth = 1;
    }
    lfo.frequency(lfoFreq);
    lfo.amplitude(lfoDepth);

    prevTime1 = currentTime;
  }

  // OLED UPDATE
  if (currentTime - prevTime >= 50) {
    updateDisplay();
    prevTime = currentTime;
  }
}

void updatePositionBox(int pitchVal, int rollVal){
  display.fillRect(82, 5, 35, 35, BLACK);
  int mappedPitch = map(accelYPrint, -100, 100, 5, 40);
  int mappedRoll = map(accelXPrint, -100, 100, 82, 117);
  display.drawPixel(mappedRoll, mappedPitch, WHITE);
}

void updateDisplay(){
    display.fillRect(50, 1, 30, 43, BLACK);
    // Pitch (positional, not musical)
    display.setCursor(50, 1);
    display.print(accelYPrint);
    // Roll
    display.setCursor(50, 12);
    display.print(accelXPrint);
    // LPF Fc
    display.setCursor(50, 24);
    display.print((int) fc1);
    // HPF Fc
    display.setCursor(50, 36);
    display.print(masterVol);

    updatePositionBox(accelYVisual, accelXVisual);
    display.display();
}


