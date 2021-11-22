#ifndef WEBRTC_CONNECTION_H_
#define WEBRTC_CONNECTION_H_

#include <memory>
#include <string>
#include "rtp_depacketizer.h"
#include "media_player.h"

#include "peer_connection.h"

class WebrtcConnection {

 public:
  WebrtcConnection();
  ~WebrtcConnection();
  void Init();
  void Deinit();

  std::string Request(std::string info);
  void StopCasting();
  std::string RequestCasting(std::string offer);
  std::string GetStatus();
  char* SetOffer(char *offer, void *data);
  inline std::string casting_name() { return casting_name_; }

 private:

  static void OnIceconnectionstatechange(IceConnectionState state, void *data);
  static void OnIcecandidate(char *sdp, void *data);
  static void OnTrack(uint8_t *packet, size_t bytes, void *data);

  bool is_available_;
  std::string casting_name_;
  std::string answer_;
  char *sdp_; 
  uint32_t video_rtp_ssrc_;
  uint32_t audio_rtp_ssrc_;

  GCond cond_;
  GMutex mutex_;

  MediaPlayer media_player_;
  RtpDepacketizer rtp_depacketizer_;

  PeerConnection *peer_connection_;
};

#endif // WEBRTC_CONNECTION_H_
