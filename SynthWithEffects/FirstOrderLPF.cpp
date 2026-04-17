#include "FirstOrderLPF.h"
// #include "Audio.h"
#include "Wire.h"

FirstOrderLPF::FirstOrderLPF()
  : AudioStream(1, inputQueueArray), b0(0), b1(0), a1(0), x_prev(0), y_prev(0) {}

void FirstOrderLPF::setCutoff(float fc, float fs) {
  if (fc <= 20.0f)    fc = 20.0f;
  if (fc >= 20000.0f) fc = 20000.0f;

  float K = tanf((float)M_PI * fc / fs);
  float b0_new = K / (1.0f + K);
  float a1_new = (K - 1.0f) / (1.0f + K);

  __disable_irq();
  b0 = b0_new;
  b1 = b0_new;
  a1 = a1_new;
  __enable_irq();
}

void FirstOrderLPF::update(void) {
  audio_block_t *block = receiveWritable();

  if (!block) return;

  for (int i = 0; i < 128; i++) {
    float x = (float) block->data[i];
    float y = (b0 * x) + (b1 * x_prev) - (a1 * y_prev);
    y_prev = y;
    x_prev = x;
    y = fmaxf(-32768.0f, fminf(32767.0f, y));
    block->data[i] = (int16_t)y;
  }
  transmit(block);
  release(block);
}

