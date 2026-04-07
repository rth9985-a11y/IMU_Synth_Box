#include "FirstOrderLPF.h"
// #include "Audio.h"
#include "Wire.h"

FirstOrderLPF::FirstOrderLPF()
  : AudioStream(1, inputQueueArray), b0(0), b1(0), a1(0), x_prev(0), y_prev(0) {}

void FirstOrderLPF::setCutoff(float fc, float fs) {
  if (fc <= 20.0f) fc = 20.0f;
  if (fc >= 20000.0f) fc = 20000.0f;

  float pi = 3.1459265f;
  float K = tanf((pi * fc) / fs);
  b0 = K / (1 + K);
  b1 = K / (1 + K);
  a1 = (K - 1.0f) / (1.0f + K);  //GIT TEST
}

void FirstOrderLPF::update(void) {
  audio_block_t *block = receiveWritable();

  if (!block) return;

  for (int i = 0; i < 128; i++) {
    float x = block->data[i];
    float y = (b0 * x) + (b1 * x_prev) - (a1 * y_prev);
    y_prev = y;
    x_prev = x;
    block->data[i] = (int16_t)y;
  }
  transmit(block);
  release(block);
}

// private:
//   audio_block_t *inputQueueArray[1];
//   float b0, b1, a1;
//   float x_prev;
//   float y_prev;
// };
