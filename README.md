# BerryShare

Share your PC screen to Raspberry Pi by WebRTC.

### Procedure

* Install the latest version of RaspiOS image (Lite or with desktop).
* Boot Raspberry Pi, connect with HDMI monitor and configure network.
* Install BerryShare application:
```
$ wget https://github.com/sepfy/BerryShare/
$ sudo dpkg -i berry-share_<version>.deb
$ sudo systemctl start berry-share
```
* Open Chrome on your PC and enter the URL https://\<your raspberry pi ip\>:30001
* Choose Entire Screen and click share
* Raspberry Pi will display your PC screen!



### Development

* Install Git and clone the repository
```
$ apt update
$ apt install -y git
$ git clone --recursive https://github.com/sepfy/BerryShare
```
* Install dependencies
```
cd BerryShare
./scripts/install-deps.sh
```
* Compile BerryShare
```
mkdir cmake
cd cmake
cmake ..
make -j4
```

### Known issues
* No audio
* Only support PC, no mobile.
* Only support Chrome.
* Only entire screen, no chrome tab.
