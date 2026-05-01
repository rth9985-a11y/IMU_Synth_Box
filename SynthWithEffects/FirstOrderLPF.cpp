#include "FirstOrderLPF.h"
// #include "Audio.h"
#include "Wire.h"

FirstOrderLPF::FirstOrderLPF()
  : AudioStream(1, inputQueueArray), b0(0), b1(0), a1(0), x_prev(0), y_prev(0) {}

void FirstOrderLPF::setCutoff(float fc, float fs) {
  // Keep within the audible frequency range
  if (fc <= 20.0f)    fc = 20.0f;
  if (fc >= 20000.0f) fc = 20000.0f;

  // Set coefficients
  float K = tanf((float)M_PI * fc / fs);
  float b0_new = K / (1.0f + K);
  float a1_new = (K - 1.0f) / (1.0f + K);

  // Stop all interuupt requests until these are set
  __disable_irq();
  b0 = b0_new;
  b1 = b0_new;
  a1 = a1_new;
  __enable_irq();
}

void FirstOrderLPF::update(void) {
  // Get audio data
  audio_block_t *block = receiveWritable();

  // Check if recived a block
  if (!block) return;

  // Loop over audio data block
  for (int i = 0; i < 128; i++) {
    // input sample
    float x = (float) block->data[i];
    // Difference equation
    float y = (b0 * x) + (b1 * x_prev) - (a1 * y_prev);
    // Set feedback states
    y_prev = y;
    x_prev = x;
    // Limit the data to to the maximum/minimum representable value
    y = fmaxf(-32768.0f, fminf(32767.0f, y));
    // return the data back as a 16 bit int (teensy audio shield uses 16 bit audio)
    block->data[i] = (int16_t)y;
  }
  // Send the processed audio block back 
  transmit(block);
  // Forget about the processed block so we can move on to the next
  release(block);
}

