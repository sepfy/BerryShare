#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spdlog/spdlog.h>
#include <bcm_host.h>

#include "omx_sink.h"

const int kOmxBufferSize = 1024*1024*8;

OmxSink::OmxSink() : MediaSink(kOmxBufferSize) {

}

OmxSink::~OmxSink() {

}

void OmxSink::Deinit() {

  ilclient_disable_tunnel(tunnels_);
  ilclient_disable_tunnel(tunnels_ + 1);
  ilclient_disable_tunnel(tunnels_ + 2);
  ilclient_disable_port_buffers(video_decode_, 130, NULL, NULL, NULL);
  ilclient_teardown_tunnels(tunnels_);

  ilclient_state_transition(components_, OMX_StateIdle);
  ilclient_state_transition(components_, OMX_StateLoaded);

  ilclient_cleanup_components(components_);

  OMX_Deinit();

  ilclient_destroy(ilclient_);
  bcm_host_deinit();

}

int OmxSink::Init() {

  SPDLOG_INFO("OMX sink initialize");
  CleanBuffer();
  bcm_host_init();
  int ret = -1;
  int status = 0;

  OMX_VIDEO_PARAM_PORTFORMATTYPE format;
  OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;

  memset(components_, 0, sizeof(components_));
  memset(tunnels_, 0, sizeof(tunnels_));

  if((ilclient_ = ilclient_init()) == NULL) {
    return -1;
  }

  do {

    if(OMX_Init() != OMX_ErrorNone)
      break;

    // create video_decode
    if(ilclient_create_component(ilclient_, &video_decode_, (char*)"video_decode",
     (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS)) != 0)
      break;

    components_[0] = video_decode_;

    // create video_render
    if(ilclient_create_component(ilclient_, &video_render_, (char*)"video_render",
     ILCLIENT_DISABLE_ALL_PORTS) != 0)
      break;

    components_[1] = video_render_;

    // create clock
    if(ilclient_create_component(ilclient_, &clock_, (char*)"clock", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      break;

    components_[2] = clock_;

    memset(&cstate, 0, sizeof(cstate));
    cstate.nSize = sizeof(cstate);
    cstate.nVersion.nVersion = OMX_VERSION;
    cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
    cstate.nWaitMask = 1;

    if(clock_ != NULL && OMX_SetParameter(ILC_GET_HANDLE(clock_),
     OMX_IndexConfigTimeClockState, &cstate) != OMX_ErrorNone)
      break;

    // create video_scheduler
    if(ilclient_create_component(ilclient_, &video_scheduler_, (char*)"video_scheduler",
     ILCLIENT_DISABLE_ALL_PORTS) != 0)
      break;

    components_[3] = video_scheduler_;

    set_tunnel(tunnels_, video_decode_, 131, video_scheduler_, 10);
    set_tunnel(tunnels_ + 1, video_scheduler_, 11, video_render_, 90);
    set_tunnel(tunnels_ + 2, clock_, 80, video_scheduler_, 12);

    if(status == 0 && ilclient_setup_tunnel(tunnels_ + 2, 0, 0) != 0)
      break;

    ilclient_change_component_state(clock_, OMX_StateExecuting);

    ilclient_change_component_state(video_decode_, OMX_StateIdle);

    memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
    format.nVersion.nVersion = OMX_VERSION;
    format.nPortIndex = 130;
    format.eCompressionFormat = OMX_VIDEO_CodingAVC;

    if(OMX_SetParameter(ILC_GET_HANDLE(video_decode_), OMX_IndexParamVideoPortFormat, &format) != OMX_ErrorNone)
      break;

    if(ilclient_enable_port_buffers(video_decode_, 130, NULL, NULL, NULL) != 0)
      break;
    ilclient_change_component_state(video_decode_, OMX_StateExecuting);

    ret = 0;
  } while(0);

  if(ret < 0)
    ilclient_destroy(ilclient_);

  return ret;

}


void OmxSink::Pause() {

  playback_ = false;
  if(playback_thread_.joinable())
    playback_thread_.join();
}

int OmxSink::Play() {

  playback_ = true;
  playback_thread_ = std::thread(OmxSink::Playback, this);

  return 0;
}


void OmxSink::Playback(void *context) {

  OmxSink *omx_sink = (OmxSink*)context;
 
  unsigned int data_len = 0;
  if(omx_sink->Init() != -1) {

    OMX_BUFFERHEADERTYPE *buf = NULL;
    int port_settings_changed = 0;
    int first_packet = 1;
    SPDLOG_INFO("Start loop");
    while(omx_sink->playback_) {

      if(omx_sink->BufferIsEmpty()) {
        usleep(100);
	continue;
      }

      if((buf = ilclient_get_input_buffer(omx_sink->video_decode_, 130, 0)) == NULL) {
        SPDLOG_INFO("Restart player..");
        continue;
      }
      // feed data and wait until we get port settings changed
      unsigned char *dest = buf->pBuffer;
      
      data_len += omx_sink->ReadMediaBuffer(dest, buf->nAllocLen);
         
      if(port_settings_changed == 0 &&
       ((data_len > 0 && ilclient_remove_event(omx_sink->video_decode_, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
       (data_len == 0 && ilclient_wait_for_event(omx_sink->video_decode_, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
       ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0))) {

        port_settings_changed = 1;

        if(ilclient_setup_tunnel(omx_sink->tunnels_, 0, 0) != 0) {
          break;
        }

        ilclient_change_component_state(omx_sink->video_scheduler_, OMX_StateExecuting);

        // now setup tunnel to video_render
        if(ilclient_setup_tunnel(omx_sink->tunnels_ + 1, 0, 1000) != 0) {
          break;
        }

        ilclient_change_component_state(omx_sink->video_render_, OMX_StateExecuting);
      }
  
      if(!data_len)
        break;

      buf->nFilledLen = data_len;
      data_len = 0;

      buf->nOffset = 0;
      if(first_packet) {
        buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
        first_packet = 0;
      }
      else
        buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

      if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(omx_sink->video_decode_), buf) != OMX_ErrorNone) {
        break;
      }
    }

    if(buf) {
      buf->nFilledLen = 0;
      buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

      OMX_EmptyThisBuffer(ILC_GET_HANDLE(omx_sink->video_decode_), buf);

      // wait for EOS from render
      ilclient_wait_for_event(omx_sink->video_render_, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0,
       ILCLIENT_BUFFER_FLAG_EOS, -1);
      // need to flush the renderer to allow video_decode to disable its input port
      ilclient_flush_tunnels(omx_sink->tunnels_, 0);
    }
  }
  omx_sink->Deinit();
  pthread_exit(NULL);
}

