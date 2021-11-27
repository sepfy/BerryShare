#!/bin/bash


sudo apt install -y libasound2-dev libboost-system-dev nlohmann-json-dev libspdlog-dev libwebsocketpp-dev cmake libglib2.0-dev libssl-dev ninja-build python3-pip
sudo pip3 install meson

BASE_PATH=$(pwd)

cd $BASE_PATH/third_party/pear/
./build-third-party.sh
mkdir cmake
cd cmake
cmake ..
make

cd /opt/vc/src/hello_pi/libs/ilclient/

make
sudo cp libilclient.a /usr/lib/
sudo cp ilclient.h /usr/include/

