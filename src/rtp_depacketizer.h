#ifndef RTP_DEPACKETIZER_H_
#define RTP_DEPACKETIZER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <rtp-packet.h>
#include <rtp-payload.h>

#include "omx_player.h"

class RtpDepacketizer {

 public:
  RtpDepacketizer();
  ~RtpDepacketizer();
  static void* Allocate(void* param, int bytes);
  static void Free(void* param, void *packet);
  static int DecodePacket(void* param, const void *packet, int bytes, uint32_t timestamp, int flags);
  void Depacket(uint8_t *buf, size_t size);
  inline void set_omx_player(OmxPlayer *omx_player) { omx_player_ = omx_player; }
  void Init();
  
 private:
  struct rtp_payload_t handler_;
  void *decoder_;
  OmxPlayer *omx_player_;
};

#endif // RTP_PACKETIZER_H_
