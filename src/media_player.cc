#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#include "omx_sink.h"
#include "alsa_sink.h"

#include "media_player.h"

MediaPlayer::MediaPlayer() {

  video_sink_ = std::make_shared<OmxSink>();
  audio_sink_ = std::make_shared<AlsaSink>();
}

MediaPlayer::~MediaPlayer() {

}

bool MediaPlayer::Play() {

  video_sink_->Play();
  audio_sink_->Play();
  return true;
}


bool MediaPlayer::Pause() {

  video_sink_->Pause();
  audio_sink_->Pause();
  return true;
}

bool MediaPlayer::Play(MediaType media_type) {

  switch(media_type) {
    case MediaType::kMediaAudio:
      audio_sink_->Play();
      break;
    case MediaType::kMediaVideo:
      video_sink_->Play();
      break;
    default:
      return false;
  }

  return true;
}


bool MediaPlayer::Pause(MediaType media_type) {

  switch(media_type) {
    case MediaType::kMediaAudio:
      audio_sink_->Pause();
      break;
    case MediaType::kMediaVideo:
      video_sink_->Pause();
      break;
    default:
      return false;
  }

  return true;
}


int MediaPlayer::WriteMediaBuffer(uint8_t *data, size_t size, MediaType media_type) {

  switch(media_type) {
    case MediaType::kMediaAudio:
      return audio_sink_->WriteMediaBuffer(data, size);
    case MediaType::kMediaVideo:
      return video_sink_->WriteMediaBuffer(data, size);
    default:
      return -1;
  }

  return -1;
}


