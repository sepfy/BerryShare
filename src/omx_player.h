#ifndef OMX_PLAYER_H_
#define OMX_PLAYER_H_

#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

#include "ring_buffer.h"

class OmxPlayer {

 public:
  OmxPlayer();
  ~OmxPlayer();
  int Init();
  void Deinit();
  int ReadVideoBuffer(uint8_t *data, size_t size);
  int WriteVideoBuffer(uint8_t *data, size_t size);
  bool VideoBufferIsEmpty();
  static void* Playback(void *data);

 private:
  RingBuffer video_buffer_; 
  pthread_mutex_t mutex_;
  pthread_t tid_;
  bool playback_;
};

#endif // OMX_PLAYER_H_
