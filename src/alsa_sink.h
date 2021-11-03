#ifndef ALSA_SINK_H_
#define ALSA_SINK_H_

#include <stdint.h>
#include <stdlib.h>
#include <thread>
#include <alsa/asoundlib.h>

#include "media_sink.h"


class AlsaSink : public MediaSink {

 public:

  AlsaSink();
  ~AlsaSink();
  static void Playback(void *context);

  int Play();
  void Pause();

 private:

  int Init();
  void Deinit();

  snd_pcm_t *pcm_handle_;
  snd_pcm_hw_params_t *pcm_hw_params_;
  snd_pcm_uframes_t pcm_frames_;

  std::thread playback_thread_;

  bool playback_;

};

#endif // ALSA_SINK_H_
