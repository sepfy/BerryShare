#include <csignal>
#include <fstream>
#include <iostream>
#include <set>
#include "signal_service.h"
#include "berry_share.h"

int main() {

  std::shared_ptr<BerryShare> berry_share = std::make_shared<BerryShare>();
  SignalService signal_service(berry_share);
  signal_service.Run();
}
