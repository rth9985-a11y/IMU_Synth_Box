#include "SecondOrderLPF.h"
#include "Wire.h"
#include "math.h"

SecondOrderLPF::SecondOrderLPF()
  : AudioStream(1, inputQueueArray), q(1.41421356237), a1(0), a2(0), b0(0), b1(0), b2(0), x_prev(0), x_prev1(0), x_prev2(0), y_prev(0), y_prev2(0) {}

  void SecondOrderLPF::setCutoff(float Fc, float Fs){
    // q = sqrt(2)
    float ff = Fc / Fs;
    float n = 1.0f/tanf(PI * ff);
    b0 = 1 / (1 + (q * n) + (n * n));
    b1 = 2 * b0;
    b2 = b0;
    a1 = (2 * b0) * ((n * n) - 1.0f);
    a2 = (-b0) * (1.0f - (q * n) + (n * n));
  }

  void SecondOrderLPF::update(void){
    audio_block_t *block = receiveWritable();
    if (!block) return;

    for (int i = 0; i < 128; i++){
      float x = block->data[i];
      // Difference equation
      float y = (b0 * x) + (b1 * x_prev) + (b2 * x_prev2) - (a1 * y_prev) - (a2 * y_prev2);
      // Coefficients
      x_prev2 = x_prev1;
      x_prev1 = x_prev;
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

  // 