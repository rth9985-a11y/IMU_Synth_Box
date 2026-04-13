#if 0
#include "SecondOrderLPF.h"
#include "Wire.h"
#include "math.h"

SecondOrderLPF::SecondOrderLPF()
  : AudioStream(1, inputQueueArray), q(0.707), a1(0), a2(0), b0(0), b1(0), b2(0), x_prev1(0), x_prev2(0), y_prev(0), y_prev2(0) {}

  void SecondOrderLPF::setCutoff(float Fc, float Fs){

    if (Fc >= 20000) Fc = 20000;
    if (Fc <= 20) Fc = 20;

    float omega = 2.0f * PI * Fc / Fs;
    float sn = sinf(omega);
    float cs = cosf(omega);
    float alpha = sn / (2.0f * Q);
    float a0 = 1.0f + alpha;

    b0 = ((1.0f - cs) / 2.0f) / a0;
    b1 = (1.0f - cs) / a0;
    b2 = ((1.0f - cs) / 2.0f) / a0;
    a1 = (-2.0f * cs) / a0;
    a2 = (1.0f - alpha) / a0;
  }

  void SecondOrderLPF::update(void){
    audio_block_t *block = receiveWritable();
    if (!block) return;

    for (int i = 0; i < 128; i++){
      float x = block->data[i];

      // Difference equation
      float y = (b0 * x) + (b1 * x_prev1) + (b2 * x_prev2) - (a1 * y_prev) - (a2 * y_prev2);

      // State shifts
      x_prev2 = x_prev1;
      x_prev1 = x;
      y_prev2 = y_prev;
      y_prev = y;
      x_prev = x;

      if (y > 32768.0f) y = 32768.0f;
      else if (y < -32768.0f) y = -32768.0f;

      block->data[i] = (int16_t)y;
    }
    transmit(block);
    release(block);
  }
#endif
  // 