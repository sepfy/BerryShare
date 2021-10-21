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

#if 0
int OmxPlayer::Init() {

  OMX_VIDEO_PARAM_PORTFORMATTYPE format;
  OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
  COMPONENT_T *video_decode = NULL, *video_scheduler = NULL, *video_render = NULL, *clock = NULL;
  COMPONENT_T *list[5];
  TUNNEL_T tunnel[4];
  ILCLIENT_T *client;

  int status = 0;
  unsigned int data_len = 0;

  memset(list, 0, sizeof(list));
  memset(tunnel, 0, sizeof(tunnel));

  if((client = ilclient_init()) == NULL) {
    return -1;
  }

  do {

    if(OMX_Init() != OMX_ErrorNone)
      break;

    // create video_decode
    if(ilclient_create_component(client, &video_decode, (char*)"video_decode",
     (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS)) != 0)
      break;

    list[0] = video_decode;

    // create video_render
    if(ilclient_create_component(client, &video_render, (char*)"video_render",
     ILCLIENT_DISABLE_ALL_PORTS) != 0)
      break;

    list[1] = video_render;

    // create clock
    if(ilclient_create_component(client, &clock, (char*)"clock", ILCLIENT_DISABLE_ALL_PORTS) != 0)
      break;

    list[2] = clock;

    memset(&cstate, 0, sizeof(cstate));
    cstate.nSize = sizeof(cstate);
    cstate.nVersion.nVersion = OMX_VERSION;
    cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
    cstate.nWaitMask = 1;

    if(clock != NULL && OMX_SetParameter(ILC_GET_HANDLE(clock),
     OMX_IndexConfigTimeClockState, &cstate) != OMX_ErrorNone)
      break;

    // create video_scheduler
    if(ilclient_create_component(client, &video_scheduler, (char*)"video_scheduler",
     ILCLIENT_DISABLE_ALL_PORTS) != 0)
      break;

    list[3] = video_scheduler;

    set_tunnel(tunnel, video_decode, 131, video_scheduler, 10);
    set_tunnel(tunnel+1, video_scheduler, 11, video_render, 90);
    set_tunnel(tunnel+2, clock, 80, video_scheduler, 12);

    if(status == 0 && ilclient_setup_tunnel(tunnel+2, 0, 0) != 0)
      break;

    ilclient_change_component_state(clock, OMX_StateExecuting);

    ilclient_change_component_state(video_decode, OMX_StateIdle);

    memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
    format.nVersion.nVersion = OMX_VERSION;
    format.nPortIndex = 130;
    format.eCompressionFormat = OMX_VIDEO_CodingAVC;

   if(OMX_SetParameter(ILC_GET_HANDLE(video_decode),
    OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone &&
    ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) != 0)
     break;

    ret = 0;
  } while(0);

  ilclient_destroy(client);
  return ret;

}
#endif

OmxPlayer::OmxPlayer() {
  playback_ = false;
}

OmxPlayer::~OmxPlayer() {

}

void OmxPlayer::Deinit() {

  playback_ = false;
  pthread_join(tid_, NULL);
  video_buffer_.Destroy();
}

int OmxPlayer::Init() {

  video_buffer_.Create(RING_BUFFER_SIZE);
  bcm_host_init();
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
   OMX_VIDEO_PARAM_PORTFORMATTYPE format;
   OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
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

   memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
   format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
   format.nVersion.nVersion = OMX_VERSION;
   format.nPortIndex = 130;
   format.eCompressionFormat = OMX_VIDEO_CodingAVC;

   if(status == 0 &&
      OMX_SetParameter(ILC_GET_HANDLE(video_decode), OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone &&
      ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) == 0)
   {
      OMX_BUFFERHEADERTYPE *buf;
      int port_settings_changed = 0;
      int first_packet = 1;

      ilclient_change_component_state(video_decode, OMX_StateExecuting);
      while(omx_player->playback_)
      {
	 SPDLOG_INFO("");
         if((buf = ilclient_get_input_buffer(video_decode, 130, 1)) == NULL) {
           continue;
         }
	 SPDLOG_INFO("");
	 printf("%d\n", omx_player->playback_);
         // feed data and wait until we get port settings changed
         unsigned char *dest = buf->pBuffer;
//         data_len += fread(dest, 1, buf->nAllocLen-data_len, in);

         while(omx_player->VideoBufferIsEmpty() && omx_player->playback_) {
             usleep(100);
         }
	 SPDLOG_INFO("");
	 //int total_size = 0;
	 //omx_player_read_buffer((uint8_t*)&total_size, sizeof(total_size));
         data_len += omx_player->ReadVideoBuffer(dest, buf->nAllocLen);
         
	 SPDLOG_INFO("");
         if(port_settings_changed == 0 &&
            ((data_len > 0 && ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
             (data_len == 0 && ilclient_wait_for_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                                                       ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0)))
         {
            port_settings_changed = 1;

            if(ilclient_setup_tunnel(tunnel, 0, 0) != 0)
            {
               status = -7;
               break;
            }

	 SPDLOG_INFO("");
            ilclient_change_component_state(video_scheduler, OMX_StateExecuting);

            // now setup tunnel to video_render
            if(ilclient_setup_tunnel(tunnel+1, 0, 1000) != 0)
            {
               status = -12;
               break;
            }

            ilclient_change_component_state(video_render, OMX_StateExecuting);
         }
         if(!data_len)
            break;

	 SPDLOG_INFO("");
         buf->nFilledLen = data_len;
         data_len = 0;

         buf->nOffset = 0;
         if(first_packet)
         {
            buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
            first_packet = 0;
         }
         else
            buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

	 SPDLOG_INFO("");
         if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         {
            status = -6;
            break;
         }
	 SPDLOG_INFO("");
      }

      buf->nFilledLen = 0;
      buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

	 SPDLOG_INFO("");
      if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
         status = -20;

      // wait for EOS from render
      ilclient_wait_for_event(video_render, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0,
                              ILCLIENT_BUFFER_FLAG_EOS, -1);
	 SPDLOG_INFO("");

      // need to flush the renderer to allow video_decode to disable its input port
      ilclient_flush_tunnels(tunnel, 0);

   }

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

  pthread_exit(NULL);
}

