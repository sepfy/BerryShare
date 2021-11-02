#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "media_player.h"
#include "berry_share.h"

using json = nlohmann::json;

BerryShare::BerryShare() {
  is_available_ = true;
  sdp_ = NULL;
  video_rtp_ssrc_ = 0;
  audio_rtp_ssrc_ = 0;
}

BerryShare::~BerryShare() {

}

std::string BerryShare::Request(std::string info) {

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

void BerryShare::StopCasting() {
  is_available_ = true;
  media_player_.Pause();
}

std::string BerryShare::RequestCasting(std::string offer) {
 	
  char *answer = SetOffer((char*)offer.c_str(), NULL);
  return std::string(answer);
}

std::string BerryShare::GetStatus() {

  json j;
  j["type"] = "status";
  j["isAvailable"] = is_available_;
  j["castingName"] = casting_name_;
  return j.dump();
}

void BerryShare::Init() {

  spdlog::set_pattern("[%H:%M:%S][%@] %v");
  rtp_depacketizer_.set_media_player(&media_player_);
}


void BerryShare::OnIceconnectionstatechange(IceConnectionState state, void *data) {
  BerryShare *berry_share = (BerryShare*)data;
  if(state == FAILED) {
    printf("Disconnect with browser... Stop streaming\n");
  }
}

void BerryShare::OnIcecandidate(char *sdp, void *data) {

  BerryShare *berry_share = (BerryShare*)data;

  if(berry_share->sdp_)
    g_free(berry_share->sdp_);

  berry_share->sdp_ = g_base64_encode((const guchar*)sdp, strlen(sdp));
  g_cond_signal(&berry_share->cond_);

}

void BerryShare::OnTrack(uint8_t *packet, size_t bytes, void *data) {

  BerryShare *berry_share = (BerryShare*)data;

  RtpDepacketizer *rtp_depacketizer = &berry_share->rtp_depacketizer_;

  uint32_t ssrc = 0;
  ssrc = ((uint32_t)packet[8] << 24) | ((uint32_t)packet[9] << 16) | ((uint32_t)packet[10] << 8) | ((uint32_t)packet[11]);
  if(ssrc == berry_share->audio_rtp_ssrc_) {
    rtp_depacketizer->DepacketAudio(packet, bytes);
  }
  else if(ssrc == berry_share->video_rtp_ssrc_) {
    rtp_depacketizer->DepacketVideo(packet, bytes);
  }
}

char* BerryShare::SetOffer(char *offer, void *data) {


  g_mutex_lock(&mutex_);

  guchar *decoded_sdp = NULL;
  gsize decoded_sdp_len = strlen(offer);
  decoded_sdp = g_base64_decode((const gchar*)offer, &decoded_sdp_len);

  video_rtp_ssrc_ = session_description_find_ssrc("video", (char*)decoded_sdp);
  audio_rtp_ssrc_ = session_description_find_ssrc("audio", (char*)decoded_sdp);

  if(decoded_sdp)
    g_free(decoded_sdp);

  SPDLOG_INFO("Get offer. RTP video ssrc = {}, audio ssrc = {}", video_rtp_ssrc_, audio_rtp_ssrc_);

  peer_connection_destroy(peer_connection_);
  peer_connection_ = peer_connection_create();

  MediaStream *media_stream = media_stream_new();
  media_stream_add_track(media_stream, CODEC_H264);
  transceiver_t transceiver = {.video = RECVONLY};

  if(audio_rtp_ssrc_ > 0) {
    media_stream_add_track(media_stream, CODEC_PCMA);
    transceiver.audio = RECVONLY;
  }

  peer_connection_add_stream(peer_connection_, media_stream);
  peer_connection_add_transceiver(peer_connection_, transceiver);
  peer_connection_ontrack(peer_connection_, (void*)BerryShare::OnTrack, this);
  peer_connection_onicecandidate(peer_connection_, (void*)BerryShare::OnIcecandidate, this);
  peer_connection_oniceconnectionstatechange(peer_connection_, (void*)&BerryShare::OnIceconnectionstatechange, this);
  peer_connection_create_answer(peer_connection_);

  g_cond_wait(&cond_, &mutex_);
  peer_connection_set_remote_description(peer_connection_, offer);
  g_mutex_unlock(&mutex_);

  return sdp_;
}

