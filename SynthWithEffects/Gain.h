#pragma once

#ifndef Gain_h
#define Gain_h
#include <Audio.h>

class Gain : public AudioStream {
  public:
    Gain();

    void setGain(float gainVal);
    
    void update(void);

  private:
    audio_block_t *inputQueueArray[1];
    float gain;
};
#endif