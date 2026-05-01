#pragma once
#ifndef FirstOrderLPF_h
#define FirstOrderLPF_h
#include <Audio.h>


class FirstOrderLPF : public AudioStream {
  public:
    FirstOrderLPF();

    void setCutoff(float fc, float fs);

    void update(void);

  private:
    // Pointer to the audio array
    audio_block_t *inputQueueArray[1];
    float b0, b1, a1;
    float x_prev;
    float y_prev;
};

#endif