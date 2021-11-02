#ifndef MEDIA_SINK_H_
#define MEDIA_SINK_H_

#include <stdint.h>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include "ring_buffer.h"

class MediaSink {

 public:

  MediaSink(size_t buffer_size);
  ~MediaSink();

  virtual int Play() = 0;
  virtual void Pause() = 0;

  int ReadMediaBuffer(uint8_t *data, size_t size);
  int WriteMediaBuffer(uint8_t *data, size_t size);
  bool BufferIsEmpty();
  void CleanBuffer();

 private:

  RingBuffer buffer_; 
  bool playback_;

  std::thread thread_;
  std::mutex mutex_;

};

#endif // MEDIA_SINK_H_
