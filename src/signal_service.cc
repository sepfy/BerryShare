#include <csignal>
#include <fstream>
#include <iostream>
#include <set>
#include <spdlog/spdlog.h>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "signal_service.h"

const std::string kTlsKeyPath = "/etc/berry-share/key.pem"; 
const std::string kTlsCertPath = "/etc/berry-share/cert.pem";
const std::string kDocRoot = "/etc/berry-share/dist/";

SignalService::SignalService(std::shared_ptr<WebrtcConnection> &webrtc_connection) {

  endpoint_.clear_access_channels(websocketpp::log::alevel::all);
  endpoint_.clear_error_channels(websocketpp::log::elevel::all);

  endpoint_.init_asio();
  endpoint_.set_open_handler(bind(&SignalService::OnOpen, this,_1));
  endpoint_.set_close_handler(bind(&SignalService::OnClose, this,_1));
  endpoint_.set_message_handler(websocketpp::lib::bind(&SignalService::OnMessage, this, ::_1, ::_2));
  endpoint_.set_http_handler(websocketpp::lib::bind(&SignalService::OnHttp, this,::_1));
  endpoint_.set_tls_init_handler(websocketpp::lib::bind(&SignalService::OnTlsInit, ::_1));
  endpoint_.set_reuse_addr(true);

  webrtc_connection_ = webrtc_connection;
}

void SignalService::OnOpen(connection_hdl hdl) {
  connections_[hdl] = "";
  endpoint_.send(hdl, webrtc_connection_->GetStatus(), websocketpp::frame::opcode::text);
}

void SignalService::OnClose(connection_hdl hdl) {
  printf("Close %s\n", connections_[hdl].c_str());
  if(connections_[hdl] == webrtc_connection_->casting_name()) {
    webrtc_connection_->StopCasting();
  }
  connections_.erase(hdl);
}

websocketpp::lib::shared_ptr<ssl_context> SignalService::OnTlsInit(connection_hdl hdl) {

  auto ctx = websocketpp::lib::make_shared<ssl_context>(ssl_context::sslv23);
  ctx->use_certificate_chain_file(kTlsCertPath);
  ctx->use_private_key_file(kTlsKeyPath, ssl_context::pem);
  return ctx;
}

void SignalService::OnMessage(connection_hdl hdl, websocketpp::config::asio::message_type::ptr msg) {
  auto response = webrtc_connection_->Request(msg->get_payload());
  endpoint_.send(hdl, response, websocketpp::frame::opcode::text);

  auto j = json::parse(msg->get_payload());
  connections_[hdl] = j["name"];

  auto status = webrtc_connection_->GetStatus();
  for(auto it : connections_) {
    endpoint_.send(it.first, status, websocketpp::frame::opcode::text);
  }

}


void SignalService::OnHttp(connection_hdl hdl) {
  
  server::connection_ptr con = endpoint_.get_con_from_hdl(hdl);

  std::ifstream file;
  std::string filename = con->get_resource();
  std::string response;

  //    m_endpoint.get_alog().write(websocketpp::log::alevel::app,
  //        "http request1: "+filename);

  if(filename == "/") {
    filename = kDocRoot + "index.html";
  }
  else {
    filename = kDocRoot + filename.substr(1);
  }

  //m_endpoint.get_alog().write(websocketpp::log::alevel::app,
  //        "http request2: "+filename);

  file.open(filename.c_str(), std::ios::in);
  if(!file) {
    // 404 error
    std::stringstream ss;

    ss << "<!doctype html><html><head>"
     << "<title>Error 404 (Resource not found)</title><body>"
     << "<h1>Error 404</h1>"
     << "<p>The requested URL " << filename << " was not found on this server.</p>"
     << "</body></head></html>";

    con->set_body(ss.str());
    con->set_status(websocketpp::http::status_code::not_found);
    return;
  }

  file.seekg(0, std::ios::end);
  response.reserve(file.tellg());
  file.seekg(0, std::ios::beg);

  response.assign((std::istreambuf_iterator<char>(file)),
   std::istreambuf_iterator<char>());

  con->set_body(response);
  con->set_status(websocketpp::http::status_code::ok);
  
}

void SignalService::Shutdown() {

  endpoint_.stop_listening();
  endpoint_.stop();
}

void SignalService::Run() {

  endpoint_.listen(30001);
  endpoint_.start_accept();
  endpoint_.run();
}

