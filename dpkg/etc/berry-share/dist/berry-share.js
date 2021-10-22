
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
    this.castingName = randomAnimal();
  }


  webrtcConnect() {

    this.pc = new RTCPeerConnection({
      iceServers: [{urls: this.stunUrl}]
    });

    let log = msg => {
      console.log(msg);
    };

    this.pc.oniceconnectionstatechange = e => log(this.pc.iceConnectionState);

    this.pc.onicecandidate = event => {
      if(event.candidate === null) {

	let sdp = this.pc.localDescription.sdp;
console.log(sdp);
        let offer = '{"type": "offer", "sdp":"' + btoa(sdp) + '", "name": "' + this.castingName + '"}';
        this.ws.send(offer);
        document.getElementById('info').innerHTML = 'BerryShare: Prepare to cast';
      }
    }

    navigator.mediaDevices.getDisplayMedia({ video: true, audio: false })
      .then(stream => {
        stream.getTracks().forEach(track => {
	  this.pc.addTrack(track, stream);
          track.onended = () => {
            this.ws.close();
	  }
	});
        this.pc.createOffer().then(d => this.pc.setLocalDescription(d)).catch(log)
      }).catch(log);

  }

  wsOpen() {
    document.getElementById('info').innerHTML = 'BerryShare: Connected';
    document.getElementById('name').innerHTML = 'Your name: ' + this.castingName;
  }

  wsOnmessage(msg) {
    let data = JSON.parse(msg.data);
    //console.log(data);
    if(data.type == "status") {

      if(data.isAvailable) {
        this.webrtcConnect();
      }
      else {
        if(data.castingName == this.castingName) {
          document.getElementById('info').innerHTML = 'BerryShare: Casting';
        }
        else {
          document.getElementById('info').innerHTML = 'BerryShare: "' + data.castingName + '" is casting';
        }
      }
    }
    else if(data.type == "answer") {
      try {
        this.pc.setRemoteDescription(new RTCSessionDescription(JSON.parse(atob(data.sdp))));
      }
      catch (e) {
        alert(e);
      }
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

