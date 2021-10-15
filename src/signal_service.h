#ifndef SIGNAL_SERVICE_H_
#define SIGNAL_SERVICE_H_

#include <csignal>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>

#include "berry_share.h"

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using ssl_context = websocketpp::lib::asio::ssl::context;

class SignalService {

 public:
  typedef websocketpp::connection_hdl connection_hdl;
  typedef websocketpp::server<websocketpp::config::asio_tls> server;
  void Run();

  SignalService(std::shared_ptr<BerryShare> &berry_share);
  
 private:

  typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

  void OnClose(connection_hdl hdl);
  void OnOpen(connection_hdl hdl);
  void OnHttp(connection_hdl hdl);
  void OnMessage(connection_hdl hdl, websocketpp::config::asio::message_type::ptr msg);
  static websocketpp::lib::shared_ptr<ssl_context> OnTlsInit(connection_hdl hdl);

  con_list connections_;
  server endpoint_;
  std::string docroot_ = "./dist/";

  std::shared_ptr<BerryShare> berry_share_;
};

#endif // SIGNAL_SERVICE_H_
