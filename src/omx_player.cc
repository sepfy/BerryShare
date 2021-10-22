#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <spdlog/spdlog.h>

#define OMX_SKIP64BIT

#include "bcm_host.h"

extern "C" {
#include "ilclient.h"
}

#include "omx_player.h"

#define RING_BUFFER_SIZE 1024*1024*8

void OmxPlayer::Deinit() {

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

int OmxPlayer::Init() {

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

OmxPlayer::OmxPlayer() {
  playback_ = false;
}

OmxPlayer::~OmxPlayer() {

}

void OmxPlayer::Stop() {

  playback_ = false;
  pthread_join(tid_, NULL);
  video_buffer_.Destroy();
}

void OmxPlayer::Restart() {
  Deinit();
  Init();
}


int OmxPlayer::Play() {

  video_buffer_.Create(RING_BUFFER_SIZE);
  playback_ = true;
  pthread_create(&tid_, NULL, OmxPlayer::Playback, this);

  return 0;
}

bool OmxPlayer::VideoBufferIsEmpty() {
  return video_buffer_.IsEmpty();
}


int OmxPlayer::ReadVideoBuffer(uint8_t *data, size_t size) {

  int ret = -1;
  pthread_mutex_lock(&mutex_);
  ret = video_buffer_.Read(data, size);
  pthread_mutex_unlock(&mutex_);
  return ret;
}

int OmxPlayer::WriteVideoBuffer(uint8_t *data, size_t size) {

  int ret = -1;
  pthread_mutex_lock(&mutex_);
  ret = video_buffer_.Write(data, size);
  pthread_mutex_unlock(&mutex_);
  return ret;
}



void* OmxPlayer::Playback(void *data) {

  OmxPlayer *omx_player = (OmxPlayer*)data;
#if 0
  OMX_VIDEO_PARAM_PORTFORMATTYPE format;
   OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
   OMX_CONFIG_DISPLAYREGIONTYPE displayconfig;
   COMPONENT_T *video_decode = NULL, *video_scheduler = NULL, *video_render = NULL, *clock = NULL;
   COMPONENT_T *list[5];
   TUNNEL_T tunnel[4];
   ILCLIENT_T *client;

   int status = 0;
   unsigned int data_len = 0;

   memset(list, 0, sizeof(list));
   memset(tunnel, 0, sizeof(tunnel));

   if((client = ilclient_init()) == NULL)
   {
//      return -3;
   }

   if(OMX_Init() != OMX_ErrorNone)
   {
      ilclient_destroy(client);
//      return -4;
   }

   // create video_decode
   if(ilclient_create_component(client, &video_decode, (char*)"video_decode", (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS)) != 0)
      status = -14;
   list[0] = video_decode;

   // create video_render
   if(status == 0 && ilclient_create_component(client, &video_render, (char*)"video_render", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[1] = video_render;

   // create clock
   if(status == 0 && ilclient_create_component(client, &clock, (char*)"clock", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[2] = clock;

   memset(&cstate, 0, sizeof(cstate));
   cstate.nSize = sizeof(cstate);
   cstate.nVersion.nVersion = OMX_VERSION;
   cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
   cstate.nWaitMask = 1;
   if(clock != NULL && OMX_SetParameter(ILC_GET_HANDLE(clock), OMX_IndexConfigTimeClockState, &cstate) != OMX_ErrorNone)
      status = -13;

   // create video_scheduler
   if(status == 0 && ilclient_create_component(client, &video_scheduler, (char*)"video_scheduler", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      status = -14;
   list[3] = video_scheduler;

   set_tunnel(tunnel, video_decode, 131, video_scheduler, 10);
   set_tunnel(tunnel+1, video_scheduler, 11, video_render, 90);
   set_tunnel(tunnel+2, clock, 80, video_scheduler, 12);

   // setup clock tunnel first
   if(status == 0 && ilclient_setup_tunnel(tunnel+2, 0, 0) != 0)
      status = -15;
   else
      ilclient_change_component_state(clock, OMX_StateExecuting);

   if(status == 0)
      ilclient_change_component_state(video_decode, OMX_StateIdle);


   memset(&displayconfig, 0, sizeof(displayconfig));
   displayconfig.nSize = sizeof(displayconfig);
   displayconfig.nVersion.nVersion = OMX_VERSION;
   displayconfig.set = (OMX_DISPLAYSETTYPE)(OMX_DISPLAY_SET_FULLSCREEN | OMX_DISPLAY_SET_DEST_RECT | OMX_DISPLAY_SET_LAYER);
   displayconfig.nPortIndex = 90;

   displayconfig.fullscreen = OMX_TRUE; 
   displayconfig.dest_rect.x_offset = 0;
   displayconfig.dest_rect.y_offset = 0;
   displayconfig.dest_rect.width = 1920;
   displayconfig.dest_rect.height = 1080;
   displayconfig.layer = 0;

   if (video_render != NULL && OMX_SetParameter(ILC_GET_HANDLE(video_render), OMX_IndexConfigDisplayRegion, &displayconfig) != OMX_ErrorNone) {
      status = -13;
      printf ("OMX_IndexConfigDisplayRegion failed\n");
   }


   memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
   format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
   format.nVersion.nVersion = OMX_VERSION;
   format.nPortIndex = 130;
   format.eCompressionFormat = OMX_VIDEO_CodingAVC;

#endif
 
  unsigned int data_len = 0;
SPDLOG_INFO("");
  if(omx_player->Init() != -1) {

    OMX_BUFFERHEADERTYPE *buf;
    int port_settings_changed = 0;
    int first_packet = 1;
SPDLOG_INFO("");

    while(omx_player->playback_) {
      if((buf = ilclient_get_input_buffer(omx_player->video_decode_, 130, 0)) == NULL) {
        SPDLOG_INFO("Restart player..");
        omx_player->Restart();
        continue;
      }
      // feed data and wait until we get port settings changed
      unsigned char *dest = buf->pBuffer;

      while(omx_player->VideoBufferIsEmpty() && omx_player->playback_) {
        usleep(100);
      }
      
      data_len += omx_player->ReadVideoBuffer(dest, buf->nAllocLen);
         
      if(port_settings_changed == 0 &&
       ((data_len > 0 && ilclient_remove_event(omx_player->video_decode_, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
       (data_len == 0 && ilclient_wait_for_event(omx_player->video_decode_, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
       ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0))) {

        port_settings_changed = 1;

        if(ilclient_setup_tunnel(omx_player->tunnels_, 0, 0) != 0) {
          break;
        }

        ilclient_change_component_state(omx_player->video_scheduler_, OMX_StateExecuting);

        // now setup tunnel to video_render
        if(ilclient_setup_tunnel(omx_player->tunnels_ + 1, 0, 1000) != 0) {
          break;
        }

        ilclient_change_component_state(omx_player->video_render_, OMX_StateExecuting);
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

      if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(omx_player->video_decode_), buf) != OMX_ErrorNone) {
        break;
      }
    }

    buf->nFilledLen = 0;
    buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

    OMX_EmptyThisBuffer(ILC_GET_HANDLE(omx_player->video_decode_), buf);

    // wait for EOS from render
    ilclient_wait_for_event(omx_player->video_render_, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0,
     ILCLIENT_BUFFER_FLAG_EOS, -1);
    // need to flush the renderer to allow video_decode to disable its input port
    ilclient_flush_tunnels(omx_player->tunnels_, 0);
  }

  omx_player->Deinit();
#if 0
   ilclient_disable_tunnel(tunnel);
   ilclient_disable_tunnel(tunnel+1);
   ilclient_disable_tunnel(tunnel+2);
   ilclient_disable_port_buffers(video_decode, 130, NULL, NULL, NULL);
   ilclient_teardown_tunnels(tunnel);

   ilclient_state_transition(list, OMX_StateIdle);
   ilclient_state_transition(list, OMX_StateLoaded);

   ilclient_cleanup_components(list);

   OMX_Deinit();

   ilclient_destroy(client);
#endif

  pthread_exit(NULL);
}

