
class BerryShare {

  constructor() {

    var pcol;
    var u = document.URL;

    if(u.substring(0, 5) === 'https') {
      pcol = 'wss://';
      u = u.substr(8);
    }
    else {
      pcol = 'ws://';
      if(u.substring(0, 4) === 'http')
        u = u.substr(7);
    }

    u = u.split('/');

    /* + "/xxx" bit is for IE10 workaround */
    this.wsEndpoint =  pcol + u[0];
    this.stunUrl = 'stun:' + u[0].split(':')[0] + ':3478';
    this.name = randomAnimal();
  }


  webrtcConnect() {

    let pc = new RTCPeerConnection({
      iceServers: [{urls: this.stunUrl}]
    });

    let log = msg => {
      console.log(msg);
    };

    pc.oniceconnectionstatechange = e => log(pc.iceConnectionState);

    pc.onicecandidate = event => {
      if(event.candidate === null) {
        console.log(btoa(pc.localDescription.sdp));
        let offer = '{"type": "offer", "sdp":"' + btoa(pc.localDescription.sdp) + '", "name": "' + this.name + '"}';
        this.ws.send(offer);
      }
    }

    navigator.mediaDevices.getDisplayMedia({ video: true, audio: false })
      .then(stream => {
        stream.getTracks().forEach(track => pc.addTrack(track, stream));
        pc.createOffer().then(d => pc.setLocalDescription(d)).catch(log)
      }).catch(log);

  }


  wsOpen() {
    document.getElementById('info').innerHTML = 'BerryShare: Connected';
    document.getElementById('name').innerHTML = 'Your name: ' + this.name;
  }

  wsOnmessage(msg) {
    let data = JSON.parse(msg.data);
    //console.log(data);
    if(data.type == "status") {

      if(data.isAvailable) {
        this.webrtcConnect();
      }
      else {
        if(data.name == this.castingName) {
          document.getElementById('info').innerHTML = 'BerryShare: Casting';
        }
        else {
          document.getElementById('info').innerHTML = 'BerryShare: "' + data.castingName + '" is casting';
        }
      }
    }
    else if(data.type == "answer") {
      console.log(data.sdp);

    }
  }

  wsOnclose() {
    document.getElementById('info').innerHTML = 'BerryShare: Disconnected';
  }

  wsInit() {
    this.ws = new WebSocket(this.wsEndpoint);
    this.ws.onopen = () => { this.wsOpen(); }
    this.ws.onmessage = (msg) => { this.wsOnmessage(msg); }
    this.ws.onclose = () => { this.wsOnclose(); }
  }


}


let berryShare = new BerryShare();
berryShare.wsInit();

