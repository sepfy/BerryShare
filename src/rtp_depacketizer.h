#ifndef RTP_DEPACKETIZER_H_
#define RTP_DEPACKETIZER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <rtp-packet.h>
#include <rtp-payload.h>
#include "peer_connection.h"

#include "media_player.h"

class RtpDepacketizer {

 public:
  RtpDepacketizer();
  ~RtpDepacketizer();
  static void* Allocate(void* param, int bytes);
  static void Free(void* param, void *packet);
  static int DecodeVideoPacket(void* param, const void *packet, int bytes, uint32_t timestamp, int flags);
  static int DecodeAudioPacket(void* param, const void *packet, int bytes, uint32_t timestamp, int flags);
  void DepacketVideo(uint8_t *buf, size_t size);
  void DepacketAudio(uint8_t *buf, size_t size);
  void Init();
  
  inline void set_media_player(MediaPlayer *media_player) { media_player_ = media_player; }
  inline void set_peer_connection(PeerConnection *pc) { pc_ = pc; }
  inline void set_audio_rtp_ssrc(uint32_t audio_rtp_ssrc) { audio_rtp_ssrc_ = audio_rtp_ssrc; }
  inline void set_video_rtp_ssrc(uint32_t video_rtp_ssrc) { video_rtp_ssrc_ = video_rtp_ssrc; }
 private:
  struct rtp_payload_t video_handler_;
  struct rtp_payload_t audio_handler_;

  void *video_decoder_;
  void *audio_decoder_;

  uint32_t audio_rtp_ssrc_;
  uint32_t video_rtp_ssrc_;

  PeerConnection *pc_;
  MediaPlayer *media_player_;
};

#endif // RTP_PACKETIZER_H_
