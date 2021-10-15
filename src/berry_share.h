#ifndef BERRY_SHARE_H_
#define BERRY_SHARE_H_

#include <string>

class BerryShare {

 public:
  BerryShare();
  ~BerryShare();
  std::string Request(std::string info);
  std::string RequestCasting(std::string offer);
  std::string GetStatus();

 private:
  bool is_available_;
  std::string casting_name_;
  std::string answer_;
};

#endif // BERRY_SHARE_H_
