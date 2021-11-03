#include <stdint.h>
#include <spdlog/spdlog.h>

#include "rtp_depacketizer.h"
#include "media_player.h"

RtpDepacketizer::RtpDepacketizer() {

  video_handler_.alloc = RtpDepacketizer::Allocate;
  video_handler_.free = RtpDepacketizer::Free;
  video_handler_.packet = RtpDepacketizer::DecodeVideoPacket;
  video_decoder_ = rtp_payload_decode_create(102, "H264", &video_handler_, this);


  audio_handler_.alloc = RtpDepacketizer::Allocate;
  audio_handler_.free = RtpDepacketizer::Free;
  audio_handler_.packet = RtpDepacketizer::DecodeAudioPacket;
  audio_decoder_ = rtp_payload_decode_create(8, "PCMA", &audio_handler_, this);

  pc_ = NULL;
}

RtpDepacketizer::~RtpDepacketizer() {

}

void RtpDepacketizer::DepacketAudio(uint8_t *buf, size_t size) {
  rtp_payload_decode_input(audio_decoder_, buf, size);
}


void RtpDepacketizer::DepacketVideo(uint8_t *buf, size_t size) {
  rtp_payload_decode_input(video_decoder_, buf, size);
}

void* RtpDepacketizer::Allocate(void* param, int bytes) {

  uint8_t *buffer = (uint8_t*)malloc(bytes + 4);
  if(buffer == NULL) {
    SPDLOG_ERROR("Allocate RTP buffer failed");
  } 
  return buffer + 4;
}

void RtpDepacketizer::Free(void* param, void *packet) {

  uint8_t *buffer = (uint8_t*)packet;
  if(buffer) {
    free(buffer);
    buffer = NULL;
  }
}

int RtpDepacketizer::DecodeAudioPacket(void* param, const void *packet, int bytes, uint32_t timestamp, int flags) {

  RtpDepacketizer *rtp_depacketizer = (RtpDepacketizer*)param;
  uint8_t *data = (uint8_t*)packet;

  rtp_depacketizer->media_player_->WriteMediaBuffer(data, bytes, MediaType::kMediaAudio);
  return 0;
}


int RtpDepacketizer::DecodeVideoPacket(void* param, const void *packet, int bytes, uint32_t timestamp, int flags) {

  RtpDepacketizer *rtp_depacketizer = (RtpDepacketizer*)param;
  PeerConnection *pc = rtp_depacketizer->pc_;
  uint8_t *data = (uint8_t*)packet;

  static int gop = 0;
  static uint8_t nalu_header[] = {0x00, 0x00, 0x00, 0x01};
  static uint8_t header[32] = {0};

  switch(data[0]) {
    case 0x67:
      if(memcmp(header, packet, bytes) != 0) {
        SPDLOG_INFO("SPS/PPS changed. Restart player");

        memset(header, 0, sizeof(header));
        memcpy(header, packet, bytes);
        rtp_depacketizer->media_player_->Pause(MediaType::kMediaVideo);
        rtp_depacketizer->media_player_->Play(MediaType::kMediaVideo);
      }
      break;
    case 0x68:
      break;
    default:
      gop++;
      if(gop%60 == 0) {
        peer_connection_send_rtcp_pil(pc, rtp_depacketizer->video_rtp_ssrc_);
	gop = 0;
      }
      break;
  }
#if 0
      for(int i = 0; i < 8; i++) {
        printf("%.2X ", ((uint8_t*)(packet))[i]);
      }
      printf("\n");
#endif
  rtp_depacketizer->media_player_->WriteMediaBuffer(nalu_header, sizeof(nalu_header), MediaType::kMediaVideo);
  rtp_depacketizer->media_player_->WriteMediaBuffer(data, bytes, MediaType::kMediaVideo);

  return 0;
}



