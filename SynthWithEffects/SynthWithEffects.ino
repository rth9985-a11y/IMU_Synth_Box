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

// -----------------------------------------------
// Class Declarations + Virtual Patching
// -----------------------------------------------
AudioSynthWaveform        waveform1;
AudioSynthWaveform        waveform2;
AudioSynthWaveform        waveform3;
AudioSynthWaveform        waveform4;
FirstOrderLPF             FirstOrderLPF1;
AudioMixer4               Mixer1;
AudioOutputI2S            i2s1;
AudioOutputAnalogStereo   dacs1;
AudioControlSGTL5000      sgtl5000_1;

// Patches
AudioConnection           patchCord1(waveform1, 0, Mixer1, 0);
AudioConnection           patchCord2(waveform2, 0, Mixer1, 1);
AudioConnection           patchCord3(waveform3, 0, Mixer1, 2);
AudioConnection           patchCord4(waveform4, 0, Mixer1, 3);
AudioConnection           patchCord5(Mixer1, 0, FirstOrderLPF1, 0);
AudioConnection           patchCord6(FirstOrderLPF1, 0, i2s1, 0);
AudioConnection           patchCord7(FirstOrderLPF1, 0, i2s1, 1);

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
  waveform1.amplitude(0.25);
  waveform2.amplitude(0.25);
  waveform3.amplitude(0.25);
  waveform4.amplitude(0.25);
  waveform1.frequency(196.0);
  waveform2.frequency(220.00);
  waveform3.frequency(246.94);
  waveform4.frequency(98.0);
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

float masterVol = 0;

// Smoothing values for filter control
float fc1Target = 1000.0f;
float fc1 = 1000.0f;
const float FC_SMOOTH = 0.085f;

float freq1 = 196.0f;
float freq2 = 246.94f;
float freq3 = 293.99f;
float freq4 = 97.999f;

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
  // waveform1.frequency(196.0);
  // waveform2.frequency(220.00);
  // waveform3.frequency(246.94);
  if (currentTime - prevTime1 >= 40){
    if (accelXPrint <= 0) {
      freq1 = fmap(accelXPrint, -100.0, 0.0, 185.0, 196.0);
      freq2 = fmap(accelXPrint, -100.0, 0.0, 220.0, 220.0);
      waveform1.frequency(freq1);
    }
    // 246.94 --> 261.63
    // 293.99 --> 311.13
    if (accelXPrint > 0){
      freq1 = fmap(accelXPrint, 0, 100, 196.0, 261.63);
      waveform1.frequency(freq1);
      freq2 = fmap(accelXPrint, 0, 100, 220.00, 293.67);
      waveform2.frequency(freq2);
      freq3 = fmap(accelXPrint, 0, 100, 246.94, 329.63);
      waveform3.frequency(freq3);
    }
    masterVol = map((float) analogRead(MASTER_VOLUME), 0.0f, 1023.0f, 0.0f, 1.0f);
    
    sgtl5000_1.volume(masterVol);

    fc1Target = fmap(accelYRaw, -10.0f, 10.0f, 150.0f, 10000.0f);
    fc1 += FC_SMOOTH * (fc1Target - fc1);
    FirstOrderLPF1.setCutoff(fc1, 44100.0f);
  
    prevTime1 = currentTime; // worked fine without this??? What???
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


