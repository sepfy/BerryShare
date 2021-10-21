#include <stdlib.h>
#include <string.h>

#include "ring_buffer.h"

#define min(x, y) ((x) < (y) ? (x) : (y))
#define ROUND_UP_2(num) (((num)+1)&~1)
#define DEFAULT_BUF_SIZE (2*1024*1024)

RingBuffer::RingBuffer() {

}

RingBuffer::~RingBuffer() {

}

int RingBuffer::Create(int length) {

  int size = ROUND_UP_2(length);

  if((size&(size-1)) || (size < DEFAULT_BUF_SIZE)) {
    size = DEFAULT_BUF_SIZE;
  }

  size_ = size;
  in_ = 0;
  out_ = 0;
  buf_ = (unsigned char *)malloc(size);
  if(!buf_)
    return -1;

  memset(buf_, 0, size);

  return 0;
}

void RingBuffer::Destroy() {
  if(buf_);
    free(buf_);
}

int RingBuffer::Reset() {

  in_ = 0;
  out_ = 0;
  memset(buf_, 0, size_);

  return 0;
}

bool RingBuffer::IsEmpty() {

  return in_ == out_;
}

int RingBuffer::Write(uint8_t *data, int length) {

  int len = 0;
  length = min(length, size_ - in_ + out_);
  len = min(length, size_ - (in_ & (size_ - 1)));

  memcpy(buf_ + (in_ & (size_ - 1)), data, len);
  memcpy(buf_, data + len, length - len);

  in_ += length;

  return length;
}

int RingBuffer::Read(uint8_t *target, int amount) {

  int len = 0;

  amount = min(amount, in_ - out_);
  len = min(amount, size_ - (out_ & (size_ - 1)));

  memcpy(target, buf_ + (out_ & (size_ - 1)), len);
  memcpy(target + len, buf_, amount - len);

  out_ += amount;

  return amount;
}

