#if 0
// #pragma once
// #ifndef SecondOrderLPF_h
// #define SecondOrderLPF_h
// #include <Audio.h>

class SecondOrderLPF : public AudioStream{
  public:

    SecondOrderLPF();

    void setCutoff(float Fc, float Fs);

    void update(void);

  private:
    audio_block_t *inputQueueArray[1];
    float q;
    float a1, a2, b0, b1, b2;
    float x_prev1;
    float x_prev2;
    float y_prev; 
    float y_prev2;
};

#endif
