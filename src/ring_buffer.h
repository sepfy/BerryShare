#include <stdio.h>
#include <stdint.h>

class RingBuffer {

 public:
  RingBuffer();
  ~RingBuffer();
  int Create(int size);
  void Destroy();
  int Write(uint8_t *data, int length);
  int Read(uint8_t *dest, int amount);
  bool IsEmpty();
  int Reset();

 private:
  uint8_t *buf_;
  int size_;
  int in_;
  int out_;

};

