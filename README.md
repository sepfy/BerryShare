# BerryShare

Share your PC desktop to Raspberry Pi with WebRTC.

![Webp net-gifmaker](https://user-images.githubusercontent.com/22016807/141047271-cb32cde9-9457-4e25-8f54-e8a903a8ccfe.gif)

### Getting 

* Install the latest version of RaspiOS image (Lite or with desktop).
* Connect monitor with HDMI cable, boot up Raspberry Pi and configure network.
* Download <b>berry-share_\<version\>.deb</b> to Raspberry Pi
* Install BerryShare package
```
$ sudo apt install ./berry-share_<version>.deb
$ sudo systemctl start berry-share
```
* Open Chrome on your PC and enter the URL https://\<your raspberry pi ip\>:30001
* It will show "Your connection is not private". Click "Advance" and "Proceed to \<ip\> (unsafe)" to skip the authorization.
* Choose "Entire Screen" and click "Share".
* Raspberry Pi will display your PC desktop!


### Development

* Install Git and clone the repository.
```
$ apt update
$ apt install -y git
$ git clone --recursive https://github.com/sepfy/BerryShare
```
* Install dependencies.
```
$ cd BerryShare
$ ./scripts/install-deps.sh
```
* Compile BerryShare.
```
$ mkdir cmake
$ cd cmake
$ cmake ..
$ make -j4
```
* Package to dpkg.
```
$ ./scripts/create-dpkg.sh
```

### Dependencies
* [websocketpp](https://github.com/zaphoyd/websocketpp): Web service and WebRTC signaling.
* [nlohmann/json](https://github.com/nlohmann/json): Json parser.
* [spdlog](https://github.com/gabime/spdlog): Logging system.
* [pear](https://github.com/sepfy/pear): WebRTC engine.
* [media-server](https://github.com/ireader/media-server): RTP packet parser.
* [userland](https://github.com/raspberrypi/userland): Video decode and playback.
* [alsa-lib](https://github.com/michaelwu/alsa-lib): Audio decode and playback.

### Known Issues
* No audio
* Only support PC, no mobile.
* Only support Chrome.
* Only entire screen, no chrome tab.

