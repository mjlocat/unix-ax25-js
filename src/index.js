const EventEmitter = require('events');
const axsocket = require('../build/Release/axsocket.node');

class UnixAX25 extends EventEmitter {
  constructor() {
    super();
    this.ports = axsocket.enumeratePorts();
    this.socket = axsocket.createAX25Socket();
    this.selectTimeout = 0;
    this.listening = false;
    this.timer = 0;
    this.UIlisten = this.UIlisten.bind(this);
  }

  startUIListener(timeout = 500) {
    this.selectTimeout = timeout;
    this.listening = true;
    this.UIlisten();
  }

  stopUIListener() {
    this.listening = false;
    if (this.timer) {
      clearTimeout(this.timer);
    }
  }

  UIlisten() {
    if (axsocket.selectReadSocket(this.socket, 0)) {
      const packet = axsocket.readAndDecodePacket(this.socket);
      packet.received = new Date();
      this.emit('data', packet);
    }
    if (this.listening) {
      this.timer = setTimeout(this.UIlisten, this.selectTimeout);
    }
  }

  writeUISocket(packet) {
    console.log(`writing packet: ${JSON.stringify(packet)}`);
    return axsocket.writeUISocket(packet.from, packet.to, packet.data, packet.via);
  }
}

module.exports = UnixAX25;
