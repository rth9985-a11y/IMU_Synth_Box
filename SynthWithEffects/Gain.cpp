#include "Gain.h"
// #include "Audio.h"
#include "AudioStream.h"

Gain::Gain()
  : AudioStream(1, inputQueueArray), gain(0) {}

void Gain::setGain(float gainVal){
  if (gainVal <= 0){
    gain = 0;
  }
  // Just in case :)
  if (gain > 20) {
    gainVal = 20;
  }
  else{
    gain = gainVal;
  }
}

void Gain::update(void){
  audio_block_t *block = receiveWritable();

  if (!block) return;

  for (int i = 0; i < 128; i ++){
    float x = block->data[i];
    float y = x * gain;
    block->data[i] = (int16_t)y;
  }
  transmit(block);
  release(block);
}

