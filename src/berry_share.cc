#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "omx_player.h"
#include "berry_share.h"

using json = nlohmann::json;

BerryShare::BerryShare() {
  is_available_ = true;
  sdp_ = NULL;
}

BerryShare::~BerryShare() {

}

std::string BerryShare::Request(std::string info) {

  auto j = json::parse(info);
  if(j["type"] == "offer" && is_available_ == true) {
    auto sdp = RequestCasting(j["sdp"]);
    omx_player_.Play();
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
  omx_player_.Stop();
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
  rtp_depacketizer_.set_omx_player(&omx_player_);
}


void BerryShare::OnIceconnectionstatechange(iceconnectionstate_t state, void *data) {
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
  RtpDepacketizer *rtp_depacketizer = (RtpDepacketizer*)data;
  rtp_depacketizer->Depacket(packet, bytes);
}

char* BerryShare::SetOffer(char *offer, void *data) {

  g_mutex_lock(&mutex_);
  peer_connection_destroy(peer_connection_);
  peer_connection_ = peer_connection_create();
  peer_connection_add_stream(peer_connection_, "H264");
  transceiver_t transceiver = {.video = RECVONLY};
  peer_connection_add_transceiver(peer_connection_, transceiver);
  peer_connection_set_on_track(peer_connection_, (void*)BerryShare::OnTrack, &rtp_depacketizer_);
  peer_connection_set_on_icecandidate(peer_connection_, (void*)BerryShare::OnIcecandidate, this);
  peer_connection_set_on_iceconnectionstatechange(peer_connection_, (void*)&BerryShare::OnIceconnectionstatechange, this);
  peer_connection_create_answer(peer_connection_);

  g_cond_wait(&cond_, &mutex_);
  peer_connection_set_remote_description(peer_connection_, offer);
  g_mutex_unlock(&mutex_);

  return sdp_;
}

