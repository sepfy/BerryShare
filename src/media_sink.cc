#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <spdlog/spdlog.h>

#include "media_sink.h"

MediaSink::MediaSink(size_t buffer_size) {

  buffer_.Create(buffer_size);
}

MediaSink::~MediaSink() {

  buffer_.Destroy();
}

void MediaSink::CleanBuffer() {

  buffer_.Reset();
}

bool MediaSink::BufferIsEmpty() {

  return buffer_.IsEmpty();
}

int MediaSink::ReadMediaBuffer(uint8_t *data, size_t size) {

  int ret = -1;
  mutex_.lock();
  ret = buffer_.Read(data, size);
  mutex_.unlock();
  return ret;
}

int MediaSink::WriteMediaBuffer(uint8_t *data, size_t size) {

  int ret = -1;
  mutex_.lock();
  ret = buffer_.Write(data, size);
  mutex_.unlock();
  return ret;
}

