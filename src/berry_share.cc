#include <csignal>
#include <fstream>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>

#include "signal_service.h"
#include "webrtc_connection.h"

std::shared_ptr<SignalService> g_signal_service;

static void SignalHandler(int signal) {
  SPDLOG_INFO("shutdown");
  g_signal_service->Shutdown(); 
}

int main(int argc, char *argv[]) {

  signal(SIGINT, SignalHandler);

  std::shared_ptr<WebrtcConnection> webrtc_connection = std::make_shared<WebrtcConnection>();
  g_signal_service = std::make_shared<SignalService>(webrtc_connection);

  webrtc_connection->Init();
  g_signal_service->Run();

  return 0;
}
