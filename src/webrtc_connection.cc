#include <iostream>
#include <chrono>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "media_player.h"
#include "webrtc_connection.h"

using json = nlohmann::json;

WebrtcConnection::WebrtcConnection() {
  is_available_ = true;
  sdp_ = NULL;
  video_rtp_ssrc_ = 0;
  audio_rtp_ssrc_ = 0;
}

WebrtcConnection::~WebrtcConnection() {

}

std::string WebrtcConnection::Request(std::string info) {

  auto j = json::parse(info);
  if(j["type"] == "offer" && is_available_ == true) {
    auto sdp = RequestCasting(j["sdp"]);
    media_player_.Play();
    json response;
    response["type"] = "answer";
    response["sdp"] = sdp;
    is_available_ = false;
    casting_name_ = j["name"];
    return response.dump();
  }

  return "";
}

void WebrtcConnection::StopCasting() {

  //peer_connection_destroy(peer_connection_);
  media_player_.Pause();
  is_available_ = true;
}

std::string WebrtcConnection::RequestCasting(std::string offer) {
 	
  char *answer = SetOffer((char*)offer.c_str(), NULL);
  return std::string(answer);
}

std::string WebrtcConnection::GetStatus() {

  json j;
  j["type"] = "status";
  j["isAvailable"] = is_available_;
  j["castingName"] = casting_name_;
  return j.dump();
}

void WebrtcConnection::Init() {

  spdlog::set_pattern("[%H:%M:%S][%@][%l] %v");
  rtp_depacketizer_.set_media_player(&media_player_);
}


void WebrtcConnection::OnIceconnectionstatechange(IceConnectionState state, void *data) {
  WebrtcConnection *webrtc_connection = (WebrtcConnection*)data;
  if(state == FAILED) {
    printf("Disconnect with browser... Stop streaming\n");
  }
}

void WebrtcConnection::OnIcecandidate(char *sdp, void *data) {

  WebrtcConnection *webrtc_connection = (WebrtcConnection*)data;

  if(webrtc_connection->sdp_)
    g_free(webrtc_connection->sdp_);

  webrtc_connection->sdp_ = g_base64_encode((const guchar*)sdp, strlen(sdp));
  g_cond_signal(&webrtc_connection->cond_);

}

void WebrtcConnection::OnTrack(uint8_t *packet, size_t bytes, void *data) {

  WebrtcConnection *webrtc_connection = (WebrtcConnection*)data;

  RtpDepacketizer *rtp_depacketizer = &webrtc_connection->rtp_depacketizer_;

  uint32_t ssrc = 0;
  ssrc = (((uint32_t)packet[8]) << 24) | (((uint32_t)packet[9]) << 16) | (((uint32_t)packet[10]) << 8) | ((uint32_t)packet[11]);

  static uint64_t audio_bitrate = 0;
  static uint64_t video_bitrate = 0;
  static auto audio_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  static auto video_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  auto current_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

  if(ssrc == webrtc_connection->audio_rtp_ssrc_) {

    rtp_depacketizer->DepacketAudio(packet, bytes);

    audio_bitrate += bytes*8;
    if(audio_bitrate > 1000000UL) {
      SPDLOG_INFO("Audio({}) bitrate: {} bps.", webrtc_connection->audio_rtp_ssrc_,
       (double)(audio_bitrate)/(double)(current_ms - audio_ms));
      audio_bitrate = 0;
      audio_ms = current_ms;
    }
  }
  else if(ssrc == webrtc_connection->video_rtp_ssrc_) {

    uint32_t rtcp_ssrc;
    memcpy(&rtcp_ssrc, packet + 8, 4);
    rtp_depacketizer->set_video_rtp_ssrc(rtcp_ssrc);
    rtp_depacketizer->DepacketVideo(packet, bytes);

    video_bitrate += bytes*8;
    if(video_bitrate > 10000000UL) {
      SPDLOG_INFO("Video({}) bitrate: {} bps", webrtc_connection->video_rtp_ssrc_,
       (double)(video_bitrate)/(double)(current_ms - video_ms));
      video_bitrate = 0;
      video_ms = current_ms;
    }

  }
}

char* WebrtcConnection::SetOffer(char *offer, void *data) {

  g_mutex_lock(&mutex_);

  guchar *decoded_sdp = NULL;
  gsize decoded_sdp_len = strlen(offer);
  decoded_sdp = g_base64_decode((const gchar*)offer, &decoded_sdp_len);

  video_rtp_ssrc_ = session_description_find_ssrc("video", (char*)decoded_sdp);
  audio_rtp_ssrc_ = session_description_find_ssrc("audio", (char*)decoded_sdp);

  if(decoded_sdp)
    g_free(decoded_sdp);

  SPDLOG_INFO("Get offer. RTP video ssrc = {}, audio ssrc = {}", video_rtp_ssrc_, audio_rtp_ssrc_);

  if(peer_connection_)
    peer_connection_destroy(peer_connection_);

  peer_connection_ = peer_connection_create();

  MediaStream *media_stream = media_stream_new();
  media_stream_add_track(media_stream, CODEC_H264);
  Transceiver transceiver = {.video = RECVONLY};

  if(audio_rtp_ssrc_ > 0) {
    media_stream_add_track(media_stream, CODEC_PCMA);
    transceiver.audio = RECVONLY;
  }

  peer_connection_add_stream(peer_connection_, media_stream);
  peer_connection_add_transceiver(peer_connection_, transceiver);
  peer_connection_ontrack(peer_connection_, (void*)WebrtcConnection::OnTrack, this);
  peer_connection_onicecandidate(peer_connection_, (void*)WebrtcConnection::OnIcecandidate, this);
  peer_connection_oniceconnectionstatechange(peer_connection_, (void*)&WebrtcConnection::OnIceconnectionstatechange, this);
  peer_connection_create_answer(peer_connection_);

  rtp_depacketizer_.set_peer_connection(peer_connection_);

  g_cond_wait(&cond_, &mutex_);
  peer_connection_set_remote_description(peer_connection_, offer);
  g_mutex_unlock(&mutex_);

  return sdp_;
}

