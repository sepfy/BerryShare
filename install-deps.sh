#!/bin/bash


sudo apt install libboost-system-dev nlohmann-json-dev libspdlog-dev libwebsocketpp-dev

BASE_PATH=$(pwd)

cd $BASE_PATH/third_party/pear/
./build-third-party.sh

cd $BASE_PATH/third_party/userland/host_applications/linux/apps/hello_pi/libs/ilclient/

make
sudo cp libilclient.a /usr/lib/
sudo cp ilclient.h /usr/include/

