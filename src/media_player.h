#ifndef MEDIA_PLAYER_H_
#define MEDIA_PLAYER_H_

#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#include "media_sink.h"

enum class MediaType {
  kMediaVideo,
  kMediaAudio,
};

class MediaPlayer {

 public:

  MediaPlayer();
  ~MediaPlayer();

  bool Play();
  bool Pause();

  bool Play(MediaType media_type);
  bool Pause(MediaType media_type);

  int WriteMediaBuffer(uint8_t *data, size_t size, MediaType media_type);

 private:
  std::shared_ptr<MediaSink> audio_sink_;
  std::shared_ptr<MediaSink> video_sink_;
  bool playback_;

};

#endif // MEDIA_PLAYER_H_
