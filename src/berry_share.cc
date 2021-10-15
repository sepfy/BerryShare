#include <iostream>
#include <nlohmann/json.hpp>

#include "berry_share.h"

using json = nlohmann::json;

BerryShare::BerryShare() {
  is_available_ = true;
}

BerryShare::~BerryShare() {

}

std::string BerryShare::Request(std::string info) {

  auto j = json::parse(info);
  std::cout << info << "\n";
  if(j["type"] == "offer" && is_available_ == true) {
    auto sdp = RequestCasting(j["sdp"]);
    json response;
    response["type"] = "answer";
    response["sdp"] = "testtesttest";
    is_available_ = false;
    casting_name_ = j["name"];
    return response.dump();
  }

  return "";
}

std::string BerryShare::RequestCasting(std::string offer) {
  
  return "testtestres";
}

std::string BerryShare::GetStatus() {

  json j;
  j["type"] = "status";
  j["isAvailable"] = is_available_;
  j["castingName"] = casting_name_;
  return j.dump();
}
