#include <stdint.h>
#include <spdlog/spdlog.h>

#include "rtp_depacketizer.h"
#include "omx_player.h"

RtpDepacketizer::RtpDepacketizer() {

  handler_.alloc = RtpDepacketizer::Allocate;
  handler_.free = RtpDepacketizer::Free;
  handler_.packet = RtpDepacketizer::DecodePacket;
  decoder_ = rtp_payload_decode_create(102, "H264", &handler_, this);

}

RtpDepacketizer::~RtpDepacketizer() {

}

void RtpDepacketizer::Depacket(uint8_t *buf, size_t size) {
  rtp_payload_decode_input(decoder_, buf, size);
}

void* RtpDepacketizer::Allocate(void* param, int bytes) {

  static uint8_t buffer[2 * 1024 * 1024 + 4] = { 0, 0, 0, 1, };
  if((sizeof(buffer) - 4) <= bytes) {
    printf("rtp package to large\n");
    exit(1);
  }
  return buffer + 4;
}

void RtpDepacketizer::Free(void* param, void *packet) {
}

int RtpDepacketizer::DecodePacket(void* param, const void *packet, int bytes, uint32_t timestamp, int flags) {

  RtpDepacketizer *rtp_depacketizer = (RtpDepacketizer*)param;
  uint8_t *data = (uint8_t*)packet;
  static uint8_t nalu_header[] = {0x00, 0x00, 0x00, 0x01};
  rtp_depacketizer->omx_player_->WriteVideoBuffer(nalu_header, sizeof(nalu_header));
  rtp_depacketizer->omx_player_->WriteVideoBuffer(data, bytes);
  return 0;
}



