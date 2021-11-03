#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <spdlog/spdlog.h>

#include "alsa_sink.h"

const int kAlsaBufferSize = 1024*1024;

AlsaSink::AlsaSink() : MediaSink(kAlsaBufferSize) {

}

AlsaSink::~AlsaSink() {

}

void AlsaSink::Deinit() {

  snd_pcm_drain(pcm_handle_);
  snd_pcm_close(pcm_handle_);
}

int AlsaSink::Init() {

  SPDLOG_INFO("ALSA sink initialize");
  int channels = 1;
  unsigned int samplerate = 8000;
  unsigned int pcm;

  if(pcm = snd_pcm_open(&pcm_handle_, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
    SPDLOG_ERROR("Can't open \"{}\" PCM device. {}\n", "default", snd_strerror(pcm));

  snd_pcm_hw_params_alloca(&pcm_hw_params_);

  snd_pcm_hw_params_any(pcm_handle_, pcm_hw_params_);

  if(pcm = snd_pcm_hw_params_set_access(pcm_handle_, pcm_hw_params_, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
    SPDLOG_ERROR("Can't set interleaved mode. %s\n", snd_strerror(pcm));

  if(pcm = snd_pcm_hw_params_set_format(pcm_handle_, pcm_hw_params_, SND_PCM_FORMAT_A_LAW) < 0)
    SPDLOG_ERROR("Can't set format. %s\n", snd_strerror(pcm));

  if(pcm = snd_pcm_hw_params_set_channels(pcm_handle_, pcm_hw_params_, channels) < 0)
    SPDLOG_ERROR("Can't set channels number. %s\n", snd_strerror(pcm));

  if(pcm = snd_pcm_hw_params_set_rate_near(pcm_handle_, pcm_hw_params_, &samplerate, 0) < 0)
    SPDLOG_ERROR("Can't set rate. %s\n", snd_strerror(pcm));

  if(pcm = snd_pcm_hw_params(pcm_handle_, pcm_hw_params_) < 0)
    SPDLOG_ERROR("Can't set harware parameters. %s\n", snd_strerror(pcm));

   /* Resume information */
   //printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle_));
   //printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle_)));

#if 0
   snd_pcm_hw_params_get_period_time(params, &tmp, NULL);
#endif
  return 0;
}


void AlsaSink::Pause() {

  playback_ = false;
  playback_thread_.join();

}

int AlsaSink::Play() {

  playback_ = true;
  playback_thread_ = std::thread(AlsaSink::Playback, this);

  return 0;
}


void AlsaSink::Playback(void *context) {


  AlsaSink *alsa_sink = (AlsaSink*)context;

  uint32_t pcm;
  int len;

  alsa_sink->Init();
  snd_pcm_uframes_t frames;
  snd_pcm_hw_params_get_period_size(alsa_sink->pcm_hw_params_, &frames, 0);

  int buff_size = frames;
  uint8_t *buf = (uint8_t*)malloc(buff_size);

  SPDLOG_INFO("Start loop");
  while(alsa_sink->playback_) {
 
    if(alsa_sink->BufferIsEmpty()) {
      usleep(10000);
      continue;
    }

    len = alsa_sink->ReadMediaBuffer(buf, buff_size);
    if(pcm = snd_pcm_writei(alsa_sink->pcm_handle_, buf, len) == -EPIPE) {
      SPDLOG_WARN("overrun.");
      snd_pcm_prepare(alsa_sink->pcm_handle_);
    }
    else if(pcm < 0) {
      SPDLOG_ERROR("ERROR. Can't write to PCM device. %s\n", snd_strerror(pcm));
    }
  }

  if(buf)
    free(buf);

  alsa_sink->Deinit();

}

