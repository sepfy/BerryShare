#ifndef OMX_PLAYER_H_
#define OMX_PLAYER_H_

#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

#include "ring_buffer.h"

#define OMX_SKIP64BIT

extern "C" {
#include "ilclient.h"
}

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
  void Restart();
  int Play();
  void Stop();
 private:
  RingBuffer video_buffer_; 
  pthread_mutex_t mutex_;
  pthread_t tid_;
  bool playback_;

  // OMX stack
  COMPONENT_T *video_decode_;
  COMPONENT_T *video_scheduler_;
  COMPONENT_T *video_render_;
  COMPONENT_T *clock_;
  COMPONENT_T *components_[5];
  TUNNEL_T tunnels_[4];
  ILCLIENT_T *ilclient_;

};

#endif // OMX_PLAYER_H_
